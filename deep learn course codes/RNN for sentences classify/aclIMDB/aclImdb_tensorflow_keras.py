#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import random
import tensorflow as tf

# Parameters
learning_rate = 0.001
training_iters = 100000
batch_size = 1000

# Network Parameters
n_inputs = 100  # MNIST data input (img shape: 28*28)
n_classes = 2  # MNIST total classes (0-9 digits)
epoch = 100

train_size = 25000
test_size = 25000


def rm_tags(text):
    re_tag = re.compile(r'<[^>]+>')
    return re_tag.sub('', text)

def read_files(filetype):
    path = "./aclImdb/"
    file_list=[]
    positive_path=path + filetype+"/pos/"
    for f in os.listdir(positive_path):
        file_list+=[positive_path+f]   
    negative_path=path + filetype+"/neg/"
    for f in os.listdir(negative_path):
        file_list+=[negative_path+f]      
    print('read',filetype, 'files:',len(file_list))      
    all_labels = ([1] * 12500 + [0] * 12500)    
    all_texts  = []
    for fi in file_list:
        with open(fi,encoding='utf8') as file_input:
            filelines = file_input.readlines()       
            all_texts += [rm_tags(filelines[0])]         
    return all_labels,all_texts

def preprocess_word():
    y_train, x_train = read_files('train')
    y_test, x_test = read_files('test')

    y_train = tf.keras.utils.to_categorical(y_train, n_classes)
    y_test = tf.keras.utils.to_categorical(y_test, n_classes)
    
    token = tf.keras.preprocessing.text.Tokenizer(num_words=2000)
    token.fit_on_texts(x_train)
    x_train_seq = token.texts_to_sequences(x_train)
    x_test_seq = token.texts_to_sequences(x_test)
    x_train_v = tf.keras.preprocessing.sequence.pad_sequences(x_train_seq, maxlen=n_inputs)
    x_test_v = tf.keras.preprocessing.sequence.pad_sequences(x_test_seq, maxlen=n_inputs)
    return x_train_v, y_train, x_test_v, y_test

x_train_v, y_train, x_test_v, y_test = preprocess_word()

input_x = tf.placeholder(tf.float32, shape=(None, n_inputs))
label_y = tf.placeholder(tf.float32, shape=(None, n_classes))

x = tf.keras.layers.Embedding(2000, 32, input_length=n_inputs)(input_x)
x = tf.keras.layers.SimpleRNN(32)(x)
x = tf.keras.layers.Dense(256, activation='relu')(x)
pred = tf.keras.layers.Dense(n_classes, activation='sigmoid')(x)

cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=pred, labels=label_y))
optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)

# Evaluate model
correct_pred = tf.equal(tf.argmax(pred, 1), tf.argmax(label_y, 1))
accuracy = tf.reduce_mean(tf.cast(correct_pred, tf.float32))

# Initializing the variables
init = tf.global_variables_initializer()
# Launch the graph
with tf.Session() as sess:
    sess.run(init)
    for ep in range(epoch):
        step = random.randint(0, 100)
        # Keep training until reach max iterations
        while step < train_size:
            if step + batch_size > train_size:
                b_size = train_size - step - 1
            else:
                b_size = batch_size
            batch_x = x_train_v[step:step + b_size]
            batch_y = y_train[step:step + b_size]
            sess.run(optimizer, feed_dict={input_x: batch_x, label_y: batch_y})
            step += (b_size + 1)
        # Calculate batch accuracy
        acc = sess.run(accuracy, feed_dict={input_x: batch_x, label_y: batch_y})
        # Calculate batch loss
        loss = sess.run(cost, feed_dict={input_x: batch_x, label_y: batch_y})
        print("Iter " + str(ep) + ", Minibatch Loss= " + "{:.6f}".format(
            loss) + ", Training Accuracy= " + "{:.5f}".format(acc))

    print("Optimization Finished!")
    print("Testing Accuracy:", sess.run(accuracy, feed_dict={input_x: x_test_v, label_y: y_test}))
