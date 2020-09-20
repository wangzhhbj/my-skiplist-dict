
#ifndef _DICT_HASH_H
#define _DICT_HASH_H

#include <string.h>
#include <unistd.h>

/*-------------------------------------------------------------------------*/
/** oneByOneHash
  @brief    Compute the hash key for a string.
  @param    key     Character string to use for key.
  @return   1 unsigned int on at least 32 bits.

  This hash function has been taken from an Article in Dr Dobbs Journal.
  This is normally a collision-free function, distributing keys evenly.
  The key is stored anyway in the struct so that collision can be avoided
  by comparing the key itself in last resort.
 */
/*--------------------------------------------------------------------------*/
unsigned str_one_by_one_hash(const char * key)
{
    size_t      len ;
    unsigned    hash ;
    size_t      i ;

    if (!key)
	{
        return 0 ;
	}

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++)
	{
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
	
    return hash ;
}



#endif

