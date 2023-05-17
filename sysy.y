%code requires {
  #include <memory>
  #include <string>
  #include <iostream>
  #include <cstring>
  #include "AST.h"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include "AST.h"
#include <cstring>
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;
class BaseAST;
%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况

%union {
  std::string *str_val;
  int int_val;
  char char_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LOR LAND EQ NE LEQ GEQ CONST IF ELSE
%token <str_val> IDENT 
%token <int_val> INT_CONST
/* %token <char_val> OPERATOR  */

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt PrimaryExp UnaryExp Exp NumberExp
%type <ast_val> AddExp MulExp LOrExp LAndExp EqExp RelExp BlockItem Decl ConstDecl
%type <ast_val> BType SinConstDef ConstInitVal ConstExp VarDecl InitVal 
%type <ast_val> SinBlockItem SinVarDef VarDefItem MulVarDef IfStmt SinIfStmt MulIfStmt
%type <ast_val> MulBlockItem ConstDefItem MulConstDef LVal LeVal
%type <int_val> Number 
/* %type <char_val> UnaryOp */
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->functype = "int";
    $$ = ast;
  }
  ;

Block
  : '{' BlockItem '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }|'{' '}'{
    auto ast = new BlockAST();
    $$ = ast;
  }
  ;

BlockItem
  :MulBlockItem{
    auto ast = new BlockItemAST();
    ast->p_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|SinBlockItem{
    auto ast = new BlockItemAST();
    ast->p_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

SinBlockItem
  :Decl{
    auto ast = new BlockItemAST();
    ast->p_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|Stmt{
    auto ast = new BlockItemAST();
    ast->p_def = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

MulBlockItem
  :SinBlockItem BlockItem{
    auto ast = new MulBlockItemAST();
    ast->SinBlockItem = unique_ptr<BaseAST>($1);
    ast->BlockItem= unique_ptr<BaseAST>($2);
    $$ = ast;
  };

Decl
  :ConstDecl{
    auto ast = new DeclAST();
    ast->decl_ast = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|VarDecl{
    auto ast = new DeclAST();
    ast->decl_ast = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  :CONST BType ConstDefItem ';'{
    auto ast = new ConstDeclAST();
    ast->BType = unique_ptr<BaseAST>($2);
    ast->ConstDef = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

ConstDefItem
  :MulConstDef{
    auto ast = new ConstDefItemAST();
    ast->ConstDef = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|SinConstDef{
    auto ast = new ConstDefItemAST();
    ast->ConstDef = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

MulConstDef
  :SinConstDef ',' ConstDefItem{
    auto ast = new MulConstDefAST();
    ast->SinConstDef = unique_ptr<BaseAST>($1);
    ast->ConstDefItem = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

BType
  :INT{
    auto ast = new BTypeAST();
    ast->type = "int";
    $$ = ast;
  };

SinConstDef
  :IDENT '=' ConstInitVal{
    auto ast = new SinConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->ConstInitVal_ast = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

ConstInitVal
  :ConstExp{
    auto ast = new ConstInitValAST();
    ast->exp_ast = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

ConstExp
  :Exp{
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

VarDecl
  :BType VarDefItem ';'{
    auto ast = new VarDeclAST();
    ast->BType = unique_ptr<BaseAST>($1);
    ast->VarDef = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

VarDefItem
  :MulVarDef{
    auto ast = new VarDefItemAST();
    ast->VarDef = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|SinVarDef{
    auto ast = new VarDefItemAST();
    ast->VarDef = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

MulVarDef
  :SinVarDef ',' VarDefItem{
    auto ast = new MulVarDefAST();
    ast->SinVarDef = unique_ptr<BaseAST>($1);
    ast->VarDefItem = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

SinVarDef
  :IDENT '=' InitVal{
    auto ast = new SinVarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->VarInitVal_ast = unique_ptr<BaseAST>($3);
    $$ = ast;
  }|IDENT{
    auto ast = new SinVarDefAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  };

InitVal
  :Exp{
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    // ast->return_str=*unique_ptr<string>(*$1);
    ast->Exp = unique_ptr<BaseAST>($2);
    ast->type = 0;
    $$ = ast;
  }|LeVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->LeVal = unique_ptr<BaseAST>($1);
    ast->Exp = unique_ptr<BaseAST>($3);
    ast->type = 1;
    $$ = ast;
  }|RETURN ';'{
    auto ast = new StmtAST();
    ast->type = 2;
    $$ = ast;
  }|Block{
    auto ast = new StmtAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    ast->type = 3;
    $$ = ast;
  }|Exp ';'{
    auto ast = new StmtAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    ast->type = 4;
    $$ = ast;
  }|';'{
    auto ast = new StmtAST();
    ast->type = 5;
    $$ = ast;
  }|IfStmt{
    auto ast = new StmtAST();
    ast->type = 6;
    ast->ifStmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

IfStmt
  :SinIfStmt{
    auto ast = new IfStmtAST();
    ast->ifStmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|MulIfStmt{
    auto ast = new IfStmtAST();
    ast->ifStmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

SinIfStmt
:IF '(' Exp ')' Stmt{
    auto ast = new SinIfStmtAST();
    ast->Exp = unique_ptr<BaseAST>($3);
    ast->ifStmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

MulIfStmt
:IF '(' Exp ')' Stmt ELSE Stmt{
    auto ast = new MulIfStmtAST();
    ast->Exp = unique_ptr<BaseAST>($3);
    ast->ifStmt = unique_ptr<BaseAST>($5);
    ast->elseStmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }


Exp
  : LOrExp{
    auto ast = new ExpAST();
    ast->LOrExp = unique_ptr<BaseAST>($1);
    $$ = ast; 
  };

LOrExp
  :LAndExp{
    auto ast = new LOrExpAST();
    ast->LAndExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| LOrExp LOR LAndExp{
    auto ast = new LOrExpAST();
    ast->LOrExp = unique_ptr<BaseAST>($1);
    ast->LAndExp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
LAndExp
  :EqExp{
    auto ast = new LAndExpAST();
    ast->EqExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| LAndExp LAND EqExp{
    auto ast = new LAndExpAST();
    ast->LAndExp = unique_ptr<BaseAST>($1);
    ast->EqExp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

EqExp
  :RelExp{
    auto ast = new EqExpAST();
    ast->RelExp = unique_ptr<BaseAST>($1); 
    $$ = ast;
  }| EqExp EQ RelExp{
    auto ast = new EqExpAST();
    ast->EqExp = unique_ptr<BaseAST>($1);
    ast->oper = "eq";
    ast->RelExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  }|EqExp NE RelExp{
    auto ast = new EqExpAST();
    ast->EqExp = unique_ptr<BaseAST>($1);
    ast->oper = "ne";
    ast->RelExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  };

RelExp
  :AddExp{
    auto ast = new RelExpAST();
    ast->AddExp = unique_ptr<BaseAST>($1); 
    $$ = ast;
  }|RelExp '<' AddExp{
    auto ast = new RelExpAST();
    ast->RelExp = unique_ptr<BaseAST>($1);
    ast->oper = "<";
    ast->AddExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  }|RelExp '>' AddExp{
    auto ast = new RelExpAST();
    ast->RelExp = unique_ptr<BaseAST>($1);
    ast->oper = ">";
    ast->AddExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  }|RelExp GEQ AddExp{
    auto ast = new RelExpAST();
    ast->RelExp = unique_ptr<BaseAST>($1);
    ast->oper = "ge";
    ast->AddExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  }|RelExp LEQ AddExp{
    auto ast = new RelExpAST();
    ast->RelExp = unique_ptr<BaseAST>($1);
    ast->oper = "le";
    ast->AddExp = unique_ptr<BaseAST>($3); 
    $$ = ast;
  };

MulExp
  :UnaryExp{
    auto ast = new MulExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|MulExp'*'UnaryExp{
    auto ast = new MulExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($3);
    ast->oper = '*';
    ast->mulExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|MulExp'/'UnaryExp{
    auto ast = new MulExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($3);
    ast->oper = '/';
    ast->mulExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }|MulExp'%'UnaryExp{
    auto ast = new MulExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($3);
    ast->oper = '%';
    ast->mulExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

AddExp
  :AddExp'+'MulExp{
    auto ast = new AddExpAST();
    ast->mulExp = unique_ptr<BaseAST>($3);
    ast->addExp = unique_ptr<BaseAST>($1);
    ast->oper = '+';
    $$ = ast;
  }| AddExp'-'MulExp{
    auto ast = new AddExpAST();
    ast->mulExp = unique_ptr<BaseAST>($3);
    ast->addExp = unique_ptr<BaseAST>($1);
    ast->oper ='-';
    $$ = ast;
  }| MulExp{
    auto ast = new AddExpAST();
    ast->mulExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

LVal
  :IDENT{
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  };

LeVal
  :IDENT{
    auto ast = new LeValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  };

PrimaryExp  
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->p_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }| NumberExp{
    auto ast = new PrimaryExpAST();
    ast->p_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| LVal{
    auto ast = new PrimaryExpAST();
    ast->p_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

UnaryExp
  :PrimaryExp{
    auto ast = new UnaryExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }| '+' UnaryExp{
    auto ast = new UnaryExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($2);
    ast->unaryOp = '+';
    $$ = ast;
  }| '-' UnaryExp{
    auto ast = new UnaryExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($2);
    ast->unaryOp = '-';
    $$ = ast;
  }| '!' UnaryExp{
    auto ast = new UnaryExpAST();
    ast->unaryExp = unique_ptr<BaseAST>($2);
    ast->unaryOp = '!';
    $$ = ast;
  };

/* UnaryOp
  :OPERATOR{
    $$ = char($1);
  }
  ; */

NumberExp
  : Number{
    auto ast = new NumberExpAST();
    ast->type = 0;
    ast->number = $1;
    $$ = ast;
  };

Number
  : INT_CONST {
    // auto ast = new NumberAST();
    // ast->number = int($1);
    $$ =int($1);
  }
  ;


%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  
    extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    int len=strlen(yytext);
    int i;
    char buf[512]={0};
    for (i=0;i<len;++i)
    {
        sprintf(buf,"%s%d ",buf,yytext[i]);
    }
    fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);

}
