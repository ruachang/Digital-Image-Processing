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
/* Osu阈值分割 */
void osu_split(Mat &src, Mat &dst);
void edge_split(Mat &src, Mat &dst, float thresh);
void multi_split(Mat &src, Mat &dst);
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
    Mat output = bmp.clone();
    Mat edge_output = bmp.clone();
    Mat multi_output = bmp.clone();

    osu_split(bmp, output);
    edge_split(bmp, edge_output, 0.95);
    multi_split(bmp, multi_output);
    imshow("original", bmp);
    imshow("splited pic", output);
    imshow("edge based splited pic", edge_output);
    imshow("multi threshold splited pic", multi_output);
    imwrite("result/Osu阈值分割.png", output);
    imwrite("result/基于边缘信息的阈值分割.png", edge_output);
    imwrite("result/双阈值分割.png", multi_output);

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
    float gray_prob[256];
    // 记录图像总的累计密度
    float gray_distribute[256] = {0};

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
    // STEP1 计算图像的统计特性
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
        if (i - gray_distribute[i] < 0.0000000001)
        {
            gray_distribute[i] = 1;
        }
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
    // STEP2 从所有可能的取值中寻找可能最佳的分界线k, 其中判断标准是类间方差
    float sigma[256] = {0};
    double k_gray[256] = {0};

    for (int i = 255; i >= 0; i--)
    {
        // 计算分界线为i时分类1的灰度均值
        for (int j = 0; j < i; j++)
        {
            k_gray[i] += gray_prob[j] * j;
        }
        // 计算测度
        if (gray_distribute[i] == 1)
        {
            sigma[i] = 0;
        }
        else
        {
            sigma[i] = (avg_gray * gray_distribute[i] - k_gray[i]) * (avg_gray * gray_distribute[i] - k_gray[i]) / (gray_distribute[i] * (1 - gray_distribute[i]));
        }
    }

    // STEP3 确定使得类间方差最大的i值
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
    // STEP4 根据分界线对图像进行分割
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

void edge_split(Mat &src, Mat &dst, float thresh)
{
    //  STEP1 提取拉普拉斯绝对值图像
    // 拉普拉斯算子
    Mat laplace = src.clone();
    Mat src_laplace = src.clone();
    int laplace_kernel = -8;
    int best = 0;
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            int laplace_tmp = 0;
            for (int p = -1; p < 2; p++)
            {
                // 默认拉普拉斯算子的格式为
                // 1          1           1
                // 1    laplace_kernel    1
                // 1          1           1
                for (int q = -1; q < 2; q++)
                {
                    if (i + p < 0 || j + q < 0 || i + p > src.rows - 1 || j + q > src.cols - 1)
                        laplace_tmp += 0;
                    else if (p == 0 && q == 0)
                        laplace_tmp += laplace_kernel * src.at<uchar>(i + p, j + q);
                    else
                        laplace_tmp += src.at<uchar>(i + p, j + q);
                }
            }
            // 取绝对值
            if (laplace_tmp < 0)
                laplace_tmp = -laplace_tmp;
            if (laplace_tmp > best)
                best = laplace_tmp;
            laplace.at<uchar>(i, j) = laplace_tmp;
        }
    }
    // STEP2 规定阈值对图像进行预处理, 得到模板图像
    int pixel_num = laplace.rows * laplace.cols;
    // 记录原图像每个像素值的点数
    int gray_num[256 * 8] = {0};
    // 记录原图像每个像素值的概率
    float gray_prob[256 * 8];
    // 记录图像总的累计密度
    float gray_distribute[256 * 8] = {0};
    for (int i = 0; i < laplace.rows; i++)
    {
        // uchar *pdata = src.ptr<uchar>(i);
        for (int j = 0; j < laplace.cols; j++)
        {
            int value = laplace.at<uchar>(i, j);
            gray_num[value]++;
        }
    }
    // * 计算每种像素值出现的可能性fp
    for (int i = 0; i < 256 * 8; i++)
    {
        gray_prob[i] = float(gray_num[i]) / pixel_num;
    }
    // * 计算概率密度分布PDF
    gray_distribute[0] = gray_prob[0];
    for (int i = 1; i < 256 * 8; i++)
    {
        gray_distribute[i] = gray_prob[i] + gray_distribute[i - 1];
        if (i - gray_distribute[i] < 0.0000000001)
        {
            gray_distribute[i] = 1;
        }
    }
    // 确定对拉普拉斯绝对值图像分割阈值
    int init_k;
    for (int i = 0; i < 256 * 8; i++)
    {
        if (gray_distribute[i] > thresh)
        {
            init_k = i;
            break;
        }
    }
    // STEP3 得到模板图像, 对原图掩膜
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            if (laplace.at<uchar>(i, j) < init_k)
                src_laplace.at<uchar>(i, j) = 0;
        }
    }
    cout << "edge based threshold: the init laplace line is " << init_k << ", ";
    // STEP4 使用Osu阈值分割对掩膜后的进行分割
    osu_split(src_laplace, dst);
}

