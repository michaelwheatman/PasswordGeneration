#include <stdafx.h>

#include "classPCFG.h"


//#define PRINT_DEBUG
//#define TRY_MANGLED_WORD


CPCFG::CPCFG(int nGrams)
{
	m_bAreProbabilities = false;

	m_nGuesses  = 0;
	m_nGuessed  = 0;
	m_nBillions = 0;
	m_nProcessor = 'a';

	m_dThresholdP = 1e-13;

	m_phtPasswords = NULL;
	//m_phtDictionary = NULL;
	
	m_nCaseMasks = 0;
	m_nStructures = 0;

	if ( nGrams )
	{
		m_pmcAlphaN  = new CMarkovChainAlpha(nGrams);
		m_pmcAlpha1  = new CMarkovChainAlpha(1);
		m_pmcDigits  = new CMarkovChainDigit;
		m_pmcSymbols = new CMarkovChainSymbol;

		m_phtCaseMask = new CHashTable(3, sizeof(CBaseStructure));	//65537
		m_phtStructures = new CHashTable(65537, sizeof(CBaseStructure));
	}
	else
	{
		m_pmcAlphaN = NULL;
		m_pmcAlpha1 = NULL;
		m_pmcDigits = NULL;
		m_pmcSymbols = NULL;

		m_phtStructures = NULL;
		m_phtCaseMask = NULL;
	}

	for ( int i = 0; i <= MAX_LENGTH; ++i) 
	{
		m_anCaseMaskLength[i] = 0;
		//m_anAlphaStringLengths[i] = 0;
		//m_anDigitStringLengths[i] = 0;
		//m_anSymbolStringLengths[i] = 0;
		//m_anMixedStringLengths[i] = 0;
	}
}



CPCFG::~CPCFG()
{
	if ( m_pmcAlphaN != NULL ) delete m_pmcAlphaN;
	if ( m_pmcAlpha1 != NULL ) delete m_pmcAlpha1;
	if ( m_pmcDigits != NULL ) delete m_pmcDigits;
	if ( m_pmcSymbols != NULL ) delete m_pmcSymbols;

	if ( m_phtCaseMask != NULL ) delete m_phtCaseMask;
	if ( m_phtPasswords != NULL ) delete m_phtPasswords;
	//if ( m_phtDictionary != NULL ) delete m_phtDictionary;
	if ( m_phtStructures != NULL ) delete m_phtStructures;
}



void CPCFG::ConvertCountsToProb(bool bSmoothProbabilities)
{
	m_bAreProbabilities = true;

	m_pmcAlphaN->ConvertCountsToProb(bSmoothProbabilities, false);
	m_pmcAlpha1->ConvertCountsToProb(bSmoothProbabilities, false);
	m_pmcDigits->ConvertCountsToProb(bSmoothProbabilities, false);
	m_pmcSymbols->ConvertCountsToProb(bSmoothProbabilities, false);
}



void CPCFG::AddCharacters(int nToAdd, int xOriginalState, char *szCharClass, int xPwd, char *szSyntax, CMarkovChain *pmc, double dProbSoFar)
{
    int           i, iChar, xPreState;
 
 
#ifdef PRINT_DEBUG
	printf("Pwd = %.*s\n", xPwd, m_szGuessing);
#endif

    if ( nToAdd <= 0 )
    {
        BuildPassword(xPwd, szSyntax, dProbSoFar);
    }
    else
    {
        xPreState = (xOriginalState % pmc->m_nReduceModulus) * pmc->m_nChars;
 
        for ( i = 0; i < pmc->m_nChars; ++i )
        {
            iChar = pmc->getTransitionProbabilityIndex(xOriginalState, i);
 
            if ( dProbSoFar * pmc->getTransitionProbability(xOriginalState, iChar) < m_dThresholdP ) break;
 
            m_szGuessing[xPwd] = pmc->ToCharacter(iChar);
			if ( *szCharClass == IS_UPPER ) m_szGuessing[xPwd] += 'A' - 'a';
 
            AddCharacters(nToAdd-1, xPreState + iChar, szCharClass+1, xPwd+1, szSyntax, pmc, dProbSoFar * pmc->getTransitionProbability(xOriginalState, iChar));
        }
    }
}
 
 
 
