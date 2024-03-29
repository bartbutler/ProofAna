/**************************************************************************
 **
 **   File:         AnalysisBase.h
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

#ifndef AnalysisBase_h
#define AnalysisBase_h

#include "TROOT.h"
#include "Timer.h"
#include "Event.h"
#include "Key.h"
#include "TDirCache.h"
#include <cmath>

class SelectorBase;
class EventBuilderBase;
class Config;
class TFile;
class TTree;
class TH1;

class AnalysisBase : public Event {
public :

	AnalysisBase() : fSelector(0), fEventBuilder(0), fConfig(0), fChainConfig(0), fDatasetConfig(0), fInput(0),
		fTimer(new Timer()), fOutputDir(0), fSaveTimers(false), fDebug(false), fOutput(true), fName(""), fPrefix("") { }
	virtual 			~AnalysisBase();
	virtual void    	SlaveBegin(TTree* tree);
	virtual Bool_t  	Process();
	virtual void    	SlaveTerminate();

	//Set inputs
	virtual void    	SetInputList(TList* input) { fInput = input; }
	virtual void    	SetConfig(Config* config);
	virtual void    	SetChainConfig(Config* config) { fChainConfig = config; }
	virtual void    	SetDatasetConfig(Config* config) { fDatasetConfig = config; }
	virtual void    	SetEventBuilder(EventBuilderBase* eb) {fEventBuilder = eb;}
	virtual void    	SetOutputDir(TDirectory* dir);
	virtual void    	SetOutput(bool output) {fOutput = output;}
    virtual void		SetSelector(SelectorBase* selector) { fSelector = selector; }
    virtual void		SetTimerOutput(TDirectory* dir);

	//Histogramming functions
	void				SetPrefix() { static AnaKey null(""); fPrefix = null;}
	void				SetPrefix(AnaKey prefix) { static AnaKey undersc("_"); fPrefix = prefix + undersc;}
	TH1*				Fill(AnaKey name, const double a);
	TH1*				Fill(AnaKey name, const double a, const double b);
	TH1*				Fill(AnaKey name, const double a, const double b, const double c);
	TH1*				Fill(AnaKey name, const double a, const double b, const double c, const double d);
	void				FixSumw2();

	void				BookCutflow();
	void				BookCutflow(AnaKey name); //same as BookCutflow() except calls SetPrefix() first
	void				BookCutflowNoPrefix(AnaKey name); //same as BookCutflow(AnaKey name) except specifies name without changing prefixes


    //Write TTree with Event class or direct skim of underlying ntuple
	void FillTree(AnaKey treename); //call during ProcessEvent to select an event to be written to tree
	void RegisterTree(AnaKey treename, bool isEvent = true); //only call during WorkerBegin()
	void ClearFillTrees() { fFillTrees.clear(); }

	//Reset object cache
	virtual void		FlushCache(TDirectory* file);

	//Member access functions
    SelectorBase*		Selector() { return fSelector; }
    EventBuilderBase*	Eb() { return fEventBuilder; }
	TList*		      	InputList() {return fInput;}
	TDirCache*	 		OutputDir() {return fOutputDir;}
	Timer*      		Time() {return fTimer;}
	Config*		     	Cfg() {return fConfig;}
	Config*		     	ChainCfg() {return fChainConfig ? fChainConfig: fConfig;}
	Config*		     	DatasetCfg() {return fDatasetConfig;}
	Bool_t				SaveTimers() {return fSaveTimers;}
	void				SetDebug(bool debug) {fDebug = debug;} //For override, normally set in configuration
	Bool_t      		Debug() {return fDebug;}
	Bool_t				Output() {return fOutput;}
    AnaKey				Name() {return fName;}
    AnaKey				Prefix() {return fPrefix;}
    const map<AnaKey, bool>& Trees() const {return fTrees;}
    const set<AnaKey>& FillTrees() const {return fFillTrees;}

	//Analysis code
	virtual bool    	ProcessEvent() {/* To be overridden by derived analysis class */ return kTRUE; }
	virtual void    	WorkerBegin() {/* To be overridden by derived analysis class */}
	virtual void    	WorkerTerminate() {/* To be overridden by derived analysis class */}

	ClassDef(AnalysisBase,0);

private :

	SelectorBase*		fSelector;
	EventBuilderBase* 	fEventBuilder;
	Config* 			fConfig;
	Config* 			fChainConfig;
	Config*				fDatasetConfig;
	TList* 				fInput;
	Timer* 				fTimer;
	TDirCache*			fOutputDir;

	map<TDirectory*,TDirCache*> fCacheMap;

	map<AnaKey,bool>	fTrees;
	set<AnaKey>			fFillTrees;

	Bool_t 				fSaveTimers;
	Bool_t 				fDebug;
	Bool_t				fOutput;
	AnaKey	 			fName;
	AnaKey	 			fPrefix;

};

#endif

