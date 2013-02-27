#Generate all the combinations of a set of words.
#This was written to help facilitate automatic compilation of files with
#lots of #defines. The idea is to -D define each possible combination, and
#compile it into a special folder. To do this, we first need a list of each
#possible combination. That's what this does: it generates that list.

#expects one argument: the name of the file with the defines on it.
#File should be of the following format:
#    #define DEF1
#    //#define DEF2
#    #define DEF3
#    ...
#
#    Nothing else (comments, etc). should be in the file. A define can be
#    commented out.

# Written by SRLM on 2013-02-26

import itertools
import sys
lines = [i.split(" ", 1)[1].strip() for i in open(sys.argv[1]).readlines()]
options =[]
[options.extend(list(itertools.combinations(lines, r))) for r in range(len(lines)+1)]

for item in options:
	for i in item:
		print i,
	print
