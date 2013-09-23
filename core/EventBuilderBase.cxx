/**************************************************************************
 **
 **   File:         EventBuilderBase.cxx
 **
 **   Description:  EventBuilder base class
 **
 **
 **   Author:       B. Butler
 **
 **   Created:      2-23-11
 **   Modified:
 **
 **************************************************************************/

#define EventBuilderBase_cxx

#include "EventBuilderBase.h"
#include "TSystem.h"
#include "AnaConfig.h"
#include "TRegexp.h"

//static initialization
set<BranchKey> EventBuilderBase::fBranchNames;
set<BranchKey> EventBuilderBase::fUsedBranchNames;
set<BranchKey> EventBuilderBase::fAllSlimBranchNames;
set<BranchKey> EventBuilderBase::fSlimBranchNames;
map<BranchKey,ThinContainer> EventBuilderBase::fThinBranches;
map<BranchKey,SizeContainer> EventBuilderBase::fSizeBranches;
map<BranchKey,BranchContainer> EventBuilderBase::fBranches;
Config* EventBuilderBase::fConfig = 0;
TList* EventBuilderBase::fInput = 0;
Long64_t EventBuilderBase::fEntry = -1;
SelectorBase* EventBuilderBase::fSelector = 0;
TString EventBuilderBase::fFilename = "";
Bool_t EventBuilderBase::fCache = false;
Bool_t EventBuilderBase::fUsed = false;
Bool_t EventBuilderBase::fAddressChange = true;
TTree* EventBuilderBase::fBaseChain = 0;
set<BranchKey> EventBuilderBase::fCalledBefore;

bool EventBuilderBase::Active(const BranchKey& key) {

	if(fAllSlimBranchNames.size()) {
		if(fSlimBranchNames.find(key)!=fSlimBranchNames.end()) return true;
	}
	else {
		if(fBranchNames.find(key)!=fBranchNames.end()) return true;
	}

	return false;
}

void EventBuilderBase::LoadAndThinBranches()
{
	set<BranchKey>::iterator iter;
	if(fAllSlimBranchNames.size()) {
		//Load specified branches
		iter = fSlimBranchNames.begin();
		for(; iter!=fSlimBranchNames.end(); ++iter) Load(*iter);
	}
	else {
		//Load all branches (default skim, no slimming, all branches "on")
		iter = fBranchNames.begin();
		for(; iter!=fBranchNames.end(); ++iter) Load(*iter);
	}

	//Do thinning
	map<BranchKey,ThinContainer>::iterator thiniter = fThinBranches.begin();
	for(; thiniter!=fThinBranches.end(); ++thiniter) {
		thiniter->second.Thin(thiniter->first,thiniter->second.Reference,thiniter->second.Indices);
	}
	//Update size branches
	map<BranchKey,SizeContainer>::iterator sizeiter = fSizeBranches.begin();
	for(; sizeiter!=fSizeBranches.end(); ++sizeiter) {
		sizeiter->second.Update(sizeiter->first,sizeiter->second.Reference);
	}

	//clear list, as multiple calls to thinning doesn't make sense.
	fThinBranches.clear();
	fSizeBranches.clear();
}

void EventBuilderBase::SetBranchStatusSlim()
{
	if(fAllSlimBranchNames.size()) {
		//set branch status off for all branches
		fBaseChain->SetBranchStatus("*",0); //TTree and TChain, if applicable

		//set branch status on for specified branches only
		set<BranchKey>::iterator iter = fSlimBranchNames.begin();
		for(; iter!=fSlimBranchNames.end(); ++iter) fBaseChain->SetBranchStatus(iter->Data(),1); //TTree and TChain, if applicable
	}
}

void EventBuilderBase::SetBranchStatusOn()
{
	if(fAllSlimBranchNames.size()) {
		Tree()->SetBranchStatus("*",1); //Only TTree
	}
}

void EventBuilderBase::CopyAddresses(TTree* tree)
{
	if(fAddressChange) {
		SetBranchStatusSlim();
		Tree()->CopyAddresses(tree);
		SetBranchStatusOn();
	}
	fAddressChange = false;
}

void EventBuilderBase::SetConfig(Config* config)
{
	fConfig = config;
	fConfig->Get("CACHE",fCache); //Also set cache flag
	fConfig->Get("USED",fUsed); //record which branches were used
}

Bool_t EventBuilderBase::NotifyBase()
{
	fAddressChange = true;
	if (!Tree()) return kFALSE;
	TFile* file = Tree()->GetCurrentFile();
	if(file) fFilename = gSystem->BaseName(file->GetName());

	SetBranchStatusOn(); //Turn on all branches in the case of TChain

	//Save branch names in this ntuple
	fBranchNames.clear();
	TObjArray* branchnames = Tree()->GetListOfBranches();
	for(int i = 0; i<branchnames->GetEntriesFast(); ++i) {
		TObjString* str = (TObjString*)branchnames->At(i);
		fBranchNames.insert(BranchKey(str->String()));
	}

	//Check if branches still exist, delete the ones which do not, set addresses for those that do
	map<BranchKey,BranchContainer>::iterator branch = fBranches.begin();
	for(; branch!=fBranches.end();) {
		if(fBranchNames.find(branch->first)==fBranchNames.end()) fBranches.erase(branch++);
		else if(!branch->second.Delete) fBranches.erase(branch++); //in case the TBranch pointer changes, minimal performance impact
		else if(branch->second.TBE) {
			Tree()->SetBranchAddress(branch->first.Data(), (void*)&branch->second.Obj, &branch->second.Branch);
			++branch;
		}
		else {
			Tree()->SetBranchAddress(branch->first.Data(), branch->second.Obj, &branch->second.Branch);
			++branch;
		}
	}

	//Check if slim branches still exist, delete the ones which do not
	fSlimBranchNames = fAllSlimBranchNames;
	set<BranchKey>::iterator iter = fSlimBranchNames.begin();
	for(; iter!=fSlimBranchNames.end();) {
		if(fBranchNames.find(*iter)==fBranchNames.end()) fSlimBranchNames.erase(iter++);
		else ++iter;
	}

	return kTRUE;
}

Bool_t EventBuilderBase::LoadTree(Long64_t entry)
{
	fEntry = entry;
	fCalledBefore.clear();
	fThinBranches.clear();
	fSizeBranches.clear();

	map<BranchKey,BranchContainer>::iterator branch = fBranches.begin();
	if(fCache) {
		for(; branch!=fBranches.end(); ++branch) {
			branch->second.Branch->GetEntry(fEntry);
			branch->second.Loaded=true;
		}
	}
	else {
		for(; branch!=fBranches.end(); ++branch) branch->second.Loaded=false;
	}

	return kTRUE;
}
