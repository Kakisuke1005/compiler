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
		for(int i=0;i<256;i++){
			if((tmp[i]>='a')&&(tmp[i]<='z')){
				tmp[i]=tmp[i]-('a'-'A');
			}
		}
		printf("%s",tmp);
	}

	fclose(fp);
	return 0;
}