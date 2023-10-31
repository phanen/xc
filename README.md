# c4 learning notes
- <https://github.com/lotabout/write-a-C-interpreter>
- <https://yearn.xyz/posts/techs/词法语法分析基础>
- <https://pandolia.net/tinyc/ch13_bison.html>
- <https://www.zhihu.com/column/marisa>
- <https://www.zhihu.com/question/28249756>

## overview
- <https://github.com/lotabout/write-a-C-interpreter>

编译原理
- 词法分析器: string -> token
- 语法分析器: token -> AST
- 目标代码的生成: AST -> machine code

四个函数
- `next()` 用于词法分析, 获取下一个标记, 它将自动忽略空白字符
- `program()` 语法分析的入口, 分析整个 C 语言程序
- `expression(level)` 用于解析一个表达式
- `eval()` 虚拟机的入口, 用于解释目标代码


## vm

内存布局 (逻辑)
```
+------------------+
|    stack   |     |      high address
|    ...     v     |
|                  |
|                  |
|                  |
|                  |
|    ...     ^     |
|    heap    |     |
+------------------+
| ............     |
+------------------+
| data segment     |
+------------------+
| text segment     |      low address
+------------------+
```

寄存器
- `pc` 程序计数器
- `sp` 栈指针
- `bp` 基址指针, 维护栈帧
- `ax` 通用寄存器, 用于存放一条指令执行后的结果

指令集
- MOV: IMM LC LI SC SI
- PUSH (没有 POP, 实际通过别的操作)
- JMP JZ/JNZ
- 子函数调用: CALL, ENT, ADJ, LEV, LEA
    - 指令太简陋, 没法做 prologue/epilogue, 干脆 就原语化了
- 运算符指令
- 内置函数 (syscall 原语化)
```
sub_function(arg1, arg2, arg3);

|    ....       | high address
+---------------+
| arg: 1        |    new_bp + 4
+---------------+
| arg: 2        |    new_bp + 3
+---------------+
| arg: 3        |    new_bp + 2
+---------------+
|return address |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| local var 1   |    new_bp - 1
+---------------+
| local var 2   |    new_bp - 2
+---------------+
|    ....       |  low address
```

## lexer

string(source code) -> token stream


我们实现的词法分析器与传统意义上的词法分析器不太相同. 传统意义上的符号表只需要知道标识符的唯一标识即可, 而我们还存放了一些只有语法分析器才会得到的信息, 如 type

```c
struct identifier {
    int token;
    int hash;
    char * name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
}
```
- token: 变量都是 Id, 另外关键字如 if, while 等, 有对应的标记
- hash: 单纯用于标识符的快速比较
- name: 存放标识符本身的字符串
- class: 该标识符的类别, 如数字, 全局变量或局部变量等
- type: 标识符的类型, 对变量: int char 或指针
- value: 存放这个标识符的值, 如标识符是函数, 刚存放函数的地址
- Bxxxx: c 语言中标识符可以是全局的也可以是局部的, 当局部标识符的名字与全局标识符相同时, 用作保存全局标识符的信息

要点
- 词法分析器的作用是对源码字符串进行预处理, 作用是减小语法分析器的复杂程度
- 词法分析器本身可以认为是一个编译器, 输入是源码, 输出是标记流
- lookahead(k) 的概念, 即向前看 k 个字符或标记
- 词法分析中如何处理标识符与符号表

## 递归下降

传统上, 编写语法分析器有两种方法
- 自顶向下: 从起始非终结符开始, 不断地对非终结符进行分解, 直到匹配输入的终结符
- 自底向上: 不断地将终结符进行合并, 直到合并成起始的非终结符

其中的自顶向下方法就是我们所说的递归下降
```
<expr> ::= <expr> + <term>
         | <expr> - <term>
         | <term>
# expr 是 term 的和

<term> ::= <term> * <factor>
         | <term> / <factor>
         | <factor>
# term 是 factor 的积

<factor> ::= ( <expr> )
           | Num
```

