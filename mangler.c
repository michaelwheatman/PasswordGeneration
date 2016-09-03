#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "hash.h"

//#define PRINT_DEBUG
//#define  NDEBUG

#define MAX_LENGTH	32

#define N_LETTERS	26
#define N_DIGITS	10
#define N_SYMBOLS	33

#define TARGET_MIN_GUESS	5e+6
#define N_NEW_ATTEMPTS		256


#define IS_UPPER    'U'                                 
#define IS_LOWER    'L'
#define IS_DIGIT    'D'
#define IS_SYMBOL	'S'


typedef struct
{
     char	szNonTerminals[2*MAX_LENGTH+1];
     double	dPercentage;
} PwdStructure;



double mcProbDigits(char *szSeq, double *pdProbStart, double (*padProbTransition)[N_DIGITS])
{
	int		i, chPrev, chNext;
	double	dP;

	if ( !szSeq[0] ) return 1.0;
	
	chPrev = szSeq[0] - '0';
	dP = pdProbStart[chPrev];

	for ( i = 1; szSeq[i]; ++i )
	{
		chNext = szSeq[i] - '0';
		dP *= padProbTransition[chPrev][chNext];
		chPrev = chNext;
	}

	return dP;
}


double mcProbSymbols(char *szSeq, double *pdProbStart, double (*padProbTransition)[N_SYMBOLS], char *szSymbols)
{
	int		i, chPrev, chNext;
	char	*pChar;
	double	dP;


	if ( !szSeq[0] ) return 1.0;
	
	pChar = strchr(szSymbols, szSeq[0]);
	assert( pChar != NULL );
	chPrev = pChar - szSymbols;
	dP = pdProbStart[chPrev];

	for ( i = 1; szSeq[i]; ++i )
	{
		pChar = strchr(szSymbols, szSeq[i]);
		assert ( pChar != NULL );
		chNext = pChar - szSymbols;
		dP *= padProbTransition[chPrev][chNext];
		chPrev = chNext;
	}

	return dP;
}


double mcProbLetters(char *szSeq, int nGrams, int nModReduce, double *pdProbStart, double (*padProbTransition)[N_LETTERS], double *pdProbSingle)
{
	int		i, ch, xNgram;
	double	dP = 1.0;


	for ( i = xNgram = 0; i < nGrams && szSeq[i]; ++i )
	{
		ch = szSeq[i];
		ch = tolower(ch) - 'a';
		dP *= pdProbSingle[ch];
		xNgram *= N_LETTERS;
		xNgram += ch;
	}
	
	if ( i < nGrams ) return dP;

	for ( dP = pdProbStart[xNgram]; szSeq[i]; ++i )
	{
		ch = szSeq[i];
		ch = tolower(ch) - 'a';
		dP *= padProbTransition[xNgram][ch];
		xNgram %= nModReduce;
		xNgram *= N_LETTERS;
		xNgram += ch;
	}

	return dP;
}


