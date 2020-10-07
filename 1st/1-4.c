// 306145 NakajimaKazuki

#include<stdio.h>
#include<stdlib.h>

int main(int argc,char *argv[])
{
	FILE *fp;
	char tmp[256];

	fp=fopen(argv[1],"r");
	if(fp==NULL){
		printf("Cannot open the file!!!!\n");
		exit(1);
	}

	while((fgets(tmp,256,fp))!=NULL){
		printf("%s",tmp);
	}

	fclose(fp);
	return 0;
}