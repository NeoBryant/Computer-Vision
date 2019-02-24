#ifndef _MYHOUGH_HPP_
#define _MYHOUGH_HPP_

#include "CImg.h"
#include "mycanny.hpp"
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
    myhough(const char*, string mode); //构造函数
    ~myhough(); //析构函数
    
    //直线
    void voting_line(); //检测每一个点的所有可能直线方程，并记录投票，以及最大值
    void localMax(double); //取局部极大值，设定阈值，过滤干扰直线（直线方程存储在lines中）
    void drawLine(); //绘制边缘直线
    void drawPoint(); //绘制直线交点

    //圆
    void voting_circle(); //检测圆的对应的参数空间，并投票
    void drawCircle(int); //画圆和圆心


private:
    CImg<unsigned char> img; //canny处理后的边缘图像
    CImg<unsigned char> hough_space; //直线霍夫空间
    CImg<unsigned char> img_src; //绘制线后的图像空间
    string _mode;  //记录是直线检测还是圆形检测
    
    //直线
    int **hough_space_voting_line; //记录直线检测霍夫空间的点的投票数
    int max_voting_line; //最大投票数
    vector<pair<int,int>> lines; //存储选出的直线的r和theta
    vector<pair<int,int>> points; //存储选出的交点的坐标

    
    //圆
    vector<pair<int,int>> circles; //存储选出的圆的圆心a和b
    int minR; //最小半径
    int maxR; //最大半径
    CImg<float> houghImage; //记录圆形检测霍夫空间的点的投票数
    vector<pair<int, int>> center; // 存放累加值最大的圆心对应坐标
    vector<int> circleWeight; // 累加矩阵
};

// mode == 'line' -> line ,  == 'circle' -> circle
myhough::myhough(const char* filename, string mode){ 
    //初始化
    _mode = mode;
    mycanny mcny(filename);
    img = mcny.getCannyResult();
    
    //画线标点
    //由于边缘图和原图大小相差6个像素宽和高，所有这里将原图进行剪切
    CImg<unsigned char> img_temp = mcny.getSrcImg();
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
    
    if (mode == "line") { //直线检测
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
    } else if (mode == "circle") { //圆形检测        
        
        //设置minR和maxR的值
        minR = 50; //最小能识别的圆
        maxR = int(img._width<img._height ? img._width/2 : img._height/2);

        //process
        voting_circle();
    }
}

myhough::~myhough(){
    if (_mode == "line") {
        for (int i = 0; i < hough_space._height; ++i){
            delete[] hough_space_voting_line[i];
        }
        delete[] hough_space_voting_line;
    }
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
    hough_space.display();
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

    //若直线多于4条过滤多余的边缘直线
    //---------------------waiting for coding---------------------------------//

    //show the r and theta of line
    cout << "there are " << lines.size() << " lines :" << endl;
    for (int i = 0; i < lines.size(); ++i) {
        //lines[i].second = lines[i].second * PI / hough_space._height; //更新theta的实际值
        cout << "(r,theta) = " << lines[i].first-(int)hough_space._width/2 
             << " " << lines[i].second * PI / hough_space._height << endl;
    }
    cout << "computed the lines" << endl;
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
    img_src.display();
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
    img_src.display();

    //show the oints of intersection
    cout << "there are " << points.size() << " points of intersection :" << endl;
    for (int i = 0; i < points.size(); ++i) {
        img_src.draw_circle(points[i].first,points[i].second,5,green);
        cout << "(x,y) = " << points[i].first << " " << points[i].second << endl;
    }
    img_src.display(); //display
    cout << "computed the points" << endl;
}



