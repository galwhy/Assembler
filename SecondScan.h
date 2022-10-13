#pragma once
#include "List.h"

typedef enum { mov, cmp, add, sub, not, clr, lea, inc, dec, jmp, bne, get, prn, jsr, rts, hlt } OpCodeCommands;

const static struct CommandOpCode {
	OpCodeCommands      val;
	const char* str;
} OpcodeConversion[] = {
	{mov, "mov"},
	{cmp, "cmd"},
	{add, "add"},
	{sub, "sub"},
	{not, "not"},
	{clr, "clr"},
	{lea, "lea"},
	{inc, "inc"},
	{dec, "dec"},
	{jmp, "jmp"},
	{bne, "bne"},
	{get, "get"},
	{prn, "prn"},
	{jsr, "jsr"},
	{rts, "rts"},
	{hlt, "hlt"}
};

const static struct TwoOperandCommands {
	OpCodeCommands      val;
	const char* str;
} TwoOperandConversion[] = {
	{mov, "mov"},
	{cmp, "cmd"},
	{add, "add"},
	{sub, "sub"},
	{lea, "lea"}
};

const static struct OneOpernadCommand {
	OpCodeCommands      val;
	const char* str;
} OneOperandConversion[] = {
	{not, "not"},
	{clr, "clr"},
	{inc, "inc"},
	{dec, "dec"},
	{jmp, "jmp"},
	{bne, "bne"},
	{get, "get"},
	{prn, "prn"},
	{jsr, "jsr"}
};

const static struct NoOperandCommand {
	OpCodeCommands      val;
	const char* str;
} NoOperandConversion[] = {
	{rts, "rts"},
	{hlt, "hlt"}
};

struct EncodedLine
{
	char* 					LineAddress;
	char* 					BinCode;
};

void EncodedLineDelete(struct Node* node);

void CreateOutputFiles(char* name, char* amContent, struct List* encodedLineList, struct List* externals, struct List* entries);

bool EncodeLineList(struct List* commandLineList, struct List* dataLineList, struct List* labelList, struct List* externals, struct List* encodedLineList);

int convertStingToEnum(const char* str);

int CheckOpernadNumber(const char* str);

char* EncodeBase32(int number);

void InsertEncodedList(int currentAddress, int BinCode, struct List* encodedLineList);