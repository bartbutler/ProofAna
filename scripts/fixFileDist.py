import os.path
import sys
import re
import subprocess

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

tidre = re.compile('.*\.([0-9]*)\._[0-9]*.*\.root.*')
userre = re.compile('.*\.([0-9]*)\..*\._[0-9]*.*\.root.*')
groupre = re.compile('.*\.([0-9]*_[0-9]*)\..*\._[0-9]*.*\.root.*')

def fileDict():
	mydict = { 'atlprf02.slac.stanford.edu': [],\
				'atlprf03.slac.stanford.edu': [],\
				'atlprf04.slac.stanford.edu': [],\
				'atlprf05.slac.stanford.edu': [],\
				'atlprf06.slac.stanford.edu': [],\
				'atlprf07.slac.stanford.edu': [],\
				'atlprf08.slac.stanford.edu': [],\
				'atlprf09.slac.stanford.edu': [],\
				'atlprf10.slac.stanford.edu': [],\
				'atlprf11.slac.stanford.edu': [],\
				'atlprf12.slac.stanford.edu': [],\
				'atlprf13.slac.stanford.edu': [],\
				'atlprf14.slac.stanford.edu': [],\
				'atlprf15.slac.stanford.edu': [],\
				'atlprf16.slac.stanford.edu': [],\
				'atlprf17.slac.stanford.edu': [],\
				'atlprf18.slac.stanford.edu': [],\
				'atlprf19.slac.stanford.edu': [],\
				'atlprf20.slac.stanford.edu': [],\
				'atlprf21.slac.stanford.edu': [],\
				'atlprf22.slac.stanford.edu': [],\
				'atlprf23.slac.stanford.edu': [],\
				'atlprf24.slac.stanford.edu': [],\
				'atlprf25.slac.stanford.edu': [],\
				'atlprf26.slac.stanford.edu': [],\
				'atlprf27.slac.stanford.edu': [],\
				'atlprf28.slac.stanford.edu': [],\
				'atlprf29.slac.stanford.edu': [],\
				'atlprf30.slac.stanford.edu': [],\
				'atlprf31.slac.stanford.edu': [],\
				'atlprf32.slac.stanford.edu': [],\
				'atlprf33.slac.stanford.edu': [],\
				'atlprf34.slac.stanford.edu': [],\
				'atlprf35.slac.stanford.edu': [],\
				'atlprf36.slac.stanford.edu': [] }
	return mydict

def hostDict():
	mydict = { 'atlprf02.slac.stanford.edu': 2,\
		'atlprf03.slac.stanford.edu': 2,\
		'atlprf04.slac.stanford.edu': 2,\
		'atlprf05.slac.stanford.edu': 2,\
		'atlprf06.slac.stanford.edu': 2,\
		'atlprf07.slac.stanford.edu': 2,\
		'atlprf08.slac.stanford.edu': 2,\
		'atlprf09.slac.stanford.edu': 2,\
		'atlprf10.slac.stanford.edu': 2,\
		'atlprf11.slac.stanford.edu': 2,\
		'atlprf12.slac.stanford.edu': 2,\
		'atlprf13.slac.stanford.edu': 2,\
		'atlprf14.slac.stanford.edu': 2,\
		'atlprf15.slac.stanford.edu': 2,\
		'atlprf16.slac.stanford.edu': 2,\
		'atlprf17.slac.stanford.edu': 3,\
		'atlprf18.slac.stanford.edu': 3,\
		'atlprf19.slac.stanford.edu': 3,\
		'atlprf20.slac.stanford.edu': 3,\
		'atlprf21.slac.stanford.edu': 3,\
		'atlprf22.slac.stanford.edu': 3,\
		'atlprf23.slac.stanford.edu': 3,\
		'atlprf24.slac.stanford.edu': 3,\
		'atlprf25.slac.stanford.edu': 3,\
		'atlprf26.slac.stanford.edu': 3,\
		'atlprf27.slac.stanford.edu': 3,\
		'atlprf28.slac.stanford.edu': 3,\
		'atlprf29.slac.stanford.edu': 3,\
		'atlprf30.slac.stanford.edu': 3,\
		'atlprf31.slac.stanford.edu': 3,\
		'atlprf32.slac.stanford.edu': 3,\
		'atlprf33.slac.stanford.edu': 3,\
		'atlprf34.slac.stanford.edu': 3,\
		'atlprf35.slac.stanford.edu': 3,\
		'atlprf36.slac.stanford.edu': 3 }
	return mydict

