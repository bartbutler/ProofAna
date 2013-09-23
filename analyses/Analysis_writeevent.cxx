/**************************************************************************
 **
 **   File:         Analysis_writeevent.cxx
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

#define Analysis_writeevent_cxx

#include "Analysis_writeevent.h"
#include "AnaConfig.h"
#include <TROOT.h>
#include <iostream>
#include <cstdlib>

void Analysis_writeevent::WorkerBegin()
{
	static const MomKey mEVENTBUILDER("EVENTBUILDER");
	static const MomKey mTREENAME("TREENAME");
	if(Cfg()->Exists(mTREENAME)) fTreeName = AnaKey(Cfg()->String(mTREENAME));
	else fTreeName = AnaKey(DatasetCfg()->String(mEVENTBUILDER));

	static const MomKey mRAW("RAW");
	bool raw = false;
	Cfg()->Get(mRAW,raw);

	RegisterTree(fTreeName,!raw);
}

bool Analysis_writeevent::ProcessEvent()
{

	FillTree(fTreeName);

	return kTRUE;

}

void Analysis_writeevent::WorkerTerminate()
{

	// Nothing

}

