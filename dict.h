#ifndef _MYDICT_H_
#define _MYDICT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

/*******************************/


#define MAX_LEVEL 32 // MAX level, 32 is big enough  

#define DICT_OK 0
#define DICT_ERR 1
#define NO_ENTRY 2

/**********************************/

// hashKey
typedef uint64_t hashKeyType;


struct dictEntry;
struct skipNode;
struct skipList;

// 用于存放字典列表头的节点
typedef struct skipNode
{
    uint32_t high;              // node height
    uint32_t count;
    hashKeyType hashKey;        // hash key
    struct dictEntry * pEntry;  // pEntry pointer
    struct skipNode * next[0];  // 后继指针数组，柔性数组 可实现结构体的变长
} skipNode;


//跳表结构
typedef struct skipList
{
    int max_lev; /* max level */
    
    int level;// 当前使用的最大层数
    
    skipNode * head; /* 弄一个头节点 */
    
    /* node 数量 */
    uint32_t count;
    
    /*
    全局缓存数组，不再额外申请内存了
    在搜寻指定Key的过程中，保存每一层level的最后一个节点的位置
    用在删除和插入节点时，对涉及到的节点的指针更新
    */
    skipNode * last[MAX_LEVEL];
} skipList;




/**
key 属性保存着键值对中的键， 而 v 属性则保存着键值对中的值， 
其中键值对的值可以是一个指针， 或者是一个 uint64_t 整数， 又或者是一个 int64_t 整数。
*/
typedef struct dictEntry {
    //键
    void * key;
    
    //值
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    
    // 指向下个哈希表节点，形成链表
    struct dictEntry *next;
    
} dictEntry;



typedef struct dictType {
    // 计算哈希值的函数
    uint64_t (*hashFunction)(const void *key);

    // 复制键的函数
    void *(*keyDup)(void *privdata, const void *key);
    
    // 设置值的函数
    void *(*valDup)(void *privdata, const void *obj);

    // 对比键的函数
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);

    // 销毁键的函数
    void (*keyDestructor)(void *privdata, void *key);
    
    // 销毁值的函数
    void (*valDestructor)(void *privdata, void *obj);
    
    // 打印键的函数，调试用
    //void (*keyPrint)(void *privdata, const void *key);
    
    // 打印值的函数，调试用
    //void (*valPrint)(void *privdata, const void *obj);

} dictType;



typedef struct dict {
    
    /* 用于支撑多态字典 */
    dictType *type;
    void *privdata;
    
    skipList * sl;
    
    uint32_t count;
} dict;


/****************************************************/



/* ------------------------------- Macros ------------------------------------*/
/* 销毁值 */
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)
/* 设置值 */
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)

#define dictSetSignedIntegerVal(entry, _val_) \
    do { (entry)->v.s64 = _val_; } while(0)

#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { (entry)->v.u64 = _val_; } while(0)

#define dictSetDoubleVal(entry, _val_) \
    do { (entry)->v.d = _val_; } while(0)

/* 销毁键 */
#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

/* 设置键 */
#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)


/* 对比键 */
#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))
/*
// 打印键 *
#define dictPrintKey(d, entry) do { \
    if ((d)->type->keyPrint) \
        (d)->type->keyPrint((d)->privdata, (entry)->key); \
    else \
        print("entry key:0x%p", (entry)->key); \
} while(0)


// 打印值 *
#define dictPrintKey(d, entry) do { \
    if ((d)->type->valPrint) \
        (d)->type->valPrint((d)->privdata, (entry)->v); \
    else \
        print("entry val:0x%p", (entry)->v); \
} while(0)
*/



#define dictHashKey(d, key) (d)->type->hashFunction(key)

#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)






//new_node生成一个Node结构体,同时生成包含n个Node *元素的数组
#define new_node(n) ((skipNode*)malloc(sizeof(skipNode) + n*sizeof(skipNode*)))

#define free_node(node) free(node);


