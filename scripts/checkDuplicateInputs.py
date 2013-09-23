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
	print "Usage: python checkDuplicateInputs.py input_file_list.txt"
	print "Usage: python checkDuplicateInputs.py input_file.root"
	sys.exit(1)

infilelist = sys.argv[1]

files = [ ]

inputslist = infilelist[:-4] + ".inputs.txt"
isRoot = False
flre = re.compile(".*\.txt$")
if not flre.search(infilelist):
	isRoot = True
	files.append(infilelist)
	inputslist = infilelist + ".inputs.txt"

if not isRoot:
	infile = 0
	try:
		infile = open(infilelist,"r")
	except:
		print "Cannot open input file list", infilelist
		sys.exit(1)

	while (infile):
		line = infile.readline().rstrip()
		n = len(line)
		if (n==0):
			break
		files.append(line)

	infile.close()

filere = re.compile("^(.*) Evts .*$")
intervalre = re.compile("^.* Evts (.*)$")
lowre = re.compile("([0-9]*)-[0-9]*")
highre = re.compile("[0-9]*-([0-9]*)")

inputs = defaultdict()
total = 0

for file in files:

	tfile = TFile(file,"READ")

	if not tfile.IsOpen():
		print "Cannot open ROOT file", file
		continue

	dir = tfile.Get("JobInfo")
	if dir == None:
		print "Cannot find JobInfo directory in", file
		continue

	for key in dir.GetListOfKeys():
		classname = key.GetClassName()
		cl = gROOT.GetClass(classname)

		if not cl:
			continue

		if classname != "TNamed":
			continue

		if not filere.search(key.GetName()):
			print key.GetName(), "does not match regexp"
			continue

		filename = filere.findall(key.GetName())[0]
		intervals = intervalre.findall(key.GetName())[0]
		lows = lowre.findall(intervals)
		highs = highre.findall(intervals)

		if not inputs.has_key(filename):
			inputs[filename] = [ ]

		for index in xrange(len(lows)):
			total = total + int(highs[index]) - int(lows[index]) + 1
			inputs[filename].append( ( int(lows[index]) , int(highs[index]) ) )

	tfile.Close()

keylist = sorted(inputs.iterkeys())
inputsfile = open(inputslist,"w")
for key in keylist:
	filename = "%s\n" % key
	inputsfile.write(filename)
	list = inputs[key]
	list.sort()
	intervals = [ ]
	index = 0
	for tuple in list:
		if len(intervals) == 0:
			intervals.append(tuple)
			continue
		if tuple[0] == (intervals[index][1]+1):
			intervals[index]= (intervals[index][0] , tuple[1])
		else:
			index = index + 1
			intervals.append(tuple)

	if len(intervals) != 1:
		print "Missing or duplicate events from input file", key
		print "Processed intervals are:"
		for interval in intervals:
			out = "	Start event %d, end event %d" % interval
			print out

inputsfile.close()

print total, "events processed."
