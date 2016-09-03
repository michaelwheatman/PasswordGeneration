/*
**  Hash routines
**    
*/

#include <stddef.h>
#ifndef HASH_H_INCLUDED
#define HASH_H_INCLUDED

#define HASH_SUPPORTS_DELETE


class CHashTableElement
{
public:
	CHashTableElement(const void *pCopyIt, unsigned uSize);
	~CHashTableElement();

	CHashTableElement	*m_pPrev;
	CHashTableElement	*m_pNext;
	char				*m_pData;
};
 

class CHashTable
{
private:
	unsigned	Hash(const char *szSearchFor);

	unsigned	m_uBuckets;
	unsigned    m_uDatumSize;
	unsigned    m_uCount;

	CHashTableElement    **m_ppChain;

public:
	CHashTable(unsigned uTableSize, unsigned uSizeOfData);
	~CHashTable();

	void *Lookup(const char *szSearchFor);
	void *Insert(const void *pItem);
	int   Delete(const char *szSearchFor);
 
	unsigned TableSize() const { return m_uBuckets; }
	unsigned DatumSize() const { return m_uDatumSize; }
	unsigned Elements()	 const { return m_uCount; }
	
	CHashTableElement	*Bucket(unsigned u) const { return (u < TableSize()) ? m_ppChain[u] : NULL; }
};




#endif
