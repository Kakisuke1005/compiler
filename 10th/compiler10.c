/*
 * How to compile:
 * - VERBOSEを有効にするときは，-DVERBOSE オプションを付ける
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* 定義 */
#define TOKEN_SIZE 128 /* トークンの長さの最大値 */
#define STR(var) #var
// define VERBOSE

/** 文字・トークンの種類 **/
typedef enum {
	LParen, RParen, /* (, ) */
	Plus, Minus, Multi, Div, /* +, -, *, / */
	Ampersand, Vline, /* &, | */
	Assign, /* = */

	/* 10-1, step1 */
	Less,Greater,
	Excl,Semicolon,
	Equal,NotEq,
	LessEq,GreaterEq,

	/* 10-2, step1 */
	SnglQuo,DblQuo,
	Literal,

	Digit, /* 0-9 */
	Letter, /* _, a - z, A - Z */
	IntNum, /* integer */
	Variable, /* variable */

	/* 10-3, step1 */
	Int,
	If,Else,While,
	Puts,

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


/* グローバル変数 */
FILE *fp;
Kind charKind[256]; /* 文字種表 */
/** 予約語表 **/
KeyWord KeyWordTable[] = {
	{LParen, "("}, {RParen, ")"},
	{Plus, "+"}, {Minus, "-"}, {Multi, "*"}, {Div, "/"},
	{Assign, "="},
	{Less, "<"}, {Greater, ">"}, 
	{Semicolon, ";"},
	
	/* 10-1, step3-2 */
	{Equal,"=="},{NotEq,"!="},
	{LessEq,"<="},{GreaterEq,">="},

	/* 10-3, step2 */
	{Int,"int"},
	{If,"if"},{Else,"else"},
	{While,"while"},
	{Puts,"puts"},

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

/** プロトタイプ宣言: データ表示関数 **/
void printToken(Token *t);
void errorExit(char *str);


/********1*********2*********3*********4*********5*********6*********7*/

int main(int argc, char *argv[]) {
	Token token;

	/* 文字種表charKindの初期化 */
#ifdef VERBOSE
	printf("initializing charKind[]\n");
#endif
	initializeCharKind();

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
	while (token.kind != EOFToken) {
		printToken(&token);
		token = nextToken();
	}

	/* close file argv[1] */
	fclose(fp);
#ifdef VERBOSE
	printf("file \"%s\" is closed.\n", argv[1]);
#endif

	return 0;
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
	charKind['<'] = Less;
	charKind['>'] = Greater;
	charKind['!'] = Excl;
	charKind[';'] = Semicolon;
	charKind['\''] = SnglQuo;
	charKind['"'] = DblQuo;
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
			setKind(&token);
			if(token.kind==Other){
				token.kind=Variable;
			}
			writeTokenStr(&pStr, '\0');
			break;

		case SnglQuo: /* 文字定数 */

			/* 10-2, step3-1: '\''を書き出す */
			/* 10-2, step3-1: SnglQuo('\'')の次の1文字を書きだす */
			/* 10-2, step3-1: '\''を書き出す */
			writeTokenStr(&pStr,(char)ch);
			ch=nextChar();
			if(charKind[ch]==Letter){
				writeTokenStr(&pStr,(char)ch);
				token.val=ch;
			}else{
				printf("error: ' is expected\n");
				exit(EXIT_SUCCESS);
			}
			ch=nextChar();
			if(charKind[ch]==SnglQuo){
				writeTokenStr(&pStr,(char)ch);
			}else{
				printf("error: ' is expected\n");
				exit(EXIT_SUCCESS);
			}
			

			token.kind = Literal; /* Cでは文字コードと捉えてIntNumとされる */
			writeTokenStr(&pStr, '\0');
			ch = nextChar();
			break;

		case DblQuo: /* 文字列定数 */

			/* 10-2, step3-2: '"'を書き出す */
			/* 10-2, step3-2: DblQuo('"')の次の1文字を書きだす */
			/* 10-2, step3-2: '"'を書き出す */

			writeTokenStr(&pStr,(char)ch);
			ch=nextChar();

			while(charKind[ch]==Letter){
				writeTokenStr(&pStr,(char)ch);
				ch=nextChar();
			}

			/*if(charKind[ch]==Letter){
				writeTokenStr(&pStr,(char)ch);
			}else{
				printf("error: ' is expected\n");
				exit(EXIT_SUCCESS);
			}*/


			//ch=nextChar();
			if(charKind[ch]==DblQuo){
				writeTokenStr(&pStr,(char)ch);
			}else{
				printf("error: \" is not terminated\n");
				exit(EXIT_SUCCESS);
			}

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
#ifdef VERBOSE
	printf("-> isTwoLettersOp: (%c, %c)\n", first, second);
#endif

	char candidate[3] = {first, second, '\0'};

	KeyWord *it = KeyWordTable;
	while (it->kind != NULLToken) {
		if (strcmp(candidate, it->str) == 0) {
#ifdef VERBOSE
	printf("<- isTwoLettersOp\n");
#endif
			return true;
		}
		it++;
	}

#ifdef VERBOSE
	printf("<- isTwoLettersOp\n");
#endif

	return false;
}

void setKind(Token *token) {
	token->kind = Other;
	KeyWord *it = KeyWordTable;

	/* 10-1, step4, and 10-3, step3 */
	while (it->kind != NULLToken) {
		if (strcmp(token->str, it->str) == 0) {
			token->kind=it->kind;
		}
		it++;
	}
	/*** sample ***/
	/*** 
	while (it->kind != NULLToken) {
		if (strcmp(token->str, it->str) == 0) {

	***/
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
		case SnglQuo: printf("%s", STR(SnglQuo)); break;
		case DblQuo: printf("%s", STR(DblQuo)); break;
		case Assign: printf("%s", STR(Assign)); break;
		case Excl: printf("%s", STR(Excl)); break;
		case Less: printf("%s", STR(Less)); break;
		case LessEq: printf("%s", STR(LessEq)); break;
		case Greater: printf("%s", STR(Greater)); break;
		case GreaterEq: printf("%s", STR(GreaterEq)); break;
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
		default: printf("not implemented?"); break;
	}
	printf("\n");
}

/* エラー表示関数 */
void errorExit(char *str) {
	printf("%s\n", str);
	exit(EXIT_SUCCESS);
}

/********1*********2*********3*********4*********5*********6*********7*/

