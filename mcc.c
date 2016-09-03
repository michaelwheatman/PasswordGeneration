#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "hash.h"


class CMarkovChain
{
private:
	int		m_nGrams;
	int		m_nPrefix;
	int		m_nReduceModulus;
};

/*#define  NDEBUG*/

#define MAX_LENGTH	32
#define N_CHARS		96	/* 128 - ' ' */


int	globintProcessor = 'a';

void BuildPassword(char *szPwd, int xPwd, int iLength, FILE *fp, int nModReduce, int xOriginalState, int (*paxState)[N_CHARS], double (*padState)[N_CHARS], double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
	int		i, iChar, xNewState, xPreState;


	if ( xPwd == iLength )
	{
		if ( ++*pnGuesses == 1000000000 )
		{
			++*pnBillions;
			*pnGuesses = 0;
		}

		szPwd[xPwd] = '\0';

		if ( DeleteHashElement(phtPasswords, szPwd) )
		{
			printf( "%c\t%d.%09d\t%s\t%.03lf\n", globintProcessor, *pnBillions, *pnGuesses, szPwd, 1e-9/dProbSoFar );
			fflush(stdout);
		}
	}
	else
	{
		xPreState = (xOriginalState % nModReduce) * N_CHARS;

		for ( i = 0; i < N_CHARS; ++i )
		{
			iChar = paxState[xOriginalState][i];
			xNewState = xPreState + iChar;

			if ( dProbSoFar * padState[xOriginalState][iChar] < dThresholdP ) break;

			szPwd[xPwd] = iChar + ' ';

			BuildPassword(szPwd, xPwd+1, iLength, fp, nModReduce, xNewState, paxState, padState, dProbSoFar * padState[xOriginalState][iChar], dThresholdP, phtPasswords, pnBillions, pnGuesses);
		}
	}
}



double *globpdSortMeUsingIndex;



int IndirectSortDoubleDescending(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	return (globpdSortMeUsingIndex[x] > globpdSortMeUsingIndex[y]) ? -1 : (globpdSortMeUsingIndex[x] != globpdSortMeUsingIndex[y]);
}



void ThresholdGuessing(FILE *fp, int nGrams, int NChars, int nModReduce, double *pdLengths, double *pdStart, double (*padState)[N_CHARS], double dThresholdP)
{
	char		szBuf[2048];
	int			nBillions = 0, nGuesses = 0, xPwd, xNGram, i, j, iLen, jStart, jEnd;
	int			axLength[1+MAX_LENGTH], *pxStart, (*paxState)[N_CHARS];
	HashTable	*phtPasswords = CreateHashTable(5000011, 1+MAX_LENGTH);
	double		dCumProb;


	pxStart = malloc(NChars * sizeof(pxStart[0]));
	paxState = malloc(NChars * sizeof(paxState[0]));

	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
		iLen = strlen(szBuf) - 1;
		if ( iLen <= MAX_LENGTH )
		{
			szBuf[iLen] = '\0';
			InsertHashTable(phtPasswords, szBuf);
		}
	}

	for ( i = 0; i <= MAX_LENGTH; ++i )
	{
		axLength[i] = i;
	}

	globpdSortMeUsingIndex = pdLengths;
	qsort(axLength, 1+MAX_LENGTH, sizeof(axLength[0]), IndirectSortDoubleDescending);

	for ( i = 0; i < NChars; ++i )
	{
		pxStart[i] = i;

		for ( j = 0; j < N_CHARS; ++j )
		{
			paxState[i][j] = j;
		}

		globpdSortMeUsingIndex = padState[i];
		qsort(paxState[i], N_CHARS, sizeof(paxState[i][0]), IndirectSortDoubleDescending);
	}

	globpdSortMeUsingIndex = pdStart;
	qsort(pxStart, NChars, sizeof(pxStart[0]), IndirectSortDoubleDescending);

	//jStart = jEnd = 0;
	
	for ( dCumProb = 0.0, j = jStart = jEnd = 0; j < NChars; ++j )
	{
		dCumProb += pdStart[pxStart[j]];

		if ( dCumProb > 0.10 ) 
		{
			jEnd = j+1;
			if ( !fork() ) break;
			jStart = jEnd;
			dCumProb = 0.0;
			++globintProcessor;
		}
	}
	

	if ( jStart == jEnd )
	{
		jEnd = NChars;
	}

	for ( j = jStart; j < jEnd; ++j )
	{
		xNGram = pxStart[j];

		if ( pdStart[xNGram] < dThresholdP ) break;

		for ( i = 0; i <= MAX_LENGTH; ++i )
		{
			iLen = axLength[i];

			if ( pdLengths[iLen] * pdStart[xNGram] < dThresholdP ) break;
		
			xPwd = 0;

			if ( nGrams == 1 )
			{
				szBuf[xPwd++] = xNGram + ' ';
			}
			else if ( nGrams == 2 )
			{
				szBuf[xPwd++] = xNGram / N_CHARS + ' ';
				szBuf[xPwd++] = xNGram % N_CHARS + ' ';
			}
			else if ( nGrams == 3 )
			{
				szBuf[xPwd++] = xNGram / (N_CHARS * N_CHARS) + ' ';
				szBuf[xPwd++] = (xNGram / N_CHARS) % N_CHARS + ' ';
				szBuf[xPwd++] = xNGram % N_CHARS + ' ';
			}

			BuildPassword(szBuf, xPwd, iLen, fp, nModReduce, xNGram, paxState, padState, pdLengths[iLen] * pdStart[xNGram], dThresholdP, phtPasswords, &nBillions, &nGuesses);
		}
	}

	DestroyHashTable(phtPasswords);

	free(pxStart);
	free(paxState);
}



