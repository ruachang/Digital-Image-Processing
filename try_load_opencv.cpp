/*
读入bmp文件: 按字节读取
读取的顺序就是存储的顺序, 高位对应前面, 低位对应后面
*/

#include <iostream>
#include <vector>
#include <string>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main()
{
    Mat img = imread("/Users/liuchang/Pictures/100262277_p0_master1200.jpeg");
    if (!img.data)
    {
        cout << "couldn't load the image";
        return -1;
    }
    else
    {
        cout << "loaded the image";
        imshow("img", img);
        waitKey(0);
    }

    // while (true)
    // {
    //     int c = waitKey(20);
    //     if (27 == (char)c)
    //         break;
    // }
    return 0;
}
