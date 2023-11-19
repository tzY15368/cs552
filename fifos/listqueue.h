#define LISTQUEUE_H 1

#ifndef MALLOC_H
#include "malloc.h"
#endif

typedef struct list_node {
    list_node_t* next;
    void* data;
} list_node_t;

typedef struct queue {
    list_node_t* head;
    int size;
} listqueue_t;

listqueue_t* listqueue_init(){
    listqueue_t* q = (listqueue_t*) malloc(sizeof(listqueue_t));
    q->head = NULL;
    q->size = 0;
    return q;
}

void listqueue_put(listqueue_t* q, void* data){
    list_node_t* node = (list_node_t*) malloc(sizeof(list_node_t));
    node->data = data;
    node->next = NULL;
    if (q->size == 0){
        q->head = node;
    } else {
        list_node_t* tail = q->head;
        while (tail->next != NULL){
            tail = tail->next;
        }
        tail->next = node;
    }
    q->size++;
}

void* listqueue_get(listqueue_t* q){
    if (q->size == 0){
        return NULL;
    }
    list_node_t* head = q->head;
    q->head = head->next;
    q->size--;
    return head->data;

}