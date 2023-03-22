# 编译原理B实验文档

本文档用于说明实验内容，并对参考代码作简要说明。

## 一、实验内容

### 1.实验一：词法分析

**实验目的**：将程序文本解析为Token序列。

**实验要求**：补全```nextToken()```函数，读取代码源文件，打印相应Token序列。

### 2.实验二：语法分析

**实验目的**：实现递归LL(1)语法分析。

**实验要求**：补全各文法符号分析函数，读取代码源文件，打印语法树。

### 3.实验三：中间代码生成

**实验目的**：分析程序语义，生成中间代码。

**实验要求**：在各文法符号分析函数中添加语义动作，读取代码源文件，打印生成的中间代码。

## 二、代码说明

实现实验要求方案不唯一，本实验代码仅供参考，可通过向[编译原理B实验代码仓库](https://gitee.com/wangsc18/ustc-compiler-b-lab.git)提交Issue反馈问题。

### 1.文件组织结构

参考代码文件组织结构如下：

```
TinyC
    |---constvar.h
    |---lexicalAnalysis.c
    |---syntaxAnalysis.c
    |---test.txt
    |---TinyC.c
```

### 2.运行方式

Windows系统中，通过命令行进入TinyC文件夹，运行编译命令生成可执行文件并运行。

```powershell
>> gcc constvar.h lexicalAnalysis.c syntaxAnalysis.c TinyC.c -o TinyC.exe
>> TinyC.exe
```