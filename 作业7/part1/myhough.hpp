#ifndef _MYHOUGH_HPP_
#define _MYHOUGH_HPP_

#include "CImg.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include <stack>
#include <algorithm>


using namespace cimg_library;
using namespace std;

const double PI = 3.1415926; //派
const unsigned char blue[] = {0,0,255}; //蓝色
const unsigned char red[] = {255,0,0}; //红色
const unsigned char green[] = {0,255,0};//绿色
class myhough {
public:
    myhough(CImg<unsigned char> ); //构造函数
    ~myhough(); //析构函数
    
    //直线
    void voting_line(); //检测每一个点的所有可能直线方程，并记录投票，以及最大值
    void localMax(double); //取局部极大值，设定阈值，过滤干扰直线（直线方程存储在lines中）
    void drawLine(); //绘制边缘直线
    void drawPoint(); //绘制直线交点

    vector<pair<int,int>> getPoints(){
        return points;
    }

private:
    CImg<unsigned char> img; //canny处理后的边缘图像
    CImg<unsigned char> hough_space; //直线霍夫空间
    CImg<unsigned char> img_src; //绘制线后的图像空间
    //string _mode;  //记录是直线检测还是圆形检测
    
    //直线
    int **hough_space_voting_line; //记录直线检测霍夫空间的点的投票数
    int max_voting_line; //最大投票数
    vector<pair<int,int>> lines; //存储选出的直线的r和theta
    vector<pair<int,int>> points; //存储选出的交点的坐标
    
};

// mode == 'line' -> line
myhough::myhough(CImg<unsigned char> src){ 
    //初始化
    img = src;
    //画线标点
    //由于边缘图和原图大小相差6个像素宽和高，所有这里将原图进行剪切
    CImg<unsigned char> img_temp = src;

    img_src = CImg<unsigned char>(img_temp._width-6,img_temp._height-6,1,3,0);
    for (int x = 0; x < img_src._width; ++x){
        for (int y = 0; y < img_src._height; ++y){
            img_src(x,y,0)=img_temp(x+3,y+3,0);
            img_src(x,y,1)=img_temp(x+3,y+3,1);
            img_src(x,y,2)=img_temp(x+3,y+3,2);
        }
    }
    cimg_forXY(img,x,y) {
        if (img(x,y) == 0) {
            img_src(x,y,0) = 0;
            img_src(x,y,1) = 0;
            img_src(x,y,2) = 0;
        } else {
            img_src(x,y,0) = 255;
            img_src(x,y,1) = 255;
            img_src(x,y,2) = 255;
        }
    }
    //img_src.display();
    
    //霍夫空间--初始化
    int width = img._width, height = img._height;
    int max_r = (int)sqrt((width/2)*(width/2)+(height/2)*(height/2));
    int max_hough_space_x = 2*max_r; //以x轴中点为真正的0点
    int max_hough_space_y = 629; //把theta分为628个刻度

    hough_space = CImg<unsigned char>(max_hough_space_x, max_hough_space_y);
    hough_space.fill(0);

    hough_space_voting_line = new int*[max_hough_space_y];
    for (int i = 0; i < max_hough_space_y; ++i){
        hough_space_voting_line[i] = new int[max_hough_space_x];
        for (int j = 0; j < max_hough_space_x; ++j){  //初始化每个元素为0
            hough_space_voting_line[i][j] = 0;
        }
    }
    
    //process
    cout << "start to process the img" << endl;
    voting_line(); //投票
    localMax(0.5); //局部极大值
    drawLine(); //画直线
    drawPoint(); //画交点
}

myhough::~myhough(){
    
    for (int i = 0; i < hough_space._height; ++i){
        delete[] hough_space_voting_line[i];
    }
    delete[] hough_space_voting_line;
}

void myhough::voting_line(){
    int cx = img._width/2, cy = img._height/2; //源图像真正的原点的坐标
    max_voting_line = 0; //最大投票数

    //投票计数
    for (int x = 0; x < img._width; ++x){
        for (int y = 0; y < img._height; ++y){
            if (img(x,y) != 0) { //对于边缘点
                for (int degree = 0; degree < hough_space._height; ++degree){
                    double theta = degree * PI / hough_space._height;
                    double r = (x-cx)*cos(theta)+(cy-y)*sin(theta); //转换坐标系后的新坐标(x',y')x=x-cx,y=cy-y
                    r += (hough_space._width/2); //原坐标下的的r值
                    hough_space((int)r,degree) = hough_space((int)r,(int)degree) + 1;
                    ++hough_space_voting_line[degree][(int)r];

                    if (max_voting_line < hough_space_voting_line[degree][(int)r]) { //更新最大投票数
                        max_voting_line = hough_space_voting_line[degree][(int)r];
                    }
                }
            }
        }
    }
    cout << "The max voting value is " << max_voting_line << endl;
    //display hough space
    //hough_space.display();
    cout << "image of hough space" << endl;
}

