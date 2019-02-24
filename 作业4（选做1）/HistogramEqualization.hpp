#ifndef HISTOGRAMEQUALIZATION_HPP
#define HISTOGRAMEQUALIZATION_HPP

#include "CImg.h"
#include <cmath>
#include <iostream>

#define PI 3.14159

using namespace std;
using namespace cimg_library;

class HistogramEqualization 
{
private:
    /* data */
    CImg<unsigned char> srcImg; //输入的源图像
    CImg<unsigned char> grayscale; //转化后的灰度图
    CImg<unsigned char> grayscale_equ; //均衡化后的灰度图

    CImg<float> rgb; //rgb图的值
    CImg<float> lab; //lab图是值

    CImg<float> source; //源图像
    CImg<float> target; //目标图像
    CImg<float> result; //结果图像

public:
    HistogramEqualization(const char*); //均衡化
    HistogramEqualization(const char*, const char*); //颜色转换
    ~HistogramEqualization();

    CImg<unsigned char> toGrayScale(); //转换为灰度图
    void generateEqualizationGrayscale(); //生成灰度直方图均衡化
    void generateEqualizationImg();//生成亮度均衡化
    void rgb2lab(); //RGB图像转lab图像
    void lab2rgb(); //lab图像转rgb图像

    void colorTransferbylab(); //颜色转换
};

//构造函数
//通过图像所在路径加载图像
HistogramEqualization::HistogramEqualization(const char* filename) 
{
    //默认先处理rgb图像的灰度图
    srcImg.load_bmp(filename); //load the test data img
    toGrayScale(); //converte the image into grayscale
    grayscale_equ.assign(srcImg.width(), srcImg.height()); //生成灰度均衡化图
    generateEqualizationGrayscale(); //生成灰度均衡化图
    
    //rgb-lab
    rgb=(CImg<float>)srcImg;
    lab=(CImg<float>)srcImg;
    generateEqualizationImg(); //生成rgb均衡化
}

HistogramEqualization::HistogramEqualization(const char* filename1, const char* filename2) {
    CImg<unsigned char> s,t;

    s.load_bmp(filename1);
    t.load_bmp(filename2);

    source = (CImg<float>)s;
    target = (CImg<float>)t;
    result = source;
    source.display();
    target.display();
    colorTransferbylab();
}

//析构函数
HistogramEqualization::~HistogramEqualization() { 

}

//converte the image into grayscale
CImg<unsigned char> HistogramEqualization::toGrayScale(){
    grayscale.assign(srcImg.width(), srcImg.height()); //To one channel
    //图像处理
    cimg_forXY(grayscale, x, y){
        int r = srcImg(x,y,0);
        int g = srcImg(x,y,1);
        int b = srcImg(x,y,2);
        double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
        grayscale(x,y) = (unsigned char)(newValue);
    }
    return grayscale;
}


//生成均衡化后的灰度图，并display
void HistogramEqualization::generateEqualizationGrayscale(){
    const int grayscaleSeries = 256; //灰度级数，默认为256
    double probabilityHistogram[grayscaleSeries] = {0}; //灰度概率直方图
    double cdfHistogram[grayscaleSeries] = {0}; //累计分布直方图

    int width = grayscale._width, height = grayscale._height; //灰度图的宽和高
    int numPixel = width*height; //像素数量
    
    cimg_forXY(grayscale, x, y){
        ++probabilityHistogram[grayscale(x,y)];  //统计灰度直方图
    }
    for (int i = 0; i < grayscaleSeries; ++i){ //统计cdf累计分布直方图
        if (i == 0) {
            cdfHistogram[i] = probabilityHistogram[i];
        } else {
            cdfHistogram[i] = cdfHistogram[i-1] + probabilityHistogram[i];
        }
    }

    cimg_forXY(grayscale_equ, x, y){ //均衡化的图像
        double newValue = round((cdfHistogram[grayscale(x,y)]-cdfHistogram[0])/(numPixel-cdfHistogram[0])*(grayscaleSeries-1));
        grayscale_equ(x,y) = (unsigned char)(newValue);
    }

    //---------------display--------------------
    grayscale.display();
    grayscale_equ.display();
} 

