/*********************************************************************************
 * 文件名称: seal_generator.h
 * 功能描述: 四字印章生成器 - 支持多种排列方式
 * 作者: 课程设计
 * 日期: 2026
 *********************************************************************************/

#ifndef SEAL_GENERATOR_H
#define SEAL_GENERATOR_H

#include <string>
#include <vector>
#include "gainchar_bytxt.h"
#include "bmpbuild.h"

namespace seal {

// 印章排列方向枚举
enum class SealDirection {
    LEFT_TO_RIGHT,      // 从左到右 (默认单行)
    TOP_TO_BOTTOM,      // 从上到下 (竖排)
    RIGHT_TO_LEFT,      // 从右到左 (传统)
    DIAGONAL_RL_TB,     // 右上→左上→右下→左下 (原有)
    DIAGONAL_TR_BR_TL_BL // 右上→右下→左上→左下 (新布局)
};

// 印章样式配置结构
struct SealStyle {
    int borderWidth;           // 边框宽度（像素）
    int padding;               // 内边距（像素）
    bmp::RGBDATA borderColor;  // 边框颜色
    bmp::RGBDATA fgColor;      // 文字颜色
    bmp::RGBDATA bgColor;      // 背景颜色
    bool hasBorder;            // 是否显示边框
    bool isRound;              // 是否为圆形印章
    int charSpacing;           // 字符间距
    
    // 默认构造函数
    SealStyle();
};

// 四字印章生成器类
class SealGenerator {
private:
    gainchar::FontFileManager& fontManager;
    SealStyle style;
    
    // 合并四个字符点阵为单个印章点阵（原有：右上→左上→右下→左下）
    gainchar::DotMatrix mergeDiagonalRLTB(
        const gainchar::DotMatrix& topRight,
        const gainchar::DotMatrix& topLeft,
        const gainchar::DotMatrix& bottomRight,
        const gainchar::DotMatrix& bottomLeft
    );
    
    // 合并四个字符点阵为单个印章点阵（新布局：右上→右下→左上→左下）
    gainchar::DotMatrix mergeDiagonalTR_BR_TL_BL(
        const gainchar::DotMatrix& topRight,
        const gainchar::DotMatrix& bottomRight,
        const gainchar::DotMatrix& topLeft,
        const gainchar::DotMatrix& bottomLeft
    );
    
    // 合并点阵 - 从左到右
    gainchar::DotMatrix mergeLeftToRight(
        const std::vector<gainchar::DotMatrix>& matrices
    );
    
    // 合并点阵 - 从上到下
    gainchar::DotMatrix mergeTopToBottom(
        const std::vector<gainchar::DotMatrix>& matrices
    );
    
    // 添加边框
    void addBorder(gainchar::DotMatrix& matrix);
    
    // 调整点阵大小（缩放）
    gainchar::DotMatrix resizeMatrix(const gainchar::DotMatrix& matrix, int newWidth, int newHeight);
    
public:
    // 构造函数
    SealGenerator(gainchar::FontFileManager& fm, const SealStyle& s = SealStyle());
    
    // 生成四字印章（原有布局：右上→左上→右下→左下）
    bool generateFourCharSeal(
        const std::string& char1,  // 右上
        const std::string& char2,  // 左上
        const std::string& char3,  // 右下
        const std::string& char4,  // 左下
        const std::string& outputFile
    );
    
    // 生成四字印章（新布局：右上→右下→左上→左下）
    bool generateFourCharSealNewLayout(
        const std::string& char1,  // 右上
        const std::string& char2,  // 右下
        const std::string& char3,  // 左上
        const std::string& char4,  // 左下
        const std::string& outputFile
    );
    
    // 生成多字印章（支持不同方向）
    bool generateMultiCharSeal(
        const std::vector<std::string>& chars,
        const std::string& outputFile,
        SealDirection direction = SealDirection::LEFT_TO_RIGHT
    );
    
    // 设置样式
    void setStyle(const SealStyle& s);
    
    // 获取当前样式
    SealStyle getStyle() const;
};

// 辅助函数：验证四字输入
bool validateFourChars(const std::vector<std::string>& chars);

// 辅助函数：获取方向描述字符串
std::string getDirectionDescription(SealDirection dir);

// 生成唯一文件名
std::string generateUniqueFilename(const std::string& basePath, const std::string& baseName, const std::string& extension);

} // namespace seal

#endif // SEAL_GENERATOR_H
