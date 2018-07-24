import os
# regular expression
import re
import random
import numpy as np
import tensorflow as tf

# Parameters
learning_rate = 0.001
batch_size = 1000

# Network Parameters
n_inputs = 10  # MNIST data input (img shape: 28*28)
n_hidden = 256
n_steps = 10
n_classes = 2  # MNIST total classes (0-9 digits)
epoch = 100

train_size = 25000
test_size = 25000

num_words = 2000
max_len = 256


def rm_tags(text):
    re_tag = re.compile(r'<[^>]+>')
    return re_tag.sub('', text)


def read_dictionary(path):
    dictionary = []
    fp = open(path, 'r')
    try:
        lines = fp.readlines()
    finally:
        fp.close()
    for line in lines:
        word_list = line.split('\n')
        dictionary.append(word_list[0])
    return dictionary


def read_files(filetype):
    path = "./aclImdb/"
    file_list = []
    positive_path = path + filetype + "/pos/"
    for f in os.listdir(positive_path):
        file_list += [positive_path + f]
    negative_path = path + filetype + "/neg/"
    for f in os.listdir(negative_path):
        file_list += [negative_path + f]
    print('read', filetype, 'files:', len(file_list))
    all_labels = ([1] * 12500 + [0] * 12500)
    all_texts = []
    for fi in file_list:
        with open(fi, 'r') as file_input:
            filelines = file_input.readlines()
            all_texts += [rm_tags(filelines[0])]
    return all_labels, all_texts


def sentence_to_seq(sentences, dictionary, path):
    fp = open(path, 'w')
    print('write start!')
    i = 0
    length = 0
    max_l = 0
    try:
        for sentence in sentences:
            i = i + 1
            if i % 1000 == 0:
                print(i)
            temp = [0] * max_len
            word_list = re.split(r"[;,\s().]", sentence)
            length += len(word_list)
            if max_l < len(word_list):
                max_l = len(word_list)
            for n in range(len(word_list)):
                if n >= max_len:
                    break
                if word_list[n] in dictionary:
                    v = dictionary.index(word_list[n])
                    if v < num_words:
                        temp[n] = v
            for t in temp:
                fp.write(str(t) + ' ')
            fp.write('\n')
        print('max_length:' + str(max_l))
        print('length:' + str(length))
        print('num:' + str(len(sentences)))
        print('avg:' + str(length / len(sentences)))
    finally:
        fp.close()
    print('write finish!')


def read_sequences(path, num):
    seq = []
    fp = open(path, 'r')
    try:
        lines = fp.readlines()
    finally:
        fp.close()
    for line in lines:
        word_list = line.split(' ')
        temp = []
        for i in range(num):
            temp.append(int(word_list[i]))
        seq.append(temp)
    return seq


def expand_label(labels, path):
    fp = open(path, 'w')
    try:
        for label in labels:
            temp = [0] * n_classes
            temp[label - 1] = 1
            for t in temp:
                fp.write(str(t) + ' ')
            fp.write('\n')
    finally:
        fp.close()
        print('write finish!')


def preprocess_word():
    y_train, x_train = read_files('train')
    print('read train sentences!')
    y_test, x_test = read_files('test')
    print('read test sentences!')
    dictionary = read_dictionary('./aclImdb/imdb.vocab')
    print('read dictionary!')

    sentence_to_seq(x_train, dictionary, 'train_sentences')
    print('Train sentences to sequences!')
    sentence_to_seq(x_test, dictionary, 'test_sentences')
    print('Test sentences to sequences!')
    expand_label(y_train, 'train_label')
    expand_label(y_test, 'test_label')
    print('expand labels to sequences!')


preprocess_word()