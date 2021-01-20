#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* 定義 */
#define TOKEN_SIZE 128 /* トークンの長さの最大値 */
#define TABLE_SIZE 64
#define STR(var) #var
// define VERBOSE

/** 文字・トークンの種類 **/
typedef enum {
	LParen, RParen, /* (, ) */
	Plus, Minus, Multi, Div, /* +, -, *, / */
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

/** 記号表 **/
typedef struct {
	Token table[TABLE_SIZE];
	Token *tail;
} SymbolTable;

/* グローバル変数 */
FILE *fp;
Kind charKind[256]; /* 文字種表 */
Token token; /* 次に処理するトークン */
SymbolTable STable; /* 記号表 */
#ifdef VERBOSE
int _depth = 0;
#endif

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

/* プロトタイプ宣言 */
/** プロトタイプ宣言: 字句解析 **/
void initializeCharKind(void);
int nextChar(void);
Token nextToken(void);
void writeTokenStr(char **p, char c); 
bool checkToken(Token *t, Kind kind);
bool isTwoLettersOp(char first, char second);
void setKind(Token *token);

/** プロトタイプ宣言: 構文解析 **/
void program(void);
void declaration(void);
void function(void);
void statement(void);
void print_statement(void);
void copyToken(Token *to, Token *from); /* トークンのコピー */


/** プロトタイプ宣言: 記号表 **/
void initializeTable(SymbolTable *st); /* 記号表の初期化 */
bool containTable(SymbolTable *st, char *name); /* 記号表の要素の確認 */
void addTable(SymbolTable *st, Token t); /* 記号表にトークンを追加する */
bool replaceElementOfTable(SymbolTable *st, Token t); /* 記号表の要素を入れ替える */
Token searchTable(SymbolTable *st, char *name); /* 記号表の要素を取得する */
void printTable(SymbolTable *st);


/** プロトタイプ宣言: データ表示関数 **/
void printToken(Token *t);
void errorExit(char *str);
#ifdef VERBOSE
void _printIndent();
#endif


/********1*********2*********3*********4*********5*********6*********7*/

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
	if ((fp = fopen(argv[1], "r")) == NULL) {
		printf("error: file \"%s\" can not open\n", argv[1]);
		exit(EXIT_FAILURE);
	} else {
#ifdef VERBOSE
		printf("file \"%s\" is opened.\n", argv[1]);
#endif
	}

	/* nextToken関数によるトークンの取得と表示 */
	token = nextToken();
	program();
	printf("Symbol Table:\n");
	printTable(&STable);

	/* close file argv[1] */
	fclose(fp);
#ifdef VERBOSE
	printf("file \"%s\" is closed.\n", argv[1]);
#endif

	exit(EXIT_SUCCESS);
}


/* 構文解析 *******2*********3*********4*********5*********6*********7*/

void program(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	/* step2 */
	declaration();
	function();

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
#endif
}

void declaration(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	switch (token.kind) {
		case Int:

			/* step 2 */

			/* hint: 次のトークンはVariableでなければならない */
			token=nextToken();
			if(!checkToken(&token,Variable)){
				printf("error\n");
				exit(EXIT_SUCCESS);
			}

			/* hint: 記号表に変数が登録されているか確認する */
			//printf("%s",token.str);
			if(!containTable(&STable,token.str)){
				addTable(&STable,token);
			}else{
				printf("error: Duplicate variable.\n");
				exit(EXIT_SUCCESS);
			}

			/* hint: 次のトークンがCommaの場合，変数宣言が続く */
			token=nextToken();
			if(checkToken(&token,Comma)){
				while(checkToken(&token,Comma)){
					token=nextToken();
					if(!checkToken(&token,Variable)){
						printf("error\n");
						exit(EXIT_SUCCESS);
					}
					if(!containTable(&STable,token.str)){
						addTable(&STable,token);
					}
					token=nextToken();
				}
			}

			
			/* hint: 行末はセミコロンで終わらなければならない */
			if(!checkToken(&token,Semicolon)){
				printf("error: Semicolon is expected.\n");
				exit(EXIT_SUCCESS);
			}
			token=nextToken();

			break;

		default:
			printf("error: Variable %s is not declared.\n",token.str);
			exit(EXIT_SUCCESS);
			break;
	}


#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
#endif
}

void function(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	/* step 2 */
	statement();
	while(!checkToken(&token, EOFToken)){
		//token=nextToken();
		statement();
	}

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
#endif
}

