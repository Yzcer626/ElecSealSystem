#ifndef GAINCHAR_BYTXT_H
#define GAINCHAR_BYTXT_H

#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace gainchar {


// 点阵数据结构
struct DotMatrix {
    int width;
    int height;
    std::string hexCode;  // GB2312十六进制编码
    std::string chinese;  // 对应的汉字
    std::vector<std::vector<bool>> data; // true表示有像素，false表示无像素
};

// ============================================
// 字体文件直接定位管理类
// ============================================
// LiShu56.txt文件编码规律：
// - 编码范围：0xA1A1 - 0xF7FE（GB2312标准）
// - 每个字符固定占用60行：
//   * 第1行：CurCode: XXXX（编码标识）
//   * 第2行：width = 7（宽度定义）
//   * 第3-58行：56行点阵数据
//   * 第59-60行：空行（分隔符）
// - 文件起始偏移：第5行开始第一个字符（前4行为文件头）
// ============================================
class FontFileManager {
private:
    std::string fontFilePath;
    std::streamoff firstCharOffset;  // 第一个字符的文件偏移量
    int bytesPerLine;                // 每行字节数（包括换行符）
    int linesPerChar;                // 每个字符占用的行数
    int dotMatrixWidth;              // 点阵宽度（像素）
    int dotMatrixHeight;             // 点阵高度（像素）
    bool initialized;
    
    // 计算编码对应的文件偏移量
    std::streamoff calculateOffset(std::vector<uint8_t> gbCode) const;
    
    // 分析文件结构
    void analyzeFileStructure();
    
public:
    // 构造函数
    FontFileManager(const std::string& fontPath);
    ~FontFileManager() {}
    
    // 根据GB2312编码查找点阵数据（直接定位，O(1)时间复杂度）
    DotMatrix findDotMatrix(std::vector<uint8_t> gbCode);
};

// 将UTF-8汉字转换为GB2312编码（返回GBK字符串）
std::string utf8ToGB2312(const std::string& utf8Char);

// 将GBK字符串转换为字节数组
std::vector<uint8_t> gbkToHex(const std::string& gbkChar);

// 将UTF-8字符串分割成单个汉字字符
std::vector<std::string> splitUTF8String(const std::string& utf8Str);


// 确保目录存在（如果不存在则创建）
bool ensureDirectoryExists(const std::string& path);

}

#endif // GAINCHAR_BYTXT_H
