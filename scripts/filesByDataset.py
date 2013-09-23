import sys
import os
import re
import subprocess

try:
    from collections import defaultdict
except:
	print "You need to be running python 2.6 or later for this script and for dq2-ls"
	sys.exit(1)

class Unbuffered:
   def __init__(self, stream):
       self.stream = stream
   def write(self, data):
       self.stream.write(data)
       self.stream.flush()
   def __getattr__(self, attr):
       return getattr(self.stream, attr)
            
def which(name, flags=os.X_OK):
	result = []
	exts = filter(None, os.environ.get('PATHEXT', '').split(os.pathsep))
	path = os.environ.get('PATH', None)
	if path is None:
		return []
	for p in os.environ.get('PATH', '').split(os.pathsep):
		p = os.path.join(p, name)
		if os.access(p, flags):
			result.append(p)
		for e in exts:
			pext = p + e
			if os.access(pext, flags):
				result.append(pext)
	return result

if(len(sys.argv)<2):
	print "Usage: python filesByDataset.py input_file_list_w_taskids.txt"
	sys.exit(1)
	
if len(which("dq2-ls")) == 0:
	print "dq2-ls not found in path! Have you loaded Panda tools?"
	sys.exit(1)

infilelist = sys.argv[1]

flre = re.compile(".*\.tid\.txt$")
if not flre.search(infilelist):
	print "File list must end in .tid.txt"
	sys.exit(1)

inconfig = infilelist[:-8] + ".config"

infile = 0
try:
	infile = open(infilelist,"r")
except:
	print "Cannot open input file list", infilelist
	sys.exit(1)
	
incfgfile = 0
try:
	incfgfile = open(inconfig,"r")
except:
	print "Cannot open input config file", inconfig
	sys.exit(1)

datasets = defaultdict()

dsre = re.compile("^name_.*=(.*)$")
tidre = re.compile(".*tid0*([0-9]*).*")

while (incfgfile):
	line = incfgfile.readline().rstrip()
	n = len(line)
	if (n==0):
		break

	if not dsre.search(line):
		continue

	ds = dsre.findall(line)[0]
	dsname = ds.rstrip("/")
	if not datasets.has_key(dsname):
		datasets[dsname] = []
	
	cmd = ["dq2-ls","-r",ds]
	p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
	stream = Unbuffered(p.stdout)
	lines = stream.readlines()
	
	for entry in lines:
		if not tidre.search(entry):
			continue
		tid = tidre.findall(entry)[0]
		datasets[dsname].append(tid)
		print "Task ID", tid, "associated with dataset", ds
		
	print "Done with dataset", ds

incfgfile.close()

#invert tid/dataset relationship
tids = defaultdict()
for key, list in datasets.items():
	dsname = key.rstrip("/")
	for tid in list:
		if tids.has_key(tid):
			if dsname != tids[tid]:
				print "Task ID", tid, "is associated to multiple datasets"
				sys.exit(1)
		else:
			tids[tid] = dsname
				
outputlists = defaultdict()

while (infile):
	line = infile.readline().rstrip()
	n = len(line)
	if (n==0):
		break

	args = line.split(" ")
	if len(args)<2:
		print "File", args[0], "has no associated task ID."
		print "Please run the inputTaskId.py script on this file list before running this script."
		sys.exit(1)
	
	dsname = ""
	
	first = True
	for arg in args:
		if first:
			first = False
			continue
		if not tids.has_key(arg):
			print "Unknown task ID:",arg
			print "Are you using the complete config file?"
			sys.exit(1)

		ds = tids[arg]
			
		if dsname == "":
			dsname = ds
		elif dsname != ds:
			print "File with task IDs from multiple datasets:", args[0]
			sys.exit(1)			
	
	if not outputlists.has_key(ds):
		filename = dsname + ".txt"
		outputlists[dsname] = open(filename,"w")
			
	out = args[0] + "\n"
	outputlists[dsname].write(out)

infile.close()

for key, file in outputlists.items():
	print "Finished writing file list for dataset ", key
	file.close()

