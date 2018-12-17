class Core :public Component {
  public:
	//用于获取解析得到的参数
	ParseXML *XML;
	//第几个核，标号
	int  ithCore;
	//用于和cacti通信的接口
	InputParameter interface_ip;
	//基础参数：时钟频率，运行时间
	double clockRate,executionTime;
	//通过cacti中的参数赋值：cacti/parameter.cc:TechnologyParameter g_tp
	//暂时不知道什么含义，没有在core.cc文件中使用
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	
	//定义一个核中的组成部分：类似于流水线的定义
	//取指单元
	InstFetchU * ifu;
	//LSQ单元
	LoadStoreU * lsu;
	//存储管理单元，包括TLB等
	MemManU    * mmu;
	//执行单元，FU
	EXECU      * exu;
	//重命名单元，RAT等
	RENAMINGU  * rnu;
	
	//流水线,logic.h 151
    Pipeline   * corepipe;
	
	//logic.h 213
    UndiffCore * undiffCore;
	
	//sharedcache.h 42
	//私有的L2 cache，虽然类名是share
	//共享的l2 cache在processor中计算
    SharedCache * l2cache;
	//core的动态参数
    CoreDynParam  coredynp;
	
	
	//构造函数,设置参数，初始化对象，计算面积，计算功率
	Core(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_);
	//设置一些参数
	void set_core_param();
	//计算功耗
	void computeEnergy(bool is_tdp=true);
	//显示计算的结果
	void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~Core();
};



//core内的第一个对象，取指单元
class InstFetchU :public Component {
  public:
	//获取参数
	ParseXML *XML;
	//核的标号
	int  ithCore;
	//传递给cacti的接口
	InputParameter interface_ip;
	//核的动态参数，计算使用
	CoreDynParam  coredynp;
	//基本参数，与core的一样
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	//WT/WB,basic_components.h 77
	enum Cache_policy cache_p;
	//指令cache的对象,array.h 70
	InstCache icache;
	//Instruction buffer指令缓冲区，用于缓存从cache中取到的指令
	//array.h 90
	ArrayST * IB;
	//BTB
	ArrayST * BTB;
	//分支预测器 branch pattern table
	BranchPredictor * BPT;
	//指令译码器,指令译码，操作数译码，混合指令
	//logic.h 95
	inst_decoder * ID_inst;
	inst_decoder * ID_operand;
	inst_decoder * ID_misc;
	//用于判断有没有生成该类的对象，析构函数中判断
	bool exist;
	
	//构造函数，设置参数，计算面积
	InstFetchU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exsit=true);
    //计算功率，power，rt_power
	void computeEnergy(bool is_tdp=true);
    //显示
	void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~InstFetchU();
};

//分支预测器
class BranchPredictor :public Component {
  public:
	//获取参数
	ParseXML *XML;
	//核的标号，只有通过核的标号才能够进一步在XML中找到具体的值
	int  ithCore;
	//cacti的接口
	InputParameter interface_ip;
	//core的动态参数
	CoreDynParam  coredynp;
	//与core设置的一样
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	//锦标赛的分支预测器，alpha 21264，比较常见
	//全局的BPT表
	ArrayST * globalBPT;
	//局部的BPT表？，没有被使用和实例化
	ArrayST * localBPT;
	//局部的第一层
	ArrayST * L1_localBPT;
	//局部的第二层
	ArrayST * L2_localBPT;
	//选择器
	ArrayST * chooser;
	//返回栈
	ArrayST * RAS;
	//用于判断对象是否存在
	bool exist;

	BranchPredictor(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exsit=true);
    void computeEnergy(bool is_tdp=true);
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~BranchPredictor();
};


//Load store单元
class LoadStoreU :public Component {
  public:
	//获取参数，一样的参数
	ParseXML *XML;
	//核标号
	int  ithCore;
	InputParameter interface_ip;
	CoreDynParam  coredynp;
	enum Cache_policy cache_p;
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	
	//高度，目前猜测是线程数
	double lsq_height;
	//L1 Dcache,array.h 90
	DataCache dcache;
	
	//它实际上是存储队列，但是对于inorder处理器，它同时充当loadQ和StoreQ
	ArrayST * LSQ;
	//loadQ
	ArrayST * LoadQ;
	//判断是否存在
	bool exist;

