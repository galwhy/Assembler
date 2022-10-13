#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "List.h"
#include "StringFunctions.h"
#include "MacroHandler.h"

#define ENDMACROOFFSET 8

/*Gets the content of the assembly file and returns a Macro struct holding the name and the content of the macro.*/
struct Macro* findMacro(char** fileContent)
{
	int macroLength = 0; //The length of the macro.
	char* macroStr; //A string that holds the macro.
	char* macroPtr; //A pointer to the start of the macro in the string. 
	char* endmacroPtr; //A pointer to the end of the macro.
	int macroIndex; //The index of the start of the macro in the string.
	int endmacroIndex; //The index of the end of the macro in the string.
	struct Macro* macro = malloc(sizeof(struct Macro)); // A struct that holds the name and content of the macro.

	macroPtr = strstr(*fileContent, "macro"); // Check if there is a macro in the file.
	if (macroPtr == NULL)
	{
		return NULL;
	}
	macroIndex = (int)(macroPtr - *fileContent); // Find the index of the macro.

	endmacroPtr = strstr(*fileContent, "endmacro"); // Find the end of the macro.
	if (endmacroPtr != NULL) endmacroIndex = (int)(endmacroPtr - *fileContent) + ENDMACROOFFSET;
	else
	{
		char* endLinePtr = strchr(*fileContent + macroIndex, '\n');
		endmacroPtr = strchr(endLinePtr + 1, '\n');
		endmacroIndex = (int)(endmacroPtr - *fileContent);
	}
	macroLength = endmacroIndex - macroIndex; // Calculate the length of the macro.

	macroStr = calloc(macroLength + 1, sizeof(char));

	if (macroStr != NULL)
	{
		strncpy(macroStr, &(*fileContent)[macroIndex], macroLength); // Copy the macro to a string.
		macroStr[macroLength] = '\0';
	}

	*fileContent = remove_substring(*fileContent, macroIndex, macroLength + 1); // Remove the found macro from the content of the file.

	char* macroName = NULL; // The name of the macro
	char* macroNamePtr = NULL; // A pointer to the macro name.
	int macroNameIndex = 0; // The index of the name in the string.
	int macroNameLength = 0; // The length of the name.

	if (macroStr != NULL) macroNamePtr = strchr(macroStr, ' ');
	macroNameIndex = (int)(macroNamePtr - macroStr) + 1;

	while (macroStr[macroNameIndex] != ' ' && macroStr[macroNameIndex] != '\n') // Calculate the length of the name.
	{
		macroNameLength++;
		macroNameIndex++;
	}

	macroName = calloc(macroNameLength + 1, sizeof(char));
	macroNameIndex -= macroNameLength;

	if (macroName != NULL)
	{
		strncpy(macroName, &macroStr[macroNameIndex], macroNameLength); // Copy the name to the string.
		macroName[macroNameLength] = '\0';
		removeCharFromString(macroName, '\r');
		macro->macroName = macroName;
	}

	int macroCodeIndex = macroNameIndex + macroNameLength + 1; // The index of the start of the macro code.
	int endmacroCodeIndex = 0; // The index of the end of the macro code.
	char* macroCode = NULL; // The code of the macro.
	int macroCodeLength = 0; // The length of the code.


	if (macroStr != NULL) endmacroPtr = strstr(macroStr, "endmacro");
	if (endmacroPtr != NULL)
	{
		endmacroCodeIndex = (int)(endmacroPtr - macroStr) - 1;
		macroCodeLength = endmacroCodeIndex - macroCodeIndex; // Calculate the length
	}
	else
	{
		macroCodeLength = macroLength - macroCodeIndex;
	}
	macroCode = calloc(macroCodeLength + 1, sizeof(char));

	if (macroCode != NULL)
	{
		strncpy(macroCode, &macroStr[macroCodeIndex], macroCodeLength); // Copy the code to a string.
		macroCode[macroCodeLength] = '\0';
		macro->macroContent = macroCode;
	}

	return macro; // Return the found macro.

}

/*Gets the content of the assembly file and replaces all the macros by their content.*/
void ReplaceMacro(char** fileContent)
{
	while (true)
	{
		struct Macro* macro = findMacro(fileContent);;
		if (macro == NULL) // Check if the macro was found, if not break.
		{
			break;
		}

		int macroNameIndex = 0; // The index of the name in the content of the file to replace.
		char* newFileContent = NULL; // Holds the file content with the name replaced by the code.
		char* macroNamePtr = NULL; // Pointer to the name in the file.
		int nextOffset = 0;
		int macroCodeLength = strlen(macro->macroContent); // The length of the name.
		int macroNameLength = strlen(macro->macroName); // The length of the code.
		while (true)
		{
			if (macro->macroName != NULL) macroNamePtr = strstr(*fileContent + macroNameIndex + nextOffset, macro->macroName); // Find the first accurance of the name in the file.
			if (macroNamePtr == NULL || macro->macroContent == NULL) // If not found, break.
			{
				break;
			}
			macroNameIndex = (int)(macroNamePtr - *fileContent);
			int fileContentSize = strlen(*fileContent) + macroCodeLength - macroNameLength; // The size of the new file content.
			if (CheckMacroName(macroNamePtr, macro->macroName))//Check if the name matches in the file.
			{
				newFileContent = calloc(fileContentSize + 1, sizeof(char)); // Expand the size of the string to fit the code.
				strncpy(newFileContent, *fileContent, macroNameIndex); // Copy the file up to the name.
				strncpy(newFileContent + macroNameIndex, macro->macroContent, macroCodeLength); // Copy the macro content.
				strncpy(newFileContent + macroNameIndex + macroCodeLength, *fileContent + macroNameIndex + macroNameLength, strlen(*fileContent) - (macroNameIndex + macroNameLength)); // Copy the rest.

				free(*fileContent);
				*fileContent = newFileContent;
				nextOffset = macroCodeLength; // Check for the next one after the code.
			}
			else nextOffset = macroNameLength; // Check for the next one after the name.
		}
		free(macro->macroContent);
		free(macro->macroName);
		free(macro);
	}
}

/*Checks if the macro name matches the line in the file, ignores white spaces.*/
bool CheckMacroName(char* macroInFile, char* macroName)
{
	char* firstEndOfLinePtr = strchr(macroInFile, '\n'); // Find the end of the line in the file.
	int firstEndOfLineIndedx = 0;
	if (firstEndOfLinePtr == NULL) firstEndOfLineIndedx = strlen(macroInFile);// Find the index of the end of the line.
	else firstEndOfLineIndedx = (int)(firstEndOfLinePtr - macroInFile);// Find the index of the end of the line.
	
	int macroNameLength = strlen(macroName);

	int i = 0;
	int j = 0;
	while (i < firstEndOfLineIndedx)
	{
		char cmpChar = '\0';
		if (j < macroNameLength) cmpChar = macroName[j];

		if (macroInFile[i] == ' ' || macroInFile[i] == '\r')// If there is a white space ignore it.
			i++;
		else if (macroInFile[i] != cmpChar)// If there is no white space and the characters dont match return false.
			return false;
		else// If the characters match continue checking the rest.
		{
			i++; j++;
		}
	}
	return true;// If everything match return true.
}


