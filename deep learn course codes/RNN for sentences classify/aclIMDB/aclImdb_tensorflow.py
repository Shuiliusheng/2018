import os
# regular expression
import re
import random
import numpy as np
import tensorflow as tf
from tensorflow.contrib import rnn

# Parameters
learning_rate = 0.001
batch_size = 50
# Network Parameters
n_hidden = 128
n_steps = 200
n_classes = 2  # MNIST total classes (0-9 digits)
epoch = 2

train_size = 12500
test_size = 25000

num_words = 1500
max_len = n_steps


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


def preprocess_word():
    x_train_v = read_sequences('train_sentences', max_len)
    print('Read Train sentences!')
    x_test_v = read_sequences('test_sentences', max_len)
    print('Read Test sentences!')
    y_train = read_sequences('train_label', n_classes)
    print('Read Train label!')
    y_test = read_sequences('test_label', n_classes)
    print('Read Test label')
    return x_train_v, y_train, x_test_v, y_test


x_train_v, y_train, x_test_v, y_test = preprocess_word()

# tf Graph input
x = tf.placeholder(tf.int32, [None, n_steps])
y = tf.placeholder(tf.float32, [None, n_classes])

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
    x = tf.one_hot(x, num_words)
    x = tf.cast(x, dtype=tf.float32)
    x = tf.unstack(x, n_steps, 1)
    # Define a lstm cell with tensorflow
    cell0 = rnn.BasicLSTMCell(64)
	cell1 = rnn.BasicLSTMCell(64)
    cell2 = rnn.BasicLSTMCell(n_hidden)
    cell=rnn.MultiRNNCell(cells=[cell0,cell1,cell2])
    # Get lstm cell output
    outputs, states = rnn.static_rnn(cell, x, dtype=tf.float32)
    # Linear activation, using rnn inner loop last output
    return tf.matmul(outputs[-1], weights['out']) + biases['out']


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
            for n in range(2):
                tstep = step + 12500 * n
                batch_x = np.array(x_train_v[tstep:tstep + b_size])
                batch_y = np.array(y_train[tstep:tstep + b_size])
                # Reshape data to get 28 seq of 28 elements
                batch_x = batch_x.reshape((b_size, n_steps))
                batch_y = batch_y.reshape((b_size, n_classes))
                # Run optimization op (backprop)
                sess.run(optimizer, feed_dict={x: batch_x, y: batch_y})

                # Calculate batch accuracy
                acc = sess.run(accuracy, feed_dict={x: batch_x, y: batch_y})
                # Calculate batch loss
                loss = sess.run(cost, feed_dict={x: batch_x, y: batch_y})
                print("Iter " + str(ep) + ",step " + str(step) + ", Minibatch Loss= " + \
                      "{:.6f}".format(loss) + ", Training Accuracy= " + "{:.5f}".format(acc))
            step += (b_size + 1)
    print("Optimization Finished!")
    acc = 0.0
    for n in range(125):
        batch_x = np.array(x_test_v[n * 200:n * 200 + 200]).reshape((200, n_steps))
        batch_y = np.array(y_test[n * 200:n * 200 + 200]).reshape((200, n_classes))
        acc1 = sess.run(accuracy, feed_dict={x: batch_x, y: batch_y})
        acc += acc1
        print('No ' + str(n * 200) + ' ' + "Testing Accuracy:", acc1)
    print('Testing Accuracy:', acc / 125)
