# coding=utf-8

import os
import re

def rm_tags(text):
    re_tag = re.compile(r'<[^>]+>')
    return re_tag.sub('', text)


def read_files(path):
    dictionary=[]
    label=[]
    fp = open(path, 'r')
	num_sentences = 0
	length = 0
    print('read', path)
    try:
        lines = fp.readlines()  # 读取出全部数据，按行存储
    finally:
        fp.close()
    for line in lines:
        line_list = line.split('|')  # 默认以空格为分隔符对字符串进行切片
        word_list = re.split(r"[;,\s().]",line_list[2])
		length += len(word_list)
		num_sentences += 1
        for word in word_list:
			if word not in dictionary :
				dictionary.append(word)
				label.append(1)
			else:
				v = dictionary.index(word)
				label[v]+=1
	print('length:'+str(length)+'\n num:'+str(num_sentences)+'\n avg:'+str(length/num_sentences))
    return dictionary,label

def bubble_sort(dictionary,label):
    length=len(label)
    for i in range(length-1,0,-1):
        for j in range(i):
            if label[j]<label[j+1]:
                label[j],label[j+1]=label[j+1],label[j]
                dictionary[j],dictionary[j+1]=dictionary[j+1],dictionary[j]
    return dictionary,label

def save_dictionary(dictionary,label):
	dict = open('dictionary.txt', 'w')
	print 'write start!'
	try:
		for i in range(len(dictionary)):
			if i==0:
				continue
			dict.write(dictionary[i]+'|'+str(label[i])+'\n')
	finally:
		dict.close()
		print 'write finish!'
	
def get_dictionary():
	dictionary, label = read_files('train.txt')
	dictionary, label = bubble_sort(dictionary,label)
	save_dictionary(dictionary,label)
	
