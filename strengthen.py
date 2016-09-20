#!/usr/bin/env python
import sys
import random

#letterOptions = ['|', '}', '{', '>', '`', '^', '[', ':', '~', '%', ']', '<', '"', ';', "'", '(', 'Q', '?', ')', '=', '&', '+', '\\', ',', 'X', '$', '#', '/', 'Z', 'W', 'V', 'F', '@', 'J', '*', 'G', 'P', '-', 'U', '!', 'K', 'Y', 'H', 'q', 'B', '_', 'D', 'C', 'T', '.', 'M', 'S', 'N', 'R', 'O', 'L', 'I', 'E', 'x', 'A', 'z', 'w', 'f', 'v', 'j']

def replace(word, replacements, r):
	toReplace = r.sample(xrange(len(word)-1), replacements)
	for i in toReplace:
		word = word[:i] + r.choice(letterOptions) + word[i+1:]
	return word

def main():
	with open(sys.argv[2], 'w') as w:
		with open(sys.argv[1], 'r') as f:
			r = random.Random()
			for line in f.readlines():
				w.write(replace(line, 3, r))
if __name__ == "__main__":	
    letterOptions = ['|', '}', '{', '>', '`', '^', '[', ':', '~', '%', ']', '<', '"', ';', "'", '(', 'Q', '?', ')', '=', '&', '+', '\\', ',', 'X', '$', '#', '/', 'Z', 'W', 'V', 'F', '@', 'J', '*', 'G', 'P', '-', 'U', '!', 'K', 'Y', 'H', 'q', 'B', '_', 'D', 'C', 'T', '.', 'M', 'S', 'N', 'R', 'O', 'L', 'I', 'E', 'x', 'A', 'z', 'w', 'f', 'v', 'j']
    main()
