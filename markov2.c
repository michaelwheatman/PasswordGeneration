#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "hash.h"


#define MAX_LENGTH	32
#define N_LETTERS	26
#define N_DIGITS	10
#define N_SYMBOLS	33
#define N_CHARS		96	/* 128 - ' ' */

#define IS_UPPER    'U'                                 
#define IS_LOWER    'L'
#define IS_DIGIT    'D'
#define IS_SYMBOL	'S'


inline char* strlwr( char* str )
{
char* orig = str;
// process the string
for ( ; *str != '\0 '; str++ )
*str = tolower(*str);
return orig;
}

typedef struct
{
        char          szNonTerminals[2*MAX_LENGTH+1];
        unsigned      uCount;
}  PwdStructure;



int main(int argc, char **argv)
{
	int		nGrams, i, j, xFile, xLetter1, xDigit1, xSymbol1, xCurrent, xPrior, xGram, NLetters, NChars, ch;
	int		nSame, iLast, iThis;
	int		(*panLetters)[2+N_LETTERS], (*panDigits)[2+N_DIGITS], (*panSymbols)[2+N_SYMBOLS], (*panChars)[2+N_CHARS], *pnLength;
	char	szBuf[2048];
	FILE	*fpIn;
	FILE	*fpPCFG, *fpMC;
	char	szTemp[8];
	char	szSymbols[] = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/? ", *pSym;
    PwdStructure syntax, *pSyntax, *pMost;
    HashTable    *phtPCFG;
	unsigned			x;
	HashTable_Element	*pEl;


	if ( strlen(szSymbols) != N_SYMBOLS )
	{
		printf( "%s - szSymbols wrong length\n", argv[0] );
		return 0;
	}
	else if ( argc < 3 )
	{
		printf( "usage: %s nGram file1 [file2 ...]\n", argv[0] );
		return 0;
	}

	pnLength = malloc(sizeof(pnLength[0]) * (1 + MAX_LENGTH));
	if ( pnLength == NULL )
	{
		printf( "%s - can't malloc space for pnLength!\n", argv[0] );
		return 0;
	}

	nGrams = atoi(argv[1]);

	if ( nGrams < 1 || nGrams > 4 )
	{
		printf( "%s - nGram must be between 1 and 4, inclusive\n", argv[0] );
		return 0;
	}

	NLetters = (int)pow((double)N_LETTERS, (double)nGrams);

	panLetters = malloc(sizeof(panLetters[0]) * NLetters);
	if ( panLetters == NULL )
	{
		free(pnLength);
		printf( "%s - can't malloc space for panLetters!\n", argv[0] );
		return 0;
	}

	panDigits = malloc(sizeof(panDigits[0]) * N_DIGITS);
	if ( panDigits == NULL )
	{
		printf( "%s - can't malloc space for panDigits!\n", argv[0] );
		free(pnLength);
		free(panLetters);
		return 0;
	}

	panSymbols = malloc(sizeof(panSymbols[0]) * N_SYMBOLS);
	if ( panSymbols == NULL )
	{
		printf( "%s - can't malloc space for panSymbols!\n", argv[0] );
		free(pnLength);
		free(panLetters);
		free(panDigits);
		return 0;
	}

	NChars = (int)pow((double)N_CHARS, (double)nGrams);

	panChars = malloc(sizeof(panChars[0]) * NChars);
	if ( panChars == NULL )
	{
		printf( "%s - can't malloc space for panChars!\n", argv[0] );
		free(pnLength);
		free(panLetters);
		free(panDigits);
		free(panSymbols);
		return 0;
	}

	for ( xFile = 2; xFile < argc; ++xFile )
	{
		for ( i = 0; i <= MAX_LENGTH; ++i )
		{
			pnLength[i] = 0;
		}

		for ( i = 0; i < NLetters; ++i )
		{
			for ( j = 0; j < 2+N_LETTERS; ++j )
			{
				panLetters[i][j] = 0;
			}
		}

		for ( i = 0; i < N_DIGITS; ++i )
		{
			for ( j = 0; j < 2+N_DIGITS; ++j )
			{
				panDigits[i][j] = 0;
			}
		}

		for ( i = 0; i < N_SYMBOLS; ++i )
		{
			for ( j = 0; j < 2+N_SYMBOLS; ++j )
			{
				panSymbols[i][j] = 0;
			}
		}

		for ( i = 0; i < NChars; ++i )
		{
			for ( j = 0; j < 2+N_CHARS; ++j )
			{
				panChars[i][j] = 0;
			}
		}

		fpIn = fopen(argv[xFile], "r");
		if ( fpIn == NULL )
		{
			printf( "%s - can't open %s for reading\n", argv[0], argv[xFile]);
			continue;
		}

		sprintf(szBuf, "%s.pcfg%d", argv[xFile], nGrams);
		fpPCFG = fopen(szBuf, "w");
		if ( fpPCFG == NULL )
		{
			printf( "%s - can't open %s for writing\n", argv[0], szBuf);
			fclose(fpIn);
			continue;
		}

		sprintf(szBuf, "%s.mc%d", argv[xFile], nGrams);
		fpMC = fopen(szBuf, "w");
		if ( fpMC == NULL )
		{
			printf( "%s - can't open %s for writing\n", argv[0], szBuf);
			fclose(fpIn);
			fclose(fpPCFG);
			continue;
		}

	    phtPCFG = CreateHashTable(177787, sizeof(syntax));
		pMost = NULL;
		syntax.uCount = 1;

		printf( "%s\n", argv[xFile] );
		fflush(stdout);

		while ( fgets(szBuf, sizeof(szBuf), fpIn) )
		{
			j = strlen(szBuf) - 1;	/* -1 due to trailing \n */
			if ( j > MAX_LENGTH ) continue;
			pnLength[j]++;

			/* Scan password, first producing stats for the 'total' Markov chain */
			
			for ( iThis = -1, i = 0; szBuf[i] >= ' '; ++i )
			{
				/* First parse into the grammatical structure */

                iLast = iThis;
                if ( isupper((int)szBuf[i]) ) iThis = IS_UPPER;
                else if ( islower((int)szBuf[i]) ) iThis = IS_LOWER;
                else if ( isdigit((int)szBuf[i]) ) iThis = IS_DIGIT;
                else iThis = IS_SYMBOL;
 
                if ( i )
                {
                    if ( iLast == iThis )
                    {
						++nSame;
                    }
                    else
                    {
                        sprintf(szTemp, "%c%d", iLast, nSame);
                        strcat(syntax.szNonTerminals, szTemp);
 
                        nSame = 1;
                    }
                }
                else
                {
                    syntax.szNonTerminals[0] = '\0';
                    nSame = 1;
                }

				if ( i+1 == nGrams )
				{
					/* Chain is exactly 'n' chars long; update start count	*/

					for ( xGram = 0, j = 0; j <= i; ++j )	/* Calculate index into table */
					{
						xGram *= N_CHARS;
						xGram += szBuf[j] - ' ';
					}

					panChars[xGram][0]++;			/* Keep track of how frequently nGram used at start of password */
				}
				else if ( i >= nGrams )
				{
					for ( xGram = 0, j = i-nGrams; j < i; ++j )	/* Calculate index into table */
					{
						xGram *= N_CHARS;
						xGram += szBuf[j] - ' ';
					}

					panChars[xGram][1]++;					/* Keep track of how frequently nGram used */
					panChars[xGram][szBuf[i] - ' ' + 2]++;	/* Track frequency of letters following nGram */
				}
			}

            sprintf(szTemp, "%c%d", iThis, nSame);
            strcat(syntax.szNonTerminals, szTemp);
 
            pSyntax = (PwdStructure *)InsertHashTable(phtPCFG, &syntax);
            pSyntax->uCount++;
 
            if ( pMost == NULL || pSyntax->uCount > pMost->uCount ) pMost = pSyntax;

			/* For sub-chains, processing is case-insensitive, so set to lowercase */

			strlwr(szBuf);

			/* Make second pass, producing the chains for similar-character sequences */

			for ( xLetter1 = xDigit1 = xSymbol1 = -1, i = 0; szBuf[i] >= ' '; ++i )
			{
				ch = szBuf[i];
					
				if ( islower(ch) && xLetter1 < 0 ) xLetter1 = i;	/* Begin new letter chain */
				else if ( !islower(ch) ) xLetter1 = -1;				/* Halt letter chain */
					
				if ( isdigit(ch) && xDigit1 < 0 ) xDigit1 = i;		/* Begin new digit chain */
				else if ( !isdigit(ch) ) xDigit1 = -1;				/* Halt digit chain */
					
				pSym = strchr(szSymbols, ch);
				if ( pSym != NULL && xSymbol1 < 0 ) xSymbol1 = i;	/* Begin new symbol chain */
				else if ( pSym == NULL ) xSymbol1 = -1;				/* Halt symbol chain */

				if ( xLetter1 >= 0 && i-xLetter1+1 >= nGrams )		/* In chain of required length */
				{
					if ( i-xLetter1+1 == nGrams )
					{
						/* Chain is exactly 'n' letters long; update start count	*/

						for ( xGram = 0, j = xLetter1; j <= i; ++j )	/* Calculate index into table */
						{
							xGram *= N_LETTERS;
							xGram += szBuf[j] - 'a';
						}

						panLetters[xGram][0]++;			/* Keep track of how frequently nGram used at start of chain */
					}
					else
					{
						for ( xGram = 0, j = i-nGrams; j < i; ++j )	/* Calculate index into table */
						{
							xGram *= N_LETTERS;
							xGram += szBuf[j] - 'a';
						}

						panLetters[xGram][1]++;				/* Keep track of how frequently nGram used */
						panLetters[xGram][ch - 'a' + 2]++;	/* Track frequency of letters following nGram */
					}
				}
				else if ( xDigit1 >= 0 )
				{
					if ( i == xDigit1 )
					{
						panDigits[ch-'0'][0]++;			/* Keep track of how frequently digit used at start of chain */
					}
					else
					{
						panDigits[szBuf[i-1]-'0'][1]++;				/* Keep track of how frequently digit used */
						panDigits[szBuf[i-1]-'0'][ch - '0' + 2]++;		/* Track frequency of the subsequent digit */
					}
				}
				else if ( xSymbol1 >= 0 )
				{
					if ( i == xSymbol1 )
					{
						xCurrent = pSym - szSymbols;
						panSymbols[xCurrent][0]++;			/* Keep track of how frequently symbol used at start of chain */
					}
					else
					{
						xPrior = xCurrent;
						xCurrent = pSym - szSymbols;
						panSymbols[xPrior][1]++;				/* Keep track of how frequently symbol used */
						panSymbols[xPrior][xCurrent + 2]++;		/* Track frequency of the subsequent symbol */
					}
				}
			}
		}

		fclose(fpIn);

		if ( pMost != NULL ) printf( "Most common structure: %s (%u)\n", pMost->szNonTerminals, pMost->uCount );

		/* Dump the letters */

		fprintf( fpPCFG, "abcdefghijklmnopqrstuvwxyz\n" );

		for ( i = 0; i < NLetters; ++i )
		{
			if ( nGrams == 4 )
				fprintf(fpPCFG, "%c%c%c%c\n", 'a' + (i/(N_LETTERS*N_LETTERS*N_LETTERS)), 'a' + ((i/(N_LETTERS*N_LETTERS))%N_LETTERS), 'a' + ((i/N_LETTERS)%N_LETTERS), 'a' + (i%(N_LETTERS*N_LETTERS*N_LETTERS)));
			else if ( nGrams == 3 )
				fprintf(fpPCFG, "%c%c%c\n", 'a' + (i/(N_LETTERS*N_LETTERS)), 'a' + ((i/N_LETTERS)%N_LETTERS), 'a' + (i%(N_LETTERS*N_LETTERS)));
			else if ( nGrams == 2 )
				fprintf(fpPCFG, "%c%c\n", 'a' + i/N_LETTERS, 'a'+ (i%N_LETTERS));
			else
				fprintf(fpPCFG, "%c\n", 'a' + i);

			for ( j = 0; j < N_LETTERS + 2; ++j )
			{
				fprintf( fpPCFG, " %d", panLetters[i][j] );
			}

			fprintf(fpPCFG, "\n");
		}

		/* Dump the digits */

		fprintf( fpPCFG, "0123456789\n" );

		for ( i = 0; i < N_DIGITS; ++i )
		{
			fprintf(fpPCFG, "%c\n", '0' + i);

			for ( j = 0; j < N_DIGITS + 2; ++j )
			{
				fprintf( fpPCFG, " %d", panDigits[i][j] );
			}

			fprintf(fpPCFG, "\n");
		}

		/* Dump the symbols */

		fprintf( fpPCFG, "%s\n", szSymbols );

		for ( i = 0; i < N_SYMBOLS; ++i )
		{
			fprintf(fpPCFG, "%c\n", szSymbols[i]);

			for ( j = 0; j < N_SYMBOLS + 2; ++j )
			{
				fprintf( fpPCFG, " %d", panSymbols[i][j] );
			}

			fprintf(fpPCFG, "\n");
		}

		/* Dump the sentence structures */

		fprintf( fpPCFG, "%d sentence structures\n", phtPCFG->uCount);

		for ( x = 0; x < phtPCFG->nBuckets; ++x )
		{
			for ( pEl = phtPCFG->ppChain[x]; pEl != NULL; pEl = pEl->pNext )
			{
				pSyntax = (PwdStructure *)pEl->pData;
				fprintf( fpPCFG, "%6d %s\n", pSyntax->uCount, pSyntax->szNonTerminals );
			}
		}

		fclose(fpPCFG);

		DestroyHashTable(phtPCFG);

		/* Dump the big table */

		for ( i = 0; i < N_CHARS; ++i )
		{
			fprintf( fpMC, "%c", ' ' + i );
		}

		fprintf(fpMC, "\n");

		for ( i = 0; i < N_CHARS; ++i )
		{
			fprintf(fpMC, "%c\n", ' ' + i);

			for ( j = 0; j < N_CHARS + 2; ++j )
			{
				fprintf( fpMC, " %d", panChars[i][j] );
			}

			fprintf(fpMC, "\n");
		}

		fprintf( fpMC, "%d password lengths\n", MAX_LENGTH );

		for ( j = 1; j <= MAX_LENGTH; ++j )
		{
			fprintf( fpMC, " %d", pnLength[j] );
		}

		fprintf(fpMC, "\n");

		fclose(fpMC);
	}

	free(pnLength);
	free(panLetters);
	free(panDigits);
	free(panSymbols);
	free(panChars);

	return 0;
}
