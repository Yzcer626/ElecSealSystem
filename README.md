# 电子印章系统 (ElecSealSystem)

[![C++](https://img.shields.io/badge/C++-11-blue.svg)](https://isocpp.org/)[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows/)

一个基于 C++ 的电子印章生成系统，根据输入的汉字从隶书字体文件生成对应的 BMP 图像。

## 功能特性

- 支持输入多个汉字，批量生成 BMP 图像
- 使用 GB2312 编码的隶书 56x56 点阵字体
- 生成 24 位真彩色 BMP 图像
- 支持自定义前景色和背景色
- 输出文件名使用 GB2312 编码的十六进制表示，避免文件名编码问题

## 项目结构

```
ElecSealSystem/
├── CMakeLists.txt          # CMake 构建配置
├── seal.cpp                # 主程序入口
├── gainchar_bytxt.cpp/h    # 字体文件管理和字符处理
├── bmpbuild.cpp/h          # BMP 图像生成
├── LiShu56.txt             # 隶书字体文件（56x56点阵）
├── bmp/                    # 生成的 BMP 图像输出目录
├── build/                  # 构建输出目录
└── README.md               # 本文件
```

## 系统要求

- **操作系统**: Windows 7/10/11
- **编译器**: MinGW-w64 或 MSVC 2015+
- **CMake**: 3.10 或更高版本
- **C++ 标准**: C++11

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/yourusername/ElecSealSystem.git
cd ElecSealSystem
```

### 2. 构建项目

```bash
# 创建构建目录
mkdir build
cd build

# 生成构建文件（MinGW）
cmake .. -G "MinGW Makefiles"

# 或者生成构建文件（MSVC）
cmake .. -G "Visual Studio 16 2019"

# 编译
cmake --build .
```

### 3. 运行程序

```bash
# 命令行参数方式
.\bin\seal.exe "快乐"

# 交互式输入方式
.\bin\seal.exe
# 然后按提示输入汉字
```

### 4. 查看结果

生成的 BMP 图像将保存在 `bmp/` 目录下，文件名格式为 `UXXXX.bmp`（XXXX 为 GB2312 编码的十六进制表示）。

## 使用示例

### 生成单个汉字

```bash
.\bin\seal.exe "中"
# 输出: bmp/UD6D0.bmp
```

### 生成多个汉字

```bash
.\bin\seal.exe "生日快乐"
# 输出:
#   bmp/UC9FA.bmp  (生)
#   bmp/UBFD5.bmp  (日)
#   bmp/UA5F6.bmp  (快)
#   bmp/UD6C0.bmp  (乐)
```

### 程序输出示例

```
========================================
   电子印章系统
========================================
输入文本: 快乐
字体文件: LiShu56.txt
BMP目录: bmp/
========================================
共检测到 2 个汉字
========================================

[1/2]
----------------------------------------
处理汉字: 快
GB2312编码: 0xbfec
查找点阵数据...
找到点阵数据: 56x56
生成BMP图像: bmp\UBFEC.bmp
汉字 '快' 处理完成！

[2/2]
----------------------------------------
处理汉字: 乐
GB2312编码: 0xc0d6
查找点阵数据...
找到点阵数据: 56x56
生成BMP图像: bmp\UC0D6.bmp
汉字 '乐' 处理完成！

========================================
处理完成！
成功: 2 个
失败: 0 个
========================================
```

## 技术细节

### 字符编码处理

本项目涉及多种字符编码的转换：

1. **输入**: 使用 Windows 宽字符 API 获取 UTF-8 编码的命令行参数
2. **转换**: UTF-8 → UTF-16 → GBK/GB2312
3. **查找**: 在字体文件中搜索对应的 GB2312 编码
4. **输出**: 使用 GB2312 编码的十六进制作为文件名

### BMP 图像格式

- **格式**: 24 位真彩色 BMP
- **尺寸**: 56x56 像素
- **存储**: 从下到上扫描行
- **对齐**: 每行 4 字节对齐

### 字体文件格式

字体文件 `LiShu56.txt` 包含 GB2312 字符集的隶书 56x56 点阵数据：

```
CurCode: XXXX       # 编码标识
width = 7           # 宽度（7组x8位=56像素）
________,________,  # 56行点阵数据
________,________,
...
________,________,

                    # 空行分隔
```

## 常见问题

### Q: 生成的图像无法查看？

A: 确保已安装支持 BMP 格式的图像查看器。Windows 自带的照片应用即可查看。

### Q: 命令行输入中文显示乱码？

A: 这是 Windows 控制台的编码问题。程序已使用宽字符 API 处理，不影响实际功能。建议使用交互式输入方式。

### Q: 某些汉字无法生成？

A: 字体文件只包含 GB2312 字符集的 7478 个常用汉字，部分生僻字可能无法找到。

### Q: 如何修改输出颜色？

A: 修改 `seal.cpp` 中的颜色定义：

```cpp
bmp::RGBDATA fgColor = {0x00, 0x00, 0xFF};  // 红色前景 (BGR格式)
bmp::RGBDATA bgColor = {0xFF, 0xFF, 0xFF};  // 白色背景
//三个变量分别为三原色蓝绿红的分量的十六进制表示，如果想要获取自己想要的颜色，
//可以使用win+shift+s的快捷键选取色器，即可获得光标处的颜色
```

<img src="C:\Users\yzc\AppData\Roaming\Typora\typora-user-images\image-20260305213741293.png" alt="image-20260305213741293" style="zoom:30%;" />

*tip:如图，#后面的36，3b，40分别为十六进制表示的蓝绿红分量*

## 开发文档

详细的开发经验总结和问题解决过程，请参考 [项目经验总结.md](项目经验总结.md)。

## 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库

2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)

3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)

4. 推送到分支 (`git push origin feature/AmazingFeature`)

5. 打开一个 Pull Request

   

## 致谢

- 本项目基于湖南科技大学C语言课程设计任务
- 原作者为湖南科技大学——向德生老师
- 隶书字体数据来源于开源项目
- 感谢字节开发的trae提供的免费ai服务
- 感谢 C++ 社区提供的优秀资源

**注意**: 本项目为课程设计作品，仅供学习交流使用。
