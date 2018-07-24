import numpy as np
import tensorflow as tf
from PIL import Image

src_path = './jpg/'


def write_train_data_to_tf(resize=[120, 120]):
    TFwriter = tf.python_io.TFRecordWriter("./train.tfrecords")
    step = 1
    # train.txt 保存着训练集的图片标号
    with open(src_path + '/train.txt') as f:
        for line in f:
            x = int(line)
            fname = src_path + 'image_' + ("%.4i" % x) + ".jpg"
            img_raw = Image.open(fname).resize(resize)
            imgRaw = img_raw.tobytes()
            print(step, len(imgRaw))
            example = tf.train.Example(features=tf.train.Features(feature={
                "label": tf.train.Feature(int64_list=tf.train.Int64List(value=[int(x / 80)])),
                'img_raw': tf.train.Feature(bytes_list=tf.train.BytesList(value=[imgRaw]))}))
            step += 1
            TFwriter.write(example.SerializeToString())
        TFwriter.close()


def write_test_data_to_tf(resize=[120, 120]):
    TFwriter = tf.python_io.TFRecordWriter("./test.tfrecords")
    step = 1
    # test.txt 保存着测试集的图片标号
    with open(src_path + '/test.txt') as f:
        for line in f:
            x = int(line)
            fname = src_path + 'image_' + ("%.4i" % x) + ".jpg"
            img_raw = Image.open(fname).resize(resize)
            imgRaw = img_raw.tobytes()
            print(step, len(imgRaw))
            example = tf.train.Example(features=tf.train.Features(feature={
                "label": tf.train.Feature(int64_list=tf.train.Int64List(value=[int(x / 80)])),
                'img_raw': tf.train.Feature(bytes_list=tf.train.BytesList(value=[imgRaw]))}))
            step += 1
            TFwriter.write(example.SerializeToString())
        TFwriter.close()


write_train_data_to_tf()
write_test_data_to_tf()
