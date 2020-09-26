#include "dict.h"

#include "hash.h"

#ifdef USE_TCMALLOC
#include <gperftools/heap-profiler.h>
#endif


static void *callbackValDup(void *privdata, const void *src) 
{
    //((void) privdata);
    int len = strlen((const char *)src);
    void *dup = malloc(len);
    memcpy(dup, src, len);
    return dup;
}


static int callbackKeyCompare(void *privdata, const void *key1, const void *key2) 
{
    int l1;
    int l2;
    //((void) privdata);

    return 0 == strcmp(key1, key2);

    /*
    l1 = strlen((const char *)key1);
    l2 = strlen((const char *)key2);
    if (l1 != l2) return 0;
    return memcmp(key1,key2,l1) == 0;
    */
}

static void callbackKeyDestructor(void *privdata, void *key) {
    ((void) privdata);
    free(key);
}

static void callbackValDestructor(void *privdata, void *val) {
    ((void) privdata);
    free(val);
}


/**/
static uint64_t hash_test(const void * key)
{
    return (uint64_t)strlen((const char * )key);
}


static dictType callbackDict = 
{
    str_one_by_one_hash,
    //hash_test,
    NULL,
    callbackValDup,
    callbackKeyCompare,
    callbackKeyDestructor,
    callbackValDestructor
};




int main(int argc, char *argv[])
{
    int ret = 0;
    unsigned i = 0;
    unsigned len = 0;
    unsigned key = 0;
    int num = 0;
    char *tmp = NULL;
    char str[128] = {0};
    dictEntry * entry = NULL;
    dict * dt = NULL;
    
    if(argc < 2)
    {
        printf("please input num, 1 ~ (int)\n");
        return -1;
    }
    
    num = atoi(argv[1]);
    if(num < 1)
    {
        printf("please input num, 1 ~ (int)\n");
        return -1;
    }
    
#ifdef USE_TCMALLOC
    HeapProfilerStart("./mem_heap");
#endif
    
    dt = dict_new(&callbackDict, NULL);
    printf("dict_new end\n");
    
    for(i = 0; i<num; i++)
    {
        key = i;
        len = snprintf(NULL, 0, "%u", key) + 1;
        tmp = malloc(len);
        if(NULL == tmp)
        {
            break;
        }
        
        len = snprintf(tmp, len, "%u", key);
        dict_add(dt, tmp, tmp);
    }
    
    printf("dict count:%u\n", dt->count);
    printf("dict sl level:%u\n", dt->sl->level);

    /*
    snprintf(str, sizeof(str), "%u", key);
    ret = dict_delete(dt, str);
    printf("delete %s---%d\n", str, ret);
    */
    
    for(i = 0; i<num; i++)
    {
        key = i;
        memset(str, 0, sizeof(str));
        snprintf(str, sizeof(str), "%u", key);
        
        entry = dict_find(dt, str);
        if(NULL == entry)
        {
            printf("not find: (%s)\n", str);
        }
    }
    
    //dict_print_str_str(dt);

    
    dict_free(dt);
    
#ifdef USE_TCMALLOC
    HeapProfilerDump("./mem_heap");
    HeapProfilerStop();
#endif
    
    return 0;
}




