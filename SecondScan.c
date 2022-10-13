#define _GNU_SOURCE
#define MAX_LINE_LENGTH 80
#define BYTE 255
#define TENBITS 1023
#define BASE32LENGTH 2
// encode indexes
#define OPECODEINDEX 6
#define SOURCEPARAMTYPEINDEX 4
#define DESTPARAMTYPEINDEX 2
#define NUMBERINDEX 2
#define SOURCEREGISTERINDEX 6
#define DESTREGISTERINDEX 2
//A R E flags
#define AFLAG 0
#define EFLAG 1
#define RFLAG 2
//var types
#define NUMBER 0
#define VAR 1
#define STRUCT 2
#define REGISTER 3
//file
#define EXTENTIONOFFSET 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "List.h"
#include "StringFunctions.h"
#include "FirstScan.h"
#include "SecondScan.h"

/*Deletes the content of Encoded Line struct.*/
void EncodedLineDelete(struct Node* node)
{
	struct EncodedLine* encodedLine = node->DataPtr;
	free(encodedLine->BinCode); // Delete the BinCode.
	free(encodedLine->LineAddress); // Delete the address.
}

/*Creates all the output files*/
void CreateOutputFiles(char* name, char* amContent, struct List* encodedLineList, struct List* externals, struct List* entries)
{
	char* fileName = calloc(strlen(name) + EXTENTIONOFFSET, sizeof(char));

	sprintf(fileName, "%s.am", name);
	FILE* currentFile;
	currentFile = fopen(fileName, "w"); // Open a new file with the extention am.
	fprintf(currentFile, "%s", amContent); // Print the content of the file without the macro.
	fclose(currentFile); // Close the file.

	sprintf(fileName, "%s.obj", name);
	currentFile = fopen(fileName, "w"); // Open a new file with extention obj.
	struct Node* currentNode = encodedLineList->FirstPtr;
	while (currentNode != NULL) // Go over the encodedLine list and print in the file the its content.
	{
		struct EncodedLine* encodedLine = currentNode->DataPtr;
		fprintf(currentFile, "%s  %s\n", encodedLine->LineAddress, encodedLine->BinCode);
		currentNode = currentNode->NextPtr;
	}
	fclose(currentFile); // Close the file.

	sprintf(fileName, "%s.ent", name);
	currentFile = fopen(fileName, "w"); // Open a new file with the extention ent.
	currentNode = entries->FirstPtr;
	while (currentNode != NULL) // Go over the entries list and print its content to the file.
	{
		struct EntryLabel* entry = currentNode->DataPtr;
		fprintf(currentFile, "%s  %s\n", entry->LabelPtr, entry->Address);
		currentNode = currentNode->NextPtr;
	}
	fclose(currentFile); // Close the file

	sprintf(fileName, "%s.ext", name);
	currentFile = fopen(fileName, "w"); // Open a new file with the extention ext.
	currentNode = externals->FirstPtr;
	while (currentNode != NULL) // Go over the external list and print its content to the file.
	{
		struct ExternalLabel* external = currentNode->DataPtr;
		struct Node* currentExternAddress = external->AdressList->FirstPtr;
		while (currentExternAddress != NULL)
		{
			char* address = currentExternAddress->DataPtr;
			fprintf(currentFile, "%s  %s\n", external->LabelPtr, address);
			currentExternAddress = currentExternAddress->NextPtr;
		}
		currentNode = currentNode->NextPtr;
	}
	fclose(currentFile); // Close the file.

	printf("Assembler completed. Output files were created.\n");
}

