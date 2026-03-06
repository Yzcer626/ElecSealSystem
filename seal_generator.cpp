/*********************************************************************************
 * 文件名称: seal_generator.cpp
 * 功能描述: 四字印章生成器实现 - 支持多种排列方式
 * 作者: 课程设计
 * 日期: 2026
 *********************************************************************************/

#include <windows.h>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "seal_generator.h"

namespace seal {

// 默认样式构造函数
SealStyle::SealStyle() 
    : borderWidth(4)
    , padding(4)
    , borderColor({0x00, 0x00, 0xFF})  // 红色边框
    , fgColor({0x00, 0x00, 0xFF})      // 红色文字
    , bgColor({0xFF, 0xFF, 0xFF})      // 白色背景
    , hasBorder(true)
    , isRound(false)
    , charSpacing(4)
{
}

// 构造函数
SealGenerator::SealGenerator(gainchar::FontFileManager& fm, const SealStyle& s)
    : fontManager(fm)
    , style(s)
{
}

// 设置样式
void SealGenerator::setStyle(const SealStyle& s) {
    style = s;
}

// 获取当前样式
SealStyle SealGenerator::getStyle() const {
    return style;
}

// 调整点阵大小（简单最近邻缩放）
gainchar::DotMatrix SealGenerator::resizeMatrix(const gainchar::DotMatrix& matrix, int newWidth, int newHeight) {
    gainchar::DotMatrix result;
    result.width = newWidth;
    result.height = newHeight;
    result.chinese = matrix.chinese;
    result.hexCode = matrix.hexCode;
    result.data = std::vector<std::vector<bool>>(newHeight, std::vector<bool>(newWidth, false));
    
    if (matrix.data.empty() || matrix.width == 0 || matrix.height == 0) {
        return result;
    }
    
    double scaleX = static_cast<double>(matrix.width) / newWidth;
    double scaleY = static_cast<double>(matrix.height) / newHeight;
    
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            int srcX = static_cast<int>(x * scaleX);
            int srcY = static_cast<int>(y * scaleY);
            srcX = std::min(srcX, matrix.width - 1);
            srcY = std::min(srcY, matrix.height - 1);
            result.data[y][x] = matrix.data[srcY][srcX];
        }
    }
    
    return result;
}

// 合并四个字符点阵为单个印章点阵（原有：右上→左上→右下→左下）
gainchar::DotMatrix SealGenerator::mergeDiagonalRLTB(
    const gainchar::DotMatrix& topRight,
    const gainchar::DotMatrix& topLeft,
    const gainchar::DotMatrix& bottomRight,
    const gainchar::DotMatrix& bottomLeft
) {
    gainchar::DotMatrix result;
    
    // 计算印章总尺寸
    int charWidth = topRight.width;
    int charHeight = topRight.height;
    int spacing = style.charSpacing;
    int border = style.hasBorder ? style.borderWidth * 2 : 0;
    int pad = style.padding * 2;
    
    result.width = charWidth * 2 + spacing + border + pad;
    result.height = charHeight * 2 + spacing + border + pad;
    result.height = std::max(result.width, result.height);
    result.width = result.height;
    
    result.data = std::vector<std::vector<bool>>(
        result.height, 
        std::vector<bool>(result.width, false)
    );
    
    int offsetX = style.hasBorder ? style.borderWidth + style.padding : style.padding;
    int offsetY = offsetX;
    
    // 右上 (char1)
    int trX = offsetX + charWidth + spacing;
    int trY = offsetY;
    
    // 左上 (char2)
    int tlX = offsetX;
    int tlY = offsetY;
    
    // 右下 (char3)
    int brX = offsetX + charWidth + spacing;
    int brY = offsetY + charHeight + spacing;
    
    // 左下 (char4)
    int blX = offsetX;
    int blY = offsetY + charHeight + spacing;
    
    auto copyMatrix = [&](const gainchar::DotMatrix& src, int startX, int startY) {
        for (int y = 0; y < src.height && (startY + y) < result.height; y++) {
            for (int x = 0; x < src.width && (startX + x) < result.width; x++) {
                if ((startY + y) >= 0 && (startX + x) >= 0) {
                    result.data[startY + y][startX + x] = src.data[y][x];
                }
            }
        }
    };
    
    copyMatrix(topRight, trX, trY);
    copyMatrix(topLeft, tlX, tlY);
    copyMatrix(bottomRight, brX, brY);
    copyMatrix(bottomLeft, blX, blY);
    
    if (style.hasBorder) {
        addBorder(result);
    }
    
    return result;
}