分析 `3 * (4 + 2)` 是 `expr`
```
3 * (4 + 2)
-> expr?
-> term?
-> term? * factor?
-> factor? * factor?
-> Num(3) * (expr?)
-> Num(3) * (term? + term?)
-> Num(3) * (factor? + factor?)
-> Num(3) * (Num(4)? + Num(2)?)
```

字符串是 expr -> 能被文法分析成 expr 的 AST
- 问题: AST 不唯一 -> 歧义性: `a+a*a`

修改文法, 消除左递归
```
<expr> ::= <term> <expr_tail>
<expr_tail> ::= + <term> <expr_tail>
              | - <term> <expr_tail>
              | <empty>

<term> ::= <factor> <term_tail>
<term_tail> ::= * <factor> <term_tail>
              | / <factor> <term_tail>
              | <empty>

<factor> ::= ( <expr> )
           | Num
```

##  ebnf

c 语言的文法
```
program ::= {global_declaration}+

global_declaration ::= enum_decl | variable_decl | function_decl

enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'

variable_decl ::= type {'*'} id { ',' {'*'} id } ';'

function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

parameter_decl ::= type {'*'} id {',' type {'*'} id}

body_decl ::= {variable_decl}, {statement}

statement ::= non_empty_statement | empty_statement

non_empty_statement ::= if_statement | while_statement | '{' statement '}'
                     | 'return' expression | expression ';'

if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]

while_statement ::= 'while' '(' expression ')' non_empty_statement
```

`global_declaration` 检测到函数或变量的 declaration
```c
    if (token == '(') {
      current_id[Class] = Fun;
      current_id[Value] = (int)(text + 1); // the memory address of function
      function_declaration();
    } else {
      // variable declaration
      current_id[Class] = Glo;       // global variable
      current_id[Value] = (int)data; // assign memory address
      data = data + sizeof(int);
    }
```

`function_declaration` 最后 resume back 隐藏的变量名
```c
  match('(');
  function_parameter();
  match(')');
  match('{');
  function_body();
  // match('}');
  // unwind local variable declarations for all local variables.
  current_id = symbols;
  while (current_id[Token]) {
    if (current_id[Class] == Loc) {
      current_id[Class] = current_id[BClass];
      current_id[Type] = current_id[BType];
      current_id[Value] = current_id[BValue];
    }
    current_id = current_id + IdSize;
  }
```


`statement`
- 简单地说, 可以认为语句就是表达式加上末尾的分号
```
if (...) <statement> [else <statement>]
while (...) <statement>
{ <statement> }
return xxx;
<empty statement>;
expression; (expression end with semicolon)
```

## codegen

编译器的语法分析部分其实是很简单的, 真正的难点是如何在语法分析时收集足够多的信息, 最终把源代码转换成目标代码(汇编)
- 对一个 expr, 我们总是先尝试 eval 他的值, 然后设法用汇编加载到 ax 里
- 下文进一步根据上文得到 ax 计算即可

取址运算: 取消 load
```c
else if (token == And) {
    // get the address of
    match(And);
    expression(Inc); // get the address of
    // +2 -1
    if (*text == LC || *text == LI) {
        text--;
    } else {
        printf("%d: bad address of\n", line);
        exit(-1);
    }
    expr_type = expr_type + PTR;
}
```


表达式有优先级, 需要先算完高优先级再轮到低优先级, 也就是需要先 codegen 高优先级
```c
void expression(int level);
while (token >= level)
```

## crt

为了 bootstrap, 不仅要按照限制语法写, 还要确保解释程序也能正确获取命令行参数
```c
argc--; argv++;
```

然后初始化入口和栈, 这里有个经典的技巧假装让 `main` 是 callee, 然后返回到栈上执行 `exit`
```c
pc = (int *)idmain[Value];
// setup stack
sp = (int *)((int)stack + poolsize);
*--sp = EXIT; // call exit if main returns
*--sp = PUSH;
tmp = sp;
*--sp = argc;
*--sp = (int)argv;
*--sp = (int)tmp;
```