void CPCFG::BuildPassword(int xPwd, char *szSyntax, double dProbSoFar)
{
    int				xClass, i, iChar;
    char			szClass[MAX_LENGTH+1];
	CMarkovChain	*pmc;
 
 
    if ( !*szSyntax )
    {
		if ( ++m_nGuesses == 1000000000 )
		{
			++m_nBillions;
			m_nGuesses = 0;
		}
             
		m_szGuessing[xPwd] = '\0';

#ifdef PRINT_DEBUG
		printf("Guessing %s\n", m_szGuessing);
#endif

        if ( m_phtPasswords->Delete(m_szGuessing) )
        {
			++m_nGuessed;
            //printf( "%c\t%d.%09d\t%s\t%.3lf\n", m_nProcessor, m_nBillions, m_nGuesses, m_szGuessing, 1e-9/dProbSoFar );
            //fflush(stdout);
#ifdef PRINT_DEBUG
			getchar();
#endif
        }
    }
    else
    {
#ifdef PRINT_DEBUG
		printf("Pwd = %s\tSyntax = %s\n", m_szGuessing, szSyntax);
#endif
        xClass = 0;
 
		do
		{
			szClass[xClass++] = *szSyntax++;
        } while (  (szClass[xClass-1] == *szSyntax)
			    || (szClass[xClass-1] == IS_UPPER && *szSyntax == IS_LOWER) 
				|| (szClass[xClass-1] == IS_LOWER && *szSyntax == IS_UPPER) 
				);
 
        szClass[xClass] = '\0';
 
		// Determine which MC we will use

        switch ( szClass[0] )
        {
        case IS_DIGIT:
			pmc = m_pmcDigits;
			break;
 
        case IS_SYMBOL:
			pmc = m_pmcSymbols;
			break;

        case IS_UPPER:
        case IS_LOWER:
			pmc = (xClass < m_pmcAlphaN->m_nGrams) ? m_pmcAlpha1 : m_pmcAlphaN;
            break;

        default:
            //printf( "Unexpected token %c in grammatical structure!!!\n", szClass[0]);
            break;
        }

#ifdef TRY_DICTIONARY_ATTACKS
		// First use the MC's dictionary to fill the slot

		if ( xClass > 1 )
		{
			for ( CBaseStructure *p = pmc->m_apWordLists[xClass]; p != NULL; p = p->m_pNext )
			{
				double	dThisProb = (double)p->m_nCount / (double)pmc->m_anWordsSeen[xClass];

				if ( dThisProb * dProbSoFar < m_dThresholdP ) break;

				for ( i = 0; i < xClass; ++i )
				{
					m_szGuessing[xPwd+i] = p->m_szWord[i];
					if ( szClass[i] == IS_UPPER ) m_szGuessing[xPwd+i] += 'A' - 'a';
				}

				BuildPassword(xPwd+xClass, szSyntax, dThisProb * dProbSoFar);
			}
		}
#endif

		// Now use the MC to build next part of the password

        for ( i = 0; i < pmc->m_nPrefix; ++i )
        {
            iChar = pmc->m_pxStartProb[i];
 
            if ( dProbSoFar * pmc->m_pdStartProb[iChar] < m_dThresholdP ) break;
 
            switch ( pmc->m_nGrams )
            {
            case 1:
                    m_szGuessing[xPwd] = pmc->ToCharacter(iChar);
					if ( szClass[0] == IS_UPPER ) m_szGuessing[xPwd] += 'A' - 'a';
                    break;
            case 2:
                    m_szGuessing[xPwd] = pmc->ToCharacter(iChar / N_LETTERS);
					if ( szClass[0] == IS_UPPER ) m_szGuessing[xPwd] += 'A' - 'a';

                    m_szGuessing[xPwd+1] = pmc->ToCharacter(iChar % N_LETTERS);
					if ( szClass[1] == IS_UPPER ) m_szGuessing[xPwd+1] += 'A' - 'a';
                    
					break;
            case 3:
                    m_szGuessing[xPwd] = pmc->ToCharacter(iChar / (N_LETTERS * N_LETTERS));
					if ( szClass[0] == IS_UPPER ) m_szGuessing[xPwd] += 'A' - 'a';
                    
					m_szGuessing[xPwd+1] = pmc->ToCharacter((iChar / N_LETTERS) % N_LETTERS);
					if ( szClass[1] == IS_UPPER ) m_szGuessing[xPwd+1] += 'A' - 'a';
                    
					m_szGuessing[xPwd+2] = pmc->ToCharacter(iChar % N_LETTERS);
					if ( szClass[2] == IS_UPPER ) m_szGuessing[xPwd+2] += 'A' - 'a';
                    
					break;
            default:
                    return;
            }
 
            AddCharacters(xClass-pmc->m_nGrams, iChar, szClass+pmc->m_nGrams, xPwd+pmc->m_nGrams, szSyntax, pmc, dProbSoFar * pmc->m_pdStartProb[iChar]);
        }
    }
}
 
 
 
