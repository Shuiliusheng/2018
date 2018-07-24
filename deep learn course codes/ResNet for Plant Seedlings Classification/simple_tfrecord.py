import os
import tensorflow as tf
from PIL import Image

src_path = 'g:/asm/train/'

setdir = ["Black-grass", "Charlock", "Cleavers", "Common Chickweed", "Common wheat", "Fat Hen",
          "Loose Silky-bent", "Maize", "Scentless Mayweed", "Shepherds Purse",
          "Small-flowered Cranesbill", "Sugar beet"]


def write_train_data_to_tf(resize=[64, 64]):
    TFwriter = tf.python_io.TFRecordWriter(src_path + "train_single_prc.tfrecords")
    TFwriter_valid = tf.python_io.TFRecordWriter(src_path + "valid_single.tfrecords")
    label = 0
    valid = -1
    for subdir in setdir:
        src_dir = 'g:/asm/train_pro/' + subdir
        num = 0
        for img_name in os.listdir(src_dir):
            print(src_dir + '/' + img_name, label)
            img_raw = Image.open(src_dir + '/' + img_name).resize(resize)
            img_raw = img_raw.convert('RGB')
            imgRaw = img_raw.tobytes()
            example = tf.train.Example(features=tf.train.Features(feature={
                "label": tf.train.Feature(int64_list=tf.train.Int64List(value=[label])),
                'img_raw': tf.train.Feature(bytes_list=tf.train.BytesList(value=[imgRaw]))}))
            if num<valid :
                TFwriter_valid.write(example.SerializeToString())
            else :
                TFwriter.write(example.SerializeToString())
            num = num + 1
        label = label + 1
    TFwriter.close()
    TFwriter_valid.close()


def read_label(name):
    label = []
    pic_name = []
    fp = open(name, 'r')
    try:
        lines = fp.readlines()  # 读取出全部数据，按行存储
    finally:
        fp.close()
    for line in lines:
        line_list = line.split(',')  # 默认以空格为分隔符对字符串进行切片
        pic_name.append(line_list[0])
        line_list1 = line_list[1].split('\n')
        label.append(setdir.index(line_list1[0]))
    return pic_name, label

def write_test_data_to_tf(src_dir, resize=[64, 64]):
    pic_name,label = read_label('sub.csv')
    TFwriter = tf.python_io.TFRecordWriter("test_prc_64.tfrecords")
    for img_name in os.listdir(src_dir):
        index = label[pic_name.index(img_name)]
        print(src_dir + '/' + img_name, index)
        img_raw = Image.open(src_dir + '/' + img_name).resize(resize)
        img_raw = img_raw.convert('RGB')
        imgRaw = img_raw.tobytes()
        example = tf.train.Example(features=tf.train.Features(feature={
            "label": tf.train.Feature(int64_list=tf.train.Int64List(value=[index])),
            'img_raw': tf.train.Feature(bytes_list=tf.train.BytesList(value=[imgRaw]))}))
        TFwriter.write(example.SerializeToString())
    TFwriter.close()


# write_test_data_to_tf('test_pro')
write_train_data_to_tf()



