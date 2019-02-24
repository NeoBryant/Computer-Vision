#ifndef _MYCANNY_H_
#define _MYCANNY_H_

#include "CImg.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include<stack>

using namespace std;
using namespace cimg_library;

class mycanny {
public:
    mycanny(const char*);

    CImg<unsigned char> toGrayScale();//change all pixel into grayscale
    vector<vector<double>> createFilter(int, int, double);//Creates a gaussian filter
    CImg<unsigned char> useFilter(CImg<unsigned char>, vector<vector<double>>);  //Use some filter
    CImg<unsigned char> sobel(); //Sobel filtering
    CImg<unsigned char> nonMaxSupp(); //Non-maxima supp.
    CImg<unsigned char> threshold(CImg<unsigned char>, int, int); //Use some filter

    CImg<unsigned char> deleteEdges(CImg<unsigned char>); //delete the edges whose length is less than 20
    bool isTip(CImg<unsigned char> imgin, int x, int y); //if the point is the tip of a edge return true

private:
    CImg<unsigned char> img;
    CImg<unsigned char> grayscaled; // Grayscale
    CImg<unsigned char> gFiltered; // Gradient
    CImg<unsigned char> sFiltered; //Sobel Filtered
    CImg<float> angles; //Angle Map
    CImg<unsigned char> non; // Non-maxima supp.
    CImg<unsigned char> thres; //Double threshold and final
    
    CImg<unsigned char> edgesDeleted; //Double threshold and final

};

//Constructor
//@param const char* filename : name of the image, path will be auto concatenated to "img/filename.bmp". 
mycanny::mycanny(const char* filename){
    img.load_bmp(filename); //load the test data img

    vector<vector<double>> filter = createFilter(3, 3, 1);
    //Print filter
    for (int i = 0; i<filter.size(); i++) {
        for (int j = 0; j<filter[i].size(); j++) {
            cout << filter[i][j] << " ";
        }
    }
    cout << "gFilter created" << endl;

    
    grayscaled = CImg<unsigned char>(toGrayScale()); //Grayscale the image
    cout << "image converted to grayscale" << endl;

    gFiltered = CImg<unsigned char>(useFilter(grayscaled, filter)); //Gaussian Filter
    cout << "image filtered with Gaussian filter" << endl;

    sFiltered = CImg<unsigned char>(sobel()); //Sobel Filter
    cout << "image filtered with sobel filter" << endl;

    non = CImg<unsigned char>(nonMaxSupp()); //Non-Maxima Suppression
    cout << "image filtered with non-maxima Suppression" << endl;

    thres = CImg<unsigned char>(threshold(non, 77, 95)); //Double Threshold and Finalize
    //thres = CImg<unsigned char>(threshold(non, 77, 95)); //Double Threshold and Finalize
    cout << "image filtered with double thresholding" << endl;

    edgesDeleted = CImg<unsigned char>(deleteEdges(thres));
    cout << "image deleted some short edges" << endl;

    //display
    /*img.display();
    grayscaled.display();
    gFiltered.display();
    sFiltered.display();
    non.display();
    thres.display();
    edgesDeleted.display();*/
    CImgDisplay img_disp(img, "Original"),
                    grayscaled_disp(grayscaled,"GrayScaled"),
                    gFiltered_disp(gFiltered, "Gaussian Blur"),
                    sFiltered_disp(sFiltered, "Sobel Filtered"),
                    non_disp(non, "Non-Maxima Supp."),
                    thres_disp(thres, "Final"),
                    edgesDeleted_disp(edgesDeleted, "edgesDeleted");
    while (!img_disp.is_closed()
            || !grayscaled_disp.is_closed()
            || !gFiltered_disp.is_closed()
            || !sFiltered_disp.is_closed()
            || !non_disp.is_closed()
            || !thres_disp.is_closed()
            || !edgesDeleted_disp.is_closed()) {
    }

}

//converte the image into grayscale
CImg<unsigned char> mycanny::toGrayScale(){
    grayscaled.assign(img.width(), img.height()); //To one channel
    //图像处理
    cimg_forXY(grayscaled, x, y){
        int r = img(x,y,0);
        int g = img(x,y,1);
        int b = img(x,y,2);
        double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);
        grayscaled(x,y) = (unsigned char)(newValue);
    }
    return grayscaled;
}

//Creates a gaussian filter
//@param int row: 
vector<vector<double>> mycanny::createFilter(int row, int column, double sigmaIn){
	vector<vector<double>> filter;
	for (int i = 0; i < row; i++) {
        vector<double> col;
        for (int j = 0; j < column; j++) {
            col.push_back(-1);
        }
		filter.push_back(col);
	}
	float coordSum = 0;
	float constant = 2.0 * sigmaIn * sigmaIn;

	// Sum is for normalization
	float sum = 0.0;
	for (int x = - row/2; x <= row/2; x++) {
		for (int y = -column/2; y <= column/2; y++) {
			coordSum = (x*x + y*y);
			filter[x + row/2][y + column/2] = (exp(-(coordSum) / constant)) / (M_PI * constant);
			sum += filter[x + row/2][y + column/2];
		}
	}
	// Normalize the Filter
	for (int i = 0; i < row; i++)
        for (int j = 0; j < column; j++)
            filter[i][j] /= sum;

	return filter;   
}

