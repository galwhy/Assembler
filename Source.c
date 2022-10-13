#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "List.h"
#include "StringFunctions.h"
#include "MacroHandler.h"
#include "FirstScan.h"
#include "SecondScan.h"

#define NAMEEXTENTIONS 5

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		char* name = calloc(strlen(argv[i]) + NAMEEXTENTIONS, sizeof(char));
		sprintf(name, "%s.as", argv[i]);

		FILE* OriginalFilePtr;
		OriginalFilePtr = fopen(name, "r"); // Opening the assembly file.
		free(name);

		if (OriginalFilePtr == NULL)
		{
			printf("Error opening file");
			exit(1);
		}

		fseek(OriginalFilePtr, 0, SEEK_END);
		size_t size = ftell(OriginalFilePtr);
		fseek(OriginalFilePtr, 0, SEEK_SET);
		char* fileContent = calloc(size + 1, sizeof(char));
		if (fileContent != NULL) fread(fileContent, size, 1, OriginalFilePtr); // If the file exists then read it.
		fclose(OriginalFilePtr);

		ReplaceMacro(&fileContent); // Replace the macro's in the file.

		struct List* externals = List_New();
		struct List* entries = List_New();
		struct List* labelList = List_New();
		struct List* commandLineList = List_New();
		struct List* encodedLineList = List_New();
		struct List* dataLineList = List_New();

		bool firstStageErrors = false;
		bool secondStageErrors = false;

		char* fileContentDup = calloc(strlen(fileContent) + 1, sizeof(char));
		if (fileContentDup != NULL) strncpy(fileContentDup, fileContent, strlen(fileContent));
		firstStageErrors = CreateAssemblyLineList(fileContentDup, entries, externals, labelList, commandLineList, dataLineList); // Do the first scan of the code.
		free(fileContentDup);

		secondStageErrors = EncodeLineList(commandLineList, dataLineList, labelList, externals, encodedLineList); // Do the second scan.

		if (!(firstStageErrors || secondStageErrors)) CreateOutputFiles(argv[i], fileContent, encodedLineList, externals, entries); // If the assembler didn't find any errors create the output files.
		else printf("Assembler did not finish. Error were found.\n"); // If the assembler found errors don't create the output files.

		List_Delete(encodedLineList); // Free the encoded line list.
		List_Delete(dataLineList); // Free the data list and labels
		List_Delete(commandLineList); // Free the command list and labels.
		List_Delete(entries); // Free the entries list.
		List_Delete(externals); // Free the eternals list.
		List_Delete(labelList); // Free the label list.
		free(fileContent); // Free the content of the file.
	}
}