/**
 * @brief create a skiplist node.
 *
 * @param  level  hashKey  entry
 *
 * @return skipNode address.
 */
skipNode *create_node(int level, hashKeyType hashKey, dictEntry * entry)
{
    skipNode *node = new_node(level);
    if(!node)
    {
        return NULL;
    }
    node->high = level;
    node->hashKey = hashKey;
    node->pEntry = entry;
    node->count = 0;
    return node;
}


/**
 * @brief create a skiplist.
 *
 * @param  void
 *
 * @return skipList address.
 */
skipList * create_skiplist(void)
{
    int i = 0;
    skipNode * node = NULL;
    skipList * sl = (skipList*)calloc(1, sizeof(skipList));//申请跳表结构内存
    if(NULL == sl)
    {
        return NULL;
    }

    sl->level = 1;// 设置跳表的层level，初始的层为1层（数组从0开始）
    sl->count = 0;
    
    //创建头结点 key 0 
    node = create_node(MAX_LEVEL, 0, NULL);
    if(NULL == node)
    {
        free(sl);
        return NULL;
    }
    sl->head = node;
    // 将header的next数组清空
    for(i=0; i<MAX_LEVEL; ++i)
    {
        node->next[i] = NULL;
    }

    srand(time(0));
    return sl;
}


/**
 * @brief random_level.
 *
 * @param  void
 *
 * @return level num, 1 2 3....
 */
static int random_level(void)
{
    int level = 1;
    const double p = 0.5;
    
    while ((random() & 0xffff) < 0xffff * p)
    {
            level++;
    }
    return level > MAX_LEVEL ? MAX_LEVEL : level;
}



/**
 * @brief find existing skipNode.
 *
 * @param  skipList hashKey
 *
 * @return always returns the hash skipNode of the specified key
 */
skipNode *skiplist_find(skipList *sl, hashKeyType hashKey)
{
    int i = 0;
    skipNode *beforeNode = sl->head;
    
    for (i = sl->level-1; i >= 0; i--)
    {
        while ((beforeNode->next[i] != NULL) && 
               (beforeNode->next[i]->hashKey < hashKey))
        {
            beforeNode = beforeNode->next[i];
        }
    }
    
    if((beforeNode->next[0] != NULL) && (beforeNode->next[0]->hashKey == hashKey))
    {
        return beforeNode->next[0];
    }
    else
    {
        return NULL;
    }
}





/**
 * @brief find or add a new skipNode.
 *
 * @param  void
 *
 * @return always returns the hash skipNode of the specified key
 */
skipNode *skiplist_find_or_insert(skipList *sl, hashKeyType hashKey)
{
    skipNode *node = NULL;
    skipNode *beforeNode = sl->head;
    int i = 0;
    
    for (i = sl->level-1; i >= 0; i--) {
        while ((beforeNode->next[i] != NULL) && 
               (beforeNode->next[i]->hashKey < hashKey))
        {
            beforeNode = beforeNode->next[i];
        }
        sl->last[i] = beforeNode;
    }
    node = beforeNode->next[0];
    /* 判断是否有该key的node */
    if((node != NULL) && (node->hashKey == hashKey))
    {
        return node;
    }
    
    /* create skiplist node failed */
    node = create_node(random_level(), hashKey, NULL);
    if(NULL == node)
    {
        return NULL;
    }
    
    /* update skiplist level */
    if (node->high > sl->level)
    {
        for(i=node->high; i>sl->level; i--)
        {
            sl->last[i-1] = sl->head;
        }
        sl->level = node->high;
    }
    
    /* 插入跳表 */
    // 逐层更新节点的指针,和普通链表插入一样
    for(i = 0; i < node->high; i++)
    {
        node->next[i] = sl->last[i]->next[i];
        sl->last[i]->next[i] = node;
    }
    
    /* node 数量增加 */
    sl->count++;
    
    return node;
}