hostWeights = hostDict()
sumWeights = 0
for host, weight in hostWeights.items():
	sumWeights += weight
	
class Dataset(object):
	def __init__(self):
		self.files = fileDict() # keys are hostnames
		self.iso = False
		self.excessFiles = []
		self.total = 0
		self.ave = hostDict()
	def add(self, dataset):
		for host, list in dataset.files.items():
			for file in list:
				self.files[host].append(file)
		self.computeTotalAve()
	def remove(self, dataset):
		for host, list in dataset.files.items():
			for file in list:
				self.files[host].remove(file)
		self.computeTotalAve()
	def setIso(self, isolate):
		self.iso = isolate
	def computeTotalAve(self):
		self.total = 0
		for key, value in self.files.items():
			self.total += len(value)
		temp = (self.total//sumWeights)
		totaltemp = 0
		for host, num in self.ave.items():
			self.ave[host] = temp*hostWeights[host]
			totaltemp += self.ave[host]

		temp = self.total-totaltemp
		while (temp>len(self.files)):
			totaltemp = 0
			for host, num in self.ave.items():
				self.ave[host] += 1
				totaltemp += self.ave[host]
			temp = self.total-totaltemp

	def rearrange(self):
		if (self.iso == True):
			besthost = ""
			n = 0
			for key, value in self.files.items():
				m = len(value)
				if (m > n):
					besthost = key
					n = m
			for key, value in self.files.items():
				if (key == besthost):
					self.excessFiles.append(value[0].oldhost)	
					continue
				for file in value:
					file.setHost(besthost)
					self.files[besthost].append(file)				
				self.files[key] = []
			self.computeTotalAve()
		else:
			self.computeTotalAve()
			for dhost in self.files.keys():
				if(self.ave[dhost]<=len(self.files[dhost])):
					continue
				for shost in self.files.keys():
					if (len(self.files[shost])<=self.ave[shost]):
						continue
					while ((len(self.files[shost])>(self.ave[shost]+1)) and (len(self.files[dhost])<self.ave[dhost])):
						file = self.files[shost][0]
						file.setHost(dhost)
						self.files[dhost].append(file)
						self.files[shost].remove(file)
				# pass for case dataset consists of hosts with at max ave+1 files
				if (len(self.files[dhost])<self.ave[dhost]):
					for shost in self.files.keys():
						if(len(self.files[shost])<=self.ave[shost]):
							continue
						while((len(self.files[shost])>(self.ave[shost])) and (len(self.files[dhost])<self.ave[dhost])):
							file = self.files[shost][0]
							file.setHost(dhost)
							self.files[dhost].append(file)
							self.files[shost].remove(file)					
			
			# at this point all hosts should have at least ave number of files
			# make sure excess files are evenly distributed
			for shost in self.files.keys():
				if ((len(self.files[shost])-self.ave[shost])<0):
					print "Should not be here!"
					sys.exit(1)
				elif ((len(self.files[shost])-self.ave[shost])==0):
					continue
				elif(self.excessFiles.count(shost)>0):
					continue
				self.excessFiles.append(shost)
				if ((len(self.files[shost])-self.ave[shost])==1):
					continue
				for dhost in self.files.keys():
					if (len(self.files[dhost])==self.ave[dhost]):
						file = self.files[shost][0]
						file.setHost(dhost)
						self.files[dhost].append(file)
						self.files[shost].remove(file)
						self.excessFiles.append(dhost)
					if ((len(self.files[shost])-self.ave[shost])<=1):
							break

class File(object):
	def __init__(self, fullpath):
		split = fullpath.split("://",1)
		self.protocol = split[0]
		split = split[1].split("/",1)
		hostport = split[0].split(":",1)
		self.oldhost = hostport[0]
		self.newhost = self.oldhost
		self.port = hostport[1]
		self.filename = split[1].rstrip("\n").replace("//","/")
		list = tidre.findall(self.filename)
		if (len(list)>0):
			self.tid = list[0]
		else:
			list = userre.findall(self.filename)
			if(len(list)>0):
				self.tid = list[0]
			else:
				list = groupre.findall(self.filename)
				if(len(list)>0):
					self.tid = list[0]
				elif(allowregexpfail == True):
					self.tid = "failedregexp"
				else:	
					print "Regexp on the filenames in this dataset failed, contact Max or Bart to fix it."
					print "Filename is",self.filename
					sys.exit(1)
				
	def setHost(self, new):
		self.newhost = new
	def oldPath(self):
		return "%s://%s:%s/%s" % (self.protocol, self.oldhost, self.port, self.filename)
	def newPath(self):
		return "%s://%s:%s/%s" % (self.protocol, self.newhost, self.port, self.filename)

mode = "simple"
testrun = False
writescripts = False
allowregexpfail = False

if (len(sys.argv) == 1):
	print "Usage: python fixFileDist.py input_resolved_file_list.txt [simple | isolate | spread | viewonly] [testrun | writescripts] [allowregexpfail]"
	sys.exit()
elif (len(sys.argv) > 2):
	if (sys.argv[2] == "simple"):
		mode = "simple"
	elif (sys.argv[2] == "isolate"):
		mode = "isolate"
	elif (sys.argv[2] == "spread"):
		mode = "spread"
	elif (sys.argv[2] == "viewonly"):
		mode = "viewonly"
	elif (sys.argv[2] == "testrun"):
		testrun = True
	elif (sys.argv[2] == "writescripts"):
		writescripts = True
	elif (sys.argv[2] == "allowregexpfail"):
		allowregexpfail = True
	else:
		print "Unrecognized option:",sys.argv[2]
if (len(sys.argv) > 3):
	if (sys.argv[3] == "testrun"):
		testrun = True
	elif (sys.argv[3] == "writescripts"):
		writescripts = True
	elif (sys.argv[3] == "allowregexpfail"):
		allowregexpfail = True
	else:
		print "Unrecognized option:",sys.argv[3]
if (len(sys.argv) > 4):
	if (sys.argv[4] == "allowregexpfail"):
		allowregexpfail = True
	else:
		print "Unrecognized option:",sys.argv[3]



outfilename = ""
if((sys.argv[1].count("resolved")<1) and (mode != "viewonly")):
	print "Input file",sys.argv[1],"does not contain 'resolved'. Are you sure you have resolved the file list first?"
	print "If you are trying to just look at a distribution, use the option 'viewonly'"
	sys.exit(1)
else:
	outfilename = sys.argv[1].replace("resolved",mode)

infile = open(sys.argv[1],"r")

filesByDS = defaultdict(Dataset)
files = [ ]

while (infile):
	line = infile.readline()
	n = len(line)
	if (n==0):
		break
	file = File(line)
	files.append(file)
	
	if (mode == "simple"):
		filesByDS['all'].files[file.oldhost].append(file)
	else:
		filesByDS[file.tid].files[file.oldhost].append(file)
		if (mode == "isolate"):
			filesByDS[file.tid].setIso(True)
			
infile.close()

overall = Dataset()

# organize by individual dataset
for dsname, ds in filesByDS.items():
	if(mode != "viewonly"):
		ds.rearrange()
	else:
		ds.computeTotalAve()
	overall.add(ds)

#try to fix overall file distribution
if((mode == "spread") or (mode == "simple")):
	for dhost in overall.files.keys():
		if(overall.ave[dhost]<=len(overall.files[dhost])):
			continue
			
		for shost in overall.files.keys():
			if (len(overall.files[shost])<=overall.ave[shost]):
				continue
			if (len(overall.files[dhost])==overall.ave[dhost]):
				break

			for dsname, ds in filesByDS.items():
				if((ds.excessFiles.count(shost)!=0) and (ds.excessFiles.count(dhost)==0)):
					file = ds.files[shost][0]
					file.setHost(dhost)
					ds.files[dhost].append(file)
					ds.files[shost].remove(file)
					ds.excessFiles.append(dhost)
					ds.excessFiles.remove(shost)
					overall.files[dhost].append(file)
					overall.files[shost].remove(file)
				# kill ds loop if done with either dhost or shost
				if (len(overall.files[dhost])==overall.ave[dhost]):
					break
				if (len(overall.files[shost])==overall.ave[shost]):
					break	
		
		# second loop is for pathological cases where balance of the individual datasets precludes 
		# balance of the overall ds. In these cases the overall ds balance wins
		for shost in overall.files.keys():
			if (len(overall.files[shost])<=overall.ave[shost]):
				continue
			if (len(overall.files[dhost])>=overall.ave[dhost]):
				break

			for dsname, ds in filesByDS.items():
				if((len(ds.files[shost])==ds.ave[shost]) and (len(ds.files[dhost])==ds.ave[dhost])):
					file = ds.files[shost][0]
					file.setHost(dhost)
					ds.files[dhost].append(file)
					ds.files[shost].remove(file)
					ds.excessFiles.append(dhost)
					overall.files[dhost].append(file)
					overall.files[shost].remove(file)
				# kill ds loop if done with either dhost or shost
				if (len(overall.files[dhost])==overall.ave[dhost]):
					break
				if (len(overall.files[shost])==overall.ave[shost]):
					break			
			
	# at this point all hosts should have at least ave number of files
	# make sure excess files are evenly distributed
	overall.excessFiles = []
	for shost in overall.files.keys():
		if ((len(overall.files[shost])-overall.ave[shost])<0):
			print "Should not be here either!"
			sys.exit(1)
		elif ((len(overall.files[shost])-overall.ave[shost])==0):
			continue
		elif(overall.excessFiles.count(shost)>0):
			continue
		overall.excessFiles.append(shost)
		if ((len(overall.files[shost])-overall.ave[shost])==1):
			continue
		for dhost in overall.files.keys():
			if (len(overall.files[dhost])!=overall.ave[dhost]):
				continue

			for dsname, ds in filesByDS.items():
				if((ds.excessFiles.count(shost)!=0) and (ds.excessFiles.count(dhost)==0)):
					file = ds.files[shost][0]
					file.setHost(dhost)
					ds.files[dhost].append(file)
					ds.files[shost].remove(file)
					ds.excessFiles.append(dhost)
					ds.excessFiles.remove(shost)
					overall.files[dhost].append(file)
					overall.files[shost].remove(file)
					overall.excessFiles.append(dhost)
					break
				# kill ds loop if done with shost
				if ((len(overall.files[shost])-overall.ave[shost])==1):
					break		
			# kill dhost loop if done with shost
			if ((len(overall.files[shost])-overall.ave[shost])==1):
				break

elif (mode == "isolate"):
	# fix overall file distribution while keeping datasets isolated
	keylist = []
	maxdev = 0
	for host in overall.files.keys():
		if ((len(overall.files[host])==overall.ave[host]) or (len(overall.files[host])==(overall.ave[host]+1))):
			continue
		if ((overall.ave[host]-len(overall.files[host]))>maxdev):
			maxdev = overall.ave[host]-len(overall.files[host])
		keylist.append(host)
	
	# pass for deviations down to 1, disallowing transfer from hosts with only one extra file
	for dev in range(maxdev, 0, -1):
		for dhost in keylist:
			if ((len(overall.files[dhost])==overall.ave[dhost]) or (len(overall.files[dhost])==(overall.ave[dhost]+1))):
				continue
			if(overall.ave[dhost]<len(overall.files[dhost])):
				continue
			if((overall.ave[dhost]-len(overall.files[dhost]))<dev):
				continue

			for shost in keylist:
				if (len(overall.files[shost])<(overall.ave[shost]+dev)):
					continue
				if ((len(overall.files[shost])==overall.ave[shost]) or (len(overall.files[shost])==(overall.ave[shost]+1))):
					continue
				if (len(overall.files[dhost])>=overall.ave[dhost]):
					break
					
				for dsname, ds in filesByDS.items():
					if((ds.excessFiles.count(shost)!=0) and (len(ds.files[shost])==dev)):
						for file in ds.files[shost]:
							file.setHost(dhost)
							ds.files[dhost].append(file)
							overall.files[dhost].append(file)
							overall.files[shost].remove(file)
						ds.files[shost] = []
						ds.excessFiles.append(dhost)
						ds.excessFiles.remove(shost)
					# kill ds loop if done with either dhost or shost
					if ((len(overall.files[shost])==overall.ave[shost]) or (len(overall.files[shost])==(overall.ave[shost]+1))):
						break
					if ((len(overall.files[dhost])==overall.ave[dhost]) or (len(overall.files[dhost])==(overall.ave[dhost]+1))):
						break
					# kill ds loop
					if (len(overall.files[shost])<(overall.ave[shost]+dev)):
						break		
	
	# pass for deviations of 1, allowing transfer from hosts with only one extra file
	for dev in range(1, 0, -1):
		for dhost in overall.files.keys():
			if (len(overall.files[dhost])>=overall.ave[dhost]):
				continue

			for shost in overall.files.keys():
				if (len(overall.files[shost])<(overall.ave[shost]+dev)):
					continue
				if (len(overall.files[dhost])>=overall.ave[dhost]):
					break
					
				for dsname, ds in filesByDS.items():
					if((ds.excessFiles.count(shost)!=0) and (len(ds.files[shost])==dev)):
						for file in ds.files[shost]:
							file.setHost(dhost)
							ds.files[dhost].append(file)
							overall.files[dhost].append(file)
							overall.files[shost].remove(file)
						ds.files[shost] = []
						ds.excessFiles.append(dhost)
						ds.excessFiles.remove(shost)
					# kill ds loop if done with either dhost or shost
					if (len(overall.files[shost])==overall.ave[shost]):
						break
					if (len(overall.files[dhost])==overall.ave[dhost]):
						break
						
	# first pass to spread out excess files
	if True: # does nothing
		for dhost in overall.files.keys():
			if (len(overall.files[dhost])>=(overall.ave[dhost]+1)):
				continue

			for shost in overall.files.keys():
				if (len(overall.files[shost])<=(overall.ave[shost]+1)):
					continue
				if (len(overall.files[dhost])>=(overall.ave[dhost]+1)):
					break
					
				for dsname, ds in filesByDS.items():
					if((ds.excessFiles.count(shost)!=0) and ((len(overall.files[dhost])+len(ds.files[shost])-overall.ave[dhost])<(len(overall.files[shost])-overall.ave[shost]))):
						for file in ds.files[shost]:
							file.setHost(dhost)
							ds.files[dhost].append(file)
							overall.files[dhost].append(file)
							overall.files[shost].remove(file)
						ds.files[shost] = []
						ds.excessFiles.append(dhost)
						ds.excessFiles.remove(shost)
					# kill ds loop if done with either dhost or shost
					if (len(overall.files[shost])<=(overall.ave[shost]+1)):
						break
					if (len(overall.files[dhost])>=(overall.ave[dhost]+1)):
						break

for dsname, ds in filesByDS.items():
	print "dataset with task ID",dsname,"and",ds.total,"total files, host numfiles target:" 
	ds.computeTotalAve()
	for host, filelist in sorted(ds.files.items()):
		if(len(filelist)>0):
			print host, len(filelist), ds.ave[host]
				
print "overall dataset with",overall.total,"total files, host numfiles target:"	
for host, filelist in sorted(overall.files.items()):
	print host, len(filelist), overall.ave[host]
	
if (mode == "viewonly"):
	sys.exit(0)
	
cmds = []
cmdDict = fileDict()
outfile = open(outfilename,"w")
for file in files:
	outline = "%s%s" % (file.newPath(),"\n")
	outfile.write(outline)
	
	if(file.oldhost == file.newhost):
		continue
	copycmd = "xrdcp %s %s" % (file.oldPath(), file.newPath())
	copycmdlocal = "xrdcp %s %s" % (file.filename, file.newPath())
	rmcmd = "xrd %s:%s rm %s" % (file.oldhost, file.port, file.filename)
	cmds.append(copycmd)
	cmds.append(rmcmd)
	cmdDict[file.oldhost].append(copycmdlocal)
	#cmdDict[file.oldhost].append(copycmd)
	cmdDict[file.oldhost].append(rmcmd)
outfile.close()
print "Number of file moves necessary:",len(cmds)/2

if(testrun == True):
	sys.exit(0)

if (writescripts == False):
	idx = 0
	for cmd in cmds:
		idx += 1
		if(subprocess.call(cmd.split())!=0):
			print "The following command failed, retrying..."
			print cmd
			if(subprocess.call(cmd.split())!=0):
				print "Retry failed, exiting. All commands can be found in cmds.dump:"
				cmdfile = open("cmds.dump","w")
				for cmd2 in cmds:
					cmdline = "%s%s" % (cmd2,"\n")
					cmdfile.write(cmdline)
				cmdfile.close()
				sys.exit(1)
		else:
			if ((idx%200) == 0):
				print "Finished moving file number",idx/2
	print "Moving files complete."
else:
	name = sys.argv[1].replace(".resolved.txt","")
	for host in cmdDict.keys():
		outfilename = "%s_%s.py" % (name,host)
		outfile = open(outfilename,"w")
		outfile.write("import sys\n")
		outfile.write("import subprocess\n")
		outfile.write("\n")
		outfile.write("cmds = []\n")
		for cmd in cmdDict[host]:
			line = "cmds.append(\"%s\")\n" % (cmd)
			outfile.write(line)
		outfile.write("\n")
		outfile.write("print \"Number of files to be moved:\",len(cmds)/2\n")
		outfile.write("\n")
		outfile.write("for cmd in cmds:\n")
		outfile.write("	if(subprocess.call(cmd.split())!=0):\n")
		outfile.write("		print \"The following command failed, retrying...\"\n")
		outfile.write("		print cmd\n")
		outfile.write("		if(subprocess.call(cmd.split())!=0):\n")
		outfile.write("			print \"Retry failed, exiting.\"\n")
		outfile.write("			sys.exit(1)\n")
		outfile.write("print \"Moving files complete.\"\n")
		outfile.close()

	outfilename = "%s.sh" % (name)
	outfile = open(outfilename,"w")
	outfile.write("#!/bin/bash\n")
	outfile.write("\n")
	outfile.write("path=`dirname $0`\n")
	line = "name=\"%s\"\n" % name
	outfile.write(line)
	outfile.write("end=\"_`hostname`.slac.stanford.edu\"\n")
	outfile.write("fileroot=$name$end\n")
	outfile.write("echo \"Executing command: nohup python -u $path/$fileroot.py >& /tmp/$fileroot.log &\"\n")
	outfile.write("nohup python -u $path/$fileroot.py >& /tmp/$fileroot.log &\n")
	outfile.write("sleep 1\n")
	outfile.write("echo \"Script complete.\"\n")
	outfile.close()

	outfilename = "%s_logs.sh" % (name)
	outfile = open(outfilename,"w")
	outfile.write("#!/bin/bash\n")
	outfile.write("\n")
	line = "name=\"%s\"\n" % name
	outfile.write(line)
	outfile.write("end=\"_`hostname`.slac.stanford.edu\"\n")
	outfile.write("fileroot=$name$end\n")
	outfile.write("echo \"Executing command: cat /tmp/$fileroot.log\"\n")
	outfile.write("cat /tmp/$fileroot.log\n")
	outfile.write("echo \"Script complete.\"\n")
	outfile.close()

	finalmsg = "Created move scripts (%s_hostname.py)" % (name)
	print finalmsg

