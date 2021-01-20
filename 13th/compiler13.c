#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
 
 
/** 文字・トークンの種類 **/
typedef enum {
    LParen, RParen, /* (, ) */
    LBrace, RBrace, /* {, } */
    Plus, Minus, Multi, Div, /* +, -, *, / */
    Percent, /* % */
    Ampersand, Vline, /* &, | */
    Assign, /* = */
    Less, Greater, /* "<", ">" */
    Excl, /* ! */
    Semicolon, /* ; */
    Comma, /* , */
    Equal, NotEq, /* "==", "!=" */
    LessEq, GreaterEq, /* "<=", ">=" */
    SnglQuo, DblQuo, /* \', " */
    Digit, /* 0-9 */
    Letter, /* _, a - z, A - Z */
    IntNum, /* integer */
    Variable, /* variable */
    Literal, /* literal */
    Int, /* int */
    If, Else, /* "if", "else" */
    While, /* "while" */
    Puts, /* "puts" */
    EOFToken, NULLToken, /* EOF, NULL */
    Other /* 上記のいずれでもない */
} Kind;
 
/** トークン **/
#define TOKEN_SIZE 128
typedef struct {
    Kind kind;                /* トークンの種類 */
    char str[TOKEN_SIZE + 1]; /* トークンの文字列 */
    int val;                  /* トークンが定数のとき，その値 */
} Token;
 
/** 予約語 **/
typedef struct {
    Kind kind;
    char *str;
} KeyWord;
 
/** 予約語表 **/
KeyWord KeyWordTable[] = {
    {LParen, "("}, {RParen, ")"},
    {Plus, "+"}, {Minus, "-"}, {Multi, "*"}, {Div, "/"},
    {Assign, "="},
    {Comma, ","}, {Semicolon, ";"},
    {Equal, "=="}, {NotEq, "!="},
    {Less, "<"}, {LessEq, "<="},
    {Greater, ">"}, {GreaterEq, ">="},
    {Int, "int"},
    {If, "if"}, {Else, "else"},
    {While, "while"},
    {Puts, "puts"},
    {NULLToken, ""}
};
 
/** 記号表 **/
typedef struct {
    Token token;
    int address;
} Entry;
 
#define TABLE_SIZE 64
/* Entry導入前のSymbolTable
typedef struct {
    Token table[TABLE_SIZE];
    Token *tail;
} SymbolTable;
*/
typedef struct {
    Entry table[TABLE_SIZE];
    Entry *tail;
} SymbolTable;
 
/** 命令コード **/
typedef enum {
    NOP, /* no operation */
    ASS,
    LD, LDA, LDI,
    LIB,
    ADD,SUB,MUL,DIV,MOD,
    EQU,NTEQ,LESS,LSEQ,GRT,GTEQ,
    JMP,JMPT,JMPF,
    START,
    END
} OperationCode;
 
typedef enum {
    LIB_PUTS
} LibraryFunction;
 
typedef struct {
    OperationCode code; /* 命令コード */
    bool flag;
    int data;
} Instruction;
 
/********1*********2*********3*********4*********5*********6*********7*/
/* global variables */
 
FILE *fin;
Kind charKind[256]; /* 文字種表 */
SymbolTable STable; /* 記号表 */
Token token;
 
#ifdef VERBOSE
int _depth = 0;
#endif
 
#define MEM_SIZE 0xFFFF
char Memory[MEM_SIZE + 1];
int address = 0;
 
#define CODE_SIZE 20000 /* コード領域のサイズ */
Instruction codes[CODE_SIZE + 1];
int codecnt = 0;
 
#define OPSTACK_SIZE 100
#define OPSTACK_BOTTOM 0
int OpStack[OPSTACK_SIZE + 1];
int opstack_num = OPSTACK_BOTTOM;
int PC; /* プログラムカウンタ */
 
/********1*********2*********3*********4*********5*********6*********7*/
 
/* プロトタイプ宣言 */
/** 主処理 **/
void compile(void);
void interpret(void);
/** 字句解析 **/
void initializeCharKind(void);
int nextChar(void);
Token nextToken(void);
void writeTokenStr(char **p, char c); 
bool checkToken(Token *t, Kind kind);
bool isTwoLettersOp(char first, char second);
void setKind(Token *token);
#define STR(var) #var
void printToken(Token *t);
void errorExit(char *str);
/** 構文解析 **/
void program(void);
void declaration(void);
void function(void);
void statement(void);
void print_statement(void);
void expression_statement(void);
void selection_statement(void);
void iteration_statement(void);
void expression(void);
void equality_expression(void);
void additive_expression(void);
void multiplicative_expression(void);
void factor(void);
 
