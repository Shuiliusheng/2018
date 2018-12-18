RegFU::RegFU(ParseXML* XML_interface, int ithCore_, 
	InputParameter* interface_ip_, const CoreDynParam & dyn_p_,
	bool exist_)
:XML(XML_interface),ithCore(ithCore_),
 interface_ip(*interface_ip_),coredynp(dyn_p_),
 IRF (0),FRF (0),RFWIN (0),exist(exist_)
 {
	if (!exist) return;
	
	//创建IRF，FRF对象
	IRF = new ArrayST(&interface_ip, "Integer Register File", Core_device, coredynp.opt_local, coredynp.core_ty);
	//计算IRF的面积=每个IRF/FRF的面积*硬件支持的线程数*流水线个数*overhead
	IRF->area.set_area(IRF->area.get_area()+ IRF->local_result.area*XML->sys.core[ithCore].number_hardware_threads*coredynp.num_pipelines*cdb_overhead);
	//将面积的结果累加到RegFU的area对象中
	area.set_area(area.get_area()+ IRF->local_result.area*XML->sys.core[ithCore].number_hardware_threads*coredynp.num_pipelines*cdb_overhead);
	
	//用于计算bypass网络时使用
	//此时需要考虑到bypass需要连接到所有寄存器，因此需要*硬件支持的线程数
	int_regfile_height= IRF->local_result.cache_ht*XML->sys.core[ithCore].number_hardware_threads*sqrt(cdb_overhead);
	fp_regfile_height = FRF->local_result.cache_ht*XML->sys.core[ithCore].number_hardware_threads*sqrt(cdb_overhead);
    
	//coredynp.regWindowing= 
	//		(XML->sys.core[ithCore].register_windows_size>0&&coredynp.core_ty==Inorder)?true:false;
	//按序处理器并且指定了寄存器窗口的大小，此时为真，否则为假
	if (coredynp.regWindowing)
	{
		//创建RFWIN对象，并且计算面积*流水线条数
		//RFWIN目前推测应该是为了保存异常发生时的寄存器堆或者是函数调用时保存寄存器
		RFWIN = new ArrayST(&interface_ip, "RegWindow", Core_device, coredynp.opt_local, coredynp.core_ty);
		RFWIN->area.set_area(RFWIN->area.get_area()+ RFWIN->local_result.area*coredynp.num_pipelines);
		area.set_area(area.get_area()+ RFWIN->local_result.area*coredynp.num_pipelines);
	}
 }

 
 void RegFU::computeEnergy(bool is_tdp)
{
	//PRF和ARF不会同时出现
	if (!exist) return;
	if (is_tdp)//初始化每周期的状态
    {
    	//1.1的目的：大约有10%的指令不会访问ALU，因此ALU_duty_cycle*1.1
		//根据Thumb得到的结论
    	IRF->stats_t.readAc.access  = coredynp.issueW*2*(coredynp.ALU_duty_cycle*1.1+
    		(coredynp.num_muls>0?coredynp.MUL_duty_cycle:0))*coredynp.num_pipelines;
    	IRF->stats_t.writeAc.access  = coredynp.issueW*(coredynp.ALU_duty_cycle*1.1+
    		(coredynp.num_muls>0?coredynp.MUL_duty_cycle:0))*coredynp.num_pipelines;
    	
		//1.05的目的和1.1类似
    	FRF->stats_t.readAc.access  = FRF->l_ip.num_rd_ports
			*coredynp.FPU_duty_cycle*1.05*coredynp.num_fp_pipelines;
    	FRF->stats_t.writeAc.access  = FRF->l_ip.num_wr_ports
			*coredynp.FPU_duty_cycle*1.05*coredynp.num_fp_pipelines;

		//似乎没有考虑这个东西，如果按序的情况下，需要自己更改
    	if (coredynp.regWindowing)
    	{
        	RFWIN->stats_t.readAc.access  = 0;//0.5*RFWIN->l_ip.num_rw_ports;
        	RFWIN->stats_t.writeAc.access  = 0;//0.5*RFWIN->l_ip.num_rw_ports;
    	}
    }
    else//初始化执行过程中的状态
    {
    	//整个执行过程中的一些操作的次数
    	IRF->stats_t.readAc.access  = XML->sys.core[ithCore].int_regfile_reads;
    	IRF->stats_t.writeAc.access  = XML->sys.core[ithCore].int_regfile_writes;

    	FRF->stats_t.readAc.access  = XML->sys.core[ithCore].float_regfile_reads;
    	FRF->stats_t.writeAc.access  = XML->sys.core[ithCore].float_regfile_writes;
		
    	if (coredynp.regWindowing)
    	{
			//在函数调用的时候(包括异常转移)时，保存寄存器
			//函数调用次数*需要保存的寄存器个数，16个
        	RFWIN->stats_t.readAc.access  = XML->sys.core[ithCore].function_calls*16;
        	RFWIN->stats_t.writeAc.access  = XML->sys.core[ithCore].function_calls*16;

			//IRF和FPR也需要增加这个因素(访问时双向的)
        	IRF->stats_t.readAc.access  = XML->sys.core[ithCore].int_regfile_reads +
        	     XML->sys.core[ithCore].function_calls*16;
        	IRF->stats_t.writeAc.access  = XML->sys.core[ithCore].int_regfile_writes +
        	     XML->sys.core[ithCore].function_calls*16;

        	FRF->stats_t.readAc.access  = XML->sys.core[ithCore].float_regfile_reads +
   	             XML->sys.core[ithCore].function_calls*16;;
        	FRF->stats_t.writeAc.access  = XML->sys.core[ithCore].float_regfile_writes+
   	             XML->sys.core[ithCore].function_calls*16;;
    	}
    }
	
	//初始化power参数
	IRF->power_t.reset();
	FRF->power_t.reset();
	
	//计算能耗
	//操作次数*操作的能量消耗
	IRF->power_t.readOp.dynamic  +=  (IRF->stats_t.readAc.access*IRF->local_result.power.readOp.dynamic
			+IRF->stats_t.writeAc.access*IRF->local_result.power.writeOp.dynamic);
	FRF->power_t.readOp.dynamic  +=  (FRF->stats_t.readAc.access*FRF->local_result.power.readOp.dynamic
			+FRF->stats_t.writeAc.access*FRF->local_result.power.writeOp.dynamic);
	if (coredynp.regWindowing)
	{
		RFWIN->power_t.reset();
		RFWIN->power_t.readOp.dynamic   +=  (RFWIN->stats_t.readAc.access*RFWIN->local_result.power.readOp.dynamic +
				RFWIN->stats_t.writeAc.access*RFWIN->local_result.power.writeOp.dynamic);
	}

	//两者基本一样，保存结果的参数不同，power/rt_power
	if (is_tdp)
	{
		//pppm_lkg_multhread={0, coredynp.num_hthreads, coredynp.num_hthreads, 0}
		//泄露功耗需要考虑到有多少个硬件线程，因为每一个硬件线程都需要有寄存器
		IRF->power  =  IRF->power_t + IRF->local_result.power *coredynp.pppm_lkg_multhread;
		FRF->power  =  FRF->power_t + FRF->local_result.power *coredynp.pppm_lkg_multhread;
		power	    =  power + (IRF->power + FRF->power);
		
		if (coredynp.regWindowing)
		{
			//pppm_lkg={0,1,1,0}
			RFWIN->power = RFWIN->power_t + RFWIN->local_result.power *pppm_lkg;
			power        = power + RFWIN->power;
		}
	}
	else
	{
		IRF->rt_power  =  IRF->power_t + IRF->local_result.power *coredynp.pppm_lkg_multhread;
		FRF->rt_power  =  FRF->power_t + FRF->local_result.power *coredynp.pppm_lkg_multhread;
		rt_power	   =  rt_power + (IRF->power_t + FRF->power_t);
		if (coredynp.regWindowing)
		{
			RFWIN->rt_power = RFWIN->power_t + RFWIN->local_result.power *pppm_lkg;
			rt_power        = rt_power + RFWIN->rt_power;
		}
	}
}


void RegFU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;
	if (is_tdp)
	{	
		/*
			Integer RF::
			Floating Point RF:
			
			Area = IRF->area.get_area()*1e-6
			Peak Dynamic = IRF->power.readOp.dynamic*clockRate
			Subthreshold Leakage = (long_channel? 
					IRF->power.readOp.longer_channel_leakage:IRF->power.readOp.leakage)
			Subthreshold Leakage with power gating = (long_channel? 
					IRF->power.readOp.power_gated_with_long_channel_leakage : 
					IRF->power.readOp.power_gated_leakage) 
			Runtime Dynamic = IRF->rt_power.readOp.dynamic/executionTime
		*/
		if (coredynp.regWindowing)
		{
			//Register Windows:RFWIN
		}
	}
	else
	{
		//Peak Dynamic = IRF->rt_power.readOp.dynamic*clockRate
		//Subthreshold Leakage = IRF->rt_power.readOp.leakage
		//Gate Leakage = IRF->rt_power.readOp.gate_leakage
	}
}
