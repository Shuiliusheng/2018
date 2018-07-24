#!/usr/bin/env python
# -*- coding:utf-8 -*-
import numpy as np
import tensorflow as tf
from keras import layers
from keras.layers import Input, Add, Dense, Activation, ZeroPadding2D, \
    BatchNormalization, Flatten, Conv2D, AveragePooling2D, MaxPooling2D, GlobalMaxPooling2D
from keras.models import Model, load_model
from keras.preprocessing import image
from keras.utils import layer_utils
from keras.utils.data_utils import get_file
from keras.applications.imagenet_utils import preprocess_input
from keras.utils.vis_utils import model_to_dot
from keras.utils import plot_model
from keras.initializers import glorot_uniform


EPOCH = 60
BATCH_SIZE = 32

TEST_SIZE = 794
TRAIN_SIZE = 4750

train_name = 'train_single.tfrecords'
test_name = 'test_raw_64.tfrecords'


# 恒等模块——identity_block
def identity_block(X, f, filters, stage, block):
    # 定义基本的名字
    conv_name_base = "res" + str(stage) + block + "_branch"
    bn_name_base = "bn" + str(stage) + block + "_branch"

    # 过滤器
    F1, F2, F3 = filters

    # 保存输入值,后面将需要添加回主路径
    X_shortcut = X

    # 主路径第一部分
    X = Conv2D(filters=F1, kernel_size=(1, 1), strides=(1, 1), padding="valid",
               name=conv_name_base + "2a", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2a")(X)
    X = Activation("relu")(X)

    # 主路径第二部分
    X = Conv2D(filters=F2, kernel_size=(f, f), strides=(1, 1), padding="same",
               name=conv_name_base + "2b", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2b")(X)
    X = Activation("relu")(X)

    # 主路径第三部分
    X = Conv2D(filters=F3, kernel_size=(1, 1), strides=(1, 1), padding="valid",
               name=conv_name_base + "2c", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2c")(X)

    # 主路径最后部分,为主路径添加shortcut并通过relu激活
    X = layers.add([X, X_shortcut])
    X = Activation("relu")(X)
    return X


# 卷积残差块——convolutional_block
def convolutional_block(X, f, filters, stage, block, s=2):
    # 定义基本的名字
    conv_name_base = "res" + str(stage) + block + "_branch"
    bn_name_base = "bn" + str(stage) + block + "_branch"

    # 过滤器
    F1, F2, F3 = filters

    # 保存输入值,后面将需要添加回主路径
    X_shortcut = X

    # 主路径第一部分
    X = Conv2D(filters=F1, kernel_size=(1, 1), strides=(s, s), padding="valid",
               name=conv_name_base + "2a", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2a")(X)
    X = Activation("relu")(X)

    # 主路径第二部分
    X = Conv2D(filters=F2, kernel_size=(f, f), strides=(1, 1), padding="same",
               name=conv_name_base + "2b", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2b")(X)
    X = Activation("relu")(X)

    # 主路径第三部分
    X = Conv2D(filters=F3, kernel_size=(1, 1), strides=(1, 1), padding="valid",
               name=conv_name_base + "2c", kernel_initializer=glorot_uniform(seed=0))(X)
    X = BatchNormalization(axis=3, name=bn_name_base + "2c")(X)

    # shortcut路径
    X_shortcut = Conv2D(filters=F3, kernel_size=(1, 1), strides=(s, s), padding="valid",
                        name=conv_name_base + "1", kernel_initializer=glorot_uniform(seed=0))(X_shortcut)
    X_shortcut = BatchNormalization(axis=3, name=bn_name_base + "1")(X_shortcut)

    # 主路径最后部分,为主路径添加shortcut并通过relu激活
    X = layers.add([X, X_shortcut])
    X = Activation("relu")(X)
    return X


# 50层ResNet模型构建
def ResNet50(input_shape=(64, 64, 3), classes=12):
    # 将输入定义为维度大小为 input_shape的张量
    X_input = Input(input_shape)

    # Zero-Padding
    #X = ZeroPadding2D((3, 3))(X_input)

    # Stage 1
    X = Conv2D(64, kernel_size=(7, 7), strides=(2, 2), name="conv1", kernel_initializer=glorot_uniform(seed=0))(X_input)
    X = BatchNormalization(axis=3, name="bn_conv1")(X)
    X = Activation("relu")(X)
    X = MaxPooling2D(pool_size=(3, 3), strides=(2, 2))(X)

    # Stage 2
    X = convolutional_block(X, f=5, filters=[64, 64, 256], stage=2, block="a", s=1)
    X = identity_block(X, f=5, filters=[64, 64, 256], stage=2, block="b")

    # Stage 3
    X = convolutional_block(X, f=3, filters=[128, 128, 512], stage=3, block="a", s=2)
    X = identity_block(X, f=3, filters=[128, 128, 512], stage=3, block="b")

    # 最后阶段
    # 平均池化
    X = AveragePooling2D(pool_size=(2, 2))(X)

    # 输出层
    X = Flatten()(X)  # 展平
    X = Dense(classes, activation="softmax", name="fc" + str(classes), kernel_initializer=glorot_uniform(seed=0))(X)

    # 创建模型
    model = Model(inputs=X_input, outputs=X, name="ResNet50")

    return model


def read_and_decode(filename):
    filename_queue = tf.train.string_input_producer([filename])
    reader = tf.TFRecordReader()
    _, serialized_example = reader.read(filename_queue)
    features = tf.parse_single_example(serialized_example, features={
        'label': tf.FixedLenFeature([], tf.int64),
        'img_raw': tf.FixedLenFeature([], tf.string)
    })

    img = tf.decode_raw(features['img_raw'], tf.uint8)
    img = tf.reshape(img, [64, 64, 3])
    img = tf.cast(img, tf.float32) * (1. / 255)

    label = tf.cast(features['label'], tf.int32)
    label = tf.one_hot(label, 12, 1, 0)
    return img, label

# 运行构建的模型图
model = ResNet50(input_shape=(64, 64, 3), classes=12)

# 编译模型来配置学习过程
model.compile(optimizer="adam", loss="categorical_crossentropy", metrics=["accuracy"])

# 加载数据集
image1, label1 = read_and_decode(train_name)
x_train, y_train = tf.train.batch([image1, label1], batch_size=TRAIN_SIZE, capacity=50000)

image2, label2 = read_and_decode(test_name)
x_test, y_test = tf.train.batch([image2, label2], batch_size=TEST_SIZE, capacity=50000)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    coord = tf.train.Coordinator()
    threads = tf.train.start_queue_runners(coord=coord)
    
    X_train, Y_train = sess.run([x_train, y_train])
    X_test, Y_test = sess.run([x_test, y_test])
    print("number of training examples = " + str(X_train.shape[0]))
    print("number of test examples = " + str(X_test.shape[0]))
    print("X_train shape: " + str(X_train.shape))
    print("Y_train shape: " + str(Y_train.shape))
    print("X_test shape: " + str(X_test.shape))
    print("Y_test shape: " + str(Y_test.shape))
    model.summary()
    for i in range(EPOCH):
        print('EPOCH '+str(i+1))
        model.fit(X_train, Y_train, epochs=1, batch_size=BATCH_SIZE, verbose=2, validation_split=0.001)
        # 测试集性能测试
        preds = model.evaluate(X_test, Y_test)
        print("    Loss = " + str(preds[0]))
        print("    Test Accuracy =" + str(preds[1]))    
    
    coord.request_stop()
    coord.join(threads)
    
