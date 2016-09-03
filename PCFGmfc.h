#include "MarkovChain.h"



#define IS_UPPER    'U'                                 
#define IS_LOWER    'L'
#define IS_DIGIT    'D'
#define IS_SYMBOL	'S'


typedef struct
{
	char	szNonTerminals[2*MAX_LENGTH+1];
    double	dPercentage;
} PwdStructure;


class CPCFG 
{
private:
	CMarkovChainAlpha	*m_pmcAlphaN;
	CMarkovChainAlpha	*m_pmcAlpha1;
	CMarkovChainDigit	*m_pmcDigits;
	CMarkovChainSymbol	*m_pmcSymbols;

	int		m_nStructures;

	double		m_dAvgWordLength;
	int			m_anWordLengths[MAX_LENGTH+1];
	int			m_anAndShorter[MAX_LENGTH+1];

	CHashTable	*m_phtDictionary;
	CHashTable	*m_phtStructures;

	int		m_nBillions;
	int		m_nGuesses;
	int		m_nProcessor;

    void	AddCharacters(int nToAdd, int xOriginalState, char *szCharClass, int xPwd, char *szSyntax, CMarkovChain *pmc, double dProbSoFar);
	void	BuildPassword(int xPwd, char *szNonTerminals, double dProbSoFar);

public:
	CPCFG();
	~CPCFG();

	int		LoadFromFile(char *szPath, int nGrams);
	int		LoadDictionary(char *szPathDictionary);
	void	ConvertCountsToProb();
	void	ThresholdGuessing();
	double	GuessNumber(char *szPassword);

	CHashTable	*m_phtPasswords;

	char	m_szGuessing[1+MAX_LENGTH];
	int		m_nLengthToGuess;

	double	m_dThresholdP;
};


