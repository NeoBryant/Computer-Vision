#ifndef MYIMAGESEGMENTATION_H
#define MYIMAGESEGMENTATION_H

#include "CImg.h"
#include <vector>
#include <list>
#include <iostream>
#include <direct.h>

#include "fstream"
using namespace std;
using namespace cimg_library;

//#define BoundaryRemoveGap 10        // 图像边缘设为白色的距离

int RED[3]{ 255, 0, 0 };

struct Point {
	int x, y;
	Point() : x(-1), y(-1) {}
	Point(int posX, int posY) : x(posX), y(posY) {}
};

class myImageSegmentation {
public:
	void run(CImg<float> warpingResult, const string baseAddress, const string imgName);

private:
	CImg<float> binaryImg, tagImg, histogramImg, dividingImg;
	vector<CImg<float>> subImageSet;     // 一行行数字图像
	vector<Point> dividePoints;          // 直方图峰值划分线点集
	int tagAccumulate = -1;              // 类别tag累加值
	vector<int> classTagSet;             // 类别tag列表
	vector<list<Point>> pointPosListSet; // 装载类别tag对应的所有点的位置的list的列表
	vector<list<Point>> pointPosListSetForDisplay;
	string lineBasePath; 				 // 数字行图片存放地址
	string basePath;                     // 单个数字图片生成、预测结果文本存放路径
	string imglisttxt = "";

private:
	// 图像二值化处理
	CImg<float> convertToBinaryImg(CImg<float> warpingResult);
	// 做y方向的直方图，找到行与行之间的分割线
	void findDividingLine();
	// 通过分割线，将图片划分为一行行
	void divideIntoBarItemImg();
	//对每一张划分的图的数字，做扩张
	void dilateImg(int barItemIndex);
	// 连通区域标记算法
	void connectedRegionsTagging(int barItemIndex);
	// 存储分割后每一张数字的图以及对应的文件名称
	void saveSingleNumImg(int barItemIndex);
	// 添加新的类tag
	void addNewTag(int x, int y, int barItemIndex);
	// 在正上、左上、左中、左下这四个邻点中找到最小的tag
	void findMinTag(int x, int y, int &minTag, Point &minTagPointPos, int barItemIndex);
	// 合并某个点(x,y)所属类别
	void mergeTagImageAndList(int x, int y, const int minTag, const Point minTagPointPos, int barItemIndex);
	// 获取单个数字的包围盒
	void getBoundingOfSingleNum(int listIndex, int& xMin, int& xMax, int& yMin, int& yMax);
	// 根据X方向直方图判断真实的拐点
	vector<int> getInflectionPosXs(const CImg<float>& XHistogramImage);
	// 获取一行行的子图的水平分割线
	vector<int> getDivideLineXofSubImage(const CImg<float>& subImg);
	// X方向2个单位的负扩张，Y方向1个单位的正扩张
	int getDilateXXY(const CImg<float>& Img, int x, int y);
	// XY方向的正扩张
	int getDilateXY(const CImg<float>& Img, int x, int y);
	// 对单个数字图像做Y方向腐蚀操作
	CImg<float> eroseImg(CImg<float>& Img);
	// 分割行子图，得到列子图
	vector<CImg<float>> getRowItemImgSet(const CImg<float>& lineImg, vector<int> _dividePosXset);
};


