//
// Created by syhxzzz on 23-5-1.
//
#pragma once
#include <cassert>
#include <iostream>
#include <memory>
#include <string>

// 所有 AST 的基类
using namespace std;
static int numCount = 0;
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
  unique_ptr<BaseAST> Exp;
  // TODO：修改Stmt的结构以满足sysy.y的需要
  void Dump() const override {
    Exp->Dump();
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
class PrimaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> p_exp; // 指向具体的primaryExp

  void Dump() const override { p_exp->Dump(); }
};

class UnaryopAST : public BaseAST {
public:
  char op;
  void Dump() const override { cout << op; }
};

class ExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> unaryExp;
  void Dump() const override { unaryExp->Dump(); }
};

class UnaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> unaryExp;
  unique_ptr<BaseAST> unaryOp = nullptr;

  void Dump() const override {
    if (unaryOp) {
      // 此时所指向的是 UnaryExp，需要递归处理
      unaryOp->Dump();
      string name;
      if (numCount) {
        cout << '%' << numCount << "= "
             << "sub 0," << '%' << numCount - 1 << endl;
      } else {
        string oper;
        switch (reinterpret_cast<UnaryopAST *>(unaryOp)->op) {}
      }
    } else {
      // 此时 unaryExp所指向的是一个PrimaryExp
      unaryExp->Dump();
    }
  }
};