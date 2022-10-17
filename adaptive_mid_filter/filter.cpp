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
/* 窗口内辅助排序 */
int window_order(Mat &src, int rows, int cols, int init_size, int max_size);
/* 自适应中值滤波 */
void adaptive_mid_filter(Mat &src, Mat &dst, int kernel_size, int max_size);

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
    adaptive_mid_filter(bmp, output, 3, 7);
    medianBlur(bmp, example_out, 3);

    // 展示和保存图片
    imwrite("result/中值滤波结果2.png", output);
    imwrite("result/中值滤波OpenCV结果2.png", example_out);

    imshow("original", bmp);
    imshow("adaptive mid value filter", output);
    imshow("mid value by opencv", example_out);
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

int window_order(Mat &src, int rows, int cols, int init_size, int max_size)
// 返回在(rows, cols)点的位置上进行滤波的结果
// init_size为开始滤波的模板大小, max_size为最大可允许滤波的模板大小
{
    int size = init_size;
    int value;
    // 当滤波器模板小于等于可允许最大值时进行
    while (size <= max_size)
    {
        int pixel_seq[size * size];
        int range = (size - 1) / 2;
        // STEP1 获得模板覆盖范围内所有值
        for (int p = 0 - range; p < range + 1; p++)
        {
            for (int q = 0 - range; q < range + 1; q++)
            {
                // 计算正在算的是第几个(其实完全没必要, 直接整个计数器更方便, 就是闲的)
                int ord = (p + range) * size + q + range;
                if (rows + p < 0 || cols + q < 0 || rows + p > src.rows - 1 || cols + q > src.cols - 1)
                {
                    pixel_seq[ord] = 0;
                }
                else
                {
                    pixel_seq[ord] = src.at<uchar>(rows + p, cols + q);
                }
            }
        }
        // STEP2 对所有值进行排序
        for (int n = 0; n < size * size; n++)
        {
            int max = pixel_seq[n];
            int max_ord = n;
            // 确定剩下的里面最大的
            for (int k = n; k < size * size; k++)
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
        // STEP3: 确定终止点是否是椒盐噪声
        int med_min, med_max;
        med_max = pixel_seq[(size * size + 1) / 2] - pixel_seq[0];
        med_min = pixel_seq[(size * size + 1) / 2] - pixel_seq[size * size - 1];
        // STEP4: 如果不是椒盐噪声
        if (med_min > 0 && med_max < 0)
        {
            int pos_min, pos_max;
            pos_max = src.at<uchar>(rows, cols) - pixel_seq[0];
            pos_min = src.at<uchar>(rows, cols) - pixel_seq[size * size - 1];
            // STEP5: 确定该点是不是噪声
            if (pos_min > 0 && pos_max < 0)
            {
                // cout << "return original point" << endl;
                return src.at<uchar>(rows, cols);
            }
            else
            {
                return pixel_seq[(size * size + 1) / 2];
            }
        }
        // STEP4: 如果是椒盐噪声, 继续循环找到不是噪声的点
        else
        {
            size += 2;
            if (size <= max_size)
            {
                // cout << "filter expand" << endl;
                continue;
            }
            else
            {
                value = pixel_seq[(max_size * max_size + 1) / 2];
            }
        }
    }
    return value;
}

void adaptive_mid_filter(Mat &src, Mat &dst, int kernel_size, int max_size)
{
    // 读取以(i, j)为中心的模板, 对模板中的值排序代替原来的点
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            dst.at<uchar>(i, j) = window_order(src, i, j, kernel_size, max_size);
        }
    }
}