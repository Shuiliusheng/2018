LoadStoreU::LoadStoreU(ParseXML* XML_interface, int ithCore_, InputParameter* interface_ip_, const CoreDynParam & dyn_p_,bool exist_)
:XML(XML_interface),ithCore(ithCore_),interface_ip(*interface_ip_),
 coredynp(dyn_p_),LSQ(0),LoadQ(0),exist(exist_)
{
	if (!exist) return;
	
	//定义dcache的caches对象，并获取面积信息，将其累加到Dcache的area和LoadstoreU的area中
	dcache.caches = new ArrayST(&interface_ip, "dcache", Core_device, coredynp.opt_local, coredynp.core_ty);
	dcache.area.set_area(dcache.area.get_area()+ dcache.caches->local_result.area);
	area.set_area(area.get_area()+ dcache.caches->local_result.area);

	//定义dcache的missb, ifb, prefetchb
	//...

	//如果替换策略为WB，则定义dcache的wbb
	if (cache_p==Write_back){}

	//定义LSQ对象，也是arrayST对象，并获取面积信息，将其累加到LSQ的area和LoadstoreU的area中
	//计算lsq的高度，当计算bypass网络的时候需要用到
	//每个arraysT对象都会自动计算
	lsq_height=LSQ->local_result.cache_ht*sqrt(cdb_overhead);

	//如果为乱序核，并且给出了LoadQ的信息，则定义LoadQ，也是arrayST对象
	if ((coredynp.core_ty==OOO) && (XML->sys.core[ithCore].load_buffer_size >0))
	{
		//重新计算，需要包括两者，因为此时LSQ只作为StoreQ使用
		lsq_height=(LSQ->local_result.cache_ht + LoadQ->local_result.cache_ht)*sqrt(cdb_overhead);
	}
	//计算整体的面积(cdb_overhead固定为1.1，未找到解释)
	area.set_area(area.get_area()*cdb_overhead);
}

