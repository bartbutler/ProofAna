/**************************************************************************
 **
 **   File:         Analysis_writeevent.h
 **
 **   Description:  Writes out events in a tree
 **
 **
 **   Author:       B. Butler
 **
 **   Created:      1-20-12
 **   Modified:
 **
 **************************************************************************/

#ifndef Analysis_writeevent_h
#define Analysis_writeevent_h

#include "AnalysisBase.h"

using std::cout;
using std::endl;

class Analysis_writeevent : public AnalysisBase {
public :

	Analysis_writeevent(TTree* /*tree*/ =0) { SetOutput(false); } //Not booking any histograms
	virtual ~Analysis_writeevent() { }

	ClassDef(Analysis_writeevent,0);

private :

	bool    ProcessEvent();
	void    WorkerBegin();
	void    WorkerTerminate();

	AnaKey fTreeName;

};

#endif

