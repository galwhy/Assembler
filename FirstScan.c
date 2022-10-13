#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "List.h"
#include "StringFunctions.h"
#include "FirstScan.h"
#include "SecondScan.h"
/*Deletes the address in the externals list*/
void AddressDelete(struct Node* node)
{
	char* address = node->DataPtr;
	free(address);
}

/*Deletes the contents of an External Label struct.*/
void ExternalLabelDelete(struct Node* node)
{
	struct ExternalLabel* label = node->DataPtr;
	free(label->LabelPtr); // Delete the label pointer.
	List_Delete(label->AdressList); // Delete the List of addresses.
}

/*Deletes the content of an Entry Label struct.*/
void EntryLabelDelete(struct Node* node)
{
	struct EntryLabel* label = node->DataPtr;
	free(label->Address); // Delete the Address pointer.
	free(label->LabelPtr); // Delete the Label pointer.
}

/*Deletes the content of an Assembly Line struct.*/
void AssemblyLineDelete(struct Node* node)
{
	struct AssemblyLine* line = node->DataPtr;
	free(line->Command); // Delete the command pointer.
	free(line->Label); // Delete the label pointer.
	for (int i = 0; i < line->numOfPrm; i++) {
		free(line->PrmArray[i]); // Delete all the pointer in the array.
	}
	free(line->PrmArray); // Delete the array.
}

/*Deletes the content of a Label struct.*/
void LabelClear(struct Node* node)
{
	struct Label* label = node->DataPtr;
	free(label->LabelPtr); // Delete the Label pointer.
}

/*Gets a new label and a list of labels and checks if the new label already exists.*/
bool checkIfLabelExists(struct List* labelList, char* label)
{
	struct Node* currentNode = labelList->FirstPtr; // Go over the list of Labels
	while (currentNode != NULL)
	{
		struct Label* currentLabel = currentNode->DataPtr;
		if (!strcmp(currentLabel->LabelPtr, label)) // Check if the new label matches with the old ones.
		{
			return true; // If so return true;
		}
		currentNode = currentNode->NextPtr;
	}
	return false; // Else return false.
}

/*Finds a label and its address and adds it to a label list.*/
int CheckForLabel(struct List* labelList, struct AssemblyLine* assemblyLine, int currentAddress, struct List* entries, struct List* externals)
{
	struct Label* label = malloc(sizeof(struct Label)); // Create a new label.
	label->BlockAdress = currentAddress; // Save the address of the label.
	label->LabelPtr = assemblyLine->Label; // Save the name of the label.

	struct Node* currentEntryNode = entries->FirstPtr;
	while (currentEntryNode != NULL && assemblyLine->Label != NULL) // Check if the label is in the Entries list
	{
		struct EntryLabel* entry = currentEntryNode->DataPtr;
		if (!strcmp(entry->LabelPtr, assemblyLine->Label))
		{
			entry->Address = EncodeBase32(currentAddress); // If so put its address in base 32 in the entries list.
			break;
		}
		currentEntryNode = currentEntryNode->NextPtr;
	}

	if (assemblyLine->Command != NULL && strlen(assemblyLine->Command) > 0)
	{
		if (!strcmp(assemblyLine->Command, ".string")) // Check if the label is a string.
		{
			for (int i = 0; i < assemblyLine->numOfPrm; i++) currentAddress += strlen(assemblyLine->PrmArray[i]) + 1; // Add to the address the length of the string.
		}
		else if (!strcmp(assemblyLine->Command, ".data")) // Check if the label is a data.
		{
			currentAddress += assemblyLine->numOfPrm; // Add to the address the number of params
		}
		else if (!strcmp(assemblyLine->Command, ".struct")) // Check if the label is a struct.
		{
			currentAddress++; // Add to the list one for the number in the struct.
			currentAddress += strlen(assemblyLine->PrmArray[1]); // Add to the address the length of the string. 
		}
		else
		{
			currentAddress++;
			int registerCount = 0; // Count how many registers there are in the command.
			for (int i = 0; i < assemblyLine->numOfPrm; i++) // Go over all the params.
			{
				struct Node* currentExternNode = externals->FirstPtr;
				char* prm = assemblyLine->PrmArray[i];

				if (strlen(prm) == 2 && prm[0] == 'r' && prm[1] >= '0' && prm[1] <= '7') // Check if the param is a register.
				{
					registerCount++;
					continue;
				}
				currentAddress++;
				char* structPtr = strchr(prm, '.');
				if (structPtr != NULL) // Check if the param is a struct.
				{
					int index = (int)(structPtr - prm);
					prm = remove_substring(prm, index, strlen(prm) - index);
				}
				while (currentExternNode != NULL) // Check if the param uses an external label
				{
					struct ExternalLabel* external = currentExternNode->DataPtr;
					if (!strcmp(external->LabelPtr, prm))
					{
						List_AddNode(external->AdressList, EncodeBase32(currentAddress - 1), &AddressDelete); // If so add its address to the list of externals.
						break;
					}
					currentExternNode = currentExternNode->NextPtr;
				}
				if (structPtr != NULL) currentAddress++; // If the param is a struct add one for the index.
			}
			if (registerCount >= 1) currentAddress++; //
		}

	}

	if (assemblyLine->Label != NULL) // If the label is not null add it to the list of labels.
	{
		List_AddNode(labelList, label, NULL);
	}
	else free(label);

	return currentAddress; // Return the updated address.
}

