// 306145 K.Nakajima
 
#include<stdio.h>
 
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
    int count=0;
 
    getWord();
    printConfiguration(s,Str+count);
    printf("\n");
 
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