//生成均衡化后的灰度图，并display
void HistogramEqualization::generateEqualizationImg(){
    CImg<unsigned char> image = srcImg;
    int width = image._width, height = image._height; //灰度图的宽和高
    int totalPixel = width*height; //像素数量
    float pixelsR[256];
    float pixelsG[256];
    float pixelsB[256];
    memset(&pixelsR, 0, 256*sizeof(float));
    memset(&pixelsG, 0, 256*sizeof(float));
    memset(&pixelsB, 0, 256*sizeof(float));
    
    float probabilityR[256];
    float probabilityG[256];
    float probabilityB[256];
    
    float newPR[256];
    float newPG[256];
    float newPB[256];
    cimg_forXY(image, x, y) {
        pixelsR[int(image(x, y, 0))]++;
        pixelsG[int(image(x, y, 1))]++;
        pixelsB[int(image(x, y, 2))]++;
    }
    for(int i = 0; i < 256; i++) {
        probabilityR[i] = pixelsR[i] / totalPixel;
        probabilityG[i] = pixelsG[i] / totalPixel;
        probabilityB[i] = pixelsB[i] / totalPixel;
        if(i == 0) {
            newPR[i] = probabilityR[i];
            newPG[i] = probabilityG[i];
            newPB[i] = probabilityB[i];
        } else {
            newPR[i] = probabilityR[i] + newPR[i-1];
            newPG[i] = probabilityG[i] + newPG[i-1];
            newPB[i] = probabilityB[i] + newPB[i-1];
        }
    }
    CImg<unsigned char> color = image;
    cimg_forXY(image, x, y) {
        int tempR = image(x, y, 0);
        int tempG = image(x, y, 1);
        int tempB = image(x, y, 2);
        tempR = int(newPR[tempR] * 255 + 0.5);
        tempG = int(newPG[tempG] * 255 + 0.5);
        tempB = int(newPB[tempB] * 255 + 0.5);
        color(x, y, 0) = tempR;
        color(x, y, 1) = tempG;
        color(x, y, 2) = tempB;
    }
    srcImg.display();
    color.display("color");   
    //--------------display---------------
} 

void HistogramEqualization::rgb2lab(){//RGB图像转lab图像

    cimg_forXY(rgb, x, y) {
        float r = rgb(x, y, 0);
        float g = rgb(x, y, 1);
        float b = rgb(x, y, 2);

        float L = 0.3811 * r + 0.5783 * g + 0.0402 * b;
        float M = 0.1967 * r + 0.7244 * g + 0.0782 * b;
        float S = 0.0241 * r + 0.1288 * g + 0.8444 * b;

        if(L == 0) L = 1;
        if(M == 0) M = 1;
        if(S == 0) S = 1;

        L = log(L);
        M = log(M);
        S = log(S);

        float l =  (1.0/sqrt(3)) * (L + M + S);
        float alpha = 1.0/sqrt(6) * L + 1.0/sqrt(6) * M - 2.0/sqrt(6) * S;
        float beta = 1.0/sqrt(2) * L - 1.0/sqrt(2) * M;

        lab(x, y, 0) = l;
        lab(x, y, 1) = alpha;
        lab(x, y, 2) = beta;
    }
} 

void HistogramEqualization::lab2rgb(){//lab图像转rgb图像
    CImg<unsigned char> img_equ;
    cimg_forXY(lab, x, y) {
        float L = sqrt(3)/3.0*lab(x, y, 0) + sqrt(6)/6.0*lab(x, y, 1) + sqrt(2)/2.0*lab(x, y, 2);
        float M = sqrt(3)/3.0*lab(x, y, 0) + sqrt(6)/6.0*lab(x, y, 1) - sqrt(2)/2.0*lab(x, y, 2);
        float S = sqrt(3)/3.0*lab(x, y, 0) - sqrt(6)/3.0*lab(x, y, 1);
        L = pow(2.71828, L);
        M = pow(2.71828, M);
        S = pow(2.71828, S);

        img_equ(x, y, 0) =(unsigned char)(4.4679 * L - 3.5873 * M + 0.1193 * S);
        img_equ(x, y, 1) =(unsigned char)(-1.2186 * L + 2.3809 * M - 0.1624 * S);
        img_equ(x, y, 2) =(unsigned char)(0.0497 * L - 0.2439 * M + 1.2045 * S);
    }
} 

