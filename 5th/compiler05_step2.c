// 306145 K.Nakajima

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

#define TOKEN_SIZE 128 // トークンの長さの最大値
#define STR(var) #var

typedef enum{
	Digit,IntNum,
	LParen,RParen,
	Plus,Minus,Multi,Div,
	Variable,Symbol,
	Other,
	EOFToken,NULLToken
}Kind;

typedef struct{
	Kind kind;
	char str[TOKEN_SIZE+1];
	int val;
}Token;

void initializeCharKind(void);
int nextChar(void);
Token nextToken(void);
void writeTokenStr(char **p, char c); 
void printToken(Token *t);

Kind charKind[256];
FILE *fp;

int main(int argc,char *argv[]){
	Token token;

	//printf("initializing CharKind[]... ");
	initializeCharKind();
	//printf("done.\n");

	if(argc<2){
		printf("no file name\n");
		exit(EXIT_FAILURE);
	}

	if((fp=fopen(argv[1],"r"))==NULL){
		printf("error: file \"%s\" can not open\n", argv[1]);
		exit(EXIT_FAILURE);
	}else{
		//printf("file \"%s\" is opened.\n", argv[1]);
	}

	token=nextToken();
	while(token.kind!=EOFToken){
		printToken(&token);
		token=nextToken();
	}

	fclose(fp);
	//printf("file \"%s\" is closed.\n", argv[1]);
	return 0;
}

void initializeCharKind(void){
	for(int i=0;i<256;i++){
		charKind[i]=Other;
	}

	for(int i='0';i<='9';i++){
		charKind[i]=Digit;
	}
	charKind['(']=LParen;
	charKind[')']=RParen;
	charKind['+']=Plus;
	charKind['-']=Minus;
	charKind['*']=Multi;
	charKind['/']=Div;

	for(int i='a';i<='z';i++){
		charKind[i]=Variable;
	}
	for(int i='A';i<='Z';i++){
		charKind[i]=Variable;
	}
	charKind['_']=Variable;
}

int nextChar(void){
	static int ch;
	if(ch==EOF){
		return ch;
	}
	ch=fgetc(fp);
	return ch;
}

Token nextToken(void){
	static int ch=' ';
	Token token={NULLToken,"",0};
	char *pStr=token.str;
	int val=0;

	if(isspace(ch)){
		ch=nextChar();
		while(isspace(ch)){
			ch=nextChar();
		}
	}

	if(ch==EOF){
		token.kind=EOFToken;
		return token;
	}
	//printf("ch=%c\n",(char)ch);
	switch(charKind[ch]){
		case Digit:
			while(charKind[ch]==Digit){
				writeTokenStr(&pStr,(char)ch);
				val=val*10+(ch-'0');
				ch=nextChar();
				//printf("ch=%c\n",(char)ch);
			}
			writeTokenStr(&pStr,'\0');
			token.kind=IntNum;
			token.val=val;
			break;
		case Variable:
			while(charKind[ch]==Variable){
				writeTokenStr(&pStr,(char)ch);
				ch=nextChar();
			}
			writeTokenStr(&pStr,'\0');
			token.kind=Variable;
			break;
		default:
			writeTokenStr(&pStr,(char)ch);
			writeTokenStr(&pStr,'\0');
			token.kind=charKind[ch];
			token.val=0;
			ch=nextChar();
			break;
	}

	return token;
}

void writeTokenStr(char **p, char c){
	**p=c;
	(*p)++;
}

void printToken(Token *t) {
	printf("%s, val=%d, kind=", t->str, t->val);
	switch (t->kind) {
		case LParen: printf("%s", STR(LParen)); break;
		case RParen: printf("%s", STR(RParen)); break;
		case Plus: printf("%s", STR(Plus)); break;
		case Minus: printf("%s", STR(Minus)); break;
		case Multi: printf("%s", STR(Multi)); break;
		case Div: printf("%s", STR(Div)); break;
		case IntNum: printf("%s", STR(IntNum)); break;
		case Symbol: printf("%s", STR(Symbol)); break;
		case EOFToken: printf("%s", STR(EOFToken)); break;
		case NULLToken: printf("%s", STR(NULLToken)); break;
		case Other: printf("%s", STR(Other)); break;
		case Variable: printf("%s", STR(Variable)); break;
		default: printf("not implemented"); break;
	}
	printf("\n");
}