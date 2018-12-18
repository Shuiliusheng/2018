RENAMINGU::RENAMINGU(ParseXML* XML_interface, int ithCore_, 
	InputParameter* interface_ip_, 
	const CoreDynParam & dyn_p_,bool exist_)
:XML(XML_interface),ithCore(ithCore_),interface_ip(*interface_ip_),
 coredynp(dyn_p_),iFRAT(0),fFRAT(0),
 iRRAT(0),fRRAT(0),
 ifreeL(0),ffreeL(0),
 idcl(0),fdcl(0),
 RAHT(0),exist(exist_)
 {
	/*
	 * Although renaming logic maybe be used in in-order processors,
     * McPAT assumes no renaming logic is used since the performance gain is very limited and
     * the only major inorder processor with renaming logic is Itainium
     * that is a VLIW processor and different from current McPAT's model.
	 * physical register base OOO must have Dual-RAT architecture or equivalent structure.FRAT:FrontRAT, RRAT:RetireRAT;
	 * i,f prefix mean int and fp
	 * RAT for all Renaming logic, random accessible checkpointing is used, but only update when instruction retires.
	 * FRAT will be read twice and written once per instruction;
	 * RRAT will be write once per instruction when committing and reads out all when context switch
	 * checkpointing is implicit
	 * Renaming logic is duplicated for each different hardware threads
	 *
	 * No Dual-RAT is needed in RS-based OOO processors,
	 * however, RAT needs to do associative search in RAT, when instruction commits and ROB release the entry,
	 * to make sure all the renamings associated with the ROB to be released are updated at the same time.
	 * RAM scheme has # ARchi Reg entry with each entry hold phy reg tag,
	 * CAM scheme has # Phy Reg entry with each entry hold ARchi reg tag,
	 *
	 * Both RAM and CAM have same DCL
	 */
	if (!exist) return;
	
	//乱序处理器的情况，继续细分为PRF/RS，然后在细分重命名表的结构设计 RAM/CAM
    if (coredynp.core_ty==OOO)
    {
		//integer pipeline
		if (coredynp.scheu_ty==PhysicalRegFile)
		{
			if (coredynp.rm_ty ==RAMbased)//rename_scheme
			{	  
				iFRAT = new ArrayST(&interface_ip, "Int FrontRAT");
				//由于多线程的情况是直接复制多份，因此需要乘以倍数
				//area=iFRAT->local_result.area*XML->sys.core[ithCore].number_hardware_threads
				fFRAT = new ArrayST(&interface_ip, "FP FrontRAT");
			}
			else if ((coredynp.rm_ty ==CAMbased))
			{
				//iFRAT，fFRAT
			}

			//iRRAT，fRRAT，ifreeL，ffreeL
			iRRAT = new ArrayST(&interface_ip, "Int RetireRAT");
			//area=iRRAT->local_result.area*XML->sys.core[ithCore].number_hardware_threads)
			
			//资源检查逻辑，没有考虑面积问题
			idcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_ireg_width);
			fdcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_freg_width);

		}
		else if (coredynp.scheu_ty==ReservationStation)
		{
			if (coredynp.rm_ty ==RAMbased){

				iFRAT = new ArrayST(&interface_ip, "Int FrontRAT");
				//mcpat的解释是弥补GC带来的影响
				iFRAT->local_result.adjust_area();
				iFRAT->local_result.power.readOp.dynamic *=1.5;
				iFRAT->local_result.power.writeOp.dynamic *=1.5;
				//area=iFRAT->local_result.area*XML->sys.core[ithCore].number_hardware_threads


				fFRAT = new ArrayST(&interface_ip, "FP FrontRAT");
				fFRAT->local_result.adjust_area();
				fFRAT->local_result.power.readOp.dynamic *=1.5;//compensate for GC
				fFRAT->local_result.power.writeOp.dynamic *=1.5;
				//area=fFRAT->local_result.area*XML->sys.core[ithCore].number_hardware_threads

			}
			else if ((coredynp.rm_ty ==CAMbased))
			{
				//iFRAT,fFRAT
			}
			//此时只有一个整体的free list
			ifreeL = new ArrayST(&interface_ip, "Unified Free List");
			//area

			idcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_ireg_width);
			fdcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_freg_width);
		}

	}
    if (coredynp.core_ty==Inorder&& coredynp.issueW>1)
    {
		//按序多发射的情况下，也需要检查发射指令之间的资源冲突问题
		idcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_ireg_width);
		fdcl  = new dep_resource_conflict_check(&interface_ip,coredynp,coredynp.phy_freg_width);
    }
}


