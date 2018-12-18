EXECU::EXECU(ParseXML* XML_interface, int ithCore_, 
	InputParameter* interface_ip_, double lsq_height_, 
	const CoreDynParam & dyn_p_, bool exist_)
:XML(XML_interface),ithCore(ithCore_),
 interface_ip(*interface_ip_),lsq_height(lsq_height_),
 coredynp(dyn_p_),rfu(0),scheu(0),fp_u(0),
 exeu(0),mul(0),int_bypass(0),intTagBypass(0),
 int_mul_bypass(0),intTag_mul_Bypass(0),fp_bypass(0),
 fpTagBypass(0),exist(exist_)
{
	if (!exist) return;
	//没有用处
	double fu_height = 0.0;

	//创建寄存器堆对象，调度单元对象，定点执行单元对象
	//并将这些组件的面积放入EXECU的area对象中
	rfu   = new RegFU(XML, ithCore, &interface_ip,coredynp);
	scheu = new SchedulerU(XML, ithCore, &interface_ip,coredynp);
	exeu  = new FunctionalUnit(XML, ithCore,&interface_ip, coredynp, ALU);
	area.set_area(area.get_area()+ exeu->area.get_area() + 
		rfu->area.get_area() +scheu->area.get_area() );
	
	//如果有fpu，则创建fpu对象
	//对象返回的参数已经考虑到了fpu的个数问题
	if (coredynp.num_fpus >0)
	{
		fp_u  = new FunctionalUnit(XML, ithCore,&interface_ip, coredynp, FPU);
		area.set_area(area.get_area()+ fp_u->area.get_area());
	}
	//如果有乘法器，则创建mul对象
	//该对象似乎是定点乘法
	if (coredynp.num_muls >0)
	{
		mul   = new FunctionalUnit(XML, ithCore,&interface_ip, coredynp, MUL);
		area.set_area(area.get_area()+ mul->area.get_area());
	}

	//如果是按序处理器的情况下，创建6个bypass逻辑对象(data/tag*mul/alu/fpu)
	if (coredynp.core_ty==Inorder)
	{
		int_bypass   = new interconnect("Int Bypass Data");
		bypass.area.set_area(bypass.area.get_area() + int_bypass->area.get_area());

		intTagBypass = new interconnect("Int Bypass tag" );
		bypass.area.set_area(bypass.area.get_area()  +intTagBypass->area.get_area());

		if (coredynp.num_muls>0)
		{
			int_mul_bypass     = new interconnect("Mul Bypass Data");
			bypass.area.set_area(bypass.area.get_area()  +int_mul_bypass->area.get_area());
			intTag_mul_Bypass  = new interconnect("Mul Bypass tag");
			bypass.area.set_area(bypass.area.get_area()  +intTag_mul_Bypass->area.get_area());
		}

		if (coredynp.num_fpus>0)
		{
			fp_bypass    = new interconnect("FP Bypass Data");
			bypass.area.set_area(bypass.area.get_area()  +fp_bypass->area.get_area());
			fpTagBypass  = new interconnect("FP Bypass tag");
			bypass.area.set_area(bypass.area.get_area()  +fpTagBypass->area.get_area());
		}
	}
	else//在乱序的情况下，当调度策略不同时，创建bypass需要传递的参数也不同
	{
		if (coredynp.scheu_ty==PhysicalRegFile)
		{
			//int_bypass,intTagBypass
			//int_mul_bypass,intTag_mul_Bypass
			//fp_bypass,fpTagBypass
		}
		else
		{
			//int_bypass,intTagBypass
			//int_mul_bypass,intTag_mul_Bypass
			//fp_bypass,fpTagBypass
		}
	}
	//加上bypass的面积
	area.set_area(area.get_area()+ bypass.area.get_area());
}

