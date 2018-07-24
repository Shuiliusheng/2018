import cv2 as cv
import os
import numpy as np

dir = "g:\\asm\\train\\"
dir1 = "g:\\asm\\train_pro\\"
[H, W] = [64, 64]

setdir = ["Black-grass", "Charlock", "Cleavers", "Common Chickweed", "Common wheat", "Fat Hen",
          "Loose Silky-bent", "Maize", "Scentless Mayweed", "Shepherds Purse",
          "Small-flowered Cranesbill", "Sugar beet"]

def get_green(pic_name):
    img = cv.imread(pic_name)
    # cv.imshow("test1",img)
    img = cv.resize(img, (H, W), interpolation=cv.INTER_CUBIC)
    [height, width, channel] = img.shape
    for i in range(height):
        for j in range(width):
            [b, g, r] = img[i, j]
            if not (g > b and g > r and r > b):
                img[i, j] = [0, 0, 0]
    # cv.imshow("test",img)
    cv.waitKey(0)
    return img


def translation_img(img, x, y):
    M = np.float32([[1, 0, x], [0, 1, y]])
    shifted = cv.warpAffine(img, M, (img.shape[1], img.shape[0]))
    return shifted


def rotate_img(img, angle):
    center = (W / 2, H / 2)
    M = cv.getRotationMatrix2D(center, angle, 1.0)
    rotated = cv.warpAffine(img, M, (W, H))
    return rotated


def process_dir_pic(dir, dir1):
    num = 0
    for subdir in setdir:
        src_dir = dir + subdir
        dst_dir = dir1 + subdir
        for f in os.listdir(src_dir):
            print(str(num) + " " + src_dir + "\\" + f)
            img = get_green(src_dir + "\\" + f)
            cv.imwrite(dst_dir + "\\" + str(num) + "_01.png", img)
            # rotate img
            img90 = rotate_img(img, 90)
            cv.imwrite(dst_dir + "\\" + str(num) + "_02.png", img90)
            img180 = rotate_img(img, 180)
            cv.imwrite(dst_dir + "\\" + str(num) + "_03.png", img180)
            # translation img
            img2 = translation_img(img, W / 4, H / 4)  # right
            cv.imwrite(dst_dir + "\\" + str(num) + "_04.png", img2)
            num = num + 1


def process_test_dir(src_dir, dst_dir):
    for f in os.listdir(src_dir):
        print(src_dir + "\\" + f)
        img = get_green(src_dir + "\\" + f)
        # img = cv.resize(img, (H, W), interpolation=cv.INTER_CUBIC)
        cv.imwrite(dst_dir + "\\" + f, img)

process_test_dir('test', 'test_pro')

# process_dir_pic(dir, dir1)
