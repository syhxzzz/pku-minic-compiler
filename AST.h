//
// Created by syhxzzz on 23-5-1.
//
#pragma once
#include <iostream>
#include <memory>
#include <string>
// 所有 AST 的基类
class BaseAST {
public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    // std::cout << "CompUnitAST { ";
    func_def->Dump();
    // std::cout << " }";
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  // FuncDefAST(){}

  void Dump() const override {
    std::cout << "fun @";
    std::cout << ident << "():";
    func_type->Dump();
    block->Dump();
  }
};
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "{\n%entry:\n";
    stmt->Dump();
    std::cout << "}\n";
  }
};
class StmtAST : public BaseAST {
public:
  int number;

  void Dump() const override {
    std::cout << "  ret " << number;
    std::cout << "\n";
  }
};
class FuncTypeAST : public BaseAST {
public:
  std::string functype;

  void Dump() const override { std::cout << "i32"; }
};
class NumberAST : public BaseAST {
public:
  int number;

  void Dump() const override { std::cout << number; }
};
