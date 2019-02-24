#include <iostream>
#include "HistogramEqualization.hpp"

//windows 编译时，后缀 -O2 -lgdi32
//mac 编译时，后缀 -O2 -L/usr/X11R6/lib -lm -lpthread -lX11
using namespace std;

int main(int argc, char const *argv[]) {

//    const char* filePath = "data1/img5.bmp"; //Filepath of input image
 //   HistogramEqualization he(filePath);


    const char* filePath1 = "data2/img5.bmp"; //Filepath of input image
    const char* filePath2 = "data2/img52.bmp"; //Filepath of input image
    HistogramEqualization he(filePath1,filePath2);

    return 0;
}

