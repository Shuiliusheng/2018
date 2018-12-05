/*****************************************************************************
 *                                McPAT
 *                      SOFTWARE LICENSE AGREEMENT
 *            Copyright 2012 Hewlett-Packard Development Company, L.P.
 *                          All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
 *
 ***************************************************************************/
#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#include "XML_Parse.h"
#include "area.h"
#include "decoder.h"
#include "parameter.h"
#include "array.h"
#include "arbiter.h"
#include <vector>
#include "basic_components.h"
#include "core.h"
#include "memoryctrl.h"
#include "router.h"
#include "sharedcache.h"
#include "noc.h"
#include "iocontrollers.h"

class Processor : public Component
{
  //所有属性都是公共属性
  public:
	//用于通过XML，获取解析得到的属性值
	ParseXML *XML;
	//处理器中的核心，可以是多个，因此使用vector
	vector<Core *> cores;
	//L2 cache，可以为多个
    vector<SharedCache *> l2array;
	//L3 cache，可以为多个
    vector<SharedCache *> l3array;
	//L1 cache的目录
    vector<SharedCache *> l1dirarray;
	//L2 cache的目录
    vector<SharedCache *> l2dirarray;
	//片上网络，NoC，可以是多个
    vector<NoC *>  nocs;
	//存储控制器，MC
    MemoryController * mc;
	//网卡，NIU，一个
    NIUController    * niu;
	//总线，PCIe
    PCIeController   * pcie;
	//闪存控制器，Flashc
    FlashController  * flashcontroller;
	
	//将部分XML中的参数放入另一个类中。
	//该类是cacti中的一个方法，因此需要传递参数过去
    InputParameter interface_ip;
   
   //处理器的一些简单参数和名称，例如
	//int  numCore, numL2, numL3, numNOC, numL1Dir, numL2Dir,numMC, numMCChannel;
    //bool homoCore, homoL2, homoL3, homoNOC, homoL1Dir, homoL2Dir;
	//double vdd;double power_gating_vcc;
	ProcParam procdynp;
	
	//处理器中可能包括的所有组件
	//不论他们的个数多少，定义一个组件用于记录数据，例如功耗，面积
	//使用core来记录所有cores的参数
    Component core, l2, l3, l1dir, l2dir, noc, mcs, cc, nius, pcies,flashcontrollers;
	
	//处理器的组件的个数
    int  numCore, numL2, numL3, numNOC, numL1Dir, numL2Dir;
    
    //构造函数，会初始化所有的属性值，包括其中的对象
	//同时会进一步调用每个对象的方法，计算功耗和面积
	//最后将结果放入到componets中
	Processor(ParseXML *XML_interface);
	
	//没有实现这个函数，应该是所有计算过程都放入到了构造函数中
    void compute();
	
	//根据XML中解析的结果设置属性中的procdynp和interface_ip中的属性
    void set_proc_param();
	
	//按照一定的格式化显示功耗，面积等结果，会显示所有组件的结果
    void displayEnergy(uint32_t indent = 0,int plevel = 100, bool is_tdp=true);
    
	//使用字符串显示设备种类
	//ITRS high performance,ITRS low standby power
	//ITRS low operating power,LP-DRAM,COMM-DRAM
	void displayDeviceType(int device_type_, uint32_t indent = 0);
	
	//使用字符串显示互连关系
	//aggressive interconnect,conservative interconnect
    void displayInterconnectType(int interconnect_type_, uint32_t indent = 0);
	
	//析构函数，删除所有实例化的对象
    ~Processor();
};

#endif /* PROCESSOR_H_ */