int nextCodeCnt(void);
void backPatch(int n,int addr);
void copyToken(Token *to, Token *from);
#ifdef VERBOSE
void _printIndent();
#endif
/** 記号表 **/
void initializeTable(SymbolTable *st);
bool containTable(SymbolTable *st, char *name);
void addTable(SymbolTable *st, Entry t);
bool replaceEntry(SymbolTable *st, Entry t);
Entry searchTable(SymbolTable *st, char *name);
void printTable(SymbolTable *st);
/** 中間コード **/
int allocateMemory(int n);
int generateCode(OperationCode op, bool flag, int data);
int generateCode2(OperationCode op, int data);
int generateCode1(OperationCode op);
void generateCodeBinary(Kind operator);
void codedump(void);
void printInstruction(Instruction *inst);
/** インタプリタ **/
int execute(void);
void execute_library(int kind);
void opstack_push(int data);
int opstack_pop(void);
void runtimeerror(char *msg);
void printOpStack(void);
void printMemory(void);
 
/********1*********2*********3*********4*********5*********6*********7*/
/** 主処理 **/
 
int main(int argc, char *argv[]) {
    /* 文字種表charKindの初期化 */
#ifdef VERBOSE
    printf("initializing charKind[]\n");
#endif
    initializeCharKind();
 
    /* 記号表の初期化 */
#ifdef VERBOSE
    printf("initializing SymbolTable\n");
#endif
    initializeTable(&STable);
 
    /* open file argv[1] */
    if (argc < 2) {
        printf("no file name\n");
        exit(EXIT_FAILURE);
    }
    if ((fin = fopen(argv[1], "r")) == NULL) {
        printf("error: file \"%s\" can not open\n", argv[1]);
        exit(EXIT_FAILURE);
    } else {
#ifdef VERBOSE
        printf("file \"%s\" is opened.\n", argv[1]);
#endif
    }
 
    /* コンパイル */
    compile();
 
    /* インタプリタによる実行 */
    interpret(); /* <- 演習12－1ではコメントアウトすること */
    //printTable(&STable);
    //printf("aa %d\n",address);
    //printOpStack();
 
    /* close file argv[1] */
    fclose(fin);
#ifdef VERBOSE
    printf("file \"%s\" is closed.\n", argv[1]);
#endif
 
    return 0;
}
 
void compile (void) {
    /* 命令コード START */
    generateCode1(START);
 
    /* プログラムの処理(コンパイル) */
    token = nextToken();
    program();
 
    /* 命令コード END */
    generateCode1(END); 
 
#ifdef VERBOSE
    /* 中間コード生成後の記号表の表示 */
    printf("\n");
    printf("Symbol Table (after compile):\n");
    printTable(&STable);
    printf("\n");
#endif
 
    /* 中間コードの表示 */
    printf("code dump:\n");
    codedump();
 
#ifdef VERBOSE
    /* 中間コードのデータ用メモリ領域の内容表示 */
    printf("memory map (after compiler):\n");
    printMemory();
    printf("\n");
#endif
 
}
 
void interpret(void) {
    printf("execution result:\n");
    execute();
 
    /* 中間コードのメモリ領域の内容表示 */
    printf("\n");
    printf("memory map:\n");
    printMemory();
}
 
/********1*********2*********3*********4*********5*********6*********7*/
/** 字句解析 **/
 
/* 文字種表charKindの初期化 */
void initializeCharKind(void) {
    int i;
 
    /* すべての要素をOtherとして初期化 */
    for (i = 0; i < 256; i++) {
        charKind[i] = Other;
    }
 
    /* '0'-'9'の文字をDigitとする */
    for (i = '0'; i <= '9'; i++) {
        charKind[i] = Digit;
    }
 
    /* _, a - z, A - Z の文字をLetterとする */
    charKind['_'] = Letter;
    for (i = 'a'; i <= 'z'; i++) {
        charKind[i] = Letter;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        charKind[i] = Letter;
    }
 
    /* 個々の文字の割当て */
    charKind['('] = LParen;
    charKind[')'] = RParen;
    charKind['}'] = RBrace;
    charKind['{'] = LBrace;
    charKind['+'] = Plus;
    charKind['-'] = Minus;
    charKind['*'] = Multi;
    charKind['/'] = Div;
    charKind['&'] = Ampersand;
    charKind['|'] = Vline;
    charKind['='] = Assign;
    charKind['!'] = Excl;
    charKind['<'] = Less;
    charKind['>'] = Greater;
    charKind['\''] = SnglQuo;
    charKind['"'] = DblQuo;
    charKind[','] = Comma;
    charKind[';'] = Semicolon;
}
 