/**
 * @brief delete the hash skipNode of the specified key
 *
 * @param  skipList hashKey
 *
 * @return 
 */
void skiplist_delete(skipList *sl, hashKeyType hashKey)
{
    skipNode *node = NULL;
    skipNode *beforeNode = sl->head;
    int i = 0;
    
    for (i = sl->level-1; i >= 0; i--) {
        while ((beforeNode->next[i] != NULL) && 
               (beforeNode->next[i]->hashKey < hashKey))
        {
            beforeNode = beforeNode->next[i];
        }
        sl->last[i] = beforeNode;
    }
    
    node = beforeNode->next[0];
    /* 判断是否有该key的node */
    if((node != NULL) && (node->hashKey != hashKey))
    {
        return;
    }
    
    //逐层删除
    for(i=sl->level-1; i>=0; --i)
    {
        if(sl->last[i]->next[i] == node) //删除节点
        {
            sl->last[i]->next[i] = node->next[i];
            //如果删除的是最高层的节点,则level--
            if(sl->head->next[i] == NULL)
            {
                sl->level--;
            }
        }
    }
    
    /* node数量减少 */
    sl->count--;
    
    free_node(node);
    
    return;
}




#define new_entry() ((dictEntry*)malloc(sizeof(dictEntry)))

/**
 * @brief add a hash dictEntry
 *
 * @param  dict key val
 *
 * @return dictEntry address
 */
dictEntry * dict_add(dict * d, void * key, void * val)
{
    skipNode * node = NULL;
    dictEntry * entry = NULL;
    dictEntry * * entryPP = NULL;
    hashKeyType hashKey = dictHashKey(d, key);
    
    /* 先从跳表找到索引头，再从链表中查询是否有该值 */
    node = skiplist_find_or_insert(d->sl, hashKey);
    if(NULL == node)
    {
        return NULL;
    }
    
    /* 从头找位置 */
    entryPP = &(node->pEntry);
    for( entry = node->pEntry; entry != NULL; entry = entry->next)
    {
        /* 找到了 */
        if( dictCompareKeys(d, entry->key, key) )
        {
            break;
        }
        entryPP = &(entry->next);
    }
    
    /* key entry 存在，修改值 */
    if(entry != NULL)
    {
        dictFreeVal(d, entry);
        dictSetVal(d, entry, val);
        return entry;
    }
    
    /* key entry 不存在 ， 新建 */
    entry = new_entry();
    if(NULL == entry)
    {
        return NULL;
    }
    
    entry->next = NULL;
    dictSetKey(d, entry, key);
    dictSetVal(d, entry, val);
    
    *entryPP = entry;
    
    /* 该node索引的entry增加 */
    node->count++;
    
    /* entry数量增加 */
    d->count++;
    
    return entry;
}





/**
 * @brief find an existing entry.
 *
 * @param  skipList hashKey
 *
 * @return always returns the hash entry of the specified key
 */
dictEntry * dict_find(dict * d, void * key)
{
    skipNode * node = NULL;
    dictEntry * entry = NULL;
    hashKeyType hashKey = dictHashKey(d, key);
    
    /* 先从跳表找到索引头，再从链表中查询是否有该值 */
    
    /* 找到索引头 */
    node = skiplist_find(d->sl, hashKey);
    if(NULL == node)
    {
        return NULL;
    }
    
    /* 从头找位置 */
    for( entry = node->pEntry; entry != NULL; entry = entry->next)
    {
        /* 找到了 */
        if( dictCompareKeys(d, entry->key, key))
        {
            break;
        }
    }
    
    return entry;
}



/**
 * @brief delete an existing entry.
 *
 * @param  dict key
 *
 * @return NO_ENTRY :  entry not exiting
 *         DICT_OK  :  delete successed
 */
