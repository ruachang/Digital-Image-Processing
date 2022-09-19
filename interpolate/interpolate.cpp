#include <iostream>
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include "bmphdr.h"

using namespace std;
using namespace cv;

/* 输出 bmp 图片头部信息 */
void bmpFileInfo(ifstream &fpbmp, int &Offset, int &rows, int &cols);
/* 将 bmp图片读入 Mat 中 */
void bmp8bitToMat(ifstream &fpbmp, Mat &bmp, int Offset);
// 最邻近插值
void interpolate_nearest(Mat &src, Mat &dst, int height, int width);
// 双线性插值
void interpolate_bilinear(Mat &src, Mat &dst, int height, int width);

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
    Mat bmp(rows, cols, CV_8UC1);
    Mat dst_nearest(rows * 1.5, cols * 1.5, CV_8UC1);
    Mat dst_bilinear(rows * 1.5, cols * 1.5, CV_8UC1);

    // * 不同色的bmp图片
    // 24
    bmp8bitToMat(fpbmp, bmp, Offset);

    interpolate_nearest(bmp, dst_nearest, rows * 1.5, cols * 1.5);
    interpolate_bilinear(bmp, dst_bilinear, rows * 1.5, cols * 1.5);

    // * 展示图片
    imshow("original", bmp);
    imshow("nearest", dst_nearest);
    imshow("bilinear", dst_bilinear);
    waitKey(0);
    destroyAllWindows();
    return 0;
}

void bmpFileInfo(ifstream &fpbmp, int &Offset, int &rows, int &cols)
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

void bmp8bitToMat(ifstream &fpbmp, Mat &bmp, int Offset)
// * 这个就是正常的载入
{
    // * 从起点之后的多少个开始读, offset为信息头
    fpbmp.seekg(Offset, fpbmp.beg);
    // * 从左到右, 从下到上开始读
    for (int i = (bmp.rows - 1); i >= 0; i--)
    {
        for (int j = 0; j < bmp.cols; j++)
        {
            fpbmp.read((char *)&bmp.at<uchar>(i, j), 1);
        }
    }
}

void interpolate_nearest(Mat &src, Mat &dst, int height, int width)
// * 最邻近: 对图像进行坐标变化后, 映射到最近的像素点上 ==> 需要计算现在放缩后的图像的像素点在原图上像素点的对应
// height, width: 目标映射的位置
{
    Mat tmp(height, width, CV_8UC1);
    // 插值计算得到的坐标点
    int res_h, res_w;
    for (int i = 0; i < height; i++)
    // 针对每一个像素做相同的操作
    {
        for (int j = 0; j < width; j++)
        {
            res_h = int(float(i) / height * src.rows + 0.5);
            res_w = int(float(j) / width * src.cols + 0.5);
            // 根据求得的坐标的位置映射到对应的位置
            tmp.at<uchar>(i, j) = src.at<uchar>(res_h, res_w);
        }
    }
    dst = tmp.clone();
}

void interpolate_bilinear(Mat &src, Mat &dst, int height, int width)
// * 双线性: 在重投影之后利用周围的四个点确定, 由周围的四个点加权得到现在值, 权重由距离决定
{
    Mat tmp(height, width, CV_8UC1);
    int res_h, res_w = 0;
    float reflect_h, reflect_w = 0.0;
    float dis_h, dis_w = 0.0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            reflect_h = float(i) / height * src.rows;
            reflect_w = float(j) / width * src.cols;
            // 得到重投影的目标点
            res_h = int(reflect_h);
            res_w = int(reflect_w);
            // 得到重投影后的距离, 代表权重
            dis_h = reflect_h - res_h;
            dis_w = reflect_w - res_w;
            // 某一个点的值等于周围四个点的加权求和
            tmp.at<uchar>(i, j) =
                (1 - dis_h) * (1 - dis_w) * src.at<uchar>(res_h, res_w) +
                (1 - dis_h) * dis_w * src.at<uchar>(res_h + 1, res_w) +
                (dis_h) * (1 - dis_w) * src.at<uchar>(res_h, res_w + 1) +
                (dis_h)*dis_w * src.at<uchar>(res_h + 1, res_w + 1);
        }
    }
    dst = tmp.clone();
}