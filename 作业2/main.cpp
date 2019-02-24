#include <iostream>
#include "mycanny.hpp"
//编译时，后缀 -O2 -lgdi32

using namespace std;

int main(int argc, char const *argv[]) {

  //  bigben.bmp  lena.bmp  stpietro.bmp  twows.bmp
  //  const char* filePath0 = "img/bigben.bmp"; //Filepath of input image
  //  mycanny mcny0(filePath0);
    const char* filePath1 = "img/lena.bmp"; //Filepath of input image
    mycanny mcny1(filePath1);
 //   const char* filePath2 = "img/stpietro.bmp"; //Filepath of input image
 //   mycanny mcny2(filePath2);
  //  const char* filePath3 = "img/twows.bmp"; //Filepath of input image
  //  mycanny mcny3(filePath3);

    return 0;
}