// 合并四个字符点阵为单个印章点阵（新布局：右上→右下→左上→左下）
gainchar::DotMatrix SealGenerator::mergeDiagonalTR_BR_TL_BL(
    const gainchar::DotMatrix& topRight,
    const gainchar::DotMatrix& bottomRight,
    const gainchar::DotMatrix& topLeft,
    const gainchar::DotMatrix& bottomLeft
) {
    gainchar::DotMatrix result;
    
    // 计算印章总尺寸
    int charWidth = topRight.width;
    int charHeight = topRight.height;
    int spacing = style.charSpacing;
    int border = style.hasBorder ? style.borderWidth * 2 : 0;
    int pad = style.padding * 2;
    
    result.width = charWidth * 2 + spacing + border + pad;
    result.height = charHeight * 2 + spacing + border + pad;
    result.height = std::max(result.width, result.height);
    result.width = result.height;
    
    result.data = std::vector<std::vector<bool>>(
        result.height, 
        std::vector<bool>(result.width, false)
    );
    
    int offsetX = style.hasBorder ? style.borderWidth + style.padding : style.padding;
    int offsetY = offsetX;
    
    // 新布局：
    // [左上 char3] [右上 char1]
    // [左下 char4] [右下 char2]
    
    // 右上 (char1)
    int trX = offsetX + charWidth + spacing;
    int trY = offsetY;
    
    // 右下 (char2)
    int brX = offsetX + charWidth + spacing;
    int brY = offsetY + charHeight + spacing;
    
    // 左上 (char3)
    int tlX = offsetX;
    int tlY = offsetY;
    
    // 左下 (char4)
    int blX = offsetX;
    int blY = offsetY + charHeight + spacing;
    
    auto copyMatrix = [&](const gainchar::DotMatrix& src, int startX, int startY) {
        for (int y = 0; y < src.height && (startY + y) < result.height; y++) {
            for (int x = 0; x < src.width && (startX + x) < result.width; x++) {
                if ((startY + y) >= 0 && (startX + x) >= 0) {
                    result.data[startY + y][startX + x] = src.data[y][x];
                }
            }
        }
    };
    
    copyMatrix(topRight, trX, trY);
    copyMatrix(bottomRight, brX, brY);
    copyMatrix(topLeft, tlX, tlY);
    copyMatrix(bottomLeft, blX, blY);
    
    if (style.hasBorder) {
        addBorder(result);
    }
    
    return result;
}

// 添加边框
void SealGenerator::addBorder(gainchar::DotMatrix& matrix) {
    int bw = style.borderWidth;
    
    if (style.isRound) {
        int cx = matrix.width / 2;
        int cy = matrix.height / 2;
        int radius = std::min(cx, cy) - bw / 2;
        
        for (int y = 0; y < matrix.height; y++) {
            for (int x = 0; x < matrix.width; x++) {
                int dx = x - cx;
                int dy = y - cy;
                int dist = static_cast<int>(std::sqrt(dx * dx + dy * dy));
                
                if (dist >= radius - bw && dist <= radius) {
                    matrix.data[y][x] = true;
                }
                else if (dist > radius) {
                    matrix.data[y][x] = false;
                }
            }
        }
    } else {
        for (int y = 0; y < matrix.height; y++) {
            for (int x = 0; x < matrix.width; x++) {
                if (y < bw || y >= matrix.height - bw || x < bw || x >= matrix.width - bw) {
                    matrix.data[y][x] = true;
                }
            }
        }
    }
}

// 合并点阵 - 从左到右
gainchar::DotMatrix SealGenerator::mergeLeftToRight(
    const std::vector<gainchar::DotMatrix>& matrices
) {
    gainchar::DotMatrix result;
    
    if (matrices.empty()) {
        return result;
    }
    
    int totalWidth = 0;
    int maxHeight = 0;
    int spacing = style.charSpacing;
    
    for (const auto& m : matrices) {
        totalWidth += m.width + spacing;
        maxHeight = std::max(maxHeight, m.height);
    }
    totalWidth -= spacing;
    
    int border = style.hasBorder ? style.borderWidth * 2 : 0;
    int pad = style.padding * 2;
    
    result.width = totalWidth + border + pad;
    result.height = maxHeight + border + pad;
    result.data = std::vector<std::vector<bool>>(
        result.height, 
        std::vector<bool>(result.width, false)
    );
    
    int offsetX = style.hasBorder ? style.borderWidth + style.padding : style.padding;
    int offsetY = (result.height - maxHeight) / 2;
    
    int currentX = offsetX;
    for (const auto& m : matrices) {
        for (int y = 0; y < m.height; y++) {
            for (int x = 0; x < m.width; x++) {
                int destY = offsetY + y;
                int destX = currentX + x;
                if (destY >= 0 && destY < result.height && destX >= 0 && destX < result.width) {
                    result.data[destY][destX] = m.data[y][x];
                }
            }
        }
        currentX += m.width + spacing;
    }
    
    if (style.hasBorder) {
        addBorder(result);
    }
    
    return result;
}