void EXECU::computeEnergy(bool is_tdp)
{
	if (!exist) return;
	double pppm_t[4]    = {1,1,1,1};

	//计算rfu,scheu,exeu的功耗，结果保存在power和rt_power中
	rfu->computeEnergy(is_tdp);
	scheu->computeEnergy(is_tdp);
	exeu->computeEnergy(is_tdp);
	
	//判断fpu和mul是否存在，如果存在计算功耗
	if (coredynp.num_fpus >0)
	{
		fp_u->computeEnergy(is_tdp);
	}
	if (coredynp.num_muls >0)
	{
		mul->computeEnergy(is_tdp);
	}

	//保存结果的目的对象不同
	if (is_tdp)
	{
		/* power的前四个属性值
			double dynamic;
			double leakage;
			double gate_leakage;
			double short_circuit;
		*/
		//2是指每个定点指令需要两个通过bypass传递2个操作数
		//不太清楚是两个bypass还是两次，应该是前者
		set_pppm(pppm_t, 2*coredynp.ALU_cdb_duty_cycle, 2, 2, 2*coredynp.ALU_cdb_duty_cycle);
		bypass.power = bypass.power + intTagBypass->power*pppm_t + int_bypass->power*pppm_t;
		if (coredynp.num_muls >0)
		{
			set_pppm(pppm_t, 2*coredynp.MUL_cdb_duty_cycle, 2, 2, 2*coredynp.MUL_cdb_duty_cycle);
			bypass.power = bypass.power + intTag_mul_Bypass->power*pppm_t + int_mul_bypass->power*pppm_t;
			power      = power + mul->power;
		}
		if (coredynp.num_fpus>0)
		{
			//浮点需要三个bypass路径
			set_pppm(pppm_t, 3*coredynp.FPU_cdb_duty_cycle, 3, 3, 3*coredynp.FPU_cdb_duty_cycle);
			bypass.power = bypass.power + fp_bypass->power*pppm_t  + fpTagBypass->power*pppm_t ;
			power      = power + fp_u->power;
		}

		power = power + rfu->power + exeu->power + bypass.power + scheu->power;
	}
	else
	{
		set_pppm(pppm_t, XML->sys.core[ithCore].cdb_alu_accesses, 2, 2, XML->sys.core[ithCore].cdb_alu_accesses);
		bypass.rt_power = bypass.rt_power + intTagBypass->power*pppm_t + int_bypass->power*pppm_t;

		if (coredynp.num_muls >0)
		{
			set_pppm(pppm_t, XML->sys.core[ithCore].cdb_mul_accesses, 2, 2, XML->sys.core[ithCore].cdb_mul_accesses);
			bypass.rt_power = bypass.rt_power + intTag_mul_Bypass->power*pppm_t + int_mul_bypass->power*pppm_t;
			rt_power      = rt_power + mul->rt_power;
		}

		if (coredynp.num_fpus>0)
		{
			set_pppm(pppm_t, XML->sys.core[ithCore].cdb_fpu_accesses, 3, 3, XML->sys.core[ithCore].cdb_fpu_accesses);
			bypass.rt_power = bypass.rt_power + fp_bypass->power*pppm_t;
			bypass.rt_power = bypass.rt_power + fpTagBypass->power*pppm_t;
			rt_power      = rt_power + fp_u->rt_power;
		}
		rt_power = rt_power + rfu->rt_power + exeu->rt_power + bypass.rt_power + scheu->rt_power;
	}
}


void EXECU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;

	if (is_tdp)
	{
		/*
			Register Files:rfu
			Instruction Scheduler:scheu
			Results Broadcast Bus:bypass
			Area = rfu->area.get_area()*1e-6
			Peak Dynamic = rfu->power.readOp.dynamic*clockRate
			Subthreshold Leakage = (long_channel? 
					rfu->power.readOp.longer_channel_leakage:rfu->power.readOp.leakage)
			Subthreshold Leakage with power gating = (long_channel? 
					rfu->power.readOp.power_gated_with_long_channel_leakage : 
					rfu->power.readOp.power_gated_leakage) 
			Runtime Dynamic = rfu->rt_power.readOp.dynamic/executionTime
		*/
		
		if (plevel>3){
			scheu->displayEnergy(indent+4,is_tdp);
		}
		exeu->displayEnergy(indent,is_tdp);
		
		if (coredynp.num_fpus>0)
		{
			fp_u->displayEnergy(indent,is_tdp);
		}
		
		if (coredynp.num_muls >0)
		{
			mul->displayEnergy(indent,is_tdp);
		}
		
	}
	else
	{
	}

}