	//构造函数
	LoadStoreU(ParseXML *XML_interface, int ithCore_, 
		InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exist_=true);
	//计算功耗，峰值功耗和动态功耗
    void computeEnergy(bool is_tdp=true);
	//显示具体信息
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~LoadStoreU();
};

//内存控制单元
class MemManU :public Component {
  public:
	//一样的参数
	ParseXML *XML;
	int  ithCore;
	InputParameter interface_ip;
	CoreDynParam  coredynp;
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	
	//指令tlb和数据tlb
	ArrayST * itlb;
	ArrayST * dtlb;
	
	
	bool exist;
	MemManU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exist_=true);
    void computeEnergy(bool is_tdp=true);
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~MemManU();
};

//执行单元
class EXECU :public Component {
  public:
	//一样的参数
	ParseXML *XML;
	int  ithCore;
	InputParameter interface_ip;
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	
	//线程数？
	double lsq_height;
	CoreDynParam  coredynp;
	
	//寄存器单元
	RegFU          * rfu;
	//调度器 IQ
	SchedulerU     * scheu;
	//功能部件：浮点，定点，乘法
	//logic.h 189
    FunctionalUnit * fp_u;
    FunctionalUnit * exeu;
    FunctionalUnit * mul;
	//互连结构，定点bypass，乘法bypass，浮点bypass和相应的tag比较
	//interconnect.h 48
	interconnect * int_bypass;
	interconnect * intTagBypass;
	interconnect * int_mul_bypass;
	interconnect * intTag_mul_Bypass;
	interconnect * fp_bypass;
	interconnect * fpTagBypass;
	
	//用于记录信息的
	Component  bypass;
	bool exist;

	EXECU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_, double lsq_height_,const CoreDynParam & dyn_p_, bool exist_=true);
    void computeEnergy(bool is_tdp=true);
	void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~EXECU();
};


class RegFU :public Component {
  public:

	ParseXML *XML;
	int  ithCore;
	InputParameter interface_ip;
	CoreDynParam  coredynp;
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	double int_regfile_height, fp_regfile_height;
	
	//int reg file
	ArrayST * IRF;
	//float reg file
	ArrayST * FRF;
	//reg window: physical register file
	ArrayST * RFWIN;
	bool exist;

	RegFU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exist_=true);
    void computeEnergy(bool is_tdp=true);
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~RegFU();
};

//调度器
class SchedulerU :public Component {
  public:

	ParseXML *XML;
	int  ithCore;
	InputParameter interface_ip;
	CoreDynParam  coredynp;
	double clockRate,executionTime;
	double scktRatio, chip_PR_overhead, macro_PR_overhead;
	double Iw_height, fp_Iw_height,ROB_height;
	
	//定点指令窗口，int IQ
	ArrayST         * int_inst_window;
	//浮点指令串口，float IQ
	ArrayST         * fp_inst_window;
	//ROB
	ArrayST         * ROB;
	//选择逻辑,logic.h 52
    selection_logic * instruction_selection;
    bool exist;

    SchedulerU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_,const CoreDynParam & dyn_p_, bool exist_=true);
    void computeEnergy(bool is_tdp=true);
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~SchedulerU();
};
class RENAMINGU :public Component {
  public:

	ParseXML *XML;
	int  ithCore;
	InputParameter interface_ip;
	double clockRate,executionTime;
	CoreDynParam  coredynp;
	
	//int/float Front RAT
	ArrayST * iFRAT;
	ArrayST * fFRAT;
	//Int float RetireRAT
	ArrayST * iRRAT;
	ArrayST * fRRAT;
	
	//int/float free list
	ArrayST * ifreeL;
	ArrayST * ffreeL;
	
	//相关资源冲突检查逻辑
	//logic.h 72
	dep_resource_conflict_check * idcl;
	dep_resource_conflict_check * fdcl;
	
	//没有被使用
	ArrayST * RAHT;//register alias history table Used to store GC
	bool exist;


	RENAMINGU(ParseXML *XML_interface, int ithCore_, InputParameter* interface_ip_, const CoreDynParam & dyn_p_, bool exist_=true);
    void computeEnergy(bool is_tdp=true);
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
	~RENAMINGU();
};