static CBaseStructure **my_ppStructures;
 
 
int SortStructureDescendingIndirectUsingIndex(const void *a, const void *b)
{
       int x = *(const int *)a;
       int y = *(const int *)b;
 
       return my_ppStructures[y]->m_nCount - my_ppStructures[x]->m_nCount;
}
 
 
 
void CPCFG::ThresholdGuessing()
{
    int		xGrammar, j, n, jStart, jEnd;
    int		*pxSyntax;
    double	dCumProb;
    unsigned	b;
	CHashTableElement    *pEl;
 
 
    // Get the grammatical structures into an array for sorting
 
    pxSyntax = new int [m_phtStructures->Elements()];
    my_ppStructures =  new CBaseStructure * [m_phtStructures->Elements()];
 
    for ( n = b = 0; b < m_phtStructures->TableSize(); ++b )
    {
        for ( pEl = m_phtStructures->Bucket(b); pEl != NULL; pEl = pEl->m_pNext )
        {
            my_ppStructures[n] = (CBaseStructure *)pEl->m_pData;
            pxSyntax[n] = n;
            ++n;
        }
    }

#ifdef PRINT_DEBUG
	printf("nStructures = %d ; uCount = %d\n", n, m_phtStructures->m_uCount);
#endif
    qsort(pxSyntax, n, sizeof(pxSyntax[0]), SortStructureDescendingIndirectUsingIndex);
#ifdef PRINT_DEBUG
	printf("Most common structure is %s (%.6lf)\n", my_ppStructures[pxSyntax[0]]->szNonTerminals, my_ppStructures[pxSyntax[0]]->m_nCount / m_nStructures);
#endif
 
    // Split the load across 10 different processors

	jStart = jEnd = 0;
	dCumProb = 1.0;

#ifdef PRINT_DEBUG
#else
	/*
    for ( dCumProb = 0.0, j = jStart = jEnd = 0; j < n; ++j )
    {
        dCumProb += (double)my_ppStructures[pxSyntax[j]]->m_nCount / (double)m_nStructures;
 
        if ( dCumProb > 0.10 )
        {
                jEnd = j+1;
                //if ( !fork() ) break;
                jStart = jEnd;
                dCumProb = 0.0;
                ++m_nProcessor;
        }
    }
	*/
#endif
 
    if ( jStart == jEnd )
    {
		jEnd = n;
	}
 
	//printf( "%c - %d to %d - %.4lf\n", globintProcessor, jStart, jEnd-1, dCumProb ); return;

    for ( j = jStart; j < jEnd; ++j )
    {
        xGrammar = pxSyntax[j];

        if ( (double)my_ppStructures[xGrammar]->m_nCount / (double)m_nStructures < m_dThresholdP ) break;
 
        BuildPassword(0, my_ppStructures[xGrammar]->m_szWord, (double)my_ppStructures[xGrammar]->m_nCount / (double)m_nStructures);
    }
 
    delete [] pxSyntax;
    delete [] my_ppStructures;
}
 
 

