/*
 * How to compile:
 * - VERBOSEを有効にする方法
 *   1: プログラム中では宣言せず，-DVERBOSE オプションを付けてコンパイルする
 *   2: プログラム中で #define する
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* 定義 */
#define TOKEN_SIZE 128 /* トークンの長さの最大値 */
#define STACK_SIZE 24 /* スタックのサイズの最大値  */
#define LINE_SIZE 128 /* 1行に含まれるトークン数の最大値 */
#define TABLE_SIZE 64 /* 記号表が格納できるトークンの最大値 */
#define STR(var) #var

/** 文字・トークンの種類 **/
typedef enum {
	Plus, Assign,
	Digit, /* 0-9 */
	Letter, /* _, a - z, A - Z */
	IntNum, /* integer */
	Variable, /* variable */
	EOFToken, NULLToken, /* EOF, NULL */
	Other /* 上記のいずれでもない */
} Kind;

/** トークン **/
typedef struct {
	Kind kind;                /* トークンの種類 */
	char str[TOKEN_SIZE + 1]; /* トークンの文字列 */
	int val;                  /* トークンが定数のとき，その値 */
} Token;

/* 記号表 */
typedef struct {
	Token table[TABLE_SIZE];
	Token *tail;
} SymbolTable;

/* グローバル変数 */
FILE *fp;
Kind charKind[256]; /* 文字種表 */
Token stack[STACK_SIZE]; /* スタック */
int stack_num = 0; /* スタック内のデータ数 */
Token token; /* 次に処理するトークン */
SymbolTable STable; /* 記号表 */
#ifdef VERBOSE
int _depth = 0;
#endif

/* プロトタイプ宣言 */
/** プロトタイプ宣言: 構文解析 **/
void statement(void); /* <statement> */
void expression(void); /* <expression> */
void factor(void); /* <factor> */
void evaluate(Kind kind); /* スタックの演算 */
void copyToken(Token *to, Token *from); /* トークンのコピー */
bool checkToken(Token *t, Kind kind); /* トークンの種類の確認 */

/** プロトタイプ宣言: 記号表 **/
void initializeTable(SymbolTable *st); /* 記号表の初期化 */
void addTable(SymbolTable *st, Token t); /* 記号表にトークンを追加する */
void printTable(SymbolTable *st);
//Token searchTable(SymbolTable *st, char *name); /* 記号表の要素を取得する */
//bool replaceElementOfTable(SymbolTable *st, Token t); /* 記号表の要素を入れ替える */
//bool containTable(SymbolTable *st, char *name); /* 記号表の要素の確認 */

/** プロトタイプ宣言: 字句解析 **/
void initializeCharKind(void);
int nextChar(void);
Token nextToken(void);
void writeTokenStr(char **p, char c); 

/** プロトタイプ宣言: スタック **/
void push(Token t); /* スタック: push */ 
Token pop(void); /* スタック: pop */
void initializeStack(void); /* スタック: スタックを初期化 */
void printStack(void); /* スタック: スタックの内容を表示  */

/** プロトタイプ宣言: データ表示関数 **/
void printToken(Token *t);
#ifdef VERBOSE
void _printIndent();
#endif

/********1*********2*********3*********4*********5*********6*********7*/

int main(int argc, char *argv[]) {
	/* 文字種表charKindの初期化 */
#ifdef VERBOSE
	printf("initializing charKind[]... ");
#endif
	initializeCharKind();
#ifdef VERBOSE
	printf("done.\n");
#endif

	/* 変数表の初期化 */
#ifdef VERBOSE
	printf("initializing SymbolTable... ");
#endif
	initializeTable(&STable);
#ifdef VERBOSE
	printf("done.\n");
#endif

	/* open file argv[1] */
	if (argc < 2) {
		printf("no file name\n");
		exit(EXIT_FAILURE);
	}
	if ((fp = fopen(argv[1], "r")) == NULL) {
		printf("error: file \"%s\" can not open\n", argv[1]);
		exit(EXIT_FAILURE);
	} else {
#ifdef VERBOSE
		printf("file \"%s\" is opened.\n", argv[1]);
#endif
	}

	/* 構文解析 */
	token = nextToken();
	statement();
	if (!checkToken(&token, EOFToken)) {
		printf("error: token(s) remaining\n");
		exit(EXIT_SUCCESS);
	}
	/* スタックの内容を表示 */

	printf("stack:\n");
	printStack();
	/* 記号表を表示 */
	printf("symbol table:\n");
	printTable(&STable);

	/* close file argv[1] */
	fclose(fp);
#ifdef VERBOSE
	printf("file \"%s\" is closed.\n", argv[1]);
#endif

	return 0;
}


/* 構文解析 *******2*********3*********4*********5*********6*********7*/

void statement(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	Token var;
	switch(token.kind){
		case Variable:

		// 後で右辺の演算結果を代入するために,変数を保存しておく
		copyToken(&var,&token);

			token=nextToken();
			if(!checkToken(&token,Assign)){
				printf("error: '=' is expected.\n");
				exit(EXIT_SUCCESS);
			}
			token=nextToken();
			expression();

			// <exprssion>の演算結果は(右辺)はスタックに格納されている
			Token calresult;
			calresult=pop();

			// 左辺の変数(var)に代入する(varのメンバを更新する)
			var.kind=calresult.kind;  // Variable
			var.val=calresult.val;
			// var.strはすでに格納されている

			// 記号表に登録
			// 本来は記号表にvarが登録されているか調べる
			addTable(&STable,var);

			break;
		default:
			printf("syntax error: %s\n",token.str);
			exit(EXIT_SUCCESS);
			break;
	}

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
	printStack();
#endif
}

