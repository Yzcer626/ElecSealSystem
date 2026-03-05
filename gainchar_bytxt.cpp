#include "gainchar_bytxt.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <windows.h>
using namespace std;

namespace gainchar {
    void FontFileManager::analyzeFileStructure() {
        ifstream fontFile(fontFilePath);
        if (!fontFile.is_open()) {
            throw runtime_error("无法打开字体文件");
        }
        string line;
        // 找到第一个 CurCode 行
        while (getline(fontFile, line)) {
            if(line.find("CurCode: ") != string::npos) {
                // 记录第一个字符数据开始的偏移（CurCode 行）
                firstCharOffset = fontFile.tellg();
                break;
            }
        }
        
        // 读取 width 行
        getline(fontFile, line);  // width = X
        
        // 读取第一行点阵数据，计算每行长度
        std::streamoff dataStartLineOffset = fontFile.tellg();
        getline(fontFile, line);
        // Windows 文件使用 CRLF (\r\n) 换行，需要加 2
        // 但为了兼容性，我们通过测量实际偏移来计算
        std::streamoff afterLineOffset = fontFile.tellg();
        bytesPerLine = afterLineOffset - dataStartLineOffset;
        
        // 根据字体文件格式，每个字符占用：
        // 1 行 CurCode + 1 行 width + 56 行点阵数据 + 2 行空行 = 60 行
        linesPerChar = 60;
        
        // 实际点阵高度为 56 行
        dotMatrixHeight = 56;
        
        // 宽度需要从 width 行解析
        // 格式：width = 7 表示 7 组，每组 8 位 = 56 像素
        dotMatrixWidth = 56;
        
        initialized = true;
        fontFile.close();
    }
    
    std::streamoff FontFileManager::calculateOffset(vector<uint8_t> gbCode) const {
        if (!initialized) return -1;
        
        // GB2312 编码范围检查
        if (gbCode[0] < 0xA1 || gbCode[0] > 0xF7 || gbCode[1] < 0xA1 || gbCode[1] > 0xFE) {
            return -1;
        }
        
        // 计算字符在 GB2312 中的索引
        int area = gbCode[0] - 0xA1;  // 区号 (0-93)
        int pos = gbCode[1] - 0xA1;   // 位号 (0-93)
        
        // 跳过 0xAA-0xAF 区（未使用）
        if (gbCode[0] > 0xAA) {
            area -= 6;
        }
        
        // 计算字符索引
        int charIndex = area * 94 + pos;
        
        // 计算文件偏移
        // 每个字符占用 linesPerChar 行，每行 bytesPerLine 字节
        return firstCharOffset + charIndex * linesPerChar * bytesPerLine;
    }
    
    FontFileManager::FontFileManager(const std::string& fontDir) : fontFilePath(fontDir),
        firstCharOffset(0), bytesPerLine(0), linesPerChar(0), 
        dotMatrixWidth(0), dotMatrixHeight(0), initialized(false) {
        analyzeFileStructure();
    }
    
    DotMatrix FontFileManager::findDotMatrix(vector<uint8_t> gbCode) {
        if(!initialized) 
            analyzeFileStructure();
        
        // 构建 CurCode 字符串
        std::ostringstream curCodeStream;
        curCodeStream << "CurCode: " << std::hex << std::setw(2) << std::setfill('0') << (int)gbCode[0] << std::setw(2) << std::setfill('0') << (int)gbCode[1] << std::dec;
        std::string targetCurCode = curCodeStream.str();
        
        ifstream fontFile(fontFilePath);
        if (!fontFile.is_open()) {
            return DotMatrix();
        }
        
        // 在文件中搜索 CurCode
        string line;
        bool found = false;
        while (getline(fontFile, line)) {
            if (line.find(targetCurCode) != string::npos) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            fontFile.close();
            return DotMatrix();
        }
        
        DotMatrix dm;
        dm.width = dotMatrixWidth;    // 56
        dm.height = dotMatrixHeight;  // 56
        dm.hexCode = "";
        dm.chinese = "";
        dm.data = vector<vector<bool>>(dm.height, vector<bool>(dm.width, false));
        
        // 跳过 width 行
        getline(fontFile, line);  // width = X
        
        // 读取 56 行点阵数据
        for(int row = 0; row < dm.height; row++) {
            if (!getline(fontFile, line)) {
                fontFile.close();
                return DotMatrix();
            }
            
            // 解析每行点阵数据
            // 格式：________,________,________,________,________,________,________,
            // 每组 8 个字符，共 7 组，表示 56 个像素
            int col = 0;
            for (size_t i = 0; i < line.length() && col < dm.width; i++) {
                char c = line[i];
                if (c == 'X' || c == '_') {
                    dm.data[row][col] = (c == 'X');
                    col++;
                }
                // 跳过逗号等其他字符
            }
        }
        
        fontFile.close();
        return dm;
    }
    string utf8ToGB2312(const string& utf8Char){
        if (utf8Char.empty()) return {};

        // 第1步：UTF-8 -> UTF-16 （转换为宽字符）
        int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Char.c_str(), -1, nullptr, 0);
        if (wlen == 0) return {};
        std::vector<wchar_t> wstr(wlen);
        MultiByteToWideChar(CP_UTF8, 0, utf8Char.c_str(), -1, wstr.data(), wlen);

        // 第2步：UTF-16 -> GBK
        // 注意：GBK 的代码页是 936，也可以用 CP_ACP（系统默认ANSI代码页，通常就是GBK）
        int len = WideCharToMultiByte(936, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
        if (len == 0) return {};
        std::vector<char> gbk(len);
        WideCharToMultiByte(936, 0, wstr.data(), -1, gbk.data(), len, nullptr, nullptr);
        string str(gbk.data());
        return str;
    }
    vector<uint8_t> gbkToHex(const string& gbkChar){
        return {static_cast<uint8_t>(gbkChar[0]), static_cast<uint8_t>(gbkChar[1])};
    }
    
    vector<string> splitUTF8String(const string& str) {
        std::vector<std::string> characters;
        size_t len = str.length();
        for (size_t i = 0; i < len; ) {
            // 1. 根据首字节判断当前字符占几个字节
            unsigned char c = str[i];
            int charLen = 1;
            if ((c & 0xf8) == 0xf0) { // 1111 0xxx -> 4字节
                charLen = 4;
            } else if ((c & 0xf0) == 0xe0) { // 1110 xxxx -> 3字节
                charLen = 3;
            } else if ((c & 0xe0) == 0xc0) { // 110x xxxx -> 2字节
                charLen = 2;
            }
            // ASCII (0xxx xxxx) 默认为1字节

            // 2. 安全检查，防止越界
            if (i + charLen > len) {
                charLen = 1; // 降级处理，避免程序崩溃
            }

            // 3. 提取完整的字符子串
            characters.push_back(str.substr(i, charLen));
            i += charLen;
        }   
        return characters;
    }


    bool ensureDirectoryExists(const std::string& path) {
        if (path.empty()) {
            return false;
        }
        DWORD attr = GetFileAttributesA(path.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES) {
            return CreateDirectoryA(path.c_str(), NULL) != 0;
        }
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
}