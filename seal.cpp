/*********************************************************************************
 * 文件名称: seal.cpp
 * 功能描述: 电子印章系统 - 根据输入汉字从LiShu56.txt生成BMP图像
 * 作者: 课程设计
 * 日期: 2026
 *********************************************************************************/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <cstdint>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>
#include <windows.h>
#include "gainchar_bytxt.h"
#include "bmpbuild.h"

using namespace std;

// 图像尺寸定义
#define Height 56             // 图像高度
#define Width 56              // 图像宽度

// 颜色定义 (BGR格式)
#define COLOR_RED    0x0000FF  // 红色
#define COLOR_WHITE  0xFFFFFF  // 白色
#define COLOR_BLACK  0x000000  // 黑色

// 输出路径配置
#define BMP_DIR "bmp/"              // BMP图像文件输出目录
#define FONT_FILE "LiShu56.txt"     // 字体文件路径

bool ensureDirectoryExists(const string& dir) {
    if (dir.empty()) {
        return false;
    }
    DWORD attr = GetFileAttributesA(dir.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return CreateDirectoryA(dir.c_str(), NULL) != 0;
    }
    return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

// 生成安全的文件名（使用GB2312编码的十六进制表示）
std::string generateSafeFilename(const std::vector<uint8_t>& hexCode) {
    std::ostringstream oss;
    oss << "U" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (int)hexCode[0] << std::setw(2) << (int)hexCode[1];
    return oss.str();
}

// 处理单个汉字：查找点阵数据、保存点阵文件、生成BMP
bool processSingleChar(const string& chinese, gainchar::FontFileManager& fontManager, 
                    const string& bmpDir) {
    if (chinese.empty()) {
        return false;
    }
    
    cout << "\n----------------------------------------" << endl;
    cout << "处理汉字: " << chinese << endl;
    
    // 将UTF-8汉字转换为GB2312编码
    string gbChar = gainchar::utf8ToGB2312(chinese);
    if (gbChar.empty()) {
        cerr << "错误：无法将汉字 '" << chinese << "' 转换为GB2312编码" << endl;
        return false;
    }
    
    vector<uint8_t> hexCode = gainchar::gbkToHex(gbChar);
    cout << "GB2312编码: 0x" << hex << setw(2) << setfill('0') << (int)hexCode[0] << setw(2) << setfill('0') << (int)hexCode[1] << dec << endl;
    
    // 查找点阵数据
    cout << "查找点阵数据..." << endl;
    gainchar::DotMatrix matrix = fontManager.findDotMatrix(hexCode);
    
    if (matrix.data.empty()) {
        cerr << "错误：未找到汉字 '" << chinese << "' 的点阵数据" << endl;
        return false;
    }
    
    matrix.chinese = chinese;
    cout << "找到点阵数据: " << matrix.width << "x" << matrix.height << endl;
    
    // 生成BMP文件 - 使用安全的文件名（GB2312编码的十六进制）
    string safeFilename = generateSafeFilename(hexCode);
    string bmpFile = string(bmpDir) + "\\" + safeFilename + ".bmp";
    cout << "生成BMP图像: " << bmpFile << endl;
    
    bmp::RGBDATA fgColor = {0x00, 0x00, 0xFF};  // 红色前景
    bmp::RGBDATA bgColor = {0xFF, 0xFF, 0xFF};  // 白色背景
    
    if (!bmp::DotMatrixToBMP(matrix, bmpFile, fgColor, bgColor)) {
        cerr << "错误：生成BMP文件失败" << endl;
        return false;
    }
    
    cout << "汉字 '" << chinese << "' 处理完成！" << endl;
    return true;
}

// 处理多个汉字
void processMultipleChars(const string& input, gainchar::FontFileManager& fontManager,
                          const string& bmpDir) {
    // 分割UTF-8字符串为单个汉字
    vector<string> chars = gainchar::splitUTF8String(input);
    
    if (chars.empty()) {
        cerr << "错误：未检测到有效的中文字符" << endl;
        return;
    }
    
    cout << "========================================" << endl;
    cout << "共检测到 " << chars.size() << " 个汉字" << endl;
    cout << "========================================" << endl;
    
    int successCount = 0;
    int failCount = 0;
    
    for (size_t i = 0; i < chars.size(); i++) {
        cout << "\n[" << (i + 1) << "/" << chars.size() << "] ";
        if (processSingleChar(chars[i], fontManager, bmpDir)) {
            successCount++;
        } else {
            failCount++;
        }
    }
    
    cout << "\n========================================" << endl;
    cout << "处理完成！" << endl;
    cout << "成功: " << successCount << " 个" << endl;
    cout << "失败: " << failCount << " 个" << endl;
    cout << "========================================" << endl;
}

// 将宽字符串转换为UTF-8编码的string
std::string wideToUtf8(const wchar_t* wideStr) {
    if (!wideStr) return "";
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) return "";
    std::vector<char> utf8Buffer(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Buffer.data(), utf8Len, nullptr, nullptr);
    return std::string(utf8Buffer.data());
}

int main(int argc, char** argv){
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    if (!ensureDirectoryExists(BMP_DIR)) {
        cerr << "错误：无法创建BMP目录" << endl;
        return 1;
    }
    
    string inputText;
    
    // 如果命令行提供了参数，使用参数；否则提示用户输入
    // 使用Windows API获取宽字符命令行参数，避免编码问题
    if (argc >= 2) {
        int wideArgc;
        wchar_t** wideArgv = CommandLineToArgvW(GetCommandLineW(), &wideArgc);
        if (wideArgv && wideArgc >= 2) {
            inputText = wideToUtf8(wideArgv[1]);
        }
        if (wideArgv) LocalFree(wideArgv);
    } else {
        cout << "========================================" << endl;
        cout << "   电子印章系统 - 汉字点阵图生成器" << endl;
        cout << "========================================" << endl;
        cout << "请输入中文文本（支持多个汉字，或输入q退出）: " << endl;
        cout << "> ";
        cin >> inputText;
        if (inputText == "q" || inputText == "Q") {
            return 0;
        }
    }
    
    if (inputText.empty()) {
        cerr << "错误：输入为空" << endl;
        return 1;
    }
    
    cout << "========================================" << endl;
    cout << "   电子印章系统" << endl;
    cout << "========================================" << endl;
    cout << "输入文本: " << inputText << endl;
    cout << "字体文件: " << FONT_FILE << endl;
    cout << "BMP目录: " << BMP_DIR << endl;
    
    // 初始化字体文件管理器
    gainchar::FontFileManager fontManager(FONT_FILE);

    // 处理输入的汉字
    processMultipleChars(inputText, fontManager, BMP_DIR);
    
    return 0;
}
