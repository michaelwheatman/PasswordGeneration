/*
#include <stdafx.h>
*/
#include "MarkovChain.h"
#include<ctype.h>

inline char* strlwr( char* str )
{
char* orig = str;
// process the string
for ( ; *str != '\0 '; str++ )
*str = tolower(*str);
return orig;
}


//#define PRINT_DEBUG



CBaseStructure::CBaseStructure()
{
	m_szWord[0] = '\0';
	m_nCount = 0;
	m_pNext = NULL;
}


static double *my_pdSortMeUsingIndex;
static CBaseStructure **my_ppSortMeUsingIndex;



static int IndirectSortDoubleDescending(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	return (my_pdSortMeUsingIndex[x] > my_pdSortMeUsingIndex[y]) ? -1 : (my_pdSortMeUsingIndex[x] != my_pdSortMeUsingIndex[y]);
}


static int IndirectSortByCountDescending(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	return (my_ppSortMeUsingIndex[x]->m_nCount > my_ppSortMeUsingIndex[y]->m_nCount) ? -1 : (my_ppSortMeUsingIndex[x]->m_nCount != my_ppSortMeUsingIndex[y]->m_nCount);
}


CMarkovChainAll::CMarkovChainAll(int nGrams) : CMarkovChain(N_CHARS, nGrams)
{
	//m_phtDictionary = new CHashTable(1500007, sizeof(CBaseStructure));
	m_phtDictionary = NULL;
}



CMarkovChainWords::CMarkovChainWords(int nGrams) : CMarkovChain(N_LETTERS+1, nGrams)
{
	m_phtDictionary = NULL;

	m_dStartCount = 0.0;

	for ( int i = 0; i < m_nPrefix; ++i )
	{
		m_pdStartProb[i] = 0.0;
		m_pxStartProb[i] = i;
		m_pdTransitionCount[i] = 0.0;

		for ( int j = 0; j < m_nChars; ++j )
		{
			setTransitionProbability(i, j, 0.0);
		}
	}
}



CMarkovChainAlpha::CMarkovChainAlpha(int nGrams) : CMarkovChain(N_LETTERS, nGrams)
{
	m_phtDictionary = new CHashTable(1500007, sizeof(CBaseStructure));
}




CMarkovChainDigit::CMarkovChainDigit() : CMarkovChain(N_DIGITS, 1)
{
	m_phtDictionary = new CHashTable(65537, sizeof(CBaseStructure));
}



CMarkovChainSymbol::CMarkovChainSymbol() : CMarkovChain(N_SYMBOLS, 1)
{
	int	n, x;

	for ( x = 0, n = ' '; n <= '~'; ++n )
	{
		if ( !isdigit(n) && !isalpha(n) ) m_szSymbols[x++] = n;
	}

	m_szSymbols[x] = '\0';
	assert(x == N_SYMBOLS);
	m_pszCharacters = m_szSymbols;

	m_phtDictionary = new CHashTable(65537, sizeof(CBaseStructure));
}



CMarkovChain::CMarkovChain(int nChars, int nGrams)
{
	int	i, j;

	m_nGuesses  = 0;
	m_nBillions = 0;
	m_nProcessor = 'a';

	m_dThresholdP = 1e-13;

	m_phtPasswords = NULL;
	m_phtDictionary = NULL;

	m_bAreProbabilities = false;

	m_nChars = nChars;
	m_nGrams = nGrams;
	m_nPrefix = (int)pow((double)m_nChars, (double)nGrams);
	m_nReduceModulus = m_nPrefix / m_nChars;

	m_pszCharacters = NULL;

	for ( i = 0; i <= MAX_LENGTH; ++i )
	{
		m_adLengthProb[i] = 1.0;
		m_anUniqueWords[i] = 0;
		m_anWordsSeen[i] = 0;
		m_apWordLists[i] = NULL;
	}

#ifdef PRINT_DEBUG
	puts("Allocating memory");
#endif

	m_pdStartProb = new double [m_nPrefix];
	m_pxStartProb = new int [m_nPrefix];

	m_pdTransitionCount = new double [m_nPrefix];
	m_pdTransitionProb  = new double [m_nPrefix * m_nChars];
	m_pxTransitionProb  = NULL; //new int [m_nPrefix * m_nChars];
#ifdef PRINT_DEBUG
	puts("Done allocating memory");
#endif

	m_dStartCount = 1.0;

	for ( i = 0; i < m_nPrefix; ++i )
	{
		m_pdStartProb[i] = 1.0 / m_nPrefix;
		m_pxStartProb[i] = i;
		m_pdTransitionCount[i] = 1.0;

		for ( j = 0; j < m_nChars; ++j )
		{
			setTransitionProbability(i, j, 1.0 / m_nChars);
			//setTransitionProbabilityIndex(i, j, j);
		}
	}
}



