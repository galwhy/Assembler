#pragma once

struct AssemblyLine
{
	char* Label;		// This is the line lable 
	char* Command;	// 
	char** PrmArray;
	int		numOfPrm;
	int		LineNumber;
};

struct Label	//Free members when deliting
{
	char* LabelPtr;
	int		BlockAdress;
};

struct ExternalLabel
{
	char* LabelPtr;
	struct List* AdressList;
};

struct EntryLabel
{
	char* LabelPtr;
	char* Address;
};
void AddressDelete(struct Node* node);

void ExternalLabelDelete(struct Node* node);

void EntryLabelDelete(struct Node* node);

void AssemblyLineDelete(struct Node* node);

void LabelClear(struct Node* node);

bool checkIfLabelExists(struct List* labelList, char* label);

bool CreateAssemblyLineList(char* fileContent, struct List* entries, struct List* externals, struct List* labelList, struct List* commandLineList, struct List* dataLineList);

int CheckForLabel(struct List* labelList, struct AssemblyLine* assemblyLine, int currentAddress, struct List* entries, struct List* externals);

