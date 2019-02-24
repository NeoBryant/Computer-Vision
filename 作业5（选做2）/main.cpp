#include "ImageMorphing.cpp"

int main() {
    const char* srcFilename = "data/img1.bmp";
    const char* dstFilename = "data/img2.bmp";

	ImageMorphing imageMorphing(srcFilename, dstFilename); // 默认中间11帧
	imageMorphing.meshBasedMorphing();  //基于特征点的变换
	//imageMorphing.crossDisolveMorphing(); //直接像素叠加变换
	return 0;
}