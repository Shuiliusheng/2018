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
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <fstream>
#include "parameter.h"
#include "array.h"
#include "const.h"
#include "basic_circuit.h"
#include "XML_Parse.h"
#include "processor.h"
#include "version.h"


Processor::Processor(ParseXML *XML_interface):XML(XML_interface), mc(0),niu(0),pcie(0),
 flashcontroller(0)
{
	//设置参数
	set_proc_param();
	
	//设置processor对象的一些基本信息参数
	//numCore，numL2，numL3，numNOC，numL1Dir，numL2Dir
	//私有cache的个数必须和核数一致
	//同构的时候这些参数为1

	double pppm_t[4]    = {1,1,1,1};
 
/*  inline void set_pppm(double * pppv, double a=1, double b=1, double c=1, double d=1)
		{
			pppv[0]= a;pppv[1]= b;pppv[2]= c;pppv[3]= d;
		}
	*/
	//遍历所有核，为每一个核生成一个Core对象，压入cores的vector中
	//计算每一个核的功耗，面积，将结果放入core组件中
	for (i = 0;i < numCore; i++)
	{
		//实例化一个Core对象，并放入cores的容器中
		cores.push_back(new Core(XML,i, &interface_ip));
		//计算新加入核的能耗
		//computeEnergy(bool is_tdp=true)默认tdp为true
		//power和rt_power都是cacti的component基类的属性
		//这两个对象中包括着更细致的功耗分类
		cores[i]->computeEnergy();//计算TDP, power
		cores[i]->computeEnergy(false);//计算dyn_power，rt_power
		
		//如果是同构核的面积计算方法
		//同构核的情况下循环只会进行一遍
		if (procdynp.homoCore){
			
			//面积为核数乘以每个核的面积
			core.area.set_area(core.area.get_area() + cores[i]->area.get_area()*procdynp.numCore);
			//将数据记录在processor的area属性中
			area.set_area(area.get_area() + core.area.get_area());
			
			set_pppm(pppm_t,cores[i]->clockRate*procdynp.numCore, procdynp.numCore,procdynp.numCore,procdynp.numCore);
			//power*pppm_t的含义：power对象中的前四个属性值分别和数组中的前四个数相乘
			//power中的dynamic*clockRate*numCore
			//power中计算的dynamic为时钟每一次反转而产生的功耗
			core.power = core.power + cores[i]->power*pppm_t;
			//将数据记录在processor的属性中
			power = power  + core.power;
			
			set_pppm(pppm_t,1/cores[i]->executionTime, procdynp.numCore,procdynp.numCore,procdynp.numCore);
			//运行时动态功耗，之前的rt_power中的dynamic应该记录的是整个运行时的能耗
			core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
			rt_power = rt_power  + core.rt_power;
		}
		else{//异构核的处理
			//计算面积
			core.area.set_area(core.area.get_area() + cores[i]->area.get_area());
			area.set_area(area.get_area() + cores[i]->area.get_area());
			
			//基本一致，但是会为每个核都计算一遍
			set_pppm(pppm_t,cores[i]->clockRate, 1, 1, 1);
			core.power = core.power + cores[i]->power*pppm_t;
			power = power  + cores[i]->power*pppm_t;

			set_pppm(pppm_t,1/cores[i]->executionTime, 1, 1, 1);
			core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
			rt_power = rt_power  + cores[i]->rt_power*pppm_t;
		}
	}
	
	//如果不是私有的L2 cache
	//如果是私有cache，则会包含在core中
	if (!XML->sys.Private_L2)
	{
		//如果有多个L2 cache
		//同构的情况下，numL2=1
		if (numL2 >0)
		{
			for (i = 0;i < numL2; i++)
			{
				//实例化共享cache对象
				l2array.push_back(new SharedCache(XML,i, &interface_ip));
				l2array[i]->computeEnergy();//power
				l2array[i]->computeEnergy(false);//rt_power
				if (procdynp.homoL2){//同构核只会计算一遍，乘以倍数即可
					
					//计算面积
					l2.area.set_area(l2.area.get_area() + l2array[i]->area.get_area()*procdynp.numL2);
					area.set_area(area.get_area() + l2.area.get_area());//placement and routing overhead is 10%, l2 scales worse than cache 40% is accumulated from 90 to 22nm
					
					//计算power和rt_power
					set_pppm(pppm_t,l2array[i]->cachep.clockRate*procdynp.numL2, procdynp.numL2,procdynp.numL2,procdynp.numL2);
					l2.power = l2.power + l2array[i]->power*pppm_t;
					set_pppm(pppm_t,1/l2array[i]->cachep.executionTime, procdynp.numL2,procdynp.numL2,procdynp.numL2);
					l2.rt_power = l2.rt_power + l2array[i]->rt_power*pppm_t;
					power = power  + l2.power;
					rt_power = rt_power  + l2.rt_power;
				}
				else{//非同构需要累加
					l2.area.set_area(l2.area.get_area() + l2array[i]->area.get_area());
					area.set_area(area.get_area() + l2array[i]->area.get_area());//placement and routing overhead is 10%, l2 scales worse than cache 40% is accumulated from 90 to 22nm

					set_pppm(pppm_t,l2array[i]->cachep.clockRate, 1, 1, 1);
					l2.power = l2.power + l2array[i]->power*pppm_t;
					power = power  + l2array[i]->power*pppm_t;;
					set_pppm(pppm_t,1/l2array[i]->cachep.executionTime, 1, 1, 1);
					l2.rt_power = l2.rt_power + l2array[i]->rt_power*pppm_t;
					rt_power = rt_power  + l2array[i]->rt_power*pppm_t;
				}
			}
		}
	}
	//如果有L3 cache，一定是在核外，则计算
	if (numL3 >0)
	{	//如果是异构的，则需要将每一个都计算一遍
		for (i = 0;i < numL3; i++)
		{
		}
	}
	//如果有L1 directory，则计算
	if (numL1Dir >0)
	{
		//计算所有的
		for (i = 0;i < numL1Dir; i++)
		{
		}
	}
	if (numL2Dir >0)
	{	  
		for (i = 0;i < numL2Dir; i++)
		{
		}
	}
	//如果youMC，则计算
	if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
	{
		mc = new MemoryController(XML, &interface_ip, MC);
		mc->computeEnergy();
		mc->computeEnergy(false);
		//面积和功耗都要乘以MC的个数，默认是同构的
		mcs.area.set_area(mcs.area.get_area()+mc->area.get_area()*XML->sys.mc.number_mcs);
		area.set_area(area.get_area()+mc->area.get_area()*XML->sys.mc.number_mcs);
		
		//计算power
		set_pppm(pppm_t,XML->sys.mc.number_mcs*mc->mcp.clockRate, XML->sys.mc.number_mcs,XML->sys.mc.number_mcs,XML->sys.mc.number_mcs);
		mcs.power = mc->power*pppm_t;
		power = power  + mcs.power;
		
		//计算rt_power
		set_pppm(pppm_t,1/mc->mcp.executionTime, XML->sys.mc.number_mcs,XML->sys.mc.number_mcs,XML->sys.mc.number_mcs);
		mcs.rt_power = mc->rt_power*pppm_t;
		rt_power = rt_power  + mcs.rt_power;

	}
	//如果有flashc
	if (XML->sys.flashc.number_mcs >0 )//flash controller
	{
	}
	//如果有NIU
	if (XML->sys.niu.number_units >0)
	{
	}
	//如果有PICe
	if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels >0)
	{
	}

	//如果有NOC，NOC分为两种：一种是bus，另一种是routers
	if (numNOC >0)	
	{	//将所有的NOC都计算一遍，如果是同构的也只有一个
		for (i = 0;i < numNOC; i++)
		{
			if (XML->sys.NoC[i].type)//如果NoC是router的结构
			{//First add up area of routers if NoC is used
				nocs.push_back(new NoC(XML,i, &interface_ip, 1));
				//同构的乘以个数即可
				if (procdynp.homoNOC)
				{
					noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area()*procdynp.numNOC);
					area.set_area(area.get_area() + noc.area.get_area());
				}
				//异构的需要累加
				else
				{
					noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area());
					area.set_area(area.get_area() + nocs[i]->area.get_area());
				}
			}
			else//NoC是bus的结构
			{
				//link_len_ = sqrt(area.get_area()*XML->sys.NoC[i].chip_coverage)
				//初始化NoC对象的时候，需要给出连接长度
				//使用面积和参数中NoC覆盖的范围，估算连接长度
				nocs.push_back(new NoC(XML,i, &interface_ip, 1, sqrt(area.get_area()*XML->sys.NoC[i].chip_coverage)));
				if (procdynp.homoNOC){
					//同构乘以个数
					noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area()*procdynp.numNOC);
					area.set_area(area.get_area() + noc.area.get_area());
				}
				else
				{	//异构的需要累加
					noc.area.set_area(noc.area.get_area() + nocs[i]->area.get_area());
					area.set_area(area.get_area() + nocs[i]->area.get_area());
				}
			}
		}
		//计算全局的每个NoC中节点的连接关系，但是在此之前，整个的芯片面积必须已知
		for (i = 0;i < numNOC; i++)
		{
			//如果有全局的连接关系，并且是router的NoC结构
			if (nocs[i]->nocdynp.has_global_link && XML->sys.NoC[i].type)
			{
				//使用芯片面积和每个router的覆盖面积估算全局的bus长度
				//total_nodes = nocdynp.horizontal_nodes*nocdynp.vertical_nodes
				//指的是NoC内部的节点个数
				nocs[i]->init_link_bus(sqrt(area.get_area()*XML->sys.NoC[i].chip_coverage));
				if (procdynp.homoNOC)//同构乘以个数
				{
					noc.area.set_area(noc.area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						* nocs[i]->nocdynp.total_nodes* procdynp.numNOC);
					area.set_area(area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						* nocs[i]->nocdynp.total_nodes* procdynp.numNOC);
				}
				else
				{
					noc.area.set_area(noc.area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						* nocs[i]->nocdynp.total_nodes);
					area.set_area(area.get_area() + nocs[i]->link_bus_tot_per_Router.area.get_area()
						* nocs[i]->nocdynp.total_nodes);
				}
			}
		}
		//计算功耗
		for (i = 0;i < numNOC; i++)
		{
			nocs[i]->computeEnergy();
			nocs[i]->computeEnergy(false);
			if (procdynp.homoNOC){//同构的成倍数
				set_pppm(pppm_t,procdynp.numNOC*nocs[i]->nocdynp.clockRate, procdynp.numNOC,procdynp.numNOC,procdynp.numNOC);
				noc.power = noc.power + nocs[i]->power*pppm_t;
				
				set_pppm(pppm_t,1/nocs[i]->nocdynp.executionTime, procdynp.numNOC,procdynp.numNOC,procdynp.numNOC);
				noc.rt_power = noc.rt_power + nocs[i]->rt_power*pppm_t;
				
				power = power  + noc.power;
				rt_power = rt_power  + noc.rt_power;
			}
			else//异构累加
			{
				set_pppm(pppm_t,nocs[i]->nocdynp.clockRate, 1, 1, 1);
				noc.power = noc.power + nocs[i]->power*pppm_t;
				power = power  + nocs[i]->power*pppm_t;
				
				set_pppm(pppm_t,1/nocs[i]->nocdynp.executionTime, 1, 1, 1);
				noc.rt_power = noc.rt_power + nocs[i]->rt_power*pppm_t;
				rt_power = rt_power  + nocs[i]->rt_power*pppm_t;
			}
		}
	}

}

