//输入参数：
//XML_interface:用于从xml文件中获取解析的数据
//ithCore_：当前核的标号
//interface_ip_:该参数是用于传递给cacti
//dyn_p：core的一些动态参数信息
//exist_:用于确定是否需要生成该对象
BranchPredictor::BranchPredictor(ParseXML* XML_interface, int ithCore_, 
								InputParameter* interface_ip_, 
								const CoreDynParam & dyn_p_, bool exist_)
:XML(XML_interface),ithCore(ithCore_),interface_ip(*interface_ip_),
 coredynp(dyn_p_),globalBPT(0),localBPT(0),L1_localBPT(0),
 L2_localBPT(0),chooser(0),RAS(0),exist(exist_)
{
	//如果不存在，则不进一步创建该对象
	if (!exist) return;
	
	//
	int  tag, data;
	
	//获取core的时钟频率和执行时间
	clockRate = coredynp.clockRate;
	executionTime = coredynp.executionTime;
	
	//设置相连度和类型，不是cam，同时相连度为1，直接映射
	interface_ip.assoc               = 1;
	interface_ip.pure_cam            = false;
	
	//是否为多线程
	if (coredynp.multithreaded)
	{//如果是多线程
		//需要为每一个表项增加线程编号，因此会增加tag位域
		//EXTRA_TAG_BITS为cacti中的参数，const int = 5
		tag							     = int(log2(coredynp.num_hthreads)+ EXTRA_TAG_BITS);
		//bool类型,是否有tag域
		interface_ip.specific_tag        = 1;
		//tag的宽度
		interface_ip.tag_w               = tag;
		
		//bool类型，是否是一个cache
		interface_ip.is_cache			 = true;
		//是否是cam类型的array
		interface_ip.pure_ram            = false;
	}
	else//如果不是多线程
	{
		//不是cache，不再是使用tag索引
		interface_ip.is_cache			 = false;
		//是一个RAM
		interface_ip.pure_ram            = true;

	}
	
	//Global predictor
	//data：全局的预测器，每个表项的宽度，向上舍入，不到8位，补到8位。结果单位是字节
	data							 = int(ceil(XML->sys.core[ithCore].predictor.global_predictor_bits/8.0));
	//每个line的大小，单位为字节
	interface_ip.line_sz             = data;
	//整个cache的大小，表项数*每个表项的大小，单位字节
	interface_ip.cache_sz            = data*XML->sys.core[ithCore].predictor.global_predictor_entries;
	//bank的个数，为1
	interface_ip.nbanks              = 1;
	//输出宽度：以bit为单位，每个表项的宽度
	interface_ip.out_w               = interface_ip.line_sz*8;
	//访问模式，0 normal, 1 seq, 2 fast
	interface_ip.access_mode         = 2;
	
	//吞吐率每周期
	interface_ip.throughput          = 1.0/clockRate;
	//延迟，一个周期
	interface_ip.latency             = 1.0/clockRate;
	
	//未在cacti中使用
	interface_ip.obj_func_dyn_energy = 0;
	interface_ip.obj_func_dyn_power  = 0;
	interface_ip.obj_func_leak_power = 0;
	interface_ip.obj_func_cycle_t    = 1;
	
	//读写端口的数目
	interface_ip.num_rw_ports    = 0;
	//单独的读端口
	//prediction_width，core的参数，一般设置为1
	interface_ip.num_rd_ports    = coredynp.predictionW;
	//单独的写端口
	interface_ip.num_wr_ports    = coredynp.predictionW;
	//单端读端口
	interface_ip.num_se_rd_ports = 0;
	
	//Core_device: enum Device_ty的第一个，Core_device uncore_device，LLC_device
	//可能代表着是核内设备
	//coredynp.opt_local，一般设置为了0
	//coredynp.core_ty，machine_type：ino/order
	//在构造函数中，已经调用了计算功率的函数
	globalBPT = new ArrayST(&interface_ip, "Global Predictor", Core_device, coredynp.opt_local, coredynp.core_ty);
	//计算面积
	globalBPT->area.set_area(globalBPT->area.get_area()+ globalBPT->local_result.area);
	//统计面积参数
	area.set_area(area.get_area()+ globalBPT->local_result.area);

	//Local BPT (Level 1)
	L1_localBPT = new ArrayST(&interface_ip, "L1 local Predictor", Core_device, coredynp.opt_local, coredynp.core_ty);

	//Local BPT (Level 2)
	L2_localBPT = new ArrayST(&interface_ip, "L2 local Predictor", Core_device, coredynp.opt_local, coredynp.core_ty);
	L2_localBPT->area.set_area(L2_localBPT->area.get_area()+ L2_localBPT->local_result.area);
	area.set_area(area.get_area()+ L2_localBPT->local_result.area);

	//Chooser
	chooser = new ArrayST(&interface_ip, "Predictor Chooser", Core_device, coredynp.opt_local, coredynp.core_ty);
	
	//RAS return address stacks are Duplicated for each thread.
	//不是cache，不需要tag索引，不管是不是多线程
	interface_ip.is_cache			 = false;
	//array的类型是暂存器，类似于寄存器
	interface_ip.pure_ram            = true;
	RAS = new ArrayST(&interface_ip, "RAS", Core_device, coredynp.opt_local, coredynp.core_ty);
}


