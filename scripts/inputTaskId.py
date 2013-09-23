import sys
import os
import re

try:
	from ROOT import *
except:
	print "Failed to import ROOT. Are the ROOT environment variables set?"
	sys.exit(1)

# default dict emulation for python 2.4
try:
    from collections import defaultdict
except:
    class defaultdict(dict):
        def __init__(self, default_factory=None, *a, **kw):
            if (default_factory is not None and
                not hasattr(default_factory, '__call__')):
                raise TypeError('first argument must be callable')
            dict.__init__(self, *a, **kw)
            self.default_factory = default_factory
        def __getitem__(self, key):
            try:
                return dict.__getitem__(self, key)
            except KeyError:
                return self.__missing__(key)
        def __missing__(self, key):
            if self.default_factory is None:
                raise KeyError(key)
            self[key] = value = self.default_factory()
            return value
        def __reduce__(self):
            if self.default_factory is None:
                args = tuple()
            else:
                args = self.default_factory,
            return type(self), args, None, None, self.items()
        def copy(self):
            return self.__copy__()
        def __copy__(self):
            return type(self)(self.default_factory, self)
        def __deepcopy__(self, memo):
            import copy
            return type(self)(self.default_factory,copy.deepcopy(self.items()))
        def __repr__(self):
            return 'defaultdict(%s, %s)' % (self.default_factory,dict.__repr__(self))

if(len(sys.argv)<2):
	print "Usage: python inputTaskId.py input_file_list.txt"
	sys.exit(1)

infilelist = sys.argv[1]

flre = re.compile(".*\.txt$")
if not flre.search(infilelist):
	print "File list must end in .txt"
	sys.exit(1)

outfilelist = infilelist[:-4] + ".tid.txt"

infile = 0
try:
	infile = open(infilelist,"r")
except:
	print "Cannot open input file list", infilelist
	sys.exit(1)
	
outfile = 0
try:
	outfile = open(outfilelist,"w")
except:
	print "Cannot open output file list", infilelist
	sys.exit(1)

tidre = re.compile(".*\.0*([0-9]*).*\._[0-9]*.*\.root.*")

while (infile):
	line = infile.readline().rstrip()
	n = len(line)
	if (n==0):
		break

	tids = defaultdict()
	tfile = TFile(line,"READ")
	
	if not tfile.IsOpen():
		print "Cannot open ROOT file", line
		continue
		
	dir = tfile.Get("JobInfo")
	if dir == None:
		print "Cannot find JobInfo directory in", line
		continue
	
	for key in dir.GetListOfKeys():
		classname = key.GetClassName()
		cl = gROOT.GetClass(classname)
		
		if not cl:
			continue

		if classname != "TNamed":
			continue

		if not tidre.search(key.GetName()):
			print key.GetName(), "does not match task ID regexp"
			continue
		else:
			tids[int(tidre.findall(key.GetName())[0])] = 1 #dummy var

	tfile.Close()
	for tid in tids.keys():
		add = " %d" % tid
		line += add
	line += "\n"
	outfile.write(line)

outfile.close()
infile.close()

print "Finished writing to:",outfilelist
