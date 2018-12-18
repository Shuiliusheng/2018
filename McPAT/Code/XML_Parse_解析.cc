
void ParseXML::parse(char* filepath)
{
	//Initialize all structures
	ParseXML::initialize();

	//第一层为root
	XMLNode xMainNode=XMLNode::openFileHelper(filepath,"component"); //the 'component' in the first layer
	//第二层为system
	XMLNode xNode2=xMainNode.getChildNode("component"); // the 'component' in the second layer
	//itmp为system下所有的参数的个数
	itmp=xNode2.nChildNode("param");
	for(i=0; i<itmp; i++)
	{
		//使用判断语句，确定当前param对应的变量，并设置
		if (strcmp(xNode2.getChildNode("param",i).getAttribute("name"),"number_of_cores")==0)
		{
			sys.number_of_cores=atoi(xNode2.getChildNode("param",i).getAttribute("value"));
			continue;
		}
		//....
	}
	//itmp为system下所有的stat信息的个数
	itmp=xNode2.nChildNode("stat");
	for(i=0; i<itmp; i++)
	{
		if (strcmp(xNode2.getChildNode("stat",i).getAttribute("name"),"total_cycles")==0) 
		{
			sys.total_cycles=atof(xNode2.getChildNode("stat",i).getAttribute("value"));
			continue;
		}
		//...代码中仅解析了一个
	}

	//确定system下组件的个数，例如core，NoC，MC，L2，L3，Flashc等
	unsigned int NumofCom_3=xNode2.nChildNode("component");
	//define the third-layer(system.core0) and fourth-layer(system.core0.predictor) xnodes
	XMLNode xNode3,xNode4; 
	unsigned int OrderofComponents_3layer=0;
	if (NumofCom_3>OrderofComponents_3layer)
	{
		//如果是同构核，则只需要提取一个core的信息即可(same core)
		if (sys.homogeneous_cores==1) 
			OrderofComponents_3layer=0;
		else 
			OrderofComponents_3layer=sys.number_of_cores-1;
		
		//提取system中的core组件，可能是多个，在异构的情况下
		for (i=0; i<=OrderofComponents_3layer; i++)
		{
			xNode3=xNode2.getChildNode("component",i);
			if (xNode3.isEmpty()==1) {}
			else{
				if(strstr(xNode3.getAttribute("name"),"core")!=NULL)
				{
					//core的参数个数
					itmp=xNode3.nChildNode("param");
					for(k=0; k<itmp; k++)
					{
						//...解析core的parameters，并保存
						
						//value有多个，需要特殊处理
						if (strcmp(xNode3.getChildNode("param",k).getAttribute("name"),"pipelines_per_core")==0){}
						if (strcmp(xNode3.getChildNode("param",k).getAttribute("name"),"pipeline_depth")==0){}
					}
					//core的stat个数
					itmp=xNode3.nChildNode("stat");
					for(k=0; k<itmp; k++)
					{
						//...解析core的stats，并保存
					}
					//core中组件的个数
					NumofCom_4=xNode3.nChildNode("component");
					for(j=0; j<NumofCom_4; j++)
					{
						xNode4=xNode3.getChildNode("component",j);
						if (strcmp(xNode4.getAttribute("name"),"PBT")==0)
						{
							itmp=xNode4.nChildNode("param");
							for(k=0; k<itmp; k++){}
							itmp=xNode4.nChildNode("stat");
							for(k=0; k<itmp; k++){}

						}
						if (strcmp(xNode4.getAttribute("name"),"itlb")==0){}
						if (strcmp(xNode4.getAttribute("name"),"icache")==0){}
						if (strcmp(xNode4.getAttribute("name"),"dtlb")==0){}
						if (strcmp(xNode4.getAttribute("name"),"dcache")==0){}
						if (strcmp(xNode4.getAttribute("name"),"BTB")==0){}
					}
				}
				else {
					printf("The value of homogeneous_cores or number_of_cores is not correct!");
				}
			}
		}

		int w,tmpOrderofComponents_3layer;
		w=OrderofComponents_3layer+1;
		tmpOrderofComponents_3layer=OrderofComponents_3layer;
		if (sys.homogeneous_L1Directories==1) OrderofComponents_3layer=OrderofComponents_3layer+1;
		else OrderofComponents_3layer=OrderofComponents_3layer+sys.number_of_L1Directories;

		//提取L1directory组件的所有信息
		for (i=0; i<(OrderofComponents_3layer-tmpOrderofComponents_3layer); i++)
		{
			xNode3=xNode2.getChildNode("component",w);
			if (xNode3.isEmpty()==1) {
				printf("The value of homogeneous_L1Directories or number_of_L1Directories is not correct!");
			}
			else
			{
				if (strstr(xNode3.getAttribute("id"),"L1Directory")!=NULL)
				{
					itmp=xNode3.nChildNode("param");
					for(k=0; k<itmp; k++){}
					itmp=xNode3.nChildNode("stat");
					for(k=0; k<itmp; k++){}
					w=w+1;
				}
				else {
					printf("The value of homogeneous_L1Directories or number_of_L1Directories is not correct!");
					exit(0);
				}
			}
		}

		w=OrderofComponents_3layer+1;
		tmpOrderofComponents_3layer=OrderofComponents_3layer;
		if (sys.homogeneous_L2Directories==1) OrderofComponents_3layer=OrderofComponents_3layer+1;
		else OrderofComponents_3layer=OrderofComponents_3layer+sys.number_of_L2Directories;
		//提取L1directory组件的所有信息
		for (i=0; i<(OrderofComponents_3layer-tmpOrderofComponents_3layer); i++)
		{
			xNode3=xNode2.getChildNode("component",w);
			if (xNode3.isEmpty()==1) {
				printf("The value of homogeneous_L2Directories or number_of_L2Directories is not correct!");
			}
			else
			{
				if (strstr(xNode3.getAttribute("id"),"L2Directory")!=NULL)
				{
					itmp=xNode3.nChildNode("param");
					itmp=xNode3.nChildNode("stat");
					w=w+1;
				}
				else {
					printf("The value of homogeneous_L2Directories or number_of_L2Directories is not correct!");
				}
			}
		}

		w=OrderofComponents_3layer+1;
		tmpOrderofComponents_3layer=OrderofComponents_3layer;
		if (sys.homogeneous_L2s==1) OrderofComponents_3layer=OrderofComponents_3layer+1;
		else OrderofComponents_3layer=OrderofComponents_3layer+sys.number_of_L2s;
		//提取L2 cache组件的所有信息
		for (i=0; i<(OrderofComponents_3layer-tmpOrderofComponents_3layer); i++)
		{
			xNode3=xNode2.getChildNode("component",w);
			if (xNode3.isEmpty()==1) {
				printf("The value of homogeneous_L2s or number_of_L2s is not correct!");
			}
			else
			{
				if (strstr(xNode3.getAttribute("name"),"L2")!=NULL)
				{
					w=w+1;
				}
				else {
					printf("The value of homogeneous_L2s or number_of_L2s is not correct!");
				}
			}
		}

		w=OrderofComponents_3layer+1;
		tmpOrderofComponents_3layer=OrderofComponents_3layer;
		if (sys.homogeneous_L3s==1) OrderofComponents_3layer=OrderofComponents_3layer+1;
		else OrderofComponents_3layer=OrderofComponents_3layer+sys.number_of_L3s;
		//提取L2 cache组件的所有信息
		for (i=0; i<(OrderofComponents_3layer-tmpOrderofComponents_3layer); i++)
		{
			xNode3=xNode2.getChildNode("component",w);
			if (xNode3.isEmpty()==1) {
				printf("The value of homogeneous_L3s or number_of_L3s is not correct!");
			}
			else
			{
				if (strstr(xNode3.getAttribute("name"),"L3")!=NULL)
				{
					w=w+1;
				}
				else {
					printf("The value of homogeneous_L3s or number_of_L3s is not correct!");
				}
			}
		}
		
		
		w=OrderofComponents_3layer+1;
		tmpOrderofComponents_3layer=OrderofComponents_3layer;
		if (sys.homogeneous_NoCs==1) OrderofComponents_3layer=OrderofComponents_3layer+1;
		else OrderofComponents_3layer=OrderofComponents_3layer+sys.number_of_NoCs;
		//提取NoC组件的所有信息
		for (i=0; i<(OrderofComponents_3layer-tmpOrderofComponents_3layer); i++)
		{
			xNode3=xNode2.getChildNode("component",w);
			if (xNode3.isEmpty()==1) {
				printf("The value of homogeneous_NoCs or number_of_NoCs is not correct!");
			}
			else
			{
				if (strstr(xNode3.getAttribute("name"),"noc")!=NULL)
				{
					w=w+1;
				}
			}
		}
		
		if (OrderofComponents_3layer>0) OrderofComponents_3layer=OrderofComponents_3layer+1;
		xNode3=xNode2.getChildNode("component",OrderofComponents_3layer);
		if (xNode3.isEmpty()==1) {
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
			exit(0);
		}
		//提取MC组件的所有信息
		if (strstr(xNode3.getAttribute("id"),"system.mc")!=NULL)
		{
			itmp=xNode3.nChildNode("param");
			for(k=0; k<itmp; k++){}
			itmp=xNode3.nChildNode("stat");
			for(k=0; k<itmp; k++){}
		}
		else{
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
		}
		
		if (OrderofComponents_3layer>0) OrderofComponents_3layer=OrderofComponents_3layer+1;
		xNode3=xNode2.getChildNode("component",OrderofComponents_3layer);
		if (xNode3.isEmpty()==1) {
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
		}
		////提取NIU组件的所有信息
		if (strstr(xNode3.getAttribute("id"),"system.niu")!=NULL)
		{
			itmp=xNode3.nChildNode("param");
			itmp=xNode3.nChildNode("stat");
		}
		else{
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
		}

		if (OrderofComponents_3layer>0) OrderofComponents_3layer=OrderofComponents_3layer+1;
		xNode3=xNode2.getChildNode("component",OrderofComponents_3layer);
		if (xNode3.isEmpty()==1) {
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
			exit(0);
		}
		////提取PCIe组件的所有信息
		if (strstr(xNode3.getAttribute("id"),"system.pcie")!=NULL)
		{
			itmp=xNode3.nChildNode("param");
	
			itmp=xNode3.nChildNode("stat");
			
		}
		else{
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
		}
		//__________________________________________Get system.flashcontroller____________________________________________
		if (OrderofComponents_3layer>0) OrderofComponents_3layer=OrderofComponents_3layer+1;
		xNode3=xNode2.getChildNode("component",OrderofComponents_3layer);
		if (xNode3.isEmpty()==1) {
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
			exit(0);
		}
		////提取FC组件的所有信息
		if (strstr(xNode3.getAttribute("id"),"system.flashc")!=NULL)
		{
			itmp=xNode3.nChildNode("param");
			itmp=xNode3.nChildNode("stat");
			
		}
		else{
			printf("some value(s) of number_of_cores/number_of_L2s/number_of_L3s/number_of_NoCs is/are not correct!");
		}
	}
}
