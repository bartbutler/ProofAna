import sys
import os
import re

try:
	from pyAMI.client import AMIClient
	from pyAMI.auth import AMI_CONFIG, create_auth_config
	from pyAMI.query import get_dataset_xsec_effic, get_dataset_info
except:
	print "This script uses the pyAMI 4.0 API. Please install pyAMI before using it."
	sys.exit(1)

client = AMIClient()
if not os.path.exists(AMI_CONFIG):
   create_auth_config()
client.read_config(AMI_CONFIG)

if(len(sys.argv)<3):
	print "Usage: python makeFileListConfig.py input_file_list.txt treename"
	sys.exit(1)

filelist = sys.argv[1]
treename = sys.argv[2]

flre = re.compile(".*\.txt$")
if not flre.search(filelist):
	print "File list must end in .txt"
	sys.exit(1)

filelistconfig = filelist[:-4] + ".config"

infile = 0
try:
	infile = open(filelist,"r")
except:
	print "Cannot open file list", filelist
	sys.exit(1)

dslist = [ ]
while (infile):
	line = infile.readline()
	n = len(line)
	if (n==0):
		break
	dirs = line.split("/")
	if len(dirs) < 2:
		print "No path information for file:",line
		print "Assuming it is a dataset name."
		if dslist.count(line) == 0:
			dslist.append(line)
		continue

	skip = True
	for dir in reversed(dirs):
		if skip:
			skip = False
			continue
		if len(dir) == 0:
			continue
		if dslist.count(dir) != 0:
			break

		dslist.append(dir)
		break

infile.close()

groupre = re.compile(".*(group\..*)")
datare = re.compile(".*(data[0-9][0-9]_[0-9][0-9]?TeV\..*)")
mcre = re.compile(".*(mc[0-9][0-9]_[0-9][0-9]?TeV\..*)")

af2re = re.compile(".*_a.*")

suffixes = [ ]
suffixes.append(re.compile("(.*)_AANT.*"))
suffixes.append(re.compile("(.*)_tid[0-9]*.*"))

newdslist = [ ]
for ds in dslist:

	isData = True
	if not datare.search(ds):
		isData = False

	origds = 0
	if groupre.search(ds) is not None:
		origds = groupre.findall(ds)[0]
	elif datare.search(ds) is not None:
		origds = datare.findall(ds)[0]
	elif mcre.search(ds) is not None:
		origds = mcre.findall(ds)[0]
	else:
		print "Dataset", ds, "does not match any standard regexp, skipping."
		continue

	# strip off known suffixes
	match = True
	while match:
		match = False
		for suffix in suffixes:
			if suffix.search(origds) is not None:
				origds = suffix.findall(origds)[0]
				match = True

	if newdslist.count(origds) != 0:
		continue

	newdslist.append(origds)

dslist = newdslist

outfile = 0
try:
	outfile = open(filelistconfig,"a")
except:
	print "Cannot open config file", filelistconfig
	sys.exit(1)

treeline = "TREENAME=%s\n" % treename
outfile.write(treeline)
for ds in dslist:

	isData = True
	if not datare.search(ds):
		isData = False

	xs = 0
	eff = 0
	slash = False
	if not isData:
		try:
			xs, eff = get_dataset_xsec_effic(client, ds+"/")
			slash = True
		except:
			try:
				xs, eff = get_dataset_xsec_effic(client, ds)
			except:
				print "Dataset", ds, "not found in AMI, skipping."
		if xs == 0:
			continue

	dsinfo = 0
	try:
		dsinfo = get_dataset_info(client, ds+"/")
	except:
		try:
			dsinfo = get_dataset_info(client, ds)
		except:
			print "Dataset", origds, "not found in AMI, skipping."
	if dsinfo == 0:
		continue

	dsnumber = 0
	if isData:
		dsnumber = dsinfo.info["runNumber"]
	else:
		dsnumber = dsinfo.info["datasetNumber"]

	name = 0
	if slash:
		name = "name_%s=%s\n" % (dsnumber, dsinfo.info["logicalDatasetName"]+"/")
	else:
		name = "name_%s=%s\n" % (dsnumber, dsinfo.info["logicalDatasetName"])

	outfile.write(name)

	events = "n_%s=%s\n" % (dsnumber, dsinfo.info["totalEvents"])
	outfile.write(events)

	if not isData:
		xsec = "xs_%s=%s\n" % (dsnumber, xs*1000.) #convert from nb to pb
		outfile.write(xsec)

		effic = "eff_%s=%s\n" % (dsnumber, eff)
		outfile.write(effic)

		kfactor = "k_%s=1.0\n" % dsnumber
		outfile.write(kfactor)

		if af2re.search(dsinfo.info["version"]):
			af2 = "af2_%s=true\n" % dsnumber
			outfile.write(af2)

outfile.close()

print "Finished writing to:",filelistconfig
print "For MC, all K-factors set to 1.0. Be sure to update this!"