double dRand()
{
	int		x = ((rand() & 0x03ff) << 10) | (rand() & 0x03ff);

	return (double)x / (double)0x100000;	/* returns 0.0 to < 1.0 */
}



void ActuallyGuess(FILE *fp, int nGrams, int NChars, int nModReduce, double *pdLengths, double *pdStart, double (*padState)[N_CHARS])
{
	char		szBuf[2048];
	int			iLen, i, j, xLo, xMid, xHi, nBillions = 0, nGuesses, nLength, xPwd, xNGram;
	double		dGuess;
	HashTable	*phtPasswords = CreateHashTable(5000011, 1+MAX_LENGTH);


	while ( fgets(szBuf, sizeof(szBuf), fp) )
	{
		iLen = strlen(szBuf) - 1;
		if ( iLen <= MAX_LENGTH )
		{
			szBuf[iLen] = '\0';
			InsertHashTable(phtPasswords, szBuf);
		}
	}

	for ( i = 1; i <= MAX_LENGTH; ++i )
	{
		pdLengths[i] += pdLengths[i-1];
	}

	assert(pdLengths[MAX_LENGTH] = 1.0); 

	for ( i = 0; i < NChars; ++i )
	{
		if ( i ) pdStart[i] += pdStart[i-1];

		for ( j = 1; j < N_CHARS; ++j )
		{
			padState[i][j] += padState[i][j-1];
		}
		assert(padState[i][N_CHARS-1] = 1.0);
	}

	assert(pdStart[NChars-1] = 1.0);

	for ( nGuesses = 0; nBillions < 2; ++nGuesses )
	{
		if ( nGuesses == 1000000000 )
		{
			nGuesses = 0;
			++nBillions;
		}

		while ( (dGuess = dRand()) <= 0.0 )
			;

		for ( xLo = 0, xHi = MAX_LENGTH; xLo < xHi; )
		{
			xMid = (xLo + xHi) / 2;
			if ( dGuess <= pdLengths[xMid] )
			{
				xHi = xMid;
			}
			else
			{
				xLo = xMid + 1;
			}
		}

		assert( xLo > 0 && xLo <= MAX_LENGTH && pdLengths[xLo] > 0.0 );
		assert( (!xLo || pdLengths[xLo-1] < dGuess) && dGuess <= pdLengths[xLo] );

		nLength = xLo;

		dGuess = dRand();

		for ( xLo = 0, xHi = NChars-1; xLo < xHi; )
		{
			xMid = (xLo + xHi) / 2;
			if ( dGuess <= pdStart[xMid] )
			{
				xHi = xMid;
			}
			else
			{
				xLo = xMid + 1;
			}
		}

		xPwd = 0;
		xNGram = xLo;
		assert( (!xLo || pdStart[xLo-1] < dGuess) && dGuess <= pdStart[xLo] );

		if ( nGrams == 1 )
		{
			szBuf[xPwd++] = xNGram + ' ';
		}
		else if ( nGrams == 2 )
		{
			szBuf[xPwd++] = xNGram / N_CHARS + ' ';
			szBuf[xPwd++] = xNGram % N_CHARS + ' ';
		}
		else if ( nGrams == 3 )
		{
			szBuf[xPwd++] = xNGram / (N_CHARS * N_CHARS) + ' ';
			szBuf[xPwd++] = (xNGram / N_CHARS) % N_CHARS + ' ';
			szBuf[xPwd++] = xNGram % N_CHARS + ' ';
		}

		while ( xPwd < nLength )
		{
			assert( xNGram >= 0 && xNGram < NChars );

			dGuess = dRand();

			for ( xLo = 0, xHi = N_CHARS-1; xLo < xHi; )
			{
				xMid = (xLo + xHi) / 2;
				if ( dGuess <= padState[xNGram][xMid] )
				{
					xHi = xMid;
				}
				else
				{
					xLo = xMid + 1;
				}
			}

			assert( xLo >= 0 && xLo < N_CHARS );
			assert( (!xLo || padState[xNGram][xLo-1] < dGuess) && dGuess <= padState[xNGram][xLo] );

			szBuf[xPwd++] = xLo + ' ';

			xNGram %= nModReduce;
			xNGram *= N_CHARS;
			xNGram += xLo;
		}

		szBuf[xPwd] = '\0';

		if ( DeleteHashElement(phtPasswords, szBuf) )
		{
			if ( nBillions ) printf( "%d", nBillions );
			printf( "%d\t", nGuesses );
			printf( "%s\n", szBuf );
		}
	}

	DestroyHashTable(phtPasswords);
}


