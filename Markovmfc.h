#ifndef MARKOVCHAIN_H_INCLUDED
#define MARKOVCHAIN_H_INCLUDED

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


#define N_CHARS		95

#define MAX_LENGTH	32

#define N_LETTERS	26
#define N_DIGITS	10
#define N_SYMBOLS	33


#include "hash.h"


class CMarkovChain
{
public:
	CMarkovChain(int nChars, int nGrams);
	virtual ~CMarkovChain();

	int		m_nChars;
	int		m_nGrams;
	int		m_nPrefix;
	int		m_nReduceModulus;

	int		m_nBillions;
	int		m_nGuesses;
	int		m_nProcessor;

	int		m_nTotalCount;
	double	m_adLengthProb[1+MAX_LENGTH];

	double	m_dStartCount;
	double	*m_pdTransitionCount;

	double	(*m_padTransitionProb)[N_CHARS];
	int		(*m_paxTransitionProb)[N_CHARS];

	double	*m_pdStartProb;
	int		*m_pxStartProb;

	virtual int ToCharacter(int x)	{ return x + ' '; }
	virtual int ToIndex(int ch)		{ return ch - ' '; }

	void	AddStartCount(int xPrefix, int n);
	void	AddTransitionCount(int xFrom, int xTo, int n);

	void	BuildPassword(int xPwd, int xOriginalState, double dProbSoFar);

	int		LoadFromFile(char *szPath);
	int		LoadFromFile(FILE *fp, CMarkovChain *p1 = NULL);
	void	ConvertCountsToProb();
	int		LoadDictionary(char *szPathDictionary);

	void	ThresholdGuessing();
	double	GuessNumber(char *szPassword);

	char	*m_pszCharacters;

	int			*m_pnWordLengths;
	CHashTable	*m_phtDictionary;
	CHashTable	*m_phtPasswords;

	char	m_szGuessing[1+MAX_LENGTH];
	int		m_nLengthToGuess;

	double	m_dThresholdP;
};





class CMarkovChainAll : public CMarkovChain
{
public:
	CMarkovChainAll(int nGrams);
};



class CMarkovChainAlpha : public CMarkovChain
{
public:
	CMarkovChainAlpha(int nGrams);

	int ToCharacter(int x)	{ return x + 'a'; }
	int ToIndex(int ch)		{ return isupper(ch) ? (ch - 'A') : (ch - 'a'); }
};



class CMarkovChainDigit : public CMarkovChain
{
public:
	CMarkovChainDigit();

	int ToCharacter(int x)	{ return x + '0'; }
	int ToIndex(int ch)		{ return ch - '0'; }
};


class CMarkovChainSymbol : public CMarkovChain
{
public:
	CMarkovChainSymbol();

	char	m_szSymbols[1+N_SYMBOLS];

	int ToCharacter(int x)	{ return m_szSymbols[x]; }
	int ToIndex(int ch)		{ return strchr(m_szSymbols, ch) - m_szSymbols; }
};



#endif
