#!/bin/bash

# order packages with dependency information

utils=`ls -d utils/*/Makefile.ProofAna 2> /dev/null | sed s?utils\/?? | sed s?\/Makefile.ProofAna??`
ordered=${utils}
lastordered=""
while [ "${ordered}" != "${lastordered}" ]; do
	lastordered=${ordered}
	for util in ${utils}; do
		deps=`cat utils/${util}/Makefile.ProofAna | grep "#DEPENDS" | sed s?#DEPENDS??`
		if [ "${deps}" != "" ]; then
			depsarr=( ${deps} )
			totdeps=${#depsarr[@]}
			numdeps=0
			temp=""
			for out in ${ordered}; do
				if [ "${out}" == "${util}" ]; then
					continue
				fi
				temp+=" ${out}"
				for dep in ${deps}; do
					if [ "${dep}" == "${out}" ]; then
						let "numdeps += 1"
						if [ ${numdeps} -eq ${totdeps} ]; then
							temp+=" ${util}"
						fi
					fi
				done
			done
			ordered=${temp}
		fi
	done
done

output=""
for out in ${ordered}; do
	output+=" utils/${out}/Makefile.ProofAna"
done

echo ${output}

exit 0
