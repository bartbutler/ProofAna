import sys
import os
import pwd
import re
import math
import operator

from ROOT import TFile,TKey,TClass,TDirectory,gROOT,TH1D

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

class Cut(object):
	def __init__(self, name):
		self.name = name
		self.n = 0
		self.nerror = 0
		self.nwgt = 0
		self.nwgterror = 0
	def add(self, cut):
		if(self.name != cut.name):
			print "Error: adding two cuts with different names, exiting."
			sys.exit(1)
		self.n += cut.n
		self.nerror = math.sqrt(self.nerror*self.nerror + cut.nerror*cut.nerror)
		self.nwgt += cut.nwgt
		self.nwgterror = math.sqrt(self.nwgterror*self.nwgterror + cut.nwgterror*cut.nwgterror)
	def __lt__(self, other):
		return self.n > other.n

if len(sys.argv) < 2:
	print "Usage: python printCutflow.py file [analysis1] [analysis2] ... [--latex] [--accept \"regexp1\" ] ... [--exclude \"regexp2\"] ..."
	print "Usage: python printCutflow.py file1 file2 ... [analysis1] [analysis2] ... [--latex]  [--accept \"regexp1\" ] ... [--exclude \"regexp2\"] ..."
	print "Usage: python printCutflow.py filepattern [analysis1] [analysis2] ... [--latex]  [--accept \"regexp1\" ] ... [--exclude \"regexp2\"] ..."
	print "All non-files supplied as arguments (unless preceded by --accept or --exclude) will be treated as analysis names."
	print "Files can be ROOT files or text files (with .txt extension) containing lists of ROOT files (file lists)."
	print "Other options are --noentries, --noabseff, --noreleff, --noweights, --noraw, and --noerrors, only for use with --latex."
	sys.exit(0)

files = [ ]
analyses = { }
accepts = [ ]
excludes = [ ]

doLatex = False
noEntries = False
noAbsEff = False
noRelEff = False
noWeights = False
noRaw = False
noErrors = False

skipnext = True
for index, arg in enumerate(sys.argv):
	if skipnext:
		skipnext = False
		continue
	if arg=="--accept":
		if index==(len(sys.argv)-1):
			print "Error: dangling --accept directive."
			sys.exit(1)
		try:
			accepts.append(re.compile("^"+sys.argv[index+1]+"$"))
		except re.error:
			print "Error: invalid regular expression",sys.argv[index+1]
			sys.exit(1)
		skipnext = True
	elif arg=="--exclude":
		if index==(len(sys.argv)-1):
			print "Error: dangling --exclude directive."
			sys.exit(1)
		try:
			excludes.append(re.compile("^"+sys.argv[index+1]+"$"))
		except re.error:
			print "Error: invalid regular expression",sys.argv[index+1]
			sys.exit(1)
		skipnext = True
	elif arg=="--latex":
		doLatex = True
	elif arg=="--noentries":
		noEntries = True
	elif arg=="--noabseff":
		noAbsEff = True
	elif arg=="--noreleff":
		noRelEff = True
	elif arg=="--noweights":
		noWeights = True
	elif arg=="--noraw":
		noRaw = True
	elif arg=="--noerrors":
		noErrors = True
	elif os.path.isfile(arg):
		files.append(arg)
	else:
		analyses[arg] = { }

allAnalyses = False
if(len(analyses)==0):
	allAnalyses = True

if len(files)==0:
	print "Error: no arguments correspond to existing files."
	sys.exit(1)

alleventsre = re.compile("^cutflow$")
cutflowre = re.compile("(.*)_cutflow$")

