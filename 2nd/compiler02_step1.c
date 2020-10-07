// 306145 Nakajima Kazuki

#include<stdio.h>
#include<stdbool.h>

#define MAX_SIZE 256

void getWord(void);

char Str[MAX_SIZE];

int main(){
	getWord();
	printf("word: %s\n",Str);
	return 0;
}

void getWord(void){
	printf("input word: ");
	scanf("%256s",Str);

}