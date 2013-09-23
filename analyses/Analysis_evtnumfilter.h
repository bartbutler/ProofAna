/**************************************************************************
 **
 **   File:         Analysis_evtnumfilter.h
 **
 **   Description:  Writes out events in an Event tree
 **                 
 **
 **   Author:       B. Butler
 **
 **   Created:      6-1-12
 **   Modified:
 **
 **************************************************************************/

#ifndef Analysis_evtnumfilter_h
#define Analysis_evtnumfilter_h

#include "AnalysisBase.h"

using std::cout;
using std::endl;

class Analysis_evtnumfilter : public AnalysisBase {
public :

	Analysis_evtnumfilter(TTree* /*tree*/ =0) : fReverse(false) { SetOutput(false); } //Not booking any histograms
	virtual ~Analysis_evtnumfilter() { }

	ClassDef(Analysis_evtnumfilter,0);

private :
	
	bool    ProcessEvent();
	void    WorkerBegin(); 
	void    WorkerTerminate();
	
	bool fReverse;
	set<int> fEvtNumbers;
	
};

#endif

