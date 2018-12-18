Core::Core(ParseXML* XML_interface, int ithCore_, InputParameter* interface_ip_)
:XML(XML_interface),ithCore(ithCore_),
 interface_ip(*interface_ip_),
 ifu  (0),lsu  (0),mmu  (0),
 exu  (0),rnu  (0),corepipe (0),
 undiffCore (0)   ,l2cache (0)
{

	bool exit_flag = true;

	//临时参数，用于就算流水线平摊到功能单元的面积
	double pipeline_area_per_unit;
	//设置Core的一些基本参数，从xml文件中获取信息
	set_core_param();

	//如果有私有的L2cache，将创建对象，在Core中计算
	if (XML->sys.Private_L2)
	{
		l2cache = new SharedCache(XML,ithCore, &interface_ip);
	}

	//创建Core的各个阶段的对象和undifferentiated core(未分化的core)
	ifu          = new InstFetchU(XML, ithCore, &interface_ip,coredynp,exit_flag);
	lsu          = new LoadStoreU(XML, ithCore, &interface_ip,coredynp,exit_flag);
	mmu          = new MemManU   (XML, ithCore, &interface_ip,coredynp,exit_flag);
	exu          = new EXECU     (XML, ithCore, &interface_ip,lsu->lsq_height, coredynp,exit_flag);
	undiffCore   = new UndiffCore(XML, ithCore, &interface_ip,coredynp,exit_flag);

	//如果是乱序核，创建重命名逻辑的对象
	if (coredynp.core_ty==OOO)
	{
		rnu = new RENAMINGU(XML, ithCore, &interface_ip,coredynp);
	}
	//创建流水线对象，主要计算每个流水级需要保存的数据，因此产生的开销
	corepipe = new Pipeline(&interface_ip,coredynp);

	//将流水线带来的面积，平均分散到每个部件中，最终不会单独显示流水级的面积
	//pipeline_area_per_unit
	if (coredynp.core_ty==OOO)
	{
		//OOO中给定了5个unit，ifu, lsu, mmu, exu, rnu
		//将流水线的面积平均到每个单元
		pipeline_area_per_unit  = (corepipe->area.get_area()*coredynp.num_pipelines)/5.0;
		if (rnu->exist)
			rnu->area.set_area(rnu->area.get_area() + pipeline_area_per_unit);
	}
	else //inorder中不包括rnu
	{
		pipeline_area_per_unit    = (corepipe->area.get_area()*coredynp.num_pipelines)/4.0;
	}

	//lsu,exu, mmu,undiffcore, private_l2,rnu
	if (ifu->exist)
	{
		ifu->area.set_area(ifu->area.get_area() + pipeline_area_per_unit);
		area.set_area(area.get_area() + ifu->area.get_area());
	}
}




