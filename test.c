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
unsigned hash_test(const void * key)
{
	return strlen((const char * )key);
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




int main()
{
	int ret = 0;
	unsigned i = 0;
	unsigned len = 0;
	unsigned num = 0;
	char *tmp = NULL;
	char str[128] = {0};
	dictEntry * entry = NULL;
	dict * dt = NULL;
	
#ifdef USE_TCMALLOC
	HeapProfilerStart("./mem_heap");
#endif
	
	dt = dict_new(&callbackDict, NULL);
	
	
	for(i = 0; i<1000000; i++)
	{
		num = i;
		len = snprintf(NULL, 0, "%u", num) + 1;
		tmp = malloc(len);
		if(NULL == tmp)
		{
			break;
		}
		
		len = snprintf(tmp, len, "%u", num);
		dict_add(dt, tmp, tmp);
		//printf("dict_add %s---%s\n", tmp, tmp);
	}
	
	printf("dict count:%u\n", dt->count);
	printf("dict sl level:%u\n", dt->sl->level);
	
	//dict_print_str_str(dt);

	/*
	entry = dict_find(dt, tmp);
	if(entry != NULL)
	{
		printf("find successed: (%s:%s)\n", (char *)entry->key, (char *)entry->v.val);
	}
	else
	{
		printf("not find\n");
	}
	
	/*
	snprintf(str, sizeof(str), "%u", num);
	ret = dict_delete(dt, str);
	printf("delete %s---%d\n", str, ret);
	*/
	
	for(i = 0; i<1000000; i++)
	{
		num = i;
		memset(str, 0, sizeof(str));
		snprintf(str, sizeof(str), "%u", num);
		
		entry = dict_find(dt, str);
		if(NULL == entry)
		{
			printf("not find: (%s)\n", str);
		}
	}
	/**/
	
	//dict_print_str_str(dt);
	/*
	entry = dict_find(dt, "77235");
	if(entry != NULL)
	{
		printf("find successed: (%s:%s)\n", (char *)entry->key, (char *)entry->v.val);
	}
	else
	{
		printf("not find\n");
	}
	*/
	
	dict_free(dt);
	
#ifdef USE_TCMALLOC
	HeapProfilerDump("./mem_heap");
	HeapProfilerStop();
#endif
	
	return 0;
}




