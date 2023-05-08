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
static int returnValue;
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
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " } ";
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
    cout << "FuncDefAST { ";
    func_type->Dump();
    cout << " , " << ident << " , ";
    block->Dump();
    cout << " } ";
  }
};
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    cout << "BlockAST { ";
    stmt->Dump();
    cout << " } ";
  }
};
class StmtAST : public BaseAST {
public:
  unique_ptr<BaseAST> Exp;
  void Dump() const override {
    cout << "Stmt {";
    Exp->Dump();
    cout << " } ";
  }
};
class FuncTypeAST : public BaseAST {
public:
  std::string functype;

  void Dump() const override { cout << "FuncTypeAST { " << functype << " }"; }
};
class NumberExpAST : public BaseAST {
public:
  int number;

  void Dump() const override {
    cout << "  %" << numCount << " = add 0, " << number << endl;
    numCount++;
  }
};

class PrimaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> p_exp; // 指向具体的primaryExp

  void Dump() const override { p_exp->Dump(); }
};

class ExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> LOrExp;
  void Dump() const override { LOrExp->Dump(); }
};

class LOrExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> LAndExp;
  unique_ptr<BaseAST> LOrExp = nullptr;
  void Dump() const override {
    if (LOrExp) {
      LOrExp->Dump();
    }
    LAndExp->Dump();
    if (LOrExp) {
      cout << "  " << '%' << numCount << " = "
           << "lor"
           << " %" << numCount - 1 << ", " << '%' << numCount - 2 << endl;
      numCount++;
    }
  }
};

class LAndExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> EqExp;
  unique_ptr<BaseAST> LAndExp = nullptr;
  int left;
  void Dump() const override {
    if (LAndExp) {
      LAndExp->Dump();
    }
    EqExp->Dump();
    if (LAndExp) {
      cout << "  " << '%' << numCount << " = "
           << "and"
           << " %" << numCount - 1 << ", " << '%' << numCount - 2 << endl;
      numCount++;
    }
  }
};

class EqExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> RelExp;
  unique_ptr<BaseAST> EqExp = nullptr;
  string oper;
  void Dump() const override {
    if (EqExp) {
      EqExp->Dump();
    }
    RelExp->Dump();
    if (EqExp) {
      cout << "  " << '%' << numCount << " = " << oper << " %" << numCount - 1
           << ", " << '%' << numCount - 2 << endl;
      numCount++;
    }
  }
};

class RelExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> AddExp;
  unique_ptr<BaseAST> RelExp = nullptr;
  string oper;
  void Dump() const override {
    if (RelExp) {
      RelExp->Dump();
    }
    AddExp->Dump();
    if (RelExp) {
      cout << "  " << '%' << numCount << " = " << oper << " %" << numCount - 1
           << ", " << '%' << numCount - 2 << endl;
      numCount++;
    }
  }
};

class AddExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> mulExp;
  unique_ptr<BaseAST> addExp = nullptr;
  char oper = '\0';
  void Dump() const override {
    if (addExp) {
      addExp->Dump();
    }
    mulExp->Dump();
    if (addExp) {
      string operStr;
      switch (oper) {
      case '+':
        operStr = "add";
        break;
      case '-':
        operStr = "sub";
        break;
      default:
        assert(false);
      }
      cout << "  " << '%' << numCount << " = " << operStr << " %"
           << numCount - 1 << ", " << '%' << numCount - 2 << endl;
      numCount++;
    }
  }
};

class MulExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> unaryExp;
  unique_ptr<BaseAST> mulExp = nullptr;
  char oper = '\0';
  int left, right;
  void Dump() const override {
    if (mulExp) {
      mulExp->Dump();
    }
    unaryExp->Dump();
    if (mulExp) {
      string operStr;
      switch (oper) {
      case '*':
        operStr = "mul";
        break;
      case '/':
        operStr = "div";
        break;
      case '%':
        operStr = "mod";
        break;
      default:
        assert(false);
      }
      if (numCount) {
        cout << "  %" << numCount << " = " << operStr << " %" << numCount - 1
             << ", %" << numCount - 2 << endl;
        // numCount++;
      } else {
        // 此时要创造第0个元素
        // cout << "  %0 = " << operStr<<;
      }
      numCount++;
    }
  }
};

class UnaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> unaryExp;
  char unaryOp = 0;

  void Dump() const override {
    if (unaryOp) {
      // 此时所指向的是 UnaryExp，需要递归处理
      unaryExp->Dump();
      string name;
      string oper;
      switch (unaryOp) {
      case '+':
        oper = "add";
        break;
      case '-':
        oper = "sub";
        break;
      case '!':
        oper = "eq";
        break;
      }
      if (numCount) {
        if (unaryOp != '+') {
          cout << "  " << '%' << numCount << " = " << oper << " 0, " << '%'
               << numCount - 1 << endl;
          numCount++;
        }
      } else {
        // if (unaryOp != '+') {
        numCount++;
        cout << "  %0 = " << oper << " 0, ";
        unaryExp->Dump();
        // }
      }
    } else {
      // 此时 unaryExp所指向的是一个PrimaryExp
      unaryExp->Dump();
    }
  }
};
