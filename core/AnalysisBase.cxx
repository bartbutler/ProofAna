/**************************************************************************
 **
 **   File:         AnalysisBase.cxx
 **
 **   Description:  Analysis base class
 **
 **
 **   Author:       B. Butler
 **
 **   Created:      2-22-11
 **   Modified:
 **
 **************************************************************************/

#define AnalysisBase_cxx

#include "SelectorBase.h"
#include "AnalysisBase.h"
#include "EventBuilderBase.h"
#include "AnaConfig.h"
#include <iostream>
#include <cstdlib>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TProfile2D.h>

AnalysisBase::~AnalysisBase()
{
	delete fTimer;

	map<TDirectory*,TDirCache*>::iterator iter = fCacheMap.begin();
	for(; iter!=fCacheMap.end(); ++iter) delete iter->second;
}

void AnalysisBase::SetConfig(Config* config)
{

	if(!config) Abort("AnalysisBase: ERROR No valid Config class");

	else fConfig = config;

	Cfg()->Get("DEBUG",fDebug);
	Cfg()->Get("SAVETIMERS",fSaveTimers);
	Cfg()->Get("OUTPUTDIR",fOutput);

	if(TString(Cfg()->GetName()).IsNull()) Abort("AnalysisBase: ERROR Identifying name not given to Config class");
	else fName = AnaKey(Cfg()->GetName());
}

void AnalysisBase::SetOutputDir(TDirectory* dir)
{
	map<TDirectory*,TDirCache*>::iterator iter = fCacheMap.lower_bound(dir);
	if(iter!=fCacheMap.end() && (dir==iter->first)) fOutputDir = iter->second;
	else {
		fOutputDir = new TDirCache(dir);
		fCacheMap.insert(iter, map<TDirectory*,TDirCache*>::value_type(dir,fOutputDir));
	}
}

void AnalysisBase::FlushCache(TDirectory* file)
{
	map<TDirectory*,TDirCache*>::iterator iter = fCacheMap.begin();
	for(; iter!=fCacheMap.end(); ) {
		if(iter->first->GetFile()==file) {
			map<TDirectory*,TDirCache*>::iterator i = iter;
			++iter;
			delete i->second;
			fCacheMap.erase(i);
		}
		else ++iter;
	}
}

void AnalysisBase::SetTimerOutput(TDirectory* dir)
{
	if(fSaveTimers) fTimer->SetHistDir(dir);
}

void AnalysisBase::SlaveBegin(TTree* /*tree*/)
{

	// When running with PROOF SlaveBegin() is called on each slave server.
	// The tree argument is deprecated (on PROOF 0 is passed).

	//TString option = GetOption();

	static const AnaKey a_SlaveBegin(":SlaveBegin");
	AnaKey aName_SlaveBegin(fName+a_SlaveBegin);
	Time()->Start(aName_SlaveBegin);

	//Set up default output directory
	static const AnaKey adefault("default");
	if(Output()) SetOutputDir((TDirectory*)Selector()->GetFile(adefault)->Get(fName.Data()));
	else SetOutputDir((TDirectory*)Selector()->GetFile(adefault));

	static const AnaKey a_SlaveBegin_WorkerBegin(":SlaveBegin:WorkerBegin");
	AnaKey aName_SlaveBegin_WorkerBegin(fName+a_SlaveBegin_WorkerBegin);
	Time()->Start(aName_SlaveBegin_WorkerBegin);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin WorkerBegin() execution on analysis " << fName << endl;
	WorkerBegin();
	if(Debug()) cout << "AnalysisBase: DEBUG Finish WorkerBegin() execution on analysis " << fName << endl;

	Time()->Stop(aName_SlaveBegin_WorkerBegin);

	Time()->Stop(aName_SlaveBegin);

}