int main(int argc, char **argv)
{
	char	szBuf[2048];
	int		iMakeGuesses = 0;
	int		nGrams, i, j, xFile, NChars, xRow, iFoundNonZero, NModReduce, nLengths, xState;
    double	dTemp, *pdStart, *pdPrefix, (*padState)[N_CHARS], *pdLengths, dTotalStart = 0, dTotalLength = 0, dNGuesses;
	FILE	*fp;

	srand((unsigned)time(NULL));

	if ( argc < 4 )
	{
		printf( "usage: %s nGrams FileToCrack TrainFile1 [TrainFile2 ...]\nIf 'nGrams' is < 0, the program will actually try to guess to the passwords in the cracking file.\nOtherwise, guess numbers are calculated for each password.", argv[0] );
		return 0;
	}

	nGrams = atoi(argv[1]);
	if ( nGrams < 0 )
	{
		iMakeGuesses = 1;
		nGrams *= -1;
	}

	if ( nGrams < 1 || nGrams > 4 )
	{
		printf( "%s - nGram must be between 1 and 4, inclusive\n", argv[0] );
		return 0;
	}

	pdLengths = malloc(sizeof(pdLengths[0]) * (1 + MAX_LENGTH));
	if ( pdLengths == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdLengths!\n", argv[0] );
		return 0;
	}

	NChars = (int)pow((double)N_CHARS, (double)nGrams);
	NModReduce = NChars / N_CHARS;

	pdStart = malloc(sizeof(pdStart[0]) * NChars);
	if ( pdStart == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdStart!\n", argv[0] );
		free(pdLengths);
		return 0;
	}

	pdPrefix = malloc(sizeof(pdPrefix[0]) * NChars);
	if ( pdPrefix == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for pdPrefix!\n", argv[0] );
		free(pdLengths);
		free(pdStart);
		return 0;
	}

	padState = malloc(sizeof(padState[0]) * NChars);
	if ( padState == NULL )
	{
		fprintf( stderr, "%s - can't malloc space for padState!\n", argv[0] );
		free(pdLengths);
		free(pdStart);
		free(pdPrefix);
		return 0;
	}

	for ( i = 0; i <= MAX_LENGTH; ++i )
	{
		pdLengths[i] = 0;
	}

	for ( i = 0; i < NChars; ++i )
	{
		pdStart[i] = 0.0;
		pdPrefix[i] = 0.0;

		for ( j = 0; j < N_CHARS; ++j )
		{
			padState[i][j] = 0.0;
		}
	}

	for ( xFile = 3; xFile < argc; ++xFile )
	{
		fp = fopen(argv[xFile], "r");
		if ( fp == NULL )
		{
			fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[xFile]);
			continue;
		}

		fprintf(stderr, "%s\n", argv[xFile]);

        fgets(szBuf, sizeof(szBuf), fp);		/* list of chars */
 
        for ( xRow = 0; xRow < NChars; ++xRow )
        {
            fgets(szBuf, sizeof(szBuf), fp);
            for ( xState = -2, i = nGrams; xState < N_CHARS; ++xState )
            {
                assert(szBuf[i] == ' ');
                sscanf( szBuf+i, "%lf", &dTemp );

                if ( xState == -2 )
                {
                    dTemp += 1.0 / NChars;						/* add fraction so no row has a 0 probability of being chosen */
                    pdStart[xRow] += dTemp;
                    dTotalStart += dTemp;
                }
                else if ( xState == -1 )
                {
                    pdPrefix[xRow] += dTemp + N_CHARS/100.0;	/* add fraction for each possible transition state */
                }
                else
                {
                    padState[xRow][xState] += dTemp + 0.01;		/* add fraction so no state has a 0 probability of being chosen */
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
 
        fgets(szBuf, sizeof(szBuf), fp); /* length count */
        sscanf(szBuf, "%d", &nLengths);
        if ( nLengths > MAX_LENGTH )
        {
            fprintf( stderr, "%s - MAX_LENGTH not long enough for %s!!", argv[0], argv[xFile]);
            exit(0);
        }
      
        fgets(szBuf, sizeof(szBuf), fp); /* read lengths into szBuf */
        for ( iFoundNonZero = 0, i = 0, xRow = 1; xRow <= nLengths; ++xRow )
        {
            assert(szBuf[i] == ' ');
 
            sscanf(szBuf+i, "%lf", &dTemp);
            if ( !iFoundNonZero ) iFoundNonZero = dTemp > 0.0;
            if ( iFoundNonZero ) ++dTemp;   /* add 1 so no length has a 0 probability of being chosen */
            pdLengths[xRow] += dTemp;
            dTotalLength += dTemp;
 
            for ( ; szBuf[i] == ' '; ++i )
            {
            }
 
            for ( ; szBuf[i] > ' '; ++i )
            {
            }
        }
 
        assert(szBuf[i] == '\n');

		fclose(fp);
	}

	/* Turn the counts into probabilities */
 
    for ( i = 0; i < NChars; ++i )
    {
        pdStart[i] /= dTotalStart;
 
        for ( j = 0; j < N_CHARS; ++j )
        {
            padState[i][j] /= pdPrefix[i];
        }
    }
 
    for ( i = 0; i <= nLengths; ++i )
    {
        pdLengths[i] /= dTotalLength;
    }

	/* Now open process the cracking file and get to work! */

	fp = fopen(argv[2], "r");
	if ( fp == NULL )
	{
		fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[2]);
		exit(0);
	}
 
	/* Either make guesses or compute guess numbers */

	if ( iMakeGuesses )
	{
		ThresholdGuessing(fp, nGrams, NChars, NModReduce, pdLengths, pdStart, padState, 1e-13);
		/*ActuallyGuess(fp, nGrams, NChars, NModReduce, pdLengths, pdStart, padState);*/
	}
	else
	{
		while ( fgets(szBuf, sizeof(szBuf), fp) )
		{
			nLengths = strlen(szBuf) - 1;
			if (nLengths > MAX_LENGTH) continue;
 
			dNGuesses = 1e-9;                   /* measure in billions */
			dNGuesses /= pdLengths[nLengths];	/* probability of choosing this length */
	/*printf("p(len) = %lf\n", pdLengths[nLengths]);*/
 
			for ( xRow = i = 0; i < nGrams; ++i )
			{
				xRow *= N_CHARS;
				xRow += szBuf[i] - ' ';
			}
 
			dNGuesses /= pdStart[xRow];			/* probability of choosing this starting nGram */
	/*printf("p(start) = %lf\n", pdStart[xRow]);*/
 
			for ( ; szBuf[i] >= ' '; ++i )
			{
				dNGuesses /= padState[xRow][szBuf[i] - ' '];    /* Probability of choosing this next state */
	/*printf("p(trans) = %lf\n", padState[xRow][szBuf[i] - ' ']);*/
				xRow %= NModReduce;			/* throw out first char in old nGram */
				xRow *= N_CHARS;			/* and calculate the new nGram */
				xRow += szBuf[i] - ' ';
			}
 
			printf( "%9.3lf\t%s", dNGuesses, szBuf );
	/*getchar();*/
		}
	}

	/* Clean up, go home */

	fclose(fp);

	free(pdLengths);
	free(padState);
	free(pdStart);
	free(pdPrefix);

	return 0;
}
