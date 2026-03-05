### 一、中文字符

1. `SetConsoleOutputCP(CP_UTF8);`  // 设置控制台输出编码为UTF-8
   `SetConsoleCP(CP_UTF8);`       // 设置控制台输入编码为UTF-8
2. **在 C 语言中：** `char` = 1 字节，**存不下一个中文**（需要用 `char[]` 数组，即多个字节拼起来表示）。

### 二、文件

1. Windows环境，换行符占 2个字节 

   在Windows系统中，文本文件的换行符是 \r\n （回车+换行），其中：

   \r (CR, Carriage Return) = 1字节

   \n (LF, Line Feed) = 1字节

   总共 = 2字节

2. tellg和tellp为获取get流和put流指针的位置，seekg和seekp为该变位置

3. direction：

   ios::beg：流开始位置

   ios::cur：流当前位置

   ios::end：流末尾处位置(文件最后一个字符的下一个位置)

4. getline 后得到的字符串不包含回车和换行符，流指针会指向 当前行的换行符之后的位置 ，也就是 下一行的开始位置 。如果文件已结束后getline，会得到false

5. get读取文件，put写入文件

### 三、进制

1. 十六进制数可以用unit_16来存储，差便是正常的十六进制数的加减
2. 十六进制和十进制都只是计算机中二进制的对外表示，他们可以相乘，因为本质相同