//Use some filter
CImg<unsigned char> mycanny::useFilter(CImg<unsigned char> img_in, vector<vector<double>> filterIn){
    int size = (int)filterIn.size()/2;
    gFiltered = CImg<unsigned char>(img_in.width() - 2*size, img_in.height() - 2*size);
	for (int i = size; i < img_in.width() - size; i++) {
		for (int j = size; j < img_in.height() - size; j++) {
			double sum = 0;
			for (int x = 0; x < filterIn.size(); x++) {
				for (int y = 0; y < filterIn.size(); y++) {
                    sum += filterIn[x][y] * (double)(img_in(i + y - size, j + x - size));
				}
            }
            gFiltered(i-size, j-size) = sum;
		}
	}
    return gFiltered;
}

//Sobel filtering
CImg<unsigned char> mycanny::sobel(){
    //Sobel X Filter
    double x1[] = {-1.0, 0, 1.0};
    double x2[] = {-2.0, 0, 2.0};
    double x3[] = {-1.0, 0, 1.0};

    vector< vector<double> > xFilter(3);
    xFilter[0].assign(x1, x1+3);
    xFilter[1].assign(x2, x2+3);
    xFilter[2].assign(x3, x3+3);
    
    //Sobel Y Filter
    double y1[] = {1.0, 2.0, 1.0};
    double y2[] = {0, 0, 0};
    double y3[] = {-1.0, -2.0, -1.0};
    
    vector<vector<double>> yFilter(3);
    yFilter[0].assign(y1, y1+3);
    yFilter[1].assign(y2, y2+3);
    yFilter[2].assign(y3, y3+3);
    
    //Limit Size
    int size = (int)xFilter.size()/2;
    
    sFiltered = CImg<unsigned char>(gFiltered.width() - 2*size, gFiltered.height() - 2*size);
    
    angles = CImg<unsigned char>(gFiltered.width() - 2*size, gFiltered.height() - 2*size); //AngleMap

	for (int i = size; i < gFiltered.height() - size; i++)
	{
		for (int j = size; j < gFiltered.width() - size; j++)
		{
			double sumx = 0;
            double sumy = 0;
            
			for (int x = 0; x < xFilter.size(); x++)
				for (int y = 0; y < xFilter.size(); y++)
				{
                    sumx += xFilter[x][y] * (double)(gFiltered(j + y - size, i + x - size)); //Sobel_X Filter Value
                    sumy += yFilter[x][y] * (double)(gFiltered(j + y - size, i + x - size)); //Sobel_Y Filter Value
				}
            double sumxsq = sumx*sumx;
            double sumysq = sumy*sumy;
            
            double sq2 = sqrt(sumxsq + sumysq);
            
            if(sq2 > 255) //Unsigned Char Fix
                sq2 =255;
            sFiltered(j-size, i-size) = sq2;
 
            if(sumx==0) //Arctan Fix
                angles(j-size, i-size) = 90;
            else
                angles(j-size, i-size) = atan(sumy/sumx);
		}
	}
    return sFiltered;
}

//Non-maxima supp.
CImg<unsigned char> mycanny::nonMaxSupp(){
    non = CImg<unsigned char>(sFiltered.width()-2, sFiltered.height()-2);
    for (int i=1; i< sFiltered.width() - 1; i++) {
        for (int j=1; j<sFiltered.height() - 1; j++) {
            float Tangent = angles(i,j) * 57.296f;
            // cout << Tangent << ' ';
            non(i-1, j-1) = sFiltered(i,j);
            //Horizontal Edge
            if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i+1,j)) || (sFiltered(i,j) < sFiltered(i-1,j)))
                    non(i-1, j-1) = 0;
            }
            //Vertical Edge
            if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i,j+1)) || (sFiltered(i,j) < sFiltered(i,j-1)))
                    non(i-1, j-1) = 0;
            }
            
            //-45 Degree Edge
            if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i+1,j+1)) || (sFiltered(i,j) < sFiltered(i-1,j-1)))
                    non(i-1, j-1) = 0;

            }
            
            //45 Degree Edge
            if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((22.5 < Tangent) && (Tangent <= 67.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i-1,j+1)) || (sFiltered(i,j) < sFiltered(i+1,j-1)))
                    non(i-1, j-1) = 0;
            }
        }
    }
    return non;
}

