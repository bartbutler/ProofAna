/**************************************************************************
 **
 **   File:         EventBuilder_event.cxx
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

#define EventBuilder_event_cxx

#include "EventBuilder_event.h"
#include "AnaConfig.h"
#include "AnalysisBase.h"
#include <iostream>


Bool_t EventBuilder_event::CopyEvent(AnalysisBase* evt)
{
	
	// Copy the event from the TTree to the event class. Return false if you want to reject
	// event at the EventBuilder level using D3PD snippets, etc. 

	TString branchname(evt->Name().Data()); //default branch to use is given by analysis name
	static const MomKey mBRANCH("BRANCH");
	evt->Cfg()->Get(mBRANCH,branchname); //allow override of this default via analysis config option

	BranchKey key(branchname);
	Event* event = 0;
	if(BranchNames().find(key)==BranchNames().end()) {
		if(BranchNames().size()==1) event = GetOwnedP<Event>(*BranchNames().begin());
		else {
			TString msg("EventBuilder_event: ERROR no branch ");
			msg.Append(key.Data());
			msg.Append(" found in TTree, and multiple branche available. Set the option BRANCH in your analysis to override the default, which is ");
			msg.Append(evt->Name().Data());
			Abort(msg);
		}
	}
	else event = GetOwnedP<Event>(key);
	
	//Copy the event into the analysis/change ownership of sub-objects to the analysis
	evt->MoveEvent(*event);
	delete event;
	
	return kTRUE;
}
