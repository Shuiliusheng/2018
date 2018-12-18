InstFetchU::InstFetchU(ParseXML* XML_interface, int ithCore_, 
	InputParameter* interface_ip_, 
	const CoreDynParam & dyn_p_, bool exist_)
:XML(XML_interface),ithCore(ithCore_),interface_ip(*interface_ip_),
 coredynp(dyn_p_),IB  (0),BTB (0),
 ID_inst  (0),ID_operand  (0),ID_misc  (0),exist(exist_)
{
	if (!exist) return;

	//cacti/parameter.cc:45:TechnologyParameter g_tp
	//一些工艺参数
	scktRatio = g_tp.sckt_co_eff;
	chip_PR_overhead = g_tp.chip_layout_overhead;
	macro_PR_overhead = g_tp.macro_layout_overhead;

	//创建icache中的一些对象，caches, missb, ifb, prefetchb
	//将这些组件的面积大小累加到icache的area和InstFetchU的area中
	icache.caches = new ArrayST(&interface_ip, "icache", Core_device, coredynp.opt_local, coredynp.core_ty);
	//area=icache.missb->local_result.area
	
	//创建指令buffer
	IB = new ArrayST(&interface_ip, "InstBuffer", Core_device, coredynp.opt_local, coredynp.core_ty);
	//IB->local_result.area
	
	//如果预测宽度大于零，意味着有预测器
	if (coredynp.predictionW>0)
	{
		//创建BTB和BPT对象
		BTB = new ArrayST(&interface_ip, "Branch Target Buffer", Core_device, coredynp.opt_local, coredynp.core_ty);
		//BTB->local_result.area

		BPT = new BranchPredictor(XML, ithCore, &interface_ip,coredynp);
		//BPT->area.get_area()
	}

	  
	//暂时不关心
	//译码指令类型
	ID_inst = new inst_decoder(is_default, &interface_ip,
		  coredynp.opcode_length, 1, coredynp.x86,
		  Core_device, coredynp.core_ty);
	//译码指令中的寄存器
	ID_operand = new inst_decoder(is_default, &interface_ip,
		  coredynp.arch_ireg_width, 1,coredynp.x86,
		  Core_device, coredynp.core_ty);
	//译码x86指令？
	ID_misc = new inst_decoder(is_default, &interface_ip,
		  8, 1,coredynp.x86,
		  Core_device, coredynp.core_ty);
	
	//根据译码宽度判断译码器的个数，然后将计算的面积放入InstFetchU的area中
	area.set_area(area.get_area()+ coredynp.decodeW*(
		   ID_inst->area.get_area()
		  +ID_operand->area.get_area()
		  +ID_misc->area.get_area()));

}



