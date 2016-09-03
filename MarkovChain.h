#ifndef MARKOVCHAIN_H_INCLUDED
#define MARKOVCHAIN_H_INCLUDED

#define _CRT_SECURE_NO_WARNINGS


#define TRY_DICTIONARY_ATTACKS
//#define TRY_DELETING_1


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>

#define N_CHARS		95	// == '~' - ' ' + 1

#define MAX_LENGTH	23

#define N_LETTERS	26
#define N_DIGITS	10
#define N_SYMBOLS	33


#include "chash.h"


class CBaseStructure
{
public:
	CBaseStructure();

	char	m_szWord[MAX_LENGTH+1];
	int		m_nCount;
	CBaseStructure *m_pNext;
};



class CMarkovChain
{
public:
	int		m_nChars;
	int		m_nGrams;
	int		m_nPrefix;
	int		m_nReduceModulus;

	int		m_nBillions;
	int		m_nGuesses;
	int		m_nProcessor;

	double	m_adLengthProb[1+MAX_LENGTH];

	bool	m_bAreProbabilities;

	double	m_dStartCount;
	double	*m_pdTransitionCount;

	double	*m_pdTransitionProb;
	int		*m_pxTransitionProb;

	double	getTransitionProbability(int i, int j)				{ return m_pdTransitionProb[i * m_nChars + j]; }
	void	increaseTransitionCount(int i, int j, double n)		{ m_pdTransitionProb[i * m_nChars + j] += n; }
	void	setTransitionProbability(int i, int j, double p)	{ m_pdTransitionProb[i * m_nChars + j] = p; }

	int		getTransitionProbabilityIndex(int i, int j)			{ return m_pxTransitionProb[i * m_nChars + j]; }
	void	setTransitionProbabilityIndex(int i, int j, int x)	{ m_pxTransitionProb[i * m_nChars + j] = x; }

	double	*m_pdStartProb;
	int		*m_pxStartProb;

	virtual int ToCharacter(int x)	{ return x + ' '; }
	virtual int ToIndex(int ch)		{ return ch - ' '; }

	char	*m_pszCharacters;

	CHashTable	*m_phtPasswords;

	char	m_szGuessing[1+MAX_LENGTH];
	int		m_nLengthToGuess;

	double	m_dThresholdP;
public:
	CMarkovChain(int nChars, int nGrams);
	virtual ~CMarkovChain();

	int			m_anUniqueWords[MAX_LENGTH+1];
	int			m_anWordsSeen[MAX_LENGTH+1];
	CBaseStructure	*m_apWordLists[MAX_LENGTH+1];
	CHashTable	*m_phtDictionary;

	void	AddToDictionary(const char *szWord, int nTimes = 1);
	void	AddStartCount(int xPrefix, int n);
	void	AddTransitionCount(int xFrom, int xTo, int n);
	void	AddOneTransition(const char *achFrom, char chTo);
	void	ProcessWord(const char *szWord);

	void	BuildPassword(int xPwd, int xOriginalState, double dProbSoFar);

	int		LoadFromFile(const char *szPath);
	int		LoadFromFile(FILE *fp, bool bReadLengths = false, bool bUseDictionaries = true);

	void	ConvertCountsToProb(bool bSmoothProbabilities, bool bDoLengths = true);
	int		LoadDictionary(const char *szPathDictionary);

	void	ThresholdGuessing();
	double	GuessNumber(const char *szPassword, bool bUseDictionary = true);

	void	WriteToTextFile(const char *szPath);
	void	WriteToTextFile(FILE *fp, bool bWriteLengths = false);
};





class CMarkovChainAll : public CMarkovChain
{
public:
	CMarkovChainAll(int nGrams);
};



class CMarkovChainWords : public CMarkovChain
{
public:
	CMarkovChainWords(int nGrams);

	int ToCharacter(int x)	{ return (x == N_LETTERS) ? '.' : (x + 'a'); }
	int ToIndex(int ch)		{ return isalpha(ch) ? (isupper(ch) ? (ch - 'A') : (ch - 'a')) : N_LETTERS; }

	void	BuildAWord(char *szNewWord, int nMaxChar);
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