//is_tdp默认为true
void Processor::displayEnergy(uint32_t indent, int plevel, bool is_tdp)
{
	int i;
	bool long_channel = XML->sys.longer_channel_device;//一般为false
	bool power_gating = XML->sys.power_gating;//一般为true
	string indent_str(indent, ' ');
	string indent_str_next(indent+2, ' ');
	if (is_tdp)
	{
		cout <<"*****************************************************************************************"<<endl;
		cout <<indent_str<<"Technology "<<XML->sys.core_tech_node<<" nm"<<endl;
		
		//cout <<indent_str<<"Device Type= "<<XML->sys.device_type<<endl;
		if (long_channel)
			cout <<indent_str<<"Using Long Channel Devices When Appropriate"<<endl;
		//cout <<indent_str<<"Interconnect metal projection= "<<XML->sys.interconnect_projection_type<<endl;
		
		displayInterconnectType(XML->sys.interconnect_projection_type, indent);
		
		cout <<indent_str<<"Core clock Rate(MHz) "<<XML->sys.core[0].clock_rate<<endl;
    	cout <<endl;
		
		cout <<"*****************************************************************************************"<<endl;
		cout <<"Processor: "<<endl;
		//面积的计算：m^2 -> mm^2^2
		//area.get_area()*1e-6
		cout << indent_str << "Area = " << area.get_area()*1e-6<< " mm^2" << endl;
		//峰值功率的计算：power.readOp.dynamic+power.readOp.gate_leakage+leakage/power.readOp.longer_channel_leakage
		//power.readOp.dynamic + (long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage
		cout << indent_str << "Peak Power = " << power.readOp.dynamic +
			(long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage <<" W" << endl;
			
		//整体泄露功率的计算：power.readOp.gate_leakage+power.readOp.leakage/power.readOp.longer_channel_leakage
		//(long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage
		cout << indent_str << "Total Leakage = " <<
			(long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) + power.readOp.gate_leakage <<" W" << endl;
		
		//峰值动态功耗：power.readOp.dynamic
		cout << indent_str << "Peak Dynamic = " << power.readOp.dynamic << " W" << endl;
		
		//亚阈值泄漏功率：power.readOp.leakage/power.readOp.longer_channel_leakage
		cout << indent_str << "Subthreshold Leakage = " << (long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) <<" W" << endl;
		
		//有功率门控的亚阈值泄漏功率：power.readOp.power_gated_with_long_channel_leakage
		// or power.readOp.power_gated_leakage
		if (power_gating) cout << indent_str << "Subthreshold Leakage with power gating = "
				<< (long_channel? power.readOp.power_gated_with_long_channel_leakage : power.readOp.power_gated_leakage)  << " W" << endl;
		
		//栅漏功率：power.readOp.gate_leakage
		cout << indent_str << "Gate Leakage = " << power.readOp.gate_leakage << " W" << endl;
		
		//运行时动态功率：rt_power.readOp.dynamic
		cout << indent_str << "Runtime Dynamic = " << rt_power.readOp.dynamic << " W" << endl;
		cout <<endl;
		
		//有核的情况，显示所有的在一起的信息
		if (numCore >0){
			//core.power
			//Total Cores: 
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic = 
		}
		//非私有的L2 cache
		if (!XML->sys.Private_L2)
		{
			if (numL2 >0){
				//l2.power
				//Total L2s: 
				//Area = 
				//Peak Dynamic = 
				//Subthreshold Leakage = 
				//Subthreshold Leakage with power gating = 
				//Gate Leakage =
				//Runtime Dynamic =
			}
		}
		if (numL3 >0){
			//l3.power
			//Total L3s: 
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (numL1Dir >0){
			//l1dir.power
			//Total First Level Directory:
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (numL2Dir >0){
			//l2dir.power
			//Total Second Level Directory:
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (numNOC >0){
			//noc.power
			//Total NoCs (Network/Bus): 
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
		{
			//mcs.power
			//Total MCs:  
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (XML->sys.flashc.number_mcs >0)
		{
			//flashcontrollers.power
			//Total Flash/SSD Controllers:   
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (XML->sys.niu.number_units >0 )
		{
			//nius.power
			//Total NIUs: 
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels>0)
		{
			//pcies.power
			//Total PCIes: 
			//device type
			//Area = 
			//Peak Dynamic = 
			//Subthreshold Leakage = 
			//Subthreshold Leakage with power gating = 
			//Gate Leakage =
			//Runtime Dynamic =
		}
		cout <<"*****************************************************************************************"<<endl;
		if (plevel >1)
		{
			for (i = 0;i < numCore; i++)
				cores[i]->displayEnergy(indent+4,plevel,is_tdp);
			
			if (!XML->sys.Private_L2)
				for (i = 0;i < numL2; i++)
					l2array[i]->displayEnergy(indent+4,is_tdp);
					
			for (i = 0;i < numL3; i++)
				l3array[i]->displayEnergy(indent+4,is_tdp);

			for (i = 0;i < numL1Dir; i++)
				l1dirarray[i]->displayEnergy(indent+4,is_tdp);

			for (i = 0;i < numL2Dir; i++)
				l2dirarray[i]->displayEnergy(indent+4,is_tdp);

			if (XML->sys.mc.number_mcs >0 && XML->sys.mc.memory_channels_per_mc>0)
				mc->displayEnergy(indent+4,is_tdp);

			if (XML->sys.flashc.number_mcs >0 && XML->sys.flashc.memory_channels_per_mc>0)
				flashcontroller->displayEnergy(indent+4,is_tdp);

			if (XML->sys.niu.number_units >0 )
				niu->displayEnergy(indent+4,is_tdp);
				
			if (XML->sys.pcie.number_units >0 && XML->sys.pcie.num_channels>0)
				pcie->displayEnergy(indent+4,is_tdp);

			for (i = 0;i < numNOC; i++)
				nocs[i]->displayEnergy(indent+4,plevel,is_tdp);
		}
	}
	else
	{

	}

}