void myImageSegmentation::run(CImg<float> warpingResult, const string baseAddress, const string imgName) {
	if (_access((baseAddress+"numbers/"+imgName).c_str(), 0) == -1)
		_mkdir((baseAddress+"numbers/"+imgName).c_str());
	
	basePath = baseAddress + "numbers/" + imgName + "/"; // 数字分割图片存储地址

	if (_access((baseAddress+"lines/"+imgName).c_str(), 0) == -1)
		_mkdir((baseAddress+"lines/"+imgName).c_str());
	lineBasePath = baseAddress + "lines/" + imgName + "/"; // 数字分割图片存储地址


	binaryImg = convertToBinaryImg(warpingResult); //对矫正后的图片进行二值化处理
	//binaryImg.display("binary"); //显示二值化后的图片

	// 行分割，按照行划分数字
	findDividingLine();
	cout << "getting number lines" << endl;

	divideIntoBarItemImg();
	cout << "dividing into bar item img" << endl;
	
	//histogramImg.display("Histogram");
	//dividingImg.display("Divide");

	// 对每张子图操作
	for (int i = 0; i < subImageSet.size(); i++) {
		dilateImg(i); // 对分割后每一张数字的图的数字，做扩张
		connectedRegionsTagging(i); // 连通区域标记算法
		saveSingleNumImg(i); // 存储分割后每一张数字的图以及对应的文件名称
		//cout << imglisttxt.c_str() << endl;
	}
	cout << "finished image segmentation" << endl;
}


// 图像二值化处理，局部自适应阈值二值化
CImg<float> myImageSegmentation::convertToBinaryImg(CImg<float> warpingResult) {
	binaryImg = CImg<float>(warpingResult._width, warpingResult._height, 1, 1, 0);
	binaryImg.fill(255); //是否需要这一步

	int compensation = 15; //补偿值
	int block = 9; //块的边长
	for (int x = 0; x < warpingResult._width; x += block){
		for (int y = 0; y < warpingResult._height; y += block){
			//计算块内平均灰度
			int mean = 0; // 平均灰度
			for (int dx = 0; dx < block; ++dx){
				for (int dy = 0; dy < block; ++dy){
					if (x+dx < warpingResult._width && y+dy<warpingResult._height){
						mean += warpingResult(x+dx, y+dy);
					}
				}
			}
			mean /= block*block; //块平均灰度
			mean -= compensation;
			for (int dx = 0; dx < block; ++dx){
				for (int dy = 0; dy < block; ++dy){
					if (x+dx < warpingResult._width && y+dy<warpingResult._height){
						if (warpingResult(x+dx,y+dy) < mean){
							binaryImg(x+dx,y+dy) = 0;
						} else {
							binaryImg(x+dx,y+dy) = 255;
						}
					}
				}
			}
		}
	}
	//消去边缘噪声影响
	const int BoundaryRemoveGap = 10;
	cimg_forXY(binaryImg, x, y) {
		if (x <= BoundaryRemoveGap || y <= BoundaryRemoveGap
			|| x >= warpingResult._width - BoundaryRemoveGap || y >= warpingResult._height - BoundaryRemoveGap) {
			binaryImg(x, y, 0) = 255; // 白色
		}
	}
	return binaryImg;
}

/*找到划分的有数字的行

*/
void myImageSegmentation::findDividingLine() {
	histogramImg = CImg<float>(binaryImg._width, binaryImg._height, 1, 3, 255);
	dividingImg = binaryImg;
	vector<int> inflectionPoints; // 拐点
	cimg_forY(histogramImg, y) {
		int blackPixel = 0;
		cimg_forX(binaryImg, x) {
			if (binaryImg(x, y, 0) == 0)
				blackPixel++;
		}
		cimg_forX(histogramImg, x) {
			if (x < blackPixel) {
				histogramImg(x, y, 0) = 0;
				histogramImg(x, y, 1) = 0;
				histogramImg(x, y, 2) = 0;
			}
		}

		// 求Y方向直方图，谷的最少黑色像素个数为0
		// 判断是否为拐点
		if (y > 0) {
			// 下白上黑：取下
			if (blackPixel <= 0 && histogramImg(0, y - 1, 0) == 0) 
				inflectionPoints.push_back(y);
			// 下黑上白：取上
			else if (blackPixel > 0 && histogramImg(0, y - 1, 0) != 0) 
				inflectionPoints.push_back(y - 1);
		}
	}

	dividePoints.push_back(Point(0, -1));

	// 两拐点中间做分割
	if (inflectionPoints.size() > 2) {
		for (int i = 1; i < inflectionPoints.size() - 1; i = i + 2) {
			int dividePoint = (inflectionPoints[i] + inflectionPoints[i + 1]) / 2;
			dividePoints.push_back(Point(0, dividePoint));
		}
	}
	dividePoints.push_back(Point(0, binaryImg._height - 1));

	//histogramImg.display("Histogram");
}


