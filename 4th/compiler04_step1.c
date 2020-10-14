// 306145 K.Nakajima

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>

int nextChar(void);

FILE *fp;

int main(int argc,char *argv[]){

	if(argc<2){
		printf("no file name\n");
		exit(EXIT_FAILURE);
	}

	if((fp=fopen(argv[1],"r"))==NULL){
		printf("error: file %s can not open\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	int ch;
	while((ch=nextChar())!=EOF){
		printf("%c,", (char)ch);
	}
	printf("\n");

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