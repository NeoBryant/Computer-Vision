# coding: utf-8

import os
import csv
import struct
import numpy as np
import time
import xlwt

from PIL import Image
from sklearn.ensemble import AdaBoostClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.externals import joblib

from load_mnist import load_mnist

'''
(x_train, t_train), (x_test, t_test) = load_mnist()
'''

# 训练
def train_mnist(model, save_file):
    (x_train, t_train), (x_test, t_test) = load_mnist() # 加载训练集、测试集
    
    
    start_time = time.time()
    print("开始训练模型..")
    model.fit(x_train, t_train) # 训练模型

    end_time = time.time()
    print("训练结束! 耗时:", end_time-start_time, "s")
    
    joblib.dump(model, save_file) # 保存模型


# 读bmp图片，转换为numpy形式
def read_img(file_name):
    with Image.open(file_name) as image:
        # img -> numpy
        img = np.array(image) # 转化为numpy
        gray_img = np.zeros(shape=(28, 28))
        # rgb转化为单通道
        for i in range(28):
            for j in range(28):
                r, g, b = img[i][j][0], img[i][j][1], img[i][j][2],
                gray_img[i][j] = r * 0.2126 + g * 0.7152 + b * 0.0722

        gray_img = gray_img.flatten()  # 降维
        
    return np.array([gray_img])


if __name__ == "__main__":
    # adaboost
    n_est = 160
    print("弱分类器数为: ", n_est)

    model = AdaBoostClassifier(DecisionTreeClassifier(max_depth=2, min_samples_split=20, min_samples_leaf=5),
                            algorithm="SAMME", n_estimators=n_est, learning_rate=0.5)

    dataset_dir = os.path.abspath(os.path.join(os.getcwd(), "..")) + "/model"
    save_dir = dataset_dir + '/model.pkl'

    if(os.path.isfile(save_dir)): # 读取模型
        model = joblib.load(save_dir) # 加载模型
    else:
	    train_mnist(model, save_dir) # 训练模型

    #-----------------------------------------------------
    workbook = xlwt.Workbook(encoding = 'utf-8')
    worksheet = workbook.add_sheet('Predict')
    #worksheet.write(0, 0, label = 'Row 0, Column 0 Value')
    worksheet.write(0, 0, "文件名")
    worksheet.write(0, 1, "角点1")
    worksheet.write(0, 2, "角点2")
    worksheet.write(0, 3, "角点3")
    worksheet.write(0, 4, "角点4")
    worksheet.write(0, 5, "学号")
    worksheet.write(0, 6, "手机号")
    worksheet.write(0, 7, "身份证号")
    workbook.save('../result/excel/result.xls')
    fileNum = 0 # 文件计数

    #------------------------------------------------------
    print('Reading images directory from txt file...')
    imgdir = "../result/imgdir.txt"
    imageDir = [] # 记录需要识别的数字图片的文件夹地址
    with open(imgdir) as f:
        imagedir = f.readline()
        while imagedir:
            imagedir = imagedir.strip('\n')
            imageDir.append(imagedir)
            imagedir = f.readline()
    # print(imageDir)
    print('Done! Reading images from directory one by one...')
    for Dir in imageDir:
        print(Dir)
        fileNum += 1 # 
        worksheet.write(fileNum, 0, Dir.split("/")[-1]+".bmp") # 写入excel
        # 角点位记录
        count = 1
        with open(Dir+"/corner.txt") as F:
            corner = F.readline()
            while corner:
                corner = corner.strip('\n')
                #print(corner)
                if count <= 4:
                    worksheet.write(fileNum, count, corner)
                count += 1
                corner = F.readline()

        images = [] # 记录每张图片对应要识别的数字子图片
        with open(Dir+"/imagelist.txt") as F:
            image = F.readline()
            while image:
                image = image.strip('\n')
                images.append(image)
                image = F.readline()
            
        results = ""
        final_results = []
        count = 5 # 记录学号、手机号、身份证号
        for i in images: # 对于每一张数字图片进行识别
            if i != "*":
                filename = Dir + '/' + i
                img = Image.open(filename)
                img = img.resize((28,28))
                arr = (np.array(img))[:,:,:1].reshape(1, 784)

                res = model.predict(arr)[0] # 进行识别
                #print(res)
                results = results + str(res)  # 每一行的预测结果
            else:
                # print(results)
                if count <= 7:
                    worksheet.write(fileNum, count, results)
                final_results.append(results)
                results = ""
                count += 1

    
    workbook.save('../result/excel/result.xls')