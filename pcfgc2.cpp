#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "hash.h"
#include "classPCFG.h"


/*#define  NDEBUG*/


//#define PRINT_DEBUG




int main(int argc, char **argv)
{
	char	szBuf[2048];
	int		iMakeGuesses = 0;
	int		nGrams, n, xFile, nPwdChars;
	FILE	*fp;
	CPCFG	pcfg;


	srand((unsigned)time(NULL));

	if ( argc < 5 )
	{
		printf( "usage: %s nGrams FileToCrack Dictionary TrainFile1 [TrainFile2 ...]\nIf 'nGrams' is < 0, the program will actually try to guess to the passwords in the cracking file.\nOtherwise, guess numbers are calculated for each password.", argv[0] );
		return 0;
	}

	nGrams = atoi(argv[1]);
	if ( nGrams < 0 )
	{
		iMakeGuesses = 1;
		nGrams *= -1;
	}

	if ( nGrams < 1 || nGrams > 3 )
	{
		printf( "%s - nGram must be between 1 and 3, inclusive\n", argv[0] );
		return 0;
	}

	// Read in the training files and accumulate statistics

	for ( xFile = 4; xFile < argc; ++xFile )
	{
		pcfg.LoadFromFile(argv[xFile], nGrams);
	}

	pcfg.ConvertCountsToProb();

	pcfg.LoadDictionary(argv[3]);

	// Now open process the cracking file and get to work! 

	fp = fopen(argv[2], "r");
	if ( fp == NULL )
	{
		fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[2]);
		exit(0);
	}
 
	// Either make guesses or compute guess numbers 

	if ( iMakeGuesses )
	{
		pcfg.m_phtPasswords = new CHashTable(5000011, 1+MAX_LENGTH);

		while ( fgets(szBuf, sizeof(szBuf), fp) )
		{
			n = strlen(szBuf) - 1;
			if ( n <= MAX_LENGTH )
			{
				szBuf[n] = '\0';
				pcfg.m_phtPasswords->Insert(szBuf);
			}
		}

		pcfg.ThresholdGuessing();
	}
	else
	{
		while ( fgets(szBuf, sizeof(szBuf), fp) )
		{
			nPwdChars = strlen(szBuf) - 1;
			if ( nPwdChars <= MAX_LENGTH )
			{
				printf( "%9.3lf\t%s", pcfg.GuessNumber(szBuf) / 1e+9, szBuf );
			}
			 
#ifdef PRINT_DEBUG
			//getchar();
#endif
		}
	}

	// Clean up, go home 

	fclose(fp);

	return 0;
}
