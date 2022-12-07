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
/* 灰度均衡 */
void osu_split(Mat &src, Mat &dst);
int main()
{
    ifstream fpbmp("yeast-cells.bmp", ifstream::in | ifstream::binary);
    fpbmp.seekg(0, fpbmp.end);
    int length = fpbmp.tellg();
    fpbmp.seekg(0, fpbmp.beg);
    cout << "Length of the bmp: " << length << endl;

    int Offset, rows, cols;
    // * 初始化信息函数
    bmpFileInfo(fpbmp, Offset, rows, cols);
    Mat bmp(rows, cols, CV_8UC1);
    bmp8bitToMat(fpbmp, bmp, Offset);
    imshow("original", bmp);
    Mat output = bmp.clone();
    // osu_split(bmp, output);
    // // imshow("original", bmp);
    // imshow("splited pic", output);

    // imwrite("result/灰度均衡结果2.png", output);

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

void osu_split(Mat &src, Mat &dst)
{
    int pixel_num;
    // 记录原图像每个像素值的点数
    int gray_num[256] = {0};
    // 记录原图像每个像素值的概率
    double gray_prob[256];
    // 记录图像总的累计密度
    double gray_distribute[256] = {0};
    // 记录重映射后的值
    int gray_reapply[256];

    int cols, rows;
    cols = src.cols;
    rows = src.rows;
    pixel_num = cols * rows;
    // 统计每个值的个数
    for (int i = 0; i < rows; i++)
    {
        // uchar *pdata = src.ptr<uchar>(i);
        for (int j = 0; j < cols; j++)
        {
            int value = src.at<uchar>(i, j);
            gray_num[value]++;
        }
    }
    // * 计算每种像素值出现的可能性fp
    for (int i = 0; i < 256; i++)
    {
        gray_prob[i] = double(gray_num[i]) / pixel_num;
    }
    // * 计算概率密度分布PDF
    gray_distribute[0] = gray_prob[0];
    for (int i = 1; i < 256; i++)
    {
        gray_distribute[i] = gray_prob[i] + gray_distribute[i - 1];
    }
    // * 计算灰度均值和方差
    double avg_gray, avg_sigma = 0;
    for (int i = 0; i < 256; i++)
    {
        avg_gray += gray_prob[i] * i;
    }
    for (int i = 0; i < 256; i++)
    {
        avg_sigma += gray_prob[i] * (i - avg_gray) * (i - avg_gray);
    }
    // * 从所有可能的取值中寻找可能最佳的分界线k, 其中判断标准是类间方差
    double sigma[256] = {0};
    double k_gray[256] = {0};

    for (int i = 0; i < 256; i++)
    {
        // 计算分界线为i时分类1的灰度均值
        for (int j = 0; j < i; j++)
        {
            k_gray[i] += gray_prob[i] * i;
        }
        // 计算测度
        sigma[i] = (avg_gray * gray_distribute[i] - k_gray[i]) * (avg_gray * gray_distribute[i] - k_gray[i]) / (gray_distribute[i] * (1 - gray_distribute[i]));
    }

    // * 确定使得类间方差最大的i值
    int best_k = 0;
    double best_sigma = sigma[best_k];
    for (int i = 0; i < 256; i++)
    {
        if (best_sigma < sigma[i])
        {
            best_k = i;
            best_sigma = sigma[best_k];
        }
    }
    double eta = best_sigma / avg_sigma;
    cout << "The best line is " << best_k << ", the eta is " << eta << ".\n";
    // * 根据分界线对图像进行分割
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (src.at<uchar>(i, j) < best_k)
            {
                dst.at<uchar>(i, j) = 0;
            }
            else
            {
                dst.at<uchar>(i, j) = 255;
            }
        }
    }
}