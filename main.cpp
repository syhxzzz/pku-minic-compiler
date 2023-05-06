#include "AST.h"
// #include "function.cpp"
#include "koopa.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
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
void parseAST(char *str);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &ret);
void Visit(const koopa_raw_binary_t &binary);
int main(int argc, const char *argv[]) {
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

  ofstream ofsForResult(outputString);
  if (ofsForResult.is_open()) {
    streambuf *cout_buf = cout.rdbuf();
    cout.rdbuf(ofsForResult.rdbuf());
    parseAST(buffers);
    cout << endl;

    cout.rdbuf(cout_buf);
    ofsForResult.close();
  } else {
    cerr << "Error: Failed to open file " << outputString << endl;
  }
  delete[] buffers;

  return 0;
}

void parseAST(char *str) {
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();

  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 访问所有全局变量
  cout << "  .value\n";
  Visit(raw.values);
  // 访问所有函数
  cout << "  .text\n";
  Visit(raw.funcs);

  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);

  // 处理 raw program
  // ...

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 传入的是 funcs中的一个func
  // 执行一些其他的必要操作
  cout << "  .global " << func->name + 1 << endl;
  cout << func->name + 1 << ":" << endl;
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 传入的是bbs中的一个bb
  // 执行一些其他的必要操作

  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    // 访问 return 指令
    Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER:
    // 访问 integer 指令
    Visit(kind.data.integer);
    break;
  case KOOPA_RVT_BINARY:
    // Visit(kind.data.binary);
    break;
  default:
    // 其他类型暂时遇不到
    cout << kind.tag << endl;
    exit(0);
    assert(false);
  }
}

void Visit(const koopa_raw_return_t &ret) {
  koopa_raw_value_t ret_value = ret.value;
  int32_t int_val = ret_value->kind.data.integer.value;
  // TODO: 重写！！！
  std::cout << "  li a0, " << int_val << endl;
  cout << "  ret\n";
}

void solve_binary(const koopa_raw_binary_t &binary) {
  koopa_raw_value_t l = binary.lhs;
  koopa_raw_value_t r = binary.rhs;
  int lreg, rreg;
  if (binary.op == 6) { // 加法
    cout << "    li t0"
         << ", " << r->kind.data.integer.value << endl;
  } else if (binary.op == 13) {
    //
  }
}

void Visit(const koopa_raw_integer_t &interger) {
  int32_t int_val;
  int_val = interger.value;
  cout << " " << int_val;
}
// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...