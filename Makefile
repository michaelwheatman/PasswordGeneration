#
# File          : Makefile


# Environment Setup
LIBDIRS=-L. -L/usr/local/lib -L/usr/lib64 
INCLUDES=-I. -I/usr/local/include
CC=gcc 
CFLAGS=-c $(INCLUDES) -g -Wall
LINK=g++ -g
LDFLAGS=$(LIBDIRS) 
AR=ar rc
RANLIB=ranlib

# Suffix rules
.cpp.o :
	${CC} ${CFLAGS} $< -o $@
.c.o :
	${CC} ${CFLAGS} $< -o $@



mangler2 : mangler2.o hash.o chash.o classPCFG.o MarkovChain.o
	$(LINK) $(LDFLAGS) mangler2.o hash.o chash.o MarkovChain.o classPCFG.o $(LIBS) -o $@

mangler : mangler.o hash.o chash.o
	$(LINK) $(LDFLAGS) mangler.o hash.o chash.o $(LIBS) -o $@

pwdstats : pwdstats.o 
	$(LINK) $(LDFLAGS) pwdstats.o $(LIBS) -o $@

splitpwd : splitpwd.o 
	$(LINK) $(LDFLAGS) splitpwd.o $(LIBS) -o $@



pcfgc : pcfgc.o hash.o chash.o
	$(LINK) $(LDFLAGS) pcfgc.o hash.o $(LIBS) -o $@


pcfgc2 : pcfgc2.o hash.o chash.o MarkovChain.o classPCFG.o
	$(LINK) $(LDFLAGS) pcfgc2.o hash.o chash.o MarkovChain.o classPCFG.o $(LIBS) -o $@

locktest : locktest.o
	$(LINK) $(LDFLAGS) locktest.o $(LIBS) -o $@

mcc2 : mcc2.o chash.o hash.o MarkovChain.o
	$(LINK) $(LDFLAGS) mcc2.o hash.o chash.o MarkovChain.o $(LIBS) -o $@

MarkovCracker : MarkovCracker.o hash.o chash.o
	$(LINK) $(LDFLAGS) MarkovCracker.o hash.o chash.o $(LIBS) -o $@


markov : markov.o hash.o chash.o
	$(LINK) $(LDFLAGS) markov.o hash.o chash.o $(LIBS) -o $@

markov2 : markov2.o hash.o chash.o
	$(LINK) $(LDFLAGS) markov2.o hash.o chash.o $(LIBS) -o $@

BASENAME=p1-crypto
tar: 
	tar cvfz $(BASENAME).tgz -C ..\
	    $(BASENAME)/Makefile \
            $(BASENAME)/cse543-gcrypt.c \
            $(BASENAME)/cse543-gcrypt.h \
	    $(BASENAME)/cse543-util.c \
	    $(BASENAME)/cse543-util.h 