/* ファイルから次の1文字を取得する */
int nextChar(void) {
    static int ch;
 
    if (ch == EOF) { /* 最後に読んだ文字がEOFならば，EOFを返す */
        return ch;
    } 
    ch = fgetc(fin);
    
    return ch;
}
 
/* ファイルから記号を1文字ずつ読み，トークンを取得する */
Token nextToken(void) {
    static int ch = ' ';
    Token token = {NULLToken, "", 0};
    char *pStr = token.str;
    int val = 0;
 
    /* 空白の読み飛ばし */
    while (isspace(ch)) {
        ch = nextChar();
    }
 
    /* 最後の読んだ文字がEOFの場合，EOFを表わすトークンを返す */
    if (ch == EOF) {
        token.kind = EOFToken;
        return token;
    }
 
    /* トークンの切り出し */
    switch (charKind[ch]) {
        case Digit: /* 数字 */
            while (charKind[ch] == Digit) {
                writeTokenStr(&pStr, (char)ch);
                val = val * 10 + (ch - '0');
                ch = nextChar();
            }
            token.kind = IntNum;
            writeTokenStr(&pStr, '\0');
            token.val = val;
            break;
 
        case Letter:
            while (charKind[ch] == Letter) {
                writeTokenStr(&pStr, (char)ch);
                ch = nextChar();
            }
            //token.kind = Variable;
            writeTokenStr(&pStr, '\0');
            break;
 
        case SnglQuo: /* 文字定数 */
            writeTokenStr(&pStr, (char)ch); /* '\''を書き出す */
            ch = nextChar();
            if (ch == EOF) {
                errorExit("\' is not terminated");
            }
            writeTokenStr(&pStr, (char)ch); /* SnglQuo('\'')の次の1文字を書きだす */
            token.val = ch;
            ch = nextChar();
            if (charKind[ch] != SnglQuo) {
                errorExit("\' is expected");
            }
            writeTokenStr(&pStr, (char)ch); /* '\''を書き出す */
            token.kind = Literal; /* Cでは文字コードと捉えてIntNumとされる */
            writeTokenStr(&pStr, '\0');
            ch = nextChar();
            break;
 
        case DblQuo: /* 文字列定数 */
            writeTokenStr(&pStr, (char)ch); /* '"'を書き出す */
            ch = nextChar();
            while (charKind[ch] != DblQuo && ch != EOF) {
                writeTokenStr(&pStr, (char)ch);
                ch = nextChar();
            }
            if (ch == EOF) {
                errorExit("\" is not terminated");
            }
            writeTokenStr(&pStr, (char)ch); /* '"'を書き出す */
            token.kind = Literal;
            writeTokenStr(&pStr, '\0');
            ch = nextChar();
            break;
 
        /* 1文字で字句を決定できる記号 */
        case LParen:
        case RParen:
        case RBrace:
        case LBrace:
        case Plus:
        case Minus:
        case Multi:
        case Div:
        case Comma:
        case Semicolon:
            token.kind = charKind[ch];
            writeTokenStr(&pStr, (char)ch);
            writeTokenStr(&pStr, '\0');
            ch = nextChar();
            break;
 
        default: /* 1文字では字句を決定できない記号 */
            {
                char first = ch;
                writeTokenStr(&pStr, (char)ch); /* 1文字目を書き込んでおく */
                ch = nextChar();
                if (isTwoLettersOp(first, ch)) {
                    writeTokenStr(&pStr, (char)ch);
                    ch = nextChar();
                }
            }
            writeTokenStr(&pStr, '\0');
            break;
    }
 
    if (token.kind == NULLToken) {
        setKind(&token);
    }
 
    return token;
}
 
