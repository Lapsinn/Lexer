#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool isDelimiter(char ch)
{
    return (ch == ' ' || ch == '+' || ch == '-' || ch == '*' ||
            ch == '/' || ch == ',' || ch == ';' || ch == '>' ||
            ch == '<' || ch == '=' || ch == '(' || ch == ')' ||
            ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
            ch == '@' || ch == '#' || ch == '$' || ch == '%' ||
            ch == '!' || ch == '&' || ch == '|' || ch == '^');
}

bool isValidIdentifier(char* str)
{
    int length = strlen(str);

    if (length == 0)
        return false;

    if ((str[0] >= '0' && str[0] <= '9') || isDelimiter(str[0]))
        return false;

    // Rule 4: Check that all middle characters are valid (letters, digits, or underscores)
    for (int i = 1; i < length; i++)
    {
        if (!((str[i] >= 'a' && str[i] <= 'z') ||
              (str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= '0' && str[i] <= '9') ||
              str[i] == '_'))
        {
            return false;
        }
    }

    // If all checks passed
    return true;
}
