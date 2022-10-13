#include "List.h"
#include <stdio.h>
#include <stdlib.h>

struct List* List_New()
{
	struct List* ListPtr = malloc(sizeof(struct List));

	if (ListPtr != NULL)
	{
		ListPtr->FirstPtr = NULL;
		ListPtr->LastPtr = NULL;
	}

	return ListPtr;
}

void List_Delete(struct List* ListPtr)
{
	struct Node* NodePtr = ListPtr->FirstPtr;

	while (NULL != NodePtr)
	{
		struct Node* NextNodePtr = NodePtr->NextPtr;

		if (NULL != NodePtr->ClearPtr) (NodePtr->ClearPtr)(NodePtr);

		free(NodePtr);

		NodePtr = NextNodePtr;
	}

	free(ListPtr);
}


void List_AddNode(struct List* ListPtr, void* DataPtr, void (*ClearPtr)(struct Node* nodePtr))
{
	struct Node* NewNodePtr = malloc(sizeof(struct Node));

	if (NewNodePtr != NULL)
	{
		NewNodePtr->DataPtr = DataPtr;
		NewNodePtr->NextPtr = NULL;
		NewNodePtr->ClearPtr = ClearPtr;
	}

	if (NULL == ListPtr->FirstPtr)
	{
		ListPtr->FirstPtr = NewNodePtr;
		ListPtr->LastPtr = NewNodePtr;
	}
	else
	{
		ListPtr->LastPtr->NextPtr = NewNodePtr;
		ListPtr->LastPtr = NewNodePtr;
	}
}

