#include <stdio.h>
#include <string.h>

#define MAX_TOKEN 50

const char *keywords[] = {"and","ask","by","continue","decimal","end","false",
"letter","not","number","or","repeat","show","start","stop","to","true","while","word","if","else"};
const char *reserved[] = {"exit","loop","main"};
const char *special[] = {"if","else","true","false","while"};

int is_in(const char *arr[], int n, char *token){
for(int i=0;i<n;i++) if(strcmp(arr[i],token)==0) return 1;
return 0;
}

int is_delim(char ch){ return ch==';'||ch=='{'||ch=='}'||ch=='('||ch==')'; }

void lexer(char line[]){
char token[MAX_TOKEN];
for(int i=0;i<strlen(line);){
while(line[i]==' '||line[i]=='\t') i++;
if(line[i]=='\0'||line[i]=='\n') break;

    if(is_delim(line[i])){
        char ch=line[i++];
        printf("Token: %c\nType: %s\n\n", ch,
            ch==';'?"Delimiter":(ch=='{'||ch=='}'?"Curly Brace":"Parenthesis"));
        continue;
    }

    int j=0;
    while(line[i]!=' '&&line[i]!='\t'&&line[i]!='\0'&&line[i]!='\n'&&!is_delim(line[i]))
        token[j++]=line[i++];
    token[j]='\0';

    if(is_in(special,5,token)) printf("Token: %s\nType: %s\n\n", token, token);
    else if(is_in(keywords,20,token)) printf("Token: %s\nType: Keyword\n\n", token);
    else if(is_in(reserved,3,token)) printf("Token: %s\nType: Reserved Word\n\n", token);
}

}

int main(){
char line[256];
printf("Enter code: ");
fgets(line,sizeof(line),stdin);
printf("\n=== LEXICAL ANALYSIS ===\n\n");
lexer(line);
return 0;
}
