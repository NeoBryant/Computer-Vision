#include "CImg.h"
#include <cmath>
#include <iostream>
//编译时，后缀 -O2 -lgdi32
using namespace cimg_library;
using namespace std;

const unsigned char blue[] = {0, 0, 255};
const unsigned char yellow[] = {255, 255, 0};
const float PI = 3.14159;

class TestCimg{
public:
    TestCimg(){}
    ~TestCimg(){}

    //读入“1.bmp”并显示
    CImg<unsigned char> readAndDisplay_hw1(const char* filename);

    //把“1.bmp”白色变红色，绿色变蓝色
    CImg<unsigned char> changeColor_hw2();

    //用cimg相关函数画圆1
    CImg<unsigned char> draw_circle_hw3();
    //不用cimg相关函数画圆1
    CImg<unsigned char> draw_circle_nocimg_hw3();

    //用cimg相关函数画圆2
    CImg<unsigned char> draw_circle_hw4();
    //不用cimg相关函数画圆2
    CImg<unsigned char> draw_circle_nocimg_hw4();

    //用cimg相关函数画直线
    CImg<unsigned char> draw_line_hw5();
    //不用cimg相关函数画直线
    CImg<unsigned char> draw_line_nocimg_hw5();

    //保存文件为“2.bmp”
    CImg<unsigned char> save_file_hw6(const char* filename);
    
    
private:
    CImg<unsigned char> SrcImg; //使用cimg函数操作
    CImg<unsigned char> tempSrcImg; //不用cimg函数操作
};

CImg<unsigned char> TestCimg::readAndDisplay_hw1(const char* filename){
    
    //读取bmp图像
    SrcImg.load_bmp(filename);
    tempSrcImg = SrcImg;

    //图像显示
    SrcImg.display();
    return SrcImg;
}

CImg<unsigned char> TestCimg::changeColor_hw2(){
    //图像处理
    cimg_forXY(SrcImg, x, y){
        //白色区域变红
        if (SrcImg(x,y,0)==255 && SrcImg(x,y,1)==255 && SrcImg(x,y,2)==255){
            SrcImg(x,y,0) = 255;
            SrcImg(x,y,1) = 0;
            SrcImg(x,y,2) = 0;
        }
        //黑色区域变绿
        if (SrcImg(x,y,0)==0 && SrcImg(x,y,1)==0 && SrcImg(x,y,2)==0){
            SrcImg(x,y,0) = 0;
            SrcImg(x,y,1) = 255;
            SrcImg(x,y,2) = 0;
        }
    }

    //图像显示
    SrcImg.display();
    return SrcImg;
}

CImg<unsigned char> TestCimg::draw_circle_hw3(){
    //unsigned char blue[] = {0, 0, 255}; //颜色是RGB数组
    SrcImg.draw_circle(50, 50, 30, blue);
    //图像显示
    SrcImg.display();
    return SrcImg;
}

CImg<unsigned char> TestCimg::draw_circle_nocimg_hw3(){
    //遍历函数画园
    cimg_forXY(tempSrcImg, x, y){
        //画蓝圆
        if ((x-50)*(x-50)+(y-50)*(y-50)<=30*30){
            tempSrcImg(x,y,0) = blue[0];
            tempSrcImg(x,y,1) = blue[1];
            tempSrcImg(x,y,2) = blue[2];
        }
    }

    //图像显示
    tempSrcImg.display();
    return tempSrcImg;
}


CImg<unsigned char> TestCimg::draw_circle_hw4(){
    //unsigned char yellow[] = {255, 255, 0}; //颜色是RGB数组
    SrcImg.draw_circle(50, 50, 3, yellow);
    //图像显示
    SrcImg.display();
    return SrcImg;
}

CImg<unsigned char> TestCimg::draw_circle_nocimg_hw4(){
    //遍历函数画园
    cimg_forXY(tempSrcImg, x, y){
        //画黄圆
        if ((x-50)*(x-50)+(y-50)*(y-50)<=3*3){
            tempSrcImg(x,y,0) = yellow[0];
            tempSrcImg(x,y,1) = yellow[1];
            tempSrcImg(x,y,2) = yellow[2];
        }
    }

    //图像显示
    tempSrcImg.display();
    return tempSrcImg;
}

CImg<unsigned char> TestCimg::draw_line_hw5(){
    //unsigned char blue[] = {0, 0, 255}; //颜色是RGB数组
    
    int x = round(100*cos(PI*35/180)); //82
    int y = round(100*sin(PI*35/180)); //57
    
    SrcImg.draw_line(0, 0, x, y, blue);
    //图像显示
    SrcImg.display();
    return SrcImg;
}

CImg<unsigned char> TestCimg::draw_line_nocimg_hw5(){    
    //遍历函数画园，dda算法 
    cimg_forXY(tempSrcImg, x, y){
        //画黄圆
        if (x <= round(100*cos(PI*35/180)) && y==round(x*tan(PI*35/180))){
            tempSrcImg(x,y,0) = blue[0];
            tempSrcImg(x,y,1) = blue[1];
            tempSrcImg(x,y,2) = blue[2];
        }
    }

    //图像显示
    tempSrcImg.display();
    return tempSrcImg;
}

CImg<unsigned char> TestCimg::save_file_hw6(const char* filename){
    //图像保存
    SrcImg.save(filename);
    return SrcImg;
}