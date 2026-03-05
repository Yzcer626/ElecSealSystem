# 中文字符处理经验总结

## 问题背景

在 Windows 平台下开发电子印章系统时，遇到了中文字符处理的典型问题：从命令行传递中文参数时，程序接收到的却是乱码，导致后续处理（编码转换、文件生成等）全部失败。

## 根本原因

### 1. Windows 控制台编码混乱

Windows 系统存在多种编码方式：
- **系统默认编码**：简体中文 Windows 使用 GBK（代码页 936）
- **控制台编码**：可以通过 `chcp` 命令查看和修改
- **程序内部编码**：通常使用 UTF-8

**问题场景**：
```
PowerShell (GBK) → 发送 "快乐" → 程序 (UTF-8) → 解析为 "����"
```

当程序设置 `SetConsoleCP(CP_UTF8)` 后，期望接收 UTF-8 编码的数据，但 PowerShell 仍然发送 GBK 编码，导致编码不匹配。

### 2. C/C++ 标准库的限制

传统的 `main(int argc, char** argv)` 使用窄字符（char）接收参数，在 Windows 上会受到系统编码的影响：
- `argv` 中的字符串编码取决于当前代码页
- 无法直接获取原始的 Unicode 字符

## 解决方案

### 方案一：使用 Windows 宽字符 API（推荐）

```cpp
#include <windows.h>
#include <vector>
#include <string>

// 将宽字符串转换为 UTF-8 编码
std::string wideToUtf8(const wchar_t* wideStr) {
    if (!wideStr) return "";
    
    // 计算所需的 UTF-8 缓冲区大小
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, 
                                       nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) return "";
    
    // 执行转换
    std::vector<char> utf8Buffer(utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, 
                        utf8Buffer.data(), utf8Len, nullptr, nullptr);
    
    return std::string(utf8Buffer.data());
}

int main(int argc, char** argv) {
    std::string inputText;
    
    // 使用 Windows API 获取宽字符命令行参数
    int wideArgc;
    wchar_t** wideArgv = CommandLineToArgvW(GetCommandLineW(), &wideArgc);
    
    if (wideArgv && wideArgc >= 2) {
        inputText = wideToUtf8(wideArgv[1]);
    }
    
    if (wideArgv) LocalFree(wideArgv);
    
    // 现在 inputText 包含正确的 UTF-8 编码字符串
    // ...
}
```

**优点**：
- 绕过控制台编码问题，直接获取 Unicode 字符
- 与系统代码页无关，跨语言环境可用

**缺点**：
- Windows 平台专属代码，降低可移植性

### 方案二：使用 UTF-8 代码页并确保终端一致

```cpp
#include <windows.h>

int main() {
    // 设置控制台为 UTF-8 模式
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // 同时需要确保终端也使用 UTF-8
    // PowerShell: $OutputEncoding = [System.Text.Encoding]::UTF8
    // CMD: chcp 65001
}
```

**注意事项**：
- 需要同时修改程序代码和终端设置
- 不同终端（CMD、PowerShell、Git Bash）设置方式不同

### 方案三：使用文件或环境变量传递参数

将中文内容写入文件，程序从文件读取：

```cpp
std::ifstream inputFile("input.txt", std::ios::binary);
std::string content((std::istreambuf_iterator<char>(inputFile)),
                     std::istreambuf_iterator<char>());
```

**优点**：
- 完全绕过命令行编码问题
- 可以处理大量文本

**缺点**：
- 使用不够便捷
- 需要额外的文件操作

## 文件名处理经验

### 问题：中文文件名导致文件创建失败

Windows 文件系统对文件名有限制，某些字符不能出现在文件名中。当使用中文作为文件名时：
- 可能包含系统保留字符
- 不同文件系统对 Unicode 支持程度不同

### 解决方案：使用编码后的安全文件名

