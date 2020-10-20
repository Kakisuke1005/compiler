// 306145 K.Nakajima

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

#define STR_SIZE 256

int nextChar(void);
char* nextBinaryToken(void);
int binaryToken2int(char *p);
void writeStr(char **p, char c);

FILE *fp;
char Str[STR_SIZE+1];

int main(int argc,char *argv[]){

	if(argc<2){
		printf("no file name\n");
		exit(EXIT_FAILURE);
	}

	if((fp=fopen(argv[1],"r"))==NULL){
		printf("error: file %s can not open\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	char *head;
	int val;
	while((head=nextBinaryToken())!=NULL){
		val=binaryToken2int(head);
		printf("%s (%d)\n", head, val);
	}

	fclose(fp);

	return 0;
}

int nextChar(void){
	static int ch;
	if(ch==EOF){
		return ch;
	}
	ch=fgetc(fp);
	return ch;
}

char* nextBinaryToken(){
	static int ch=' ';
	char *pStr=Str;
	char *head=pStr;

	ch=nextChar();
	while(isspace(ch)){
		ch=nextChar();
	}

	if(ch==EOF){
		return NULL;
	}

	while(ch=='0'||ch=='1'){
		writeStr(&pStr,(char)ch);
		ch=nextChar();
	}
	writeStr(&pStr,'\0');

	return head;
}

void writeStr(char **p, char c){
	**p=c;
	(*p)++;
}

int binaryToken2int(char *p){
	int val;
	return val;
}