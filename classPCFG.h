#include "MarkovChain.h"


#define IS_ALPHA	'A'
#define IS_UPPER    'U'                                 
#define IS_LOWER    'L'
#define IS_DIGIT    'D'
#define IS_SYMBOL	'S'

class CPCFG 
{
private:
	CMarkovChainAlpha	*m_pmcAlphaN;
	CMarkovChainAlpha	*m_pmcAlpha1;
	CMarkovChainDigit	*m_pmcDigits;
	CMarkovChainSymbol	*m_pmcSymbols;

	bool	m_bAreProbabilities;

	int		m_nStructures;	// non-unique: sum of counts of all structures seen
	int		m_nCaseMasks;
	int		m_anCaseMaskLength[1+MAX_LENGTH];
	//int		m_anAlphaStringLengths[1+MAX_LENGTH];
	//int		m_anDigitStringLengths[1+MAX_LENGTH];
	//int		m_anSymbolStringLengths[1+MAX_LENGTH];
	//int		m_anMixedStringLengths[1+MAX_LENGTH];

	//CHashTable	*m_phtDictionary;
	CHashTable	*m_phtStructures;
	CHashTable	*m_phtCaseMask;

	CBaseStructure	*m_apStructures;

	int		m_nBillions;
	int		m_nGuesses;
	int		m_nGuessed;
	int		m_nProcessor;

    void	AddCharacters(int nToAdd, int xOriginalState, char *szCharClass, int xPwd, char *szSyntax, CMarkovChain *pmc, double dProbSoFar);
	void	BuildPassword(int xPwd, char *szNonTerminals, double dProbSoFar);

public:
	CPCFG(int nGrams = 0);
	~CPCFG();

	int		nGrams() const { return m_pmcAlphaN->m_nGrams; }
	void	ProcessWord(const char *szWord);
	void	AddCaseMask(const char *szMask);
	int		LoadFromFile(const char *szPath, int nGrams, bool bUseDictionaries = true);
	int		LoadDictionary(const char *szPathDictionary);
	int		Guessed() const { return m_nGuessed; }
	//void	AddWordToDictionary(char *szWord);
	void	ConvertCountsToProb(bool bSmoothProbabilities);
	void	ThresholdGuessing();
	double	GuessNumber(const char *szPassword);

	CHashTable	*m_phtPasswords;

	char	m_szGuessing[1+MAX_LENGTH];
	int		m_nLengthToGuess;

	double	m_dThresholdP;

	void	WriteToTextFile(const char *szPath);
};