// 根据行分割线划分图片
void myImageSegmentation::divideIntoBarItemImg() {
	vector<Point> tempDivideLinePointSet;
	int count = -1;

	for (int i = 1; i < dividePoints.size(); i++) {
		//cout << "dividePoints  " << i ;

		int barHeight = dividePoints[i].y - dividePoints[i - 1].y;
		int blackPixel = 0;
		CImg<float> barItemImg = CImg<float>(binaryImg._width, barHeight, 1, 1, 0);
		
		//cout << " a ";
		cimg_forXY(barItemImg, x, y) {
			barItemImg(x, y, 0) = binaryImg(x, dividePoints[i - 1].y + 1 + y, 0);
			if (barItemImg(x, y, 0) == 0)
				blackPixel++;
		}
		//cout << "b ";
		double blackPercent = (double)blackPixel / (double)(binaryImg._width * barHeight);

		//cout << "c ";
		// 只有当黑色像素个数超过图像大小一定比例0.001时，才可视作有数字
		if (blackPercent > 0.001) {
			++count;
			// 将划分的行存储
			char addr[200];
			string postfix = ".bmp";
			sprintf(addr, "%s%d%s", lineBasePath.c_str(), count, postfix.c_str());
			barItemImg.save(addr);

			//cout << 1;
			vector<int> dividePosXset = getDivideLineXofSubImage(barItemImg);
			
			//cout << 2;
			vector<CImg<float>> rowItemImgSet = getRowItemImgSet(barItemImg, dividePosXset);

			//cout << 3;
			for (int j = 0; j < rowItemImgSet.size(); j++) {
				subImageSet.push_back(rowItemImgSet[j]);
				tempDivideLinePointSet.push_back(Point(dividePosXset[j], dividePoints[i - 1].y));
			}
			//cout << 4;
			if (i > 1) {
				histogramImg.draw_line(0, dividePoints[i - 1].y,
					histogramImg._width - 1, dividePoints[i - 1].y, RED, 1, 3);
				dividingImg.draw_line(0, dividePoints[i - 1].y,
					histogramImg._width - 1, dividePoints[i - 1].y, RED, 1, 3);
			}
			//cout << 5;
			// 绘制竖线
			for (int j = 1; j < dividePosXset.size() - 1; j++) {
				dividingImg.draw_line(dividePosXset[j], dividePoints[i - 1].y,
					dividePosXset[j], dividePoints[i].y, RED, 1, 3);
			}
			//cout << 6;
		}
		//cout << "d " << endl;
	}

	dividePoints.clear();
	for (int i = 0; i < tempDivideLinePointSet.size(); i++){
		dividePoints.push_back(tempDivideLinePointSet[i]);
	}
}

// 获取一行行的子图的水平分割线
vector<int> myImageSegmentation::getDivideLineXofSubImage(const CImg<float>& subImg) {
	// 先绘制X方向灰度直方图
	//cout << "i ";
	CImg<float> XHistogramImage = CImg<float>(subImg._width, subImg._height, 1, 3, 255);
	//cout << "ii ";
	cimg_forX(subImg, x) {
		int blackPixel = 0;
		cimg_forY(subImg, y) {
			if (subImg(x, y, 0) == 0)
				blackPixel++;
		}
		// 对于每一列x，只有黑色像素多于一定值，才绘制在直方图上
		// 求X方向直方图，谷的最少黑色像素个数        
		if (blackPixel >= 4) {
			cimg_forY(subImg, y) {
				if (y < blackPixel) {
					XHistogramImage(x, y, 0) = 0;
					XHistogramImage(x, y, 1) = 0;
					XHistogramImage(x, y, 2) = 0;
				}
			}
		}
	}
	//cout << "iii ";
	vector<int> InflectionPosXs = getInflectionPosXs(XHistogramImage);    //获取拐点
	//cout << "iiii ";
	for (int i = 0; i < InflectionPosXs.size(); i++){
		XHistogramImage.draw_line(InflectionPosXs[i], 0, InflectionPosXs[i], XHistogramImage._height - 1, RED, 1, 3);
	}
	//cout << "iiiii ";
	// 两拐点中间做分割
	vector<int> dividePosXs;
	dividePosXs.push_back(-1);
	if (InflectionPosXs.size() > 2) {
		for (int i = 1; i < InflectionPosXs.size() - 1; i = i + 2) {
			int divideLinePointX = (InflectionPosXs[i] + InflectionPosXs[i + 1]) / 2;
			dividePosXs.push_back(divideLinePointX);
		}
	}
	dividePosXs.push_back(XHistogramImage._width - 1);
	//cout << "iiiiii ";

	return dividePosXs;
}

