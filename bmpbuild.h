#ifndef BMPBUILD_H
#define BMPBUILD_H

#include <cstdint>
#include "gainchar_bytxt.h"

// BMP文件相关类型定义 - 使用标准C++类型确保跨平台兼容性
// 避免与Windows头文件中的类型定义冲突
#ifndef _WINDOWS_H
#ifndef _INC_WINDOWS
typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t  LONG;
#endif
#endif


namespace bmp{
    // BMP文件头结构 - 使用 packed 属性确保无填充字节
    #pragma pack(push, 1)
    struct BMPFileHeader {
        WORD bfType;      // 文件类型，'BM'
        DWORD bfSize;      // 文件大小
        WORD bfReserved1; // 保留
        WORD bfReserved2; // 保留
        DWORD bfOffBits;   // 数据偏移
    };

    struct BMPInfoHeader {
        DWORD biSize;          // 头大小
        LONG biWidth;         // 宽度
        LONG biHeight;        // 高度
        WORD biPlanes;        // 平面数
        WORD biBitCount;      // 位数
        DWORD biCompression;   // 压缩
        DWORD biSizeImage;     // 图像大小
        LONG biXPelsPerMeter; // X分辨率
        LONG biYPelsPerMeter; // Y分辨率
        DWORD biClrUsed;       // 使用颜色数
        DWORD biClrImportant;  // 重要颜色数
    };
    #pragma pack(pop)

    // RGB像素数据结构
    struct RGBDATA {
        BYTE rgbBlue;
        BYTE rgbGreen;
        BYTE rgbRed;    
    };

    // 将点阵数据转换为BMP图像
    bool DotMatrixToBMP(const gainchar::DotMatrix& matrix, const std::string& outputFile, 
                        const RGBDATA& fgColor = {0x00, 0x00, 0x00},  // 默认黑色前景
                        const RGBDATA& bgColor = {0xFF, 0xFF, 0xFF}); // 默认白色背景
}
#endif //BMPBUILD_H