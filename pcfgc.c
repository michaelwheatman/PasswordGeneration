#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "hash.h"


/*#define  NDEBUG*/

#define MAX_LENGTH	32

#define N_LETTERS	26
#define N_DIGITS	10
#define N_SYMBOLS	33


#define PRINT_DEBUG

#define IS_UPPER    'U'                                 
#define IS_LOWER    'L'
#define IS_DIGIT    'D'
#define IS_SYMBOL	'S'


typedef struct
{
        char	szNonTerminals[2*MAX_LENGTH+1];
        double	dPercentage;
} PwdStructure;



extern void BuildPassword(char *szPwd, int xPwd, char *szSyntax,
                                  int nGrams, int NChars, int nModReduce,
                                  int *pxLetterSingle, double *pdLetterSingle,
                                  int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                                  int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                                  int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                                  double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses);


void AddLetters(int nLetters, int xState, char *szCase,
                                  char *szPwd, int xPwd, char *szSyntax,
                                  int nGrams, int NChars, int nModReduce,
                                  int *pxLetterSingle, double *pdLetterSingle,
                                  int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                                  int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                                  int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                                  double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
       int           i, iLetter, xPreState;
 
 
#ifdef PRINT_DEBUG
		printf("Pwd = %.*s\n", xPwd, szPwd);
#endif

       if ( nLetters <= 0 )
       {
              BuildPassword(szPwd, xPwd, szSyntax,
                            nGrams, NChars, nModReduce,
                            pxLetterSingle, pdLetterSingle,
                            pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                            pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                            pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                            dProbSoFar, dThresholdP, phtPasswords, pnBillions, pnGuesses);
       }
       else
       {
              xPreState = (xState % nModReduce) * N_LETTERS;
 
              for ( i = 0; i < N_LETTERS; ++i )
              {
                     iLetter = paxStateLetterCombo[xState][i];
 
                     if ( dProbSoFar * padStateLetterCombo[xState][iLetter] < dThresholdP ) break;
 
                     szPwd[xPwd] = iLetter + ((*szCase == IS_UPPER) ? 'A' : 'a');
 
                     AddLetters(nLetters-1, xPreState + iLetter, szCase+1,
                                szPwd, xPwd+1, szSyntax,
                                nGrams, NChars, nModReduce,
                                pxLetterSingle, pdLetterSingle,
                                pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                dProbSoFar * padStateLetterCombo[xState][iLetter], dThresholdP, phtPasswords, pnBillions, pnGuesses);
              }
 
       }
}
 
 
 
void AddSingleLetter(int nLetters, char *szCase,
                        char *szPwd, int xPwd, char *szSyntax,
                        int nGrams, int NChars, int nModReduce,
                        int *pxLetterSingle, double *pdLetterSingle,
                        int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                        int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                        int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                        double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
       int           i, iLetter;
 
 
#ifdef PRINT_DEBUG
		printf("Pwd = %.*s\n", xPwd, szPwd);
#endif

       if ( nLetters <= 0 )
       {
              BuildPassword(szPwd, xPwd, szSyntax,
                            nGrams, NChars, nModReduce,
                            pxLetterSingle, pdLetterSingle,
                            pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                            pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                            pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                            dProbSoFar, dThresholdP, phtPasswords, pnBillions, pnGuesses);
       }
       else
       {
              for ( i = 0; i < N_LETTERS; ++i )
              {
                     iLetter = pxLetterSingle[i];
 
                     if ( dProbSoFar * pdLetterSingle[iLetter] < dThresholdP ) break;
 
                     szPwd[xPwd] = iLetter + ((*szCase == IS_UPPER) ? 'A' : 'a');
 
                     AddSingleLetter(nLetters-1, szCase+1,
                                    szPwd, xPwd+1, szSyntax,
                                    nGrams, NChars, nModReduce,
                                    pxLetterSingle, pdLetterSingle,
                                    pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                    pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                    pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                    dProbSoFar * pdLetterSingle[iLetter], dThresholdP, phtPasswords, pnBillions, pnGuesses);
              }
       }
}
 
 
 
