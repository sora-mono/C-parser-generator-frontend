﻿// Parser.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#ifdef _DEBUG
#define USE_PRODUCTION_NODE_SHIFT_FUNCTION
#define USE_PRODUCTION_NODE_INIT_FUNCTION
#define USE_PRODUCTION_NODE_REDUCTION_FUNCTION

#endif  // _DEBUG

#include <iostream>

#include "Generator/SyntaxGenerator/process_functions_classes_register.h"
#include "Generator/SyntaxGenerator/syntax_generator_classes_register.h"
#include "SyntaxMachine/syntax_machine.h"

int main() { frontend::parser::syntax_machine::SyntaxMachine syntax_machine; }

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5.
//   转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