CMarkovChain::~CMarkovChain()
{
	delete [] m_pdStartProb;
	delete [] m_pxStartProb;
	delete [] m_pdTransitionProb;
	if ( m_pdTransitionCount != NULL ) delete [] m_pdTransitionCount;
	if ( m_pxTransitionProb != NULL ) delete [] m_pxTransitionProb;
	if ( m_phtDictionary != NULL ) delete m_phtDictionary;
	if ( m_phtPasswords != NULL ) delete m_phtPasswords;
}



void CMarkovChain::AddStartCount(int xPrefix, int n)
{
	m_dStartCount += n;
	m_pdStartProb[xPrefix] += n;
}



void CMarkovChain::AddTransitionCount(int xFrom, int xTo, int n)
{
	m_pdTransitionCount[xFrom] += n;
	increaseTransitionCount(xFrom, xTo, n);
}


void CMarkovChain::ConvertCountsToProb(bool bSmoothProbabilities, bool bDoLengths)
{
	int		i, j;
	double	dStartEqual = 1.0 / (double)m_nPrefix;
	double	dTransEqual = 1.0 / (double)m_nChars;
	double	dStartCountLess = 0.0, dStartTotalLess = 0.0;


	m_pxTransitionProb  = new int [m_nPrefix * m_nChars];

	for ( i = 0; i < m_nPrefix; ++i )
	{
		for ( j = 0; j < m_nChars; ++j )
		{
			setTransitionProbabilityIndex(i, j, j);
		}
	}

	m_bAreProbabilities = true;

	for ( i = 0; i < m_nPrefix; ++i )
	{
		double	dTransCountLess = 0.0, dTransTotalLess = 0.0;

		m_pdStartProb[i] /= m_dStartCount;

		if ( m_pdStartProb[i] < dStartEqual ) 
		{
			dStartCountLess += 1.0;
			dStartTotalLess += m_pdStartProb[i];
		}

		for ( j = 0; j < m_nChars; ++j )
		{
			setTransitionProbability(i, j, getTransitionProbability(i, j) / m_pdTransitionCount[i]);
			
			if ( getTransitionProbability(i, j) < dTransEqual ) 
			{
				dTransCountLess += 1.0;
				dTransTotalLess += getTransitionProbability(i, j);
			}
		}

		for ( j = 0; bSmoothProbabilities && j < m_nChars; ++j )
		{
			if ( getTransitionProbability(i, j) < dTransEqual ) setTransitionProbability(i, j, dTransTotalLess / dTransCountLess);
		}

		my_pdSortMeUsingIndex = &m_pdTransitionProb[i * m_nChars];
		qsort(&m_pxTransitionProb[i * m_nChars], m_nChars, sizeof(m_pxTransitionProb[0]), IndirectSortDoubleDescending);
	}

	delete [] m_pdTransitionCount;
	m_pdTransitionCount = NULL;

	for ( i = 0; bSmoothProbabilities && i < m_nPrefix; ++i )
	{
		if ( m_pdStartProb[i] < dStartEqual ) m_pdStartProb[i] = dStartTotalLess / dStartCountLess;
	}

	my_pdSortMeUsingIndex = m_pdStartProb;
	qsort(m_pxStartProb, m_nPrefix, sizeof(m_pxStartProb[0]), IndirectSortDoubleDescending);

	if ( bDoLengths )
	{
		for ( i = 1; i <= MAX_LENGTH; ++i )
		{
			m_adLengthProb[i] /= (m_dStartCount - m_nPrefix + MAX_LENGTH);
		}
	}

	// Sort word lists

	if ( m_apWordLists != NULL )
	{
		for ( i = 1; i <= MAX_LENGTH; ++i )
		{
			int				j, nWords = m_anUniqueWords[i], *px;
			CBaseStructure *p = m_apWordLists[i];

			if ( nWords <= 1 ) continue;

			my_ppSortMeUsingIndex = new CBaseStructure * [nWords];
			px = new int [nWords];

			for ( j = 0; p != NULL; ++j )
			{
				px[j] = j;
				my_ppSortMeUsingIndex[j] = p;
				p = p->m_pNext;
			}

			qsort(px, nWords, sizeof(px[0]), IndirectSortByCountDescending);

			m_apWordLists[i] = my_ppSortMeUsingIndex[px[0]];

			for ( j = 1; j < nWords; ++j )
			{
				my_ppSortMeUsingIndex[px[j-1]]->m_pNext = my_ppSortMeUsingIndex[px[j]];
			}

			my_ppSortMeUsingIndex[px[j-1]]->m_pNext = NULL;

			delete [] my_ppSortMeUsingIndex;
			delete [] px;
		}
	}
}



