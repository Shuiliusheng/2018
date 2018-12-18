MemManU::MemManU(ParseXML* XML_interface, int ithCore_, InputParameter* interface_ip_, const CoreDynParam & dyn_p_,bool exist_)
:XML(XML_interface),ithCore(ithCore_),
 interface_ip(*interface_ip_),coredynp(dyn_p_),
 itlb(0),dtlb(0),
 exist(exist_)
{
	  if (!exist) return;
	  
	  itlb = new ArrayST(&interface_ip, "ITLB", Core_device, coredynp.opt_local, coredynp.core_ty);
	  itlb->area.set_area(itlb->area.get_area()+ itlb->local_result.area);
	  area.set_area(area.get_area()+ itlb->local_result.area);
	  
	  dtlb = new ArrayST(&interface_ip, "DTLB", Core_device, coredynp.opt_local, coredynp.core_ty);
	  dtlb->area.set_area(dtlb->area.get_area()+ dtlb->local_result.area);
	  area.set_area(area.get_area()+ dtlb->local_result.area);
}

void MemManU::computeEnergy(bool is_tdp)
{

	//当is_tdp为true时，power = energy_per_cycle* clock_rate
	//在该函数中只计算得到每个周期该组件会消耗的能量energy_per_cycle
	//在displayEnergy函数中，将会使用该公式计算得到峰值power，即每个周期都在工作时的功率。
	//此时的计算结果保存在power中
	
	//当is_tdp为false时，power = total energy / Total execution time。
	//同样该函数中只计算该组件在整个执行过程中会消耗的所有能量（使用组件的访问次数等计算），
	//在displayEnergy函数中，将其除以整体的执行时间（cycle count / clock rate），得到运行时的动态功耗。
	//此时的计算结果保存在rt_power中

	if (!exist) return;
	if (is_tdp)
    {
    	//init stats for Peak
    	itlb->stats_t.readAc.access  = itlb->l_ip.num_search_ports*coredynp.IFU_duty_cycle;
    	itlb->stats_t.readAc.miss    = 0;
    	itlb->stats_t.readAc.hit     = itlb->stats_t.readAc.access - itlb->stats_t.readAc.miss;
    	itlb->tdp_stats = itlb->stats_t;

    	dtlb->stats_t.readAc.access  = dtlb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;
    	dtlb->stats_t.readAc.miss    = 0;
    	dtlb->stats_t.readAc.hit     = dtlb->stats_t.readAc.access - dtlb->stats_t.readAc.miss;
    	dtlb->tdp_stats = dtlb->stats_t;
     }
    else
    {
    	//init stats for Runtime Dynamic (RTP)
    	itlb->stats_t.readAc.access  = XML->sys.core[ithCore].itlb.total_accesses;
    	itlb->stats_t.readAc.miss    = XML->sys.core[ithCore].itlb.total_misses;
    	itlb->stats_t.readAc.hit     = itlb->stats_t.readAc.access - itlb->stats_t.readAc.miss;
    	itlb->rtp_stats = itlb->stats_t;

    	dtlb->stats_t.readAc.access  = XML->sys.core[ithCore].dtlb.total_accesses;
    	dtlb->stats_t.readAc.miss    = XML->sys.core[ithCore].dtlb.total_misses;
    	dtlb->stats_t.readAc.hit     = dtlb->stats_t.readAc.access - dtlb->stats_t.readAc.miss;
    	dtlb->rtp_stats = dtlb->stats_t;
    }

    itlb->power_t.reset();
    dtlb->power_t.reset();
	//计算能耗=操作数*每个操作的能量开销
	itlb->power_t.readOp.dynamic +=  itlb->stats_t.readAc.access*itlb->local_result.power.searchOp.dynamic//FA spent most power in tag, so use total access not hits
	                      +itlb->stats_t.readAc.miss*itlb->local_result.power.writeOp.dynamic;
	dtlb->power_t.readOp.dynamic +=  dtlb->stats_t.readAc.access*dtlb->local_result.power.searchOp.dynamic//FA spent most power in tag, so use total access not hits
	                      +dtlb->stats_t.readAc.miss*dtlb->local_result.power.writeOp.dynamic;

	if (is_tdp)
	{
		//pppm_lkg={0，1，1，0}
		//power的第二个和第三个属性的累加：leakage，gate_leakage
		itlb->power = itlb->power_t + itlb->local_result.power *pppm_lkg;
		dtlb->power = dtlb->power_t + dtlb->local_result.power *pppm_lkg;
		power     = power + itlb->power + dtlb->power;
	}
	else
	{
		itlb->rt_power = itlb->power_t + itlb->local_result.power *pppm_lkg;
		dtlb->rt_power = dtlb->power_t + dtlb->local_result.power *pppm_lkg;
		rt_power     = rt_power + itlb->rt_power + dtlb->rt_power;
	}
}

void MemManU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;
	
	if (is_tdp)
	{
		/*
			Itlb:
			Dtlb:
			
			Area = itlb->area.get_area()*1e-6
			Peak Dynamic = itlb->power.readOp.dynamic*clockRate
			Subthreshold Leakage = (long_channel? 
					itlb->power.readOp.longer_channel_leakage:itlb->power.readOp.leakage)
			Subthreshold Leakage with power gating = (long_channel? 
					itlb->power.readOp.power_gated_with_long_channel_leakage : 
					itlb->power.readOp.power_gated_leakage) 
			Runtime Dynamic = itlb->rt_power.readOp.dynamic/executionTime
		*/
	}
	else
	{
		//Itlb:
		//Dtlb:
		//Peak Dynamic = itlb->rt_power.readOp.dynamic*clockRate
		//Subthreshold Leakage = itlb->rt_power.readOp.leakage
		//Gate Leakage = itlb->rt_power.readOp.gate_leakage
	}

}