/*Gets the line list and creates a new list with the encoded values*/
bool EncodeLineList(struct List* commandLineList, struct List* dataLineList, struct List* labelList, struct List* externals, struct List* encodedLineList)
{
	struct Node* currentLineListNode = commandLineList->FirstPtr;
	struct AssemblyLine* assemblyLine = NULL;

	bool ErrorsExist = false;
	int currentAddress = 100;
	while (currentLineListNode != NULL) // encode the code section in the object file
	{
		assemblyLine = currentLineListNode->DataPtr;
		if (assemblyLine->Command != NULL)
		{
			int commandEncode = 0; // The Enocoded command.
			int sourcePrmEncode = 0; // The encoded source param.
			int destPrmEncode = 0; // The encoded dest param.
			int sourceIndexEncode = 0; // The encoded index of the source param.
			int destIndexEncode = 0; // The encoded index of the dest param.
			int RegistersEncode = 0; // The encoded registers.


			int commandOpecode = convertStingToEnum(assemblyLine->Command); // Find the opcode of the command.
			int allowedNumOfPrms = CheckOpernadNumber(assemblyLine->Command); // Find how many operands the command needs.

			if (commandOpecode == -1) // If the opcode is -1 the command doesn't exist.
			{
				ErrorsExist = true;
				printf("Command is not reconized. Line: %d\n", assemblyLine->LineNumber);
			}
			else if (allowedNumOfPrms > assemblyLine->numOfPrm) // If the allowd operand is bigger then the number of operand than there are too few operands.
			{
				ErrorsExist = true;
				printf("To few operands: %d\n", assemblyLine->LineNumber);
			}
			else if (allowedNumOfPrms < assemblyLine->numOfPrm) // If the allowed operands is smaller than the number of operands then there are too many opernads.
			{
				ErrorsExist = true;
				printf("To many operands: %d\n", assemblyLine->LineNumber);
			}

			commandEncode = commandEncode | (commandOpecode << OPECODEINDEX); // Insert the opcode to the bin encode.


			char* sourceprm = assemblyLine->PrmArray[0]; // The source param.
			char* destprm = assemblyLine->PrmArray[1]; // The dest param.

			if (assemblyLine->numOfPrm > 0)
			{
				//Check the source param.
				char* indexPtr = strchr(sourceprm, '.');
				if (strlen(sourceprm) == 2 && sourceprm[0] == 'r' && sourceprm[1] >= '0' && sourceprm[1] <= '7') // Source param is a register.
				{
					if (!strcmp(assemblyLine->Command, "lea")) // If the command is lea then throw an error.
					{
						printf("Illegal operand Type. Line: %d\n", assemblyLine->LineNumber);
						ErrorsExist = true;
					}

					if (assemblyLine->numOfPrm == 1) commandEncode = commandEncode | REGISTER << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					else commandEncode = commandEncode | REGISTER << SOURCEPARAMTYPEINDEX;
					RegistersEncode = RegistersEncode | atoi(sourceprm + 1) << SOURCEREGISTERINDEX; // Insert the encode of the register.
				}
				else if (sourceprm[0] == '#') // Source is a number.
				{
					if (!strcmp(assemblyLine->Command, "lea") || (strcmp(assemblyLine->Command, "prn") && allowedNumOfPrms == 1)) // If the command is lea of the command has one param and is not print then throw an error.
					{
						printf("Illegal operand Type. Line: %d\n", assemblyLine->LineNumber);
						ErrorsExist = true;
					}

					if (assemblyLine->numOfPrm == 1) commandEncode = commandEncode | NUMBER << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					else commandEncode = commandEncode | NUMBER << SOURCEPARAMTYPEINDEX;
					if (atoi(sourceprm + 1) >= 0) sourceIndexEncode = sourceIndexEncode | atoi(sourceprm + 1) << NUMBERINDEX; // If the number is bigger than zero insert its bin value
					else sourceIndexEncode = (unsigned)(sourceIndexEncode | (((~abs(atoi(sourceprm + 1))) + 1) & BYTE) << NUMBERINDEX); // Else calculate its negitive value in bin.
				}
				else if (indexPtr != NULL)  // Source is a struct.
				{
					if (assemblyLine->numOfPrm == 1) commandEncode = commandEncode | STRUCT << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					else commandEncode = commandEncode | STRUCT << SOURCEPARAMTYPEINDEX;
					int index = (int)(indexPtr - sourceprm); // The index in the struct.
					sourceprm = remove_substring(sourceprm, index, strlen(sourceprm) - index); // Romove the index from the param.
					sourceIndexEncode = sourceIndexEncode | atoi(indexPtr + 1) << NUMBERINDEX;
					int labelAddress = -1;
					struct Node* currentNode = labelList->FirstPtr;
					while (currentNode != NULL) // Check if the param exists
					{
						struct Label* label = currentNode->DataPtr;
						if (!strcmp(sourceprm, label->LabelPtr)) // If so encode its address.
						{
							labelAddress = label->BlockAdress;
							sourcePrmEncode = sourcePrmEncode | RFLAG;
							break;
						}
						currentNode = currentNode->NextPtr;
					}
					if (labelAddress == -1) // If the label is not found check if its an external label.
					{
						currentNode = externals->FirstPtr;
						while (currentNode != NULL)
						{
							struct Label* label = currentNode->DataPtr;
							if (!strcmp(sourceprm, label->LabelPtr)) // If so Put 0 as the address.
							{
								labelAddress = 0;
								sourcePrmEncode = sourcePrmEncode | EFLAG;
								break;
							}
							currentNode = currentNode->NextPtr;
						}
					}
					if (labelAddress == -1) // If the param is still not found throw an error.
					{
						ErrorsExist = true;
						printf("Operand is not recognized. Line: %d\n", assemblyLine->LineNumber);
					}
					sourcePrmEncode = sourcePrmEncode | labelAddress << NUMBERINDEX;
				}
				else // Source a normal var.
				{
					if (assemblyLine->numOfPrm == 1) commandEncode = commandEncode | VAR << DESTPARAMTYPEINDEX; // Insert the opcode to the bin encode.
					else commandEncode = commandEncode | VAR << SOURCEPARAMTYPEINDEX;
					int labelAddress = -1;
					struct Node* currentNode = labelList->FirstPtr;
					while (currentNode != NULL) // Check if the param exists.
					{
						struct Label* label = currentNode->DataPtr;
						if (!strcmp(sourceprm, label->LabelPtr)) // If so encode its address.
						{
							labelAddress = label->BlockAdress;
							sourcePrmEncode = sourcePrmEncode | RFLAG;
							break;
						}
						currentNode = currentNode->NextPtr;
					}
					if (labelAddress == -1) // If the label is not found check if its an external label.
					{
						currentNode = externals->FirstPtr;
						while (currentNode != NULL)
						{
							struct Label* label = currentNode->DataPtr;
							if (!strcmp(sourceprm, label->LabelPtr)) // If so Put 0 as the address.
							{
								labelAddress = 0;
								sourcePrmEncode = sourcePrmEncode | EFLAG;
								break;
							}
							currentNode = currentNode->NextPtr;
						}
					}
					if (labelAddress == -1) // If the param is still not found throw an error.
					{
						ErrorsExist = true;
						printf("Operand is not recognized. Line: %d\n", assemblyLine->LineNumber);
					}
					sourcePrmEncode = sourcePrmEncode | labelAddress << NUMBERINDEX;
				}
			}
			if (assemblyLine->numOfPrm == 2)
			{
				//check dest param
				char* indexPtr = strchr(destprm, '.');
				if (strlen(destprm) == 2 && destprm[0] == 'r' && destprm[1] >= '0' && destprm[1] <= '7') // Dest param is a register.
				{
					commandEncode = commandEncode | REGISTER << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					RegistersEncode = RegistersEncode | atoi(destprm + 1) << DESTPARAMTYPEINDEX; // Insert the encode of the register.
				}
				else if (destprm[0] == '#') // Dest param is a number.
				{
					if (strcmp(assemblyLine->Command, "cmp"))
					{
						printf("Illegal operand Type. Line: %d\n", assemblyLine->LineNumber);
						ErrorsExist = true;
					}

					commandEncode = commandEncode | NUMBER << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					if (atoi(destprm + 1) >= 0) destPrmEncode = destPrmEncode | atoi(destprm + 1) << NUMBERINDEX; // If the number is bigger than zero insert its bin value
					else destPrmEncode = (unsigned)(destPrmEncode | (~atoi(destprm + 1) + 1) << NUMBERINDEX); // Else calculate its negitive value in bin.
				}
				else if (indexPtr != NULL)  // Dest is a struct.
				{
					commandEncode = commandEncode | STRUCT << DESTPARAMTYPEINDEX; // Insert the type of the param to the command bin code.
					int index = (int)(indexPtr - destprm); // The index in the struct.
					destprm = remove_substring(destprm, index, strlen(destprm) - index); // Romove the index from the param.
					destIndexEncode = destIndexEncode | atoi(destprm) << NUMBERINDEX;
					int labelAddress = -1;
					struct Node* currentNode = labelList->FirstPtr;
					while (currentNode != NULL) // Check if the param exists
					{
						struct Label* label = currentNode->DataPtr;
						if (!strcmp(destprm, label->LabelPtr)) // If so encode its address.
						{
							labelAddress = label->BlockAdress;
							destPrmEncode = destPrmEncode | RFLAG;
							break;
						}
						currentNode = currentNode->NextPtr;
					}
					if (labelAddress == -1) // If the label is not found check if its an external label.
					{
						currentNode = externals->FirstPtr;
						while (currentNode != NULL)
						{
							struct Label* label = currentNode->DataPtr;
							if (!strcmp(sourceprm, label->LabelPtr)) // If so Put 0 as the address.
							{
								labelAddress = 0;
								destPrmEncode = destPrmEncode | EFLAG;
								break;
							}
							currentNode = currentNode->NextPtr;
						}
					}
					if (labelAddress == -1) // If the param is still not found throw an error.
					{
						ErrorsExist = true;
						printf("Operand is not recognized. Line: %d\n", assemblyLine->LineNumber);
					}
					destPrmEncode = destPrmEncode | labelAddress << NUMBERINDEX;
				}
				else // Dest is a normal var.
				{
					commandEncode = commandEncode | VAR << DESTPARAMTYPEINDEX;  // Insert the opcode to the bin encode.
					int labelAddress = -1;
					struct Node* currentNode = labelList->FirstPtr;
					while (currentNode != NULL) // Check if the param exists
					{
						struct Label* label = currentNode->DataPtr;
						if (!strcmp(destprm, label->LabelPtr)) // If so encode its address.
						{
							labelAddress = label->BlockAdress;
							destPrmEncode = destPrmEncode | RFLAG;
							break;
						}
						currentNode = currentNode->NextPtr;
					}
					if (labelAddress == -1) // If the label is not found check if its an external label.
					{
						currentNode = externals->FirstPtr;
						while (currentNode != NULL)
						{
							struct Label* label = currentNode->DataPtr;
							if (!strcmp(destprm, label->LabelPtr)) // If so Put 0 as the address.
							{
								labelAddress = 0;
								destPrmEncode = destPrmEncode | EFLAG;
								break;
							}
							currentNode = currentNode->NextPtr;
						}
					}
					if (labelAddress == -1) // If the param is still not found throw an error.
					{
						ErrorsExist = true;
						printf("Operand is not recognized. Line: %d\n", assemblyLine->LineNumber);
					}
					destPrmEncode = destPrmEncode | labelAddress << NUMBERINDEX;
				}

			}


			if (!ErrorsExist)
			{
				InsertEncodedList(currentAddress, commandEncode, encodedLineList); currentAddress++;
				if (sourcePrmEncode != 0)
				{
					InsertEncodedList(currentAddress, sourcePrmEncode, encodedLineList); currentAddress++;
				}
				if (sourceIndexEncode != 0)
				{
					InsertEncodedList(currentAddress, sourceIndexEncode, encodedLineList); currentAddress++;
				}
				if (RegistersEncode != 0)
				{
					InsertEncodedList(currentAddress, RegistersEncode, encodedLineList); currentAddress++;
				}
				if (destPrmEncode != 0)
				{
					InsertEncodedList(currentAddress, destPrmEncode, encodedLineList); currentAddress++;
				}
				if (destIndexEncode != 0)
				{
					InsertEncodedList(currentAddress, destIndexEncode, encodedLineList); currentAddress++;
				}
			}
		}

		currentLineListNode = currentLineListNode->NextPtr;
	}

	currentLineListNode = dataLineList->FirstPtr;
	while (currentLineListNode != NULL) // Encode the data section in the file.
	{
		assemblyLine = currentLineListNode->DataPtr;

		if (assemblyLine->Command != NULL)
		{
			if (strcmp(assemblyLine->Command, ".string") == 0) // Check if the data is a string.
			{
				char** prmArray = assemblyLine->PrmArray;
				for (int i = 0; i < assemblyLine->numOfPrm; i++)
				{
					char* currentString = prmArray[i];
					for (unsigned int j = 0; j < strlen(currentString); j++) // Encode the ascii of the characters in the string.
					{
						int asciiCode = (int)currentString[j];
						InsertEncodedList(currentAddress, asciiCode, encodedLineList); currentAddress++; // Add to the list
					}
					InsertEncodedList(currentAddress, 0, encodedLineList); currentAddress++; // Encode null value and add to the list.
				}
			}
			else if (strcmp(assemblyLine->Command, ".data") == 0) // Check if the data is a number.
			{
				int numberEncode = 0;
				char** prmArray = assemblyLine->PrmArray;
				for (int i = 0; i < assemblyLine->numOfPrm; i++) // Encode all the numbers.
				{
					int currentNum = atoi(prmArray[i]);
					if (currentNum < 0)  numberEncode = ((unsigned)(numberEncode | ((~abs(currentNum) + 1) & TENBITS)));
					else numberEncode = currentNum;
					InsertEncodedList(currentAddress, numberEncode, encodedLineList); currentAddress++; // Add to the list.
				}
			}
			else if (strcmp(assemblyLine->Command, ".struct") == 0) // Check if the data is a struct.
			{
				int numberEncode = 0;
				char** prmArray = assemblyLine->PrmArray;
				int currentNum = atoi(prmArray[0]);
				if (currentNum < 0) numberEncode = ((unsigned)(numberEncode | ((~abs(currentNum) + 1) & TENBITS))); // Encode the number if negitive.
				else numberEncode = currentNum; 
				InsertEncodedList(currentAddress, numberEncode, encodedLineList); currentAddress++; // Add to the list.

				char* currentString = prmArray[1];
				for (unsigned int i = 0; i < strlen(currentString); i++) // Encode all the chracters in the string.
				{
					int asciiCode = (int)currentString[i];
					InsertEncodedList(currentAddress, asciiCode, encodedLineList); currentAddress++; // Add to the list.
				}
				InsertEncodedList(currentAddress, 0, encodedLineList); currentAddress++; // Add null value to the list.
			}
		}

		currentLineListNode = currentLineListNode->NextPtr;
	}

	return ErrorsExist; // Return if the second scan found errors.
}