void writeTokenStr(char **p, char c) {
    **p = c;
    (*p)++;
}
 
bool checkToken(Token *t, Kind kind) {
    return (t->kind == kind); 
}
 
bool isTwoLettersOp(char first, char second) {
    char candidate[3] = {first, second, '\0'};
 
    KeyWord *it = KeyWordTable;
    while (it->kind != NULLToken) {
        if (strcmp(candidate, it->str) == 0) {
            return true;
        }
        it++;
    }
 
    return false; 
}
 
void setKind(Token *token) {
    token->kind = Other;
    KeyWord *it = KeyWordTable;
    while (it->kind != NULLToken) {
        if (strcmp(token->str, it->str) == 0) {
            token->kind = it->kind;
            return;
        }
        it++;
    }
    if (charKind[(int)*(token->str)] == Letter) {
        token->kind = Variable;
    } 
}
 
/* Tokenの表示 */
void printToken(Token *t) {
    printf("%s \t val=%d \t kind=", t->str, t->val);
    switch (t->kind) {
        case LParen: printf("%s", STR(LParen)); break;
        case RParen: printf("%s", STR(RParen)); break;
        case Plus: printf("%s", STR(Plus)); break;
        case Minus: printf("%s", STR(Minus)); break;
        case Multi: printf("%s", STR(Multi)); break;
        case Div: printf("%s", STR(Div)); break;
        case SnglQuo: printf("%s", STR(SnglQuo)); break;
        case DblQuo: printf("%s", STR(DblQuo)); break;
        case Assign: printf("%s", STR(Assign)); break;
        case Excl: printf("%s", STR(Excl)); break;
        case Less: printf("%s", STR(Less)); break;
        case LessEq: printf("%s", STR(LessEq)); break;
        case Greater: printf("%s", STR(Greater)); break;
        case GreaterEq: printf("%s", STR(GreaterEq)); break;
        case Comma: printf("%s", STR(Comma)); break;
        case Semicolon: printf("%s", STR(Semicolon)); break;
        case Equal: printf("%s", STR(Equal)); break;
        case NotEq: printf("%s", STR(NotEq)); break;
        case IntNum: printf("%s", STR(IntNum)); break;
        case Variable: printf("%s", STR(Variable)); break;
        case Literal: printf("%s", STR(Literal)); break;
        case Int: printf("%s", STR(Int)); break;
        case If: printf("%s", STR(If)); break;
        case Else: printf("%s", STR(Else)); break;
        case While: printf("%s", STR(While)); break;
        case Puts: printf("%s", STR(Puts)); break;
        case EOFToken: printf("%s", STR(EOFToken)); break;
        case Other: printf("%s", STR(Other)); break;
        case LBrace: printf("%s", STR(LBrace)); break;
        case RBrace: printf("%s", STR(RBrace)); break;
        default: printf("not implemented?"); break;
    }
}
 
/* エラー表示関数 */
void errorExit(char *str) {
    if (strlen(str) > 0) {
        printf("%s\n", str);
    }
    exit(EXIT_FAILURE);
}
 
/********1*********2*********3*********4*********5*********6*********7*/
/** 構文解析 **/
 