double CPCFG::GuessNumber(const char *szPwd)
{
	char			szTemp[MAX_LENGTH+1], *pSym;
	bool			bCrackedIt = false, bAllAlpha = true, bAllDigits = true;
	int				i, xLetter1, xDigit1, xSymbol1, nSameClass, iLast, iThis, ch;
    double			dTemp, dNGuesses = 1.0, dFixedGuesses = 0.0;
	CBaseStructure	syntax, *pSyntax;

	
#ifdef PRINT_DEBUG
	printf( "Computing GuessNumber for %s\n", szPwd);
#endif

	// Try dictionary attacks first

#ifdef TRY_MANGLED_WORD
	if ( !bCrackedIt )
	{
		double	dWouldNeedToTry = 1.0;

		// Test for mangled dictionary word by stripping out all non-letters
 
       for ( xLetter1 = -1, i = 0; szPwd[i] >= ' '; ++i )
       {
			if ( isalpha(szPwd[i]) )
			{
				szTemp[++xLetter1] = szPwd[i];
				bAllDigits = false;
			}
			else if ( isdigit(szPwd[i]) )
			{
				dWouldNeedToTry *= N_DIGITS;
				bAllAlpha = false;
			}
			else
			{
				dWouldNeedToTry *= N_SYMBOLS;
				bAllDigits = false;
				bAllAlpha = false;
			}
       }
 
        szTemp[++xLetter1] = '\0';
	   
		if ( xLetter1 ) 
		{
			dWouldNeedToTry *= m_pmcAlphaN->m_anUniqueWords[xLetter1] / 2;
		}

#ifdef PRINT_DEBUG
		printf( "%s --> %.*s, requiring %.0lf tries (%d for words)\n",
					szPwd, xLetter1, szTemp, dWouldNeedToTry, m_anWordLengths[xLetter1] );
		getchar();
#endif
 
       if ( dWouldNeedToTry <= 100e+9 && !bAllDigits ) //!iAllAlpha && 
       {
			_strlwr(szTemp);
 
			dFixedGuesses += dWouldNeedToTry;
 
			if ( !xLetter1 || m_pmcAlphaN->m_phtDictionary->Lookup(szTemp) )
			{
				bCrackedIt = true;
#ifdef PRINT_DEBUG
				printf( "Cracked in %.3lf billion attempts\n", dFixedGuesses/1e+9 );
#endif
			}
       }
	}
#endif
 
	// Dictionary attack failed - use grammar

	for ( iThis = xLetter1 = xDigit1 = xSymbol1 = -1, nSameClass = i = 0; szPwd[i] >= ' '; ++i )
	{
		ch = szPwd[i];
					
		iLast = iThis;
		if ( isupper(ch) ) iThis = IS_UPPER;
		else if ( islower(ch) ) iThis = IS_LOWER;
		else if ( isdigit(ch) ) iThis = IS_DIGIT;
		else iThis = IS_SYMBOL;

		syntax.m_szWord[i] = iThis;

		if ( !i || (iLast == iThis) || (iLast == IS_LOWER && iThis == IS_UPPER) || (iLast == IS_UPPER && iThis == IS_LOWER) )
		{
			++nSameClass;
		}
		else
		{
			memcpy(szTemp, szPwd+i-nSameClass, nSameClass);
			szTemp[nSameClass] = '\0';

			if ( iLast == IS_SYMBOL )
			{
#ifdef PRINT_DEBUG
				printf( "Computing g# for >>%s<<\n", szTemp);
#endif
				dNGuesses *= m_pmcSymbols->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
				printf( "%s has g# %.3lf\n", szTemp, m_pmcSymbols->GuessNumber(szTemp, 1.0)/1e+9);
#endif
			}
			else if ( iLast == IS_DIGIT )
			{
#ifdef PRINT_DEBUG
				printf( "Computing g# for >>%s<<\n", szTemp);
#endif
				dNGuesses *= m_pmcDigits->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
				printf( "%s has g# %.3lf\n", szTemp, m_pmcDigits->GuessNumber(szTemp, 1.0)/1e+9);
#endif
			}
			else if ( nSameClass < m_pmcAlphaN->m_nGrams )
			{
#ifdef PRINT_DEBUG
				printf( "Computing g# for >>%s<<\n", szTemp);
#endif
				dNGuesses *= m_pmcAlpha1->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
				printf( "%s has g# %.3lf\n", szTemp, m_pmcAlpha1->GuessNumber(szTemp, 1.0)/1e+9);
#endif
			}
			else
			{
#ifdef PRINT_DEBUG
				printf( "Computing g# for >>%s<<\n", szTemp);
#endif
				dNGuesses *= m_pmcAlphaN->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
				printf( "%s has g# %.3lf\n", szTemp, m_pmcAlphaN->GuessNumber(szTemp, 1.0)/1e+9);
#endif
			}

			nSameClass = 1;
		}

		if ( isalpha(ch) && xLetter1 < 0 ) xLetter1 = i;	/* Begin new letter chain */
		else if ( !isalpha(ch) ) xLetter1 = -1;				/* Halt letter chain */
					
		if ( isdigit(ch) && xDigit1 < 0 ) xDigit1 = i;		/* Begin new digit chain */
		else if ( !isdigit(ch) ) xDigit1 = -1;				/* Halt digit chain */
					
		pSym = strchr(m_pmcSymbols->m_szSymbols, ch);
		if ( pSym != NULL && xSymbol1 < 0 ) xSymbol1 = i;	/* Begin new symbol chain */
		else if ( pSym == NULL ) xSymbol1 = -1;				/* Halt symbol chain */
	}

	syntax.m_szWord[i] = '\0';

	memcpy(szTemp, szPwd+i-nSameClass, nSameClass);
	szTemp[nSameClass] = '\0';

	if ( iThis == IS_SYMBOL )
	{
		dNGuesses *= m_pmcSymbols->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
		printf( "%s has g# %.3lf\n", szTemp, m_pmcSymbols->GuessNumber(szTemp, 1.0)/1e+9);
#endif
	}
	else if ( iThis == IS_DIGIT )
	{
		dNGuesses *= m_pmcDigits->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
		printf( "%s has g# %.3lf\n", szTemp, m_pmcDigits->GuessNumber(szTemp, 1.0)/1e+9);
#endif
	}
	else if ( nSameClass < m_pmcAlphaN->m_nGrams )
	{
		dNGuesses *= m_pmcAlpha1->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
		printf( "%s has g# %.3lf\n", szTemp, m_pmcAlpha1->GuessNumber(szTemp, 1.0)/1e+9);
#endif
	}
	else
	{
		dNGuesses *= m_pmcAlphaN->GuessNumber(szTemp);
#ifdef PRINT_DEBUG
		printf( "%s has g# %.3lf\n", szTemp, m_pmcAlphaN->GuessNumber(szTemp, 1.0)/1e+9);
#endif
	}

	pSyntax = (CBaseStructure *)m_phtStructures->Lookup(syntax.m_szWord);
	dTemp = (pSyntax == NULL) ? 1.0/(1.0 + m_nStructures) : (double)pSyntax->m_nCount/(double)m_nStructures;	// probability of choosing this structure
 
	if ( bCrackedIt ) dFixedGuesses /= dTemp;
	else dNGuesses /= dTemp;	

#ifdef PRINT_DEBUG
	printf("p(%s) = %lf\n", syntax.szNonTerminals, dTemp);
#endif

	return dFixedGuesses + (bCrackedIt ? 0 : dNGuesses);
}