// 根据X方向直方图判断真实的拐点
vector<int> myImageSegmentation::getInflectionPosXs(const CImg<float>& XHistogramImage) {
	//cout << "k ";
	vector<int> resultInflectionPosXs, tempInflectionPosXs;
	int totalDist = 0, dist = 0;
	//cout << "kk ";
	// 查找拐点
	cimg_forX(XHistogramImage, x) {
		if (x >= 1) {
			// 白转黑
			if (XHistogramImage(x, 0, 0) == 0 && XHistogramImage(x - 1, 0, 0) == 255) 
				tempInflectionPosXs.push_back(x - 1);
			// 黑转白
			else if (XHistogramImage(x, 0, 0) == 255 && XHistogramImage(x - 1, 0, 0) == 0) 
				tempInflectionPosXs.push_back(x);
		}
	}
	//cout << "kkk ";
	//cout << "size " << tempInflectionPosXs.size()-1 << " ";
	/*if (3 < tempInflectionPosXs.size()){
		cout << "**";
	}*/
	if (tempInflectionPosXs.size() == 0){
		return resultInflectionPosXs;
	}
	for (int i = 2; i+1 < tempInflectionPosXs.size(); i = i + 2) {  //bug1:
		//cout << i;
		int tempdist = tempInflectionPosXs[i] - tempInflectionPosXs[i - 1];
		//cout << "p";
		if (tempdist <= 0) {
			tempdist--;
		}
		//cout << "r";
		totalDist += tempdist;
	}
	//cout << "kkkk ";
	// 计算间距平均距离
	dist += (tempInflectionPosXs.size() - 2) / 2;
	int avgDist = 0;
	if (dist != 0) {
		avgDist = totalDist / dist;
	}
	//cout << "kkkkk ";
	resultInflectionPosXs.push_back(tempInflectionPosXs[0]); //头
	// 当某个间距大于平均距离的一定倍数时，视为分割点所在间距
	for (int i = 2; i+1 < tempInflectionPosXs.size(); i = i + 2) { //bug2
		int dist = tempInflectionPosXs[i] - tempInflectionPosXs[i - 1];
		if (dist > avgDist * 4) {
			resultInflectionPosXs.push_back(tempInflectionPosXs[i - 1]);
			resultInflectionPosXs.push_back(tempInflectionPosXs[i]);
		}
	}
	resultInflectionPosXs.push_back(tempInflectionPosXs[tempInflectionPosXs.size() - 1]); //尾
	//cout << "kkkkkk ";
	return resultInflectionPosXs;
}