void CMarkovChain::BuildPassword(int xPwd, int xOriginalState, double dProbSoFar)
{
	int		i, iChar, xPreState;


	if ( xPwd == m_nLengthToGuess )
	{
		if ( ++m_nGuesses == 1000000000 )
		{
			++m_nBillions;
			m_nGuesses = 0;
		}

		m_szGuessing[xPwd] = '\0';

		if ( m_phtPasswords->Delete(m_szGuessing) )
		{
			printf( "%c\t%d.%09d\t%s\t%.03lf\n", m_nProcessor, m_nBillions, m_nGuesses, m_szGuessing, 1e-9/dProbSoFar );
			fflush(stdout);
		}
	}
	else
	{
		xPreState = (xOriginalState % m_nReduceModulus) * m_nChars;

		for ( i = 0; i < m_nChars; ++i )
		{
			iChar = getTransitionProbabilityIndex(xOriginalState, i);

			if ( dProbSoFar * getTransitionProbability(xOriginalState, iChar) < m_dThresholdP ) break;

			m_szGuessing[xPwd] = this->ToCharacter(iChar);

			BuildPassword(xPwd+1, xPreState + iChar, dProbSoFar * getTransitionProbability(xOriginalState, iChar));
		}
	}
}



double CMarkovChain::GuessNumber(const char *szPwd, bool bUseDictionary)
{
	int		i, xRow;
	int		nPwdChars = strlen(szPwd);
	char	szTemp[1+MAX_LENGTH];
	double	dFixedGuesses = 0, dNGuesses = 1.0;
	double	dLengthProb = m_bAreProbabilities ? m_adLengthProb[nPwdChars] : (m_adLengthProb[nPwdChars] / m_dStartCount);


#ifdef TRY_DICTIONARY_ATTACKS
	if ( m_phtDictionary != NULL && bUseDictionary )
	{
		strcpy(szTemp, szPwd);

		// Try dictionary attacks first

		dFixedGuesses += m_anUniqueWords[nPwdChars] / 2;

		if ( m_phtDictionary->Lookup(szTemp) )
		{
#ifdef PRINT_DEBUG
			printf( "Cracked %s in %lf attempts - word\n", szPwd, dFixedGuesses );
#endif
			return dFixedGuesses / dLengthProb;
		}
		else
		{
			dFixedGuesses += m_anUniqueWords[nPwdChars] / 2;
#ifdef PRINT_DEBUG
			printf( "%s not in dictionary\n", szTemp );
#endif
		}

#ifdef TRY_DELETING_1
		for ( i = 0; szPwd[i]; ++i )
		{
			dFixedGuesses += nPwdChars * m_anUniqueWords[nPwdChars-1] * m_nChars / 2;

			sprintf(szTemp, "%.*s%s", i, szPwd, &szPwd[i+1]);
			if ( m_phtDictionary->Lookup(szTemp) )
			{
#ifdef PRINT_DEBUG
				printf( "Cracked %s in %lf attempts - delete 1\n", szPwd, dFixedGuesses );
#endif
				return dFixedGuesses / dLengthProb;
			}
			else
			{
				dFixedGuesses += nPwdChars * m_anUniqueWords[nPwdChars-1] * m_nChars / 2;
#ifdef PRINT_DEBUG
				printf( "%s not in dictionary\n", szTemp );
#endif
			}
		}

#endif
	}
#endif

	if ( nPwdChars < m_nGrams )
	{
		return 0.5 * (dFixedGuesses + pow((double)m_nChars, nPwdChars)) / dLengthProb;
	}

	// Dictionary attacks failed - go to the chain!
	for ( xRow = i = 0; i < m_nGrams; ++i )
	{
		xRow *= m_nChars;
		xRow += this->ToIndex(szPwd[i]);
	}
 
	if ( m_bAreProbabilities )
		dNGuesses /= m_pdStartProb[xRow];					// probability of choosing this starting nGram
	else
		dNGuesses /= m_pdStartProb[xRow] / m_dStartCount;	// probability of choosing this starting nGram

#ifdef PRINT_DEBUG
	printf("p(start %d) = %lf (%lf)\n", xRow, m_pdStartProb[xRow], dNGuesses);
#endif
 
	for ( ; szPwd[i] >= ' '; ++i )
	{
		if ( m_bAreProbabilities )
			dNGuesses /= getTransitionProbability(xRow, this->ToIndex(szPwd[i]));								// Probability of choosing this next state
		else
			dNGuesses /= getTransitionProbability(xRow, this->ToIndex(szPwd[i])) / m_pdTransitionCount[xRow];	// Probability of choosing this next state

#ifdef PRINT_DEBUG
		printf("p(trans: [%d,%d]) = %lf (%lf)\n", xRow, this->ToIndex(szPwd[i]), getTransitionProbability(xRow, this->ToIndex(szPwd[i])), dNGuesses);
#endif
		xRow %= m_nReduceModulus;			// throw out first char in old nGram
		xRow *= m_nChars;					// and calculate the new nGram 
		xRow += this->ToIndex(szPwd[i]);
	}
 
#ifdef PRINT_DEBUG
	getchar();
#endif

#ifdef PRINT_DEBUG
	printf("p(len) = %lf\n", dLengthProb);
#endif

	return (dFixedGuesses + dNGuesses) / dLengthProb;
}



