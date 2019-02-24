#include <iostream>
#include "mycanny.hpp"
#include "myhough.hpp"
//windows 编译时，后缀 -O2 -lgdi32
//mac 编译时，后缀 -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
using namespace std;

int main(int argc, char const *argv[]) {


    //直线检测
    const char* filePath = "img/line1.bmp"; //Filepath of input image
    myhough mhgh(filePath,"line");

    //圆检测
    //const char* filePath = "img/circle4.bmp"; //Filepath of input image
    //myhough mhgh(filePath,"circle");


    return 0;
}