void multi_split(Mat &src, Mat &dst)
{
    int pixel_num;
    // 记录原图像每个像素值的点数
    int gray_num[256] = {0};
    // 记录原图像每个像素值的概率
    float gray_prob[256];
    // 记录图像总的累计密度
    float gray_distribute[256] = {0};

    int cols, rows;
    cols = src.cols;
    rows = src.rows;
    pixel_num = cols * rows;
    // STEP1 计算图像的统计特性
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
        if (i - gray_distribute[i] < 0.0000000001)
        {
            gray_distribute[i] = 1;
        }
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

    // STEP2 二重循环, 计算每一组可能 的分类的组合对应的方差
    float sigma_k[256][256] = {0};
    for (int i = 0; i < 256; i++)
    {
        for (int j = i; j < 256; j++)
        {
            // 计算三类的概率和灰度均值
            float gray_k[3] = {0}, prob_k[3] = {0};
            for (int k = 0; k < i; k++)
            {
                gray_k[0] += gray_prob[k] * k;
            }
            prob_k[0] = gray_distribute[i];
            for (int k = i; k < j; k++)
            {
                gray_k[1] += gray_prob[k] * k;
                prob_k[1] += gray_prob[k];
            }
            for (int k = j; k < 256; k++)
            {
                gray_k[2] += gray_prob[k] * k;
                prob_k[2] += gray_prob[k];
            }
            for (int k = 0; k < 3; k++)
            {
                // 如果出现概率为0, 则灰度均值必为0
                if (prob_k[k] == 0)
                    gray_k[k] = 0;
                else
                    gray_k[k] /= prob_k[k];
            }
            // 计算对应的类间方差
            for (int k = 0; k < 3; k++)
                sigma_k[i][j] += (gray_k[k] - avg_gray) * (gray_k[k] - avg_gray) * prob_k[k];
        }
    }

    // STEP3 寻找阵列中最大的k1, k2值
    // 寻找第一类固定时最佳的第二类
    int best_k2[256] = {0};
    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            if (sigma_k[i][j] > sigma_k[i][best_k2[i]])
                best_k2[i] = j;
        }
    }
    // 寻找最佳的第一类
    int best_k1 = 0;
    for (int i = 0; i < 256; i++)
    {
        if (sigma_k[i][best_k2[i]] > sigma_k[best_k1][best_k2[i]])
            best_k1 = i;
    }
    float eta = sigma_k[best_k1][best_k2[best_k1]] / avg_sigma;
    cout << "multi threshould: the best k1, k2 is (" << best_k1 << ", " << best_k2[best_k1] << "), the eta is " << eta << ".\n";
    // STEP4 对图像做阈值分割
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (src.at<uchar>(i, j) < best_k1)
            {
                dst.at<uchar>(i, j) = 0;
            }
            else if (src.at<uchar>(i, j) < best_k2[best_k1])
            {
                dst.at<uchar>(i, j) = 128;
            }
            else
            {
                dst.at<uchar>(i, j) = 255;
            }
        }
    }
}