void myImageSegmentation::dilateImg(int barItemIndex) {
	// 扩张Dilation -X-X-X-XYY方向
	CImg<float> answerXXY = CImg<float>(subImageSet[barItemIndex]._width, subImageSet[barItemIndex]._height, 1, 1, 0);
	cimg_forXY(subImageSet[barItemIndex], x, y) {
		answerXXY(x, y, 0) = getDilateXXY(subImageSet[barItemIndex], x, y);
	}

	// 扩张Dilation -X-X-X-XYY方向
	CImg<float> answerXXY2 = CImg<float>(answerXXY._width, answerXXY._height, 1, 1, 0);
	cimg_forXY(answerXXY, x, y) {
		answerXXY2(x, y, 0) = getDilateXXY(answerXXY, x, y);
	}

	//扩张Dilation XY方向
	CImg<float> answerXY = CImg<float>(answerXXY2._width, answerXXY2._height, 1, 1, 0);
	cimg_forXY(answerXXY2, x, y) {
		answerXY(x, y, 0) = getDilateXY(answerXXY2, x, y);
	}

	cimg_forXY(subImageSet[barItemIndex], x, y) {
		subImageSet[barItemIndex](x, y, 0) = answerXY(x, y, 0);
	}
}

// 连通区域标记算法
void myImageSegmentation::connectedRegionsTagging(int barItemIndex) {
	tagImg = CImg<float>(subImageSet[barItemIndex]._width, subImageSet[barItemIndex]._height, 1, 1, 0);
	tagAccumulate = -1;

	cimg_forX(subImageSet[barItemIndex], x)
		cimg_forY(subImageSet[barItemIndex], y) {
		// 第一行和第一列
		if (x == 0 || y == 0) {
			int intensity = subImageSet[barItemIndex](x, y, 0);
			if (intensity == 0) {
				addNewTag(x, y, barItemIndex);
			}
		}
		// 其余的行和列
		else {
			int intensity = subImageSet[barItemIndex](x, y, 0);
			if (intensity == 0) {
				// 检查正上、左上、左中、左下这四个邻点
				int minTag = INT_MAX; //最小的tag
				Point minTagPointPos(-1, -1);
				// 先找最小的标记
				findMinTag(x, y, minTag, minTagPointPos, barItemIndex);

				// 当正上、左上、左中、左下这四个邻点有黑色点时，合并；
				if (minTagPointPos.x != -1 && minTagPointPos.y != -1) {
					mergeTagImageAndList(x, y - 1, minTag, minTagPointPos, barItemIndex);
					for (int i = -1; i <= 1; i++) {
						if (y + i < subImageSet[barItemIndex]._height)
							mergeTagImageAndList(x - 1, y + i, minTag, minTagPointPos, barItemIndex);
					}
					// 当前位置
					tagImg(x, y, 0) = minTag;
					Point cPoint(x + dividePoints[barItemIndex].x + 1, y + dividePoints[barItemIndex].y + 1);
					pointPosListSet[minTag].push_back(cPoint);

				}
				// 否则，作为新类
				else {
					addNewTag(x, y, barItemIndex);
				}
			}
		}
	}
}