void statement(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	/* step 2 */
	Token tmp;
	switch(token.kind){
		case Variable:
			copyToken(&tmp,&token);
			token=nextToken();
			if(!checkToken(&token,Assign)){
				printf("error: '=' is expected.\n");
				exit(EXIT_SUCCESS);
			}
			token=nextToken();
			tmp.val=token.val;
			if(!replaceElementOfTable(&STable,tmp)){
				//printf("error: Duplicate variable.\n");
				printf("error: Variable %s is not declared.\n",tmp.str);
				exit(EXIT_SUCCESS);
			}
			token=nextToken();
			if(!checkToken(&token,Semicolon)){
				printf("error: Semicolon is expected.\n");
				exit(EXIT_SUCCESS);
			}
			token=nextToken();
		break;
		case Puts:
			print_statement();
			token=nextToken();
			if(!checkToken(&token,Semicolon)){
				printf("error: Semicolon is expected.\n");
				exit(EXIT_SUCCESS);
			}
			token=nextToken();
		break;
		case Int:
			declaration();
		break;
		default:
			//printf("error: %s\n",token.str);
			//printToken(&token);
			//exit(EXIT_SUCCESS);
		break;
	}

#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
#endif
}

void print_statement(void) {
#ifdef VERBOSE
	_printIndent(); printf("-> %s, ", __func__); printToken(&token);
	_depth++;
#endif

	assert(token.kind == Puts);

	token=nextToken();
	switch(token.kind){
		case Variable:
			if(containTable(&STable,token.str)){
				printf("%d\n",searchTable(&STable,token.str).val);
			}
		break;
		default:
			printf("error\n");
			exit(EXIT_SUCCESS);
		break;
	}
	
#ifdef VERBOSE
	_depth--;
	_printIndent(); printf("<- %s\n", __func__);
#endif
}

void copyToken(Token *to, Token *from) {
	to->kind = from->kind;
	strcpy(to->str, from->str);
	to->val = from->val;
}


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
	charKind['('] = LParen;
	charKind[')'] = RParen;
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
#ifdef VERBOSE
	assert (token->kind == Other);
#endif
	if (charKind[(int)*(token->str)] == Letter) {
		token->kind = Variable;
	} 
}

/* 記号表 *********2*********3*********4*********5*********6*********7*/

void initializeTable(SymbolTable *st) {
	st->tail = st->table;
}

bool containTable(SymbolTable *st, char *name) {
	Token *it = st->table;
	
	while (it != st->tail) {
		if (strcmp(it->str, name) == 0) {
			return true;
		}
		it++;
	}

	return false;
}

void addTable(SymbolTable *st, Token t) {
	*(st->tail) = t;
	(st->tail)++; 
}

bool replaceElementOfTable(SymbolTable *st, Token t) {
	Token *it = st->table;
	bool replaced = false;

	/* step 1 */
	while(it!=st->tail){
		if(strcmp(it->str,t.str)==0){
			replaced=true;
			*it=t;
		}
		it++;
	}

	return replaced;
}

Token searchTable(SymbolTable *st, char *name) {
	Token t = {NULLToken, "", 0};
	Token *it = st->table;

	/* step 1 */
	while(it!=st->tail){
		if(strcmp(it->str,name)==0){
			t=*it;
			break;
		}
		it++;
	}
	return t;

	return t;
}

void printTable(SymbolTable *st) {
	Token *it = st->table;
	while (it != st->tail) {
		printToken(it);
		it++;
	}
}


/* データ表示関数 *2*********3*********4*********5*********6*********7*/

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
		case Ampersand: printf("%s", STR(Ampersand)); break;
		case Vline: printf("%s", STR(Vline)); break;
		case Assign: printf("%s", STR(Assign)); break;
		case Less: printf("%s", STR(Less)); break;
		case Greater: printf("%s", STR(Greater)); break;
		case Excl: printf("%s", STR(Excl)); break;
		case Semicolon: printf("%s", STR(Semicolon)); break;
		case Equal: printf("%s", STR(Equal)); break;
		case NotEq: printf("%s", STR(NotEq)); break;
		case LessEq: printf("%s", STR(LessEq)); break;
		case GreaterEq: printf("%s", STR(GreaterEq)); break;
		case SnglQuo: printf("%s", STR(SnglQuo)); break;
		case DblQuo: printf("%s", STR(DblQuo)); break;
		case Comma: printf("%s", STR(Comma)); break;
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
		default: printf("not implemented?"); break;
	}
	printf("\n");
}

/* エラー表示関数 */
void errorExit(char *str) {
	if (strlen(str) > 0) {
		printf("%s\n", str);
	}
	exit(EXIT_SUCCESS);
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

