#include "link_list.h"

#ifndef MAX_CACHE_SIZE
#define MAX_CACHE_SIZE 1049000
#endif

#ifndef MAX_OBJECT_SIZE
#define MAX_OBJECT_SIZE 102400
#endif


// BKDR Hash Function
extern unsigned int BKDRHash(char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    
    while (*str)
    {
        hash = hash * seed + (*str++);
    }
    
    return (hash & 0x7FFFFFFF);
}


extern void init_link_list(struct link_list* list)
{
    typedef struct link_node node;
    node* p = (node*)malloc(sizeof(node));
    list->head = p;
    list->tail = p;
    p->pred = NULL;
    p->succ = NULL;
    p->size = 0;
    p->data = NULL;
    p->label = -1;//0xffffffff
    list->total_size = 0;
}

extern struct link_node* update_cache(struct link_list*list,char* uri,const char* data,size_t size)
{
    if(size>MAX_OBJECT_SIZE)
    {
        #ifdef DEBUG_LINK_LIST
            printf("object of %lu out of %u\n",size,MAX_OBJECT_SIZE);
        #endif
        return NULL;
    }
    typedef struct link_node node;
    node* now = list->tail;
    size_t total_size = list->total_size;
    while (total_size+size>MAX_CACHE_SIZE)
    {
        total_size-= now->size;
        node*temp = now;
        now = now->pred;
        #ifdef DEBUG_LINK_LIST
            printf("delete item(data=\"%s\",len=%lu,hash=%u)\n",temp->data,temp->size,temp->label);
        #endif
        free(temp);
    }

    node* p = (node*)malloc(sizeof(node));
    now->succ = p;
    list->tail = p;
    p->pred = now;
    p->succ = NULL;
    p->size = size;
    p->data = (char*)malloc(sizeof(char)*size);
    p->label = BKDRHash(uri);
    strcpy(p->data,data);

    total_size += size;
    list->total_size = total_size;
    #ifdef DEBUG_LINK_LIST
        printf("cache used = %d;cache remain= %d\n",(int)list->total_size,(int)MAX_CACHE_SIZE - (int)list->total_size);
    #endif
    return p;
}
extern struct link_node* get_cache(struct link_list*list,char* uri)
{
    unsigned label = BKDRHash(uri);
    typedef struct link_node node;
    node* head = list->head;
    node* now = list->head->succ;
    while(now!=NULL)
    {
        if (now->label==label)
        {
            if(now==list->head||(now==list->tail && list->tail==list->head->succ))
            {
                return now;
            }
            else if (now==list->tail)
            {
                    list->tail = now->pred;
                    now->pred->succ =NULL;
                    now->pred = head;
                    now->succ = head->succ;
                    head->succ->pred = now;
                    head->succ = now;
            }
            else{
                now->pred->succ = now->succ;
                now->succ->pred = now->pred;
                now->pred = head;
                now->succ = head->succ;
                head->succ->pred = now;
                head->succ = now;
            }
            return now;
        }
        now = now->succ;
    }
    return NULL;
}
extern void free_list(struct link_list*list)
{
    typedef struct link_node node;
    node* now = list->head;
    while(now->succ)
    {
        node* temp = now;
        now = now->succ;
        free(temp->data);
        free(temp); 
    }
    return;
}
extern void show_list(struct link_list*list)
{
    typedef struct link_node node;
    node* now = list->head;
    printf("\n");
    printf("cache used = %d;cache remain= %d\n",(int)list->total_size,(int)MAX_CACHE_SIZE - (int)list->total_size);
    while(now->succ)
    {
        now = now->succ;
        printf("- item(data=\"%s\",hash=%u,size=%lu)\n",now->data,now->label,now->size);
    }
    printf("\n");
    fflush(stdout);
    return;
}
