SchedulerU::SchedulerU(ParseXML* XML_interface, int ithCore_, 
	InputParameter* interface_ip_, 
	const CoreDynParam & dyn_p_, bool exist_)
:XML(XML_interface),ithCore(ithCore_),interface_ip(*interface_ip_),
 coredynp(dyn_p_),int_inst_window(0),
 fp_inst_window(0),ROB(0),instruction_selection(0),
 exist(exist_)
{
	if (!exist) return;
	bool  is_default=true;

	//按序处理器中(多线程的情况)只定义了整型的指令窗口
	//主要是从多个线程中选择指令
	if ((coredynp.core_ty==Inorder && coredynp.multithreaded))
	{
		int_inst_window = new ArrayST("InstFetchQueue");
		//area=int_inst_window->local_result.area*coredynp.num_pipelines
		//bypass逻辑会连到指令窗口，因此需要计算height
		Iw_height      =int_inst_window->local_result.cache_ht;

		//对于指令的选择逻辑，没有考虑面积影响，只计算了功耗
		instruction_selection = new selection_logic(
				coredynp.peak_issueW*XML->sys.core[ithCore].number_hardware_threads);
	}

	//乱序的情况下，则定义了整型，浮点的指令窗口和ROB
    if (coredynp.core_ty==OOO)
    {
		//tmp_name=issueQueue / Reservation Station
		int_inst_window = new ArrayST(tmp_name);
		//area = int_inst_window->local_result.area*coredynp.num_pipelines);
		
		//计算height
		Iw_height      =int_inst_window->local_result.cache_ht;

		fp_inst_window = new ArrayST(tmp_name);
		fp_Iw_height      =fp_inst_window->local_result.cache_ht;

		if (XML->sys.core[ithCore].ROB_size >0)
		{
			ROB = new ArrayST("ReorderBuffer");
			//area=ROB->local_result.area*coredynp.num_pipelines
			
			//bypass也会连接到ROB中
			ROB_height      =ROB->local_result.cache_ht;
		}

		//不计算选择逻辑的面积
		instruction_selection = new selection_logic(coredynp.peak_issueW);
    }
}