Bool_t AnalysisBase::Process()
{
	// The Process() function is called for each entry in the tree (or possibly
	// keyed object in the case of PROOF) to be processed. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	// It can be passed to either AnalysisBase::GetEntry() or TBranch::GetEntry()
	// to read either all or the required parts of the data. When processing
	// keyed objects with PROOF, the object is already loaded and is available
	// via the fObject pointer.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	//
	// Use fStatus to set the return value of TTree::Process().
	//
	// The return value is currently not used.

	static const AnaKey a_Process(":Process");
	AnaKey aName_Process(fName+a_Process);
	Time()->Start(aName_Process);

	//Reset default output directory
	static const AnaKey adefault("default");
	if(Output()) SetOutputDir((TDirectory*)Selector()->GetFile(adefault)->Get(fName.Data()));
	else SetOutputDir((TDirectory*)Selector()->GetFile(adefault));

	//Clear list of trees to be filled
	fFillTrees.clear();

	static const AnaKey a_Process_ClearEvent(":Process:ClearEvent");
	AnaKey aName_Process_ClearEvent(fName+a_Process_ClearEvent);
	Time()->Start(aName_Process_ClearEvent);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin ClearEvent() execution on analysis " << fName << endl;
	ClearEvent(); //Clear previous event
	if(Debug()) cout << "AnalysisBase: DEBUG Finish ClearEvent() execution on analysis " << fName << endl;

	Time()->Stop(aName_Process_ClearEvent);

	static const AnaKey a_Process_CopyEvent(":Process:CopyEvent");
	AnaKey aName_Process_CopyEvent(fName+a_Process_CopyEvent);
	Time()->Start(aName_Process_CopyEvent);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin CopyEvent() execution on analysis " << fName << endl;
	bool reject = !fEventBuilder->CopyEvent(this);
	if(Debug()) cout << "AnalysisBase: DEBUG Finish CopyEvent() execution on analysis " << fName << endl;

	Time()->Stop(aName_Process_CopyEvent);

	//Select appropriate output file if multi-output
	static const MomKey moutput_("output_");
	static const MomKey mChannelNumber("ChannelNumber");
	static const MomKey mRunNumber("RunNumber");
	MomKey moutput_key;

	TString key;
	if(Get(mChannelNumber,key)) moutput_key = moutput_ + MomKey(key); //R17
	else moutput_key = moutput_ + MomKey(String(mRunNumber)); //R16 and before, data

	static const MomKey mOutput("Output");
	if(Get(mOutput,key)) moutput_key = moutput_ + MomKey(key); //override RunNumber if desired
	if(DatasetCfg()->Exists(moutput_key)) {
		if(Output()) SetOutputDir((TDirectory*)Selector()->GetFile(DatasetCfg()->String(moutput_key))->Get(fName.Data()));
		else SetOutputDir((TDirectory*)Selector()->GetFile(DatasetCfg()->String(moutput_key)));
	}

	static const AnaKey a_Process_InitEvent(":Process:InitEvent");
	AnaKey aName_Process_InitEvent(fName+a_Process_InitEvent);
	Time()->Start(aName_Process_InitEvent);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin InitEvent() execution on analysis " << fName << endl;
	InitEvent(); //Compute things like weights..
	if(Debug()) cout << "AnalysisBase: DEBUG Finish InitEvent() execution on analysis " << fName << endl;

	Time()->Stop(aName_Process_InitEvent);

	//Count this event as processed even if rejected at EventBuilder level
	if(Output()) {
		SetPrefix();
		BookCutflow();
	}
	if(reject) {
		Time()->Stop(aName_Process);
		return false;
	}

	static const AnaKey a_Process_ProcessEvent(":Process:ProcessEvent");
	AnaKey aName_Process_ProcessEvent(fName+a_Process_ProcessEvent);
	Time()->Start(aName_Process_ProcessEvent);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin ProcessEvent() execution on analysis " << fName << endl;
	ProcessEvent();
	if(Debug()) cout << "AnalysisBase: DEBUG Finish ProcessEvent() execution on analysis " << fName << endl;

	Time()->Stop(aName_Process_ProcessEvent);

	Time()->Stop(aName_Process);
	return true;

}