// 存储分割后每一张数字的图以及对应的文件名称
void myImageSegmentation::saveSingleNumImg(int barItemIndex) {
	// 先统计每张数字图像黑色像素个数平均值
	int totalBlacks = 0, numberCount = 0;
	for (int i = 0; i < pointPosListSet.size(); i++) {
		if (pointPosListSet[i].size() != 0) {
			totalBlacks += pointPosListSet[i].size();
			numberCount++;
		}
	}
	int avgBlacks = totalBlacks / numberCount;

	for (int i = 0; i < pointPosListSet.size(); i++) {
		// 只有黑色像素个数大于平均值的一定比例，才可视为数字图像（去除断点）
		// 单张数字图像黑色像素个数超过所有数字图像，黑色像素个数均值的一定比例0.35才算作有数字
		if (pointPosListSet[i].size() != 0 && pointPosListSet[i].size() > avgBlacks * 0.35) {
			// 先找到数字的包围盒
			int xMin, xMax, yMin, yMax;
			getBoundingOfSingleNum(i, xMin, xMax, yMin, yMax);

			int width = xMax - xMin;
			int height = yMax - yMin;

			// 将单个数字填充到新图像：扩充到正方形
			// 单张数字图像边缘填充宽度为5
			int imgSize = (width > height ? width : height) + 5 * 2;
			CImg<float> singleNum = CImg<float>(imgSize, imgSize, 1, 1, 0);

			list<Point>::iterator it = pointPosListSet[i].begin();
			for (; it != pointPosListSet[i].end(); it++) {
				int x = (*it).x;
				int y = (*it).y;
				int singleNumImgPosX, singleNumImgPosY;
				if (height > width) {
					singleNumImgPosX = (x - xMin) + (imgSize - width) / 2;
					singleNumImgPosY = (y - yMin) + 5;
				}
				else {
					singleNumImgPosX = (x - xMin) + 5;
					singleNumImgPosY = (y - yMin) + (imgSize - height) / 2;
				}
				singleNum(singleNumImgPosX, singleNumImgPosY, 0) = 255;
			}

			// 对单个数字图像做Y方向腐蚀操作
			singleNum = eroseImg(singleNum);

			string postfix = ".bmp";
			char shortImgName[200];
			sprintf(shortImgName, "%d_%d%s\n", barItemIndex, classTagSet[i], postfix.c_str());
			imglisttxt += string(shortImgName);

			char addr[200];
			sprintf(addr, "%s%d_%d%s", basePath.c_str(), barItemIndex, classTagSet[i], postfix.c_str());
			singleNum.save(addr);

			pointPosListSetForDisplay.push_back(pointPosListSet[i]);
		}
	}
	// 存储图像文件名
	imglisttxt += "*\n";
	ofstream predictImageListOutput(basePath + "imagelist.txt");
	predictImageListOutput << imglisttxt.c_str();
	predictImageListOutput.close();

	// 把tag集、每一类链表数据集清空
	classTagSet.clear();
	for (int i = 0; i < pointPosListSet.size(); i++) {
		pointPosListSet[i].clear();
	}
	pointPosListSet.clear();
}



// 分割行子图，得到列子图
vector<CImg<float>> myImageSegmentation::getRowItemImgSet(const CImg<float>& lineImg, vector<int> _dividePosXset) {
	vector<CImg<float>> result;
	for (int i = 1; i < _dividePosXset.size(); i++) {
		int rowItemWidth = _dividePosXset[i] - _dividePosXset[i - 1];
		CImg<float> rowItemImg = CImg<float>(rowItemWidth, lineImg._height, 1, 1, 0);
		cimg_forXY(rowItemImg, x, y) {
			rowItemImg(x, y, 0) = lineImg(x + _dividePosXset[i - 1] + 1, y, 0);
		}
		result.push_back(rowItemImg);
	}

	return result;
}

// XY方向的正扩张
int myImageSegmentation::getDilateXY(const CImg<float>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (0 <= x + i && x + i < Img._width && 0 <= y + j && y + j < Img._height) {
					if (i != -1 && j != -1 || i != 1 && j != 1 || i != 1 && j != -1 || i != -1 && j != 1)
						if (Img(x + i, y + j, 0) == 0) {
							intensity = 0;
							break;
						}
				}
			}
			if (intensity != 255)
				break;
		}
	}
	return intensity;
}

// X方向2个单位的负扩张，Y方向1个单位的正扩张
int myImageSegmentation::getDilateXXY(const CImg<float>& Img, int x, int y) {
	int intensity = Img(x, y, 0);
	if (intensity == 255) {    //若中间点为白色
		int blackAccu = 0;

		for (int i = -1; i <= 1; i++) {
			if (0 <= y + i && y + i < Img._height) {    //垂直方向累加
				if (Img(x, y + i, 0) == 0)
					blackAccu++;
			}
		}
		for (int i = -2; i <= 2; i++) {
			if (0 <= x + i && x + i < Img._width) {     //水平方向累减
				if (Img(x + i, y, 0) == 0)
					blackAccu--;
			}
		}

		if (blackAccu > 0)
			intensity = 0;
	}
	return intensity;
}

