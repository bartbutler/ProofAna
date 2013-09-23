# ProofAna makefile
# B. Butler

# --- External configuration ----------------------------------
ARCH_LOC_1 := $(wildcard $(shell root-config --prefix)/etc/Makefile.arch)
ARCH_LOC_2 := $(wildcard $(shell root-config --prefix)/test/Makefile.arch)
ARCH_LOC_3 := $(wildcard $(shell root-config --prefix)/share/root/test/Makefile.arch)
ARCH_LOC_4 := $(wildcard $(shell root-config --prefix)/share/doc/root/test/Makefile.arch)

ifneq ($(strip $(ARCH_LOC_1)),)
  $(info Using $(ARCH_LOC_1))
  include $(ARCH_LOC_1)
else
  ifneq ($(strip $(ARCH_LOC_2)),)
    $(info Using $(ARCH_LOC_2))
    include $(ARCH_LOC_2)
  else
    ifneq ($(strip $(ARCH_LOC_3)),)
      $(info Using $(ARCH_LOC_3))
      include $(ARCH_LOC_3)
    else
      ifneq ($(strip $(ARCH_LOC_4)),)
         $(info Using $(ARCH_LOC_4))
         include $(ARCH_LOC_4)
      else
         $(error Could not find Makefile.arch! Is ROOT not set up?)
      endif
    endif
  endif
endif

# -------------------------------------------------------------
# Hack for Mac OSX 10.6+
# -------------------------------------------------------------
ifeq ($(PLATFORM),macosx)
SOFLAGS += -undefined dynamic_lookup
endif

# -------------------------------------------------------------
# General flags
# -------------------------------------------------------------
PACKAGE    := ProofAna
OUTPUTDIR  := lib

# glibc memcmp much faster (factor of 3) than gcc builtin for gcc 4.0-4.7, we use it heavily for comparing strings
ifneq ($(CXX),clang++)
	CXXFLAGS += -fno-builtin-memcmp
else
	CXXFLAGS += -Qunused-arguments
endif

# SLC5 Athena release in SL6
CXXFLAGS += $(CPPEXPFLAGS)

INCLUDES += -I. -Icore -Ihelpers -Ianalyses -Ieventbuilders -Iutils -I${ROOTSYS}/include

# Need these to avoid loading dependent libraries when ROOT starts
LINKLIBS := -L${ROOTSYS}/lib -lHist -lMatrix -lTree -lNet -lProofPlayer -lProof -lRIO -lThread -lMathCore -lPhysics

default: parlib

# -------------------------------------------------------------
# ProofAna packages
# -------------------------------------------------------------
# These have the form NAME/eventbuilders, NAME/utils NAME/analyses, etc. They are detected via scripts which
# use symbolic links in order to integrate them with ProofAna

PACKAGEOUT := $(shell sh scripts/package_links.sh)
PACKAGEFLAG := $(findstring SUCCESS,$(PACKAGEOUT))
PACKAGEMSG := $(filter-out SUCCESS,$(PACKAGEOUT))

ifneq (SUCCESS,$(PACKAGEFLAG))
$(error ProofAna package symbolic link processing failed: $(PACKAGEMSG))
endif

# -------------------------------------------------------------
# External utilities
# -------------------------------------------------------------

# DEPS is a list of rules to make util dependencies for ProofAna, can be shared libraries or other files
# LOADS is a list of CINT commands to execute to load utils libraries
# CLEANS are rules to make clean for utils packages
# EXCLUDES are files (in the form "--exclude FILE1 --exclude FILE2") which should NOT be put in the PAR tarball
# GRIDEXECS are CINT commands that must be executed only on the grid

-include $(shell sh scripts/order_utils.sh)

# -------------------------------------------------------------
# ROOT Cint
# -------------------------------------------------------------
CINT       := ProofAnaCint
LDEFFILE   := $(OUTPUTDIR)/LinkDef.h
CINTFILE   := $(OUTPUTDIR)/$(CINT).cxx
CINTOBJ    := $(patsubst %.cxx,%.o,$(CINTFILE))