void CMarkovChain::ThresholdGuessing()
{
	int			xPwd, xNGram, i, j, iLen, jStart, jEnd;
	int			axLength[1+MAX_LENGTH];
	//double		dCumProb;


	for ( i = 0; i <= MAX_LENGTH; ++i )
	{
		axLength[i] = i;
	}

	my_pdSortMeUsingIndex = m_adLengthProb;
	qsort(axLength, 1+MAX_LENGTH, sizeof(axLength[0]), IndirectSortDoubleDescending);

	jStart = jEnd = 0;
	
	/*
	for ( dCumProb = 0.0, j = jStart = jEnd = 0; j < m_nPrefix; ++j )
	{
		dCumProb += m_pdStartProb[m_pxStartProb[j]];

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
	

	if ( jStart == jEnd )
	{
		jEnd = m_nChars;
	}

	for ( j = jStart; j < jEnd; ++j )
	{
		xNGram = m_pxStartProb[j];

		if ( m_pdStartProb[xNGram] < m_dThresholdP ) break;

		for ( i = 0; i <= MAX_LENGTH; ++i )
		{
			iLen = axLength[i];

			if ( m_adLengthProb[iLen] * m_pdStartProb[xNGram] < m_dThresholdP ) break;
		
			xPwd = 0;

			if ( m_nGrams == 1 )
			{
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram);
			}
			else if ( m_nGrams == 2 )
			{
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram / m_nChars);
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram % m_nChars);
			}
			else if ( m_nGrams == 3 )
			{
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram / (m_nChars * m_nChars));
				m_szGuessing[xPwd++] = this->ToCharacter((xNGram / m_nChars) % m_nChars);
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram % m_nChars);
			}
			else if ( m_nGrams == 4 )
			{
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram / (m_nChars * m_nChars * m_nChars));
				m_szGuessing[xPwd++] = this->ToCharacter((xNGram / (m_nChars * m_nChars)) % m_nChars);
				m_szGuessing[xPwd++] = this->ToCharacter((xNGram / m_nChars) % m_nChars);
				m_szGuessing[xPwd++] = this->ToCharacter(xNGram % m_nChars);
			}

			BuildPassword(xPwd, xNGram, m_adLengthProb[iLen] * m_pdStartProb[xNGram]);
		}
	}
}



int CMarkovChain::LoadDictionary(const char *szPathDictionary)
{
	char	szBuf[2048];
	int		n;
	FILE	*fp = fopen(szPathDictionary, "r");


	if ( fp == NULL ) return 0;

	if ( m_phtDictionary == NULL ) m_phtDictionary = new CHashTable(1500007, sizeof(CBaseStructure));

	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
		n = strlen(szBuf) - 1;
		if ( n <= MAX_LENGTH )
		{
			szBuf[n] = '\0';
			AddToDictionary(szBuf);
		}
	}

#ifdef PRINT_DEBUG
	printf("%d words in dictionary\n", m_phtDictionary->m_uCount);
#endif

	fclose(fp);

	return 1;
}



int	CMarkovChain::LoadFromFile(const char *szPath)
{
	int		iOK;
	FILE	*fp = fopen(szPath, "r");


	if ( fp != NULL )
	{
		iOK = LoadFromFile(fp, true);
		fclose(fp);
	}
	else
	{
		iOK = 0;
	}

	return iOK;
}



int CMarkovChain::LoadFromFile(FILE *fp, bool bReadLengths, bool bUseDictionaries)
{
	char	szBuf[2048];
	int		i, xRow, xState, nLengths, n;


    fgets(szBuf, sizeof(szBuf), fp);		// list of letters
	if ( m_pszCharacters != NULL ) 
	{
		szBuf[strlen(szBuf) - 1] = '\0';
		strcpy(m_pszCharacters, szBuf);
#ifdef PRINT_DEBUG
		printf( "m_pszCharacters is now %s\n", m_pszCharacters);
#endif
	}
	else
	{
#ifdef PRINT_DEBUG
		printf( "%sm_pszCharacters is NULL\n", szBuf);
#endif
	}
 
    for ( xRow = 0; xRow < m_nPrefix; ++xRow )
    {
        fgets(szBuf, sizeof(szBuf), fp);

        for ( xState = -1, i = m_nGrams; xState < m_nChars; ++xState )
        {
            if ( szBuf[i] != ' ' && szBuf[i] != '\t' ) 
			{
#ifdef PRINT_DEBUG
				printf("szBuf[i] != ' '\n%s[%d] = %d", szBuf, i, szBuf[i]);
				getchar();
#endif
				return 0;
			}

            sscanf( szBuf+i, "%d", &n );

            if ( xState == -1 )
            {
				AddStartCount(xRow, n);
            }
            else
            {
				AddTransitionCount(xRow, xState, n);
            }
 
            for ( ; szBuf[i] == ' ' || szBuf[i] == '\t'; ++i )
            {
            }

			for ( ; szBuf[i] > ' '; ++i )
            {
            }
        }

		if (szBuf[i] != '\n') 
		{
#ifdef PRINT_DEBUG
			printf("szBuf[i] != '\n'\n%s[%d] = %d", szBuf, i, szBuf[i]);
			getchar();
#endif
			return 0;
		}
    }

	// Write the length distributions, if requested -- yes for stand-alone MC, no if part of PCFG

	if ( bReadLengths )
	{
		fgets(szBuf, sizeof(szBuf), fp); /* length count */
		sscanf(szBuf, "%d", &nLengths);
		if ( nLengths > MAX_LENGTH )
		{
#ifdef PRINT_DEBUG
				fprintf( stderr, "MAX_LENGTH not long enough for %s!!", szPath);
#endif
			return 0;
		}
		else
		{
			fgets(szBuf, sizeof(szBuf), fp); /* read lengths into szBuf */
			for ( i = 0, xRow = 1; xRow <= nLengths; ++xRow )
			{
				if ( szBuf[i] != ' ')
				{
					return 0;
				}
 
				sscanf(szBuf+i, "%d", &n);
				m_adLengthProb[xRow] += n;
#ifdef PRINT_DEBUG
				printf( "m_adLengthProb[%d] = %lf\n", xRow, m_adLengthProb[xRow]);
#endif
				for ( ; szBuf[i] == ' '; ++i )
				{
				}
 
				for ( ; szBuf[i] > ' '; ++i )
				{
				}
			}
 
			if ( szBuf[i] != '\n' )
			{
#ifdef PRINT_DEBUG
				printf( "szBuf[i] != '\n');" );
#endif
				return 0;
			}
		}
	}

	while ( fgets(szBuf, sizeof(szBuf), fp) && szBuf[0] > ' ' )
	{
		char	szWord[1+MAX_LENGTH];
		int		nCount;

		//if ( m_phtDictionary == NULL ) m_phtDictionary = new CHashTable(1500007, sizeof(CBaseStructure));

		if ( bUseDictionaries && sscanf(szBuf, "%s %d", szWord, &nCount) == 2 )
		{
			if ( nCount <= 0 ) continue;

			for ( i = 0; szWord[i]; ++i )
			{
				if ( szWord[i] == 0x7f ) szWord[i] = ' ';
			}

			AddToDictionary(szWord, nCount);
		}
	}

	return 1;
}


