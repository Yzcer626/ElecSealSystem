# BMP彩色图像文件格式

#### 一、位图文件头

位图文件头结构BITMAPFILEHEADER包含位图文件的类型大小信息和版面信息。

结构如下：

```c++
 typedef struct tagBITMAPFILEHEADER {  // bmfh
     WORD bfType;
     DWORD bfSize;
     WORD bfReserved1;
     WORD bfReserved1;
     DWORD bfOffBits;
 } BITMAPFILEHEADER;
```

 这个结构的长度是固定的，为14个字节，各个域的说明如下： 

 bfType：指定文件类型，必须是0x4D42，即字符串"BM"。也就是说所有BMP文件 的头两个字节都是"BM"。 

 bfSize：指定整个文件的大小（以字节为单位）。 

 bfReserved1：保留，一般为0。 

 bfReserved2：保留，一般为0。 

 bfOffBits：指定从文件头到实际的位图像素数据首部的字节偏移量。即图5中前 3个部分的长度之和。

### 二、位图信息头

位图信息头结构BITMAPINFOHEADER包含图像本身的属性。其定义如下： 

```c++
typedef struct tagBITMAPINFOHEADER // bmih{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
}BITMAPINFOHEADER;
```

这个结构的长度是固定的，为40个字节，各个域的说明如下： 

 biSize：指定 BITMAPINFOHEADER 结构的长度，为40个字节。 

 biWidth：指定位图的宽度（以象素为单位）。 

 biHeight：指定位图的高度（以象素为单位）。 

 biPlanes：指定目标设备的位面数。这个成员变量的值必须为1。

  biBitCount：指定每个象素的位数。常用的值为1（黑白二色图）、4（16色图） 、8（256色图）、24（真彩色图）。 

 biCompression：指定压缩位图的压缩类型。有效的值为BI_RGB(0), BI_RLE8(1), BI_RLE4(2), BI_BITFIELDS(3)。用得不多，在24位格式中，该变量被设置为0。 

 biSizeImage：指定图像的大小（以字节为单位）。如果位图的格式是BI_RGB， 则将此成员变量设置为0是有效的。该值可以根据biWidth'和biHeight的乘积计算 出来。要注意的是：上述公式中的biWidth'必须是4的整倍数（所以计算乘积时写 的是biWidth'，表示大于或等于biWidth的、离4最近的整倍数。例如，若 biWidth=240，则biWidth'=240；若biWidth=241，则biWidth'=244）。 

 biXPelsPerMeter：为位图指定目标设备的水平分辨率（以"象素/米"为单位）。 

 biYPelsPerMeter：为位图指定目标设备的垂直分辨率（以"象素/米"为单位）。和上面的都一般为0

 biClrUsed：指定位图实际用到的颜色数。如果该值为0，则用到的颜色数为2的 biBitCount次方。 

 biClrImportant：指定对位图的显示有重要影响的颜色数。如果此值为0，则所 有颜色都很重要。

### 三、图像数据

```c++
typedef struct tagRGBDATA{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
}RGBDATA;
```

对于24位真彩图，三个字节表示一个像素的颜色

(1) BMP文件按从下到上，从左到右的顺序存储图像数据。即从文件中最先读到的是图 像最下面一行的左边第一个像素，然后是左边第二个像素……接下来是倒数第二行左边第 一个像素，左边第二个像素……依次类推，最后得到的是最上面一行的最右一个像素。 

(2) 对于24位色真彩色位图而言，数据的排列顺序以图像的左下角为起点，从左到右 、从下到上，每连续3个字节便描述图像一个像素点的颜色信息，这三个字节分别代表蓝、 绿、红三基色在此像素中的亮度，若某连续三个字节为：00H，00H，FFH，则表示该像素的 颜色为红色。



### 补充

1. 在 Windows 编程中，这些类型通常定义如下：(包含于<windows.h>文件中)

   ​	WORD ：16位无符号整数（2字节）

   ​	DWORD ：32位无符号整数（4字节）

   ​	LONG ：32位有符号整数（4字节）

   ​	BYTE ：8位无符号整数（1字节）

2. 为了防止因为系统自动的内存对齐使文件头和信息头的数据被打乱格式，可用

   `\#pragma pack(push, 1)   // 保存当前对齐设置，并设置为 1 字节对齐`和`\#pragma pack(pop)     // 恢复之前保存的对齐设置`来保证格式有效（push为了使对齐设置压栈保存

3. int main(int argc, char *argv[]）