double GuessNumber(char *szPwd, int nGrams, int nModReduce,
					double *pdStartLetterProb, double (*padNextLetterProb)[N_LETTERS], double *pdSingleLetterProb,
					double *pdStartDigitProb, double (*padNextDigitProb)[N_DIGITS],
					double *pdStartSymbolProb, double (*padNextSymbolProb)[N_SYMBOLS], char *szSymbols,
					HashTable *phtGrammar, double dTotalGrammars
					)
{
	char			szTemp[MAX_LENGTH+1], *pSym;
	int				i, xLetter1, xDigit1, xSymbol1, nExactSame, nSameClass, iLast, iThis, ch;
    double			dTemp, dNGuesses;
	PwdStructure	syntax, *pSyntax;


	dNGuesses = 1e-9;                   /* measure in billions */

	for ( iThis = xLetter1 = xDigit1 = xSymbol1 = -1, i = 0; szPwd[i] >= ' '; ++i )
	{
		ch = szPwd[i];
					
		iLast = iThis;
        if ( isupper(ch) ) iThis = IS_UPPER;
        else if ( islower(ch) ) iThis = IS_LOWER;
        else if ( isdigit(ch) ) iThis = IS_DIGIT;
        else iThis = IS_SYMBOL;
 
        if ( i )
        {
            if ( iLast == iThis )
            {
				++nSameClass;
				++nExactSame;
            }
            else
            {
                sprintf(szTemp, "%c%d", iLast, nExactSame);
                strcat(syntax.szNonTerminals, szTemp);
				nExactSame = 1;
 
				if ( !( (iLast == IS_LOWER && iThis == IS_UPPER) || (iLast == IS_UPPER && iThis == IS_LOWER) ) )
				{
					memcpy(szTemp, szPwd+i-nSameClass, nSameClass);
					szTemp[nSameClass] = '\0';

					if ( iLast == IS_SYMBOL )
					{
						dNGuesses /= mcProbSymbols(szTemp, pdStartSymbolProb, padNextSymbolProb, szSymbols);
#ifdef PRINT_DEBUG
						printf( "%s has prob %.12lf\n", szTemp, mcProbSymbols(szTemp, pdStartSymbolProb, padNextSymbolProb, szSymbols));
#endif
					}
					else if ( iLast == IS_DIGIT )
					{
						dNGuesses /= mcProbDigits(szTemp, pdStartDigitProb, padNextDigitProb);
#ifdef PRINT_DEBUG
						printf( "%s has prob %.12lf\n", szTemp, mcProbDigits(szTemp, pdStartDigitProb, padNextDigitProb));
#endif
					}
					else
					{
						dNGuesses /= mcProbLetters(szTemp, nGrams, nModReduce, pdStartLetterProb, padNextLetterProb, pdSingleLetterProb);
#ifdef PRINT_DEBUG
						printf( "%s has prob %.12lf\n", szTemp, mcProbLetters(szTemp, nGrams, nModReduce, pdStartLetterProb, padNextLetterProb, pdSingleLetterProb));
#endif
					}

					nSameClass = 1;
				}
				else
				{
					++nSameClass;
				}
            }
        }
        else
        {
            syntax.szNonTerminals[0] = '\0';
            nSameClass = 1;
			nExactSame = 1;
        }

		if ( isalpha(ch) && xLetter1 < 0 ) xLetter1 = i;	/* Begin new letter chain */
		else if ( !isalpha(ch) ) xLetter1 = -1;				/* Halt letter chain */
					
		if ( isdigit(ch) && xDigit1 < 0 ) xDigit1 = i;		/* Begin new digit chain */
		else if ( !isdigit(ch) ) xDigit1 = -1;				/* Halt digit chain */
					
		pSym = strchr(szSymbols, ch);
		if ( pSym != NULL && xSymbol1 < 0 ) xSymbol1 = i;	/* Begin new symbol chain */
		else if ( pSym == NULL ) xSymbol1 = -1;				/* Halt symbol chain */
	}

    sprintf(szTemp, "%c%d", iThis, nExactSame);
    strcat(syntax.szNonTerminals, szTemp);

	memcpy(szTemp, szPwd+i-nSameClass, nSameClass);
	szTemp[nSameClass] = '\0';

	if ( iThis == IS_SYMBOL )
	{
		dNGuesses /= mcProbSymbols(szTemp, pdStartSymbolProb, padNextSymbolProb, szSymbols);
#ifdef PRINT_DEBUG
		printf( "%s has prob %.12lf\n", szTemp, mcProbSymbols(szTemp, pdStartSymbolProb, padNextSymbolProb, szSymbols));
#endif
	}
	else if ( iThis == IS_DIGIT )
	{
		dNGuesses /= mcProbDigits(szTemp, pdStartDigitProb, padNextDigitProb);
#ifdef PRINT_DEBUG
		printf( "%s has prob %.12lf\n", szTemp, mcProbDigits(szTemp, pdStartDigitProb, padNextDigitProb));
#endif
	}
	else
	{
		dNGuesses /= mcProbLetters(szTemp, nGrams, nModReduce, pdStartLetterProb, padNextLetterProb, pdSingleLetterProb);
#ifdef PRINT_DEBUG
		printf( "%s has prob %.12lf\n", szTemp, mcProbLetters(szTemp, nGrams, nModReduce, pdStartLetterProb, padNextLetterProb, pdSingleLetterProb));
#endif
	}
 
	pSyntax = LookupHashTable(phtGrammar, syntax.szNonTerminals);
	dTemp = (pSyntax == NULL) ? 1.0/dTotalGrammars : pSyntax->dPercentage;	/* probability of choosing this structure */
 
	dNGuesses /= dTemp;	
#ifdef PRINT_DEBUG
	printf("p(%s) = %lf; total %lf\n", syntax.szNonTerminals, dTemp, dNGuesses);
#endif

	return dNGuesses;
}


 
 
