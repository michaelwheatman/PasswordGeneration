#!/usr/bin/env bash
# Michael Wheatman 2016

# remove existing output files 
rm -f crack-file
rm -f mod-crack-file
rm -f *.pcfg1
rm -f *.mc1

echo "Strengthening Lists"
# My strengthen program takes an an existing file
# and the name of the output file
# You should modify this for however you generate
# stregthened training and test data
strengthen.py rockyou.txt.6.1.a modOutTrain.txt
strengthen.py rockyou.txt.6.1.b modOutTest.txt

echo "Generated Strengthened Lists"
# Train the markov models, we don't need the output
# file name so redirect it to /dev/null
markov 1 modOutTrain.txt > /dev/null
markov 1 rockyou.txt.6.1.a > /dev/null
echo "Generated Markov Models"

# Run the pcfgc program and specify the output files
pcfgc 1 modOutTest.txt modOutTrain.txt.pcfg1 > mod-crack-file
pcfgc 1 rockyou.txt.6.1.b rockyou.txt.6.1.a.pcfg1 > crack-file
echo "Ran PCFGS"

# I'm choosing to show both the modified and unmodified lists'
# cracking data
printf "\nModified Passwords\n"
pwdstats < mod-crack-file
printf "\nUnmodified Passwords\n"
pwdstats < crack-file