//Use some filter
CImg<unsigned char> mycanny::threshold(CImg<unsigned char> imgin,int low, int high){
    if(low > 255)
        low = 255;
    if(high > 255)
        high = 255;
    
    thres = CImg<unsigned char>(imgin.width(), imgin.height());
    for (int i=0; i<imgin.width(); i++) {
        for (int j = 0; j<imgin.height(); j++) {
            thres(i,j) = imgin(i,j);
            if(thres(i,j) > high)
                thres(i,j) = 255;
            else if(thres(i,j) < low)
                thres(i,j) = 0;
            else {
                bool anyHigh = false;
                bool anyBetween = false;
                for (int x=i-1; x < i+2; x++)  {
                    for (int y = j-1; y<j+2; y++) {
                        //Wang Note: a missing "x" in Hasan's code.
                        if(x <= 0 || y <= 0 || x > thres.width() || y > thres.height()) //Out of bounds
                            continue;
                        else {
                            if(thres(x,y) > high) {
                                thres(i,j) = 255;
                                anyHigh = true;
                                break;
                            }
                            else if(thres(x,y) <= high && thres(x,y) >= low)
                                anyBetween = true;
                        }
                    }
                    if(anyHigh)
                        break;
                }
                if(!anyHigh && anyBetween)
                    for (int x=i-2; x < i+3; x++) {
                        for (int y = j-1; y<j+3; y++) {
                            if(x < 0 || y < 0 || x > thres.width() || y > thres.height()) //Out of bounds
                                continue;
                            else {
                                if(thres(x,y) > high) {
                                    thres(i,j) = 255;
                                    anyHigh = true;
                                    break;
                                }
                            }
                        }
                        if(anyHigh)
                            break;
                    }
                if(!anyHigh)
                    thres(i,j) = 0;
            }
        }
    }
    return thres;
}

// 使用非递归dfs
CImg<unsigned char> mycanny::deleteEdges(CImg<unsigned char> imgin) {
    //const int depth = 20; 

    edgesDeleted = imgin;
    //edgesDeleted = CImg<unsigned char>(threshold(non, 100, 120));
    //return edgesDeleted;
    int width = imgin.width(), height = imgin.height();
    //vector<bool> notShort(width*height, false); 
    for (int i=0; i<width; i++) {
        for (int j = 0; j<height; j++) {
            if (isTip(edgesDeleted,i,j)) {
                bool isMoreThan20 = false; //判断dfs深度是否到达20
                //非递归 dfs 判断深度是否能到20 ,若不能，则删除edges
                vector<bool> visited(width*height, false); //标记访问结点 y*width+x
                stack<int> stk; //栈
                stk.push(j*width+i);
                visited[j*width+i] =true;

                int depth = 0; //初始化深度
                while (!stk.empty()) { //dfs 循环
                    int x = stk.top()%width, y = stk.top()/width;
                    bool hasAdj = false; //是否有未访问过的邻近点
                    for (int ax = -1; ax < 2; ++ax) {
                        bool isBreak = false;
                        for (int ay = -1; ay < 2; ++ay) {
                            if (!(ax+x<0 || ay+y<0 || ax+x>width || ay+y>height)) { //在图片范围内
                                if (edgesDeleted(ax+x,ay+y)==255 && ax!=0 && ay!=0) { //不是自己
                                    int point = (ay+y)*width+ax+x;
                                    if (!visited[point]){ //该点未访问过
                                        hasAdj=true; //存在未访问的邻接结点
                                        visited[point] = true;
                                        stk.push(point);
                                        ++depth; //增加深度
                                        isBreak = true; //标记退出外层for循环
                                        break; 
                                    }
                                }
                            }
                        }
                        if (isBreak) {
                            break;
                        }
                    }
                    if (!hasAdj) { 
                        stk.pop();
                        --depth; //减小深度
                    }
                    if (depth >= 10){ //若深度可以到达20，即有边缘线长超过20
                        isMoreThan20 = true;
                        break; //退出dfs
                    }
                }
                if (isMoreThan20) {
                    //...
                } else {
                    for (int k = 0; k < width*height; ++k){
                        if (visited[k]) {
                            int x = k%width, y = k/width;
                            edgesDeleted(x,y) = 0; //删除边缘线
                        }
                    }
                }
            }
        }
    }
    
    return edgesDeleted;
}


//判断一个像素点是否为边缘线的端点/尖点
bool mycanny::isTip(CImg<unsigned char> imgin, int x, int y) {
    
    int width = imgin.width(), height = imgin.height();
    int count = 0; //统计8邻域内边缘点数量, count==2->true
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            if (!(x+i<0 || y+j<0 || x+i>=width || y+j>=height)) {
                if (imgin(x+i, y+j)==255) {
                    ++count;
                }
                if (count>=2){
                    return false;
                }
            }
        }
    }
    return true;
}

#endif