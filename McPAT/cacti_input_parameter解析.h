	//单位byte，整个cache的大小
    unsigned int cache_sz;  // in bytes
    //单位byte，每个cache line的大小
	unsigned int line_sz;
	//cache的相连度
    unsigned int assoc;
    //cache的bank个数
	unsigned int nbanks;
	//单位bit，input/output的bus宽度，cache line的宽度
    unsigned int out_w;// == nr_bits_out
	//用于指定tag的宽度是否需要cacti自己计算
	//为false，则cacti计算宽度
	//为true，则直接外部指令，不再计算
    bool     specific_tag;
	//单位bit，tag的宽度
    unsigned int tag_w;
	//访问模式
	//fast(2): 数据和tag同时访问
	//sequential(1)：先访问tag，然后再访问data
	//normal(0): 数据查找和tag访问同时进行，然后通过tag的信号，确定数据
    unsigned int access_mode;
	
	//未找到在cacti的使用
    unsigned int obj_func_dyn_energy;
    unsigned int obj_func_dyn_power;
    unsigned int obj_func_leak_power;
    unsigned int obj_func_cycle_t;

	//单位nm，工艺尺寸
    double   F_sz_nm;          // feature size in nm
    //单位um，工艺尺寸，F_sz_nm=1000*F_sz_um
	double   F_sz_um;          // feature size in um
    //用于指定hp vdd是否使用ITRS预测的值
	//false为不指定，true是指定
	//高性能类型
	bool     specific_hp_vdd;     // whether to have user defined vdd that is different from ITRS
	//用户指定的hp-vdd的值
    double   hp_Vdd;			   // user specified vdd
    //类似于LSTP：低待机功耗
    bool     specific_lstp_vdd;     // whether to have user  defined vdd that is different from ITRS
    double   lstp_Vdd;
    //低工作功耗
	bool     specific_lop_vdd;     // whether to have user defined vdd that is different from ITRS
    double   lop_Vdd;
    
	//用于决定是否使用用户指定的vcc_min来实现功率门控
	//false不使用，true使用
	bool     specific_vcc_min;     // whether to have user defined vcc_min for power-gating that is different from the value constrained by technology for maintaining states
    double   user_defined_vcc_min;
    //标志，当用户指令的vcc太低以至于无法保持电路稳定时，给出指示表示
	bool     user_defined_vcc_underflow; //flag to indicate when user defined vcc is too low for the circuit to retain state
   
	//读写端口数
    unsigned int num_rw_ports;
	//独占的读端口数
    unsigned int num_rd_ports;
	//独占的写端口数
    unsigned int num_wr_ports;
	//单端读端口数
    unsigned int num_se_rd_ports;  // number of single ended read ports
	//CAM存储类型中的搜索端口的数目
    unsigned int num_search_ports;  // number of search ports for CAM
	
	//cache type: cache, ram, cam, main memory
	//是否是主存，以决定是否按照主存建模
    bool     is_main_mem;
	//array的类型是否是cache类型的
    bool     is_cache;
	//array的类型是暂存器，类似于寄存器
    bool     pure_ram;
	//array的类型是CAM
    bool     pure_cam;
	//解析函数中，设置为true
    bool     rpters_in_htree;  // if there are repeaters in htree segment
	
	//没有看到赋值的地方，在mcpat中
    unsigned int ver_htree_wires_over_array;
    unsigned int broadcast_addr_din_over_ver_htrees;
	
	//工作温度
    unsigned int temp;

	//itrs-hp(0),lstp(1),lop(2),lp_dram(3),comm-dram(4)
    unsigned int ram_cell_tech_type;
	
	//未设置
    unsigned int peri_global_tech_type;
	
	//itrs-hp(0),lstp(1),lop(2),lp_dram(3),comm-dram(4)
    unsigned int data_arr_ram_cell_tech_type;
	//itrs-hp(0),lstp(1),lop(2)
	//Data array peripheral type(数据阵列外设类型)
    unsigned int data_arr_peri_global_tech_type;
	//itrs-hp(0),lstp(1),lop(2),lp_dram(3),comm-dram(4)
    unsigned int tag_arr_ram_cell_tech_type;
	//itrs-hp(0),lstp(1),lop(2)
	//tag array peripheral type(数据阵列外设类型)
    unsigned int tag_arr_peri_global_tech_type;

	//这三个参数只对主存意义
	//突发长度：连续传输的周期数就是突发长度
    unsigned int burst_len;
	//内部预取宽度
    unsigned int int_prefetch_w;
	//单位bit，页面大小
    unsigned int page_sz_bits;
	
	//互连规划设计的类型
	//aggressive(0), conservative(1)
    unsigned int ic_proj_type;      // interconnect_projection_type
    //mat内部的连线类型：global(2), local(0), else(1)
	unsigned int wire_is_mat_type;  // wire_inside_mat_type
    //mat外部的连线类型，global(2), else(1)
	unsigned int wire_os_mat_type; // wire_outside_mat_type
   
    //导线的类型：
	/*
		Global 		 : gloabl wires with repeaters 
		Global_5 	 : 5% delay penalty 
		Global_10 	 : 10% delay penalty 
		Global_20  	 : 20% delay penalty 
		Global_30 	 :	30% delay penalty 
		Low_swing 	 : differential low power wires with high area overhead 
		Semi_global  : mid-level wires with repeaters
		Transmission : tranmission lines with high area overhead 
		Optical 	 : optical wires 
		Invalid_wtype
	*/
    enum Wire_type wt;
	//除了当wt为global时为0，其余均为1
    int force_wiretype;
	
	//是否输出输入的参数信息
    bool print_input_args;
	
	//未设置
    unsigned int nuca_cache_sz; // TODO
	//强迫使用用户提供的一些cache优化的参数
    int ndbl, ndwl, nspd, ndsam1, ndsam2, ndcm;
	//决定是否使用用户提供的参数
    bool force_cache_config;

	//L2(0),L3(1)
    int cache_level;
	//核数，不能够大于16
    int cores;
	//NUCA的bank数量
    int nuca_bank_count;
	// 0/1(nuca_bank_count不为零，则都设置为1)
    int force_nuca_bank;

	//weight delay, dynamic power, leakage power, cycle time, area
    int delay_wt, dynamic_power_wt, leakage_power_wt,
        cycle_time_wt, area_wt;
	//NUCA的weight delay, dynamic power, leakage power, cycle time, area
    int delay_wt_nuca, dynamic_power_wt_nuca, leakage_power_wt_nuca,
        cycle_time_wt_nuca, area_wt_nuca;
	
	//deviate背离，delay, dynamic power, leakage power, cycle time, area
    int delay_dev, dynamic_power_dev, leakage_power_dev,
        cycle_time_dev, area_dev;
	//NUCA的deviate，delay, dynamic power, leakage power, cycle time, area
    int delay_dev_nuca, dynamic_power_dev_nuca, leakage_power_dev_nuca,
        cycle_time_dev_nuca, area_dev_nuca;
		
	//ed=1(ED),ed=2(ED^2)，优化的目标参数
    int ed; //ED or ED2 optimization
	//0(UCA), 1(NUCA)
    int nuca;
	
	//在error_checking函数中，根据access_mode设置
	//access_mode=normal/seq, fast_access=false
	//access_mode=fast, fast_access=true
    bool     fast_access;
	
	//在error_checking函数中设置
	//block_sz=line_sz
    unsigned int block_sz;  // bytes
	
	//如果是全相连，tag_assoc=cache_sz/nbanks/line_sz
	//如果不是全相连，tag_assoc=assoc
    unsigned int tag_assoc;
	
	//如果是全相连，data_assoc=1
	//如果不是全相连并且access mode=seq, 则data_assoc=1
	//否则data_assoc=tag_assoc
    unsigned int data_assoc;
	
	//如果access mode=seq,则为true，否则false
    bool     is_seq_acc;
	
	//在error_checking函数中设置
	//如果是cache，并且assoc=0，则是全相联
    bool     fully_assoc;
	
	//在error_checking函数中计算得到的组数
    unsigned int nsets;  // == number_of_sets
	
	
	//Detailed(1),Concise(0)
    int print_detail;

	//是否有ECC校验码
	bool     add_ecc_b_;
	
	//设计约束的参数，cacti中未设定
	//parameters for design constraint
	double throughput;
	double latency;
	
	//是否可流水化
	bool pipelinable;
	//流水线的个数
	int pipeline_stages;
	//流水线中每个阶段需要的vector/寄存器的数量
	int per_stage_vector;
	//false：全局时钟不驱动本地终端节点
	bool with_clock_grid;

	
	//是否支持array的功率门控，只在cacti中进行了设置
	bool array_power_gated;
	//是否支持位线浮动，只在cacti中进行了设置
	bool bitline_floating;
	//是否支持wordline功率门控，只在cacti中进行了设置
	bool wl_power_gated;
	//是否支持cacheline功率门控，只在cacti中进行了设置
	bool cl_power_gated;
	//是否支持互连结构功率门控，只在cacti中进行了设置
	bool interconect_power_gated;
	
	//是否支持门控技术
	bool power_gating;
	//门控所带来的性能损失
	double perfloss;
	//CLDriver vertical，只在cacti中进行了设置
	bool cl_vertical;
	//动态电压调节，假设无论每个阵列的设备类型如何，都将相同的电压应用于标签和数据阵列
	//，只在cacti中进行了设置
	std::vector<double> dvs_voltage;

	//是否使用长（10％）通道设备（true/false，当false时,假设90％的设备（非时间关键）可以是长通道设备）
	bool long_channel_device;

