#include <list.h>
#include <kernel/interrupt.h>
#include <string.h>

void listInit(struct list *list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

void listInsert(struct listNode *before, struct listNode *node) {
    enum intrStatus oldStatus = intrDisable();

    before->prev->next = node;
    node->prev = before->prev;
    node->next = before;
    before->prev = node;

    intrSetStatus(oldStatus);
}

// listPush inserts the node into the head of the list.
void listPush(struct list *list, struct listNode *node) {
    listInsert(list->head.next, node);
}

// listAppend inserts the node into the tail of the list.
void listAppend(struct list *list, struct listNode *node) {
    listInsert(&list->tail, node);
}


// listRemove removes the node from the list where the node exists.
void listRemove(struct listNode *node) {
    enum intrStatus oldStatus = intrDisable();

    node->prev->next = node->next;
    node->next->prev = node->prev;

    intrSetStatus(oldStatus);
}

// listPop removes the first node of the list.
// return address of removed node if not empty;
// return NULL if empty;
struct listNode *listPop(struct list *list) {
    if (listEmpty(list)) { return NULL; }

    struct listNode *node = list->head.next;
    listRemove(node);

    return node;
}

// listExist checks if the node exists in the list.
bool listExist(struct list *list, struct listNode *node) {
    struct listNode *cur = list->head.next;
    while (cur != &list->tail) {
        if (cur == node) { return true; }
        cur = cur->next;
    }

    return false;
}

// listTraversal traverses entire list to find a node satisfying func.
// return NULL if no one.
struct listNode *listTraversal(struct list *list, bool (*func)(struct listNode *, int), int arg) {
    struct listNode *cur = list->head.next;
    while (cur != &list->tail) {
        if (func(cur, arg)) { return cur; }
        cur = cur->next;
    }

    return NULL;
}

// listLen calculates the length of the list.
uint32 listLen(struct list *list) {
    uint32 len = 0;
    struct listNode *cur = list->head.next;
    while (cur != &list->tail) {
        len += 1;
        cur = cur->next;
    }

    return len;
}

// listEmpty checks if the list is empty.
bool listEmpty(struct list *list) {
    return list->head.next == &list->tail;
}