void expression(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	Kind operator;
	factor();
	switch(token.kind){
		case Plus:
			operator=token.kind;
			token=nextToken();
			factor();
			evaluate(operator);
			break;
		default:
			break;
	}

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
	printStack();
#endif
}

void factor(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	switch(token.kind){
		case IntNum:
			push(token);
			break;
		default:
			printf("syntax error: %s\n",token.str);
			break;
	}
	token=nextToken();

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
	printStack();
#endif
}

void evaluate(Kind kind) {
	Token t1, t2, tmp = {NULLToken, "", 0};

	t2 = pop();
	t1 = pop();
	switch (kind) {
		case Plus:
			tmp.val=t1.val+t2.val;
			tmp.kind=IntNum;
			break;
		default:
			printf("error: invalid Kind\n");
			exit(EXIT_SUCCESS);
			break;
	}
	push(tmp);
}

void copyToken(Token *to, Token *from) {
	to->kind = from->kind;
	strcpy(to->str, from->str);
	to->val = from->val;
}

bool checkToken(Token *t, Kind kind) {
	return (t->kind == kind); 
}


/* 記号表 *********2*********3*********4*********5*********6*********7*/

void initializeTable(SymbolTable *st) {
	st->tail = st->table;
}

void addTable(SymbolTable *st, Token t) {
	*(st->tail) = t;
	*(st->tail)++; 
}

void printTable(SymbolTable *st) {
	Token *it = st->table;
	while (it != st->tail) {
		printToken(it);
		it++;
	}
}

//bool containTable(SymbolTable *st, char *name) {
//	/* step1 */
//	return false;
//}
//
//bool replaceElementOfTable(SymbolTable *st, Token t) {
//	/* step1 */
//	return replaced;
//}
//
//Token searchTable(SymbolTable *st, char *name) {
//	/* step1 */
//	return t;
//}


/* 字句解析 *******2*********3*********4*********5*********6*********7*/

/*
 * 文字種表charKindの初期化
 */
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
	charKind['+'] = Plus;
	charKind['='] = Assign;
}

/*
 * ファイルから次の1文字を取得する
 */
int nextChar(void) {
	static int ch;

	if (ch == EOF) { /* 最後に読んだ文字がEOFならば，EOFを返す */
		return ch;
	} 
	ch = fgetc(fp);
	
	return ch;
}

/*
 * ファイルから記号を1文字ずつ読み，トークンを取得する
 */
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
			token.kind = Variable;
			writeTokenStr(&pStr, '\0');
			break;

		case Plus:
		case Assign:
			token.kind = charKind[ch];
			writeTokenStr(&pStr, (char)ch);
			writeTokenStr(&pStr, '\0');
			ch = nextChar();
			break;

		default:
			token.kind = Other;
			writeTokenStr(&pStr, (char)ch);
			writeTokenStr(&pStr, '\0');
			ch = nextChar();
			break;
	}

	return token;
}

void writeTokenStr(char **p, char c) {
	**p = c;
	(*p)++;
}

/* スタック *******2*********3*********4*********5*********6*********7*/

/* スタック: push */
void push(Token t) {
	if (stack_num < STACK_SIZE) {
		stack[stack_num] = t;
		stack_num++;
	} else {
		printf("stack overflow\n");
		exit(EXIT_SUCCESS);
	}
}

/* スタック: pop */
Token pop(void) {
	Token t;

	if (stack_num > 0) {
		t = stack[stack_num - 1];
		stack_num--;
	} else {
		printf("stack underflow\n");
		exit(EXIT_SUCCESS);
	}

	return t;
}

/* スタック: スタックを初期化 */
void initializeStack(void) {
	stack_num = 0;
}

/* スタック: スタックの内容を表示 */
void printStack(void) {
	int i;
	if (stack_num == 0) {
		printf("(empty)\n");
	} else {
		for (i = 0; i < stack_num; i++) {
			printf("'%s' ", stack[i].str);
		} 
		printf("\n");
	}
}

/* データ表示関数 *2*********3*********4*********5*********6*********7*/

/* Tokenの表示 */
void printToken(Token *t) {
	printf("%s, val=%d, kind=", t->str, t->val);
	switch (t->kind) {
		case Plus: printf("%s", STR(Plus)); break;
		case Assign: printf("%s", STR(Assign)); break;
		case IntNum: printf("%s", STR(IntNum)); break;
		case Variable: printf("%s", STR(Variable)); break;
		case EOFToken: printf("%s", STR(EOFToken)); break;
		case NULLToken: printf("%s", STR(NULLToken)); break;
		case Other: printf("%s", STR(Other)); break;
		default: printf("not implemented"); break;
	}
	printf("\n");
}

#ifdef VERBOSE
void _printIndent() {
	int i;
	for (i = 0; i < _depth; i++) {
		printf(" ");
	}
}
#endif

/********1*********2*********3*********4*********5*********6*********7*/