int InsertChars(char *szMangled, char *szOriginal, int nCharsInsert, double dThreshold,
                           int nPrefix, int nModReduce,
                           double *pdLetterPrefix, double (*padLetterTrans)[N_LETTERS], double *pdSingleLetter,
                           double *pdDigitPrefix, double (*padDigitTrans)[N_DIGITS],
                           double *pdSymbolPrefix, double (*padSymbolTrans)[N_SYMBOLS], char *szSymbols,
                           HashTable *phtGrammar, double dTotalGrammars
                           )
{
       char   szTemp[MAX_LENGTH+1], szBest[MAX_LENGTH+1], szMask[1+MAX_LENGTH];
       int    nTries, x, ch, nAdded;
       int    iLength = strlen(szOriginal);
	   int	  iHasLower = 0, iHasUpper = 0, iHasSymbol = 0, iHasDigit = 0;
	   int	  iNewLower, iNewUpper, iNewSymbol, iNewDigit;
       double dGuesses, dBest = -1;
 
 
	   strcpy(szMangled, szOriginal);

	   for ( x = 0; szOriginal[x]; ++x )
	   {
		   ch = szOriginal[x];
		   if ( islower(ch) ) 
			{
				iHasLower++;
				szMask[x] = IS_LOWER;
		   }
		   else if ( isupper(ch) )
			{
				iHasLower++;
				szMask[x] = IS_UPPER;
		   }
		   else if ( isdigit(ch) )
			{
				iHasLower++;
				szMask[x] = IS_DIGIT;
		   }
		   else
			{
				iHasLower++;
				szMask[x] = IS_SYMBOL;
		   }
	   }

		if ( nCharsInsert == 1 && (iHasLower != 0) + (iHasUpper != 0) + (iHasDigit != 0) + (iHasSymbol != 0) == 1 ) 
			nCharsInsert = 2;

       if ( iLength + nCharsInsert > MAX_LENGTH ) return 0;
#ifdef PRINT_DEBUG
		dGuesses = GuessNumber(szOriginal,
								nPrefix, nModReduce, 
								pdLetterPrefix, padLetterTrans, pdSingleLetter,
								pdDigitPrefix, padDigitTrans,
								pdSymbolPrefix, padSymbolTrans, szSymbols,
								phtGrammar, dTotalGrammars
								);
		printf( "Password was \"%s\"\t%.3lf\t%lf\n", szOriginal, dGuesses, dThreshold);
#endif

	   strcpy(szBest, szOriginal);
 
       for ( nTries = 0; nTries < N_NEW_ATTEMPTS; ++nTries )
       {
			do
			{
				strcpy(szMangled, szOriginal);
 
				iNewLower = iHasLower;
				iNewUpper = iHasUpper;
				iNewDigit = iHasDigit;
				iNewSymbol = iHasSymbol;
 
				for ( nAdded = 0; nAdded < nCharsInsert && iLength + nAdded < MAX_LENGTH; ++nAdded )
				{
						x = rand() % (iLength+nAdded+1);
						ch = (rand() % ('~' - ' ' + 1)) + ' ';
						if ( islower(ch) ) ++iNewLower;
						else if ( isupper(ch) ) ++iNewUpper;
						else if ( isdigit(ch) ) ++iNewDigit;
						else ++iNewSymbol;
						sprintf(szTemp, "%.*s%c%s", x, szMangled, ch, szMangled+x);
						strcpy(szMangled, szTemp);
				}
			} while ( !((iNewLower != 0) + (iNewUpper != 0) + (iNewDigit != 0) + (iNewSymbol != 0) >= 3) && !(iNewLower + iNewUpper + iNewDigit >= 8 && iNewSymbol >= 2) );
 
			dGuesses = GuessNumber(szMangled,
									nPrefix, nModReduce, 
									pdLetterPrefix, padLetterTrans, pdSingleLetter,
									pdDigitPrefix, padDigitTrans,
									pdSymbolPrefix, padSymbolTrans, szSymbols,
									phtGrammar, dTotalGrammars
									);

			if ( dGuesses > dBest )
			{
#ifdef PRINT_DEBUG
				printf( "Pword (%d) is \"%s\"\t%.3lf\n", nCharsInsert, szMangled, dBest);
#endif
				strcpy(szBest, szMangled);
				dBest = dGuesses;
			}

			if ( dGuesses >= dThreshold ) return 1;
       }

	   strcpy(szMangled, szBest);
#ifdef PRINT_DEBUG
		printf( "Pword (%d) is \"%s\"\t%.3lf\n", nCharsInsert, szMangled, dBest);
#endif
       return 0; //(dBest >= dThreshold) ? 1 : 0;
}
 
 
 
