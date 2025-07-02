#ifndef __INCLUDE_LIST_H
#define __INCLUDE_LIST_H

#include <stdint.h>

#define offset(structType, member) (int) (&((structType *) 0)->member)
#define elem2entry(structType, memberName, elemPtr) \
                  (structType *) ((int) elemPtr - offset(structType, memberName))

struct listNode {
    struct listNode *prev;
    struct listNode *next;
};

struct list {
    struct listNode head;
    struct listNode tail;
};

void listInit(struct list *list);
void listInsert(struct listNode *before, struct listNode *node);
void listPush(struct list *list, struct listNode *node);
void listIterate(struct list *list);
void listAppend(struct list *list, struct listNode *node);
void listRemove(struct listNode *node);
struct listNode *listPop(struct list *list);
bool listEmpty(struct list *list);
uint32 listLen(struct list *list);
struct listNode *listTraversal(struct list *list, bool (*func)(struct listNode *, int), int arg);
bool listExist(struct list *list, struct listNode *node);

#endif
