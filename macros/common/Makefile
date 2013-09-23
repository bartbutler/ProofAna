# Common classes makefile
# B. Butler

# --- External configuration ----------------------------------
ifeq ($(wildcard $(ROOTSYS)/test/Makefile.arch),)
   include $(ROOTSYS)/etc/Makefile.arch
else
   include $(ROOTSYS)/test/Makefile.arch
endif

# -------------------------------------------------------------
# General flags
# -------------------------------------------------------------
PACKAGE    = Common
OUTPUTDIR  = .

# Get these from Makefile.arch above
#CCC         = g++
#CXXFLAGS    = -g -m32 -fPIC -Wall -W -Woverloaded-virtual -Wno-parentheses -Wno-unused-parameter -Wno-unused-variable
#LDFLAGS    = -g -m32 -fPIC 

# These warnings bother me, so I turn them off.
# CXXFLAGS  += -Wunused-variable
# CXXFLAGS  += -g

# add utils subdirectories if needed
INCLUDES   += -I. -I/Users/bbutler/work/atlasstyle -I${HOME}/atlasstyle

# Need these to avoid loading dependent libraries when ROOT starts
#LINKLIBS = `root-config --libs`
LINKLIBS = -L${ROOTSYS}/lib -lHist -lMatrix

# -------------------------------------------------------------
# Hack for Mac OSX 10.6+
# -------------------------------------------------------------
ifeq ($(PLATFORM),macosx)
SOFLAGS += -undefined dynamic_lookup
endif

# -------------------------------------------------------------
# ROOT Cint
# -------------------------------------------------------------
CINT       = CommonCint
LDEFFILE   = LinkDef.h
CINTFILE   = $(CINT).cxx
CINTOBJ    = $(patsubst %.cxx,%.o,$(CINTFILE))

# -------------------------------------------------------------
# ProofAna source
# -------------------------------------------------------------
CXXLIST = $(patsubst $(CINTFILE),,$(wildcard *.cxx)) $(CINTFILE)
HLIST   = $(patsubst $(CINT).h,,$(patsubst LinkDef.h,,$(wildcard *.h)))
OLIST   = $(patsubst %.cxx,%.o,$(CXXLIST))
DLIST   = $(patsubst %.h,%.d,$(HLIST))


# -------------------------------------------------------------
# Libraries
# -------------------------------------------------------------
LIBFILE  = $(OUTPUTDIR)/libCommon.so

ifeq ($(PLATFORM),macosx)
EXTRALDFLAGS = -install_name @rpath/libCommon.so
endif

# get libraries of ROOT
define ldlinksuffixROOT
   $(addsuffix $(LDLINKSUFFIX),$(Lib)) $(shell if [ "$(findstring -Ldlink2,$(OPTIONS))" ]; then echo $(addsuffix _pkgid_$(ROOTVER),$(Lib)); fi)
endef

# -------------------------------------------------------------
# Compilation
# -------------------------------------------------------------

default: shlib

# Implicit rule to compile all classes
%.o : %.cxx
	@echo "Compiling $<"
	@$(CXX) -MD $(CXXFLAGS) $(INCLUDES) -c $< -o $*.o 

# Rule to make ROOTCINT output file
$(CINTOBJ) : $(HLIST) $(LDEFFILE)
	@echo "Running rootcint"
	@$(ROOTSYS)/bin/rootcint -f $(CINTFILE) -c -p  $(CXXFLAGS) $(INCLUDES) $(HLIST) $(LDEFFILE)
	@echo "Compiling $(CINTFILE)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(CINTFILE) -o $@

rootcint : $(HLIST) $(LDEFFILE)
	@echo "Running rootcint"
	$(ROOTSYS)/bin/rootcint -f $(CINTFILE) -c -p  $(CXXFLAGS) $(INCLUDES) $(HLIST) $(LDEFFILE)

# Rule to combine objects into a shared library
$(LIBFILE): $(OLIST)
	@echo "Linking $(LIBFILE)"
	@mkdir -p $(OUTPUTDIR)
	@rm -f $(LIBFILE)
	@$(LD) $(CXXFLAGS) $(SOFLAGS) $(LINKLIBS) $(EXTRALDFLAGS) $(OLIST) -o $(LIBFILE) 
	@rlibmap -o libCommon.rootmap -l libCommon.so -c LinkDef.h


-include $(DLIST)

CommonCint: $(CINTOBJ)
shlib: solib
solib: $(LIBFILE) 

clean:
	@rm -f *.o *.d $(CINT).* *.so *.rootmap

