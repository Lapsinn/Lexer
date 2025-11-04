#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void removeComments(char line[]) {
    bool insideComment = false;

    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == '#') { 
            insideComment = true; 
            break;
        }
        printf("%c", line[i]); 
    }
}

int main() {
    char line[256];

    printf("Enter a line of code: ");
    fgets(line, sizeof(line), stdin);

    printf("\nAfter removing comments:\n");
    removeComments(line);

    return 0;
}