def printCutflow(name,cuts):
	# convert to list, sort by nentries
	mylist = [ ]
	for cutnames, cut in cuts.items():
		mylist.append(cut)

	mylist.sort()

	ncol = 1
	if not noEntries:
		if not noRaw:
			ncol += 1
		if not noWeights:
			ncol += 1
	if not noAbsEff:
		if not noRaw:
			ncol += 1
		if not noWeights:
			ncol += 1
	if not noRelEff:
		if not noRaw:
			ncol += 1
		if not noWeights:
			ncol += 1

	if doLatex:
		name = name.replace("_","\_")
		print "\\begin{table}"
		print "\\footnotesize"
		print "\\begin{center}\\renewcommand\\arraystretch{1.6}"
		str = "\\begin{tabular}{|c|"
		for i in xrange(1,ncol):
			if i==1:
				str += "|"
			str += "c|"
		str += "}\hline"
		print str
		print "\multicolumn{%d}{|c|}{%s Cut Flow} \\\\ \hline" % (ncol, name)
		str = "Cut"
		if not noEntries:
			if not noRaw:
				str += " & Entries"
			if not noWeights:
				str += " & Weighted Entries"
		if not noAbsEff:
			if not noRaw:
				str += " & Abs. Eff."
			if not noWeights:
				str += "& W. Abs. Eff."
		if not noRelEff:
			if not noRaw:
				str += "& Rel. Eff."
			if not noWeights:
				str += "& W. Rel. Eff."
		str += "\\\\ \hline\hline"
		print str
	else:
		print "Analysis",name,"cut flow:"
		print "********************************************************************************************************************************************************"
		print "* Cut                                  | Entries                | Weighted Entries               | Abs. Eff. | W. Abs. Eff. | Rel. Eff. | W. Rel. Eff. *"
		print "*------------------------------------------------------------------------------------------------------------------------------------------------------*"

	ntotal = mylist[0].n
	nwgttotal = mylist[0].nwgt
	prevntotal = mylist[0].n
	prevnwgttotal = mylist[0].nwgt

	percent = "%"
	pm = "+/-"
	if doLatex:
		percent = "\%"
		pm = "$\pm$"

	for cut in mylist:
		first = cut.name
		if doLatex:
			first = cut.name.replace("_","\_")
		second = '%.0f' % cut.n
		if not noErrors:
			err = " %s %.1f" % (pm, cut.nerror)
			second += err
		third = '%.2f' % cut.nwgt
		if not noErrors:
			err =  " %s %.2f" % (pm, cut.nwgterror)
			third += err
		abseff = 100.*cut.n/ntotal
		fourth = '%.1f%s' % (abseff, percent)
		wabseff = 100.*cut.nwgt/nwgttotal
		fifth = '%.1f%s' % (wabseff, percent)
		releff = 100.*cut.n/prevntotal
		sixth = '%.1f%s' % (releff, percent)
		wreleff = 100.*cut.nwgt/prevnwgttotal
		seventh = '%.1f%s' % (wreleff, percent)
		if doLatex:
			str = first
			if not noEntries:
				if not noRaw:
					str += " & "
					str += second
				if not noWeights:
					str += " & "
					str += third
			if not noAbsEff:
				if not noRaw:
					str += " & "
					str += fourth
				if not noWeights:
					str += " & "
					str += fifth
			if not noRelEff:
				if not noRaw:
					str += " & "
					str += sixth
				if not noWeights:
					str += " & "
					str += seventh
			str += "\\\\"
			print str
		else:
			print "* %-36s | %22s | %30s | %9s | %12s | %9s | %12s *" % (first, second, third, fourth, fifth, sixth, seventh)
		prevntotal = cut.n
		prevnwgttotal = cut.nwgt

	if doLatex:
		print "\hline"
		print "\end{tabular}"
		str = "\caption{Cut flow for the %s analysis.}\label{tab:%s}" % (name, name)
		print str
		print "\end{center}"
		print "\end{table}"
	else:
		print "********************************************************************************************************************************************************"
	print ""

def readCutflow(dir,mydict):
	for key in dir.GetListOfKeys():
		classname = key.GetClassName()
		cl = gROOT.GetClass(classname)
		if not cl:
			continue

		histoname = key.GetName()
		name = "All Events"
		if not cutflowre.search(histoname):
			if not alleventsre.search(histoname):
				continue
		else:
			name = cutflowre.findall(histoname)[0]

		# check accept and exclude regexps
		accept = True
		if len(accepts):
			accept = False

		for regexp in accepts:
			if regexp.search(name):
				accept = True
				break

		exclude = False
		for regexp in excludes:
			if regexp.search(name):
				exclude = True
				break

		if not accept or exclude:
			continue

		if not cl.InheritsFrom(TH1D.Class()):
			continue

		histo = dir.Get(histoname)

		mycut = Cut(name)
		mycut.n = histo.GetEntries()
		mycut.nerror = math.sqrt(histo.GetEntries())
		mycut.nwgt = histo.GetBinContent(1)
		mycut.nwgterror = histo.GetBinError(1)

		if mydict.has_key(name):
			mydict[name].add(mycut)
		else:
			mydict[name] = mycut

txtre = re.compile(".*\.txt$")

for file in files:

	if txtre.search(file):
		infile = open(file,"r")
		while (infile):
			line = infile.readline().rstrip()
			n = len(line)
			if (n==0):
				break

			if os.path.isfile(line):
				files.append(line)

		infile.close()
		continue

	tfile = TFile.Open(file,"READ")

	if not tfile.IsOpen():
		print "Error: File",file,"exists but cannot be opened."
		sys.exit(1)

	#loop on all analysis TDirectoryFiles in this directory
	for key in tfile.GetListOfKeys():
		classname = key.GetClassName()
		cl = gROOT.GetClass(classname)
		if not cl:
			continue
		if cl.InheritsFrom(TDirectory.Class()):
			subdir = tfile.Get(key.GetName())
			if not allAnalyses:
				if analyses.has_key(key.GetName()):
					readCutflow(subdir,analyses[key.GetName()])
			else:
				if not analyses.has_key(key.GetName()):
					analyses[key.GetName()] = { }
				readCutflow(subdir,analyses[key.GetName()])

	tfile.Close()

printsomething = False

for ana, cuts in analyses.items():
	if len(cuts)==0: # not an analysis TDirectoryFile apparently
		continue
	printsomething = True
	printCutflow(ana,cuts)

if not printsomething:
	print "Error: No valid cutflows found matching input criteria."
	print "Analysis names:"
	for name in analyses.keys():
		print "	", name
	print "Accept regexps (OR'ed):"
	for regexp in accepts:
		print "	", regexp.pattern
	print "Exclude regexps (AND'ed):"
	for regexp in excludes:
		print "	", regexp.pattern
