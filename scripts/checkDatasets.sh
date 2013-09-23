#!/bin/bash

# This script should be used in conjunction with getFilesFromGridXrootd.sh, as
# it checks the download log files produced by it. It must be run from the datasets
# directory

# use any argument to trigger an additional check using dq2-ls -f

echo
echo "Usage: sh checkDatasets.sh [\"regexp\"] [dq2]"
echo

dq2=false
regexp=".*"
while [[ ! -z $1 ]]; do
	if [[ "$1" == "dq2" ]]; then
		dq2=true
	else
		regexp="$1"
	fi
	shift
done

if [[ ".*" != "${regexp}" ]]; then
	echo "Starting check datasets matching regexp: ${regexp}"
else
	echo "Starting to check all datasets..."
fi

datasets=`ls -d */ | grep -e "${regexp}"`
prev_dir=${PWD}

## check datasets
for ds in ${datasets}; do
	echo "Checking ${ds}..."
	cd ${ds}
        ds_name=$(echo ${ds%/})
	if [[ -e get_${ds_name}_missed.log ]]; then
		blacklist="`grep -c "NO AVAILABLE SITES ERROR" get_${ds_name}_missed.log`"
		files="`grep -c File: get_${ds_name}_missed.log`"
		successful="`grep -c SUCCESSFUL get_${ds_name}_missed.log`"
		exception="`grep -c \"LFC exception\" get_${ds_name}_missed.log`"
		finished="`grep -c Finished get_${ds_name}_missed.log`"
		if [[ ! ${files} -eq ${successful} ]]; then
			echo "Not all files downloaded successfully."
		fi
                if [[ ${finished} -lt 1 ]]; then
                        echo "Download not finished."
                fi
                if [[ ${exception} -gt 0 ]]; then
                        echo "LFC exception."
                fi
                if [[ ${blacklist} -gt 0 ]]; then
                        echo "Some files only available from blacklisted sites."
                fi
	else
		echo "Missing ${ds}get_${ds_name}_missed.log, skipping grep check."
	fi
	if [[ ${dq2} == true ]]; then
		numdq2="`dq2-ls -f ${ds} | grep -c .root`"
		numlocal="`ls * | grep -c .root`"
		if [[ ${numdq2} -ne ${numlocal} ]]; then
			echo "${ds} not completely downloaded (DQ2 check)."
			echo "Number of local ROOT files: ${numlocal}"
			echo "Number of remote ROOT files: ${numdq2}"
		fi
	fi
	cd $prev_dir
done

