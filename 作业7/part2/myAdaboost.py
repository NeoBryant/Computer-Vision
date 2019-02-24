# coding: utf-8

import os
import numpy as np
import time

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

if __name__ == "__main__":
    # adaboost
    n_est = 20
    print("弱分类器数为: ", n_est)

    model = AdaBoostClassifier(DecisionTreeClassifier(max_depth=2, min_samples_split=20, min_samples_leaf=5),
                            algorithm="SAMME", n_estimators=n_est, learning_rate=0.5)

    dataset_dir = os.path.abspath(os.path.join(os.getcwd(), "..")) + "/model"
    save_dir = dataset_dir + '/model.pkl'

    if(os.path.isfile(save_dir)): # 读取模型
        model = joblib.load(save_dir) # 加载模型
    else:
	    train_mnist(model, save_dir) # 训练模型
    
    (x_train, t_train), (x_test, t_test) = load_mnist() # 加载训练集、测试集
    print("测试集识别精确度:", model.score(x_test, t_test))

    