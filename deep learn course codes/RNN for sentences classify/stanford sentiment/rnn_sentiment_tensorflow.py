import os
from keras.preprocessing import sequence
from keras.preprocessing.text import Tokenizer
from keras.utils.np_utils import to_categorical
# regular expression
import re
import random
import numpy as np
import tensorflow as tf
from tensorflow.contrib import rnn

# Parameters
learning_rate = 0.001
training_iters = 100000
batch_size = 50
display_step = 10

# Network Parameters
n_inputs = 5  # MNIST data input (img shape: 28*28)
n_steps = 20  # timesteps
n_hidden = 128  # hidden layer num of features
n_classes = 5  # MNIST total classes (0-9 digits)
epoch = 100

train_size = 8544
test_size = 2210


def rm_tags(text):
    re_tag = re.compile(r'<[^>]+>')
    return re_tag.sub('', text)


def read_files(path):
    all_labels = []
    all_texts = []
    fp = open(path, 'r')
    print('read', path)
    try:
        lines = fp.readlines()  # 读取出全部数据，按行存储
    finally:
        fp.close()
    for line in lines:
        line_list = line.split('|')  # 默认以空格为分隔符对字符串进行切片
        all_labels.append(int(line_list[1]) - 1)
        all_texts.append(rm_tags(line_list[2]))
    return all_labels, all_texts


def preprocess_word():
    y_train, x_train = read_files('/home/kesci/input/sentiment6266/train.txt')
    y_test, x_test = read_files('/home/kesci/input/sentiment2491/test.txt')
    y_train = to_categorical(y_train, 5)
    y_test = to_categorical(y_test, 5)

    token = Tokenizer(num_words=2000)
    token.fit_on_texts(x_train)
    x_train_seq = token.texts_to_sequences(x_train)
    x_test_seq = token.texts_to_sequences(x_test)
    x_train_v = sequence.pad_sequences(x_train_seq, maxlen=100)
    x_test_v = sequence.pad_sequences(x_test_seq, maxlen=100)
    return x_train_v, y_train, x_test_v, y_test


x_train_v, y_train, x_test_v, y_test = preprocess_word()

# tf Graph input
x = tf.placeholder("float", [None, n_steps, n_inputs])
y = tf.placeholder("float", [None, n_classes])

# Define weights
weights = {
    'layer1': tf.Variable(tf.random_normal([n_hidden, n_hidden])),
    'out': tf.Variable(tf.random_normal([n_hidden, n_classes]))
}
biases = {
    'layer1': tf.Variable(tf.random_normal([n_hidden])),
    'out': tf.Variable(tf.random_normal([n_classes]))
}


def RNN(x, weights, biases):
    # Prepare data shape to match `rnn` function requirements
    # Current data input shape: (batch_size, n_steps, n_input)
    # Required shape: 'n_steps' tensors list of shape (batch_size, n_input)

    # Unstack to get a list of 'n_steps' tensors of shape (batch_size, n_input)
    x = tf.unstack(x, n_steps, 1)

    # Define a lstm cell with tensorflow
    lstm_cell = rnn.BasicLSTMCell(n_hidden)

    # Get lstm cell output
    outputs, states = rnn.static_rnn(lstm_cell, x, dtype=tf.float32)

    layer1 = tf.matmul(outputs[-1], weights['layer1']) + biases['layer1']
    # Linear activation, using rnn inner loop last output
    return tf.matmul(layer1, weights['out']) + biases['out']


pred = RNN(x, weights, biases)

# Define loss and optimizer
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=pred, labels=y))
optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)

# Evaluate model
correct_pred = tf.equal(tf.argmax(pred, 1), tf.argmax(y, 1))
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
            Batch_size = batch_size
            if step + Batch_size > train_size:
                b_size = train_size - step - 1
            else:
                b_size = Batch_size
            batch_x = x_train_v[step:step + b_size]
            batch_y = y_train[step:step + b_size]
            # Reshape data to get 28 seq of 28 elements
            batch_x = batch_x.reshape((b_size, n_steps, n_inputs))
            batch_y = batch_y.reshape((b_size, 5))
            # Run optimization op (backprop)
            sess.run(optimizer, feed_dict={x: batch_x, y: batch_y})
            step += (b_size + 1)
        # Calculate batch accuracy
        acc = sess.run(accuracy, feed_dict={x: batch_x, y: batch_y})
        # Calculate batch loss
        loss = sess.run(cost, feed_dict={x: batch_x, y: batch_y})
        print("Iter " + str(ep) + ", Minibatch Loss= " + \
              "{:.6f}".format(loss) + ", Training Accuracy= " + \
              "{:.5f}".format(acc))
    print("Optimization Finished!")
    batch_x = x_test_v.reshape((test_size, n_steps, n_inputs))
    batch_y = y_test.reshape((test_size, 5))
    print("Testing Accuracy:", \
          sess.run(accuracy, feed_dict={x: batch_x, y: batch_y}))