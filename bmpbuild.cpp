#include "bmpbuild.h"
#include <fstream>

namespace bmp{
    bool DotMatrixToBMP(const gainchar::DotMatrix& matrix, const std::string& outputFile, 
                        const RGBDATA& fgColor, const RGBDATA& bgColor) {
        if (matrix.data.empty() || matrix.width <= 0 || matrix.height <= 0) {
            return false;
        }

        // BMP每行需要4字节对齐
        int rowSize = ((matrix.width * 3 + 3) / 4) * 4;
        int imageSize = rowSize * matrix.height;

        BMPFileHeader fileHeader;
        fileHeader.bfType = 0x4D42; // 'BM'
        fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
        fileHeader.bfReserved1 = 0;
        fileHeader.bfReserved2 = 0;
        fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

        BMPInfoHeader infoHeader;
        infoHeader.biSize = sizeof(BMPInfoHeader);
        infoHeader.biWidth = matrix.width;
        infoHeader.biHeight = matrix.height;
        infoHeader.biPlanes = 1;
        infoHeader.biBitCount = 24;
        infoHeader.biCompression = 0;
        infoHeader.biSizeImage = imageSize;
        infoHeader.biXPelsPerMeter = 0;
        infoHeader.biYPelsPerMeter = 0;
        infoHeader.biClrUsed = 0;
        infoHeader.biClrImportant = 0;

        std::ofstream bmpFile(outputFile, std::ios::binary);
        if (!bmpFile.is_open()) {
            return false;
        }

        // 写入文件头
        bmpFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
        bmpFile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

        // 写入像素数据（BMP是从下到上存储）
        std::vector<uint8_t> row(rowSize, 0);
        for (int y = matrix.height - 1; y >= 0; y--) {
            for (int x = 0; x < matrix.width; x++) {
                if (matrix.data[y][x]) {
                    // 前景色
                    row[x * 3 + 0] = fgColor.rgbBlue;
                    row[x * 3 + 1] = fgColor.rgbGreen;
                    row[x * 3 + 2] = fgColor.rgbRed;
                } else {
                    // 背景色
                    row[x * 3 + 0] = bgColor.rgbBlue;
                    row[x * 3 + 1] = bgColor.rgbGreen;
                    row[x * 3 + 2] = bgColor.rgbRed;
                }
            }
            bmpFile.write(reinterpret_cast<const char*>(row.data()), rowSize);
        }

        bmpFile.close();
        return true;
    }
}