// 306145 K.Nakajima

#include<stdio.h>
#include<stdlib.h>

#define MAX_SIZE 255

typedef enum{
	Sp, Sq, Sr, Sd
}STATE;

void printConfiguration(STATE s, char *str);
void printInfer(void); 
void getWord(void);
char nextChar(void);

char Str[MAX_SIZE+1];

int main(){
	STATE s=Sp;
	char tmp;
	int count=0;

	getWord();
	printConfiguration(s,Str+count);

	// 文字列を読み込みオートマトンを実行させる
	while((tmp=nextChar())!='\0'){
		printf(" ");
		printInfer();
		printf(" ");

		switch(s){
			case Sp:
				if(tmp=='a'){
					s=Sq;
				}else{
					s=Sd;
				}
				break;
			case Sq:
				if(tmp=='a'){
					s=Sq;
				}else if(tmp=='b'){
					s=Sr;
				}else{
					s=Sd;
				}
				break;
			case Sr:
				if(tmp=='b'){
					s=Sr;
				}else{
					s=Sd;
				}
				break;
			case Sd:
				s=Sd;
				break;
			default:
				printf("error!\n");
				exit(1);
				break;
		}

		count++;
		printConfiguration(s,Str+count);
	}
	printf("\n");

	// 受理判定
	if(s==Sr){
		printf("accept\n");
	}else{
		printf("reject\n");
	}

	return 0;
}

void printConfiguration(STATE s, char *str){
	printf("(");
	switch(s){
		case Sp:
			printf("p");
			break;
		case Sq:
			printf("q");
			break;
		case Sr:
			printf("r");
			break;
		case Sd:
			printf("d");
			break;
		default:
			printf("error\n");
			exit(1);
			break;
	}
	printf(", ");
	printf("%s",str);
	printf(")");
}

void printInfer(void){
	printf("|-");
}

void getWord(void){
	printf("w = ");
	scanf("%256s",Str);
}

char nextChar(void){
	static char *pStr=Str;
	char c;
	if(*pStr!='\0'){
		c=*pStr;
		pStr++;
	}else{
		c=*pStr;
	}
	return c;
}