void program(void) {
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    function();
 
#ifdef VERBOSE
    _depth--;
    _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void declaration(void) {
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    Entry entry = {{NULLToken, "", 0}, 0};
    int addr;
 
    switch (token.kind) {
        case Int:
            /* 次のトークンはVariableでなければならない */
            token = nextToken();
            if (!checkToken(&token, Variable)) {
                errorExit("Variable is expected.");
            }
            /* 記号表に変数が登録されているか確認する */
            if (containTable(&STable, token.str)) {
                errorExit("Duplicate variable.");
            } 
            entry.token = token;
            addr = allocateMemory(sizeof(int));
            entry.address = addr;
            addTable(&STable, entry);
 
            /* 次のトークンがCommaの場合，変数宣言が続く */
            token = nextToken();
            while (token.kind != EOFToken && token.kind == Comma) {
                token = nextToken();
                if (!checkToken(&token, Variable)) {
                    errorExit("Variable is expected.");
                }
                /* 記号表に変数が登録されているか確認する */
                if (containTable(&STable, token.str)) {
                    errorExit("Duplicate variable.");
                }
                entry.token = token;
                addr = allocateMemory(sizeof(int));
                entry.address = addr;
                addTable(&STable, entry);
                token = nextToken();
            }
 
            /* 行末はセミコロンで終わらなければならない */
            if (!checkToken(&token, Semicolon)) {
                errorExit("Semicolon is expected.");
            }
            token = nextToken();
            break;
 
        default:
            errorExit("cannot happen");
            break;
    }
 
 
#ifdef VERBOSE
    _depth--;
    _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void function(void) {
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    while(token.kind!=EOFToken){
        switch(token.kind){
            case Int:
                declaration();
                break;
            default:
                statement();
                break;
        }
    }
 
#ifdef VERBOSE
    _depth--;
    _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void statement(void) {
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    //token=nextToken();
    switch (token.kind) {
        case LBrace:
                /* lesson13, "{" <statement> "}" */
                /* lesson13, <statement>の処理 */
                /* lesson13, RBraceで終わることを確認する */
            token=nextToken();
            statement();
            //token=nextToken();
            while(token.kind!=RBrace){
                statement();
            }
        break;
        case If:
            selection_statement();
        break;
        case While:
            iteration_statement();
            token=nextToken();
        break;
        case Puts:
            print_statement();
        break;
        default:
            expression_statement();
        break;
    }
 
#ifdef VERBOSE
    _depth--;
    _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void print_statement(void) {
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    assert(token.kind == Puts);
    Entry entry;
 
    token = nextToken();
    if (checkToken(&token, EOFToken)) {
        errorExit("syntax error: Variable is expected.");
    }
    if (!containTable(&STable, token.str)) {
        printf("Variable %s is not declared.\n", token.str);
        errorExit("");
    } else {
        entry = searchTable(&STable, token.str);
    }
    token = nextToken();
 
    /* Semicolonで終わらなければならない */
    if (!checkToken(&token, Semicolon)) {
        errorExit("Semicolon is expected.");
    }
    token = nextToken();
 
    /* intermediate code */
    generateCode2(LD, entry.address);
    generateCode2(LIB, (int)(LIB_PUTS));
 
#ifdef VERBOSE
    _depth--;
    _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void expression_statement(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    if (token.kind != EOFToken) {
        expression();
    }
    /* Semicolonで終わらなければならない */
    if (!checkToken(&token, Semicolon)) {
        errorExit("Semicolon is expected.");
    }
    token = nextToken();
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void selection_statement(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
    int position_jmpf, pos_jmp;
 
    switch (token.kind) {
        case If:
            /* lesson13, LParenの確認 */
            /* lesson13, <equality_expression>の処理 */
            /* lesson13, RParenの確認 */
            token=nextToken();
            if(token.kind==LParen){
                equality_expression();
            }else{
                errorExit("( is expected");
            }
            //token=nextToken();
            if(token.kind!=RParen){
                //printToken(&token);
                errorExit(") is expected");
            }
 
            /* 条件式が偽のときのためバックパッチ用の仮の番地を設定 */
            position_jmpf = generateCode2(JMPF, -1)-1;
 
            /* lesson13, <statement>の処理 */
            token=nextToken();
            statement();
						token=nextToken();
 
            if (!checkToken(&token, Else)) {
                // else文がないとき
                backPatch(position_jmpf, nextCodeCnt());
                //token=nextToken();
                break;
            } else {
								pos_jmp=generateCode2(JMP,-1)-1;
								backPatch(position_jmpf,nextCodeCnt());
                // else文があるとき
                token=nextToken();
                statement();
                /* lesson13, バックパッチを用いたelse文の処理 */
                backPatch(pos_jmp,nextCodeCnt());
								token=nextToken();
            }
            break;
 
        default:
            errorExit("cannot happen");
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void iteration_statement(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    int position_begin;
    int position_jmpf;
 
    switch (token.kind) {
        case While:
            position_begin = nextCodeCnt();
            token = nextToken();
 
            /* lesson13, LParenの確認 */
            /* lesson13, <equality_expression>の処理 */
            /* lesson13, RParenの確認 */
            if(token.kind==LParen){
                equality_expression();
            }else{
                errorExit("( is expected");
            }
            //token=nextToken();
            if(token.kind!=RParen){
                errorExit("( is expected");
            }
 
            position_jmpf = generateCode2(JMPF, -1)-1; 
            token = nextToken();
            
            statement();
 
            /* lesson13, バックパッチ */
            generateCode2(JMP,position_begin);
            backPatch(position_jmpf,nextCodeCnt());
 
            break;
 
        default:
            errorExit("cannot happen");
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void expression(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
 
    //token=nextToken();
    switch (token.kind) {
        case Variable: /* VARIABLE = <expression> */
 
            /* lesson13 */
            generateCode(LDA,true,searchTable(&STable,token.str).address);
            token=nextToken();
            if(token.kind!=Assign){
                errorExit("= is expected");
            }
            token=nextToken();
            additive_expression();
            generateCode1(ASS);
            break;
 
        default:
            errorExit("cannot happen: in <expression>");
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void equality_expression(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    Kind operator;
 
    token=nextToken();
    additive_expression();
    switch (token.kind) {
        case Less:
        case LessEq:
        case Greater:
        case GreaterEq:
        case Equal:
        case NotEq:
            operator = token.kind;
            token = nextToken();
            additive_expression();
            /* intermediate code */
            generateCodeBinary(operator);
            break;
 
        default:
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void additive_expression(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    Kind operator;
 
    multiplicative_expression();
    switch (token.kind) {
        case Plus:
        case Minus:
            operator = token.kind;
            token = nextToken();
            multiplicative_expression();
            /* intermediate code */
            generateCodeBinary(operator);
            break;
 
        default:
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void multiplicative_expression(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    Kind operator;
 
    factor();
    switch (token.kind) {
        case Multi:
        case Div:
        case Percent:
            operator = token.kind;
            token = nextToken();
            factor();
            /* intermediate code */
            generateCodeBinary(operator);
            break;
 
        default:
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
void factor(void){
#ifdef VERBOSE
    _printIndent(); printf("-> %s, ", __func__); printToken(&token); printf("\n");
    _depth++;
#endif
 
    Entry entry;
    //token=nextToken();
    switch (token.kind) {
        case Variable:
            if (!containTable(&STable, token.str)) {
                printf("Variable %s is not declared.\n", token.str);
                errorExit("");
            }
            /* intermediate code */
            entry = searchTable(&STable, token.str);
            if (entry.token.kind == NULLToken) {
                printf("Variable %s is not declared", token.str);
                errorExit("");
            }
            generateCode2(LD, entry.address);
            //generateCode1(ASS);
            token = nextToken();
            break;
 
        case IntNum:
            generateCode2(LDI, token.val);
            //generateCode1(ASS);
            token = nextToken();
            break;
 
        case LParen:
            token = nextToken();
            additive_expression();
            if (!checkToken(&token, RParen)) {
                printf("syntax error: ')' is expected\n");
                exit(EXIT_FAILURE);
            }
            token = nextToken();
            break;
 
        default:
            break;
    }
 
#ifdef VERBOSE
    _depth--; _printIndent(); printf("<- %s\n", __func__);
#endif
}
 
int nextCodeCnt(void){
    return codecnt;
}
 
void backPatch(int n,int addr){
    codes[n].data=addr;
}
 
/*void copyToken(Token *to, Token *from) {
    to->kind = from->kind;
    strcpy(to->str, from->str);
    to->val = from->val;
}*/
 
 
#ifdef VERBOSE
void _printIndent() {
    int i;
    for (i = 0; i < _depth; i++) {
        printf(" ");
    }
}
#endif
 
/********1*********2*********3*********4*********5*********6*********7*/
/** 記号表 **/
 
void initializeTable(SymbolTable *st) {
    st->tail = st->table;
}
 
bool containTable(SymbolTable *st, char *name) {
    Entry *it = st->table;
    
    while (it != st->tail) {
        if (strcmp((it->token).str, name) == 0) {
            return true;
        }
        it++;
    }
 
    return false;
}
 
void addTable(SymbolTable *st, Entry t) {
    *(st->tail) = t;
    (st->tail)++;
}
 
/* 関数名変更: Lesson11ではreplaceElementOfTable */
bool replaceEntry(SymbolTable *st, Entry t) {
    Entry *it = st->table;
    bool replaced = false;
 
    /* lesson11 */
    while(it!=st->tail){
        if(strcmp(it->token.str,t.token.str)==0){
            replaced=true;
            *it=t;
        }
        it++;
    }
 
    return replaced;
}
 
Entry searchTable(SymbolTable *st, char *name) {
    Entry e = {{NULLToken, "", 0}, 0};
    Entry *it = st->table;
 
    /* lesson11 */
    while(it!=st->tail){
        if(strcmp(it->token.str,name)==0){
            e=*it;
            break;
        }
        it++;
    }
 
    return e;
}
 
void printTable(SymbolTable *st) {
    Entry *it = st->table;
    while (it != st->tail) {
        printToken(&(it->token));
        printf(", address=0x%04x", it->address);
        printf("\n");
        it++;
    }
}
 
/********1*********2*********3*********4*********5*********6*********7*/
/** 中間コード **/
 
int allocateMemory(int n) {
    if (address + n > MEM_SIZE) {
        errorExit("out of memory");
    }
 
    static int flag=0;
 
    /* 12-1, step 2-3 */
    /* 確保したメモリ領域を0で埋める(念のための初期化) */
    if(flag==0){
        flag=1;
        return 0;
    }
 
    address+=n;
    for(int i=address;i<address+n;i++){
        Memory[i]=0;
    }
 
    return address; /* <- この戻り値はダミーです．変更する必要があります */
}
 
int generateCode(OperationCode op, bool flag, int data) {
    Instruction inst = {NOP, false, 0};
 
    /* 12-1, step 3-2 */
    inst.code=op;
    inst.flag=flag;
    inst.data=data;
 
    codes[codecnt]=inst;
    codecnt++;
 
 
    return codecnt;
}
 
int generateCode2(OperationCode op, int data) {
    return generateCode(op, false, data);
}
 
int generateCode1(OperationCode op) {
    return generateCode(op, false, 0);
}
 
void generateCodeBinary(Kind operator){
    switch(operator){
      case Less:
          generateCode1(LESS);
      break;
      case LessEq:
    	  generateCode1(LSEQ);
      break;
      case Greater:
        generateCode1(GRT);
      break;
      case GreaterEq:
    	  generateCode1(GTEQ);
      break;
      case Equal:
        generateCode1(EQU);
      break;
      case NotEq:
        generateCode1(NTEQ);
      break;
      case Plus:
        generateCode1(ADD);
      break;
      case Minus:
        generateCode1(SUB);
      break;
      case Multi:
        generateCode1(MUL);
      break;
      case Div:
        generateCode1(DIV);
      break;
      case Percent:
        generateCode1(MOD);
      default:
        errorExit("error");
      break;
    }
}
 
void codedump(void) {
    Instruction inst;
 
    int i;
    for (i = 0; i < codecnt; i++) {
        inst = codes[i];
        /* コード番号 */
        printf("%04d: ", i);
        /* instruction */
        printInstruction(&inst);
        printf("\n");
    }
    printf("\n");
}
 
void printInstruction(Instruction *inst) {
        /* 命令コード */
        switch ((*inst).code) {
            case ASS: printf("%s", STR(ASS)); break;
            case LD: printf("%s", STR(LD)); break;
            case LDA: printf("%s", STR(LDA)); break;
            case LDI: printf("%s", STR(LDI)); break;
            case LIB: printf("%s", STR(LIB)); break;
            case ADD: printf("%s", STR(ADD)); break;
            case SUB: printf("%s", STR(SUB)); break;
            case MUL: printf("%s", STR(MUL)); break;
            case DIV: printf("%s", STR(DIV)); break;
            case EQU: printf("%s", STR(EQU)); break;
            case NTEQ: printf("%s", STR(NTEQ)); break;
            case LESS: printf("%s", STR(LESS)); break;
            case LSEQ: printf("%s", STR(LSEQ)); break;
            case GRT: printf("%s", STR(GRT)); break;
            case GTEQ: printf("%s", STR(GTEQ)); break;
            case JMP: printf("%s", STR(JMP)); break;
            case JMPT: printf("%s", STR(JMPT)); break;
            case JMPF: printf("%s", STR(JMPF)); break;
            case START: printf("%s", STR(START)); break;
            case END: printf("%s", STR(END)); break;
            default:
                errorExit("invalid operation code");
                break;
        }
        printf(", ");
        /* フラグ */
        /*
        if ((*inst).flag == true) {
            printf("true");
        } else {
            printf("false");
        }
        printf(", ");
        */
        /* データ */
        printf("%d", (*inst).data);
}
 
/********1*********2*********3*********4*********5*********6*********7*/
/** インタプリタ **/
 
int execute(void) {
#ifdef VERBOSE
    printf("-> %s\n", __func__);
#endif
 
    Instruction inst;
    int addr;
    int result = 0;
    int pop1, pop2;
    bool isEndFlag = false;
 
    PC = 0;
    opstack_num = OPSTACK_BOTTOM;
 
    while (!isEndFlag) {
        if (PC < 0 || codecnt < PC) {
            runtimeerror("fatal error");
        }
        if (opstack_num < OPSTACK_BOTTOM) {
            runtimeerror("stack underflow");
        }
        if (opstack_num > OPSTACK_SIZE) {
            runtimeerror("stack overflow");
        }
 
        inst = codes[PC];
        addr = inst.data;
 
        switch (inst.code) {
            case ASS:
                pop2 = opstack_pop();
                pop1 = opstack_pop();
                *(int *)(Memory + pop1) = pop2;
                //Memory[pop1]=pop2;
                //printf("pop1 %d\n",pop1);
                //printf("pop2 %d\n",pop2);
                //printf("memory: %d\n",(int)Memory[0]);
                //printMemory();
            break;
            case LD:
                //opstack_push((int)Memory[addr]);
                opstack_push(*(int *)(Memory + addr));
            break;
            case LDA:
                opstack_push(addr);
            break;
            case LDI:
                opstack_push(addr);
            break;
            case LIB:
                execute_library(LIB_PUTS);
            break;
            case ADD:
                pop2 = opstack_pop();
                pop1 = opstack_pop();
                opstack_push(pop1+pop2);
            break;
            case SUB:
                pop2 = opstack_pop();
                pop1 = opstack_pop();
                opstack_push(pop1-pop2);
            break;
            case MUL:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1*pop2);
            break;
            case DIV:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1/pop2);
            break;
            case MOD:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1%pop2);
            break;
            case EQU:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1==pop2);
            break;
            case NTEQ:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1!=pop2);
            break;
            case LESS:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1<pop2);
            break;
            case LSEQ:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1<=pop2);
            break;
            case GRT:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1>pop2);
            break;
            case GTEQ:
                pop2=opstack_pop();
                pop1=opstack_pop();
                opstack_push(pop1>=pop2);
            break;
            case JMP:
                PC=addr-1;
            break;
            case JMPT:
                pop1=opstack_pop();
                if(pop1){
                    PC=addr-1;
                }
            break;
            case JMPF:
                pop1=opstack_pop();
                if(!pop1){
                    PC=addr-1;
                }
            break;
            case START:
                isEndFlag=false;
            break;
            case END:
                isEndFlag=true;
            break;
 
 
            default:
                runtimeerror("invalid operation code");
                break;
        }
        PC++;
        //printf("stack:");
        //printOpStack();
    }
 
#ifdef VERBOSE
    printf("<- %s\n", __func__);
#endif
 
    return result;
}
 
void execute_library(int kind) {
    int data;
 
    switch (kind) {
        case LIB_PUTS:
            data = opstack_pop();
            printf("%d\n", data);
            break;
        default:
            printf("???\n");
            break;
    }
}
 
void opstack_push(int data) {
    if (opstack_num < OPSTACK_SIZE) {
        OpStack[opstack_num] = data;
        opstack_num++;
    } else {
        runtimeerror("stack overflow");
    }
}
 
int opstack_pop(void) {
    int data;
 
    if (opstack_num > OPSTACK_BOTTOM) {
        data = OpStack[opstack_num - 1];
        opstack_num--;
    } else {
        runtimeerror("stack underflow");
    }
 
    return data;
}
 
void runtimeerror(char *msg) {
    fprintf(stderr, "Runtime Error: PC=%d, %s\n", PC - 1, msg);
    exit(EXIT_FAILURE);
}
 
void printOpStack(void) {
    int i;
    bool isFirst = true;
 
    printf("OpStack: (%d) ", opstack_num);
    for (i = 0; i < opstack_num; i++) {
        if (isFirst) {
            isFirst = false;
        } else  {
            printf(", ");
        }
        printf("%d ", OpStack[i]);
    }
    printf("\n");
}
 
void printMemory(void) {
    int i;
    for (i = 0; i <= address; i+=sizeof(int)) {
        printf("0x%04x: %d", i, *(int *)(Memory + i));
        printf("\n");
    }
}
 