//颜色转换
void HistogramEqualization::colorTransferbylab(){
    float sum1[3], sum2[3], sum1_squ[3], sum2_squ[3];
    float mean1[3], mean2[3], vari1[3], vari2[3];

    int width1 = source._width, height1 = source._height;
    int total1 = width1*height1;

    int width2 = target._width, height2 = target._height;
    int total2 = width2*height2;

    for (int i = 0; i < 3; i++) {
        sum1[i] = sum2[i] = sum1_squ[i] = sum2_squ[i] = 0;
        mean1[i] = mean2[i] = vari1[i] = vari2[i] = 0;
    }

    // RGB ----->lab
    cimg_forXY(source, x, y) {
        float r = source(x, y, 0);
        float g = source(x, y, 1);
        float b = source(x, y, 2);

        float L = 0.3811 * r + 0.5783 * g + 0.0402 * b;
        float M = 0.1967 * r + 0.7244 * g + 0.0782 * b;
        float S = 0.0241 * r + 0.1288 * g + 0.8444 * b;

        if(L == 0) L = 1;
        if(M == 0) M = 1;
        if(S == 0) S = 1;

        L = log(L);
        M = log(M);
        S = log(S);

        float l =  (1.0/sqrt(3)) * (L + M + S);
        float alpha = 1.0/sqrt(6) * L + 1.0/sqrt(6) * M - 2.0/sqrt(6) * S;
        float beta = 1.0/sqrt(2) * L - 1.0/sqrt(2) * M;

        result(x, y, 0) = l;
        result(x, y, 1) = alpha;
        result(x, y, 2) = beta;
        //cout << l << ", " << alpha << ", " << beta << endl;
        sum1[0] += l;
        sum1[1] += alpha;
        sum1[2] += beta;
    }

    mean1[0] = sum1[0] / total1;
    mean1[1] = sum1[1] / total1;
    mean1[2] = sum1[2] / total1;

    cimg_forXY(result, x, y) {
        sum1_squ[0] += (result(x, y, 0) - mean1[0]) * (result(x, y, 0) - mean1[0]);
        sum1_squ[1] += (result(x, y, 1) - mean1[1]) * (result(x, y, 1) - mean1[1]);
        sum1_squ[2] += (result(x, y, 2) - mean1[2]) * (result(x, y, 2) - mean1[2]);
    }

    vari1[0] = sqrt(sum1_squ[0] / total1);
    vari1[1] = sqrt(sum1_squ[1] / total1);
    vari1[2] = sqrt(sum1_squ[2] / total1);
    // target RGB ------->lab
    cimg_forXY(target, x, y) {
        float r = target(x, y, 0);
        float g = target(x, y, 1);
        float b = target(x, y, 2);

        float L = 0.3811 * r + 0.5783 * g + 0.0402 * b;
        float M = 0.1967 * r + 0.7244 * g + 0.0782 * b;
        float S = 0.0241 * r + 0.1288 * g + 0.8444 * b;

        if(L == 0) L = 1;
        if(M == 0) M = 1;
        if(S == 0) S = 1;

        L = log(L);
        M = log(M);
        S = log(S);

        float l =  (1.0/sqrt(3)) * (L + M + S);
        float alpha = 1.0/sqrt(6) * L + 1.0/sqrt(6) * M - 2.0/sqrt(6) * S;
        float beta = 1.0/sqrt(2) * L - 1.0/sqrt(2) * M;

        target(x, y, 0) = l;
        target(x, y, 1) = alpha;
        target(x, y, 2) = beta;

        sum2[0] += l;
        sum2[1] += alpha;
        sum2[2] += beta;
    }

    mean2[0] = sum2[0] / total2;
    mean2[1] = sum2[1] / total2;
    mean2[2] = sum2[2] / total2;

    cimg_forXY(target, x, y) {
        sum2_squ[0] += (target(x, y, 0) - mean2[0]) * (target(x, y, 0) - mean2[0]);
        sum2_squ[1] += (target(x, y, 1) - mean2[1]) * (target(x, y, 1) - mean2[1]);
        sum2_squ[2] += (target(x, y, 2) - mean2[2]) * (target(x, y, 2) - mean2[2]);
    }

    vari2[0] = sqrt(sum2_squ[0] / total2);
    vari2[1] = sqrt(sum2_squ[1] / total2);
    vari2[2] = sqrt(sum2_squ[2] / total2);

    //transfer between images
    cimg_forXY(result, x, y) {
        result(x, y, 0) = (result(x, y, 0) - mean1[0]) * vari2[0] / vari1[0] + mean2[0];
        result(x, y, 1) = (result(x, y, 1) - mean1[1]) * vari2[1] / vari1[1] + mean2[1];
        result(x, y, 2) = (result(x, y, 2) - mean1[2]) * vari2[2] / vari1[2] + mean2[2];
    }

    // lab -----> RGB
    cimg_forXY(target, x, y) {
        float L = sqrt(3)/3 * target(x, y, 0) + sqrt(6)/6 * target(x, y, 1) + sqrt(2)/2 * target(x, y, 2);
        float M = sqrt(3)/3 * target(x, y, 0) + sqrt(6)/6 * target(x, y, 1) - sqrt(2)/2 * target(x, y, 2);
        float S = sqrt(3)/3 * target(x, y, 0) - sqrt(6)/3 * target(x, y, 1);
        L = pow(2.71828, L);
        M = pow(2.71828, M);
        S = pow(2.71828, S);
        target(x, y, 0) = 4.4679 * L - 3.5873 * M + 0.1193 * S;
        target(x, y, 1) = -1.2186 * L + 2.3809 * M - 0.1624 * S;
        target(x, y, 2) = 0.0497 * L - 0.2439 * M + 1.2045 * S;
    }

    cimg_forXY(result, x, y) {
        float L = sqrt(3)/3 * result(x, y, 0) + sqrt(6)/6 * result(x, y, 1) + sqrt(2)/2 * result(x, y, 2);
        float M = sqrt(3)/3 * result(x, y, 0) + sqrt(6)/6 * result(x, y, 1) - sqrt(2)/2 * result(x, y, 2);
        float S = sqrt(3)/3 * result(x, y, 0) - sqrt(6)/3 * result(x, y, 1);
        L = pow(2.71828, L);
        M = pow(2.71828, M);
        S = pow(2.71828, S);
        result(x, y, 0) = 4.4679 * L - 3.5873 * M + 0.1193 * S;
        result(x, y, 1) = -1.2186 * L + 2.3809 * M - 0.1624 * S;
        result(x, y, 2) = 0.0497 * L - 0.2439 * M + 1.2045 * S;
    }
    cout << "tranverse finished" << endl;

    result.display();
}


#endif