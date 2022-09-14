#include <iostream>
#include <fstream>
// include有顺序要求, 顺序反了就进不来了, 必须从上向下的来
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include "bmphdr.h"

using namespace std;
using namespace cv;

/* 输出 bmp 图片头部信息 */
void bmpFileInfo(ifstream &fpbmp, int &Offset, int &rows, int &cols);
/* 将 bmp24bit 图片读入 Mat 中 */
void bmp24bitToMat(ifstream &fpbmp, Mat &bmp, int Offset);
/* 将 bmp16色 图片读入 Mat 中: 将原本24位的通过调色盘放到16位 */
void bmp16ToMat(ifstream &fpbmp, Mat &bmp, int Offset);

int main()
{
    // * 打开 bmp 文件
    ifstream fpbmp("lena512.bmp", ifstream::in | ifstream::binary);
    fpbmp.seekg(0, fpbmp.end);
    int length = fpbmp.tellg();
    fpbmp.seekg(0, fpbmp.beg);
    cout << "Length of the bmp: " << length << endl;

    int Offset, rows, cols;
    // * 初始化信息函数
    bmpFileInfo(fpbmp, Offset, rows, cols);
    // ! Mat pic(rows, cols, type), type与图片色位数和通道数相关
    Mat bmp(rows, cols, CV_8UC1);

    // * 不同色的bmp图片
    // 24
    bmp24bitToMat(fpbmp, bmp, Offset);
    // 16
    // bmp16ToMat(fpbmp, bmp, Offset);

    // * 展示图片
    // namedWindow("bmp", WINDOW_AUTOSIZE);
    imshow("bmp", bmp);
    waitKey(0);
    destroyWindow("bmp");

    return 0;
}

void bmpFileInfo(ifstream &fpbmp, int &Offset, int &rows, int &cols)
// * 不管用什么办法读取, 这里应该都是通用的, 即获取图片的基本信息, 并把读取的指针指向像素的第一位
{
    BITMAPFILEHEADER fh;
    WORD bfType;
    fpbmp.read((char *)&bfType, sizeof(WORD));
    fpbmp.read((char *)&fh, sizeof(BITMAPFILEHEADER));

    // if (fh.bfType != 'MB')
    // {
    //     cout << "It's not a bmp file" << endl;
    //     exit(1);
    // }

    cout << "\ttagBITMAPFILEHEADER info \t" << endl;
    cout << "bfType: \t" << (bfType) << endl;
    cout << "bfSize: \t" << (fh.bfSize) << endl;
    cout << "bfReserved1: \t" << (fh.bfReserved1) << endl;
    cout << "bfReserved2: \t" << (fh.bfReserved2) << endl;
    cout << "bfOffbits: \t" << (fh.bfOffBits) << endl;
    cout << "总字节偏移: \t" << (fpbmp.tellg()) << endl;

    BITMAPINFOHEADER fih;
    fpbmp.read((char *)&fih, sizeof(BITMAPINFOHEADER));
    cout << "\t\t tagBITMAPINFOHEADER info\t\t" << endl;
    cout << "位图信息头字节数: \t" << (fih.biSize) << endl;
    cout << "图像像素宽度: \t" << (fih.biWidth) << endl;
    cout << "图像像素高等: \t" << (fih.biHeight) << endl;
    cout << "位面数: \t" << (fih.biPlanes) << endl;
    cout << "单像素位数: \t" << (fih.biBitCount) << endl;
    cout << "压缩说明: \t" << (fih.biCompression) << endl;
    cout << "图像大小: \t" << (fih.biSizeImage) << endl;
    cout << "水平分辨率: \t" << (fih.biXPelsPerMeter) << endl;
    cout << "垂直分辨率: \t" << (fih.biYPelsPerMeter) << endl;
    cout << "位图使用颜色数:\t" << (fih.biClrUsed) << endl;
    cout << "重要颜色数:\t" << (fih.biClrImportant) << endl;
    cout << "总字节偏移:\t" << (fpbmp.tellg()) << endl;

    Offset = fh.bfOffBits;
    rows = fih.biHeight;
    cols = fih.biWidth;

    if (Offset != 54)
    {
        RGBQUAD rgbPalette[16];
        fpbmp.read((char *)&rgbPalette, 16 * sizeof(RGBQUAD));
        cout << "\t\t tagRGBQUAD info \t\t" << endl;
        cout << "   (R, G, B, Alpha)" << endl;
        for (int i = 0; i < 16; i++)
        {
            cout << " No." << i << "	"
                 << "("
                 << (rgbPalette[i].rgbRed + 0) << ", "
                 << (rgbPalette[i].rgbGreen + 0) << ", "
                 << (rgbPalette[i].rgbBlue + 0) << ", "
                 << (rgbPalette[i].rgbReserved + 0) << ")" << endl;
            // if (i != 0 && i % 4 == 0) cout << endl;
        }
        cout << " 总字节偏移 ：" << fpbmp.tellg() << endl;
    }
}

void bmp24bitToMat(ifstream &fpbmp, Mat &bmp, int Offset)
// * 这个就是正常的载入
// * 由于直接用了cv::Mat类, 这里的读入比较简单, 直接循环读就好
// ? 不知道直接用二维数组或者链表怎么读, 直接读入不知道之后方不方便进行操作
{
    // * 从起点之后的多少个开始读, offset为信息头
    fpbmp.seekg(Offset, fpbmp.beg);
    // * 从左到右, 从下到上开始读
    for (int i = (bmp.rows - 1); i >= 0; i--)
    {
        for (int j = 0; j < bmp.cols; j++)
        {
            // ? 三通道是三个都长这样, 单通道(灰度)则需要改变类型, 用uchar
            fpbmp.read((char *)&bmp.at<uchar>(i, j), 1);
            // fpbmp.read((char *)&bmp.at<Vec3b>(i, j)[1], 1);
            // fpbmp.read((char *)&bmp.at<Vec3b>(i, j)[2], 1);
        }
    }
}

void bmp16ToMat(ifstream &fpbmp, Mat &bmp, int Offset)
{
    RGBQUAD rgbPalette[16];
    fpbmp.seekg(54, fpbmp.beg);
    fpbmp.read((char *)&rgbPalette, 16 * sizeof(RGBQUAD));
    fpbmp.seekg(Offset, fpbmp.beg);
    char tmp;
    for (int i = (bmp.rows - 1); i >= 0; i--)
    {
        for (int j = 0; j < bmp.cols; j += 2)
        {
            fpbmp.read((char *)&tmp, 1);
            bmp.at<Vec3b>(i, j)[0] = rgbPalette[tmp >> 4 & 0x0f].rgbBlue;
            bmp.at<Vec3b>(i, j)[1] = rgbPalette[tmp >> 4 & 0x0f].rgbGreen;
            bmp.at<Vec3b>(i, j)[2] = rgbPalette[tmp >> 4 & 0x0f].rgbRed;
            bmp.at<Vec3b>(i, j + 1)[0] = rgbPalette[tmp & 0x0f].rgbBlue;
            bmp.at<Vec3b>(i, j + 1)[1] = rgbPalette[tmp & 0x0f].rgbGreen;
            bmp.at<Vec3b>(i, j + 1)[2] = rgbPalette[tmp & 0x0f].rgbRed;
        }
    }
}