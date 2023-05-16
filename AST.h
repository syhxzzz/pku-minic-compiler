//
// Created by syhxzzz on 23-5-1.
//
#pragma once
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
// 所有 AST 的基类
using namespace std;
static int numCount = 0;
static int returnValue;
static std::unordered_map<string, int> const_val;
static std::unordered_map<string, int> var_type;
inline void LookForProblem() { cout << "Warning!!!\n"; }
class BaseAST {
public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual int Calc() const = 0;
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
  int Calc() const override { return 0; }
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
  int Calc() const override { return 0; }
};
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "{\n%entry:\n";
    stmt->Dump();
    std::cout << "}\n";
  }
  int Calc() const override { return 0; }
};

class BlockItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> p_def;
  // p_def指的可能是 SinBlockAST 或者 MulBlockAST
  void Dump() const override { p_def->Dump(); }
  int Calc() const override { return 0; }
};

class MulBlockItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> SinBlockItem;
  unique_ptr<BaseAST> BlockItem;
  void Dump() const override {
    SinBlockItem->Dump();
    BlockItem->Dump();
  }
  int Calc() const override { return 0; }
};

class SinBlockItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> p_def;
  // p_def 可能是 Decl 或者 Stmt；
  // Stmt 是 Statment
  void Dump() const override { p_def->Dump(); }
  int Calc() const override { return 0; }
};

class StmtAST : public BaseAST {
public:
  unique_ptr<BaseAST> Exp;
  unique_ptr<BaseAST> LVal;
  // TODO:
  void Dump() const override {
    Exp->Dump();
    std::cout << "  ret %" << numCount - 1 << endl;
  }
  int Calc() const override { return 0; }
};

class DeclAST : public BaseAST {
public:
  unique_ptr<BaseAST> decl_ast;
  // decl_ast可能是 ConstDecl 或者 VarDecl;
  void Dump() const override { decl_ast->Dump(); }
  int Calc() const override { return 0; }
};

class ConstDeclAST : public BaseAST {
public:
  unique_ptr<BaseAST> BType;
  unique_ptr<BaseAST> ConstDef;
  void Dump() const override {

    Calc();
    ConstDef->Dump();
  }
  int Calc() const override { return 0; }
};

class ConstDefItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> ConstDef;
  // ConstDef 可指向 MulConstDef 或者 SinConstDef
  void Dump() const override { ConstDef->Dump(); }
  int Calc() const override { return 0; }
};

class MulConstDefAST : public BaseAST {
public:
  unique_ptr<BaseAST> SinConstDef;
  unique_ptr<BaseAST> ConstDefItem;
  void Dump() const override {
    SinConstDef->Dump();
    ConstDefItem->Dump();
  }
  int Calc() const override { return 0; }
};

class SinConstDefAST : public BaseAST {
public:
  string ident;
  unique_ptr<BaseAST> ConstInitVal_ast;
  void Dump() const override {
    var_type[ident] = 1;
    const_val[ident] = ConstInitVal_ast->Calc();
    // cout << "const[" << ident << "] = " << const_val[ident] << endl;
    // printf("const[%s] = %d\n", ident.c_str(), const_val[ident]);
  }
  int Calc() const override { return 0; }
};

class BTypeAST : public BaseAST {
public:
  string type;
  void Dump() const override {
    // TODO:还没想好实现
  }
  int Calc() const override { return 0; }
};

class ConstExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  void Dump() const override { exp->Dump(); }

  int Calc() const override { return exp->Calc(); }
};

class ConstInitValAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp_ast;
  void Dump() const override {
    // TODO:还没想好实现
  }
  int Calc() const override {
    // printf("ConstInitValAST's Calc() return %d\n", exp_ast->Calc());
    return exp_ast->Calc();
  }
};
class VarDeclAST : public BaseAST {
public:
  unique_ptr<BaseAST> BType;
  unique_ptr<BaseAST> VarDef;
  void Dump() const override {
    // TODO:还没想好要怎么写
  }
  int Calc() const override { return 0; }
};

