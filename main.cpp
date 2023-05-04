#include "AST.h"
// #include "function.cpp"
#include "koopa.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <string>
using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
void parse_AST(const char *);
int main(int argc, const char *argv[]) {
  // 这段代码没看懂，记得搜
  // TODO()!!!
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];
  string inputString = input;
  string outputString = output;
  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  if (mode[1] == 'k') {
    ofstream ofs(outputString);
    if (ofs.is_open()) {
      // 此时应该把标准输出流重定向至 ofs,并保留好标准输出流副本cout_buf
      streambuf *cout_buf = cout.rdbuf();
      cout.rdbuf(ofs.rdbuf());
      ast->Dump();
      cout << endl;

      // 将标准输出流恢复先
      cout.rdbuf(cout_buf);
      ofs.close();
    } else {
      cerr << "Error: Failed to open file " << outputString << endl;
    }
    return 0;
  }
  // 将输出打印到所指定的文件中
  // freopen("compiler-output.txt", "w", stdout);
  // ast->Dump();
  // freopen("/dev/tty", "w", stdout);
  string middleResult = "AST.txt";
  ofstream ofs(middleResult);
  if (ofs.is_open()) {
    // 流程如上所述
    streambuf *cout_buf = cout.rdbuf();
    cout.rdbuf(ofs.rdbuf());
    ast->Dump();
    cout << endl;

    // 将标准输出流恢复先
    cout.rdbuf(cout_buf);
    ofs.close();
  } else {
    cerr << "Error: Failed to open file " << outputString << endl;
  }
  // 然后恢复stdout

  ifstream infile(middleResult);
  char *buffers = nullptr;
  if (infile) {
    infile.seekg(0, infile.end);
    int length = infile.tellg();
    infile.seekg(0, infile.beg);
    buffers = new char[length];
    infile.read(buffers, length);
  } else {
    cout << "Failed to open file:" << output << endl;
  }
  freopen(output, "w", stdout);
  parse_AST(buffers);
  delete[] buffers;

  return 0;
}

void parse_AST(const char *str) {
  // 解析字符串 str, 得到 Koopa IR 程序
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS); // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

  std::cout << "  .text\n";

  for (size_t i = 0; i < raw.funcs.len; ++i) {
    // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
    // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
    assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
    // 获取当前函数
    koopa_raw_function_t func = (koopa_raw_function_t)raw.funcs.buffer[i];

    cout << "  .globl " << func->name + 1 << endl;
    cout << func->name + 1 << endl;
    for (size_t j = 0; j < func->bbs.len; ++j) {
      // 说明 func->bbs是一个basic_block
      assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
      koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)func->bbs.buffer[j];
      for (size_t k = 0; k < bb->insts.len; k++) {
        koopa_raw_value_t value = (koopa_raw_value_t)bb->insts.buffer[k];
        assert(value->kind.tag == KOOPA_RVT_RETURN);
        koopa_raw_value_t ret_value = value->kind.data.ret.value;
        int32_t int_val = ret_value->kind.data.integer.value;
        // assert(int_val == 0);
        std::cout << "  li a0," << int_val << std::endl;
        std::cout << "  ret";
      }
    }
  }
  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  // 处理 raw program
  // ...

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}