#include <iostream>
#include "CImg.h"
#include <vector>

#include <string>
#include <fstream>
#include <sstream>
#include <string>


using namespace std;
using namespace cimg_library;

struct Point { //存储特征点的横坐标和纵坐标
	float x, y;
	Point(float _x, float _y): x(_x), y(_y) {}
};
struct Triangle { //存储三角形的三个顶点
	Point p1, p2, p3;
	Triangle(Point _p1, Point _p2, Point _p3): p1(_p1), p2(_p2), p3(_p3) {}
};

class ImageMorphing {
private:
	vector<Point> src_points; //源图像的特征点列表
	vector<Point> dst_points; //目标图像的特征点列表
	vector<vector<Point>> mid_points; //中间图像的特征点列表

	vector<vector<int>> index; //三角形的顶点下标
	vector<Triangle> src_triangle_list;  //源图像的三角形列表
	vector<Triangle> dst_triangle_list;  //目标图像的三角形列表
	vector<vector<Triangle>> mid_triangle_list; //中间图像的三角形列表

	CImg<float> src; //源图像
	CImg<float> dst; //目标图像
	CImgList<float> result; //过渡图像列表，一共13张（包括中间11帧变换图像、源图像和目标图像）

	int frame_cnt;
public:
	ImageMorphing(const char*, const char*);
	void setSrcTriangleList();
	void setDstTriangleList();
	void setMidTriangleList();
	CImg<float> computeTransformMatrix(Triangle before, Triangle after); // 计算变换矩阵
	bool isInTriangle(Point P, Triangle tri); // 重心法判断一个点是否在三角形内
	
	CImgList<float> meshBasedMorphing(); //基于特征点的变换函数
	CImgList<float> crossDisolveMorphing(); //像素值直接叠加变换函数

	void readPointAndTrianglesPoints(); //从文件夹points中读取数据
	void saveImgs();  //存储图像到文件夹result中
}; 