class VarDefItemAST : public BaseAST {
public:
  unique_ptr<BaseAST> VarDef;
  // ConstDef 可指向 MulVarDef 或者 SinVarDef
  void Dump() const override { VarDef->Dump(); }
  int Calc() const override { return 0; }
};

class MulVarDefAST : public BaseAST {
public:
  unique_ptr<BaseAST> SinVarDef;
  unique_ptr<BaseAST> VarDefItem;
  void Dump() const override {
    SinVarDef->Dump();
    VarDefItem->Dump();
  }
  int Calc() const override { return 0; }
};

class SinVarDefAST : public BaseAST {
public:
  string ident;
  unique_ptr<BaseAST> VarInitVal_ast;
  void Dump() const override {
    cout << "@" << ident << " = alloc i32" << std::endl;
    var_type[ident] = 1;
    const_val[ident] = 0;
    if (VarInitVal_ast) {
      cout << "store %" << numCount - 1 << ",@" << ident << endl;
      const_val[ident] = VarInitVal_ast->Calc();
    }
  }
  int Calc() const override { return 0; }
};

class InitValAST : public BaseAST {
public:
  unique_ptr<BaseAST> exp;
  void Dump() const override { exp->Dump(); }
  int Calc() const override { return 0; }
};

class FuncTypeAST : public BaseAST {
public:
  std::string functype;

  void Dump() const override { std::cout << "i32"; }
  int Calc() const override { return 0; }
};
class NumberExpAST : public BaseAST {
public:
  int number;

  void Dump() const override {
    cout << "  %" << numCount << " = add 0, " << number << endl;
    numCount++;
  }
  int Calc() const override { return number; }
};

class PrimaryExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> p_exp; // 指向具体的primaryExp

  void Dump() const override { p_exp->Dump(); }
  int Calc() const override { return p_exp->Calc(); }
};

class LValAST : public BaseAST {
public:
  string ident;
  void Dump() const override {
    cout << "  %" << numCount << " = add 0, " << const_val[ident] << endl;
    numCount++;
  }
  int Calc() const override { return 0; }
};

class ExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> LOrExp;
  void Dump() const override { LOrExp->Dump(); }
  int Calc() const override { return LOrExp->Calc(); }
};

class LOrExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> LAndExp;
  unique_ptr<BaseAST> LOrExp = nullptr;
  void Dump() const override {
    int left, right;
    if (LOrExp) {
      LOrExp->Dump();
      left = numCount - 1;
    }
    LAndExp->Dump();
    right = numCount - 1;
    if (LOrExp) {
      cout << "  %" << numCount << " = eq %" << left << ", 0" << endl;
      cout << "  %" << numCount + 1 << " = eq %" << numCount << ", 0" << endl;
      cout << "  %" << numCount + 2 << " = eq %" << right << ", 0" << endl;
      cout << "  %" << numCount + 3 << " = eq %" << numCount << ", 0" << endl;
      cout << "  %" << numCount + 4 << " = or %" << numCount + 3 << ", %"
           << numCount + 1 << endl;
      numCount += 5;
    }
  }
  int Calc() const override {
    if (LOrExp) {
      return LAndExp->Calc() || LOrExp->Calc();
    }
    return LAndExp->Calc();
  }
};

class LAndExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> EqExp;
  unique_ptr<BaseAST> LAndExp = nullptr;
  void Dump() const override {
    int left, right;
    if (LAndExp) {
      LAndExp->Dump();
      left = numCount - 1;
    }
    EqExp->Dump();
    right = numCount - 1;
    if (LAndExp) {
      cout << "  %" << numCount << " = eq %" << left << ", 0" << endl;
      cout << "  %" << numCount + 1 << " = eq %" << numCount << ", 0" << endl;
      cout << "  %" << numCount + 2 << " = eq %" << right << ", 0" << endl;
      cout << "  %" << numCount + 3 << " = eq %" << numCount << ", 0" << endl;
      cout << "  %" << numCount + 4 << " = and %" << numCount + 3 << ", %"
           << numCount + 1 << endl;
      numCount += 5;
    }
  }
  int Calc() const override {
    if (LAndExp) {
      return LAndExp->Calc() && EqExp->Calc();
    }
    return EqExp->Calc();
  }
};

class EqExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> RelExp;
  unique_ptr<BaseAST> EqExp = nullptr;
  string oper;
  void Dump() const override {
    int left, right;
    if (EqExp) {
      EqExp->Dump();
      left = numCount - 1;
    }
    RelExp->Dump();
    right = numCount - 1;

    if (EqExp) {
      cout << "  " << '%' << numCount << " = " << oper << " %" << left << ", "
           << '%' << right << endl;
      numCount++;
    }
  }
  int Calc() const override {
    if (EqExp) {
      if (oper == "==") {
        return RelExp->Calc() == EqExp->Calc();
      }
      return RelExp->Calc() != EqExp->Calc();
    }
    return RelExp->Calc();
  }
};

class RelExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> AddExp;
  unique_ptr<BaseAST> RelExp = nullptr;
  string oper;
  void Dump() const override {
    int left, right;
    if (RelExp) {
      RelExp->Dump();
      left = numCount - 1;
    }
    AddExp->Dump();
    right = numCount - 1;
    if (RelExp) {
      if (oper == "ge" || oper == "le") {
        cout << "  " << '%' << numCount << " = " << oper << " %" << left << ", "
             << '%' << right << endl;
        numCount++;
      } else {
        string operToOut = "le";
        if (oper == "<") {
          operToOut = "ge";
        }
        cout << "  %" << numCount << " = " << operToOut << " %" << left << ", %"
             << right << endl;
        cout << "  %" << numCount + 1 << " = eq %" << numCount << ", 0" << endl;
        numCount += 2;
      }
    }
  }
  int Calc() const override {
    if (RelExp) {
      if (oper == "ge") {
        return RelExp->Calc() >= AddExp->Calc();
      } else if (oper == "le") {
        return RelExp->Calc() <= AddExp->Calc();
      } else if (oper == "<") {
        return RelExp->Calc() < AddExp->Calc();
      } else { // oper==">"
        return RelExp->Calc() > AddExp->Calc();
      }
    }
    return AddExp->Calc();
  }
};

class AddExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> mulExp;
  unique_ptr<BaseAST> addExp = nullptr;
  char oper = '\0';
  void Dump() const override {
    int left, right;
    if (addExp) {
      addExp->Dump();
      left = numCount - 1;
    }
    mulExp->Dump();
    right = numCount - 1;
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
      printf("Warning!\n");
      cout << "  " << '%' << numCount << " = " << operStr << " %" << left
           << ", " << '%' << right << endl;
      numCount++;
    }
  }
  int Calc() const override {
    if (addExp) {
      if (oper == '+') {
        return addExp->Calc() + mulExp->Calc();
      }
      return addExp->Calc() - mulExp->Calc();
    }
    return mulExp->Calc();
  }
};

class MulExpAST : public BaseAST {
public:
  unique_ptr<BaseAST> unaryExp;
  unique_ptr<BaseAST> mulExp = nullptr;
  char oper = '\0';
  void Dump() const override {
    int left, right;
    if (mulExp) {
      mulExp->Dump();
      left = numCount - 1;
    }
    unaryExp->Dump();
    right = numCount - 1;

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
        cout << "  %" << numCount << " = " << operStr << " %" << left << ", %"
             << right << endl;
        // numCount++;
      } else {
        // 此时要创造第0个元素
        // cout << "  %0 = " << operStr<<;
      }
      numCount++;
    }
  }
  int Calc() const override {
    if (mulExp) {
      if (oper == '*') {
        return mulExp->Calc() * unaryExp->Calc();
      }
      return mulExp->Calc() / unaryExp->Calc();
    }
    return unaryExp->Calc();
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
        numCount++;
        int num = unaryExp->Calc();
        // printf("num = %d\n", num);
        cout << "  %0 = " << oper << " 0, " << num;
      }
    } else {
      // 此时 unaryExp所指向的是一个PrimaryExp
      unaryExp->Dump();
    }
  }
  int Calc() const override {
    if (unaryOp) {
      if (unaryOp == '+') {
        return unaryExp->Calc();
      } else if (unaryOp == '-') {
        return -unaryExp->Calc();
      }
      return !unaryExp->Calc();
    }
    return unaryExp->Calc();
  }
};