// 添加新的类tag
void myImageSegmentation::addNewTag(int x, int y, int barItemIndex) {
	tagAccumulate++;
	tagImg(x, y, 0) = tagAccumulate;
	classTagSet.push_back(tagAccumulate);
	list<Point> pList;
	Point cPoint(x + dividePoints[barItemIndex].x + 1, y + dividePoints[barItemIndex].y + 1);
	pList.push_back(cPoint);
	pointPosListSet.push_back(pList);
}

// 在正上、左上、正左、左下这四个邻点中找到最小的tag
void myImageSegmentation::findMinTag(int x, int y, int &minTag, Point &minTagPointPos, int barItemIndex) {
	// 正上
	if (subImageSet[barItemIndex](x, y - 1, 0) == 0) {
		if (tagImg(x, y - 1, 0) < minTag) {
			minTag = tagImg(x, y - 1, 0);
			minTagPointPos.x = x;
			minTagPointPos.y = y - 1;
		}
	}
	// 左上、左中、左下
	for (int i = -1; i <= 1; i++) {
		if (y + i < subImageSet[barItemIndex]._height) {
			if (subImageSet[barItemIndex](x - 1, y + i, 0) == 0 && tagImg(x - 1, y + i, 0) < minTag) {
				minTag = tagImg(x - 1, y + i, 0);
				minTagPointPos.x = x - 1;
				minTagPointPos.y = y + i;
			}
		}
	}
}


// 合并某个点(x,y)所属类别
void myImageSegmentation::mergeTagImageAndList(int x, int y, const int minTag, const Point minTagPointPos, int barItemIndex) {
	// 赋予最小标记，合并列表
	if (subImageSet[barItemIndex](x, y, 0) == 0) {
		int tagBefore = tagImg(x, y, 0);
		if (tagBefore != minTag) {
			//把所有同一类的tag替换为最小tag、把list接到最小tag的list
			list<Point>::iterator it = pointPosListSet[tagBefore].begin();
			for (; it != pointPosListSet[tagBefore].end(); it++) {
				tagImg((*it).x - dividePoints[barItemIndex].x - 1, (*it).y - dividePoints[barItemIndex].y - 1, 0) = minTag;
			}
			pointPosListSet[minTag].splice(pointPosListSet[minTag].end(), pointPosListSet[tagBefore]);
		}
	}
}


// 获取单个数字的包围盒
void myImageSegmentation::getBoundingOfSingleNum(int listIndex, int& xMin, int& xMax, int& yMin, int& yMax) {
	xMin = yMin = INT_MAX;
	xMax = yMax = -1;
	if (!pointPosListSet.empty()) {
		list<Point>::iterator it = pointPosListSet[listIndex].begin();
		for (; it != pointPosListSet[listIndex].end(); it++) {
			int x = (*it).x, y = (*it).y;
			xMin = x < xMin ? x : xMin;
			yMin = y < yMin ? y : yMin;
			xMax = x > xMax ? x : xMax;
			yMax = y > yMax ? y : yMax;
		}
	}
	else {
		list<Point>::iterator it = pointPosListSetForDisplay[listIndex].begin();
		for (; it != pointPosListSetForDisplay[listIndex].end(); it++) {
			int x = (*it).x, y = (*it).y;
			xMin = x < xMin ? x : xMin;
			yMin = y < yMin ? y : yMin;
			xMax = x > xMax ? x : xMax;
			yMax = y > yMax ? y : yMax;
		}
	}
}


// 对单个数字图像做Y方向腐蚀操作
CImg<float> myImageSegmentation::eroseImg(CImg<float>& Img) {
	CImg<float> result = CImg<float>(Img._width, Img._height, 1, 1, 0);
	cimg_forXY(Img, x, y) {
		result(x, y, 0) = Img(x, y, 0);
		if (Img(x, y, 0) == 255) {
			if (y - 1 >= 0) {
				if (Img(x, y - 1, 0) == 0)
					result(x, y, 0) = 0;
			}

			if (y + 1 < Img._height) {
				if (Img(x, y + 1, 0) == 0)
					result(x, y, 0) = 0;
			}
		}
	}
	return result;
}

#endif