void AddSymbols(int nSymbols, int xState,
                                  char *szPwd, int xPwd, char *szSyntax,
                                  int nGrams, int NChars, int nModReduce,
                                  int *pxLetterSingle, double *pdLetterSingle,
                                  int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                                  int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                                  int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                                  double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
       int           i, iSymbol;
 
 
#ifdef PRINT_DEBUG
		printf("Pwd = %.*s\n", xPwd, szPwd);
#endif

       if ( nSymbols <= 0 )
       {
              BuildPassword(szPwd, xPwd, szSyntax,
                            nGrams, NChars, nModReduce,
                            pxLetterSingle, pdLetterSingle,
                            pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                            pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                            pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                            dProbSoFar, dThresholdP, phtPasswords, pnBillions, pnGuesses);
       }
       else
       {
              for ( i = 0; i < N_SYMBOLS; ++i )
              {
                     iSymbol = paxStateSymbol[xState][i];
 
                     if ( dProbSoFar * padStateSymbol[xState][iSymbol] < dThresholdP ) break;
 
                     szPwd[xPwd] = szSymbols[iSymbol];
 
                     AddSymbols(nSymbols-1, iSymbol,
                                szPwd, xPwd+1, szSyntax,
                                nGrams, NChars, nModReduce,
                                pxLetterSingle, pdLetterSingle,
                                pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                dProbSoFar * padStateSymbol[xState][iSymbol], dThresholdP, phtPasswords, pnBillions, pnGuesses);
              }
       }
}
 
 
 
void AddDigits(int nDigits, int xState, 
				char *szPwd, int xPwd, char *szSyntax,
                int nGrams, int NChars, int nModReduce,
                int *pxLetterSingle, double *pdLetterSingle,
                int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
       int           i, iDigit;
 
 
#ifdef PRINT_DEBUG
		printf("Pwd = %.*s\n", xPwd, szPwd);
#endif

       if ( nDigits <= 0 )
       {
              BuildPassword(szPwd, xPwd, szSyntax,
                            nGrams, NChars, nModReduce,
                            pxLetterSingle, pdLetterSingle,
                            pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                            pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                            pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                            dProbSoFar, dThresholdP, phtPasswords, pnBillions, pnGuesses);
       }
       else
       {
              for ( i = 0; i < N_DIGITS; ++i )
              {
                     iDigit = paxStateDigit[xState][i];
 
                     if ( dProbSoFar * padStateDigit[xState][iDigit] < dThresholdP ) break;
 
                     szPwd[xPwd] = iDigit + '0';
 
                     AddDigits(nDigits-1, iDigit,
                                szPwd, xPwd+1, szSyntax,
                                nGrams, NChars, nModReduce,
                                pxLetterSingle, pdLetterSingle,
                                pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                dProbSoFar * padStateDigit[xState][iDigit], dThresholdP, phtPasswords, pnBillions, pnGuesses);
              }
       }
}