int CPCFG::LoadFromFile(const char *szPathPCFG, int nGrams, bool bUseDictionaries)
{
	char			szBuf[2048];
	FILE			*fp = fopen(szPathPCFG, "r");
	CBaseStructure	Word, *pWord;


	if ( fp == NULL ) return 0;

	if ( m_phtStructures == NULL )
	{
		m_phtStructures = new CHashTable(65537, sizeof(CBaseStructure));
		m_phtCaseMask = new CHashTable(3, sizeof(CBaseStructure));	//65537

		m_pmcAlphaN  = new CMarkovChainAlpha(nGrams);
		m_pmcAlpha1  = new CMarkovChainAlpha(1);
		m_pmcDigits  = new CMarkovChainDigit;
		m_pmcSymbols = new CMarkovChainSymbol;
	}

	if (   !m_pmcAlpha1->LoadFromFile(fp, false, bUseDictionaries)		// read in the markov chain data for the letter n-grams
		|| !m_pmcAlphaN->LoadFromFile(fp, false, bUseDictionaries)		// read in the markov chain data for the letter 1-grams
		|| !m_pmcDigits->LoadFromFile(fp, false, bUseDictionaries)		// read in the markov chain data for the digits 
		|| !m_pmcSymbols->LoadFromFile(fp, false, bUseDictionaries)		// read in the markov chain data for the symbols 
		)
	{
#ifdef PRINT_DEBUG
		printf("MC load failed - aborting");
#endif
		return 0;
	}

	// next, read the case masks
		
    while ( fgets(szBuf, sizeof(szBuf), fp) && szBuf[0] > ' ' )
    {
        if ( sscanf(szBuf, "%s %d", Word.m_szWord, &Word.m_nCount) == 2 )
		{
			pWord = (CBaseStructure *)m_phtCaseMask->Lookup(Word.m_szWord);
			if ( pWord == NULL )
			{
				m_phtCaseMask->Insert((void *)&Word);
			}
			else
			{
				pWord->m_nCount += Word.m_nCount;
			}

			m_nCaseMasks += Word.m_nCount;
		}
		else
		{
#ifdef PRINT_DEBUG
			fprintf( stderr, "can't parse %s\n", szBuf);
			getchar();
#endif
		}
    }
 
	// last, read the sentence structures 
		
    while ( fgets(szBuf, sizeof(szBuf), fp) && szBuf[0] > ' ' )
    {
        if ( sscanf(szBuf, "%s %d", Word.m_szWord, &Word.m_nCount) == 2 )
		{
			pWord = (CBaseStructure *)m_phtStructures->Lookup(Word.m_szWord);
			if ( pWord == NULL )
			{
				m_phtStructures->Insert((void *)&Word);
			}
			else
			{
				pWord->m_nCount += Word.m_nCount;
			}

			m_nStructures += Word.m_nCount;
		}
		else
		{
#ifdef PRINT_DEBUG
			fprintf( stderr, "can't parse %s\n", szBuf);
			getchar();
#endif
		}
    }
 
	// All done!

	fclose(fp);

	return 1;
}