void CMarkovChain::ProcessWord(const char *szWord)
{
	int		xFrom = 0, i;


	for ( i = 0; szWord[i] && i < m_nGrams; ++i )
	{
		xFrom *= m_nChars;
		xFrom += this->ToIndex(szWord[i]);
	}

	if ( i == m_nGrams )
	{
		m_pdStartProb[xFrom]++;
		m_dStartCount++;
		m_adLengthProb[strlen(szWord)]++;
		AddToDictionary(szWord);
	}

	for ( ; szWord[i]; ++i )
	{
		m_pdTransitionCount[xFrom]++;
		increaseTransitionCount(xFrom, this->ToIndex(szWord[i]), 1);
		xFrom %= m_nReduceModulus;
		xFrom *= m_nChars;
		xFrom += this->ToIndex(szWord[i]);
	}
}



void CMarkovChain::AddToDictionary(const char *szWord, int nTimes)
{
	CBaseStructure	Word, *pWord;
	int				nChars = strlen(szWord);


	if ( m_phtDictionary != NULL ) 
	{
		strcpy(Word.m_szWord, szWord);
		strlwr(Word.m_szWord);
		Word.m_nCount = 0;
		Word.m_pNext = NULL;

		pWord = (CBaseStructure *)m_phtDictionary->Insert(&Word);
		if ( !pWord->m_nCount ) 
		{
			m_anUniqueWords[nChars]++;
			pWord->m_pNext = m_apWordLists[nChars];
			m_apWordLists[nChars] = pWord;
		}

		pWord->m_nCount += nTimes;
		m_anWordsSeen[nChars] += nTimes;
	}
}