void myhough::localMax(double p) { //p是0-1之间的数
    //设置阈值
    int threshold = int(max_voting_line * p);
    cout << "the threshold is " << threshold << endl;

    for (int y = 0; y < hough_space._height; ++y) { 
        for (int x = 0; x < hough_space._width; ++x) {
            bool isNewLine = true; //判断是否为新线
            int temp_voting = hough_space_voting_line[y][x]; //临时变量，用于简化程序
            if (temp_voting > threshold) {
                for (int k = 0; k < lines.size(); ++k) {
                    //if (( abs(lines[k].second-y)<30 || abs(((int)hough_space._height-lines[k].second)+y)<30)  
                    if (( abs(lines[k].second-y)<30 || abs(lines[k].second+y-(int)hough_space._height)<30)  
                        && (abs(lines[k].first-x)<300 || abs(lines[k].first+x-(int)hough_space._width)<300)) { //局部比较difthetaa<15,difr<300
                            if (temp_voting > hough_space_voting_line[lines[k].second][lines[k].first]){ //更新极大值
                                lines[k].first = x;
                                lines[k].second = y;
                            }
                            isNewLine = false;
                    }
                }
                if (isNewLine) {
                    lines.push_back(make_pair(x,y)); //(r, theta)
                }
            }
        }
    }

    //show the r and theta of line
    /*cout << "there are " << lines.size() << " lines :" << endl;
    for (int i = 0; i < lines.size(); ++i) {
        //lines[i].second = lines[i].second * PI / hough_space._height; //更新theta的实际值
        cout << "(r, theta) = " << lines[i].first-(int)hough_space._width/2 
             << " " << lines[i].second * PI / hough_space._height << endl;
    }
    cout << "computed the lines" << endl;*/
}


//画线
void myhough::drawLine(){
    int cx = img_src._width/2, cy = img_src._height/2;
    for (int i = 0; i < lines.size(); ++i) {
        int r = lines[i].first-(int)hough_space._width/2;
        double theta = lines[i].second * PI / (int)hough_space._height;
        
        int px = r*cos(theta), py = r*sin(theta); //r与直线垂直的交点的新坐标系下的坐标
        int l = r*r; //r的平方
        int h = (int)img_src._height;
        int w = (int)img_src._width;
        //int b = r / sin(theta);
        //double k = sin(PI/2-theta)/cos(PI/2-theta);
        if (px != 0){ //非水平线
            int x1,y1,x2,y2;
            x1 = (2*l-h*py)/(2*px); //k<0
            y1 = h/2;
            x2 = (2*l+h*py)/(2*px);
            y2 = -1*h/2;
            img_src.draw_line(x1+cx,cy-y1,x2+cx,cy-y2,blue);
        } else {  //水平线
            img_src.draw_line(0,cy-py,cx*2,cy-py,blue);
        }
    }
    //img_src.display();
    cout << "draw all the lines" << endl;
}

void myhough::drawPoint() {
    vector<vector<int>> num_edges_8; //八邻域内的边缘直线的像素点数量
    int len_diagonal_img = (int)sqrt(img_src._height*img_src._height+img_src._width*img_src._width);
    double p = 0.01; //用于与len_diagonal_img相乘得到判断点相邻距离的阈值
    //遍历img_src，计算每个点的八邻域内的直线像素点
    for (int x = 0; x < img_src._width; ++x) { 
        vector<int> temp;
        for (int y = 0; y < img_src._height; ++y) {
            int counter = 0; //计数变量
            for (int i = x-1; i < x+2; ++i){ //八邻域循环
                for (int j = y-1; j < y+2; ++j){
                    if (i < 0 || j < 0 || i >= img_src._width || j >= img_src._height || (i==x && j==y)){
                        continue;
                    } else {
                        if (img_src(i,j,0)==blue[0]&&img_src(i,j,1)==blue[1]&&img_src(i,j,2)==blue[2]) { //直线像素点
                            ++counter;
                        }
                    }
                }
            }
            temp.push_back(counter);
        }
        num_edges_8.push_back(temp);
    }

    for (int x = 0; x < img_src._width; ++x) { 
        for (int y = 0; y < img_src._height; ++y) {
            if (num_edges_8[x][y] > 3) {
                bool isNewPoint=true;
                for (int k = 0; k < points.size(); ++k) {
                    int l = (int)sqrt((points[k].first-x)*(points[k].first-x)+(points[k].second-y)*(points[k].second-y)); //已经存储点到该点的距离
                    if (l < p*len_diagonal_img){
                        if (num_edges_8[x][y] > num_edges_8[points[k].first][points[k].second]) {
                            points[k].first = x;
                            points[k].second = y;
                        }
                        isNewPoint = false;
                    }
                }
                if (isNewPoint) {
                    points.push_back(make_pair(x,y));
                }
            }
        }
    }
    //red
    cimg_forXY(img,x,y) {
        if (img(x,y) == 255) {
            bool isBreak = false;
            for (int i = x-10; i < x+11; ++i){ //9*9-1邻域循环
                for (int j = y-10; j < y+11; ++j){
                    if (i < 0 || j < 0 || i >= img_src._width || j >= img_src._height) {
                        continue;
                    } else {
                        if (img_src(i,j,0)==blue[0]&&img_src(i,j,1)==blue[1]&&img_src(i,j,2)==blue[2]) {
                            img_src(x,y,0)=red[0];
                            img_src(x,y,1)=red[1];
                            img_src(x,y,2)=red[2];
                            isBreak = true;
                            break;
                        }
                    }
                }
                if (isBreak) {
                    break;
                }
            }
        }
    }
    //img_src.display();

    //show the oints of intersection
    cout << "there are " << points.size() << " points of intersection :" << endl;
    for (int i = 0; i < points.size(); ++i) {
        img_src.draw_circle(points[i].first,points[i].second,5,green);
        cout << "(x,y) = " << points[i].first << " " << points[i].second << endl;
    }
    //img_src.display(); //display
    cout << "computed the points" << endl;
}



#endif