```cpp
#include <sstream>
#include <iomanip>

// 生成安全的文件名（使用编码的十六进制表示）
std::string generateSafeFilename(const std::vector<uint8_t>& hexCode) {
    std::ostringstream oss;
    oss << "U" << std::uppercase << std::hex << std::setfill('0') 
        << std::setw(2) << (int)hexCode[0] 
        << std::setw(2) << (int)hexCode[1];
    return oss.str();
}

// 使用示例：
// 输入：快 (GB2312: 0xBFEC)
// 输出：UBFEC.bmp
```

**优点**：
- 文件名只包含 ASCII 字符，兼容所有文件系统
- 可以从文件名反推出原始字符编码
- 避免中文路径导致的各种问题

## UTF-8 与 GB2312/GBK 转换

### 转换流程

```
UTF-8 字符串 → UTF-16 (宽字符) → GBK/GB2312
```

### 实现代码

```cpp
#include <windows.h>
#include <vector>
#include <string>

// UTF-8 转 GBK
std::string utf8ToGbk(const std::string& utf8Str) {
    if (utf8Str.empty()) return "";
    
    // UTF-8 → UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
    if (wlen == 0) return "";
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wstr.data(), wlen);
    
    // UTF-16 → GBK (代码页 936)
    int len = WideCharToMultiByte(936, 0, wstr.data(), -1, 
                                   nullptr, 0, nullptr, nullptr);
    if (len == 0) return "";
    std::vector<char> gbk(len);
    WideCharToMultiByte(936, 0, wstr.data(), -1, 
                        gbk.data(), len, nullptr, nullptr);
    
    return std::string(gbk.data());
}

// GBK 转 UTF-8
std::string gbkToUtf8(const std::string& gbkStr) {
    if (gbkStr.empty()) return "";
    
    // GBK → UTF-16
    int wlen = MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, nullptr, 0);
    if (wlen == 0) return "";
    std::vector<wchar_t> wstr(wlen);
    MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, wstr.data(), wlen);
    
    // UTF-16 → UTF-8
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, 
                                   nullptr, 0, nullptr, nullptr);
    if (len == 0) return "";
    std::vector<char> utf8(len);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, 
                        utf8.data(), len, nullptr, nullptr);
    
    return std::string(utf8.data());
}
```

## 调试技巧

### 1. 检查字符串的十六进制表示

```cpp
void printHex(const std::string& str, const std::string& label) {
    std::cout << label << ": ";
    for (unsigned char c : str) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << (int)c << " ";
    }
    std::cout << std::dec << std::endl;
}

// 使用示例
printHex(inputText, "Input bytes");
```

### 2. 验证编码转换结果

```cpp
// 检查 GB2312 编码范围（简体中文常用）
bool isValidGb2312(uint8_t high, uint8_t low) {
    // GB2312 编码范围：0xA1A1 - 0xF7FE
    return (high >= 0xA1 && high <= 0xF7) && 
           (low >= 0xA1 && low <= 0xFE);
}
```

### 3. 使用工具验证

- **Notepad++**：查看和转换文件编码
- **HxD**：查看文件十六进制内容
- **chcp**：查看/设置控制台代码页

## 最佳实践总结

1. **程序内部统一使用 UTF-8**
   - 源代码文件保存为 UTF-8 with BOM（Windows）或 UTF-8（Linux/Mac）
   - 字符串字面量使用 UTF-8 编码
   - 文件 I/O 使用二进制模式避免编码转换

2. **边界处进行编码转换**
   - 与操作系统交互时（文件路径、命令行参数）进行必要的编码转换
   - 与外部系统通信时明确编码协议

3. **避免依赖系统默认编码**
   - 不要假设系统使用某种特定编码
   - 显式指定编码参数

4. **测试覆盖多种场景**
   - 不同 Windows 版本（Win7/Win10/Win11）
   - 不同语言环境（中文/英文/日文系统）
   - 不同终端（CMD/PowerShell/Git Bash）

## 参考资源

- [Microsoft Docs: Code Pages](https://docs.microsoft.com/en-us/windows/win32/intl/code-pages)
- [UTF-8 Everywhere](http://utf8everywhere.org/)
- [Windows 控制台 Unicode 支持](https://docs.microsoft.com/en-us/windows/console/setconsoleoutputcp)

---

*总结日期：2026-03-05*
*项目：电子印章系统*
