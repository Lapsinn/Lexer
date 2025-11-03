#include <stdio.h>
#include <ctype.h>
#include <string.h>

int isSpecialCharacter(char c)
{
    char specialChars[] = "+-*/%=><()[];,.:?!_\"&";
    for (int i = 0; i < strlen(specialChars); i++)
    {
        if (c == specialChars[i])
            return 1;
    }
    return 0;
}

void identifyCharacter(char c)
{
    if (isalpha(c))
    {
        printf("Token: %c\tType: Alphabet\n", c);
    }
    else if (isdigit(c))
    {
        printf("Token: %c\tType: Digit\n", c);
    }
    else if (isSpecialCharacter(c))
    {
        printf("Token: %c\tType: Special_Character\n", c);
    }
    else if (!isspace(c))
    {
        printf("Token: %c\tType: Unknown\n", c);
    }
}

int main()
{
    char input[200];

    printf("Enter an expression (ExtenCive Language): ");
    fgets(input, sizeof(input), stdin);

    printf("\n--- Character Classification ---\n");
    for (int i = 0; i < strlen(input); i++)
    {
        identifyCharacter(input[i]);
    }

    return 0;
}