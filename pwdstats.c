#include <stdlib.h>
#include <stdio.h>


#define MAX_LENGTH	32
#define MAX_N		(1 << 24)	// 16 million, give or take


int SortDoubleAscending(const void *a, const void *b)
{
	double	p = *(const double *)a;
	double	q = *(const double *)b;

	return (p < q) ? -1 : (p != q);
}


int main(int argc, char **argv)
{
	char	szBuf[2048];
	double	*pdGuessNumbers = malloc(sizeof(double) * MAX_N);
	int		nNumbers = 0, i;


	while ( gets(szBuf) )
	{
		if ( sscanf(szBuf, "%lf", pdGuessNumbers+nNumbers) == 1 ) 
			++nNumbers;
	}

	qsort(pdGuessNumbers, nNumbers, sizeof(*pdGuessNumbers), SortDoubleAscending);

	printf( "%d\tN\n", nNumbers);
	printf( "%.3lg\tMax\n", pdGuessNumbers[nNumbers-1] );
	for ( i = 9; i; --i )
	{
		double dTemp = pdGuessNumbers[(int)(nNumbers * i/10.0)];
	
		printf( (dTemp >= 1e+6) ? "%.3le" : "%.3lf", dTemp );
		if ( i == 5 ) puts("\tmedian");
		else printf("\t%dth ptile\n", i*10 );
	}
	printf( "%.3lg\tMin\n", pdGuessNumbers[0] );

	free(pdGuessNumbers);

	return 0;
}
