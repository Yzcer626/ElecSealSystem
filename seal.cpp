/*********************************************************************************
 * 文件名称: seal.cpp
 * 功能描述: 电子印章系统 - 根据输入汉字从LiShu56.txt生成BMP图像
 *           功能：四字印章生成、用户交互界面
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
#include <algorithm>
#include <windows.h>
#include "gainchar_bytxt.h"
#include "bmpbuild.h"
#include "seal_generator.h"

using namespace std;

// 图像尺寸定义
#define Height 56             // 图像高度
#define Width 56              // 图像宽度

// 颜色定义 (BGR格式)
#define COLOR_RED    0x0000FF  // 红色
#define COLOR_WHITE  0xFFFFFF  // 白色
#define COLOR_BLACK  0x000000  // 黑色

// 预设颜色结构
struct PresetColor {
    std::string name;
    uint32_t colorValue;  // BGR格式
    uint8_t r, g, b;      // RGB分量
};

// 预设颜色列表（至少5种常用颜色）
const PresetColor PRESET_COLORS[] = {
    {"红色",     0x0000FF, 0xFF, 0x00, 0x00},  // Red
    {"蓝色",     0xFF0000, 0x00, 0x00, 0xFF},  // Blue
    {"绿色",     0x00FF00, 0x00, 0xFF, 0x00},  // Green
    {"黑色",     0x000000, 0x00, 0x00, 0x00},  // Black
    {"白色",     0xFFFFFF, 0xFF, 0xFF, 0xFF},  // White
    {"黄色",     0x00FFFF, 0xFF, 0xFF, 0x00},  // Yellow
    {"紫色",     0xFF00FF, 0xFF, 0x00, 0xFF},  // Purple
    {"青色",     0xFFFF00, 0x00, 0xFF, 0xFF},  // Cyan
    {"橙色",     0x00A5FF, 0xFF, 0xA5, 0x00},  // Orange
    {"深红色",   0x00008B, 0x8B, 0x00, 0x00},  // DarkRed
    {"深蓝色",   0x8B0000, 0x00, 0x00, 0x8B},  // DarkBlue
    {"深绿色",   0x006400, 0x00, 0x64, 0x00}   // DarkGreen
};
const int PRESET_COLOR_COUNT = sizeof(PRESET_COLORS) / sizeof(PRESET_COLORS[0]);

// 当前颜色状态
struct ColorState {
    uint8_t fgR, fgG, fgB;  // 前景色RGB
    uint8_t bgR, bgG, bgB;  // 背景色RGB
    
    ColorState() : fgR(0xFF), fgG(0x00), fgB(0x00),  // 默认红色前景
                   bgR(0xFF), bgG(0xFF), bgB(0xFF) {} // 默认白色背景
    
    bmp::RGBDATA getFgColor() const { return {fgB, fgG, fgR}; }
    bmp::RGBDATA getBgColor() const { return {bgB, bgG, bgR}; }
    
    uint32_t getFgColorBGR() const { return (fgB << 16) | (fgG << 8) | fgR; }
    uint32_t getBgColorBGR() const { return (bgB << 16) | (bgG << 8) | bgR; }
    
    void setFgFromBGR(uint32_t bgr) {
        fgR = bgr & 0xFF;
        fgG = (bgr >> 8) & 0xFF;
        fgB = (bgr >> 16) & 0xFF;
    }
    
    void setBgFromBGR(uint32_t bgr) {
        bgR = bgr & 0xFF;
        bgG = (bgr >> 8) & 0xFF;
        bgB = (bgr >> 16) & 0xFF;
    }
    
    void setFgRGB(uint8_t r, uint8_t g, uint8_t b) {
        fgR = r; fgG = g; fgB = b;
    }
    
    void setBgRGB(uint8_t r, uint8_t g, uint8_t b) {
        bgR = r; bgG = g; bgB = b;
    }
};

// 输出路径配置
#define BMP_DIR_RELATIVE "bmp"     // BMP图像文件输出目录（相对路径）
#define FONT_FILE "LiShu56.txt"     // 字体文件路径（相对路径）

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

// 获取可执行文件所在目录
std::string getExecutableDirectory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of('\\');
    if (lastSlash != std::string::npos) {
        return exePath.substr(0, lastSlash + 1);
    }
    return "";
}

// 获取项目根目录（可执行文件所在目录的上一级）
std::string getProjectRootDirectory() {
    std::string exeDir = getExecutableDirectory();
    if (exeDir.empty()) {
        return "";
    }
    std::string targetDir = "elec_seal_system\\";
    size_t pos = exeDir.rfind(targetDir);
    if (pos != std::string::npos) {
        return exeDir.substr(0, pos + targetDir.length());
    }
    return exeDir;
}

// 生成安全的文件名（使用GB2312编码的十六进制表示）
std::string generateSafeFilename(const std::vector<uint8_t>& hexCode) {
    std::ostringstream oss;
    oss << "U" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (int)hexCode[0] << std::setw(2) << (int)hexCode[1];
    return oss.str();
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

// 印章类型枚举
enum class SealType {
    SINGLE,         // 单字模式
    FOUR_CHAR_NEW,  // 四字印章（新布局：右上→右下→左上→左下）
    MULTI_CHAR,     // 多字印章
    EXIT            // 退出
};

// 显示主菜单
void showMainMenu() {
    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║              电子印章系统 - 印章类型选择                 ║" << endl;
    cout << "╠══════════════════════════════════════════════════════════╣" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [1] 单字印章模式                                        ║" << endl;
    cout << "║      - 每个汉字生成独立的BMP图像                         ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [2] 四字印章模式（右上→右下→左上→左下）                 ║" << endl;
    cout << "║      - 四个汉字排列成2x2印章                             ║" << endl;
    cout << "║      - 输入顺序：右上→右下→左上→左下                     ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [3] 多字印章模式（横向排列）                            ║" << endl;
    cout << "║      - 多个汉字横向排列生成一个印章                      ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [4] 多字印章模式（竖向排列）                            ║" << endl;
    cout << "║      - 多个汉字竖向排列生成一个印章                      ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [5] 颜色设置                                            ║" << endl;
    cout << "║      - 设置印章前景色和背景色                            ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║  [0] 退出程序                                            ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;
    cout << "\n请选择印章类型 [0-5]: ";
}

// 验证十六进制字符串（00-FF范围）
bool isValidHexByte(const std::string& hexStr) {
    if (hexStr.length() != 2) {
        return false;
    }
    for (char c : hexStr) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

// 将十六进制字符串转换为uint8_t
uint8_t hexToUint8(const std::string& hexStr) {
    int value;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> value;
    return static_cast<uint8_t>(value);
}

// 将uint8_t转换为十六进制字符串
std::string uint8ToHex(uint8_t value) {
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(value);
    return ss.str();
}

// 显示预设颜色列表
void showPresetColors() {
    cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║                    预设颜色选择                          ║" << endl;
    cout << "╠══════════════════════════════════════════════════════════╣" << endl;
    cout << "║                                                          ║" << endl;

    for (int i = 0; i < PRESET_COLOR_COUNT; i++) {
        // 计算编号宽度（1或2位）
        int numWidth = (i + 1 < 10) ? 1 : 2;
        cout << "║  [" << (i + 1) << "] "
             << std::left << std::setw(8) << std::setfill(' ') << PRESET_COLORS[i].name
             << " - RGB("
             << uint8ToHex(PRESET_COLORS[i].r) << ","
             << uint8ToHex(PRESET_COLORS[i].g) << ","
             << uint8ToHex(PRESET_COLORS[i].b) << ")";
        // 补齐空格使边框对齐（总宽度52字符）
        // 内容长度 = [ + n + ] + 空格 + 名称(8) + 空格 + - RGB( + 6个十六进制 + , + ) = 4 + numWidth + 1 + 8 + 1 + 7 + 6 + 1 = 28 + numWidth
        int contentLen = 28 + numWidth;
        int padding = 52 - contentLen;
        for (int j = 0; j < padding; j++) cout << " ";
        cout << "       ║" << endl;
    }

    cout << "║                                                          ║" << endl;
    cout << "║  [0] 自定义RGB颜色                                       ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;
}

// 显示当前颜色状态（在边框内）
void showCurrentColors(const ColorState& state) {
    cout << "║  当前颜色设置：                                          ║" << endl;
    // 前景色行: "║    前景色(文字): RGB(FF,00,00)" 共 32 字符，需补 20 空格
    cout << "║    前景色(文字): RGB(" << uint8ToHex(state.fgR) << ","
         << uint8ToHex(state.fgG) << "," << uint8ToHex(state.fgB) << ")";
    int padding1 = 20;
    for (int j = 0; j < padding1; j++) cout << " ";
    cout << "       ║" << endl;
    // 背景色行: "║    背景色:       RGB(FF,FF,FF)" 共 32 字符，需补 20 空格
    cout << "║    背景色:       RGB(" << uint8ToHex(state.bgR) << ","
         << uint8ToHex(state.bgG) << "," << uint8ToHex(state.bgB) << ")";
    int padding2 = 20;
    for (int j = 0; j < padding2; j++) cout << " ";
    cout << "       ║" << endl;
}

// 输入RGB十六进制值（带验证）
bool inputRGBValues(uint8_t& r, uint8_t& g, uint8_t& b) {
    std::string rStr, gStr, bStr;
    
    cout << "\n请输入RGB颜色值（十六进制，范围00-FF）：" << endl;
    
    // 输入R值
    while (true) {
        cout << "  红色(R) [00-FF]: ";
        cin >> rStr;
        if (isValidHexByte(rStr)) {
            break;
        }
        cout << "  错误：请输入有效的十六进制值（00-FF）！" << endl;
    }
    
    // 输入G值
    while (true) {
        cout << "  绿色(G) [00-FF]: ";
        cin >> gStr;
        if (isValidHexByte(gStr)) {
            break;
        }
        cout << "  错误：请输入有效的十六进制值（00-FF）！" << endl;
    }
    
    // 输入B值
    while (true) {
        cout << "  蓝色(B) [00-FF]: ";
        cin >> bStr;
        if (isValidHexByte(bStr)) {
            break;
        }
        cout << "  错误：请输入有效的十六进制值（00-FF）！" << endl;
    }
    
    r = hexToUint8(rStr);
    g = hexToUint8(gStr);
    b = hexToUint8(bStr);
    
    return true;
}

// 颜色设置菜单
void colorSettingsMenu(ColorState& state) {
    bool inColorMenu = true;

    while (inColorMenu) {
        cout << "\n╔══════════════════════════════════════════════════════════╗" << endl;
        cout << "║                    颜色设置菜单                          ║" << endl;
        cout << "╠══════════════════════════════════════════════════════════╣" << endl;
        showCurrentColors(state);
        cout << "╠══════════════════════════════════════════════════════════╣" << endl;
        cout << "║  [1] 设置前景色（文字颜色）                              ║" << endl;
        cout << "║  [2] 设置背景色                                          ║" << endl;
        cout << "║  [3] 恢复默认颜色（红色文字，白色背景）                  ║" << endl;
        cout << "║  [0] 返回主菜单                                          ║" << endl;
        cout << "╚══════════════════════════════════════════════════════════╝" << endl;
        cout << "\n请选择操作 [0-3]: ";
        
        string choice;
        cin >> choice;
        
        if (choice == "0") {
            inColorMenu = false;
        } else if (choice == "1") {
            // 设置前景色
            showPresetColors();
            cout << "\n请选择颜色 [1-" << PRESET_COLOR_COUNT << " 或 0]: ";

            string colorChoice;
            cin >> colorChoice;

            int colorIdx = atoi(colorChoice.c_str());
            if (colorIdx >= 1 && colorIdx <= PRESET_COLOR_COUNT) {
                // 使用预设颜色
                const PresetColor& preset = PRESET_COLORS[colorIdx - 1];
                state.setFgRGB(preset.r, preset.g, preset.b);
                cout << "\n前景色已设置为：" << preset.name
                     << " RGB(" << uint8ToHex(preset.r) << ", "
                     << uint8ToHex(preset.g) << ", " << uint8ToHex(preset.b) << ")" << endl;
            } else if (colorIdx == 0) {
                // 自定义RGB
                uint8_t r, g, b;
                if (inputRGBValues(r, g, b)) {
                    state.setFgRGB(r, g, b);
                    cout << "\n前景色已设置为：RGB(" << uint8ToHex(r) << ", "
                         << uint8ToHex(g) << ", " << uint8ToHex(b) << ")" << endl;
                }
            } else {
                cout << "\n无效的选择！" << endl;
            }
        } else if (choice == "2") {
            // 设置背景色
            showPresetColors();
            cout << "\n请选择颜色 [1-" << PRESET_COLOR_COUNT << " 或 0]: ";

            string colorChoice;
            cin >> colorChoice;

            int colorIdx = atoi(colorChoice.c_str());
            if (colorIdx >= 1 && colorIdx <= PRESET_COLOR_COUNT) {
                // 使用预设颜色
                const PresetColor& preset = PRESET_COLORS[colorIdx - 1];
                state.setBgRGB(preset.r, preset.g, preset.b);
                cout << "\n背景色已设置为：" << preset.name
                     << " RGB(" << uint8ToHex(preset.r) << ", "
                     << uint8ToHex(preset.g) << ", " << uint8ToHex(preset.b) << ")" << endl;
            } else if (colorIdx == 0) {
                // 自定义RGB
                uint8_t r, g, b;
                if (inputRGBValues(r, g, b)) {
                    state.setBgRGB(r, g, b);
                    cout << "\n背景色已设置为：RGB(" << uint8ToHex(r) << ", "
                         << uint8ToHex(g) << ", " << uint8ToHex(b) << ")" << endl;
                }
            } else {
                cout << "\n无效的选择！" << endl;
            }
        } else if (choice == "3") {
            // 恢复默认
            state = ColorState();
            cout << "\n颜色已恢复为默认值！" << endl;
        } else {
            cout << "\n无效的选择，请重新输入！" << endl;
        }
    }
}

// 显示帮助信息
void showHelp() {
    cout << "\n========================================" << endl;
    cout << "   电子印章系统 - 使用帮助" << endl;
    cout << "========================================" << endl;
    cout << "命令格式:" << endl;
    cout << "  seal.exe [选项] [输入文本]" << endl;
    cout << "\n选项:" << endl;
    cout << "  -h, --help          显示帮助信息" << endl;
    cout << "\n交互模式:" << endl;
    cout << "  直接运行程序将进入交互模式，可以选择印章类型" << endl;
    cout << "\n四字印章输入顺序说明:" << endl;
    cout << "  布局：  [左上] [右上]" << endl;
    cout << "          [左下] [右下]" << endl;
    cout << "  输入顺序：右上→右下→左上→左下" << endl;
    cout << "========================================" << endl;
}

// 询问用户是否继续
bool askToContinue() {
    cout << "\n----------------------------------------" << endl;
    cout << "是否继续创建新的印章？" << endl;
    cout << "[1] 是 - 继续创建" << endl;
    cout << "[0] 否 - 退出程序" << endl;
    cout << "----------------------------------------" << endl;
    cout << "请选择 [0-1]: ";
    
    string choice;
    cin >> choice;
    
    return (choice == "1" || choice == "y" || choice == "Y" || choice == "yes");
}

// 处理单个汉字：查找点阵数据、保存点阵文件、生成BMP
bool processSingleChar(const string& chinese, gainchar::FontFileManager& fontManager,
                    const string& bmpDir, const ColorState& colorState, bool useUniqueFilename = true) {
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

    // 生成BMP文件
    string bmpFile;
    if (useUniqueFilename) {
        // 使用唯一文件名避免覆盖
        string safeFilename = generateSafeFilename(hexCode);
        bmpFile = seal::generateUniqueFilename(bmpDir, safeFilename, ".bmp");
    } else {
        string safeFilename = generateSafeFilename(hexCode);
        bmpFile = bmpDir + safeFilename + ".bmp";
    }

    cout << "生成BMP图像: " << bmpFile << endl;

    // 使用用户设置的颜色
    bmp::RGBDATA fgColor = colorState.getFgColor();
    bmp::RGBDATA bgColor = colorState.getBgColor();

    if (!bmp::DotMatrixToBMP(matrix, bmpFile, fgColor, bgColor)) {
        cerr << "错误：生成BMP文件失败" << endl;
        return false;
    }

    cout << "汉字 '" << chinese << "' 处理完成！" << endl;
    return true;
}

// 处理多个汉字
void processMultipleChars(const string& input, gainchar::FontFileManager& fontManager,
                          const string& bmpDir, const ColorState& colorState, bool useUniqueFilename = true) {
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
        if (processSingleChar(chars[i], fontManager, bmpDir, colorState, useUniqueFilename)) {
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

// 生成四字印章（新布局：右上→右下→左上→左下）
bool generateFourCharSealNewLayout(const vector<string>& chars, gainchar::FontFileManager& fontManager,
                          const string& bmpDir, const ColorState& colorState) {
    if (chars.size() != 4) {
        cerr << "错误：四字印章需要恰好4个汉字，当前提供了 " << chars.size() << " 个" << endl;
        return false;
    }

    cout << "\n========================================" << endl;
    cout << "   生成四字印章（右上→右下→左上→左下）" << endl;
    cout << "========================================" << endl;
    cout << "布局说明：" << endl;
    cout << "  [左上: " << chars[2] << "] [右上: " << chars[0] << "]" << endl;
    cout << "  [左下: " << chars[3] << "] [右下: " << chars[1] << "]" << endl;
    cout << "----------------------------------------" << endl;
    cout << "输入顺序：" << endl;
    cout << "  第1字(右上): " << chars[0] << endl;
    cout << "  第2字(右下): " << chars[1] << endl;
    cout << "  第3字(左上): " << chars[2] << endl;
    cout << "  第4字(左下): " << chars[3] << endl;

    // 创建样式并使用用户设置的颜色
    seal::SealStyle style;
    style.fgColor = colorState.getFgColor();
    style.bgColor = colorState.getBgColor();
    style.borderColor = colorState.getFgColor();  // 边框使用前景色

    seal::SealGenerator generator(fontManager, style);

    // 使用唯一文件名
    string outputFile = seal::generateUniqueFilename(bmpDir, "seal_4char", ".bmp");

    if (!generator.generateFourCharSealNewLayout(chars[0], chars[1], chars[2], chars[3], outputFile)) {
        cerr << "错误：生成四字印章失败" << endl;
        return false;
    }

    cout << "\n四字印章生成成功！" << endl;
    cout << "输出文件: " << outputFile << endl;
    cout << "========================================" << endl;

    return true;
}

// 生成多字横向印章
bool generateMultiCharSealHorizontal(const vector<string>& chars, gainchar::FontFileManager& fontManager,
                          const string& bmpDir, const ColorState& colorState) {
    if (chars.empty()) {
        cerr << "错误：未检测到有效的中文字符" << endl;
        return false;
    }

    cout << "\n========================================" << endl;
    cout << "   生成多字印章（从左到右）" << endl;
    cout << "========================================" << endl;
    cout << "文字内容: ";
    for (const auto& ch : chars) {
        cout << ch;
    }
    cout << endl;

    // 创建样式并使用用户设置的颜色
    seal::SealStyle style;
    style.fgColor = colorState.getFgColor();
    style.bgColor = colorState.getBgColor();
    style.borderColor = colorState.getFgColor();

    seal::SealGenerator generator(fontManager, style);

    // 使用唯一文件名
    string outputFile = seal::generateUniqueFilename(bmpDir, "seal_multi_h", ".bmp");

    if (!generator.generateMultiCharSeal(chars, outputFile, seal::SealDirection::LEFT_TO_RIGHT)) {
        cerr << "错误：生成多字印章失败" << endl;
        return false;
    }

    cout << "多字印章生成成功！" << endl;
    cout << "输出文件: " << outputFile << endl;
    cout << "========================================" << endl;

    return true;
}

// 生成多字竖向印章
bool generateMultiCharSealVertical(const vector<string>& chars, gainchar::FontFileManager& fontManager,
                          const string& bmpDir, const ColorState& colorState) {
    if (chars.empty()) {
        cerr << "错误：未检测到有效的中文字符" << endl;
        return false;
    }

    cout << "\n========================================" << endl;
    cout << "   生成多字印章（从上到下）" << endl;
    cout << "========================================" << endl;
    cout << "文字内容: ";
    for (const auto& ch : chars) {
        cout << ch;
    }
    cout << endl;

    // 创建样式并使用用户设置的颜色
    seal::SealStyle style;
    style.fgColor = colorState.getFgColor();
    style.bgColor = colorState.getBgColor();
    style.borderColor = colorState.getFgColor();

    seal::SealGenerator generator(fontManager, style);

    // 使用唯一文件名
    string outputFile = seal::generateUniqueFilename(bmpDir, "seal_multi_v", ".bmp");

    if (!generator.generateMultiCharSeal(chars, outputFile, seal::SealDirection::TOP_TO_BOTTOM)) {
        cerr << "错误：生成多字印章失败" << endl;
        return false;
    }

    cout << "多字印章生成成功！" << endl;
    cout << "输出文件: " << outputFile << endl;
    cout << "========================================" << endl;

    return true;
}

// 交互模式主循环
void interactiveMode(gainchar::FontFileManager& fontManager, const string& bmpDir) {
    bool running = true;
    ColorState colorState;  // 颜色状态

    while (running) {
        showMainMenu();

        string choice;
        cin >> choice;

        if (choice == "0") {
            cout << "\n感谢使用电子印章系统，再见！" << endl;
            running = false;
            continue;
        }

        string inputText;

        if (choice == "1") {
            // 单字模式
            cout << "\n请输入中文文本（支持多个汉字）: ";
            cin >> inputText;

            if (!inputText.empty() && inputText != "q") {
                processMultipleChars(inputText, fontManager, bmpDir, colorState, true);
            }

        } else if (choice == "2") {
            // 四字印章模式（新布局）
            cout << "\n========================================" << endl;
            cout << "四字印章输入说明：" << endl;
            cout << "  布局：  [左上] [右上]" << endl;
            cout << "          [左下] [右下]" << endl;
            cout << "  请按以下顺序输入4个汉字：" << endl;
            cout << "  第1字 → 右上位置" << endl;
            cout << "  第2字 → 右下位置" << endl;
            cout << "  第3字 → 左上位置" << endl;
            cout << "  第4字 → 左下位置" << endl;
            cout << "========================================" << endl;
            cout << "请输入4个汉字: ";
            cin >> inputText;

            vector<string> chars = gainchar::splitUTF8String(inputText);

            if (chars.size() == 4) {
                generateFourCharSealNewLayout(chars, fontManager, bmpDir, colorState);
            } else {
                cerr << "错误：需要恰好4个汉字，您输入了 " << chars.size() << " 个" << endl;
            }

        } else if (choice == "3") {
            // 多字横向印章
            cout << "\n请输入中文文本（多个汉字将横向排列）: ";
            cin >> inputText;

            vector<string> chars = gainchar::splitUTF8String(inputText);

            if (!chars.empty()) {
                generateMultiCharSealHorizontal(chars, fontManager, bmpDir, colorState);
            } else {
                cerr << "错误：未检测到有效的中文字符" << endl;
            }

        } else if (choice == "4") {
            // 多字竖向印章
            cout << "\n请输入中文文本（多个汉字将竖向排列）: ";
            cin >> inputText;

            vector<string> chars = gainchar::splitUTF8String(inputText);

            if (!chars.empty()) {
                generateMultiCharSealVertical(chars, fontManager, bmpDir, colorState);
            } else {
                cerr << "错误：未检测到有效的中文字符" << endl;
            }

        } else if (choice == "5") {
            // 颜色设置
            colorSettingsMenu(colorState);
            continue;  // 跳过询问是否继续

        } else {
            cout << "\n无效的选择，请重新输入！" << endl;
            continue;
        }

        // 询问是否继续
        if (running) {
            running = askToContinue();
            if (!running) {
                cout << "\n感谢使用电子印章系统，再见！" << endl;
            }
        }
    }
}

// 主函数
int main(int argc, char** argv){
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    std::string projectRoot = getProjectRootDirectory();
    std::string bmpDir = projectRoot + BMP_DIR_RELATIVE;
    std::string fontFile = projectRoot + FONT_FILE;
    
    if (!ensureDirectoryExists(bmpDir)) {
        cerr << "错误：无法创建BMP目录: " << bmpDir << endl;
        return 1;
    }
    
    // 检查是否有命令行参数
    bool hasCommandLineArgs = false;
    
    // 使用Windows API获取宽字符命令行参数
    int wideArgc;
    wchar_t** wideArgv = CommandLineToArgvW(GetCommandLineW(), &wideArgc);
    
    if (wideArgc > 1) {
        hasCommandLineArgs = true;
        
        // 解析命令行参数
        for (int i = 1; i < wideArgc; i++) {
            string arg = wideToUtf8(wideArgv[i]);
            
            if (arg == "-h" || arg == "--help") {
                showHelp();
                LocalFree(wideArgv);
                return 0;
            }
        }
    }
    
    if (wideArgv) {
        LocalFree(wideArgv);
    }
    
    // 显示欢迎信息
    cout << "╔══════════════════════════════════════════════════════════╗" << endl;
    cout << "║                                                          ║" << endl;
    cout << "║              欢迎使用电子印章系统                        ║" << endl;
    cout << "║              版本: 1.1.0                                 ║" << endl;
    cout << "║                                                          ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════╝" << endl;
    cout << "\n字体文件: " << fontFile << endl;
    cout << "输出目录: " << bmpDir << endl;
    
    // 初始化字体文件管理器
    gainchar::FontFileManager fontManager(fontFile);
    
    // 进入交互模式
    interactiveMode(fontManager, bmpDir);
    
    return 0;
}
