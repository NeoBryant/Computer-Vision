# coding: utf-8
try:
    import urllib.request
except ImportError:
    raise ImportError('You should use Python 3.x')
import os.path
import gzip
import pickle
import os
import numpy as np

dataset_dir = os.path.abspath(os.path.join(os.getcwd(), "..")) + "/data"
save_file = dataset_dir + "/mnist.pkl"

# 读取gz文件标签数据，并转化为numpy array类型
def _load_label(file_name):
    file_path = dataset_dir + "/" + file_name
    
    print("Converting " + file_name + " to NumPy Array ...")
    with gzip.open(file_path, 'rb') as f:
        labels = np.frombuffer(f.read(), np.uint8, offset=8)
    print("Done")
    
    return labels

# 读取gz文件图像数据，并转化为numpy array类型
def _load_img(file_name):
    img_size = 784
    file_path = dataset_dir + "/" + file_name
    
    print("Converting " + file_name + " to NumPy Array ...")    
    with gzip.open(file_path, 'rb') as f:
        data = np.frombuffer(f.read(), np.uint8, offset=16)
    data = data.reshape(-1, img_size)
    print("Done")
    
    return data
    
# 将数据集文件转化为numpy array
def _convert_numpy():
    dataset = {} # 字典str: str
    dataset['train_img'] =  _load_img('train-images-idx3-ubyte.gz')
    dataset['train_label'] = _load_label('train-labels-idx1-ubyte.gz')    
    dataset['test_img'] = _load_img('t10k-images-idx3-ubyte.gz')
    dataset['test_label'] = _load_label('t10k-labels-idx1-ubyte.gz')
    return dataset

def init_mnist(): # 读取gz文件另存为pkl文件
    dataset = _convert_numpy() # 读取数据集gz文件，并转化为numpy array类型
    print("Creating pickle file ...")
    with open(save_file, 'wb') as f: # 将数据集保存为pkl文件
        pickle.dump(dataset, f, -1) 
    print("Done!")

def load_mnist():
    """读入MNIST数据集
    Returns
    (训练图像, 训练标签), (测试图像, 测试标签)
    """
        
    with open(save_file, 'rb') as f:
        dataset = pickle.load(f)

    for key in ('train_img', 'train_label', 'test_img', 'test_label'):
        dataset[key] = dataset[key].astype('int32')

    return (dataset['train_img'], dataset['train_label']), (dataset['test_img'], dataset['test_label']) 


if __name__ == '__main__':
    init_mnist() # 读取gz文件另存为pkl文件
    