// 306145 Nakajima Kazuki

#include<stdio.h>
#include<stdbool.h>

#define MAX_SIZE 256

void getWord(void);
char nextChar(void);
bool isAlphabet(char c);

char Str[MAX_SIZE];
char Sigma[]={'c','e','i','l','m','o','p','r'};

int main(){
	char tmp;
	getWord();
	printf("word: %s\n",Str);
	while((tmp=nextChar())!='\0'){
		if(isAlphabet(tmp)){
			printf("%c(Y) ",tmp);
		}else{
			printf("%c(N) ",tmp);
		}
	}
	printf("\n");
	return 0;
}

void getWord(void){
	printf("input word: ");
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

bool isAlphabet(char c){
	char *tmp=Sigma;
	while(*tmp!='\0'){
		if(c==*tmp){
			return true;
		}
		tmp++;
	}
	return false;
}