void LoadStoreU::computeEnergy(bool is_tdp)
{
	if (!exist) return;
	if (is_tdp)
	{
		//初始化Dcache的caches的参数
		//每周期的访问次数
		dcache.caches->stats_t.readAc.access  = 
			0.67*dcache.caches->l_ip.num_rw_ports*coredynp.LSU_duty_cycle;
		//每周期读取miss次数
		dcache.caches->stats_t.readAc.miss    = 0;
		//每周期读取hit次数
		dcache.caches->stats_t.readAc.hit     = 
			dcache.caches->stats_t.readAc.access - dcache.caches->stats_t.readAc.miss;
		//每周期写入的次数
		dcache.caches->stats_t.writeAc.access = 
			0.33*dcache.caches->l_ip.num_rw_ports*coredynp.LSU_duty_cycle;
		//每周期写入的miss次数
		dcache.caches->stats_t.writeAc.miss   = 0;
		//每周期写入的hit次数
		dcache.caches->stats_t.writeAc.hit    = 
			dcache.caches->stats_t.writeAc.access -	dcache.caches->stats_t.writeAc.miss;


		//初始化dcache的missb状态
		dcache.missb->stats_t.readAc.access  = 
			dcache.missb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;
		dcache.missb->stats_t.writeAc.access = 
			dcache.missb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;

		//初始化ifb的状态
		dcache.ifb->stats_t.readAc.access  = 
			dcache.ifb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;
		dcache.ifb->stats_t.writeAc.access = 
			dcache.ifb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;

		//初始化prefetchb的状态
		dcache.prefetchb->stats_t.readAc.access  = 
			dcache.prefetchb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;
		dcache.prefetchb->stats_t.writeAc.access = 
			dcache.ifb->l_ip.num_search_ports*coredynp.LSU_duty_cycle;

		//如果包含wbb，则初始化其状态
		if (cache_p==Write_back)
		{
			dcache.wbb->stats_t.readAc.access  = dcache.wbb->l_ip.num_search_ports;
			dcache.wbb->stats_t.writeAc.access = dcache.wbb->l_ip.num_search_ports;
		}

		//初始化LSQ的状态
		LSQ->stats_t.readAc.access = LSQ->stats_t.writeAc.access = 
			LSQ->l_ip.num_search_ports*coredynp.LSU_duty_cycle;

		if (如果LoadQ存在)
		{
			LoadQ->stats_t.readAc.access = LoadQ->stats_t.writeAc.access = 
				LoadQ->l_ip.num_search_ports*coredynp.LSU_duty_cycle;
		}
	}
	else
	{
		//init stats for Runtime Dynamic (RTP)
		dcache.caches->stats_t.readAc.access  = XML->sys.core[ithCore].dcache.read_accesses;
		dcache.caches->stats_t.readAc.miss    = XML->sys.core[ithCore].dcache.read_misses;
		dcache.caches->stats_t.readAc.hit     = dcache.caches->stats_t.readAc.access - dcache.caches->stats_t.readAc.miss;
		dcache.caches->stats_t.writeAc.access = XML->sys.core[ithCore].dcache.write_accesses;
		dcache.caches->stats_t.writeAc.miss   = XML->sys.core[ithCore].dcache.write_misses;
		dcache.caches->stats_t.writeAc.hit    = dcache.caches->stats_t.writeAc.access -	dcache.caches->stats_t.writeAc.miss;
		dcache.caches->rtp_stats = dcache.caches->stats_t;

		if (cache_p==Write_back)
		{
			dcache.missb->stats_t.readAc.access  = dcache.caches->stats_t.writeAc.miss;
			dcache.missb->stats_t.writeAc.access = dcache.caches->stats_t.writeAc.miss;
			dcache.missb->rtp_stats = dcache.missb->stats_t;

			dcache.ifb->stats_t.readAc.access  = dcache.caches->stats_t.writeAc.miss;
			dcache.ifb->stats_t.writeAc.access = dcache.caches->stats_t.writeAc.miss;
			dcache.ifb->rtp_stats = dcache.ifb->stats_t;

			dcache.prefetchb->stats_t.readAc.access  = dcache.caches->stats_t.writeAc.miss;
			dcache.prefetchb->stats_t.writeAc.access = dcache.caches->stats_t.writeAc.miss;
			dcache.prefetchb->rtp_stats = dcache.prefetchb->stats_t;

			dcache.wbb->stats_t.readAc.access  = dcache.caches->stats_t.writeAc.miss;
			dcache.wbb->stats_t.writeAc.access = dcache.caches->stats_t.writeAc.miss;
			dcache.wbb->rtp_stats = dcache.wbb->stats_t;
		}
		else
		{
			dcache.missb->stats_t.readAc.access  = dcache.caches->stats_t.readAc.miss;
			dcache.missb->stats_t.writeAc.access = dcache.caches->stats_t.readAc.miss;
			dcache.missb->rtp_stats = dcache.missb->stats_t;

			dcache.ifb->stats_t.readAc.access  = dcache.caches->stats_t.readAc.miss;
			dcache.ifb->stats_t.writeAc.access = dcache.caches->stats_t.readAc.miss;
			dcache.ifb->rtp_stats = dcache.ifb->stats_t;

			dcache.prefetchb->stats_t.readAc.access  = dcache.caches->stats_t.readAc.miss;
			dcache.prefetchb->stats_t.writeAc.access = dcache.caches->stats_t.readAc.miss;
			dcache.prefetchb->rtp_stats = dcache.prefetchb->stats_t;
		}

		LSQ->stats_t.readAc.access  = (XML->sys.core[ithCore].load_instructions + XML->sys.core[ithCore].store_instructions)*2;//flush overhead considered
		LSQ->stats_t.writeAc.access = (XML->sys.core[ithCore].load_instructions + XML->sys.core[ithCore].store_instructions)*2;
		LSQ->rtp_stats = LSQ->stats_t;

		if ((coredynp.core_ty==OOO) && (XML->sys.core[ithCore].load_buffer_size >0))
		{
			LoadQ->stats_t.readAc.access  = XML->sys.core[ithCore].load_instructions + XML->sys.core[ithCore].store_instructions;
			LoadQ->stats_t.writeAc.access = XML->sys.core[ithCore].load_instructions + XML->sys.core[ithCore].store_instructions;
			LoadQ->rtp_stats = LoadQ->stats_t;
		}

	}
	//初始化参数，设置为0
	dcache.power_t.reset();
	LSQ->power_t.reset();
	
	//根据操作次数和每个操作的能耗，计算caches的energy_per_cycle/total_energy
	//并将结果累加到dcache的临时变量中
    dcache.power_t.readOp.dynamic	+= (
			dcache.caches->stats_t.readAc.hit*dcache.caches->local_result.power.readOp.dynamic+
    		dcache.caches->stats_t.readAc.miss*dcache.caches->local_result.power.readOp.dynamic+ 
    		dcache.caches->stats_t.writeAc.miss*dcache.caches->local_result.tag_array2->power.readOp.dynamic+
    		dcache.caches->stats_t.writeAc.access*dcache.caches->local_result.power.writeOp.dynamic);
	//如果是写回策略，则写入miss也会带来功耗
    if (cache_p==Write_back)
    	dcache.power_t.readOp.dynamic	+= dcache.caches->stats_t.writeAc.miss*dcache.caches->local_result.power.writeOp.dynamic;

	//将missb，ifb，prefetchb的能耗计算出来，累加到dcache中
    dcache.power_t.readOp.dynamic	+=  dcache.missb->stats_t.readAc.access*dcache.missb->local_result.power.searchOp.dynamic +
            dcache.missb->stats_t.writeAc.access*dcache.missb->local_result.power.writeOp.dynamic;//each access to missb involves a CAM and a write
    dcache.power_t.readOp.dynamic	+=  dcache.ifb->stats_t.readAc.access*dcache.ifb->local_result.power.searchOp.dynamic +
            dcache.ifb->stats_t.writeAc.access*dcache.ifb->local_result.power.writeOp.dynamic;
    dcache.power_t.readOp.dynamic	+=  dcache.prefetchb->stats_t.readAc.access*dcache.prefetchb->local_result.power.searchOp.dynamic +
            dcache.prefetchb->stats_t.writeAc.access*dcache.prefetchb->local_result.power.writeOp.dynamic;
    //如果是写回策略，则需要计算wbb的能耗
	if (cache_p==Write_back)
    {
    	dcache.power_t.readOp.dynamic	+=  dcache.wbb->stats_t.readAc.access*dcache.wbb->local_result.power.searchOp.dynamic
			+ dcache.wbb->stats_t.writeAc.access*dcache.wbb->local_result.power.writeOp.dynamic;
    }

	//如果存在loadQ，则需要计算其功耗
	//LSQ都需要计算
    if ((coredynp.core_ty==OOO) && (XML->sys.core[ithCore].load_buffer_size >0))
    {
    	LoadQ->power_t.reset();
    	LoadQ->power_t.readOp.dynamic  +=  LoadQ->stats_t.readAc.access*(LoadQ->local_result.power.searchOp.dynamic+ LoadQ->local_result.power.readOp.dynamic)+
    	        LoadQ->stats_t.writeAc.access*LoadQ->local_result.power.writeOp.dynamic;//every memory access invloves at least two operations on LoadQ

    	LSQ->power_t.readOp.dynamic  +=  LSQ->stats_t.readAc.access*(LSQ->local_result.power.searchOp.dynamic + LSQ->local_result.power.readOp.dynamic)
    		        + LSQ->stats_t.writeAc.access*LSQ->local_result.power.writeOp.dynamic;//every memory access invloves at least two operations on LSQ

    }
    else
    {
    	LSQ->power_t.readOp.dynamic  +=  LSQ->stats_t.readAc.access*(LSQ->local_result.power.searchOp.dynamic + LSQ->local_result.power.readOp.dynamic)
    		        + LSQ->stats_t.writeAc.access*LSQ->local_result.power.writeOp.dynamic;//every memory access invloves at least two operations on LSQ

    }

	//1. 计算所有类型的功耗
	//2. 累加所有部分的功耗信息
    if (is_tdp)
    {
		//const double pppm_lkg[4]  = {0,1,1,0}
		//累加power中的第二个参数和第三个参数，leakage和gate_leakage
    	dcache.power = dcache.power_t + (dcache.caches->local_result.power +
    			dcache.missb->local_result.power +
    			dcache.ifb->local_result.power +
    			dcache.prefetchb->local_result.power) *pppm_lkg;
    	if (cache_p==Write_back)
    	{
    		dcache.power = dcache.power + dcache.wbb->local_result.power*pppm_lkg;
    	}

    	LSQ->power = LSQ->power_t + LSQ->local_result.power *pppm_lkg;
    	power     = power + dcache.power + LSQ->power;

    	if ((coredynp.core_ty==OOO) && (XML->sys.core[ithCore].load_buffer_size >0))
    	{
    		LoadQ->power = LoadQ->power_t + LoadQ->local_result.power *pppm_lkg;
    		power     = power + LoadQ->power;
    	}
    }
    else//和之前的计算过程一样，但是结果保存的地方不同
    {
    	dcache.rt_power = dcache.power_t + (dcache.caches->local_result.power +
    			dcache.missb->local_result.power +
    			dcache.ifb->local_result.power +
    			dcache.prefetchb->local_result.power )*pppm_lkg;

    	if (cache_p==Write_back)
    	{
    		dcache.rt_power = dcache.rt_power + dcache.wbb->local_result.power*pppm_lkg;
    	}

    	LSQ->rt_power = LSQ->power_t + LSQ->local_result.power *pppm_lkg;
    	rt_power     = rt_power + dcache.rt_power + LSQ->rt_power;

    	if ((coredynp.core_ty==OOO) && (XML->sys.core[ithCore].load_buffer_size >0))
    	{
    		LoadQ->rt_power = LoadQ->power_t + LoadQ->local_result.power *pppm_lkg;
    		rt_power     = rt_power + LoadQ->rt_power;
    	}
    }
}


