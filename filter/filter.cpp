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
/* 中值滤波 */
void mid_filter(Mat &src, Mat &dst, int kernel_size);
/* 低通高斯滤波 */
void lp_gauss_filter(Mat &src, Mat &dst, int kernel_size, double sigma);

int main()
{
    ifstream fpbmp("girl256-pepper&salt.bmp", ifstream::in | ifstream::binary);
    fpbmp.seekg(0, fpbmp.end);
    int length = fpbmp.tellg();
    fpbmp.seekg(0, fpbmp.beg);
    cout << "Length of the bmp: " << length << endl;

    int Offset, rows, cols;
    // * 初始化信息函数
    bmpFileInfo(fpbmp, Offset, rows, cols);
    Mat bmp(rows, cols, CV_8UC1);
    bmp8bitToMat(fpbmp, bmp, Offset);
    Mat output = bmp.clone();
    Mat example_out = bmp.clone();
    Mat output1 = bmp.clone();
    Mat example_out1 = bmp.clone();
    lp_gauss_filter(bmp, output1, 3, 1);
    GaussianBlur(bmp, example_out1, Size(3, 3), 1);
    mid_filter(bmp, output, 3);
    medianBlur(bmp, example_out, 3);

    // 展示和保存图片
    imwrite("result/中值滤波结果2.png", output);
    imwrite("result/低通高斯滤波结果2.png", output1);
    imwrite("result/中值滤波OpenCV结果2.png", example_out);
    imwrite("result/低通高斯滤波OpenCV结果2.png", example_out1);

    imshow("original", bmp);
    imshow("mid value filter", output);
    imshow("mid value by opencv", example_out);
    imshow("median blur value filter", output1);
    imshow("median blur value by opencv", example_out1);
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

void lp_gauss_filter(Mat &src, Mat &dst, int kernel_size, double sigma)
// 高斯低通滤波器
{
    int range = (kernel_size - 1) / 2;
    double gauss_mask[kernel_size][kernel_size];
    // 根据高斯函数计算高斯核
    double tmp = 0.0;
    for (int i = 0 - range; i < range + 1; i++)
    {
        for (int j = 0 - range; j < range + 1; j++)
        {
            gauss_mask[i + range][j + range] = exp(-(i * i + j * j) / (2 * sigma));
            tmp += gauss_mask[i + range][j + range];
        }
    }
    // 归一化
    for (int i = 0 - range; i < range + 1; i++)
    {
        for (int j = 0 - range; j < range + 1; j++)
        {
            gauss_mask[i + range][j + range] /= tmp;
        }
    }

    // 空间域卷积
    for (int i = 0; i < dst.rows; i++)
    {
        for (int j = 0; j < dst.cols; j++)
        {
            double value = 0.0;
            double pic_value = 0.0;
            // 在滤波器模板范围内进行依序相乘, 叠加
            for (int m = -range; m <= range; m++)
            {
                for (int n = -range; n <= range; n++)
                {
                    // 若不在边界就正常取值, 在边界则补零填充
                    if (i + m >= 0 && j + n >= 0 && i + m < dst.rows && j + n < dst.cols)
                    {
                        pic_value = src.at<uchar>(i + m, j + n);
                    }
                    else
                    {
                        pic_value = 0.0;
                    }
                    // 相乘叠加
                    value += gauss_mask[range + m][range + n] * pic_value;
                }
            }
            dst.at<uchar>(i, j) = value;
        }
    }
}

void mid_filter(Mat &src, Mat &dst, int kernel_size)
{
    // 读取以(i, j)为中心的模板, 对模板中的值排序代替原来的点
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            int pixel_seq[kernel_size * kernel_size];
            // 滤波器模板覆盖范围
            int range = (kernel_size - 1) / 2;
            for (int p = 0 - range; p < range + 1; p++)
            {
                for (int q = 0 - range; q < range + 1; q++)
                {
                    // 计算正在算的是第几个(其实完全没必要, 直接整个计数器更方便, 就是闲的)
                    int ord = (p + range) * kernel_size + q + range;
                    if (i + p < 0 || j + q < 0 || i + p > src.rows - 1 || j + q > src.cols - 1)
                    {
                        pixel_seq[ord] = 0;
                    }
                    else
                    {
                        pixel_seq[ord] = src.at<uchar>(i + p, j + q);
                    }
                }
            }

            // 太菜了, 居然不会排序了,o(╥﹏╥)o, 只能用最暴力的方法了
            for (int n = 0; n < kernel_size * kernel_size; n++)
            {
                int max = pixel_seq[n];
                int max_ord = n;
                // 确定剩下的里面最大的
                for (int k = n; k < kernel_size * kernel_size; k++)
                {
                    if (pixel_seq[k] > max)
                    {
                        max = pixel_seq[k];
                        max_ord = k;
                    }
                }
                // 逆序排列
                // 交换
                int tmp = pixel_seq[n];
                pixel_seq[n] = max;
                pixel_seq[max_ord] = tmp;
            }
            dst.at<uchar>(i, j) = pixel_seq[(kernel_size * kernel_size + 1) / 2];
        }
    }
}