void InstFetchU::computeEnergy(bool is_tdp)
{
	if (!exist) return;
//初始化组件的状态信息
	if (is_tdp)//初始化组件在每周期内的状态参数
    {
		//此时计算的是caches每个周期中的操作数目，
		//为了之后计算峰值功耗，因此有些数据会较高
		//对于caches会有三个操作，访问，miss，hit
		//access=读写端口数*取指部件的占空比(一个周期内的工作时间)
    	icache.caches->stats_t.readAc.access  = icache.caches->l_ip.num_rw_ports*coredynp.IFU_duty_cycle;
    	icache.caches->stats_t.readAc.miss    = 0;
    	icache.caches->stats_t.readAc.hit     = icache.caches->stats_t.readAc.access - icache.caches->stats_t.readAc.miss;
    	icache.caches->tdp_stats = icache.caches->stats_t;

    	icache.missb->stats_t.readAc.access  = icache.missb->stats_t.readAc.hit=  icache.missb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.missb->stats_t.writeAc.access = icache.missb->stats_t.writeAc.hit= icache.missb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.missb->tdp_stats = icache.missb->stats_t;

    	icache.ifb->stats_t.readAc.access  = icache.ifb->stats_t.readAc.hit=  icache.ifb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.ifb->stats_t.writeAc.access = icache.ifb->stats_t.writeAc.hit= icache.ifb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.ifb->tdp_stats = icache.ifb->stats_t;

    	icache.prefetchb->stats_t.readAc.access  = icache.prefetchb->stats_t.readAc.hit= icache.prefetchb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.prefetchb->stats_t.writeAc.access = icache.ifb->stats_t.writeAc.hit= icache.ifb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	icache.prefetchb->tdp_stats = icache.prefetchb->stats_t;

    	IB->stats_t.readAc.access = IB->stats_t.writeAc.access = XML->sys.core[ithCore].peak_issue_width;
    	IB->tdp_stats = IB->stats_t;

    	if (coredynp.predictionW>0)
    	{
    		BTB->stats_t.readAc.access  = coredynp.predictionW;//XML->sys.core[ithCore].BTB.read_accesses;
    		BTB->stats_t.writeAc.access = 0;//XML->sys.core[ithCore].BTB.write_accesses;
    	}

    	ID_inst->stats_t.readAc.access     = coredynp.decodeW;
    	ID_operand->stats_t.readAc.access  = coredynp.decodeW;
    	ID_misc->stats_t.readAc.access     = coredynp.decodeW;
    }
    else//初始化整个运行过程中的组件的状态参数
    {
     	//此时计算的是在整个运行过程中，组件的操作数目
		//这是为了之后计算组件的运行时的动态功耗
		//access=xml中给定的icache的读取次数
    	icache.caches->stats_t.readAc.access  = XML->sys.core[ithCore].icache.read_accesses;
    	icache.caches->stats_t.readAc.miss    = XML->sys.core[ithCore].icache.read_misses;
    	icache.caches->stats_t.readAc.hit     = icache.caches->stats_t.readAc.access - icache.caches->stats_t.readAc.miss;
    	icache.caches->rtp_stats = icache.caches->stats_t;

    	icache.missb->stats_t.readAc.access  = icache.caches->stats_t.readAc.miss;
    	icache.missb->stats_t.writeAc.access = icache.caches->stats_t.readAc.miss;
    	icache.missb->rtp_stats = icache.missb->stats_t;

    	icache.ifb->stats_t.readAc.access  = icache.caches->stats_t.readAc.miss;
    	icache.ifb->stats_t.writeAc.access = icache.caches->stats_t.readAc.miss;
    	icache.ifb->rtp_stats = icache.ifb->stats_t;

    	icache.prefetchb->stats_t.readAc.access  = icache.caches->stats_t.readAc.miss;
    	icache.prefetchb->stats_t.writeAc.access = icache.caches->stats_t.readAc.miss;
    	icache.prefetchb->rtp_stats = icache.prefetchb->stats_t;

    	IB->stats_t.readAc.access = IB->stats_t.writeAc.access = XML->sys.core[ithCore].total_instructions;
    	IB->rtp_stats = IB->stats_t;

    	if (coredynp.predictionW>0)
    	{
    		BTB->stats_t.readAc.access  = XML->sys.core[ithCore].BTB.read_accesses;//XML->sys.core[ithCore].branch_instructions;
    		BTB->stats_t.writeAc.access = XML->sys.core[ithCore].BTB.write_accesses;//XML->sys.core[ithCore].branch_mispredictions;
    		BTB->rtp_stats = BTB->stats_t;
    	}

    	ID_inst->stats_t.readAc.access     = XML->sys.core[ithCore].total_instructions;
    	ID_operand->stats_t.readAc.access  = XML->sys.core[ithCore].total_instructions;
    	ID_misc->stats_t.readAc.access     = XML->sys.core[ithCore].total_instructions;
    	ID_inst->rtp_stats = ID_inst->stats_t;
    	ID_operand->rtp_stats = ID_operand->stats_t;
    	ID_misc->rtp_stats = ID_misc->stats_t;

    }

//初始化状态之后，需要计算能耗(操作个数*操作消耗的能量)
    //将power中的参数初始化为0
	icache.power_t.reset();
    IB->power_t.reset();
    if (coredynp.predictionW>0)
    {
    	BTB->power_t.reset();
    }

    icache.power_t.readOp.dynamic	+= (icache.caches->stats_t.readAc.hit*icache.caches->local_result.power.readOp.dynamic+
    		//icache.caches->stats_t.readAc.miss*icache.caches->local_result.tag_array2->power.readOp.dynamic+
    		icache.caches->stats_t.readAc.miss*icache.caches->local_result.power.readOp.dynamic+ //assume tag data accessed in parallel
    		icache.caches->stats_t.readAc.miss*icache.caches->local_result.power.writeOp.dynamic); //read miss in Icache cause a write to Icache
    icache.power_t.readOp.dynamic	+=  icache.missb->stats_t.readAc.access*icache.missb->local_result.power.searchOp.dynamic +
            icache.missb->stats_t.writeAc.access*icache.missb->local_result.power.writeOp.dynamic;//each access to missb involves a CAM and a write
    icache.power_t.readOp.dynamic	+=  icache.ifb->stats_t.readAc.access*icache.ifb->local_result.power.searchOp.dynamic +
            icache.ifb->stats_t.writeAc.access*icache.ifb->local_result.power.writeOp.dynamic;
    icache.power_t.readOp.dynamic	+=  icache.prefetchb->stats_t.readAc.access*icache.prefetchb->local_result.power.searchOp.dynamic +
            icache.prefetchb->stats_t.writeAc.access*icache.prefetchb->local_result.power.writeOp.dynamic;

	//总的能耗=操作数*操作的能耗
	//energy=readOp.dynamic*readAc.access+writeAc.access*writeOp.dynamic
	IB->power_t.readOp.dynamic   +=  IB->local_result.power.readOp.dynamic*IB->stats_t.readAc.access +
			IB->stats_t.writeAc.access*IB->local_result.power.writeOp.dynamic;

	if (coredynp.predictionW>0)
	{
		BTB->power_t.readOp.dynamic   +=  BTB->local_result.power.readOp.dynamic*BTB->stats_t.readAc.access +
		BTB->stats_t.writeAc.access*BTB->local_result.power.writeOp.dynamic;

		BPT->computeEnergy(is_tdp);
	}

//最后计算泄露功耗(需要考虑线程数)，将结果保存在RENAMING的power/rt_power中
	//const double pppm_lkg[4]  = {0,1,1,0};
    if (is_tdp)
    {
    	icache.power = icache.power_t +
    	        (icache.caches->local_result.power +
    			icache.missb->local_result.power +
    			icache.ifb->local_result.power +
    			icache.prefetchb->local_result.power)*pppm_lkg;

    	//pppm_lkg={0,1,1,0}
		//power的四个参数是：dynamic, leakage, gate_leakage, short_circuit
		//因此结果是将local_result.power的leakage和gate_leakage
		//直接放入到IB->power的这两个参数中
		//IB->power的dynamic和short_circuit为IB->power_t的两个参数
		IB->power = IB->power_t + IB->local_result.power*pppm_lkg;
		//在tdp的情况下，将结果放入到InstFetchU的power中，否则放入rt_powet中
		power     = power + icache.power + IB->power;
    	if (coredynp.predictionW>0)
    	{
    		BTB->power = BTB->power_t + BTB->local_result.power*pppm_lkg;
    		power     = power  + BTB->power + BPT->power;
    	}

    	ID_inst->power_t.readOp.dynamic    = ID_inst->power.readOp.dynamic;
    	ID_operand->power_t.readOp.dynamic = ID_operand->power.readOp.dynamic;
    	ID_misc->power_t.readOp.dynamic    = ID_misc->power.readOp.dynamic;

    	ID_inst->power.readOp.dynamic    *= ID_inst->tdp_stats.readAc.access;
    	ID_operand->power.readOp.dynamic *= ID_operand->tdp_stats.readAc.access;
    	ID_misc->power.readOp.dynamic    *= ID_misc->tdp_stats.readAc.access;

    	power = power + (ID_inst->power +
							ID_operand->power +
							ID_misc->power);
    }
    else
    {
    	icache.rt_power = icache.power_t +
    	        (icache.caches->local_result.power +
    			icache.missb->local_result.power +
    			icache.ifb->local_result.power +
    			icache.prefetchb->local_result.power)*pppm_lkg;

    	IB->rt_power = IB->power_t + IB->local_result.power*pppm_lkg;
    	rt_power     = rt_power + icache.rt_power + IB->rt_power;
    	if (coredynp.predictionW>0)
    	{
    		BTB->rt_power = BTB->power_t + BTB->local_result.power*pppm_lkg;
    		rt_power     = rt_power + BTB->rt_power + BPT->rt_power;
    	}

    	ID_inst->rt_power.readOp.dynamic    = ID_inst->power_t.readOp.dynamic*ID_inst->rtp_stats.readAc.access;
    	ID_operand->rt_power.readOp.dynamic = ID_operand->power_t.readOp.dynamic * ID_operand->rtp_stats.readAc.access;
    	ID_misc->rt_power.readOp.dynamic    = ID_misc->power_t.readOp.dynamic * ID_misc->rtp_stats.readAc.access;

    	rt_power = rt_power + (ID_inst->rt_power +
							ID_operand->rt_power +
							ID_misc->rt_power);
    }
}

void InstFetchU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;

	if (is_tdp)
	{
		//icache
		//Area = icache.area.get_area()*1e-6
		//Peak Dynamic = icache.power.readOp.dynamic*clockRate
		//Subthreshold Leakage = (long_channel? 
		//		icache.power.readOp.longer_channel_leakage:icache.power.readOp.leakage)
		//Subthreshold Leakage with power gating = (long_channel? 
		//		icache.power.readOp.power_gated_with_long_channel_leakage 
		//		: icache.power.readOp.power_gated_leakage)
		//Gate Leakage = icache.power.readOp.gate_leakage
		//Runtime Dynamic = icache.rt_power.readOp.dynamic/executionTime 
		
		if (coredynp.predictionW>0)
		{
			//BTB
			if (BPT->exist)
			{
				//BPT
				if (plevel>3)
				{
					BPT->displayEnergy(indent+4, plevel, is_tdp);
				}
			}
		}
		//IB, Instruction Decoder:(ID_inst+ID_operand+ID_misc)
	}
}

