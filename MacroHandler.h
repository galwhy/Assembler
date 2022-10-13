#pragma once

struct Macro 
{
	char* macroName;
	char* macroContent;
};

struct Macro* findMacro(char** fileContent);

void ReplaceMacro(char** fileContent);

bool CheckMacroName(char* macroInFile, char* macroName);
