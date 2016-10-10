#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct node_t{
    int data;
    struct node_t *next;
}node_t;

#define LIST_CNT 10;

void printf_node(struct node_t **node_list)
{
    struct node_t *node_tmp = NULL;
    struct node_t *node_list_1 = *node_list;
    int size = LIST_CNT;
    for(int i = 0; i < size; i++){
        node_tmp = *node_list;
        printf("node tmp: %d\n", node_tmp->data);
        *node_list = (*node_list)->next;
    }
}

int main()
{
    struct node_t *node_head = (struct node_t*)malloc(sizeof(struct node_t));
    node_head->data = 0;

    struct node_t *node_s = node_head;
    struct node_t *node_tmp = NULL;
    int size = LIST_CNT;
    for(int i = 1; i < size; i++){
        node_tmp = (struct node_t*)malloc(sizeof(struct node_t));
        node_tmp->data = i;
        node_s->next = node_tmp;
        node_s = node_s->next;
    }
    printf(" node_head: %p\n", node_head);
    printf_node(&node_head);
    printf(" node_head: %p\n", node_head);
    return 0;
}
