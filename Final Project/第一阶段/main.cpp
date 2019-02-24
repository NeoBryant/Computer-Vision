#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <direct.h>

#include "mycanny.hpp"
#include "myhough.hpp"
#include "myImageWarping.hpp"
#include "myImageSegmentation.hpp"

using namespace std;

/*
usage(windows):
    g++ -o main.exe main.cpp -O2 -lgdi32
*/

int main() {

	string filepath = "../testdata/"; // 测试图像文件夹
	
    //vector<string> filenames = {"15331353.bmp"}; // 读取图像文件名
    vector<string> filenames = {"15331029.bmp", "15331046.bmp", "15331052.bmp", "15331180.bmp", "15331341.bmp", 
                            "15331344.bmp", "15331348.bmp", "15331351.bmp", "15331353.bmp", "15331364.bmp"}; 
	
    ofstream out("../result/imgdir.txt", ios::app);

    for (int i = 0; i < filenames.size(); i++) {
		string filename = filepath + filenames[i];
		cout << filename << endl;

        // Canny边缘检测
        mycanny c;
        CImg<float> canny_result = c.run(filename);
        int scale = c.getScale();
        //canny_result.display("after canny");

        // 霍夫变换
        myhough h;
        CImg<float> hough_result = h.run(canny_result, scale);
        //hough_result.display("after hough");
        vector<pair<int, int> > corner = h.getCorner();
        //角点写入txt中
        ofstream cornerTxt("../result/numbers/"+filenames[i].substr(0,8)+"/corner.txt");
        for (int i = 0; i < corner.size(); ++i){
            cornerTxt << corner[i].first << "," << corner[i].second << endl;
        }
        cornerTxt.close();

        // A4纸校正
        myImageWarping warp;
        CImg<float> warping_result = warp.run(filename, corner);
        //warping_result.display("after warping");
        string warpingpath = "../result/warping/"; // 存放校正后的图像 
        warpingpath = warpingpath + filenames[i];
        //warping_result.save(warpingpath.c_str());

        // 存储文件夹路径，用于python读取后进行识别工作
        string sni = "../result/";
        //sni = sni + filenames[i].substr(0,8); 
        cout << sni.c_str() << endl;
        if (out.is_open()) {
            out << sni + "numbers/" + filenames[i].substr(0,8) << endl;
        }
        
        CImg<float> grayImg = c.toGrayScale(warping_result); //得到灰度图
        //grayImg.display("gray image");

        // 数字字符切割
        myImageSegmentation numberSegmentation;
        numberSegmentation.run(grayImg, sni.c_str(), filenames[i].substr(0,8));
	}
    out.close();
    return 0;
}
