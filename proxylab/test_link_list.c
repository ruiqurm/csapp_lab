#include "link_list.h"
#define MAX_CACHE_SIZE 100
#define MAX_OBJECT_SIZE 30

int test_list_update()
{
    printf("--------------------\nTEST1\n");
    struct link_list list;
    init_link_list(&list);
    char buffer[150];
    strcpy(buffer,"aaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.baidu.com",buffer,strlen(buffer));

    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.baidu.com",buffer,strlen(buffer));
    
    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.baidu.com",buffer,strlen(buffer));
    
    strcpy(buffer,"aaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.baidu.com",buffer,strlen(buffer));
    
    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.baidu.com",buffer,strlen(buffer));
    
    free_list(&list);
    printf("----------------------------\n");
    return 0;
}
int test_list_get_cache()
{
    struct link_node* node;
    printf("--------------------\nTEST2\n");
    struct link_list list;
    init_link_list(&list);
    char buffer[150];
    strcpy(buffer,"aaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.a.com",buffer,strlen(buffer));

    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.b.com",buffer,strlen(buffer));
    
    show_list(&list);

    node =  get_cache(&list,"www.a.com");
    if (node!=NULL)
    {
        printf("Get item(data=\"%s\",len=%lu,hash=%u)\n",node->data,node->size,node->label);
    }
    else{
        printf("Missing item(hash=%u)\n",BKDRHash("www.a.com"));
    }
    show_list(&list);

    node =  get_cache(&list,"www.c.com");
    if (node!=NULL)
    {
        printf("Get item(data=\"%s\",len=%lu,hash=%u)\n",node->data,node->size,node->label);
    }
    else{
        printf("Missing item(hash=%u)\n",BKDRHash("www.c.com"));
    }

    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.c.com",buffer,strlen(buffer));

    strcpy(buffer,"aaaaaaaaaa");;
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.d.com",buffer,strlen(buffer));

    strcpy(buffer,"aaaaaaaaaa");;
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.e.com",buffer,strlen(buffer));

    strcpy(buffer,"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("add item(data=\"%s\",len=%lu,hash=%u)\n",buffer,strlen(buffer),BKDRHash(buffer));
    update_cache(&list,"www.f.com",buffer,strlen(buffer));

    show_list(&list);

    node =  get_cache(&list,"www.f.com");
    if (node!=NULL)
    {
        printf("Get item www.f.com (data=\"%s\",len=%lu,hash=%u)\n",node->data,node->size,node->label);
    }
    else{
        printf("Missing item(hash=%u)\n",BKDRHash("www.f.com"));
    }
    show_list(&list);

    show_list(&list);

    node =  get_cache(&list,"www.f.com");
    if (node!=NULL)
    {
        printf("Get item www.f.com (data=\"%s\",len=%lu,hash=%u)\n",node->data,node->size,node->label);
    }
    else{
        printf("Missing item(hash=%u)\n",BKDRHash("www.f.com"));
    }
    show_list(&list);

        show_list(&list);

    node =  get_cache(&list,"www.b.com");
    if (node!=NULL)
    {
        printf("Get item www.b.com (data=\"%s\",len=%lu,hash=%u)\n",node->data,node->size,node->label);
    }
    else{
        printf("Missing item(hash=%u)\n",BKDRHash("www.b.com"));
    }
    show_list(&list);

    free_list(&list);
    printf("----------------------------\n");
    return 0;
}
int main()
{
    test_list_update();
    test_list_get_cache();
    return 0;
}