void AnalysisBase::SlaveTerminate()
{
	// The SlaveTerminate() function is called after all entries or objects
	// have been processed. When running with PROOF SlaveTerminate() is called
	// on each slave server.
	static const AnaKey a_SlaveTerminate(":SlaveTerminate");
	AnaKey aName_SlaveTerminate(fName+a_SlaveTerminate);
	Time()->Start(aName_SlaveTerminate);

	static const AnaKey a_SlaveTerminate_WorkerTerminate(":SlaveTerminate:WorkerTerminate");
	AnaKey aName_SlaveTerminate_WorkerTerminate(fName+a_SlaveTerminate_WorkerTerminate);
	Time()->Start(aName_SlaveTerminate_WorkerTerminate);

	if(Debug()) cout << "AnalysisBase: DEBUG Begin WorkerTerminate() execution on analysis " << fName << endl;
	WorkerTerminate();
	if(Debug()) cout << "AnalysisBase: DEBUG Finish WorkerTerminate() execution on analysis " << fName << endl;

	Time()->Stop(aName_SlaveTerminate_WorkerTerminate);

	Time()->Stop(aName_SlaveTerminate);
	// worker timers
	cout << fName << " timers" << endl;
	Time()->ShowStopped();

}

void AnalysisBase::FillTree(AnaKey treename)
{
	fFillTrees.insert(treename);
}

void AnalysisBase::RegisterTree(AnaKey treename, bool isEvent) //only call during WorkerBegin()
{
	fTrees[treename]=isEvent;
}

void AnalysisBase::BookCutflow()
{
	static const AnaKey acutflow("cutflow");
	AnaKey name = fPrefix + acutflow;

	TH1D* cutflow = (TH1D*)OutputDir()->Get(name); //C-style cast okay, we know it'll be a TH1D if it exists

	if(!cutflow) {
		OutputDir()->cd();
		cutflow = new TH1D(name.Data(),"Events",1,-0.5,0.5);
		cutflow->Sumw2();
	}

	cutflow->Fill(0.,Weight());
}

void AnalysisBase::BookCutflow(AnaKey prefix)
{
	static const AnaKey null("");
	SetPrefix(prefix);
	if(prefix()!=null()) BookCutflow(); //Don't double fill with no prefix, already called once
}

void AnalysisBase::BookCutflowNoPrefix(AnaKey flowpoint)
{
	TH1D* cutflow = (TH1D*)OutputDir()->Get(flowpoint); //C-style cast okay, we know it'll be a TH1D if it exists

	if(!cutflow) {
		OutputDir()->cd();
		cutflow = new TH1D(flowpoint.Data(),"Events",1,-0.5,0.5);
		cutflow->Sumw2();
	}

	cutflow->Fill(0.,Weight());
}

TH1* AnalysisBase::Fill(AnaKey name, const double a)
{
	static TClass* th1cl = TH1::Class();

	AnaKey histoname(fPrefix+name);

	TObject* obj = OutputDir()->Get(histoname);

	if(!obj) Abort(TString::Format("AnalysisBase: ERROR No object %s found",histoname.Data()));

	if(obj->InheritsFrom(th1cl)) {
		TH1* th1 = static_cast<TH1*>(obj);
		th1->Fill(a,Weight());
		return th1;
	}

	TString msg("AnalysisBase: ERROR No valid object ");
	msg.Append(name);
	msg.Append(" found");
	Abort(msg);

	return 0;
}

