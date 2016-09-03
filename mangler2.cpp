#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "classPCFG.h"

//#define PRINT_DEBUG
//#define  NDEBUG

#define MAX_PCFG	12
#define MAX_MC		12

#define TARGET_MIN_GUESS	10e+6
#define MIN_ORIGINAL_GUESS	100
#define N_NEW_ATTEMPTS		32


int InsertChars(char *szAddToMe)
{
	char   szTemp[MAX_LENGTH*2+1];
	int    x, ch;
	int    iLength = strlen(szAddToMe);
 
 
	if ( iLength >= MAX_LENGTH ) return 0;

	x = rand() % (iLength+1);
	ch = (rand() % ('~' - ' ' + 1)) + ' ';
	sprintf(szTemp, "%.*s%c%s", x, szAddToMe, ch, szAddToMe+x);
	strcpy(szAddToMe, szTemp);

	return 1;
}
 
 
 
int ChangeChars(char *szAddToMe)
{
    int    x, ch;
    int    iLength = strlen(szAddToMe);
 
 
	x = rand() % iLength;
	ch = (rand() % ('~' - ' ' + 1)) + ' ';
	szAddToMe[x] = ch;

    return 1;
}
 



int main(int argc, char **argv)
{
	char	szBuf[2048], szStrong[1+MAX_LENGTH], *pszDictionary = NULL, *pszInputFile = NULL;
	int		nChangesMin = 1, iFilter = 0, iMangle = 1, iMCfile = 0, iPCFGfile = 0, nToDo;
	int		nGrams, i, nAttempts, xArg, nPwdChars, nPCFG = 0, nMC = 0;
	double	dNGuesses;
	double	dLowThreshold = MIN_ORIGINAL_GUESS;
	double	dRequiredGuesses = TARGET_MIN_GUESS;
	FILE	*fp;
	CPCFG	apcfg[MAX_PCFG];
	CMarkovChain	*apmc[MAX_MC];


	srand((unsigned)time(NULL));

	if ( argc == 1 )
	{
		printf( "usage: %s -i InputFile -g MinGuess# -l LowerGuess# -d DictionaryFile -c MinChanges -M|-F -m[123] MCfile1 ... -p[123] PCFGfile ... \n", argv[0] );
		return 0;
	}

	for ( xArg = 1; xArg < argc; ++xArg )
	{
		if ( argv[xArg][0] == '-' )
		{
			iMCfile = 0;
			iPCFGfile = 0;

			switch( argv[xArg][1] )
			{
			case 'd':
				pszDictionary = argv[++xArg];
				break;
			case 'c':
				nChangesMin = argv[xArg][2] ? atoi(&argv[xArg][2]) : atoi(argv[++xArg]);
				break;
			case 'g':
				dRequiredGuesses = argv[xArg][2] ? atof(&argv[xArg][2]) : atof(argv[++xArg]);
				break;
			case 'l':
				dLowThreshold = argv[xArg][2] ? atof(&argv[xArg][2]) : atof(argv[++xArg]);
				break;
			case 'M':
				iMangle = 1;
				iFilter = 0;
				break;
			case 'F':
				iMangle = 0;
				iFilter = 1;
				break;
			case 'i':
				pszInputFile = argv[++xArg];
				break;
			case 'm':
				iMCfile = 1;
				if ( argv[xArg][2] < '1' || argv[xArg][2] > '3' )
				{
					printf( "%s: unknown argument %s \n", argv[0], argv[xArg] );
					return 0;
				}
				nGrams = argv[xArg][2] - '0';
				break;
			case 'p':
				iPCFGfile = 1;
				if ( argv[xArg][2] < '1' || argv[xArg][2] > '3' )
				{
					printf( "%s: unknown argument %s \n", argv[0], argv[xArg] );
					return 0;
				}
				nGrams = argv[xArg][2] - '0';
				break;
			default:
				printf( "%s: unknown argument %s \n", argv[0], argv[xArg] );
				return 0;
			}
		}
		else if ( iMCfile )
		{
			if ( nMC == MAX_MC )
			{
				printf( "%s: too many markov chain files; %d max\n", argv[0], MAX_MC );
				return 0;
			}

			fprintf(stderr, "Loading %s\n", argv[xArg]);

			apmc[nMC] = new CMarkovChain(N_CHARS, nGrams);
			if ( !apmc[nMC]->LoadFromFile(argv[xArg]) )
			{
				printf( "%s: Load of %s failed!\n", argv[0], argv[xArg] );
				return 0;
			}
			else
			{
				apmc[nMC]->ConvertCountsToProb(true);
			}

			if ( pszDictionary != NULL && !apmc[nMC]->LoadDictionary(pszDictionary) )
			{
				printf( "%s: Load of dictionary %s failed!\n", argv[0], pszDictionary );
			}

			++nMC;
		}
		else if ( iPCFGfile )
		{
			if ( nPCFG == MAX_PCFG )
			{
				printf( "%s: too many PCFG files; %d max\n", argv[0], MAX_PCFG );
				return 0;
			}

			fprintf(stderr, "Loading %s\n", argv[xArg]);

			if ( !apcfg[nPCFG].LoadFromFile(argv[xArg], nGrams) )
			{
				printf( "%s: Load of %s failed!\n", argv[0], argv[xArg] );
				return 0;
			}
			else
			{
				apcfg[nPCFG].ConvertCountsToProb(true);
			}

			if ( pszDictionary != NULL && !apcfg[nPCFG].LoadDictionary(pszDictionary) )
			{
				printf( "%s: Load of dictionary %s failed!\n", argv[0], pszDictionary );
			}

			++nPCFG;
		}
		else
		{
			printf( "%s: unknown argument %s \n", argv[0], argv[xArg] );
			return 0;
		}
	}

	if ( !nMC && !nPCFG )
	{
		printf( "%s: must specify at least one MC or PCFG file\n", argv[0] );
		return 0;
	}
	else if ( !pszInputFile )
	{
		printf( "%s: must specify input file\n", argv[0] );
		return 0;
	}

	fp = fopen(pszInputFile, "r");
	if ( fp == NULL )
	{
		printf( "%s: can't open input file %s\n", argv[0], pszInputFile );
		return 0;
	}

	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
#ifdef PRINT_DEBUG
		fprintf(stderr, "%s", szBuf);
#endif
		nPwdChars = strlen(szBuf);
        szBuf[--nPwdChars] = '\0';
		if (nPwdChars > MAX_LENGTH) continue;

		strcpy(szStrong, szBuf);

		for ( dNGuesses = 1e+99, i = 0; i < nMC; ++i )
		{
			double	dNewN = apmc[i]->GuessNumber(szStrong) / 1e+9;
#ifdef PRINT_DEBUG
			fprintf(stderr, "mcg#[%d] = %lg\n", i, dNewN);
#endif
			if ( dNewN < dNGuesses ) dNGuesses = dNewN;
		}

        for ( i = 0; i < nPCFG; ++i )
		{
			double	dNewN = apcfg[i].GuessNumber(szStrong) / 1e+9;
#ifdef PRINT_DEBUG
			fprintf(stderr, "pcfgg#[%d] = %lg\n", i, dNewN);
#endif
			if ( dNewN < dNGuesses ) dNGuesses = dNewN;
		}

		if ( dNGuesses < dLowThreshold ) continue;
		
		if ( iFilter )
		{
			puts(szBuf);
			continue;
		}

#ifdef PRINT_DEBUG
		fprintf(stderr, "Original g# = %lf\n", dNGuesses); //getchar();
#endif
 
        for ( nToDo = nChangesMin; dNGuesses < dRequiredGuesses; ++nToDo )
        {
			for ( nAttempts = 0; nAttempts < N_NEW_ATTEMPTS && dNGuesses < dRequiredGuesses; ++nAttempts )
			{
				strcpy(szStrong, szBuf);
#ifdef PRINT_DEBUG
				printf("%s - making %d changes\n", szStrong, nToDo);
#endif
				for ( i = 0; i < nToDo;  )
				{
					i += (rand() & 1) ? ChangeChars(szStrong) : InsertChars(szStrong);
#ifdef PRINT_DEBUG
					printf("%s - after %d changes\n", szStrong, i);
#endif
				}

				for ( dNGuesses = 1e+99, i = 0; i < nMC; ++i )
				{
					double	dNewN = apmc[i]->GuessNumber(szStrong) / 1e+9;
#ifdef PRINT_DEBUG
					fprintf(stderr, "mcg#[%d] = %lf\n", i, dNewN);
#endif
					if ( dNewN < dNGuesses ) dNGuesses = dNewN;
				}

				for ( i = 0; i < nPCFG; ++i )
				{
					double	dNewN = apcfg[i].GuessNumber(szStrong) / 1e+9;
#ifdef PRINT_DEBUG
					fprintf(stderr, "pcfgg#[%d] = %lg\n", i, dNewN);
#endif
					if ( dNewN < dNGuesses ) dNGuesses = dNewN;
				}
#ifdef PRINT_DEBUG
				printf( "%s --> --|%s|-- ; %d changes; g# now %lg\n", szBuf, szStrong, nToDo, dNGuesses );
#endif
			}
		}
		
		printf("%s\n", szStrong);
#ifdef PRINT_DEBUG
		//getchar();
#endif
	}
	
	fclose(fp);

	/* Clean up, go home */

	for ( i = 0; i < nMC; ++i )
	{
		delete apmc[i];
	}

	return 0;
}

