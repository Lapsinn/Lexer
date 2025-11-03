#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void removeBlanks(char line[]) {
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            continue;
        }        
        printf("%c", line[i]);
    }
}

int main() {
    char line[256];

    printf("Enter a line of code: ");
    fgets(line, sizeof(line), stdin);

    printf("\nAfter removing blanks/spaces:\n");
    removeBlanks(line);

    return 0;
}
