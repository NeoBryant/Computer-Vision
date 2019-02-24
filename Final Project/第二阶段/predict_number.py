# coding: utf-8

import os
import csv
import struct
import numpy as np
import time
import xlwt
import xlrd

from PIL import Image
from sklearn.metrics import accuracy_score
from sklearn.neural_network import MLPClassifier
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

    # 查看测试集识别精度
    print(model.predict(x_test))
    print(accuracy_score(t_test, model.predict(x_test)))

if __name__ == "__main__":
    # 神经网络

    model = MLPClassifier(hidden_layer_sizes=(100, 100), activation='logistic', 
				solver='sgd', learning_rate_init=0.001, max_iter=200, verbose = True)

    dataset_dir = os.path.abspath(os.path.join(os.getcwd(), "..")) + "/model"
    save_dir = dataset_dir + '/model.pkl'

    if(os.path.isfile(save_dir)): # 读取模型
        model = joblib.load(save_dir) # 加载模型
    else:
	    train_mnist(model, save_dir) # 训练模型

    (x_train, t_train), (x_test, t_test) = load_mnist() # 加载训练集、测试集
    print("mnist 测试集识别精确率： ", accuracy_score(t_test, model.predict(x_test)))


    # 读取labels.xlsx
    data = xlrd.open_workbook('../result/excel/labels.xlsx')
    table = data.sheets()[0]          #通过索引顺序获取
    #labels = table.row_values(1)
    #print(str(int(labels[0])))
    #print(str(int(labels[1])))
    #print(labels[2])

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
    worksheet.write(0, 8, "检测结果") # y：全部正确，1：学号有误，2：手机号有误，3：身份证号有误
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
    for Dir in imageDir: # 对于每一张A4纸图片
        print(Dir)
        fileNum += 1 # 要读取的图片序号
        labels = table.row_values(fileNum) # 读取excel：labels.xlsx 
        worksheet.write(fileNum, 0, Dir.split("/")[-1]+".bmp") # 写入excel，result.xlsx
        isRight = "" # 记录识别结果是否正确

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
                    if count == 5:
                        if len(results) > 8: 
                            results = results[:8] # 防止最终位数不对
                        if str(int(labels[0])) != results: # 若识别不正确
                            isRight += "1"
                    if count == 6:
                        if len(results) > 11 : 
                            results = results[:11]
                        if str(int(labels[1])) != results:# 若识别不正确
                            isRight += "2"
                    if count == 7:
                        if len(results) > 18 : 
                            results = results[:18]
                        if labels[2] != results: # 若识别不正确
                            isRight += "3"
                    worksheet.write(fileNum, count, results)
                final_results.append(results)
                results = ""
                count += 1
        # 记录识别结果准确性
        worksheet.write(fileNum, 8, isRight)

    
    workbook.save('../result/excel/result.xls')

    