void RENAMINGU::computeEnergy(bool is_tdp)
{
	if (!exist) return;
	double pppm_t[4]    = {1,1,1,1};
	if (is_tdp)
	{
		if (coredynp.core_ty==OOO)
		{
			if (coredynp.scheu_ty==PhysicalRegFile)
			{
				if (coredynp.rm_ty ==RAMbased)
				{
					iFRAT->stats_t.readAc.access   = iFRAT->l_ip.num_rd_ports;
					iFRAT->stats_t.writeAc.access  = iFRAT->l_ip.num_wr_ports;

					fFRAT->stats_t.readAc.access   = fFRAT->l_ip.num_rd_ports;
					fFRAT->stats_t.writeAc.access  = fFRAT->l_ip.num_wr_ports;

				}
				else if ((coredynp.rm_ty ==CAMbased))
				{
					iFRAT->stats_t.readAc.access   = iFRAT->l_ip.num_search_ports;
					iFRAT->stats_t.writeAc.access  = iFRAT->l_ip.num_wr_ports;

					fFRAT->stats_t.readAc.access   = fFRAT->l_ip.num_search_ports;
					fFRAT->stats_t.writeAc.access  = fFRAT->l_ip.num_wr_ports;
				}

				iRRAT->stats_t.readAc.access   = iRRAT->l_ip.num_rd_ports;
				iRRAT->stats_t.writeAc.access  = iRRAT->l_ip.num_wr_ports;
				
				fRRAT->stats_t.readAc.access   = fRRAT->l_ip.num_rd_ports;
				fRRAT->stats_t.writeAc.access  = fRRAT->l_ip.num_wr_ports;

				ifreeL->stats_t.readAc.access   = coredynp.decodeW;
				ifreeL->stats_t.writeAc.access  = coredynp.decodeW;

				ffreeL->stats_t.readAc.access   = coredynp.decodeW;
				ffreeL->stats_t.writeAc.access  = coredynp.decodeW;
			}
			else if (coredynp.scheu_ty==ReservationStation)
			{
				if (coredynp.rm_ty ==RAMbased)
				{
					iFRAT->stats_t.readAc.access    = iFRAT->l_ip.num_rd_ports;
					iFRAT->stats_t.writeAc.access   = iFRAT->l_ip.num_wr_ports;
					iFRAT->stats_t.searchAc.access  = iFRAT->l_ip.num_search_ports;
					iFRAT->tdp_stats = iFRAT->stats_t;

					fFRAT->stats_t.readAc.access    = fFRAT->l_ip.num_rd_ports;
					fFRAT->stats_t.writeAc.access   = fFRAT->l_ip.num_wr_ports;
					fFRAT->stats_t.searchAc.access  = fFRAT->l_ip.num_search_ports;
					fFRAT->tdp_stats = fFRAT->stats_t;

				}
				else if ((coredynp.rm_ty ==CAMbased))
				{
					iFRAT->stats_t.readAc.access   = iFRAT->l_ip.num_search_ports;
					iFRAT->stats_t.writeAc.access  = iFRAT->l_ip.num_wr_ports;

					fFRAT->stats_t.readAc.access   = fFRAT->l_ip.num_search_ports;
					fFRAT->stats_t.writeAc.access  = fFRAT->l_ip.num_wr_ports;
				}
				//Unified free list for both int and fp
				ifreeL->stats_t.readAc.access   = coredynp.decodeW;
				ifreeL->stats_t.writeAc.access  = coredynp.decodeW;
			}
			idcl->stats_t.readAc.access = coredynp.decodeW;
			fdcl->stats_t.readAc.access = coredynp.decodeW;
		}
		else
		{
			if (coredynp.issueW>1)
			{
				idcl->stats_t.readAc.access = coredynp.decodeW;
				fdcl->stats_t.readAc.access = coredynp.decodeW;
			}
		}

	}
	else
	{
		if (coredynp.core_ty==OOO){
			if (coredynp.scheu_ty==PhysicalRegFile)
			{
				if (coredynp.rm_ty ==RAMbased)
				{
					iFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].rename_reads;
					iFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].rename_writes;
					iFRAT->rtp_stats = iFRAT->stats_t;

					fFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_reads;
					fFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].fp_rename_writes;
					fFRAT->rtp_stats = fFRAT->stats_t;
				}
				else if ((coredynp.rm_ty ==CAMbased))
				{
					iFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].rename_reads;
					iFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].rename_writes;
					iFRAT->rtp_stats = iFRAT->stats_t;

					fFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_reads;
					fFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].fp_rename_writes;
					fFRAT->rtp_stats = fFRAT->stats_t;
				}

				iRRAT->stats_t.readAc.access   = XML->sys.core[ithCore].rename_writes;
				iRRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].rename_writes;

				fRRAT->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_writes;
				fRRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].fp_rename_writes;

				ifreeL->stats_t.readAc.access   = XML->sys.core[ithCore].rename_reads;
				ifreeL->stats_t.writeAc.access  = 2*XML->sys.core[ithCore].rename_writes;

				ffreeL->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_reads;
				ffreeL->stats_t.writeAc.access  = 2*XML->sys.core[ithCore].fp_rename_writes;
			}
			else if (coredynp.scheu_ty==ReservationStation)
			{
				if (coredynp.rm_ty ==RAMbased)
				{
					iFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].rename_reads;
					iFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].rename_writes;
					iFRAT->stats_t.searchAc.access  = XML->sys.core[ithCore].committed_int_instructions;

					fFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_reads;
					fFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].fp_rename_writes;
					fFRAT->stats_t.searchAc.access  = XML->sys.core[ithCore].committed_fp_instructions;
				}
				else if ((coredynp.rm_ty ==CAMbased))
				{
					iFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].rename_reads;
					iFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].rename_writes;
					
					fFRAT->stats_t.readAc.access   = XML->sys.core[ithCore].fp_rename_reads;
					fFRAT->stats_t.writeAc.access  = XML->sys.core[ithCore].fp_rename_writes;
				}
				//Unified free list for both int and fp since the ROB act as physcial registers
				ifreeL->stats_t.readAc.access   = 
					XML->sys.core[ithCore].rename_reads + XML->sys.core[ithCore].fp_rename_reads;
				ifreeL->stats_t.writeAc.access  = 
					2*(XML->sys.core[ithCore].rename_writes + XML->sys.core[ithCore].fp_rename_writes);
					//HACK: 2-> since some of renaming in the same group are terminated early
			}
			idcl->stats_t.readAc.access = 3*coredynp.decodeW*coredynp.decodeW*XML->sys.core[ithCore].rename_reads;
			fdcl->stats_t.readAc.access = 3*coredynp.fp_issueW*coredynp.fp_issueW*XML->sys.core[ithCore].fp_rename_writes;
		}
		else
		{
			if (coredynp.issueW>1)
			{
				idcl->stats_t.readAc.access = 2*XML->sys.core[ithCore].int_instructions;
				fdcl->stats_t.readAc.access = XML->sys.core[ithCore].fp_instructions;
			}
		}

	}
    /* Compute engine */
	if (coredynp.core_ty==OOO)
	{
		if (coredynp.scheu_ty==PhysicalRegFile)
		{
			if (coredynp.rm_ty ==RAMbased)
			{
				iFRAT->power_t.reset();
				fFRAT->power_t.reset();

				iFRAT->power_t.readOp.dynamic  +=  (iFRAT->stats_t.readAc.access
						*(iFRAT->local_result.power.readOp.dynamic + idcl->power.readOp.dynamic)
						+iFRAT->stats_t.writeAc.access*iFRAT->local_result.power.writeOp.dynamic);
				fFRAT->power_t.readOp.dynamic  +=  (fFRAT->stats_t.readAc.access
						*(fFRAT->local_result.power.readOp.dynamic + fdcl->power.readOp.dynamic)
						+fFRAT->stats_t.writeAc.access*fFRAT->local_result.power.writeOp.dynamic);
			}
			else if ((coredynp.rm_ty ==CAMbased))
			{
				iFRAT->power_t.reset();
				fFRAT->power_t.reset();
				iFRAT->power_t.readOp.dynamic  +=  (iFRAT->stats_t.readAc.access
						*(iFRAT->local_result.power.searchOp.dynamic + idcl->power.readOp.dynamic)
						+iFRAT->stats_t.writeAc.access*iFRAT->local_result.power.writeOp.dynamic);
				fFRAT->power_t.readOp.dynamic  +=  (fFRAT->stats_t.readAc.access
						*(fFRAT->local_result.power.searchOp.dynamic + fdcl->power.readOp.dynamic)
						+fFRAT->stats_t.writeAc.access*fFRAT->local_result.power.writeOp.dynamic);
			}

			iRRAT->power_t.reset();
			fRRAT->power_t.reset();
			ifreeL->power_t.reset();
			ffreeL->power_t.reset();

			iRRAT->power_t.readOp.dynamic  +=  (iRRAT->stats_t.readAc.access*iRRAT->local_result.power.readOp.dynamic
					+iRRAT->stats_t.writeAc.access*iRRAT->local_result.power.writeOp.dynamic);
			fRRAT->power_t.readOp.dynamic  +=  (fRRAT->stats_t.readAc.access*fRRAT->local_result.power.readOp.dynamic
					+fRRAT->stats_t.writeAc.access*fRRAT->local_result.power.writeOp.dynamic);
			ifreeL->power_t.readOp.dynamic  +=  (ifreeL->stats_t.readAc.access*ifreeL->local_result.power.readOp.dynamic
					+ifreeL->stats_t.writeAc.access*ifreeL->local_result.power.writeOp.dynamic);
			ffreeL->power_t.readOp.dynamic  +=  (ffreeL->stats_t.readAc.access*ffreeL->local_result.power.readOp.dynamic
					+ffreeL->stats_t.writeAc.access*ffreeL->local_result.power.writeOp.dynamic);

		}
		else if (coredynp.scheu_ty==ReservationStation)
		{
			if (coredynp.rm_ty ==RAMbased)
			{
				iFRAT->power_t.reset();
				fFRAT->power_t.reset();

				iFRAT->power_t.readOp.dynamic  +=  (iFRAT->stats_t.readAc.access
						*(iFRAT->local_result.power.readOp.dynamic + idcl->power.readOp.dynamic)
						+iFRAT->stats_t.writeAc.access*iFRAT->local_result.power.writeOp.dynamic
						+iFRAT->stats_t.searchAc.access*iFRAT->local_result.power.searchOp.dynamic);
				fFRAT->power_t.readOp.dynamic  +=  (fFRAT->stats_t.readAc.access
						*(fFRAT->local_result.power.readOp.dynamic + fdcl->power.readOp.dynamic)
						+fFRAT->stats_t.writeAc.access*fFRAT->local_result.power.writeOp.dynamic
						+fFRAT->stats_t.searchAc.access*fFRAT->local_result.power.searchOp.dynamic);
			}
			else if ((coredynp.rm_ty ==CAMbased))
			{
				iFRAT->power_t.reset();
				fFRAT->power_t.reset();
				iFRAT->power_t.readOp.dynamic  +=  (iFRAT->stats_t.readAc.access
						*(iFRAT->local_result.power.searchOp.dynamic + idcl->power.readOp.dynamic)
						+iFRAT->stats_t.writeAc.access*iFRAT->local_result.power.writeOp.dynamic);
				fFRAT->power_t.readOp.dynamic  +=  (fFRAT->stats_t.readAc.access
						*(fFRAT->local_result.power.searchOp.dynamic + fdcl->power.readOp.dynamic)
						+fFRAT->stats_t.writeAc.access*fFRAT->local_result.power.writeOp.dynamic);
			}
			ifreeL->power_t.reset();
			ifreeL->power_t.readOp.dynamic  +=  (ifreeL->stats_t.readAc.access*ifreeL->local_result.power.readOp.dynamic
					+ifreeL->stats_t.writeAc.access*ifreeL->local_result.power.writeOp.dynamic);
		}

	}
	else
	{
		if (coredynp.issueW>1)
		{
			idcl->power_t.reset();
			fdcl->power_t.reset();
			set_pppm(pppm_t, idcl->stats_t.readAc.access, coredynp.num_hthreads, coredynp.num_hthreads, idcl->stats_t.readAc.access);
			idcl->power_t = idcl->power * pppm_t;
			set_pppm(pppm_t, fdcl->stats_t.readAc.access, coredynp.num_hthreads, coredynp.num_hthreads, idcl->stats_t.readAc.access);
			fdcl->power_t = fdcl->power * pppm_t;
		}

	}

	//assign value to tpd and rtp
	if (is_tdp)
	{
		if (coredynp.core_ty==OOO)
		{
			if (coredynp.scheu_ty==PhysicalRegFile)
			{
				//pppm_lkg_multhread={0, coredynp.num_hthreads, coredynp.num_hthreads, 0}
				iFRAT->power   =  iFRAT->power_t + (iFRAT->local_result.power ) * coredynp.pppm_lkg_multhread + idcl->power_t;
				fFRAT->power   =  fFRAT->power_t + (fFRAT->local_result.power ) * coredynp.pppm_lkg_multhread + fdcl->power_t;
				iRRAT->power   =  iRRAT->power_t + iRRAT->local_result.power * coredynp.pppm_lkg_multhread;
				fRRAT->power   =  fRRAT->power_t + fRRAT->local_result.power * coredynp.pppm_lkg_multhread;
				ifreeL->power  =  ifreeL->power_t + ifreeL->local_result.power * coredynp.pppm_lkg_multhread;
				ffreeL->power  =  ffreeL->power_t + ffreeL->local_result.power * coredynp.pppm_lkg_multhread;
				power	       =  power + (iFRAT->power + fFRAT->power)
				                 + (iRRAT->power + fRRAT->power)
				                 + (ifreeL->power + ffreeL->power);
			}
			else if (coredynp.scheu_ty==ReservationStation)
			{
				iFRAT->power   =  iFRAT->power_t + (iFRAT->local_result.power ) * coredynp.pppm_lkg_multhread + idcl->power_t;
				fFRAT->power   =  fFRAT->power_t + (fFRAT->local_result.power ) * coredynp.pppm_lkg_multhread + fdcl->power_t;
				ifreeL->power  =  ifreeL->power_t + ifreeL->local_result.power * coredynp.pppm_lkg_multhread;
				power	       =  power + (iFRAT->power + fFRAT->power)
				                 + ifreeL->power;
			}
		}
		else
		{
			power   =  power + idcl->power_t + fdcl->power_t;
		}

	}
	else
	{
		if (coredynp.core_ty==OOO)
		{
			if (coredynp.scheu_ty==PhysicalRegFile)
			{
				iFRAT->rt_power = iFRAT->power_t + (iFRAT->local_result.power ) * coredynp.pppm_lkg_multhread 
				                  + idcl->power_t;
				fFRAT->rt_power = fFRAT->power_t + (fFRAT->local_result.power ) * coredynp.pppm_lkg_multhread 
								  + fdcl->power_t;
				iRRAT->rt_power = iRRAT->power_t + iRRAT->local_result.power * coredynp.pppm_lkg_multhread;
				fRRAT->rt_power = fRRAT->power_t + fRRAT->local_result.power * coredynp.pppm_lkg_multhread;
				ifreeL->rt_power= ifreeL->power_t + ifreeL->local_result.power * coredynp.pppm_lkg_multhread;
				ffreeL->rt_power= ffreeL->power_t + ffreeL->local_result.power * coredynp.pppm_lkg_multhread;
				rt_power	    = rt_power + (iFRAT->rt_power + fFRAT->rt_power)
				                   + (iRRAT->rt_power + fRRAT->rt_power) 
								   + (ifreeL->rt_power + ffreeL->rt_power);
			}
			else if (coredynp.scheu_ty==ReservationStation)
			{
				iFRAT->rt_power = iFRAT->power_t + (iFRAT->local_result.power ) * coredynp.pppm_lkg_multhread
								  + idcl->power_t;
				fFRAT->rt_power = fFRAT->power_t + (fFRAT->local_result.power ) * coredynp.pppm_lkg_multhread
								  + fdcl->power_t;
				ifreeL->rt_power= ifreeL->power_t + ifreeL->local_result.power * coredynp.pppm_lkg_multhread;
				rt_power	    = rt_power + (iFRAT->rt_power + fFRAT->rt_power) + ifreeL->rt_power;
			}
		}
		else
		{
			rt_power   =  rt_power + idcl->power_t + fdcl->power_t;
		}

	}
}

void RENAMINGU::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;

	if (is_tdp)
	{
		if (coredynp.core_ty==OOO)
		{
			/*
				Int Front End RAT:iFRAT
				FP Front End RAT:fFRAT
				Free List:ifreeL
				
				Area = iFRAT->area.get_area()*1e-6
				Peak Dynamic = iFRAT->power.readOp.dynamic*clockRate
				Subthreshold Leakage = (long_channel? 
						iFRAT->power.readOp.longer_channel_leakage:iFRAT->power.readOp.leakage)
				Subthreshold Leakage with power gating = (long_channel? 
						iFRAT->power.readOp.power_gated_with_long_channel_leakage : 
						iFRAT->power.readOp.power_gated_leakage) 
				Runtime Dynamic = iFRAT->rt_power.readOp.dynamic/executionTime
			*/

			if (coredynp.scheu_ty==PhysicalRegFile)
			{
				/*
					Int Retire RAT:iRRAT
					FP Retire RAT:fRRAT
					FP Free List:ffreeL
				*/
			}
		}
		else
		{
			/*
				Int DCL:idcl
				FP DCL:fdcl
			*/
		}
	}
	else
	{
	}

}