// 合并点阵 - 从上到下
gainchar::DotMatrix SealGenerator::mergeTopToBottom(
    const std::vector<gainchar::DotMatrix>& matrices
) {
    gainchar::DotMatrix result;
    
    if (matrices.empty()) {
        return result;
    }
    
    int maxWidth = 0;
    int totalHeight = 0;
    int spacing = style.charSpacing;
    
    for (const auto& m : matrices) {
        maxWidth = std::max(maxWidth, m.width);
        totalHeight += m.height + spacing;
    }
    totalHeight -= spacing;
    
    int border = style.hasBorder ? style.borderWidth * 2 : 0;
    int pad = style.padding * 2;
    
    result.width = maxWidth + border + pad;
    result.height = totalHeight + border + pad;
    result.data = std::vector<std::vector<bool>>(
        result.height, 
        std::vector<bool>(result.width, false)
    );
    
    int offsetX = (result.width - maxWidth) / 2;
    int offsetY = style.hasBorder ? style.borderWidth + style.padding : style.padding;
    
    int currentY = offsetY;
    for (const auto& m : matrices) {
        for (int y = 0; y < m.height; y++) {
            for (int x = 0; x < m.width; x++) {
                int destY = currentY + y;
                int destX = offsetX + x;
                if (destY >= 0 && destY < result.height && destX >= 0 && destX < result.width) {
                    result.data[destY][destX] = m.data[y][x];
                }
            }
        }
        currentY += m.height + spacing;
    }
    
    if (style.hasBorder) {
        addBorder(result);
    }
    
    return result;
}

// 生成四字印章（原有布局：右上→左上→右下→左下）
bool SealGenerator::generateFourCharSeal(
    const std::string& char1,  // 右上
    const std::string& char2,  // 左上
    const std::string& char3,  // 右下
    const std::string& char4,  // 左下
    const std::string& outputFile
) {
    auto getMatrix = [&](const std::string& ch) -> gainchar::DotMatrix {
        std::string gbChar = gainchar::utf8ToGB2312(ch);
        if (gbChar.empty()) {
            return gainchar::DotMatrix();
        }
        std::vector<uint8_t> hexCode = gainchar::gbkToHex(gbChar);
        gainchar::DotMatrix matrix = fontManager.findDotMatrix(hexCode);
        matrix.chinese = ch;
        return matrix;
    };
    
    gainchar::DotMatrix tr = getMatrix(char1);
    gainchar::DotMatrix tl = getMatrix(char2);
    gainchar::DotMatrix br = getMatrix(char3);
    gainchar::DotMatrix bl = getMatrix(char4);
    
    if (tr.data.empty() || tl.data.empty() || br.data.empty() || bl.data.empty()) {
        return false;
    }
    
    gainchar::DotMatrix seal = mergeDiagonalRLTB(tr, tl, br, bl);
    return bmp::DotMatrixToBMP(seal, outputFile, style.fgColor, style.bgColor);
}

// 生成四字印章（新布局：右上→右下→左上→左下）
bool SealGenerator::generateFourCharSealNewLayout(
    const std::string& char1,  // 右上
    const std::string& char2,  // 右下
    const std::string& char3,  // 左上
    const std::string& char4,  // 左下
    const std::string& outputFile
) {
    auto getMatrix = [&](const std::string& ch) -> gainchar::DotMatrix {
        std::string gbChar = gainchar::utf8ToGB2312(ch);
        if (gbChar.empty()) {
            return gainchar::DotMatrix();
        }
        std::vector<uint8_t> hexCode = gainchar::gbkToHex(gbChar);
        gainchar::DotMatrix matrix = fontManager.findDotMatrix(hexCode);
        matrix.chinese = ch;
        return matrix;
    };
    
    gainchar::DotMatrix tr = getMatrix(char1);  // 右上
    gainchar::DotMatrix br = getMatrix(char2);  // 右下
    gainchar::DotMatrix tl = getMatrix(char3);  // 左上
    gainchar::DotMatrix bl = getMatrix(char4);  // 左下
    
    if (tr.data.empty() || br.data.empty() || tl.data.empty() || bl.data.empty()) {
        return false;
    }
    
    gainchar::DotMatrix seal = mergeDiagonalTR_BR_TL_BL(tr, br, tl, bl);
    return bmp::DotMatrixToBMP(seal, outputFile, style.fgColor, style.bgColor);
}