EBCXXLIST := $(filter-out %Cint.cxx,$(wildcard eventbuilders/EventBuilder_*.cxx))
ANACXXLIST := $(filter-out %Cint.cxx,$(wildcard analyses/Analysis_*.cxx))

EBHLIST := $(patsubst %.cxx,%.h,$(EBCXXLIST))
ANAHLIST := $(patsubst %.cxx,%.h,$(ANACXXLIST))

PAHLIST := $(wildcard core/*.h)

FECINTOBJLIST := $(patsubst %.h,%_Cint.o,$(EBHLIST))
ANACINTOBJLIST := $(patsubst %.h,%_Cint.o,$(ANAHLIST))


# -------------------------------------------------------------
# ProofAna source
# -------------------------------------------------------------
# add util subdirectories if needed
CXXLIST := $(wildcard core/*.cxx) $(EBCXXLIST) $(ANACXXLIST)
HLIST   := $(PAHLIST) $(EBHLIST) $(ANAHLIST)
OLIST   := $(patsubst %.cxx,%.o,$(CXXLIST)) $(FECINTOBJLIST) $(ANACINTOBJLIST)  $(CINTOBJ)
DLIST   := $(patsubst %.h,%.d,$(HLIST))

# -------------------------------------------------------------
# Libraries
# -------------------------------------------------------------
LIBFILE  := $(OUTPUTDIR)/libProofAna.so
PARFILE  := $(OUTPUTDIR)/libProofAna.par

ifeq ($(PLATFORM),macosx)
EXTRALDFLAGS := -install_name @rpath/libProofAna.so
endif

# get libraries of ROOT
define ldlinksuffixROOT
   $(addsuffix $(LDLINKSUFFIX),$(Lib)) $(shell if [ "$(findstring -Ldlink2,$(OPTIONS))" ]; then echo $(addsuffix _pkgid_$(ROOTVER),$(Lib)); fi)
endef

# -------------------------------------------------------------
# PAR file extras
# -------------------------------------------------------------

# use "make root" to include ROOT files in the PAR file (can increase the size of the PAR file enormously)
ifeq (,$(findstring root,$(MAKECMDGOALS)))
EXCLUDES += --exclude \*.root\*
else
$(shell if [ -e $(PARFILE) ]; then rm $(PARFILE); fi)
endif

# use "make binparlib" to include binary libraries in par file to avoid compilation on workers
# do not use this option on the Grid!
ifeq (,$(findstring binparlib,$(MAKECMDGOALS)))
EXCLUDES += --exclude \*.so --exclude \*.dylib --exclude \*Cint\* --exclude \*.d --exclude \*.o --exclude packages.lock
else
EXCLUDESBKP := $(EXCLUDES)
EXCLUDES := binparlib $(EXCLUDESBKP)
$(shell if [ -e $(PARFILE) ]; then rm $(PARFILE); fi)
endif

# -------------------------------------------------------------
# Compilation
# -------------------------------------------------------------

# Implicit rule to compile Analysis classes
Analysis_%.o : Analysis_%.cxx $(DEPS)
	@echo "Compiling $<"
	@$(CXX) -MD $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Implicit rule to compile EventBuilder classes
EventBuilder_%.o : EventBuilder_%.cxx $(DEPS)
	@echo "Compiling $<"
	@$(CXX) -MD $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Implicit rule to compile core and generic classes
%.o : %.cxx
	@echo "Compiling $<"
	@$(CXX) -MD $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Implicit rule to write ROOTCINT dictionary classes for Analyses, EventBuilders
%_Cint.cxx : %.h $(PAHLIST) %_LinkDef.h
	@echo "Running rootcint for $*"
	@$(ROOTSYS)/bin/rootcint -f $@ -c -p  $(CXXFLAGS) $(INCLUDES) $^

# Implicit rule to write LinkDef.h files for Analyses, EventBuilders
%_LinkDef.h : $(DEPS)
	@echo "Creating LinkDef file for class $(patsubst analyses/%,%,$(patsubst eventbuilders/%,%,$*))"
	@echo "#ifndef __$(patsubst analyses/%,%,$(patsubst eventbuilders/%,%,$*))__" >> $@
	@echo "#define __$(patsubst analyses/%,%,$(patsubst eventbuilders/%,%,$*))__" >> $@
	@echo "" >> $@
	@echo "#ifdef __CINT__" >> $@
	@echo "" >> $@
	@echo "#pragma link off all globals;" >> $@
	@echo "#pragma link off all defined_ines;" >> $@
	@echo "#pragma link off all functions;" >> $@
	@echo "#pragma link C++ nestedclasses;" >> $@
	@echo "" >> $@
	@echo "#pragma link C++ class $(patsubst analyses/%,%,$(patsubst eventbuilders/%,%,$*))+;" >> $@
	@echo "" >> $@
	@echo "#endif" >> $@
	@echo "#endif" >> $@

# Rules to make ROOTCINT output file
$(CINTOBJ) : $(PAHLIST) $(LDEFFILE)
	@echo "Running rootcint for $(CINT)"
	@$(ROOTSYS)/bin/rootcint -f $(CINTFILE) -c -p  $(CXXFLAGS) $(INCLUDES) $(PAHLIST) $(LDEFFILE)
	@echo "Compiling $(CINTFILE)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(CINTFILE) -o $@

# Rule to combine objects into a shared library
$(LIBFILE): $(OLIST) $(OUTPUTDIR)/packages.lock
	@echo "Linking $(LIBFILE)"
	@mkdir -p $(OUTPUTDIR)
	@rm -f $(LIBFILE)
	@$(LD) $(CXXFLAGS) $(SOFLAGS) $(LINKLIBS) $(EXTRALDFLAGS) $(OLIST) -o $(LIBFILE)

# Rule to create PAR file
$(PARFILE): $(DEPS) $(CXXLIST) $(HLIST) $(LDEFFILE) $(LIBFILE)
	@echo "Making lib/libProofAna.par"
	@sh scripts/make_ProofAna_par.sh $(EXCLUDES) --exclude \*.log --exclude \*.status --exclude \*~ --exclude .svn --exclude \*.par

# Rule to write library-loading script
scripts/loadLibraries.C:
	@if [ -e scripts/loadLibraries.C ]; then rm scripts/loadLibraries.C; fi
	@echo "Writing loadLibraries function..."
	@echo "// Generated by ProofAna makefile, do not edit manually" >> $@
	@echo "int loadLibraries() {" >> $@
	@echo "	if(!gSystem->Getenv(\"PROOFANADIR\")) gSystem->Setenv(\"PROOFANADIR\",\"..\");" >> $@
	@echo "	gSystem->AddDynamicPath(gSystem->Getenv(\"PROOFANADIR\"));" >> $@
	@echo "	$(LOADS)" >> $@
	@echo "	if (gSystem->Load(\"$(LIBFILE)\") == -1) return -1;" >> $@
	@echo "	return 0;" >> $@
	@echo "}" >> $@

-include $(DLIST)

.PHONY: ProofAnaCint parlib binparlib root solib clean $(CLEANS) scripts/loadLibraries.C

# Prevents deletion of intermediate files
.SECONDARY:

ProofAnaCint: $(CINTOBJ) $(ANACINTOBJLIST) $(FECINTOBJLIST)
parlib: solib $(PARFILE)
binparlib: parlib
	@:
root: parlib
	@:
solib: $(DEPS) $(LIBFILE) scripts/loadLibraries.C

clean: $(CLEANS)
	@rm -f */*.o */*.d */*Cint* */*.so */*.lock */*.par