int ChangeChars(char *szMangled, char *szOriginal, int nCharsChange, double dThreshold,
                    int nPrefix, int nModReduce,
                    double *pdLetterPrefix, double (*padLetterTrans)[N_LETTERS], double *pdSingleLetter,
                    double *pdDigitPrefix, double (*padDigitTrans)[N_DIGITS],
                    double *pdSymbolPrefix, double (*padSymbolTrans)[N_SYMBOLS], char *szSymbols,
                    HashTable *phtGrammar, double dTotalGrammars
                    )
{
       char   abModified[MAX_LENGTH], szBest[1+MAX_LENGTH], szMask[1+MAX_LENGTH];
       int    nTries, x, ch, nChanged;
       int    iLength = strlen(szOriginal);
	   int	  iHasLower = 0, iHasUpper = 0, iHasSymbol = 0, iHasDigit = 0;
	   int	  iNewLower, iNewUpper, iNewSymbol, iNewDigit;
       double dGuesses, dBest = -1;
 
 
       if ( iLength > MAX_LENGTH || iLength < nCharsChange ) return 0;

	   for ( x = 0; szOriginal[x]; ++x )
	   {
		   ch = szOriginal[x];
		   if ( islower(ch) ) 
			{
				iHasLower++;
				szMask[x] = IS_LOWER;
		   }
		   else if ( isupper(ch) )
			{
				iHasLower++;
				szMask[x] = IS_UPPER;
		   }
		   else if ( isdigit(ch) )
			{
				iHasLower++;
				szMask[x] = IS_DIGIT;
		   }
		   else
			{
				iHasLower++;
				szMask[x] = IS_SYMBOL;
		   }
	   }

		if ( nCharsChange == 1 && (iHasLower != 0) + (iHasUpper != 0) + (iHasDigit != 0) + (iHasSymbol != 0) == 1 ) 
			nCharsChange = 2;

#ifdef PRINT_DEBUG
		printf( "%s (%d %d %d %d) %d\n", szOriginal, iHasLower, iHasUpper, iHasDigit, iHasSymbol, nCharsChange );

		dGuesses = GuessNumber(szOriginal,
								nPrefix, nModReduce, 
								pdLetterPrefix, padLetterTrans, pdSingleLetter,
								pdDigitPrefix, padDigitTrans,
								pdSymbolPrefix, padSymbolTrans, szSymbols,
								phtGrammar, dTotalGrammars
								);
		printf( "Password was \"%s\"\t%.3lf\t%lf\n", szOriginal, dGuesses, dThreshold);
#endif

	   strcpy(szBest, szOriginal);

       for ( nTries = 0; nTries < N_NEW_ATTEMPTS; ++nTries )
       {
			do
			{
				strcpy(szMangled, szOriginal);
 
				for ( x = 0; x < MAX_LENGTH; ++x )
				{
					abModified[x] = 0;
				}
 
				iNewLower = iHasLower;
				iNewUpper = iHasUpper;
				iNewDigit = iHasDigit;
				iNewSymbol = iHasSymbol;

				for ( nChanged = 0; nChanged < nCharsChange; ++nChanged )
				{
					do
					{
						x = rand() % iLength;
					} while ( abModified[x] 
							//|| (szMask[x] == IS_LOWER && iNewLower == 1)
							//|| (szMask[x] == IS_UPPER && iNewUpper == 1)
							//|| (szMask[x] == IS_DIGIT && iNewDigit == 1)
							//|| (szMask[x] == IS_SYMBOL && iNewSymbol == 1)
							// Don't take away only instance of a character class
							);
 
					if ( szMask[x] == IS_LOWER ) --iNewLower;
					else if ( szMask[x] == IS_UPPER ) --iNewUpper;
					else if ( szMask[x] == IS_DIGIT ) --iNewDigit;
					else if ( szMask[x] == IS_SYMBOL ) --iNewSymbol;

					abModified[x] = 1;
 
					do
					{
						ch = (rand() % ('~' - ' ')) + ' ';
					} while ( ch == szMangled[x]
							|| (szMask[x] == IS_LOWER && islower(ch))
							|| (szMask[x] == IS_UPPER && isupper(ch))
							|| (szMask[x] == IS_DIGIT && isdigit(ch))
							|| (szMask[x] == IS_SYMBOL && !islower(ch) && !isupper(ch) && !isdigit(ch))
							// Must change character class
							);
 
					szMangled[x] = ch;

					if ( islower(ch) ) ++iNewLower;
					else if ( isupper(ch) ) ++iNewUpper;
					else if ( isdigit(ch) ) ++iNewDigit;
					else ++iNewSymbol;
				}
#ifdef PRINT_DEBUG
		printf( "%s (%d %d %d %d)\n", szMangled, iNewLower, iNewUpper, iNewDigit, iNewSymbol);
#endif
			} while ( (iNewLower != 0) + (iNewUpper != 0) + (iNewDigit != 0) + (iNewSymbol != 0) < 3 );
 
			dGuesses = GuessNumber(szMangled,
                                    nPrefix, nModReduce, pdLetterPrefix, padLetterTrans, pdSingleLetter,
                                    pdDigitPrefix, padDigitTrans,
                                    pdSymbolPrefix, padSymbolTrans, szSymbols,
                                    phtGrammar, dTotalGrammars
                                    );
			if ( dGuesses > dBest )
			{
#ifdef PRINT_DEBUG
				printf( "Pword (%d) is \"%s\"\t%.3lf\n", nCharsChange, szMangled, dGuesses);
#endif
				strcpy(szBest, szMangled);
				dBest = dGuesses;
			}

			if ( dGuesses >= dThreshold ) return 1;
       }
 
	   strcpy(szMangled, szBest);
#ifdef PRINT_DEBUG
		printf( "Pword (%d) is \"%s\"\t%.3lf\n", nCharsChange, szMangled, dBest);
#endif
       return 0; //(dBest >= dThreshold) ? 1 : 0;
}
 


