#!/bin/bash
# If you execute this script directly (not from the makefile), first of all
# 1. Don't do it. Use the makefile.
# 2. If you insist, call it from the top directory of ProofAna, or it won't work
#    sh scripts/package_links.sh

# update existing links/remove defunct links

alreadylinked=""
pkgtouch=0

for dir in `ls -d * 2> /dev/null`; do
	if [ -d $dir ]; then # is it a directory?
		for link in `ls -d $dir/* 2> /dev/null`; do
			if [ -L $link ]; then # is it a symbolic link?
				target=`readlink $link`
				if [ ! -e $dir/$target ]; then # target not exist?
					rm $link
					if [ "$dir" == "analyses" ]; then
						pkgtouch=1
					elif [ "$dir" == "eventbuilders" ]; then
						pkgtouch=1
					elif [ "$dir" == "utils" ]; then
						pkgtouch=1
					fi
					if [ $? != 0 ]; then
						echo "Failure to remove link $link"
						exit 1
					fi
				else
					alreadylinked+="$target " # these are already-linked targets that should be left alone
				fi
			fi
		done
	fi
done

if [ ! -e lib/packages.lock ]; then
	echo "This file gets touched when a ProofAna package is removed." > lib/packages.lock
fi

# Flag relinking of libProofAna.so and rewriting of loadLibraries.C script if any links have been removed

if [ $pkgtouch -eq 1 ]; then
	touch lib/packages.lock
fi

# make links for new files

for dir in `ls -d * 2> /dev/null`; do
	dirname=${dir##*/}
	if [ $dirname == "analysis" ]; then
		continue
	fi
	if [ $dirname == "config" ]; then
		continue
	fi
	if [ $dirname == "core" ]; then
		continue
	fi
	if [ $dirname == "eventbuilders" ]; then
		continue
	fi
	if [ $dirname == "filelists" ]; then
		continue
	fi
	if [ $dirname == "lib" ]; then
		continue
	fi
	if [ $dirname == "macros" ]; then
		continue
	fi
	if [ $dirname == "run" ]; then
		continue
	fi
	if [ $dirname == "scripts" ]; then
		continue
	fi
	if [ $dirname == "utils" ]; then
		continue
	fi
	if [ -d $dir ]; then # is it a directory?
		for packagedir in `ls -d $dir/* 2> /dev/null`; do
			if [ -d $packagedir ]; then
				subdir=${packagedir##*/}
				if [ -d $subdir ]; then
					for file_cand in `ls -d $packagedir/* 2> /dev/null`; do
						newtarget="../$file_cand"
						if [[ $alreadylinked != *$newtarget* ]]; then
							link="$subdir/${file_cand##*/}"
							if [ -e $link ]; then
								echo "File/directory/link $link already exists, package conflict?"
								exit 1
							else
								ln -s $newtarget $link
								if [ $? != 0 ]; then
									echo "Failure to create link $link to target $file_cand"
									exit 1
								fi
							fi
						fi
					done
				fi
			fi
		done
	fi
done

echo "SUCCESS"
exit 0
