#!/bin/bash
# If you execute this script directly (not from the makefile), first of all
# 1. Don't do it. Use the makefile, that way it verifies that your code compiles
# 2. If you insist, call it from the top directory of ProofAna, or it won't work
#    sh scripts/make_ProofAna_par.sh [binparlib] [excludes]

binparlib=0
if [ ! -z $1 ]; then
	if [ "$1" == "binparlib" ]; then
		binparlib=1
		shift
	fi
fi

ORIG=$PWD
DIR=/tmp/ProofAnaBuild-$$
LIBDIR=$DIR/libProofAna

rm -rf $DIR
mkdir -p $LIBDIR

ln -sf $ORIG/Makefile $LIBDIR/
ln -sf $ORIG/analyses $LIBDIR/
ln -sf $ORIG/core $LIBDIR/
ln -sf $ORIG/eventbuilders $LIBDIR/
ln -sf $ORIG/lib $LIBDIR/
ln -sf $ORIG/scripts $LIBDIR/
ln -sf $ORIG/utils $LIBDIR/

mkdir $LIBDIR/PROOF-INF
cd $LIBDIR/PROOF-INF

if [ "$binparlib" -eq 1 ]; then
	cat > BUILD.sh <<EOF
#! /bin/bash
# binparlib mode, do nothing
EOF
else
	cat > BUILD.sh <<EOF
#! /bin/bash
# Build libProofAna library.

if [ "\$1" = "clean" ]; then
   make clean
   exit 0
fi

make solib -j5
EOF
fi

cat > SETUP.C <<EOF
#include <TString.h>
#include <fstream>
#include <iostream>
#include "scripts/loadLibraries.C"

int SETUP()
{
	gSystem->Setenv("PROOFANADIR",".");
	return loadLibraries();
}
EOF

chmod 755 BUILD.sh

cd $DIR
tar -czhf libProofAna.par $@ libProofAna
mv libProofAna.par $ORIG/lib
cd $ORIG
rm -rf $DIR

exit 0
