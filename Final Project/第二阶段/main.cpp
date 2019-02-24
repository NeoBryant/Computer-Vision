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

vector<string> getFiles(string cate_dir);

int main() {

	string filepath = "../testdata/"; // 测试图像文件夹
	
    //vector<string> filenames = {"12330029.bmp"}; // 读取图像文件名

	vector<string> filenames = getFiles(filepath); // 读取图像文件名

    
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
        warping_result.save(warpingpath.c_str());

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

vector<string> getFiles(string cate_dir) {
    // 存放文件名
    vector<string> files;

#ifdef WIN32
    _finddata_t file;
    long lf;
    // 输入文件夹路径
    if ((lf=_findfirst(cate_dir.append("\\*").c_str(), &file)) == -1L) {
        cout<<cate_dir<<" not found!!!"<<endl;
    } else {
        while(_findnext(lf, &file) == 0) {
            // 输出文件名
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
                continue;
            files.push_back(file.name);
        }
    }
    _findclose(lf);
#endif

#ifdef linux
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    if ((dir=opendir(cate_dir.c_str())) == NULL)
        {
        perror("Open dir error...");
                exit(1);
        }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
                continue;
        else if(ptr->d_type == 8)    ///file
            files.push_back(ptr->d_name);
        else if(ptr->d_type == 10)    ///link file
            continue;
        else if(ptr->d_type == 4)    ///dir
        {
            files.push_back(ptr->d_name);
        }
    }
    closedir(dir);
#endif
    // 排序，按从小到大排序
    sort(files.begin(), files.end());
    return files;
}