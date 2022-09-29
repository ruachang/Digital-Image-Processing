# 常用的OpenCV C++ 的接口或者是常用函数的作用

## 读取某个图像的某个像素点的值

* 方法1: 直接使用自定义的方法: 
  * 读取(x, y)点的: `src.at<data_type>(x, y)`
  * 其中`data_type`与实际图像的类型有关, 例如单通道的就是`uchar`

## 某些函数的接口

### 滤波器

* 中值滤波: `medianBlur(Mat &src, Mat &dst, int ksize)` 
  * 使用大小为 `kernel_size` 的滤波器模板对图像进行中值滤波, 会起到 ***模糊*** 的作用
  * 要求输入为 ***单通道, 三通道或者四通道***, 输出的数据类型和大小和输入一样
  * kernel_size 必须是 ***大于1的奇数***
* 高斯滤波: `GaussianBlur(Mat &src, Mat &dst, Size(length, width), X_sigma, Y_sigma)`
  * 使用大小为 `Size: length * width` 的滤波器模板对图像进行高斯滤波, 理论上是低通, 去掉尖锐的部分, 也就是 ***模糊***
### 灰度直方图

* 直方图均衡: `equalizeHist(Mat &src, Mat &dst)`
  * 对图像进行灰度直方图均衡, 可以起到调节亮度的作用

