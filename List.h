#pragma once


struct Node
{
	void* DataPtr;
	struct	Node* NextPtr;

	void (*ClearPtr)(struct Node* nodePtr);
};

struct List
{
	struct	Node* FirstPtr;
	struct	Node* LastPtr;
};


struct List* List_New();

void List_Delete(struct List* ListPtr);

void List_AddNode(struct List
	* ListPtr, void* DataPtr, void (*ClearPtr)(struct Node* nodePtr));