void CMarkovChain::WriteToTextFile(const char *szPath)
{
	FILE	*fp = fopen(szPath, "w");
	
	if ( fp != NULL )
	{
		WriteToTextFile(fp, true);
		fclose(fp);
	}
}



void CMarkovChain::WriteToTextFile(FILE *fp, bool bWriteLengths)
{
	int		i, j;


	for ( i = 0; i < m_nChars; ++i )
	{
		fprintf(fp, "%c", this->ToCharacter(i));
	}

	fputc('\n', fp);

	for ( i = 0; i < m_nPrefix; ++i )
	{
		for ( int k1 = m_nGrams - 1; k1 >= 0; --k1 )
		{
			j = i;

			for ( int k2 = k1; k2; --k2 )
			{
				j /= m_nChars;
			}

			fprintf(fp, "%c", this->ToCharacter(j % m_nChars));
		}

		fprintf(fp, "\t%.*lf", m_bAreProbabilities ? 9 : 0, m_pdStartProb[i]);

		for ( j = 0; j < m_nChars; ++j )
		{
			fprintf(fp, "\t%.*lf", m_bAreProbabilities ? 9 : 0, getTransitionProbability(i,j));
		}

		fputc('\n', fp);
	}

	if ( bWriteLengths )
	{
		fprintf(fp, "%d word lengths\n", MAX_LENGTH);

		for ( i = 1; i <= MAX_LENGTH; ++i )
		{
			fprintf(fp, " %.*lf", m_bAreProbabilities ? 9 : 0, m_adLengthProb[i]);
		}

		fputc('\n', fp);
	}

	if ( m_phtDictionary != NULL )
	{
		for ( unsigned x = 0; x < m_phtDictionary->TableSize(); ++x )
		{
			for ( CHashTableElement *pEl = m_phtDictionary->Bucket(x); pEl != NULL; pEl = pEl->m_pNext )
			{
				CBaseStructure	*p = (CBaseStructure *)pEl->m_pData;

				for ( i = 0; p->m_szWord[i]; ++i )
				{
					if ( p->m_szWord[i] == ' ' ) p->m_szWord[i] = 0x7f;
				}

				fprintf(fp, "%s %d\n", p->m_szWord, p->m_nCount);
			}
		}
	}

	fputc('\n', fp);
}



