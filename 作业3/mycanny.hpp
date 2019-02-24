#ifndef _MYCANNY_HPP_
#define _MYCANNY_HPP_

#include "CImg.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>
#include <stack>

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
    CImg<unsigned char> getSrcImg(); //返回为处理的源图像
    CImg<unsigned char> getCannyResult(); //返回处理后的边缘点图

private:
    CImg<unsigned char> img;
    CImg<unsigned char> grayscaled; // Grayscale
    CImg<unsigned char> gFiltered; // Gradient
    CImg<unsigned char> sFiltered; //Sobel Filtered
    CImg<float> angles; //Angle Map
    CImg<unsigned char> non; // Non-maxima supp.
    CImg<unsigned char> thres; //Double threshold and final
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

    thres = CImg<unsigned char>(threshold(non, 90, 100)); //Double Threshold and Finalize
    cout << "image filtered with double thresholding" << endl;

    //display
/*    img.display();
    grayscaled.display();
    gFiltered.display();
    sFiltered.display();
    non.display();*/
    thres.display();


    /*
    CImgDisplay img_disp(img, "Original"), 
            thres_disp(thres, "AfterCanny");

    while (!thres_disp.is_closed()) {
        thres_disp.wait();
    }*/
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
//>high为强边缘点，low-high为弱边缘点
CImg<unsigned char> mycanny::threshold(CImg<unsigned char> imgin,int low, int high)
{
    if(low > 255)
        low = 255;
    if(high > 255)
        high = 255;
    
    thres = CImg<unsigned char>(imgin._width, imgin._height);
    
    for (int i=0; i<imgin._width; i++) 
    {
        for (int j = 0; j<imgin._height; j++) 
        {
            //printf("(%d,%d)",i,j);
            thres(i,j) = imgin(i,j);
            if(thres(i,j) > high) {
                thres(i,j) = 255; //白，即边缘点
            } else if(thres(i,j) < low) {
                thres(i,j) = 0; //黑
            } else { //处理弱边缘点
                bool anyHigh = false;
                bool anyBetween = false;
                for (int x=i-1; x < i+2; x++) {  //这个循环的作用是寻找八邻域内是否有强边缘点或弱边缘点
                    for (int y = j-1; y<j+2; y++) { //(x,y)为八邻域的边缘点
                         //Wang Note: a missing "x" in Hasan's code.
                        if(x < 0 || y < 0 || x >= thres._width || y >= thres._height || (x==i&&y==j)) {//Out of bounds
                            continue; //在图片有效范围内，且不是该点
                        } else {
                            if(thres(x,y) > high) { //八邻域内有强边缘点，则该点变为强边缘点，退出寻找
                                thres(i,j) = 255;
                                anyHigh = true;
                                break;
                            } else if(thres(x,y) <= high && thres(x,y) >= low) { //八邻域内有弱边缘点，则标记
                                anyBetween = true;
                            }
                        }
                    }
                    if(anyHigh) { //若八邻域内有强边缘点，则退出寻找强边缘点
                        break;
                    }
                }
                if(!anyHigh && anyBetween) {//若八邻域内没有强边缘点，但有弱边缘点
                    for (int x=i-2; x < i+3; x++) { //扩大查找范围为24邻域
                        for (int y = j-2; y<j+3; y++) { //bug：原来为j-1！
                            if(x < 0 || y < 0 || x >= thres._width || y >= thres._height){ //Out of bounds
                                continue;
                            } else {
                                if(thres(x,y) > high) {//若找到强边缘点
                                    thres(i,j) = 255;
                                    anyHigh = true;
                                    break;
                                }
                            }
                        }
                        if(anyHigh) {//若24邻域内有强边缘点，则退出寻找强边缘点
                            break;
                        }
                    }
                }
                if(!anyHigh) { //若没有强边缘点，也没有弱边缘点，则把该点不是边缘点
                    thres(i,j) = 0;
                }
            }
        }
    }
    return thres;
}

CImg<unsigned char> mycanny::getSrcImg(){
    return img;
}

CImg<unsigned char> mycanny::getCannyResult(){
    return thres;
}

#endif