int    globintProcessor = 'a';
 
 
void BuildPassword(char *szPwd, int xPwd, char *szSyntax,
                    int nGrams, int NChars, int nModReduce,
                    int *pxLetterSingle, double *pdLetterSingle,
                    int *pxStartLetterCombo, double *pdStartLetterCombo, int (*paxStateLetterCombo)[N_LETTERS], double (*padStateLetterCombo)[N_LETTERS],
                    int *pxStartDigit, double *pdStartDigit, int (*paxStateDigit)[N_DIGITS], double (*padStateDigit)[N_DIGITS],
                    int *pxStartSymbol, double *pdStartSymbol, int (*paxStateSymbol)[N_SYMBOLS], double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                    double dProbSoFar, double dThresholdP, HashTable *phtPasswords, int *pnBillions, int *pnGuesses)
{
       int           xClass, nChars, i, iChar;
       char   szClass[MAX_LENGTH+1];
 
 
       if ( !*szSyntax )
       {
              if ( ++*pnGuesses == 1000000000 )
              {
                     ++*pnBillions;
                     *pnGuesses = 0;
              }
             
              szPwd[xPwd] = '\0';

#ifdef PRINT_DEBUG
			  printf("Guessing %s\n", szPwd);
#endif

              if ( DeleteHashElement(phtPasswords, szPwd) )
              {
                     printf( "%c\t%d.%09d\t%s\t%.3lf\n", globintProcessor, *pnBillions, *pnGuesses, szPwd, 1e-9/dProbSoFar );
                     fflush(stdout);
#ifdef PRINT_DEBUG
					  getchar();
#endif
              }
       }
       else
       {
#ifdef PRINT_DEBUG
			  printf("Pwd = %s\tSyntax = %s\n", szPwd, szSyntax);
#endif
              xClass = 0;
 
              do
              {
                     szClass[xClass++] = *szSyntax++;
 
                     for ( nChars = 0; isdigit((int)*szSyntax); ++szSyntax )
                     {
                           nChars = nChars * 10 + (*szSyntax - '0');
                     }
 
                     for ( i = 1; i < nChars; ++i, ++xClass )
                     {
                           szClass[xClass] = szClass[xClass-1];
                     }
              } while ( (szClass[xClass-1] == IS_UPPER && *szSyntax == IS_LOWER) || (szClass[xClass-1] == IS_LOWER && *szSyntax == IS_UPPER) );
 
              szClass[xClass] = '\0';
 
              switch ( szClass[0] )
              {
              case IS_DIGIT:
                     for ( i = 0; i < N_DIGITS; ++i )
                     {
                           iChar = pxStartDigit[i];
 
                           if ( dProbSoFar * pdStartDigit[iChar] < dThresholdP ) break;
 
                           szPwd[xPwd] = iChar + '0';
 
                           AddDigits(xClass-1, iChar,
										szPwd, xPwd+1, szSyntax,
										nGrams, NChars, nModReduce,
										pxLetterSingle, pdLetterSingle,
										pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
										pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
										pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
										dProbSoFar * pdStartDigit[iChar], dThresholdP, phtPasswords, pnBillions, pnGuesses);
                     }
 
                     break;
 
              case IS_SYMBOL:
                     for ( i = 0; i < N_SYMBOLS; ++i )
                     {
                           iChar = pxStartSymbol[i];
 
                           if ( dProbSoFar * pdStartSymbol[iChar] < dThresholdP ) break;
 
                           szPwd[xPwd] = szSymbols[iChar];
 
                           AddSymbols(xClass-1, iChar,
										szPwd, xPwd+1, szSyntax,
										nGrams, NChars, nModReduce,
										pxLetterSingle, pdLetterSingle,
										pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
										pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
										pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
										dProbSoFar * pdStartSymbol[iChar], dThresholdP, phtPasswords, pnBillions, pnGuesses);
                     }
 
                     break;
 
              case IS_UPPER:
              case IS_LOWER:
                     if ( xClass < nGrams )
                     {
                           AddSingleLetter(xClass, szClass,
                                            szPwd, xPwd, szSyntax,
                                            nGrams, NChars, nModReduce,
                                            pxLetterSingle, pdLetterSingle,
                                            pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                            pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                            pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                            dProbSoFar, dThresholdP, phtPasswords, pnBillions, pnGuesses);
                     }
                     else
                     {
                           for ( i = 0; i < NChars; ++i )
                           {
                                  iChar = pxStartLetterCombo[i];
 
                                  if ( dProbSoFar * pdStartLetterCombo[iChar] < dThresholdP ) break;
 
                                  switch ( nGrams )
                                  {
                                  case 1:
                                         szPwd[xPwd] = iChar + ((szClass[0] == IS_UPPER) ? 'A' : 'a');
                                         break;
                                  case 2:
                                         szPwd[xPwd] = (iChar / N_LETTERS) + ((szClass[0] == IS_UPPER) ? 'A' : 'a');
                                         szPwd[xPwd+1] = (iChar % N_LETTERS) + ((szClass[1] == IS_UPPER) ? 'A' : 'a');
                                         break;
                                  case 3:
                                         szPwd[xPwd] = (iChar / (N_LETTERS * N_LETTERS)) + ((szClass[0] == IS_UPPER) ? 'A' : 'a');
                                         szPwd[xPwd+1] = ((iChar / N_LETTERS) % N_LETTERS) + ((szClass[1] == IS_UPPER) ? 'A' : 'a');
                                         szPwd[xPwd+2] = (iChar % N_LETTERS)               + ((szClass[2] == IS_UPPER) ? 'A' : 'a');
                                         break;
                                  default:
                                         return;
                                  }
 
                                  AddLetters(xClass-nGrams, iChar, szClass+nGrams,
                                                szPwd, xPwd+nGrams, szSyntax,
                                                nGrams, NChars, nModReduce,
                                                pxLetterSingle, pdLetterSingle,
                                                pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                                                pxStartDigit, pdStartDigit, paxStateDigit, padStateDigit,
                                                pxStartSymbol, pdStartSymbol, paxStateSymbol, padStateSymbol, szSymbols,
                                                dProbSoFar * pdStartLetterCombo[iChar], dThresholdP, phtPasswords, pnBillions, pnGuesses);
                           }
                     }
 
                     break;
              default:
                     printf( "Unexpected token %c in grammatical structure!!!\n", szClass[0]);
                     break;
              }
       }
}
 
 
 