void Core::computeEnergy(bool is_tdp)
{
	/*
	 * When computing TDP, power = energy_per_cycle (the value computed in this function) * clock_rate (in the display_energy function)
	 * When computing dyn_power; power = total energy (the value computed in this function) / Total execution time (cycle count / clock rate)
	 */
	//power_point_product_masks
	double pppm_t[4]    = {1,1,1,1};
    double rtp_pipeline_coe;
    double num_units = 4.0;
	
	if (is_tdp)
	{
		ifu->computeEnergy(is_tdp);
		lsu->computeEnergy(is_tdp);
		mmu->computeEnergy(is_tdp);
		exu->computeEnergy(is_tdp);

		if (coredynp.core_ty==OOO)
		{
			num_units = 5.0;
			rnu->computeEnergy(is_tdp);
			//coredynp.num_pipelines/num_units
			//将功耗平均到每个单元中，不单独显示流水级而带来的功耗
			set_pppm(pppm_t, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);//User need to feed a duty cycle to improve accuracy
			if (rnu->exist)
			{
				rnu->power = rnu->power + corepipe->power*pppm_t;
				power     = power + rnu->power;
			}
		}

		if (ifu->exist)
		{
			//IFU_duty_cycle，ifu在每周期的工作时间
			
			//平均的结果=流水线的参数*流水线个数/单元数
			//对于每周期的能耗开销，需要考虑到当前单元在一个周期内的占空比
			//即分摊只在当前单元工作的时候会分摊能耗
			set_pppm(pppm_t, coredynp.num_pipelines/num_units*coredynp.IFU_duty_cycle, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			ifu->power = ifu->power + corepipe->power*pppm_t;
			power     = power + ifu->power;
		}
		if (lsu->exist)
		{
			//LSU_duty_cycle，lsu在每周期的工作时间
			set_pppm(pppm_t, coredynp.num_pipelines/num_units*coredynp.LSU_duty_cycle, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			lsu->power = lsu->power + corepipe->power*pppm_t;
			power     = power + lsu->power;
		}
		if (exu->exist)
		{
			//ALU_duty_cycle，alu在每周期的工作时间
			set_pppm(pppm_t, coredynp.num_pipelines/num_units*coredynp.ALU_duty_cycle, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			exu->power = exu->power + corepipe->power*pppm_t;
			power     = power + exu->power;
		}
		if (mmu->exist)
		{
			//(0.5+0.5*coredynp.LSU_duty_cycle)
			//itlb一定工作，dtlb只在lsu工作的时候工作
			set_pppm(pppm_t, coredynp.num_pipelines/num_units*(0.5+0.5*coredynp.LSU_duty_cycle), coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			mmu->power = mmu->power + corepipe->power*pppm_t;
			power     = power +  mmu->power;
		}
		
		power     = power +  undiffCore->power;

		if (XML->sys.Private_L2)
		{
			l2cache->computeEnergy(is_tdp);
			//根据时钟的不同，计算得到峰值功耗的结果
			set_pppm(pppm_t,l2cache->cachep.clockRate/clockRate, 1,1,1);
			power = power  + l2cache->power*pppm_t;
		}

	}
	else
	{
		ifu->computeEnergy(is_tdp);
		lsu->computeEnergy(is_tdp);
		mmu->computeEnergy(is_tdp);
		exu->computeEnergy(is_tdp);

		if (coredynp.core_ty==OOO)
		{
			num_units = 5.0;
			rnu->computeEnergy(is_tdp);
			//rtp_pipeline_coe代表流水线的整体工作时长
			//如果是同构多核，需要乘以核数
			if (XML->sys.homogeneous_cores==1)
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * XML->sys.total_cycles * XML->sys.number_of_cores;
			else
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.total_cycles;
			
			//将流水线带来的动态功耗平摊到每个单元上
        	set_pppm(pppm_t, coredynp.num_pipelines*rtp_pipeline_coe/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			if (rnu->exist)
			{
				rnu->rt_power = rnu->rt_power + corepipe->power*pppm_t;
				rt_power      = rt_power + rnu->rt_power;
			}
		}
		else
		{
			num_units = 4.0;
		}

		if (ifu->exist)
		{
			//考虑IFU_duty_cycle，即IFU的工作时间内，流水线会消耗的整体动态功耗
			
			//t=coredynp.pipeline_duty_cycle*XML->sys.total_cycles
			//计算单个流水线在整体执行过程中会执行的时钟周期数
			//t*coredynp.IFU_duty_cycle
			//计算在IFU工作的时间内流水线的工作时长
			if (XML->sys.homogeneous_cores==1)
			{
				//如果是同构多核的情况，需要将核数考虑进去
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.IFU_duty_cycle * XML->sys.total_cycles * XML->sys.number_of_cores;
			}
			else
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.IFU_duty_cycle * coredynp.total_cycles;
			}
			set_pppm(pppm_t, coredynp.num_pipelines*rtp_pipeline_coe/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			//corepipe->power代表着每周期流水线的消耗的能耗
			//dynamic *(t1*rtp_pipeline_coe)
			//代表在整个运行时间中，流水线会消耗的能耗/单元数
			ifu->rt_power = ifu->rt_power + corepipe->power*pppm_t;
			rt_power     = rt_power + ifu->rt_power ;
		}
		if (lsu->exist)
		{
			//考虑LSU_duty_cycle，即LSU的工作时间内，流水线会消耗的整体动态功耗
			if (XML->sys.homogeneous_cores==1)
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.LSU_duty_cycle * XML->sys.total_cycles * XML->sys.number_of_cores;
			}
			else
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.LSU_duty_cycle * coredynp.total_cycles;
			}
			set_pppm(pppm_t, coredynp.num_pipelines*rtp_pipeline_coe/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);

			lsu->rt_power = lsu->rt_power + corepipe->power*pppm_t;
			rt_power     = rt_power  + lsu->rt_power;
		}
		if (exu->exist)
		{
			//考虑ALU_duty_cycle，即ALU的工作时间内，流水线会消耗的整体动态功耗
			if (XML->sys.homogeneous_cores==1)
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.ALU_duty_cycle * XML->sys.total_cycles * XML->sys.number_of_cores;
			}
			else
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * coredynp.ALU_duty_cycle * coredynp.total_cycles;
			}
			set_pppm(pppm_t, coredynp.num_pipelines*rtp_pipeline_coe/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			exu->rt_power = exu->rt_power + corepipe->power*pppm_t;
			rt_power     = rt_power  + exu->rt_power;
		}
		if (mmu->exist)
		{
			//考虑(0.5+0.5*coredynp.LSU_duty_cycle)，即mmu的工作时间内，流水线会消耗的整体动态功耗
			if (XML->sys.homogeneous_cores==1)
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * (0.5+0.5*coredynp.LSU_duty_cycle) * XML->sys.total_cycles * XML->sys.number_of_cores;
			}
			else
			{
				rtp_pipeline_coe = coredynp.pipeline_duty_cycle * (0.5+0.5*coredynp.LSU_duty_cycle) * coredynp.total_cycles;
			}
			set_pppm(pppm_t, coredynp.num_pipelines*rtp_pipeline_coe/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units, coredynp.num_pipelines/num_units);
			mmu->rt_power = mmu->rt_power + corepipe->power*pppm_t;
			rt_power     = rt_power +  mmu->rt_power ;

		}
		rt_power     = rt_power +  undiffCore->power;
		if (XML->sys.Private_L2)
		{
			l2cache->computeEnergy(is_tdp);
			rt_power = rt_power  + l2cache->rt_power;
		}
	}

}

void Core::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (is_tdp)
	{
		/*
			Core:
			Area = area.get_area()*1e-6
			Peak Dynamic =  power.readOp.dynamic*clockRate
			Subthreshold Leakage = (long_channel? 	
					power.readOp.longer_channel_leakage:power.readOp.leakage)
			Subthreshold Leakage with power gating = (long_channel? 
					power.readOp.power_gated_with_long_channel_leakage : power.readOp.power_gated_leakage) 
			Gate Leakage = power.readOp.gate_leakage
			Runtime Dynamic = rt_power.readOp.dynamic/executionTime
		*/

		if (ifu->exist)
		{
			//Instruction Fetch Unit:ifu
			if (plevel >2){
				ifu->displayEnergy(indent+4,plevel,is_tdp);
			}
		}
		if (coredynp.core_ty==OOO)
		{
			if (rnu->exist)
			{
				//Renaming Unit:rnu
				if (plevel >2){
					rnu->displayEnergy(indent+4,plevel,is_tdp);
				}
			}

		}
		if (lsu->exist)
		{
			//Load Store Unit:lsu
			if (plevel >2){
				lsu->displayEnergy(indent+4,plevel,is_tdp);
			}
		}
		if (mmu->exist)
		{
			//Memory Management Unit:mmu
			if (plevel >2){
				mmu->displayEnergy(indent+4,plevel,is_tdp);
			}
		}
		if (exu->exist)
		{
			//Execution Unit:exu
			if (plevel >2){
				exu->displayEnergy(indent+4,plevel,is_tdp);
			}
		}

		if (XML->sys.Private_L2)
		{
			l2cache->displayEnergy(4,is_tdp);
		}

	}
	else
	{
	}
}
