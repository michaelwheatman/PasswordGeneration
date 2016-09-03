#include "chash.h"
#include <cstring>
/*
#include <stdafx.h>
*/

/*
**  Hash routines
**    
**     Assume item to hash is null-terminated string at start of structure
**
*/


 
CHashTable::CHashTable(unsigned uTableSize, unsigned uSizeOfData)
{
    unsigned      i;

 
    m_uBuckets = uTableSize;
    m_uDatumSize = uSizeOfData;
    m_uCount = 0;
    m_ppChain = new CHashTableElement * [uTableSize];

	for ( i = 0; i < uTableSize; ++i )
    {
        m_ppChain[i] = NULL;
    }
}
 

CHashTable::~CHashTable()
{
    unsigned			x;
    CHashTableElement	*p, *q;
 
 
    for ( x = 0; x < m_uBuckets; ++x )
    {
		for ( p = m_ppChain[x]; p != NULL; p = q )
		{
			q = p->m_pNext;
			delete p;
		}
    }

	delete [] m_ppChain;
 
    return;
}


CHashTableElement::CHashTableElement(const void *pCopyIt, unsigned uSize)
{
#ifdef HASH_SUPPORTS_DELETE
	m_pPrev = NULL;
#endif
	m_pNext = NULL;
	m_pData = new char [uSize];
	memcpy(m_pData, pCopyIt, uSize);
}


 
CHashTableElement::~CHashTableElement()
{
	delete [] m_pData;
}



unsigned CHashTable::Hash(const char *szSearchFor)
{
    unsigned      uHashValue = 2166136261u;
    unsigned      uPrimeFNV = (1 << 24) + (1 << 8) + 0x93;


    while ( *szSearchFor )
    {
        uHashValue ^= *szSearchFor++;
        uHashValue *= uPrimeFNV;
    }
 
    return uHashValue % m_uBuckets;
}
 
 
void * CHashTable::Lookup(const char *szSearchFor)
{
    unsigned             xBucket = Hash(szSearchFor);
    CHashTableElement    *pEl;
 
 
    for ( pEl = m_ppChain[xBucket]; pEl != NULL && strcmp(szSearchFor, (char *)pEl->m_pData) != 0; pEl = pEl->m_pNext )
    {
    }
 
    return *szSearchFor && pEl != NULL ? pEl->m_pData : NULL;
}


void * CHashTable::Insert(const void *pItem)
{
    void                 *p = Lookup((char *)pItem);
    unsigned             xBucket;
    CHashTableElement    *pEl;
 
      
    if ( p != NULL ) return p;
      
    pEl = new CHashTableElement(pItem, m_uDatumSize);
      
    xBucket = Hash((char *)pItem);
    pEl->m_pNext = m_ppChain[xBucket];
#ifdef HASH_SUPPORTS_DELETE
    if ( pEl->m_pNext != NULL ) pEl->m_pNext->m_pPrev = pEl;
    pEl->m_pPrev = NULL;
#endif
	m_ppChain[xBucket] = pEl;

	++m_uCount;
 
    return pEl->m_pData;
}



int CHashTable::Delete(const char *szSearchFor)
{
    unsigned             xBucket = Hash(szSearchFor);
    CHashTableElement    *pEl;
 
 
    for ( pEl = m_ppChain[xBucket]; pEl != NULL && strcmp(szSearchFor, (char *)pEl->m_pData) != 0; pEl = pEl->m_pNext )
    {
    }

	if ( pEl != NULL )
	{
		if ( pEl->m_pNext != NULL ) 
			pEl->m_pNext->m_pPrev = pEl->m_pPrev;
		   
		if ( pEl->m_pPrev != NULL ) 
			pEl->m_pPrev->m_pNext = pEl->m_pNext;
		else
			m_ppChain[xBucket] = pEl->m_pNext;

		delete pEl;

		return 1;
	}

	return 0;
}