double *globpdSortMeUsingIndex;
 
 
int SortDoubleDescendingUsingIndex(const void *a, const void *b)
{
       int x = *(const int *)a;
       int y = *(const int *)b;
 
       return (globpdSortMeUsingIndex[x] > globpdSortMeUsingIndex[y]) ? -1 : (globpdSortMeUsingIndex[x] != globpdSortMeUsingIndex[y]);
}
 
 
PwdStructure **globppStructures;
 
 
int SortStructureDescendingIndirectUsingIndex(const void *a, const void *b)
{
       int x = *(const int *)a;
       int y = *(const int *)b;
 
       return (globppStructures[x]->dPercentage > globppStructures[y]->dPercentage) ? -1 : (globppStructures[x]->dPercentage != globppStructures[y]->dPercentage);
}
 
 
 
void ThresholdGuessing(FILE *fp, int nGrams, int NChars, int nModReduce, HashTable *phtGrammar,
                                         double *pdLetterSingle, double *pdStartLetterCombo, double (*padStateLetterCombo)[N_LETTERS],
                                         double *pdStartDigit, double (*padStateDigit)[N_DIGITS],
                                         double *pdStartSymbol, double (*padStateSymbol)[N_SYMBOLS], char *szSymbols,
                                         double dThresholdP)
{
       char          szBuf[2048];
       unsigned      b;
       int                  nBillions = 0, nGuesses = 0, xGrammar, i, j, iPwdLen, jStart, jEnd, nStructures;
       int                  *pxSyntax;
       int                  *pxStartLetterCombo, (*paxStateLetterCombo)[N_LETTERS], axLetterSingle[N_LETTERS];
       int                  axStartDigit[N_DIGITS], aaxStateDigit[N_DIGITS][N_DIGITS];
       int                  axStartSymbol[N_SYMBOLS], aaxStateSymbol[N_SYMBOLS][N_SYMBOLS];
       HashTable            *phtPasswords = CreateHashTable(5000011, 1+MAX_LENGTH);
	   HashTable_Element    *pEl;
       double        dCumProb;
 
 
       // Store passwords to crack in hash table
 
       while ( fgets(szBuf, sizeof(szBuf), fp) )
       {
              iPwdLen = strlen(szBuf) - 1;
              if ( iPwdLen <= MAX_LENGTH )
              {
                     szBuf[iPwdLen] = '\0';
                     InsertHashTable(phtPasswords, szBuf);
              }
       }
 
       // Grab memory for indexing
 
       pxStartLetterCombo = (int *)malloc(NChars * sizeof(pxStartLetterCombo[0]));
       paxStateLetterCombo = (int (*)[26])malloc(NChars * sizeof(paxStateLetterCombo[0]));
 
       // Build indexing arrays and sort ...
       // ... letters first
 
       for ( i = 0; i < NChars; ++i )
       {
              pxStartLetterCombo[i] = i;
 
              for ( j = 0; j < N_LETTERS; ++j )
              {
                     paxStateLetterCombo[i][j] = j;
              }
 
              globpdSortMeUsingIndex = padStateLetterCombo[i];
              qsort(paxStateLetterCombo[i], N_LETTERS, sizeof(paxStateLetterCombo[i][0]), SortDoubleDescendingUsingIndex);
       }
 
       globpdSortMeUsingIndex = pdStartLetterCombo;
       qsort(pxStartLetterCombo, NChars, sizeof(pxStartLetterCombo[0]), SortDoubleDescendingUsingIndex);
#ifdef PRINT_DEBUG
	   printf("Most common starting nGram is %c (%.6lf)\n", pxStartLetterCombo[0] + 'a', pdStartLetterCombo[pxStartLetterCombo[0]]);
#endif
 
       // ... digits next
 
       for ( i = 0; i < N_DIGITS; ++i )
       {
              axStartDigit[i] = i;
 
              for ( j = 0; j < N_DIGITS; ++j )
              {
                     aaxStateDigit[i][j] = j;
              }
 
              globpdSortMeUsingIndex = padStateDigit[i];
              qsort(aaxStateDigit[i], N_DIGITS, sizeof(aaxStateDigit[i][0]), SortDoubleDescendingUsingIndex);
       }
 
       globpdSortMeUsingIndex = pdStartDigit;
       qsort(axStartDigit, N_DIGITS, sizeof(axStartDigit[0]), SortDoubleDescendingUsingIndex);

#ifdef PRINT_DEBUG
	   printf("Most common starting digit is %c (%.6lf)\n", axStartDigit[0] + '0', pdStartDigit[axStartDigit[0]]);
#endif
 
       // ... then symbols
 
       for ( i = 0; i < N_SYMBOLS; ++i )
       {
              axStartSymbol[i] = i;
 
              for ( j = 0; j < N_SYMBOLS; ++j )
              {
                     aaxStateSymbol[i][j] = j;
              }
 
              globpdSortMeUsingIndex = padStateSymbol[i];
              qsort(aaxStateSymbol[i], N_SYMBOLS, sizeof(aaxStateSymbol[i][0]), SortDoubleDescendingUsingIndex);
       }

	   globpdSortMeUsingIndex = pdStartSymbol;
       qsort(axStartSymbol, N_SYMBOLS, sizeof(axStartSymbol[0]), SortDoubleDescendingUsingIndex);
 
#ifdef PRINT_DEBUG
	   printf("Most common starting symbol is %c (%.6lf)\n", szSymbols[axStartSymbol[0]], pdStartSymbol[axStartSymbol[0]]);
#endif
 
       // ... single letters last
 
       for ( i = 0; i < N_LETTERS; ++i )
       {
              axLetterSingle[i] = i;
       }
 
       globpdSortMeUsingIndex = pdLetterSingle;
       qsort(axLetterSingle, N_LETTERS, sizeof(axLetterSingle[0]), SortDoubleDescendingUsingIndex);
#ifdef PRINT_DEBUG
	   printf("Most common single letter is %c (%.6lf)\n", axLetterSingle[0] + ' ', pdLetterSingle[axLetterSingle[0]]);
#endif

       // Get the grammatical structures into an array for sorting
 
       pxSyntax = (int *)malloc(phtGrammar->uCount * sizeof(pxSyntax[0]));
       globppStructures = (PwdStructure **)malloc(phtGrammar->uCount * sizeof(globppStructures[0]));
 
       for ( nStructures = b = 0; b < phtGrammar->nBuckets; ++b )
       {
              for ( pEl = phtGrammar->ppChain[b]; pEl != NULL; pEl = pEl->pNext )
              {
                     globppStructures[nStructures] = (PwdStructure *)pEl->pData;
                     pxSyntax[nStructures] = nStructures;
                     ++nStructures;
              }
       }

#ifdef PRINT_DEBUG
	   printf("nStructures = %d ; uCount = %d\n", nStructures, phtGrammar->uCount);
#endif
       qsort(pxSyntax, nStructures, sizeof(pxSyntax[0]), SortStructureDescendingIndirectUsingIndex);
#ifdef PRINT_DEBUG
	   printf("Most common structure is %s (%.6lf)\n", globppStructures[pxSyntax[0]]->szNonTerminals, globppStructures[pxSyntax[0]]->dPercentage);
#endif
 
       // Split the load across 10 different processors

#ifdef PRINT_DEBUG
	   jStart = jEnd = 0;
	   dCumProb = 1.0;
#else
       for ( dCumProb = 0.0, j = jStart = jEnd = 0; j < nStructures; ++j )
       {
              dCumProb += globppStructures[pxSyntax[j]]->dPercentage;
 
              if ( dCumProb > 0.10 )
              {
                     jEnd = j+1;
                     if ( !fork() ) break;
                     jStart = jEnd;
                     dCumProb = 0.0;
                     ++globintProcessor;
              }
       }
#endif
 
       if ( jStart == jEnd )
       {
              jEnd = nStructures;
       }
 
	   //printf( "%c - %d to %d - %.4lf\n", globintProcessor, jStart, jEnd-1, dCumProb ); return;

       for ( j = jStart; j < jEnd; ++j )
       {
              xGrammar = pxSyntax[j];

              if ( globppStructures[xGrammar]->dPercentage < dThresholdP ) break;
 
              BuildPassword(szBuf, 0, globppStructures[xGrammar]->szNonTerminals,
                            nGrams, NChars, nModReduce, axLetterSingle, pdLetterSingle, pxStartLetterCombo, pdStartLetterCombo, paxStateLetterCombo, padStateLetterCombo,
                            axStartDigit,  pdStartDigit, aaxStateDigit, padStateDigit,
                            axStartSymbol, pdStartSymbol, aaxStateSymbol, padStateSymbol, szSymbols,
                            globppStructures[xGrammar]->dPercentage, dThresholdP, phtPasswords, &nBillions, &nGuesses);
       }
 
       DestroyHashTable(phtPasswords);
 
       free(pxStartLetterCombo);
       free(paxStateLetterCombo);
       free(pxSyntax);
       free(globppStructures);
}
 
 

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
	printf("p(%s) = %lf\n", syntax.szNonTerminals, dTemp);
