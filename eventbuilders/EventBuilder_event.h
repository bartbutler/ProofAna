/**************************************************************************
 **
 **   File:         EventBuilder_event.h
 **
 **   Description:  Generic EventBuilder class for Event object trees 
 ** 				written by ProofAna (skims).            
 **
 **   Author:       B. Butler
 **
 **   Created:      7-25-11
 **   Modified:
 **
 **************************************************************************/

#ifndef EventBuilder_event_h
#define EventBuilder_event_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "EventBuilderBase.h"

class Event;

class EventBuilder_event : public EventBuilderBase {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain

	//Added for EventBuilder
	Bool_t   LoadTree(Long64_t entry);
	Bool_t   CopyEvent(AnalysisBase* evt);


   EventBuilder_event(TTree * /*tree*/ =0) { }
   virtual ~EventBuilder_event() { }

   ClassDef(EventBuilder_event,0);
   
};

#endif