/*

void CPCFG::AddWordToDictionary(char *szWord)
{
	bool	bIsAllAlpha = true;
	bool	bIsAllDigits = true;
	bool	bIsAllSymbols = true;
	int		i;


	for ( i = 0; szWord[i]; ++i )
	{
		if ( isalpha(szWord[i]) )
		{
			bIsAllDigits = false;
			bIsAllSymbols = false;
		}
		else if ( isdigit(szWord[i]) )
		{
			bIsAllAlpha = false;
			bIsAllSymbols = false;
		}
		else if ( szWord[i] >= ' ' && szWord[i] <= '~' )
		{
			bIsAllAlpha = false;
			bIsAllDigits = false;
		}
		else
		{
			return;
		}
	}

	m_phtDictionary->Insert(strlwr(szWord));
			
	if ( bIsAllAlpha ) m_anAlphaStringLengths[i]++;
	else if ( bIsAllDigits ) m_anDigitStringLengths[i]++;
	else if ( bIsAllSymbols ) m_anSymbolStringLengths[i]++;
	else m_anMixedStringLengths[i]++;
}

*/




int CPCFG::LoadDictionary(const char *szPathDictionary)
{
	return m_pmcAlphaN->LoadDictionary(szPathDictionary);
	/*
	char	szBuf[2048];
	FILE	*fp = fopen(szPathDictionary, "r");


	if ( fp == NULL ) return 0;

	if ( m_phtDictionary == NULL ) m_phtDictionary = new CHashTable(1500007, sizeof(CBaseStructure));

	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
		int		n = strlen(szBuf) - 1;

		if ( n > 0 && n <= MAX_LENGTH )
		{
			szBuf[n] = '\0';
			AddWordToDictionary(szBuf);
		}
	}

	fclose(fp);

	return 1;
	*/
}