void LoadStoreU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;
	
	if (is_tdp)
	{
		/*
			Data Cache:
			
			Area = dcache.area.get_area()*1e-6
			Peak Dynamic = dcache.power.readOp.dynamic*clockRate
			Subthreshold Leakage = (long_channel?
					dcache.power.readOp.longer_channel_leakage:dcache.power.readOp.leakage )
			Subthreshold Leakage with power gating = (long_channel? 
					dcache.power.readOp.power_gated_with_long_channel_leakage : 
					dcache.power.readOp.power_gated_leakage) 
			Runtime Dynamic = dcache.rt_power.readOp.dynamic/executionTime 
		*/
		
		if (coredynp.core_ty==Inorder)
		{
			//Load/Store Queue: LSQ
		}
		else
		{
			if (XML->sys.core[ithCore].load_buffer_size >0)
			{
				//LoadQ:
			}
			//StoreQ: LSQ
		}
	}
	else
	{
		//Peak Dynamic = dcache.rt_power.readOp.dynamic*clockRate 
		//Subthreshold Leakage = dcache.rt_power.readOp.leakage
		//Gate Leakage = dcache.rt_power.readOp.gate_leakage
		if (coredynp.core_ty==Inorder)
		{
			//LSQ
		}
		else
		{
			//LSQ
			//LoadQ
		}
	}
}

