#include <iostream>
#include <cstring>
#include <vector>
#include "CImg.h"
#include "MyImageWarping.hpp"
#include "myhough.hpp"

using namespace std;
using namespace cimg_library;


class MyOtsu
{
private:
    /* data */
    CImg<unsigned char> srcImg; //源图像
    CImg<unsigned char> grayscaled; // Grayscale
    CImg<unsigned char> edgeImg; //边缘图像
    CImg<unsigned char> tempImg;
    CImg<unsigned char> dstImg; //目标图像

    double histogram[256]; //灰度直方图

    char* file_name; //图片文件名
public:
    MyOtsu(const char*);
    ~MyOtsu();

    CImg<unsigned char> toGrayScale();//change all pixel into grayscale

    CImg<unsigned char> imageSegmentation(); //图像分割
    double getBetweenClassVarirance(double, int); //计算类间方差

    CImg<unsigned char> imageWarping(); //矫正A4纸
    vector<pair<int,int>> getPoints(); //求A4纸4个角点的坐标
};

MyOtsu::MyOtsu(const char* file_name){
    strcpy(this->file_name, file_name);
    //cout << this->file_name << endl;
    cout << "1" << endl;
    srcImg.load_bmp(file_name); //加载源图像
    tempImg = srcImg;
    cout << "2" << endl;
    srcImg = srcImg.get_blur(10);
    cout << "3" << endl;
    toGrayScale(); //转换为灰度图
    cout << "4" << endl;
    imageSegmentation(); //图像分割，并腐蚀得到边缘图
    cout << "5" << endl;
    imageWarping(); //A4纸矫正
    cout << "6" << endl;
}

MyOtsu::~MyOtsu(){

}

//将图片转化为灰度图，并生成灰度直方图，再正规化
CImg<unsigned char> MyOtsu::toGrayScale(){
    double MN = 0; //灰度值总和

    //初始化灰度直方图
    for (int i = 0; i < 256; ++i){
        histogram[i] = 0;
    }

    grayscaled.assign(srcImg.width(), srcImg.height()); //单通道
    //图像处理
    cimg_forXY(grayscaled, x, y){
        int r = srcImg(x,y,0);
        int g = srcImg(x,y,1);
        int b = srcImg(x,y,2);
        double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
        grayscaled(x,y) = (unsigned char)(newValue);

        histogram[int(grayscaled(x,y))] += 1; //灰度直方图
    }
    //计算MN
    for (int i = 0; i < 256; ++i){
        MN += histogram[i];
    }
    //正规化
    for (int i = 0; i < 256; ++i){
        histogram[i] = histogram[i] / MN;
    }
    return grayscaled;
}

CImg<unsigned char> MyOtsu::imageSegmentation(){
    double mG = 0; //mean global intensity
    double sigma2B, sigma2G = 0; //类间方差，全局方差
    double maxSigma2B = 0; //sigma2B的最大值
    int T = 0; //最优阈值
    const CImg<float> structEle = CImg<float>(3, 3, 1, 1, 1);//结构元
    //计算mG
    for (int i = 0; i < 256; ++i){
        mG += i * histogram[i];
    }
    //计算sigma2G，即全局方差
    for (int i = 0; i < 256; ++i){
        sigma2G += (i-mG)*(i-mG)*histogram[i];
    }
    //遍历k找最优阈值
    for (int i = 0; i < 256; ++i){
        sigma2B = getBetweenClassVarirance(mG, i);
        if (sigma2B > maxSigma2B){
            maxSigma2B = sigma2B;
            T = i;
        }
    }
    cout << "T = " << T << ", ratio = " << maxSigma2B/sigma2G << endl;
    //图像分割
    CImg<unsigned char> edgeImg;
    edgeImg.assign(srcImg.width(), srcImg.height()); //单通道
    cimg_forXY(edgeImg, x, y){
        //edgeImg(x,y) = (unsigned char)(0) ? (unsigned char)(255) : grayscaled(x,y) < T;
        edgeImg(x,y) = (unsigned char)(0) ? (unsigned char)(255) : grayscaled(x,y) > T;
    }
    //edgeImg.display(); //display the img after Otsu
    //用腐蚀处理提取边缘
    edgeImg = edgeImg - edgeImg.get_erode(structEle);
    edgeImg.display();
    return edgeImg;
}


double MyOtsu::getBetweenClassVarirance(double mG, int k){
    double sigma2B = 0; //类间方差
    
    //计算P1(k)、m(k)
    double P1 = 0; //robability of the class C1
    double m = 0; //the mean intensity up to the k level
    for (int i = 0; i <= k; ++i){
        P1 += histogram[i];
        m += i*histogram[i];
    }

    sigma2B = ((mG*P1-m)*(mG*P1-m))/(P1*(1-P1));

    return sigma2B;
}

CImg<unsigned char> MyOtsu::imageWarping(){
    vector<pair<int,int>> point = getPoints(); //求得A4纸4个角点

    MyImageWarping myimagewarping; //A4矫正
    dstImg = myimagewarping.run(tempImg, point);

    dstImg.display();

    return dstImg; //????????????待修改
}


vector<pair<int,int>> MyOtsu::getPoints(){
    myhough hough(edgeImg);

    vector<pair<int,int>> point = hough.getPoints(); //A4纸4个角的坐标（x，y）
    /*if (strcmp(file_name, "../data/A4file0.bmp") == 0){
        point.push_back(make_pair(756, 612));
        point.push_back(make_pair(2763, 779));
        point.push_back(make_pair(684, 3549));
        point.push_back(make_pair(2634, 3513));
    } else if (strcmp(file_name, "../data/A4file1.bmp") == 0){
        point.push_back(make_pair(281, 259));
        point.push_back(make_pair(2444, 346));
        point.push_back(make_pair(90, 3450));
        point.push_back(make_pair(2467, 3464));
    } else if (strcmp(file_name, "../data/A4file2.bmp") == 0){
        point.push_back(make_pair(267, 2247));
        point.push_back(make_pair(805, 714));
        point.push_back(make_pair(2350, 2957));
        point.push_back(make_pair(2861, 1524));      
    } else if (strcmp(file_name, "../data/A4file3.bmp") == 0){
        point.push_back(make_pair(624, 530));
        point.push_back(make_pair(2892, 610));
        point.push_back(make_pair(698, 3695));
        point.push_back(make_pair(2785, 3590));
    } else if (strcmp(file_name, "../data/A4file4.bmp") == 0){   
        point.push_back(make_pair(148, 2599));
        point.push_back(make_pair(1025, 991));
        point.push_back(make_pair(2289, 3675));
        point.push_back(make_pair(3032, 2197));  
    } else if (strcmp(file_name, "../data/A4file5.bmp") == 0){ 
        point.push_back(make_pair(653, 801));
        point.push_back(make_pair(2603, 925));
        point.push_back(make_pair(655, 3637));
        point.push_back(make_pair(2504, 3515));     
    } else {
        cout << "wrong file name" << endl;
    }*/
    
    cout << "The four points are: " << endl;
    for (int i = 0; i < 4; ++i){
        cout << "point " << i+1 << ": (" << point[i].first << ", " << point[i].second << ")" << endl;
    }
    
    return point;
}