/*Gets the content of the assembly file and creates a list of struct Assembly Line.*/
bool CreateAssemblyLineList(char* fileContent, struct List* entries, struct List* externals, struct List* labelList, struct List* commandLineList, struct List* dataLineList)
{
	char delim[] = "\n";
	char* linePtr = strtok(fileContent, delim); // A pointer to the start of a line.
	int currentAddress = 100; // The current address.
	int currentLine = 1; // The current Line.
	bool errorsExist = false; // True if there are errors and false if there aren't.

	while (linePtr != NULL) // Go over all the lines in the file.
	{
		if (strlen(linePtr) > 80) // Check if the length of the line is greater than 80.
		{
			printf("Line is too long. Line: %d\n", currentLine); // If so print an error.
			errorsExist = true;
		}
		removeWhiteSpacesFromStart(linePtr); // Remove all the white spaces from the start of the line.
		removeCharFromString(linePtr, '\r'); // remove returns

		if (*linePtr == ';') // check if the line is a comment, if so ignore it.
		{
			linePtr = strtok(NULL, delim);
			currentLine++;
			continue;
		}

		struct AssemblyLine* newLine = malloc(sizeof(struct AssemblyLine)); // Create a struct to save the line.
		newLine->LineNumber = currentLine; // Save the number of the line.
		newLine->Command = NULL;
		newLine->Label = NULL;
		newLine->numOfPrm = 0;
		newLine->PrmArray = NULL;

		char* labelPtr = strchr(linePtr, ':'); // Search for a label.
		int labelIndex = 0; // The index of the label.
		char* label = NULL; // The label string.

		char* command = NULL; // The command string.

		char* commandPtr = NULL; // The pointer to the command.
		unsigned int commandStartIndex = 0; // The index of the start of the command.
		int commandEndIndex = -1; // The index to the end of the command

		if (labelPtr != NULL)
		{
			labelIndex = (int)(labelPtr - linePtr); // Check for the index of the label.
			label = calloc(labelIndex + 1, sizeof(char));
			strncpy(label, linePtr, labelIndex);
			if (label != NULL && strlen(label) > 30) // Check if the name Of the label is greater than 30.
			{
				printf("Label name is too long. Line: %d\n", currentLine); // If so print an error.
				errorsExist = true;
			}
			newLine->Label = label;

			if (checkIfLabelExists(labelList, label)) // Check if the label already exists.
			{
				errorsExist = true;
				printf("Label already Exists. Line: %d\n", currentLine); // If so print an error.
			}

			commandPtr = strchr(linePtr + labelIndex, ' '); // Search for the start of the command
			commandStartIndex = (int)(commandPtr - linePtr) + 1;
		}

		while (commandStartIndex < strlen(linePtr))
		{
			if (*(linePtr + commandStartIndex) != ' ') // Search for the start of the command.
			{
				commandPtr = strchr(linePtr + commandStartIndex, ' ');
				if (commandPtr == NULL) commandEndIndex = strlen(linePtr);
				else commandEndIndex = (int)(commandPtr - linePtr);
				command = calloc(commandEndIndex - commandStartIndex + 1, sizeof(char));
				strncpy(command, linePtr + commandStartIndex, commandEndIndex - commandStartIndex);
				command[commandEndIndex - commandStartIndex] = '\0';
				removeCharFromString(command, '\r'); // remove returns
				newLine->Command = command;
				break;

			}
			commandStartIndex++;
		}

		if (command != NULL) // Check for externs and entries. 
		{
			bool isEntry = !strcmp(command, ".entry");
			bool isExtern = !strcmp(command, ".extern");
			if (isEntry || isExtern)
			{
				if (label != NULL) printf("Warning. ignoring label is line: %d\n", currentLine);
				char* labelName = calloc(30, sizeof(char));
				if (labelName != NULL) strncpy(labelName, linePtr + commandEndIndex, strlen(linePtr + commandEndIndex));
				removeCharFromString(labelName, ' ');

				if (isEntry)
				{
					struct EntryLabel* label = malloc(sizeof(struct EntryLabel));
					label->LabelPtr = labelName;
					label->Address = NULL;
					List_AddNode(entries, label, &EntryLabelDelete); // Add the entry to the list of entries 
				}
				else
				{
					struct ExternalLabel* label = malloc(sizeof(struct ExternalLabel));
					label->LabelPtr = labelName;
					label->AdressList = List_New();
					List_AddNode(externals, label, &ExternalLabelDelete); // Add it to the list of externals.
				}

				linePtr = strtok(NULL, delim);
				currentLine++;
				continue; // Continue to the next line.
			}
		}
		
		removeCharFromString(linePtr + commandEndIndex, ' '); // remove white spaces
		removeCharFromString(linePtr + commandEndIndex, '\"'); // remove Apostrophes

		int numOfPrms = 0;
		int operandIndex = 0;
		char* operandPtr = NULL;
		if (commandEndIndex == strlen(linePtr) || *(linePtr + commandEndIndex) == ';') // check if there are parameters
		{
			newLine->PrmArray = NULL;
			newLine->numOfPrm = 0;
		}
		else
		{
			numOfPrms = 1;
			while (true) // find out how many operands are there.
			{
				operandPtr = strchr(linePtr + operandIndex + 1, ',');
				if (operandPtr == NULL) break;
				operandIndex = (int)(operandPtr - linePtr);
				numOfPrms++;
			}
		}

		newLine->PrmArray = calloc(numOfPrms, sizeof(char*));
		newLine->numOfPrm = numOfPrms;

		operandPtr = linePtr + commandEndIndex; // A pointer to the first operand.


		for (int i = 0; i < numOfPrms; i++)
		{
			char* nextPrmPtr = strchr(operandPtr, ','); // A pointer to the next operand.
			int prmLength = 0;
			if (nextPrmPtr == NULL) // If there is no next param check for comments and save the length of the operand.
			{
				char* endOfLinePtr = strchr(operandPtr, ';');
				if (endOfLinePtr == NULL)prmLength = strlen(operandPtr);
				else prmLength = strlen(operandPtr) - strlen(endOfLinePtr);
			}
			else // Else Find the length of the operand untill the next one.
			{
				int nextPrnIndex = (int)(nextPrmPtr - operandPtr);
				prmLength = nextPrnIndex;
			}
			char* prm = calloc(prmLength + 1, sizeof(char));
			if (prm != NULL)
			{
				strncpy(prm, operandPtr, prmLength);
				prm[prmLength] = '\0';
			}
			if (newLine->PrmArray != NULL)
			{
				newLine->PrmArray[i] = prm; // Add the operand to the array.
			}

			operandPtr = nextPrmPtr + 1;
		}

		currentAddress = CheckForLabel(labelList, newLine, currentAddress, entries, externals); // check for labels and update the current address
		if (newLine->Command != NULL && strlen(newLine->Command) > 0)
		{
			if (!strcmp(newLine->Command, ".string") || !strcmp(newLine->Command, ".data") || !strcmp(newLine->Command, ".struct"))List_AddNode(dataLineList, newLine, &AssemblyLineDelete); // If the command is a data command, add the line to the data lisr
			else List_AddNode(commandLineList, newLine, &AssemblyLineDelete); // Else add the command to the command list.
		}

		linePtr = strtok(NULL, delim);
		currentLine++;
	}
	return errorsExist; // Return if the scan found any errors.
}