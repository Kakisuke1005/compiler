// 306145 Nakajima Kazuki

#include<stdio.h>
#include<string.h>
#include<stdbool.h>

#define MAX_SIZE 256

void getWord(void);
char nextChar(void);
bool isAlphabet(char c);

char Str[MAX_SIZE];
char Sigma[]={'c','e','i','l','m','o','p','r'};

int main(){
	char tmp;
	int count=0;
	getWord();
	printf("word: %s\n",Str);
	while((tmp=nextChar())!='\0'){
		if(isAlphabet(tmp)){
			count++;
		}
	}
	if(count==strlen(Str)){
		printf("%s is an element of Sigma\n", Str);
	}else{
		printf("%s is NOT an element of Sigma\n", Str);
	}
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