void BranchPredictor::computeEnergy(bool is_tdp)
{
	if (!exist) return;
	double r_access;//记录读的次数
	double w_access;//记录写的次数
	//计算power，热设计功耗
	//TDP的含义是“当处理器达到最大负荷的时候，所释放出的热量”
	if (is_tdp)
    {//计算静态功耗，这个时候需要考虑的时BP在处理器通电之后，被使用的时间
		//为很么没有w-access，默认是逻辑电路，类似于读操作，不会有写
		//BR_duty_cycle，初始化为1，有些xml中未设置，core的stats
    	r_access = coredynp.predictionW*coredynp.BR_duty_cycle;
		//设置为0
    	w_access = 0*coredynp.BR_duty_cycle;
		//arrayST中包括statsDef的三个对象：tdp_stats,rtp_stats, stats_t
		//stats_t: 临时变量
		//statsDef包括了三个statsComponents对象：readAc, writeAc, searchAc
		//statsCompents包括三个属性：access, hit, miss
    	globalBPT->stats_t.readAc.access  = r_access;
    	globalBPT->stats_t.writeAc.access = w_access;
    	globalBPT->tdp_stats = globalBPT->stats_t;

    	L1_localBPT->stats_t.readAc.access  = r_access;
    	L1_localBPT->stats_t.writeAc.access = w_access;
    	L1_localBPT->tdp_stats = L1_localBPT->stats_t;

    	L2_localBPT->stats_t.readAc.access  = r_access;
    	L2_localBPT->stats_t.writeAc.access = w_access;
    	L2_localBPT->tdp_stats = L2_localBPT->stats_t;

    	chooser->stats_t.readAc.access  = r_access;
    	chooser->stats_t.writeAc.access = w_access;
    	chooser->tdp_stats = chooser->stats_t;

    	RAS->stats_t.readAc.access  = r_access;
    	RAS->stats_t.writeAc.access = w_access;
    	RAS->tdp_stats = RAS->stats_t;
    }
    else//计算rt_power，动态功耗
    {
		//访问次数的粒度很粗，因为模拟器的问题
		//所有的分支指令都会读一遍预测器
    	r_access = XML->sys.core[ithCore].branch_instructions;
    	//当分支预测错误或者状态迁移时(10%的分支指令)的时候需要写一次预测器
		w_access = XML->sys.core[ithCore].branch_mispredictions + 0.1*XML->sys.core[ithCore].branch_instructions;
		//设置rtp_stats
    	globalBPT->stats_t.readAc.access  = r_access;
    	globalBPT->stats_t.writeAc.access = w_access;
    	globalBPT->rtp_stats = globalBPT->stats_t;

    	L1_localBPT->stats_t.readAc.access  = r_access;
    	L1_localBPT->stats_t.writeAc.access = w_access;
    	L1_localBPT->rtp_stats = L1_localBPT->stats_t;

    	L2_localBPT->stats_t.readAc.access  = r_access;
    	L2_localBPT->stats_t.writeAc.access = w_access;
    	L2_localBPT->rtp_stats = L2_localBPT->stats_t;

    	chooser->stats_t.readAc.access  = r_access;
    	chooser->stats_t.writeAc.access = w_access;
    	chooser->rtp_stats = chooser->stats_t;

		//RAS的读和写的次数为函数调用的次数
    	RAS->stats_t.readAc.access  = XML->sys.core[ithCore].function_calls;
    	RAS->stats_t.writeAc.access = XML->sys.core[ithCore].function_calls;
    	RAS->rtp_stats = RAS->stats_t;
   }

    //初始化，设置为0
	//power_t：powerDef  (cacti/cacti_interface.h:87)
	//powerDef有三个powerComponents类的对象：readOp，writeOp, searchOp（CAM才有）
	//powerComponents有dynamic，leakage，gate_leakage，short_circuit，longer_channel_leakage
	//，power_gated_leakage，power_gated_with_long_channel_leakage七个属性
	globalBPT->power_t.reset();
	L1_localBPT->power_t.reset();
	L2_localBPT->power_t.reset();
	chooser->power_t.reset();
	RAS->power_t.reset();

	//读操作的动态功耗=local_result.power.readOp.dynamic*读操作的频率+local_result.power.writeOp.dynamic*写频率
    globalBPT->power_t.readOp.dynamic   +=  globalBPT->local_result.power.readOp.dynamic*globalBPT->stats_t.readAc.access +
                globalBPT->stats_t.writeAc.access*globalBPT->local_result.power.writeOp.dynamic;
    
	L1_localBPT->power_t.readOp.dynamic   +=  L1_localBPT->local_result.power.readOp.dynamic*L1_localBPT->stats_t.readAc.access +
                L1_localBPT->stats_t.writeAc.access*L1_localBPT->local_result.power.writeOp.dynamic;

    L2_localBPT->power_t.readOp.dynamic   +=  L2_localBPT->local_result.power.readOp.dynamic*L2_localBPT->stats_t.readAc.access +
                L2_localBPT->stats_t.writeAc.access*L2_localBPT->local_result.power.writeOp.dynamic;

    chooser->power_t.readOp.dynamic   +=  chooser->local_result.power.readOp.dynamic*chooser->stats_t.readAc.access +
                chooser->stats_t.writeAc.access*chooser->local_result.power.writeOp.dynamic;
    RAS->power_t.readOp.dynamic   +=  RAS->local_result.power.readOp.dynamic*RAS->stats_t.readAc.access +
                RAS->stats_t.writeAc.access*RAS->local_result.power.writeOp.dynamic;

				
	//cacti/const.h:262: const double pppm_lkg[4]  = {0,1,1,0};
	//core.cc:4276: set_pppm(coredynp.pppm_lkg_multhread, 0, coredynp.num_hthreads, coredynp.num_hthreads, 0);
	//power_t只有dynamic是不为零的值
    if (is_tdp)
    {
		//除了RAS和多线程相关之外，其它都一样
		//dynamic=power_t.readop.dynamic
		//leakage=local_result.power.readop.leakage
		//gate_leakage=local_result.power.readop.gate_leakage
    	globalBPT->power = globalBPT->power_t + globalBPT->local_result.power*pppm_lkg;
		L1_localBPT->power = L1_localBPT->power_t + L1_localBPT->local_result.power*pppm_lkg;
    	L2_localBPT->power = L2_localBPT->power_t + L2_localBPT->local_result.power*pppm_lkg;
    	chooser->power = chooser->power_t + chooser->local_result.power*pppm_lkg;
		
		//pppm_lkg_multhread={0, coredynp.num_hthreads, coredynp.num_hthreads, 0}
		//dynamic=power_t.readop.dynamic
		//leakage=local_result.power.readop.leakage*num_hthreads
		//gate_leakage=local_result.power.readop.gate_leakage*num_hthreads
    	RAS->power = RAS->power_t + RAS->local_result.power*coredynp.pppm_lkg_multhread;

		//总和
    	power = power + globalBPT->power + L1_localBPT->power + L2_localBPT->power + chooser->power + RAS->power;
    }
    else
    {
    	globalBPT->rt_power = globalBPT->power_t + globalBPT->local_result.power*pppm_lkg;
    	L1_localBPT->rt_power = L1_localBPT->power_t + L1_localBPT->local_result.power*pppm_lkg;
    	L2_localBPT->rt_power = L2_localBPT->power_t + L2_localBPT->local_result.power*pppm_lkg;
    	chooser->rt_power = chooser->power_t + chooser->local_result.power*pppm_lkg;
    	RAS->rt_power = RAS->power_t + RAS->local_result.power*coredynp.pppm_lkg_multhread;
    	rt_power = rt_power + globalBPT->rt_power + L1_localBPT->rt_power + L2_localBPT->rt_power + chooser->rt_power + RAS->rt_power;
    }
}


//is_tdp默认都是true，不和computerEnergy一样
void BranchPredictor::displayEnergy(uint32_t indent,int plevel,bool is_tdp)
{
	if (!exist) return;
	string indent_str(indent, ' ');
	string indent_str_next(indent+2, ' ');
	bool long_channel = XML->sys.longer_channel_device;
	bool power_gating = XML->sys.power_gating;
	if (is_tdp)//肯定会执行
	{
		
		//Area = globalBPT->area.get_area()*1e-6
		//Peak Dynamic = globalBPT->power.readOp.dynamic*clockRate
		//Subthreshold Leakage = (long_channel? globalBPT->power.readOp.longer_channel_leakage:globalBPT->power.readOp.leakage)
		//Subthreshold Leakage with power gating = (long_channel? globalBPT->power.readOp.power_gated_with_long_channel_leakage : globalBPT->power.readOp.power_gated_leakage)
		//Gate Leakage = globalBPT->power.readOp.gate_leakage
		//Runtime Dynamic = globalBPT->rt_power.readOp.dynamic/executionTime
	}
}