// 生成多字印章（支持不同方向）
bool SealGenerator::generateMultiCharSeal(
    const std::vector<std::string>& chars,
    const std::string& outputFile,
    SealDirection direction
) {
    if (chars.empty()) {
        return false;
    }
    
    std::vector<gainchar::DotMatrix> matrices;
    for (const auto& ch : chars) {
        std::string gbChar = gainchar::utf8ToGB2312(ch);
        if (gbChar.empty()) {
            continue;
        }
        std::vector<uint8_t> hexCode = gainchar::gbkToHex(gbChar);
        gainchar::DotMatrix matrix = fontManager.findDotMatrix(hexCode);
        if (!matrix.data.empty()) {
            matrix.chinese = ch;
            matrices.push_back(matrix);
        }
    }
    
    if (matrices.empty()) {
        return false;
    }
    
    gainchar::DotMatrix seal;
    switch (direction) {
        case SealDirection::LEFT_TO_RIGHT:
            seal = mergeLeftToRight(matrices);
            break;
        case SealDirection::TOP_TO_BOTTOM:
            seal = mergeTopToBottom(matrices);
            break;
        case SealDirection::DIAGONAL_RL_TB:
            if (matrices.size() >= 4) {
                seal = mergeDiagonalRLTB(matrices[0], matrices[1], matrices[2], matrices[3]);
            } else {
                return false;
            }
            break;
        case SealDirection::DIAGONAL_TR_BR_TL_BL:
            if (matrices.size() >= 4) {
                seal = mergeDiagonalTR_BR_TL_BL(matrices[0], matrices[1], matrices[2], matrices[3]);
            } else {
                return false;
            }
            break;
        default:
            seal = mergeLeftToRight(matrices);
            break;
    }
    
    return bmp::DotMatrixToBMP(seal, outputFile, style.fgColor, style.bgColor);
}

// 验证四字输入
bool validateFourChars(const std::vector<std::string>& chars) {
    return chars.size() == 4;
}

// 获取方向描述字符串
std::string getDirectionDescription(SealDirection dir) {
    switch (dir) {
        case SealDirection::LEFT_TO_RIGHT:
            return "从左到右";
        case SealDirection::TOP_TO_BOTTOM:
            return "从上到下";
        case SealDirection::RIGHT_TO_LEFT:
            return "从右到左";
        case SealDirection::DIAGONAL_RL_TB:
            return "右上→左上→右下→左下";
        case SealDirection::DIAGONAL_TR_BR_TL_BL:
            return "右上→右下→左上→左下";
        default:
            return "未知";
    }
}

// 生成唯一文件名
std::string generateUniqueFilename(const std::string& basePath, const std::string& baseName, const std::string& extension) {
    // 首先尝试基础文件名
    std::string fullPath = basePath + baseName + extension;
    
    // 检查文件是否存在
    DWORD attr = GetFileAttributesA(fullPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        // 文件不存在，可以使用
        return fullPath;
    }
    
    // 文件已存在，添加时间戳和序号
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    std::ostringstream oss;
    oss << baseName << "_" 
        << std::setfill('0') << std::setw(4) << st.wYear
        << std::setw(2) << st.wMonth
        << std::setw(2) << st.wDay << "_"
        << std::setw(2) << st.wHour
        << std::setw(2) << st.wMinute
        << std::setw(2) << st.wSecond;
    
    fullPath = basePath + oss.str() + extension;
    
    // 再次检查（以防万一）
    attr = GetFileAttributesA(fullPath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        // 添加毫秒和随机数确保唯一性
        oss << "_" << st.wMilliseconds << "_" << (rand() % 1000);
        fullPath = basePath + oss.str() + extension;
    }
    
    return fullPath;
}

} // namespace seal