/*Converts a string with its enum value*/
int convertStingToEnum(const char* command)
{
	for (int j = 0; j < sizeof(OpcodeConversion) / sizeof(OpcodeConversion[0]); ++j) // Find the string in the array of the opcodes.
	{
		if (!strcmp(command, OpcodeConversion[j].str))
			return OpcodeConversion[j].val; // Return the enum value.
	}
	return -1; // If not found return -1.
}

/*Gets a command and check how many operands it needs*/
int CheckOpernadNumber(const char* command)
{
	for (int j = 0; j < sizeof(TwoOperandConversion) / sizeof(TwoOperandConversion[0]); ++j) // Check if the command is in the array of two operands
	{
		if (!strcmp(command, TwoOperandConversion[j].str))
			return 2; // If so return 2.
	}
	for (int j = 0; j < sizeof(OneOperandConversion) / sizeof(OneOperandConversion[0]); ++j) // Check if the command is in the array of one operand
	{
		if (!strcmp(command, OneOperandConversion[j].str))
			return 1; // If so return 1.
	}
	for (int j = 0; j < sizeof(NoOperandConversion) / sizeof(NoOperandConversion[0]); ++j) // Check if the command is in the array of zero operands
	{
		if (!strcmp(command, NoOperandConversion[j].str))
			return 0; // If so return 0;
	}
	return -1; // If not found return -1.
}

/*Gets a decimal number and encodeds it to base 32*/
char* EncodeBase32(int number)
{
	char numberBase32[] = "!@#$%^&*<>abcdefghijklmnopqrstuv"; // All the numbers in base 32.
	char* base32 = calloc(BASE32LENGTH + 1, sizeof(char));

	for (int i = BASE32LENGTH - 1; i >= 0 && base32 != NULL; i--)
	{
		base32[i] = numberBase32[number & 0x1F]; // Encode the first 5 bytes of the number.
		number = number >> 5; // Shift the bytes to the right.
	}
	return base32; // Return the encoded number.
}

/*Gets a value and adds it to the Encoded Lines List*/
void InsertEncodedList(int currentAddress, int BinCode, struct List* encodedLineList)
{
	struct EncodedLine* encodedLine = malloc(sizeof(struct EncodedLine));
	if (encodedLine != NULL)
	{
		encodedLine->LineAddress = EncodeBase32(currentAddress); // Encodes the address.
		encodedLine->BinCode = EncodeBase32(BinCode); // Encodes the bincode.
	}
	List_AddNode(encodedLineList, encodedLine, &EncodedLineDelete); // Adds to the list.
}