void CMarkovChainWords::BuildAWord(char *szWord, int nMaxChars)
{
	int		xNGram, xChar, xWord = 0;
	double	dThisProb, dCumProb;


	dThisProb = (double)(rand() & 0x7fff) / (double)0x7fff;

	for ( xNGram = 0, dCumProb = 0.0; (dCumProb += m_pdStartProb[m_pxStartProb[xNGram]]) < dThisProb; ++xNGram )
	{
	}

	xNGram = m_pxStartProb[xNGram];

	if ( m_nGrams == 1 )
	{
		szWord[xWord++] = this->ToCharacter(xNGram);
	}
	else if ( m_nGrams == 2 )
	{
		szWord[xWord++] = this->ToCharacter(xNGram / m_nChars);
		szWord[xWord++] = this->ToCharacter(xNGram % m_nChars);
	}
	else if ( m_nGrams == 3 )
	{
		szWord[xWord++] = this->ToCharacter(xNGram / (m_nChars * m_nChars));
		szWord[xWord++] = this->ToCharacter((xNGram / m_nChars) % m_nChars);
		szWord[xWord++] = this->ToCharacter(xNGram % m_nChars);
	}
	else if ( m_nGrams == 4 )
	{
		szWord[xWord++] = this->ToCharacter(xNGram / (m_nChars * m_nChars * m_nChars));
		szWord[xWord++] = this->ToCharacter((xNGram / (m_nChars * m_nChars)) % m_nChars);
		szWord[xWord++] = this->ToCharacter((xNGram / m_nChars) % m_nChars);
		szWord[xWord++] = this->ToCharacter(xNGram % m_nChars);
	}

	do
	{
		dThisProb = (double)(rand() & 0x7fff) / (double)0x7fff;

		for ( xChar = 0, dCumProb = 0.0; (dCumProb += getTransitionProbability(xNGram, getTransitionProbabilityIndex(xNGram, xChar))) < dThisProb; ++xChar )
		{
		}

		xChar = getTransitionProbabilityIndex(xNGram, xChar);

		if ( xChar != N_LETTERS )
		{
			szWord[xWord++] = this->ToCharacter(xChar);
			xNGram %= m_nReduceModulus;
			xNGram *= m_nChars;
			xNGram += xChar;
		}
	} while ( xWord < nMaxChars && xChar != N_LETTERS );

	szWord[xWord] = '\0';
}

