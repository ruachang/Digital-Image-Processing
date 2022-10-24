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
void bmp24bitToMat(ifstream &fpbmp, Mat &bmp, int Offset);
/* 将RGB空间转化成HSI空间 */
void rgbTohsi(Mat &src, Mat &dst);
/* 灰度均衡 */
void histo_equal(Mat &src, Mat &dst);
/* 将HSI空间转化为RGB空间 */
void hsiTorgb(Mat &scr_I, Mat &dst, Mat &dst_I);
int main()
{
    ifstream fpbmp("data4.bmp", ifstream::in | ifstream::binary);
    fpbmp.seekg(0, fpbmp.end);
    int length = fpbmp.tellg();
    fpbmp.seekg(0, fpbmp.beg);
    cout << "Length of the bmp: " << length << endl;

    int Offset, rows, cols;
    // * 初始化信息函数
    bmpFileInfo(fpbmp, Offset, rows, cols);
    Mat bmp(rows, cols, CV_8UC3);
    Mat bmp_I(rows, cols, CV_8UC1);
    bmp24bitToMat(fpbmp, bmp, Offset);
    // 色彩模型转化, 得到I通道的结果
    rgbTohsi(bmp, bmp_I);
    Mat output_I = bmp_I.clone();
    Mat dst = bmp.clone();
    // 对I通道结果进行灰度均衡
    histo_equal(bmp_I, output_I);
    // 将得到的新的I通道转化会RGB通道
    hsiTorgb(bmp_I, dst, output_I);
    imshow("original", bmp);
    imshow("histogram_equalization", dst);

    imwrite("result/灰度均衡结果.png", dst);

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

void bmp24bitToMat(ifstream &fpbmp, Mat &bmp, int Offset)
// * 这个就是正常的载入
{
    // * 从起点之后的多少个开始读, offset为信息头
    fpbmp.seekg(Offset, fpbmp.beg);
    // * 从左到右, 从下到上开始读
    for (int i = (bmp.rows - 1); i >= 0; i--)
    {
        for (int j = 0; j < bmp.cols; j++)
        {
            fpbmp.read((char *)&bmp.at<Vec3b>(i, j)[0], 1);
            fpbmp.read((char *)&bmp.at<Vec3b>(i, j)[1], 1);
            fpbmp.read((char *)&bmp.at<Vec3b>(i, j)[2], 1);
        }
    }
}

void rgbTohsi(Mat &src, Mat &dst)
// 计算src的I空间的值, 返回一张灰度图
{
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            int b_value = src.at<Vec3b>(i, j)[0];
            int g_value = src.at<Vec3b>(i, j)[1];
            int r_value = src.at<Vec3b>(i, j)[2];

            int i_value = 1.0 / 3.0 * (b_value + g_value + r_value);
            dst.at<uchar>(i, j) = i_value;
        }
    }
}

void histo_equal(Mat &src, Mat &dst)
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
    // * 对灰度重新分配
    for (int i = 0; i < 256; i++)
    {
        gray_reapply[i] = uchar(255 * gray_distribute[i] + 0.5);
        // gray_reapply[i] = uchar(255 * gray_distribute[i]);
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            dst.at<uchar>(i, j) = gray_reapply[src.at<uchar>(i, j)];
        }
    }
}

void hsiTorgb(Mat &src_I, Mat &dst, Mat &dst_I)
// 根据新得到的I通道的值修改RBG通道的值
{
    for (int i = 0; i < src_I.rows; i++)
    {
        for (int j = 0; j < src_I.cols; j++)
        {
            double k;
            if (src_I.at<uchar>(i, j) == 0)
            {
                k = 0;
            }
            else
            {
                k = float(dst_I.at<uchar>(i, j)) / src_I.at<uchar>(i, j);
            }
            dst.at<Vec3b>(i, j)[0] *= k;
            dst.at<Vec3b>(i, j)[1] *= k;
            dst.at<Vec3b>(i, j)[2] *= k;
        }
    }
}
