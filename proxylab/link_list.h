

#ifndef MY_LINK_LIST_H
#define MY_LINK_LIST_H
#include<stddef.h>
#include <stdlib.h>
#include<string.h>
#include<stdio.h>
unsigned int BKDRHash(char *str);

struct link_node{
    struct link_node* pred,*succ;
    char* data;
    size_t size;
    unsigned label;
};
struct link_list{
    struct link_node* head,*tail;
    size_t total_size;
};
void init_link_list(struct link_list* list);
struct link_node* update_cache(struct link_list*list,char* uri,const char* data,size_t size);
void free_list(struct link_list*list);
struct link_node* get_cache(struct link_list*list,char* uri);
// struct link_node*
#endif
