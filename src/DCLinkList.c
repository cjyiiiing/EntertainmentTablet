#include "DCLinkList.h"

/**
 * @brief 创建链表节点
 *
 * @param data 节点数据
 * @return DCLinkList *节点
*/
DCLinkList *CreateNode(datatype data) {
        DCLinkList *node = (DCLinkList *)malloc(sizeof(DCLinkList));
        node->prev = node->next = node;
        node->data = (datatype *)malloc(sizeof(datatype));
        memcpy(node->data, &data, sizeof(datatype));
        return node;
}

/**
 * @brief 尾插
 *
 * @param head  目标链表
 * @param newNode       要插入的节点
 * @return true表示成功，false表示失败
*/
bool InsertAtTail(DCLinkList **head, DCLinkList *newNode) {
        if (newNode == NULL) {
                return false;
        }

        if (*head == NULL) {
                *head = newNode;
                return true;
        }

        newNode->prev = (*head)->prev;
        newNode->next = (*head);
        (*head)->prev->next = newNode;
        (*head)->prev = newNode;

        return true;
}

/**
 * @brief 在链表中搜索节点
 *
 * @param head 链表的头节点
 * @param data 要搜索的节点的数据
 * @param cmp 比较函数
 * @return 返回搜索到的节点
*/
DCLinkList *searchNode(DCLinkList *head, datatype data, bool (*cmp)(datatype, datatype)) {
        if (head == NULL) {
                return NULL;
        }
        DCLinkList *pos = head;
        while (pos != head->prev) {
                if (cmp(*(pos->data), data)) {
                        return pos;
                }
                pos = pos->next;
        }
        if (cmp(*(pos->data), data)) {          /* 检查head->prev是否是要找的节点 */
                return pos;
        }
        return NULL;
}

/**
 * @brief 释放单个节点
 *
 * @param node 要释放的节点
 * @return void
 * @note 注意：传入的参数是双重指针，这样才能正确使指针置NULL
*/
void destroyNode(DCLinkList **node) {
        (*node)->prev->next = (*node)->next;    /* 注意要加括号，因为->优先级高于* */
        (*node)->next->prev = (*node)->prev;
        free((*node)->data);       /* 注意要先释放其中的data */
        free(*node);
        *node = NULL;   /* 注意要置NULL，否则会成为野指针 */
}

/**
 * @brief 释放链表
 *
 * @param head 要释放的链表
 * @return void
 * @note 注意：传入的参数是双重指针，这样才能正确使指针置NULL
*/
void destroyList(DCLinkList **head) {
        DCLinkList *tmp = NULL;
        while (*head != NULL) {
                tmp = (*head)->next;
                if (tmp != *head) {
                        destroyNode(&tmp);
                }
                else {
                        free((*head)->data);
                        free(*head);
                        *head = NULL;
                }
        }
}