void CPCFG::ProcessWord(const char *szWord)
{
	int				xWord, xState, xBegin, x;
	char			szBuf[1+MAX_LENGTH], szMask[1+MAX_LENGTH];
	CBaseStructure	Syntax, *pSyntax;


	for ( xWord = xBegin = xState = x = 0; szWord[xWord]; ++xWord )
	{
		if ( isalpha(szWord[xWord]) )
		{
			if ( xWord && xState != IS_ALPHA )
			{
				szBuf[x] = '\0';

				if ( xState == IS_SYMBOL )
				{
					m_pmcSymbols->ProcessWord(szBuf);
				}
				else
				{
					m_pmcDigits->ProcessWord(szBuf);
				}

				xBegin = xWord;
				x = 0;
			}

			xState = IS_ALPHA;
		}
		else if ( isdigit(szWord[xWord]) )
		{
			if ( xWord && xState != IS_DIGIT )
			{
				szBuf[x] = '\0';
				szMask[x] = '\0';

				if ( xState == IS_SYMBOL )
				{
					m_pmcSymbols->ProcessWord(szBuf);
				}
				else
				{
					if ( x < m_pmcAlphaN->m_nGrams ) 
						m_pmcAlpha1->ProcessWord(szBuf);
					else
						m_pmcAlphaN->ProcessWord(szBuf);

					AddCaseMask(szMask);
				}

				xBegin = xWord;
				x = 0;
			}

			xState = IS_DIGIT;
		}
		else
		{
			if ( xWord && xState != IS_SYMBOL )
			{
				szBuf[x] = '\0';
				szMask[x] = '\0';

				if ( xState == IS_DIGIT )
				{
					m_pmcDigits->ProcessWord(szBuf);
				}
				else
				{
					if ( x < m_pmcAlphaN->m_nGrams ) 
						m_pmcAlpha1->ProcessWord(szBuf);
					else
						m_pmcAlphaN->ProcessWord(szBuf);

					AddCaseMask(szMask);
				}

				xBegin = xWord;
				x = 0;
			}

			xState = IS_SYMBOL;
		}

		szBuf[x] = szWord[xWord];
		szMask[x] = (xState == IS_ALPHA) ? (isupper(szWord[xWord]) ? IS_UPPER : IS_LOWER) : xState;
		Syntax.m_szWord[xWord] = szMask[x];
		++x;
	}

	szBuf[x] = '\0';
	szMask[x] = '\0';

	if ( xState == IS_ALPHA )
	{
		if ( x < m_pmcAlphaN->m_nGrams ) 
			m_pmcAlpha1->ProcessWord(szBuf);
		else
			m_pmcAlphaN->ProcessWord(szBuf);

		AddCaseMask(szMask);
	}
	else if ( xState == IS_DIGIT )
	{
		m_pmcDigits->ProcessWord(szBuf);
	}
	else
	{
		m_pmcSymbols->ProcessWord(szBuf);
	}

	Syntax.m_szWord[xWord] = '\0';
	Syntax.m_nCount = 0;

	pSyntax = (CBaseStructure *)m_phtStructures->Insert(&Syntax);
	pSyntax->m_nCount++;
	m_nStructures++;
}



void CPCFG::AddCaseMask(const char *szMask)
{
	return;
	/*
	CBaseStructure	Mask, *pMask;


	strcpy(Mask.m_szWord, szMask);
	Mask.m_nCount = 0;

	pMask = (CBaseStructure *)m_phtCaseMask->Insert(&Mask);
	pMask->m_nCount++;

	m_nCaseMasks++;
	m_anCaseMaskLength[strlen(szMask)]++;
	*/
}



void CPCFG::WriteToTextFile(const char *szPath)
{
	unsigned	x;
	FILE	*fp = fopen(szPath, "w");


	if ( fp == NULL ) return;

	m_pmcAlpha1->WriteToTextFile(fp);
	m_pmcAlphaN->WriteToTextFile(fp);
	m_pmcDigits->WriteToTextFile(fp);
	m_pmcSymbols->WriteToTextFile(fp);

	for ( x = 0; x < m_phtCaseMask->TableSize(); ++x )
	{
		for ( CHashTableElement *pEl = m_phtCaseMask->Bucket(x); pEl != NULL; pEl = pEl->m_pNext )
		{
			CBaseStructure	*p = (CBaseStructure *)pEl->m_pData;

			fprintf(fp, "%s %d\n", p->m_szWord, p->m_nCount);
		}
	}

	fputc('\n', fp);

	for ( x = 0; x < m_phtStructures->TableSize(); ++x )
	{
		for ( CHashTableElement *pEl = m_phtStructures->Bucket(x); pEl != NULL; pEl = pEl->m_pNext )
		{
			CBaseStructure	*p = (CBaseStructure *)pEl->m_pData;

			fprintf(fp, "%s %d\n", p->m_szWord, p->m_nCount);
		}
	}

	fputc('\n', fp);

	fclose(fp);
}
