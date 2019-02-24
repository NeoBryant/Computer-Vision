#include "MyOtsu.hpp"
//windows编译时，后缀 -O2 -lgdi32

int main(int argc, char const *argv[]){
    const char* file_name0 = "../data/A4file0.bmp";
    MyOtsu ostu0(file_name0);

    return 0;
}
