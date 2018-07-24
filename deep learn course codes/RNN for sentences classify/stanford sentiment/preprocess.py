#! /usr/bin/env python
#coding=utf-8

# 读取sentiment_labels文件中对应的标签
def Read_Label():
	label = []
	fp = open('sentiment_labels.txt', 'r')
	try:
		lines = fp.readlines()#读取出全部数据，按行存储
	finally:
		fp.close()
	for line in lines:
		line_list = line.split('|') #默认以空格为分隔符对字符串进行切片
		if line_list[0] == 'phrase ids':
			continue
		value = float(line_list[1])*10
		if value <= 2 :#[0,2]
			value=1
		elif value <= 4:#(2,4]
			value=2
		elif value <= 6:#(4,6]
			value=3
		elif value <= 8 :#(6,8]
			value=4
		else:#(8,10]
			value=5
		label.append(value)
	return label

	
# 读取dictionary文件中所有的表项，用于查找，包括每句话的内容和标签
def Read_Dictionary():
	content = []
	label = []
	fp = open('dictionary.txt', 'r')
	try:
		lines = fp.readlines()#读取出全部数据，按行存储
	finally:
		fp.close()
	for line in lines:
		line_list = line.split('|') #默认以空格为分隔符对字符串进行切片
		content.append(line_list[0])
		label.append(int(line_list[1]))
	return content,label
	
#读取datasetSplit文件中对应于sentences的每个类别划分
def Read_Split():
	Split = []
	fp = open('datasetSplit.txt', 'r')
	try:
		lines = fp.readlines()#读取出全部数据，按行存储
	finally:
		fp.close()
	for line in lines:
		line_list = line.split(',') #默认以空格为分隔符对字符串进行切片
		if line_list[0] == 'sentence_index':
			continue
		Split.append(int(line_list[1]))
	return Split
	
#读取所有的语句
def Read_Sentences():
	content = []
	fp = open('datasetSentences.txt', 'r')
	try:
		lines = fp.readlines()#读取出全部数据，按行存储
	finally:
		fp.close()
	for line in lines:
		line_list = line.split('	') #默认以空格为分隔符对字符串进行切片
		if line_list[0] == 'sentence_index':
			continue
		content.append(line_list[1].strip('\n'))
	return content

	
#抽取三个文件train,test,validation，同时给出每个句子对应的label：1，2，3，4，5
def Write_File(label,dict_content,dict_label,Split,sentences):
	train = open('train.txt', 'w')
	test = open('test.txt','w')
	vad = open('valid.txt','w')
	print 'write start!'
	try:
		n1=0
		n2=0
		n3=0
		for i in range(len(sentences)):
			#print('write -> '+str(i)+' '+str(sentences[i] in dict_content))
			# 判断是否能够在dictionary中查找到该表项
			if sentences[i] not in dict_content :
				print('write -> '+str(i)+' '+str(sentences[i] in dict_content))
				continue
			v1 = dict_content.index(sentences[i])
			v = label[dict_label[v1]]
			if Split[i] == 1:
				n1=n1+1;
				train.write(str(n1)+'|'+str(v)+'|'+sentences[i]+'\n')
			elif Split[i] == 2:
				n2=n2+1
				test.write(str(n2)+'|'+str(v)+'|'+sentences[i]+'\n')
			else:
				n3=n3+1
				vad.write(str(n3)+'|'+str(v)+'|'+sentences[i]+'\n')
	finally:
		train.close()
		test.close()
		vad.close()
		print 'write finish!'


label = Read_Label()
dict_content,dict_label=Read_Dictionary()
Split = Read_Split()
sentences = Read_Sentences()
Write_File(label,dict_content,dict_label,Split,sentences)
