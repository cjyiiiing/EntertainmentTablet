#ifndef __DCLINKLIST_H__
#define __DCLINKLIST_H__

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct DCLinkListData
{
	int num;
	char bmpName[50];
} DCLinkListData;

typedef DCLinkListData datatype;

/* 存储所有相册图像的链表 */
typedef struct DCLinkList
{
        datatype *data;
        struct DCLinkList *prev;
        struct DCLinkList *next;
} DCLinkList;

DCLinkList *CreateNode(datatype data);
bool InsertAtTail(DCLinkList **head, DCLinkList *newNode);
DCLinkList *searchNode(DCLinkList *head, datatype data, bool (*cmp)(datatype, datatype));
void destroyNode(DCLinkList **node);
void destroyList(DCLinkList **head);

#endif // !__DCLINKLIST_H__