#include <stdio.h>

#include <malloc.h>

#include <string.h>

#include "hash.h"


/*

**  Hash routines

**    

**     Assume item to hash is null-terminated string at start of structure

**

*/








 


HashTable     *CreateHashTable(unsigned uTableSize, unsigned uSizeOfData)

{

       unsigned      i;

       HashTable     *pTable = (HashTable *)malloc(sizeof(HashTable));


       if ( pTable != NULL )

       {

              pTable->nBuckets = uTableSize;

              pTable->uDatumSize = uSizeOfData;

              pTable->uCount = 0;

              pTable->ppChain = (HashTable_Element **)malloc(uTableSize * sizeof(HashTable_Element *));

              if ( pTable->ppChain == NULL )

              {

                     free(pTable);

                     pTable = NULL;

              }


              else

              {
                    for ( i = 0; i < uTableSize; ++i )

                     {

                           pTable->ppChain[i] = NULL;

                     }

              }

       }


       return pTable;

}



static unsigned      Hash(HashTable *pTable, char *szSearchFor)

{

       unsigned      uHashValue = 2166136261u;

       unsigned      uPrimeFNV = (1 << 24) + (1 << 8) + 0x93;




       while ( *szSearchFor )

       {
              uHashValue ^= *szSearchFor++;

              uHashValue *= uPrimeFNV;

       }


       return uHashValue % pTable->nBuckets;

}


 

void   *LookupHashTable(HashTable *pTable, char *szSearchFor)

{

       unsigned                   xBucket = Hash(pTable, szSearchFor);

       HashTable_Element    *pEl;


       for ( pEl = pTable->ppChain[xBucket]; pEl != NULL && strcmp(szSearchFor, (char *)pEl->pData) != 0; pEl = pEl->pNext )

       {


       }

       return pEl != NULL ? pEl->pData : NULL;


}


 


 


void   *InsertHashTable(HashTable *pTable, void *pItem)


{

       void                 *p = LookupHashTable(pTable, (char *)pItem);

       unsigned             xBucket;

       HashTable_Element    *pEl;    


       if ( p != NULL ) return p;


       pEl = (HashTable_Element *)malloc(sizeof(HashTable_Element));


       if ( pEl != NULL )

       {

             pEl->pData = malloc(pTable->uDatumSize);

              if ( pEl->pData == NULL )

              {

                    free((void *)pEl);

                     pEl = NULL;

              }

              else

              {
                     memcpy(pEl->pData, pItem, pTable->uDatumSize);

                     xBucket = Hash(pTable, (char *)pItem);

                     pEl->pNext = pTable->ppChain[xBucket];


#ifdef HASH_SUPPORTS_DELETE


                     if ( pEl->pNext != NULL ) pEl->pNext->pPrev = pEl;

                     pEl->pPrev = NULL;

#endif

                     pTable->ppChain[xBucket] = pEl;

                     ++pTable->uCount;

              }

       }

       return pEl->pData;

}




void   DestroyHashTable(HashTable *pTable)

{

       unsigned				x;

       HashTable_Element    *p, *q;

       for ( x = 0; x < pTable->nBuckets; ++x )

       {

              for ( p = pTable->ppChain[x]; p != NULL; p = q )

              {

                     free(p->pData);

                     q = p->pNext;

                     free((void *)p);

              }


       }

       free((void *)pTable);

       return;

}


 


 


 


double HashEfficiency(HashTable *pTable)

{

       int				nHit = 0;

       unsigned			i;

	   HashTable_Element *p;

       for ( i = 0; i < pTable->nBuckets; ++i )

       {

              nHit += (pTable->ppChain[i] != NULL);

              for ( p = pTable->ppChain[i]; p != NULL; p = p->pNext )

              {

              }


       }

       return (double)nHit / (double)(pTable->nBuckets < pTable->uCount ? pTable->nBuckets : pTable->uCount);

}








#ifdef HASH_SUPPORTS_DELETE





int DeleteHashElement(HashTable *pTable, char *szSearchFor)

{

       unsigned             xBucket = Hash(pTable, szSearchFor);

       HashTable_Element    *pEl;

       for ( pEl = pTable->ppChain[xBucket]; pEl != NULL && strcmp(szSearchFor, (char *)pEl->pData) != 0; pEl = pEl->pNext )

       {


       }


	   if ( pEl != NULL )

	   {

		   free(pEl->pData);

		   if ( pEl->pNext != NULL ) 

			   pEl->pNext->pPrev = pEl->pPrev;


		   if ( pEl->pPrev != NULL ) 

			   pEl->pPrev->pNext = pEl->pNext;

		   else

			   pTable->ppChain[xBucket] = pEl->pNext;

		   return 1;

	   }


	   return 0;


}



#endif