void SchedulerU::computeEnergy(bool is_tdp)
{
	if (!exist) return;
	double ROB_duty_cycle;
	ROB_duty_cycle = 1;
	
	if (is_tdp)
	{
		//如果是乱序核，则包括三个对象
		if (coredynp.core_ty==OOO)
		{
			//三个操作，查找，读取，写入
			//用发射宽度确定
			int_inst_window->stats_t.readAc.access    = coredynp.issueW*coredynp.num_pipelines;
			int_inst_window->stats_t.writeAc.access   = coredynp.issueW*coredynp.num_pipelines;
			int_inst_window->stats_t.searchAc.access  = coredynp.issueW*coredynp.num_pipelines;
			
			//用端口数确定，假设每周期都在工作
			fp_inst_window->stats_t.readAc.access     = fp_inst_window->l_ip.num_rd_ports*coredynp.num_fp_pipelines;
			fp_inst_window->stats_t.writeAc.access    = fp_inst_window->l_ip.num_wr_ports*coredynp.num_fp_pipelines;
			fp_inst_window->stats_t.searchAc.access   = fp_inst_window->l_ip.num_search_ports*coredynp.num_fp_pipelines;

			if (XML->sys.core[ithCore].ROB_size >0)
			{
				//当指令提交时，ROB一定会被读取一次
				//PRF时读取物理寄存器tag，RS时读取数据
				//没有查找操作，因为是顺序的访问
				ROB->stats_t.readAc.access   = coredynp.commitW*coredynp.num_pipelines*ROB_duty_cycle;
				ROB->stats_t.writeAc.access  = coredynp.issueW*coredynp.num_pipelines*ROB_duty_cycle;
			}

		}
		else if (coredynp.multithreaded)//按序多线程的情况下
		{
			int_inst_window->stats_t.readAc.access   = coredynp.issueW*coredynp.num_pipelines;
			int_inst_window->stats_t.writeAc.access  = coredynp.issueW*coredynp.num_pipelines;
			int_inst_window->stats_t.searchAc.access = coredynp.issueW*coredynp.num_pipelines;
		}

     }
    else
    {
		if (coredynp.core_ty==OOO)
		{
			int_inst_window->stats_t.readAc.access   = XML->sys.core[ithCore].inst_window_reads;
			int_inst_window->stats_t.writeAc.access  = XML->sys.core[ithCore].inst_window_writes;
			int_inst_window->stats_t.searchAc.access = XML->sys.core[ithCore].inst_window_wakeup_accesses;
	
			fp_inst_window->stats_t.readAc.access    = XML->sys.core[ithCore].fp_inst_window_reads;
			fp_inst_window->stats_t.writeAc.access   = XML->sys.core[ithCore].fp_inst_window_writes;
			fp_inst_window->stats_t.searchAc.access  = XML->sys.core[ithCore].fp_inst_window_wakeup_accesses;


			if (XML->sys.core[ithCore].ROB_size >0)
			{
				//ROB的写操作：当指令插入时/当结果产生时
				//ROB的读操作：当指令提交时
				ROB->stats_t.readAc.access   = XML->sys.core[ithCore].ROB_reads;
				ROB->stats_t.writeAc.access  = XML->sys.core[ithCore].ROB_writes;
			}

		}
		else if (coredynp.multithreaded)
		{
			int_inst_window->stats_t.readAc.access    = 
				XML->sys.core[ithCore].int_instructions + XML->sys.core[ithCore].fp_instructions;
			int_inst_window->stats_t.writeAc.access   = 
				XML->sys.core[ithCore].int_instructions + XML->sys.core[ithCore].fp_instructions;
			//两个操作数都要查找指令窗口
			int_inst_window->stats_t.searchAc.access  = 
				2*(XML->sys.core[ithCore].int_instructions + XML->sys.core[ithCore].fp_instructions);
		}
    }

	//computation engine
	if (coredynp.core_ty==OOO)
	{
		int_inst_window->power_t.reset();
		fp_inst_window->power_t.reset();

		//指令窗口的读操作次数*能耗+写操作*能耗+搜索操作*能耗+选择逻辑能耗*次数
		//选择逻辑参与次数为指令窗口的访问次数(包括int和fp)
		int_inst_window->power_t.readOp.dynamic  +=  
					int_inst_window->local_result.power.readOp.dynamic * int_inst_window->stats_t.readAc.access
					+ int_inst_window->local_result.power.searchOp.dynamic * int_inst_window->stats_t.searchAc.access
					+ int_inst_window->local_result.power.writeOp.dynamic  * int_inst_window->stats_t.writeAc.access
					+ int_inst_window->stats_t.readAc.access * instruction_selection->power.readOp.dynamic;

		fp_inst_window->power_t.readOp.dynamic   +=  
					fp_inst_window->local_result.power.readOp.dynamic * fp_inst_window->stats_t.readAc.access
					+ fp_inst_window->local_result.power.searchOp.dynamic * fp_inst_window->stats_t.searchAc.access
					+ fp_inst_window->local_result.power.writeOp.dynamic * fp_inst_window->stats_t.writeAc.access
					+ fp_inst_window->stats_t.writeAc.access * instruction_selection->power.readOp.dynamic;

		//计算ROB的能耗开销：读次数*能耗+写次数*能耗
		if (XML->sys.core[ithCore].ROB_size >0)
		{
			ROB->power_t.reset();
			ROB->power_t.readOp.dynamic   +=  ROB->local_result.power.readOp.dynamic*ROB->stats_t.readAc.access +
						ROB->stats_t.writeAc.access*ROB->local_result.power.writeOp.dynamic;
		}
	}
	else if (coredynp.multithreaded)
	{
		int_inst_window->power_t.reset();
		//读操作，写操作，搜索操作+选择逻辑的参与次数
		int_inst_window->power_t.readOp.dynamic  += 
			int_inst_window->local_result.power.readOp.dynamic * int_inst_window->stats_t.readAc.access
		  + int_inst_window->local_result.power.searchOp.dynamic * int_inst_window->stats_t.searchAc.access
		  + int_inst_window->local_result.power.writeOp.dynamic  * int_inst_window->stats_t.writeAc.access
		  + int_inst_window->stats_t.writeAc.access * instruction_selection->power.readOp.dynamic;
	}

	//赋值，基本一致，目的不同：power/rt_power
	if (is_tdp)
	{
		if (coredynp.core_ty==OOO)
		{
			//pppm_lkg={0,1,1,0}
			//计算泄露功耗
			int_inst_window->power = int_inst_window->power_t + 
				(int_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			fp_inst_window->power = fp_inst_window->power_t + 
				(fp_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			power = power + int_inst_window->power + fp_inst_window->power;
			if (XML->sys.core[ithCore].ROB_size >0)
			{
				ROB->power = ROB->power_t + ROB->local_result.power*pppm_lkg;
				power	   = power + ROB->power;
			}

		}
		else if (coredynp.multithreaded)
		{
			int_inst_window->power = int_inst_window->power_t + 
				(int_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			power	   = power + int_inst_window->power;
      	}

    }
    else
    {//rtp
		if (coredynp.core_ty==OOO)
		{
			int_inst_window->rt_power = int_inst_window->power_t + 
				(int_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			fp_inst_window->rt_power  = fp_inst_window->power_t + 
				(fp_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			rt_power	              = rt_power + 
				int_inst_window->rt_power + fp_inst_window->rt_power;
			if (XML->sys.core[ithCore].ROB_size >0)
			{
				ROB->rt_power = ROB->power_t + ROB->local_result.power*pppm_lkg;
				rt_power	              = rt_power + ROB->rt_power;
			}

		}
		else if (coredynp.multithreaded)
		{
			int_inst_window->rt_power = int_inst_window->power_t + 
				(int_inst_window->local_result.power +instruction_selection->power) *pppm_lkg;
			rt_power	              = rt_power + int_inst_window->rt_power;
      	}
    }
}

void SchedulerU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;
	bool long_channel = XML->sys.longer_channel_device;
	bool power_gating = XML->sys.power_gating;
	if (is_tdp)
	{
		if (coredynp.core_ty==OOO)
		{
			/*
				Instruction Window:
				FP Instruction Window:
				ROB:
				
				
				Area = int_inst_window->area.get_area()*1e-6
				Peak Dynamic = int_inst_window->power.readOp.dynamic*clockRate
				Subthreshold Leakage = (long_channel? 		
					int_inst_window->power.readOp.longer_channel_leakage:int_inst_window->power.readOp.leakage) 
				Subthreshold Leakage with power gating = long_channel? 
					int_inst_window->power.readOp.power_gated_with_long_channel_leakage : 
					int_inst_window->power.readOp.power_gated_leakage
				Gate Leakage = int_inst_window->power.readOp.gate_leakage
				Runtime Dynamic = int_inst_window->rt_power.readOp.dynamic/executionTime
			*/
		}
		else if (coredynp.multithreaded)
		{
			//Instruction Window:
		}
	}
	else
	{
		if (coredynp.core_ty==OOO)
		{
			//Peak Dynamic = int_inst_window->rt_power.readOp.dynamic*clockRate 
			//Subthreshold Leakage = int_inst_window->rt_power.readOp.leakage 
			//Gate Leakage = int_inst_window->rt_power.readOp.gate_leakage 
		}
		else if (coredynp.multithreaded)
		{
			//Instruction Window:
		}
	}
}

