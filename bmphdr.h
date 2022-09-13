/**
    bmphdr.h
*/

#ifndef __BMPHDR_H
#define __BMPHDR_H

#include <stdint.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
// * bmp文件头
// ! 为了保证struct对齐, 需要单独定义BFType
typedef struct tagBITMAPFILEHEADER
{
    // WORD  bfType;
    DWORD bfSize;     // 文件大小
    WORD bfReserved1; // 一般设置为0
    WORD bfReserved2; // 一般设置为0
    DWORD bfOffBits;
} BITMAPFILEHEADER;
// * 文件信息头
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;         // 结构所占字节数
    LONG biWidth;         // 图像宽度
    LONG biHeight;        // 图像高度
    WORD biPlanes;        // 位面信息数, 一般设置为1
    WORD biBitCount;      // 单像素位数
    DWORD biCompression;  // 压缩说明
    DWORD biSizeImage;    // 图像大小, 如果没有压缩, 则可以设置为零
    LONG biXPelsPerMeter; // 水平分辨率
    LONG biYPelsPerMeter; // 垂直分辨率
    DWORD biClrUsed;      // 位图使用颜色数, 如果使用全部颜色, 则设置为0
    DWORD biClrImportant; // 重要颜色数目
} BITMAPINFOHEADER;
// * 调色板
typedef struct tagRGBQUAD
{
    BYTE rgbBlue;     // 调色板中蓝色强度
    BYTE rgbGreen;    // 调色板中绿色强度
    BYTE rgbRed;      // 调色板中红色强度
    BYTE rgbReserved; // 保留, 一般设置为零
} RGBQUAD;
// * ??
typedef struct tagRGBTRIPLE
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} RGBTRIPLE;
// * 位图数据, 扫描读入, 从左下扫描到右上
#endif
/*  bmphdr.h  */