TH1* AnalysisBase::Fill(AnaKey name, const double a, const double b)
{
	static TClass* th1cl = TH1::Class();
	static TClass* tpcl = TProfile::Class();
	static TClass* th2cl = TH2::Class();

	AnaKey histoname(fPrefix+name);

	TObject* obj = OutputDir()->Get(histoname);

	if(!obj) Abort(TString::Format("AnalysisBase: ERROR No object %s found",histoname.Data()));

	//TProfile/TH1 order matters
	if(obj->InheritsFrom(th2cl)) {
		TH2* th2 = static_cast<TH2*>(obj);
		th2->Fill(a,b,Weight());
		return th2;
	}

	if(obj->InheritsFrom(tpcl)) {
		TProfile* tp = static_cast<TProfile*>(obj);
		tp->Fill(a,b,Weight());
		return tp;
	}

	if(obj->InheritsFrom(th1cl)) {
		TH1* th1 = static_cast<TH1*>(obj);
		th1->Fill(a,Weight()*b);
		return th1;
	}

	TString msg("AnalysisBase: ERROR No valid object ");
	msg.Append(name);
	msg.Append(" found");
	Abort(msg);

	return 0;
}

TH1* AnalysisBase::Fill(AnaKey name, const double a, const double b, const double c)
{
	static TClass* th2cl = TH2::Class();
	static TClass* th3cl = TH3::Class();
	static TClass* tp2Dcl = TProfile2D::Class();
	static TClass* tpcl = TProfile::Class();

	AnaKey histoname(fPrefix+name);

	TObject* obj = OutputDir()->Get(histoname);

	if(!obj) Abort(TString::Format("AnalysisBase: ERROR No object %s found",histoname.Data()));

	//most likely first, TProfile2D/TH2D order matters
	if(obj->InheritsFrom(th3cl)) {
		TH3* th3 = static_cast<TH3*>(obj);
		th3->Fill(a,b,c,Weight());
		return th3;
	}

	if(obj->InheritsFrom(tp2Dcl)) {
		TProfile2D* tp2D = static_cast<TProfile2D*>(obj);
		tp2D->Fill(a,b,c,Weight());
		return tp2D;
	}

	if(obj->InheritsFrom(th2cl)) {
		TH2* th2 = static_cast<TH2*>(obj);
		th2->Fill(a,b,Weight()*c);
		return th2;
	}

	if(obj->InheritsFrom(tpcl)) {
		TProfile* tp = static_cast<TProfile*>(obj);
		tp->Fill(a,b,Weight()*c);
		return tp;
	}

	TString msg("AnalysisBase: ERROR No valid object ");
	msg.Append(name);
	msg.Append(" found");
	Abort(msg);

	return 0;
}

TH1* AnalysisBase::Fill(AnaKey name, const double a, const double b, const double c, const double d)
{
	static TClass* th3cl = TH3::Class();
	static TClass* tp2Dcl = TProfile2D::Class();

	AnaKey histoname(fPrefix+name);

	TObject* obj = OutputDir()->Get(histoname);

	if(!obj) Abort(TString::Format("AnalysisBase: ERROR No object %s found",histoname.Data()));

	if(obj->InheritsFrom(th3cl)) {
		TH3* th3 = static_cast<TH3*>(obj);
		th3->Fill(a,b,c,Weight()*d);
		return th3;
	}

	if(obj->InheritsFrom(tp2Dcl)) {
		TProfile2D* tp2D = static_cast<TProfile2D*>(obj);
		tp2D->Fill(a,b,c,Weight()*d);
		return tp2D;
	}

	TString msg("AnalysisBase: ERROR No valid object ");
	msg.Append(name);
	msg.Append(" found");
	Abort(msg);

	return 0;
}

void AnalysisBase::FixSumw2() {

	TIter nextkey(OutputDir()->GetList());
	while ( TObject *obj = nextkey() ) {
		if ( obj->IsA()->InheritsFrom( "TH1" ) ) {
			// descendant of TH1
			TH1 *h1 = (TH1*)obj;

			//check if sumw2 is set (from TH1::Sumw2() source file)
			if (h1->GetSumw2N()) {
				continue;
			}
			h1->Sumw2();
		}
	}
}
