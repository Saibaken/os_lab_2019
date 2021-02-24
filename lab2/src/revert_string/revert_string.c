#include "revert_string.h"

void RevertString(char *str)
{
	char temp;
    int strlen = 0;
    int i = 0;
    char* ptr = str;
    while (*ptr!='\0')
    {
        strlen++;
        ptr++;
    }
    for (i = 0; i<strlen/2; i++)
    {
        temp = str[i];
        str[i] = str[strlen-i-1];
        str[strlen-i-1] = temp;
    }
}

