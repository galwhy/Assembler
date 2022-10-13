#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*Gets a string and char and removes that char from the string.*/
void removeCharFromString(char* string, char removeChar) 
{
    int len = strlen(string);
    for (int i = 0; i < len; i++) // Go over the string.
    {
        if (string[i] == removeChar) // If the current char equals to the char we want to remove.
        {
            for (int j = i; j < len; j++)
            {
                string[j] = string[j + 1]; // Replace it by the next chars.
            }
            len--;
            i--;
        }
    }
}

/*removes all the white spaces from the start of the string.*/
void removeWhiteSpacesFromStart(char* string)
{
    int len = strlen(string);
    for (int i = 0; i < len; i++) // Go over the string.
    {
        if (string[i] == ' ' || string[i] == '\t') // If the Char equals to a white space
        {
            for (int j = i; j < len; j++)
            {
                string[j] = string[j + 1]; // Remove it.
            }
            len--;
            i--;
        }
        else break; // Else return.
    }
}

/*Gets a string and a substring and removes the substring from the string.*/
char* remove_substring(char* str, int removeIndex, int removeLength) 
{
    // You need to do some checking before calling malloc
    if (removeLength == 0) return str;
    size_t len = strlen(str);
    if (removeLength < 0 || removeIndex < 0 || removeIndex + removeLength > len) return NULL;
    size_t newLen = len - removeLength + 1;
    char* newStr = malloc(newLen);
    if (newStr == NULL) return NULL;
    char* tempPtr = newStr;
    // Now let's use the two familiar loops,
    // except printf("%c"...) will be replaced with *p++ = ...
    for (int i = 0; i < removeIndex; i++) {
        *tempPtr++ = str[i];
    }
    for (int i = removeIndex + removeLength; i < strlen(str); i++) {
        *tempPtr++ = str[i];
    }
    *tempPtr = '\0';
    return newStr;
}

/*Gets a string and two chars and replaces the first char by the second char.*/
void replaceCharInStr(char* str, char target, char replaceWith)
{
    for (int i = 0; i < strlen(str); i++) // Go over the string.
    {
        if (str[i] == target)
        {
            str[i] = replaceWith; // If we encounter the target char, replace it by the other char.
        }
    }
}