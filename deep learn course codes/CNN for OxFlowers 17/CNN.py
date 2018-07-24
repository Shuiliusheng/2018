import numpy as np
import tensorflow as tf

train_times = 2001
batch_size = 32
dropout = 0.95
learning_rate = 0.002
test_size = 85


# 读取训练数据
def tf_train_reader():
    filename_queue = tf.train.string_input_producer(["train.tfrecords"])
    reader = tf.TFRecordReader()
    _, serialized_example = reader.read(filename_queue)
    features = tf.parse_single_example(serialized_example, features={
        'label': tf.FixedLenFeature([], tf.int64),
        'img_raw': tf.FixedLenFeature([], tf.string)})
    image = tf.decode_raw(features['img_raw'], tf.uint8)
    image = tf.reshape(image, [120, 120, 3])
    image = tf.cast(image, tf.float32)
    lab = tf.cast(features['label'], tf.int32)
    return image, lab


# 读取测试数据
def tf_test_reader():
    filename_queue = tf.train.string_input_producer(["test.tfrecords"])
    reader = tf.TFRecordReader()
    _, serialized_example = reader.read(filename_queue)
    features = tf.parse_single_example(serialized_example, features={
        'label': tf.FixedLenFeature([], tf.int64),
        'img_raw': tf.FixedLenFeature([], tf.string)})
    image = tf.decode_raw(features['img_raw'], tf.uint8)
    image = tf.reshape(image, [120, 120, 3])
    image = tf.cast(image, tf.float32)
    lab = tf.cast(features['label'], tf.int32)
    return image, lab


# 定义卷操作
def conv2d(name, x, W, b, strides=1):
    x = tf.nn.conv2d(x, W, strides=[1, strides, strides, 1], padding='SAME')
    x = tf.nn.bias_add(x, b)
    return tf.nn.relu(x, name=name)


# 定义池化操作
def maxpool2d(name, x, k=2):
    return tf.nn.max_pool(x, ksize=[1, k, k, 1], strides=[1, k, k, 1], padding='SAME', name=name)


# 归一化层LRN
def norm(name, l_input, lsize=4):
    return tf.nn.lrn(l_input, lsize, bias=1.0, alpha=0.001 / 9.0, beta=0.75, name=name)


# 网络内部参数，正太分布
weights = {
    'wc1': tf.Variable(tf.random_normal([5, 5, 3, 36])),
    'wc2': tf.Variable(tf.random_normal([3, 3, 36, 512])),
    'wd1': tf.Variable(tf.random_normal([4 * 4 * 512, 4096])),
    'wd2': tf.Variable(tf.random_normal([4096, 4096])),
    'out': tf.Variable(tf.random_normal([4096, 17]))
}

biases = {
    'bc1': tf.Variable(tf.random_normal([36])),
    'bc2': tf.Variable(tf.random_normal([512])),
    'bd1': tf.Variable(tf.random_normal([4096])),
    'bd2': tf.Variable(tf.random_normal([4096])),
    'out': tf.Variable(tf.random_normal([17]))
}


# 定义CNN 网络
def CNN(x, weights, biases, dropout):
    # the first layer convolution
    conv1 = conv2d('conv1', x, weights['wc1'], biases['bc1'])
    pool1 = maxpool2d('pool1', conv1, k=8)
    norm1 = norm('norm1', pool1, lsize=4)

    # the second layer convolution
    conv2 = conv2d('conv2', norm1, weights['wc2'], biases['bc2'])
    pool2 = maxpool2d('pool2', conv2, k=4)
    norm2 = norm('norm2', pool2, lsize=4)

    # the full connect layer1
    fc1 = tf.reshape(norm2, [-1, weights['wd1'].get_shape().as_list()[0]])
    fc1 = tf.add(tf.matmul(fc1, weights['wd1']), biases['bd1'])
    fc1 = tf.nn.relu(fc1)

    # the full connect layer2
    fc2 = tf.reshape(fc1, [-1, weights['wd2'].get_shape().as_list()[0]])
    fc2 = tf.add(tf.matmul(fc2, weights['wd2']), biases['bd2'])
    fc2 = tf.nn.relu(fc2)

    # dropout
    fc2 = tf.nn.dropout(fc2, dropout)

    # output layer
    out = tf.add(biases['out'], tf.matmul(fc2, weights['out']))
    return out


# 读取单个训练数据
img, lbl = tf_train_reader()
# 使用shuffle打乱并且打包成batch
img_batch, label_batch = tf.train.shuffle_batch([img, lbl], batch_size=batch_size,
                                                capacity=2000, min_after_dequeue=200, num_threads=2)
# 读取测试数据，用于测试
test_img, test_lbl = tf_test_reader()
test_image, test_label = tf.train.shuffle_batch([test_img, test_lbl], batch_size=test_size,
                                                capacity=2000, min_after_dequeue=200, num_threads=2)

# 定义网络中使用的参数
X = tf.placeholder(tf.float32, [None, 120, 120, 3])
Y1 = tf.placeholder(tf.int32, [None])
keep_prob = tf.placeholder(tf.float32)  # dropout
# label转换为向量
Y = tf.one_hot(Y1, 17, 1, 0)

# 调用CNN
Pred = CNN(X, weights, biases, keep_prob)

# 定义损失函数
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=Pred, labels=Y))
# 定义优化器
optimizer = tf.train.AdamOptimizer(learning_rate=learning_rate).minimize(cost)

# 测试当前batch的识别率
correct_pred = tf.equal(tf.argmax(Pred, 1), tf.argmax(Y, 1))
accuracy = tf.reduce_mean(tf.cast(correct_pred, tf.float32))

with tf.Session() as sess:
    # 初始化变量
    init = tf.initialize_all_variables()
    sess.run(init)
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners(coord=coord)

    for step in range(train_times):
        # 读取训练的batch数据
        x, y = sess.run([img_batch, label_batch])
        # 进行训练
        sess.run(optimizer, feed_dict={X: x, Y1: y, keep_prob: dropout})
        # 计算该batch的损失函数值和预测率
        loss, acc = sess.run([cost, accuracy], feed_dict={X: x, Y1: y, keep_prob: 1.})
        print("Iter " + str(step * batch_size) + ", Minibatch Loss= " + \
              "{:.6f}".format(loss) + ", Training Accuracy= " + "{:.5f}".format(acc))

        # 每过20次，测试一次测试集的预测率，分成四个batch，batchsize为85，
        # 计算四次取平均
        if step % 20 == 0:
            total_acc = 0.0
            for n in range(4):
                image_test, label_test = sess.run([test_image, test_label])
                acc1 = sess.run(accuracy, feed_dict={X: image_test, Y1: label_test, keep_prob: 1.})
                total_acc += acc1
            print("step:", step, "Total accuracy: ", total_acc / 4)
    print("Train ending!")

    # 训练完成之后，再测试一次测试集的预测率
    total_acc = 0.0
    for n1 in range(4):
        image_test, label_test = sess.run([test_image, test_label])
        acc1 = sess.run(accuracy, feed_dict={X: image_test, Y1: label_test, keep_prob: 1.})
        total_acc += acc1
        print("Testing Accuracy:", acc1)
    print("Total accuracy: ", total_acc / 4)
    coord.request_stop()
    coord.join(threads)
