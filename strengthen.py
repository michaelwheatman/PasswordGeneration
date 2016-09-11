#!/usr/bin/env python
import sys
import random

letterOptions = ['!', '#', '$', '%', '&', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '^', '?', '\'', '"']
def getRandomChar(r, prev):
	return r.choice(letterOptions)

def replace(word, replacements, r):
	toReplace = r.sample(xrange(len(word)-1), replacements)
	for i in toReplace:
		#word = word[:i] + getRandomChar(r, word[i-1]) + word[i+1:]
		word = word[:i] + r.choice(letterOptions) + word[i+1:]
	return word

def main():
	with open(sys.argv[2], 'w') as w:
		with open(sys.argv[1], 'r') as f:
			r = random.Random()
			for line in f.readlines():
				w.write(replace(line, 3, r))
if __name__ == "__main__":
	main()
