// 306145 Nakajima Kazuki

#include<stdio.h>
#include<stdbool.h>

#define MAX_SIZE 256

void getWord(void);
char nextChar(void);

char Str[MAX_SIZE];

int main(){
	char tmp;
	getWord();
	printf("word: %s\n",Str);
	while((tmp=nextChar())!='\0'){
		printf("%c ",tmp);
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