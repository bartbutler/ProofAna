#!/bin/bash

ORIGDIR=$PWD

cd utils/RootCore

rm -rf bin
mkdir bin
rm -rf include
mkdir include
rm -rf python
mkdir python
rm -rf user_scripts
mkdir user_scripts
rm -rf data
mkdir data
rm -rf lib
mkdir lib

./configure
cd ..
unset ROOTCOREDIR
unset ROOTCOREBIN
source RootCore/scripts/setup.sh
sh RootCore/scripts/find_packages.sh

PKGFILE=$ROOTCOREDIR/packages

for pkg in `cat $PKGFILE`
do
    name=`basename $pkg`
    #echo relinking $pkg
    if cd $pkg/cmt
    then
	true
    else
	echo "failed to enter package $pkg"
	exit 2
    fi
    $ROOTCOREDIR/scripts/link_package.sh $pkg
done

#if test "x$ROOTCOREGRID" != "x1"
#then
#    if root -l -b -q $ROOTCOREDIR/scripts/load_packages.C+
#    then
#	true
#    else
#	echo "failed to test load libraries"
#	exit 4
#    fi
#fi

cd $ORIGDIR
