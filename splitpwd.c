#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv)
{
	char	szBuf[2048];
	FILE	*aafp[5][5];
	int		x, i, j, nMinChars, nClasses, nLower, nUpper, nDigit, nSymbol, nBad;


	srand((unsigned long)time(NULL));

	if ( argc < 3 )
	{
		printf( "usage: %s MinimumLength file1 [file2 ...]\n", argv[0] );
		exit(0);
	}

	nMinChars = atoi(argv[1]);
	if ( nMinChars < 6 || nMinChars > 16 )
	{
		printf( "%s - MinimumLength parameter must be between 6 and 16, inclusive\n", argv[0] );
		exit(0);
	}

	for ( x = 2; x < argc; ++x )
	{
		aafp[0][0] = fopen(argv[x], "r");
		if ( aafp[0][0] == NULL )
		{
			printf( "error opening %s\n", argv[x]);
			continue;
		}

		for ( i = 1; i <= 4; ++i )
		{
			for ( j = 0; j < 5; ++j )
			{
				if ( j == 4 )
				{
					sprintf( szBuf, "%s.%d.%d", argv[x], nMinChars, i );
				}
				else
				{
					sprintf( szBuf, "%s.%d.%d.%c", argv[x], nMinChars, i, j + 'a' );
				}

				aafp[i][j] = fopen(szBuf, "w");
				if ( aafp[i][j] == NULL )
				{
					printf( "%s - can't open %s, bailing out!!\n", argv[0], szBuf );
					exit(0);
				}
			}
		}

		while ( fgets(szBuf, sizeof(szBuf), aafp[0][0]) != NULL )
		{
			nClasses = 0;
			nLower = 0;
			nUpper = 0;
			nDigit = 0;
			nSymbol = 0;
			nBad = 0;

			for ( i = 0; szBuf[i]; ++i )
			{
				if ( szBuf[i] == '\n' )
				{
					break;
				}
				else if ( (szBuf[i] < ' ') || (szBuf[i] & 0x80) ) 
				{
					++nBad;
				}
				else if ( islower(szBuf[i]) )
				{
					nClasses += !nLower;
					++nLower;
				}
				else if ( isupper(szBuf[i]) )
				{
					nClasses += !nUpper;
					++nUpper;
				}
				else if ( isdigit(szBuf[i]) )
				{
					nClasses += !nDigit;
					++nDigit;
				}
				else
				{
					nClasses += !nSymbol;
					++nSymbol;
				}
			}

			if ( !nBad && (nLower + nUpper + nDigit + nSymbol >= nMinChars) )
			{
				for ( i = 1; i < 5; ++i )
				{
					if ( nClasses >= i )
					{
						fprintf(aafp[i][rand() & 3], "%s", szBuf);
						fprintf(aafp[i][4], "%s", szBuf);
					}
				}
			}
		}

		for ( i = 0; i < 5; ++i )
		{
			if ( i )
			{
				for ( j = 0; j < 5; ++j )
				{
					fclose(aafp[i][j]);
				}
			}
			else
			{
				fclose(aafp[i][0]);
			}
		}
	}

	return 0;
}
