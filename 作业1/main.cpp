#include "TestCimg.hpp"
//windows 编译时，后缀 -O2 -lgdi32
//mac 编译时，后缀 -O2 -L/usr/X11R6/lib -lm -lpthread -lX11

int main(int argc, char const *argv[])
{
    const char* filename = "1.bmp";
    const char* filename2 = "2.bmp";
    TestCimg testcimg;

    testcimg.readAndDisplay_hw1(filename); //读入“1.bmp”并显示
    
    testcimg.changeColor_hw2(); //把“1.bmp”白色变红色，绿色变蓝色

    testcimg.draw_circle_nocimg_hw3();//不用cimg相关函数画圆1
    testcimg.draw_circle_hw3();//用cimg相关函数画圆1
    
    testcimg.draw_circle_nocimg_hw4();//不用cimg相关函数画圆2
    testcimg.draw_circle_hw4();//用cimg相关函数画圆2

    testcimg.draw_line_nocimg_hw5(); //不用cimg相关函数画直线
    testcimg.draw_line_hw5(); //用cimg相关函数画直线
    
    testcimg.save_file_hw6(filename2);//保存文件为“2.bmp”

    return 0;
}