#endif

	return dNGuesses;
}



int main(int argc, char **argv)
{
	char	szBuf[2048], szSymbols[N_SYMBOLS+2];
	int		iMakeGuesses = 0;
	int		nGrams, i, j, xFile, NLetters, xRow, NModReduce, nPwdChars, xState, nPCFG;
    double	dTemp, dTotalLetters = 0, dTotalDigits = 0, dTotalSymbols = 0, dTotalGrammars = 0, dNGuesses;
	double	*pdLetterPrefix, *pdLetterStart, (*padLetters)[N_LETTERS], *pdDigitPrefix, *pdDigitStart, (*padDigits)[N_DIGITS], *pdSymbolPrefix, *pdSymbolStart, (*padSymbols)[N_SYMBOLS], adSingleLetter[N_LETTERS];
	FILE	*fp;
	PwdStructure		syntax, *pSyntax;
    HashTable			*phtGrammar = CreateHashTable(177787, sizeof(syntax));
    unsigned             xBucket;
    HashTable_Element    *pEl;


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

	for ( xFile = 3; xFile < argc; ++xFile )
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

	fp = fopen(argv[2], "r");
	if ( fp == NULL )
	{
		fprintf( stderr, "%s - can't open %s for reading\n", argv[0], argv[2]);
		exit(0);
	}
 
	/* Either make guesses or compute guess numbers */

	if ( iMakeGuesses )
	{
		ThresholdGuessing(fp, nGrams, NLetters, NModReduce, phtGrammar,
							adSingleLetter, pdLetterStart, padLetters,
                            pdDigitStart, padDigits,
                            pdSymbolStart, padSymbols, szSymbols,
                            1e-13);
	}
	else
	{
		while ( fgets(szBuf, sizeof(szBuf), fp) )
		{
			nPwdChars = strlen(szBuf) - 1;
			if (nPwdChars > MAX_LENGTH) continue;

			dNGuesses = GuessNumber(szBuf, nGrams, NModReduce,
									pdLetterStart, padLetters, adSingleLetter,
									pdDigitStart, padDigits,
									pdSymbolStart, padSymbols, szSymbols,
									phtGrammar, dTotalGrammars
									);
			 
			printf( "%9.3lf\t%s", dNGuesses, szBuf );
#ifdef PRINT_DEBUG
			//getchar();
#endif
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