int main(int argc, char **argv)
{
	char	szBuf[2048], szSymbols[N_SYMBOLS+2], szNewPwd[1+MAX_LENGTH];
	int		nGrams, i, j, xFile, NLetters, xRow, NModReduce, nPwdChars, xState, nPCFG, iGotNew, nChanges, nChangesNow;
    double	dTemp, dTotalLetters = 0, dTotalDigits = 0, dTotalSymbols = 0, dTotalGrammars = 0, dNGuesses, dThreshold;
	double	*pdLetterPrefix, *pdLetterStart, (*padLetters)[N_LETTERS], *pdDigitPrefix, *pdDigitStart, (*padDigits)[N_DIGITS], *pdSymbolPrefix, *pdSymbolStart, (*padSymbols)[N_SYMBOLS], adSingleLetter[N_LETTERS];
	FILE	*fp;
	PwdStructure		syntax, *pSyntax;
    HashTable			*phtGrammar = CreateHashTable(177787, sizeof(syntax));
    unsigned             xBucket;
    HashTable_Element    *pEl;


	srand((unsigned)time(NULL));

	if ( argc < 4 )
	{
		printf( "usage: %s nChanges nGrams FileToMangle TrainFile1 [TrainFile2 ...]\n", argv[0] );
		return 0;
	}

	nChanges = atoi(argv[1]);
	if ( nChanges < 1 || nChanges > 8 )
	{
		printf( "%s - nChanges must be between 1 and 8, inclusive\n", argv[0] );
		return 0;
	}

	nGrams = atoi(argv[2]);
	if ( nGrams < 1 || nGrams > 4 )
	{
		printf( "%s - nGram must be between 1 and 4, inclusive\n", argv[0] );
		return 0;
	}

	NLetters = (int)pow((double)N_LETTERS, (double)nGrams);
	NModReduce = NLetters / N_LETTERS;

	pdLetterStart = malloc(sizeof(pdLetterStart[0]) * NLetters);
	if ( pdLetterStart == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdLetterStart!\n", argv[0] );
		return 0;
	}

	pdDigitStart = malloc(sizeof(pdLetterStart[0]) * N_DIGITS);
	if ( pdDigitStart == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdDigitStart!\n", argv[0] );
		free(pdLetterStart);
		return 0;
	}

	pdSymbolStart = malloc(sizeof(pdLetterStart[0]) * N_SYMBOLS);
	if ( pdSymbolStart == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdSymbolStart!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		return 0;
	}

	pdLetterPrefix = malloc(sizeof(pdLetterPrefix[0]) * NLetters);
	if ( pdLetterPrefix == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdLetterPrefix!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		return 0;
	}

	pdDigitPrefix = malloc(sizeof(pdDigitPrefix[0]) * N_DIGITS);
	if ( pdDigitPrefix == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdDigitPrefix!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		free(pdLetterPrefix);
		return 0;
	}

	pdSymbolPrefix = malloc(sizeof(pdSymbolPrefix[0]) * N_SYMBOLS);
	if ( pdSymbolPrefix == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdSymbolPrefix!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		free(pdLetterPrefix);
		free(pdDigitPrefix);
		return 0;
	}

	padLetters = malloc(sizeof(padLetters[0]) * NLetters);
	if ( padLetters == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for padLetters!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		free(pdLetterPrefix);
		free(pdDigitPrefix);
		free(pdSymbolPrefix);
		return 0;
	}

	padDigits = malloc(sizeof(padLetters[0]) * N_DIGITS);
	if ( padDigits == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for padDigits!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		free(pdLetterPrefix);
		free(pdDigitPrefix);
		free(pdSymbolPrefix);
		free(padLetters);
		return 0;
	}

	padSymbols = malloc(sizeof(padSymbols[0]) * N_SYMBOLS);
	if ( padSymbols == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for padSymbols!\n", argv[0] );
		free(pdLetterStart);
		free(pdDigitStart);
		free(pdSymbolStart);
		free(pdLetterPrefix);
		free(pdDigitPrefix);
		free(pdSymbolPrefix);
		free(padLetters);
		free(padDigits);
		return 0;
	}

	for ( i = 0; i < N_LETTERS; ++i )
	{
		adSingleLetter[i] = 0.0;
	}

	for ( i = 0; i < NLetters; ++i )
	{
		pdLetterStart[i] = 0.0;
		pdLetterPrefix[i] = 0.0;

		for ( j = 0; j < N_LETTERS; ++j )
		{
			padLetters[i][j] = 0.0;
		}
	}

	for ( i = 0; i < N_DIGITS; ++i )
	{
		pdDigitStart[i] = 0.0;
		pdDigitPrefix[i] = 0.0;

		for ( j = 0; j < N_DIGITS; ++j )
		{
			padDigits[i][j] = 0.0;
		}
	}

	for ( i = 0; i < N_SYMBOLS; ++i )
	{
		pdSymbolStart[i] = 0.0;
		pdSymbolPrefix[i] = 0.0;

		for ( j = 0; j < N_SYMBOLS; ++j )
		{
			padSymbols[i][j] = 0.0;
		}
	}

	for ( xFile = 4; xFile < argc; ++xFile )
	{
		fp = fopen(argv[xFile], "r");
		if ( fp == NULL )
		{
			fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[xFile]);
			continue;
		}

		fprintf(stderr, "%s\n", argv[xFile]);

		/* read in the markov chain data for the letters */

        fgets(szBuf, sizeof(szBuf), fp);		/* list of chars */
 
        for ( xRow = 0; xRow < NLetters; ++xRow )
        {
            fgets(szBuf, sizeof(szBuf), fp);
            for ( xState = -2, i = nGrams; xState < N_LETTERS; ++xState )
            {
                assert(szBuf[i] == ' ');
                sscanf( szBuf+i, "%lf", &dTemp );

                if ( xState == -2 )
                {
                    dTemp += 1.0 / NLetters;				/* add fraction so no row has a 0 probability of being chosen */
                    pdLetterStart[xRow] += dTemp;
                    dTotalLetters += dTemp;

					adSingleLetter[xRow / NModReduce] += dTemp;
                }
                else if ( xState == -1 )
                {
                    pdLetterPrefix[xRow] += dTemp + 1.0;	/* add fraction for each possible transition state */
                }
                else
                {
                    padLetters[xRow][xState] += dTemp + 1.0/(double)N_LETTERS;	/* add fraction so no state has a 0 probability of being chosen */
                }
 
                for ( ; szBuf[i] == ' '; ++i )
                {
                }

				for ( ; szBuf[i] > ' '; ++i )
                {
                }
            }

			assert(szBuf[i] == '\n');
        }
 
		/* read in the markov chain data for the digits */

        fgets(szBuf, sizeof(szBuf), fp);		/* list of chars */
 
        for ( xRow = 0; xRow < N_DIGITS; ++xRow )
        {
            fgets(szBuf, sizeof(szBuf), fp);
            for ( xState = -2, i = 1; xState < N_DIGITS; ++xState )
            {
                assert(szBuf[i] == ' ');
                sscanf( szBuf+i, "%lf", &dTemp );

                if ( xState == -2 )
                {
                    dTemp += 1.0 / N_DIGITS;					/* add fraction so no row has a 0 probability of being chosen */
                    pdDigitStart[xRow] += dTemp;
                    dTotalDigits += dTemp;
                }
                else if ( xState == -1 )
                {
                    pdDigitPrefix[xRow] += dTemp + 1.0;			/* add fraction for each possible transition state */
                }
                else
                {
                    padDigits[xRow][xState] += dTemp + 0.1;		/* add fraction so no state has a 0 probability of being chosen */
                }
 
                for ( ; szBuf[i] == ' '; ++i )
                {
                }

				for ( ; szBuf[i] > ' '; ++i )
                {
                }
            }

			assert(szBuf[i] == '\n');
        }
 
 
		/* read in the markov chain data for the symbols */

        fgets(szBuf, sizeof(szBuf), fp);		/* list of chars */
		szBuf[strlen(szBuf)-1] = '\0';
		assert(strlen(szBuf) == N_SYMBOLS);
		strcpy(szSymbols, szBuf);
 
        for ( xRow = 0; xRow < N_SYMBOLS; ++xRow )
        {
            fgets(szBuf, sizeof(szBuf), fp);
            for ( xState = -2, i = 1; xState < N_SYMBOLS; ++xState )
            {
                assert(szBuf[i] == ' ');
                sscanf( szBuf+i, "%lf", &dTemp );

                if ( xState == -2 )
                {
                    dTemp += 1.0 / N_SYMBOLS;							/* add fraction so no row has a 0 probability of being chosen */
                    pdSymbolStart[xRow] += dTemp;
                    dTotalSymbols += dTemp;
                }
                else if ( xState == -1 )
                {
                    pdSymbolPrefix[xRow] += dTemp + 1.0;				/* add fraction for each possible transition state */
                }
                else
                {
                    padSymbols[xRow][xState] += dTemp + 1.0/N_SYMBOLS;	/* add fraction so no state has a 0 probability of being chosen */
                }
 
                for ( ; szBuf[i] == ' '; ++i )
                {
                }

				for ( ; szBuf[i] > ' '; ++i )
                {
                }
            }

			assert(szBuf[i] == '\n');
        }

		/* last, read the sentence structures */
		
        fgets(szBuf, sizeof(szBuf), fp);
        sscanf(szBuf, "%d", &nPCFG);

		syntax.dPercentage = 0.0;

        for ( xRow = 0; xRow < nPCFG; ++xRow )
        {
	        fgets(szBuf, sizeof(szBuf), fp);
		 
            if ( sscanf(szBuf, "%lf %s", &dTemp, syntax.szNonTerminals) == 2 )
			{
				pSyntax = InsertHashTable(phtGrammar, (void *)&syntax);
				pSyntax->dPercentage += dTemp;
				dTotalGrammars += dTemp;
			}
			else
			{
				fprintf( stderr, "%s - can't parse %s\n", argv[0], szBuf);
			}
        }
 
        assert(szBuf[i] == '\n');

		fclose(fp);
	}

	/* Turn the counts into probabilities */
 
    for ( j = 0; j < N_LETTERS; ++j )
    {
        adSingleLetter[j] /= dTotalLetters;
    }

    for ( i = 0; i < NLetters; ++i )
    {
        pdLetterStart[i] /= dTotalLetters;
 
        for ( j = 0; j < N_LETTERS; ++j )
        {
            padLetters[i][j] /= pdLetterPrefix[i];
        }
    }
 
    for ( i = 0; i < N_DIGITS; ++i )
    {
        pdDigitStart[i] /= dTotalDigits;
 
        for ( j = 0; j < N_DIGITS; ++j )
        {
            padDigits[i][j] /= pdDigitPrefix[i];
        }
    }

    for ( i = 0; i < N_SYMBOLS; ++i )
    {
        pdSymbolStart[i] /= dTotalSymbols;
 
        for ( j = 0; j < N_SYMBOLS; ++j )
        {
            padSymbols[i][j] /= pdSymbolPrefix[i];
        }
    }

	for ( xBucket = 0; xBucket < phtGrammar->nBuckets; ++xBucket )
    {
		for ( pEl = phtGrammar->ppChain[xBucket]; pEl != NULL; pEl = pEl->pNext )
		{
			pSyntax = (PwdStructure *)pEl->pData;
			pSyntax->dPercentage /= dTotalGrammars;
		}
    }

	/* Now open process the cracking file and get to work! */

	fp = fopen(argv[3], "r");
	if ( fp == NULL )
	{
		fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[2]);
		exit(0);
	}
 
	// Let the mangling begin!!

	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
		nPwdChars = strlen(szBuf);
        szBuf[--nPwdChars] = '\0';
		if (nPwdChars > MAX_LENGTH) continue;

        iGotNew = 0;
 
        dNGuesses = GuessNumber(szBuf,
                                nGrams, NModReduce, pdLetterStart, padLetters, adSingleLetter,
                                pdDigitStart, padDigits,
                                pdSymbolStart, padSymbols, szSymbols,
                                phtGrammar, dTotalGrammars
						       );
 
        if ( dNGuesses < TARGET_MIN_GUESS )
        {
			nChangesNow = nChanges;

			for ( dThreshold = TARGET_MIN_GUESS; !iGotNew; )
			{
				iGotNew = InsertChars(szNewPwd, szBuf, nChangesNow, dThreshold,
										nGrams, NModReduce,
										pdLetterStart, padLetters, adSingleLetter,
										pdDigitStart,  padDigits,
										pdSymbolStart, padSymbols, szSymbols,
										phtGrammar, dTotalGrammars
									)
							||
						ChangeChars(szNewPwd, szBuf, nChangesNow, dThreshold,
										nGrams, NModReduce,
										pdLetterStart, padLetters, adSingleLetter,
										pdDigitStart,  padDigits,
										pdSymbolStart, padSymbols, szSymbols,
										phtGrammar, dTotalGrammars
									);

				dThreshold /= 2.0;
			}

			puts(szNewPwd);
#ifdef PRINT_DEBUG
			getchar();
#endif
        }
		else
		{
			puts(szBuf);
		}
 
	}

	/* Clean up, go home */

	fclose(fp);

	DestroyHashTable(phtGrammar);

	free(pdLetterPrefix);
	free(padLetters);
	free(pdLetterStart);
	free(pdDigitStart);
	free(pdDigitPrefix);
	free(padDigits);
	free(pdSymbolStart);
	free(pdSymbolPrefix);
	free(padSymbols);

	return 0;
}