void myhough::voting_circle(){
    int width = img._width, height = img._height;
    int max = 0;
    vector<pair<int,int>> vote_set; //记录最大投票数和对应的r

    vector<pair<int,int>> centers_set; //候选圆心

    //将0-359度对应的cos和sin值存储为数组,为了后面减少计算时间
    vector<double> Cos, Sin;
    for (int i = 0; i < 360; ++i) {
        Cos.push_back(cos(1.0*i*PI/180));
        Sin.push_back(sin(1.0*i*PI/180));
    }

    cout << "the minR and maxR is " << minR << " " << maxR << endl;
    for (int r = minR; r < maxR ; ++r) { //遍历每一种可能的 r
        max = 0; //当前r的最大投票数
        //投票记录数组
		houghImage = CImg<float>(width, height);
		houghImage.fill(0);
        cimg_forXY(img, x, y) {  //进行遍历投票
			int value = img(x, y);
			if (value != 0) {
				for (int i = 0; i < 360; i++) { //用360度，而不是弧度值
					int x0 = x + r * Cos[i]; //(x0,y0)为圆的中心的估计点
					int y0 = y + r * Sin[i];
					/*进行voting投票*/
					if (x0 > 0 && x0 < width && y0 > 0 && y0 < height) {
						houghImage(x0, y0)++; //当前r的
                        //voting_circle_counter(x0,y0)++; //所有r的
					}
				}
			}
		}
        //对每一个r，找到最大投票数及其坐标（x0，y0）并记录
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (houghImage(x, y) > max) {
					max = houghImage(x, y);
				}
			}
		}
        vote_set.push_back(make_pair(max,r));
    } 
    cout << "the size of vote_set is " << vote_set.size() << endl;
    //排序 以 最大投票数
	sort(vote_set.begin(), vote_set.end(), [](const pair<int, int>& x, const pair<int, int>& y) -> int {
		return x.first > y.first;
	});

    //检测圆心数量
    /*double p = 0.03; 
    for (int i = 0; i < int(vote_set.size()*p); ++i){
        CImg<float> voting_circle_counter = CImg<float>(width, height);; //记录所有r下的边缘投票数目
        voting_circle_counter.fill(0);

        cimg_forXY(img, x, y){
            if (img(x,y) != 0) {
                int x0 = x + vote_set[i].second * Cos[i]; //(x0,y0)为圆的中心的估计点
                int y0 = y + vote_set[i].second * Sin[i];
                if (x0 > 0 && x0 < width && y0 > 0 && y0 < height) {
                    ++voting_circle_counter(x0,y0);
                }
            }
        }
        vector<pair<int, int>> _circles;
        vector<int> _circleWeight;
        cimg_forXY(houghImage, x, y) {
            if (houghImage(x, y) != 0) {
                _circles.push_back(make_pair(x, y));
                _circleWeight.push_back(houghImage(x, y));
            }
        } 
    }*/

    //voting_circle_counter.display();
    /*cout << "the number of circle is " << centers_set.size() << endl;
    for (int i = 0; i < centers_set.size(); ++i) {
        cout << "center (x,y) = " << centers_set[i].first << " " << centers_set[i].second << endl;
    }*/

    int circleNumber = 2; //待修改********************************
    cout << endl
         << "the number of circle is " << circleNumber 
         << endl;
	for (int i = 0; i < circleNumber; i++) {  //对于每一个圆
		houghImage = CImg<float>(width, height);
		houghImage.fill(0);

		cimg_forXY(img, x, y) {
			int value = img(x, y);
			if (value != 0) {
				for (int j = 0; j < 360; j++) {
					int x0 = x + vote_set[i].second * Cos[j];
					int y0 = y + vote_set[i].second * Sin[j];
					//进行voting投票
					if (x0 > 0 && x0 < width && y0 > 0 && y0 < height) {
						houghImage(x0, y0)++;
					}
				}
			}
		}
		cout << "The radius is " << vote_set[i].second << endl;
		//houghCirclesDetect();
        //将霍夫图像中所有不为0的点对应圆心的坐标存入数组
        cimg_forXY(houghImage, x, y) {
            if (houghImage(x, y) != 0) {
                circles.push_back(make_pair(x, y));
                circleWeight.push_back(houghImage(x, y));
            }
        }        
        
		drawCircle(vote_set[i].second); //一个个的画圆
        cout << "draw " << i+1 << " circles " << endl;
	}
    img_src.display();

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
    img_src.display();
}

void myhough::drawCircle(int r){
    cout << "start drawing circle -- ";
    int width = img_src._width, height = img_src._height;
    int count = 0;

    vector<int> sortCircleWeight = circleWeight;
    // 将累加矩阵从大到小进行排序
	sort(sortCircleWeight.begin(), sortCircleWeight.end(), [](const int& x, const int& y) -> int {
		return x > y;
	});

    while (1){
		int weight = sortCircleWeight[count];
        int index;
		vector<int>::iterator iter = find(circleWeight.begin(), circleWeight.end(), weight);
		index = iter - circleWeight.begin();
		int a = circles[index].first, b = circles[index].second;
		count++;

		int i;
		for (i = 0; i < center.size(); i++) {
			if (sqrt(pow((center[i].first - a), 2) + pow((center[i].second - b), 2)) < minR) {
				break; // 判断检测出来的圆心坐标是否跟已检测的圆心坐标的距离，如果距离过小，默认是同个圆
			}
		}
		if (i == center.size()) {
			center.push_back(make_pair(a, b));
			img_src.draw_circle(a, b, r, blue, 5.0f, 1);
            cout << "center is (" << a << "," << b << "), r is " << r << endl;
			break;
		} 
    }
}


#endif