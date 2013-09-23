/**************************************************************************
 **
 **   File:         Analysis_evtnumfilter.cxx
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

#define Analysis_evtnumfilter_cxx

#include "Analysis_evtnumfilter.h"
#include "AnaConfig.h"
#include <TObjString.h>
#include <TObjArray.h>
#include <TROOT.h>
#include <iostream>
#include <cstdlib>

void Analysis_evtnumfilter::WorkerBegin()
{
	static const MomKey mEVENTS("EVENTS");
	TString events(Cfg()->String(mEVENTS));
	
	static const MomKey mREVERSE("REVERSE");
	Cfg()->Get(mREVERSE,fReverse);
	
	TObjArray* arr = events.Tokenize(",");
	for(int i=0; i<arr->GetEntries(); ++i) {
		fEvtNumbers.insert(((TObjString*)arr->At(i))->String().Atoi());
	}
	
	arr->SetOwner(true);
	delete arr;
}

bool Analysis_evtnumfilter::ProcessEvent()
{

	set<int>::iterator iter = fEvtNumbers.find(EventNumber());
	
	if(fReverse) {
		if(iter==fEvtNumbers.end()) return true;
		return false;	
	}
	
	if(iter!=fEvtNumbers.end()) return true;
	return false;
	
}

void Analysis_evtnumfilter::WorkerTerminate()
{

	// Nothing

}