int dict_delete(dict * d, void * key)
{
    int i = 0;
    dictEntry * entry = NULL;
    dictEntry * * entryPP = NULL;
    skipNode * beforeNode = d->sl->head;
    skipNode * node = NULL;
    hashKeyType hashKey = dictHashKey(d, key);
    
    for (i = d->sl->level-1; i >= 0; i--)
    {
        while ((beforeNode->next[i] != NULL) && 
               (beforeNode->next[i]->hashKey < hashKey))
        {
            beforeNode = beforeNode->next[i];
        }
        d->sl->last[i] = beforeNode;
    }
    
    if((beforeNode->next[0] != NULL) && (beforeNode->next[0]->hashKey == hashKey))
    {
        node = beforeNode->next[0];
    }
    else
    {
        return NO_ENTRY;
    }
    
    /* 从头找位置 */
    entryPP = &(node->pEntry);
    for( entry = node->pEntry; entry != NULL; entry = entry->next)
    {
        /* 找到了 */
        if( dictCompareKeys(d, entry->key, key) )
        {
            break;
        }
        entryPP = &(entry->next);
    }
    
    if(NULL == entry)
    {
        return NO_ENTRY;
    }
    
    /* delete entry */
    *entryPP = entry->next;
    node->count--;
    if(0 == node->count)
    {
        //逐层删除
        for(i=d->sl->level-1; i>=0; --i)
        {
            if(d->sl->last[i]->next[i] == node) //删除节点
            {
                d->sl->last[i]->next[i] = node->next[i];
                //如果删除的是最高层的节点,则level--
                if(d->sl->head->next[i] == NULL)
                {
                    d->sl->level--;
                }
            }
        }
        
        /* node数量减少 */
        d->sl->count--;
        free_node(node);
    }
    
    d->count--;
    
    dictFreeKey(d, entry);
    dictFreeVal(d, entry);
    free(entry);
    
    return DICT_OK;
}


/**
 * @brief Create a new hash dict.
 *
 * @param  dictType privDataPtr
 *
 * @return dict address
 */
dict *dict_new(dictType *type, void *privDataPtr)
{
    dict *d = malloc(sizeof(*d));

    d->type = type;
    d->privdata = privDataPtr;
    d->count = 0;
    
    d->sl = create_skiplist();
    if(NULL == d->sl)
    {
        free(d);
        d = NULL;
    }
    
    return d;
}



/**
 * @brief free a hash dict.
 *
 * @param  dict
 *
 * @return void
 */
void dict_free(dict *d)
{
    dictEntry * entry = NULL;
    dictEntry * entryNext = NULL;
    skipNode *node = NULL;
    skipNode *nodeNext = NULL;
    if(!d)
    {
        return;
    }
    
    if(!d->sl)
    {
        return;
    }
    
    node = d->sl->head;
    while(node)
    {
        nodeNext = node->next[0];
        
        entry = node->pEntry;
        while(entry != NULL)
        {
            entryNext = entry->next;
            dictFreeKey(d, entry);
            dictFreeVal(d, entry);
            free(entry);
            entry = entryNext;
        }
        
        free(node);
        node = nodeNext;
    }
    
    free(d->sl);
    
    free(d);
    
    return;
}



/**
 * @brief print a hash dict.
 *        entry->key & entry->v.val must be a string
 *
 * @param  dict
 *
 * @return void
 */
void dict_print_str_str(dict * d)
{
    skipNode * node = NULL;
    dictEntry * entry = NULL;
    int i;

    printf("total Level: %d, nums: %u\n", d->sl->level, d->count);
    for (i = 0; i < d->sl->level; i++)
    {
        node = d->sl->head->next[i];
        printf("Level[%d]:", i);

        while (node)
        {
            printf("%lu {\n", node->hashKey);
            
            entry = node->pEntry;
            while(entry != NULL)
            {
                printf("(%s : %s) \n", (char *)entry->key, (char *)entry->v.val);
                entry = entry->next;
            }
            
            printf("} -> ");
            
            node = node->next[i];
        }
        printf("\n\n");
    }
}



#endif



