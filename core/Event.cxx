/**************************************************************************
 **
 **   File:         Event.cxx
 **
 **   Description:  Event class
 **
 **
 **   Author:       B. Butler
 **
 **   Created:      3-7-11
 **   Modified:
 **
 **************************************************************************/

#define Event_cxx

#include "Event.h"
#include "AnaConfig.h"

Event::~Event()
{

	// Destructor

}

void Event::ResetDefaults()
{
	static const MomKey leptons("leptons"), electrons("electrons"), muons("muons"), jets("jets"),
		vtxs("vtxs"), tracks("tracks"), photons("photons"), clusters("clusters"),
		truths("truths"), recos("recos"), mx("Metx"), my("Mety");

	fDefaultLeptons = MomKey(leptons);
	fDefaultElectrons = MomKey(electrons);
	fDefaultMuons = MomKey(muons);
	fDefaultJets = MomKey(jets);
	fDefaultVtxs = MomKey(vtxs);
	fDefaultTracks = MomKey(tracks);
	fDefaultPhotons = MomKey(photons);
	fDefaultClusters = MomKey(clusters);
	fDefaultTruths = MomKey(truths);
	fDefaultRecos = MomKey(recos);
	fDefaultMetx = MomKey(mx);
	fDefaultMety = MomKey(my);

}

void Event::SetTruthDefaults()
{
	static const MomKey leptons("leptonstruth"), electrons("electronstruth"), muons("muonstruth"), jets("jetstruth"),
		vtxs("vtxstruth"), tracks("trackstruth"), photons("photonstruth"), clusters("clusterstruth"),
		truths("truthstruth"), recos("recostruth"), mx("Metxtruth"), my("Metytruth");

	fDefaultLeptons = MomKey(leptons);
	fDefaultElectrons = MomKey(electrons);
	fDefaultMuons = MomKey(muons);
	fDefaultJets = MomKey(jets);
	fDefaultVtxs = MomKey(vtxs);
	fDefaultTracks = MomKey(tracks);
	fDefaultPhotons = MomKey(photons);
	fDefaultClusters = MomKey(clusters);
	fDefaultTruths = MomKey(truths);
	fDefaultRecos = MomKey(recos);
	fDefaultMetx = MomKey(mx);
	fDefaultMety = MomKey(my);

}

void Event::ClearEvent()
{
	Reset();
}

void Event::InitEvent()
{
	fLumi = 0;

	bool MCweights = true;
	static const MomKey mcws("MCWEIGHTS");
	Cfg()->Get(mcws,MCweights);

	bool evtweights = true;
	static const MomKey ews("EVTWEIGHTS");
	Cfg()->Get(ews,evtweights);

	static const MomKey ew("EventWeight"), lum("LUMI"), eb("EVENTBUILDER");

	static const bool isEvent = !(DatasetCfg()->String(eb).CompareTo("event"));
	if(!isEvent) {
		//Compute/reset weight
		if((fPrevChannelNumber!=ChannelNumber())||fFirstEvent) {

			fPrevChannelNumber=ChannelNumber();

			if(MCweights && isMC()) ComputeCrossSectionWeight();
			else fCrossSectionWeight = 1.;
		}
		fDefaultWeight = fCrossSectionWeight;
		if(MCweights && isMC()) fLumi = Cfg()->Double(lum);

		//do Mc@Nlo/overlap/trigger prescale weights (weights you want applied regardless of analysis)
		if(Exists(ew) && evtweights) fDefaultWeight*=Float(ew);
	}
	else { //Reweight luminosity, cross-section if info available

		if(MCweights && isMC()) {
			TString xs_str = TString::Format("xs_%i",ChannelNumber());
			double xs = 0.;
			if(DatasetCfg()->Get(xs_str,xs)) ComputeCrossSectionWeight(); //Only recompute if info available (skim has config file)
			else if(fLumi>0.) {
				fCrossSectionWeight*=(Cfg()->Double(lum)/fLumi); //Rescale luminosity, leaving cross section calculation unchanged
			}
			else Abort("Event: ERROR Attempted luminosity reweighting on MC missing cross section weights. Supply a config file with cross section information");
			fLumi = Cfg()->Double(lum);
		}
		else fCrossSectionWeight = 1.; //Remove cross-section weights if desired

		fDefaultWeight = fCrossSectionWeight;

		//do Mc@Nlo/overlap/trigger prescale weights (weights you want applied regardless of analysis)
		if(Exists(ew)&&evtweights) fDefaultWeight*=Float(ew);
	}

	fWeight = fDefaultWeight;

	if(fFirstEvent) fFirstEvent = false;

}

void Event::ComputeCrossSectionWeight()
{

	double xs = 0.;
	double k = 1.;
	double eff = 1.;
	double n = 0;
	static const MomKey mLUMI("LUMI");
	static const MomKey mxs_("xs_");
	static const MomKey mk_("k_");
	static const MomKey meff_("eff_");
	static const MomKey mn_("n_");

	MomKey chnum(String("ChannelNumber"));

	if(!DatasetCfg()->Get(mxs_+chnum,xs) || !DatasetCfg()->Get(mn_+chnum,n)) {
		TString msg("Event: WARNING Missing cross-section or number of entries information for MC process number ");
		msg.Append(String("ChannelNumber"));
		msg.Append(", set MCWEIGHTS to false if you do not want cross-section/luminosity weighting");
		Abort(msg);
	}

	//k-factor (optional)
	DatasetCfg()->Get(mk_+chnum,k);
	//efficiency (optional)
	DatasetCfg()->Get(meff_+chnum,eff);

	fCrossSectionWeight = (Cfg()->Double(mLUMI)*xs*eff*k)/n;

}

bool Event::isMC()
{
	bool isMC;
	static const MomKey ismc("isMC"), chn("ChannelNumber"), rn("RunNumber");

	if(Get(ismc,isMC)) return isMC;
	int channelnum;
	if(Get(chn,channelnum)) { //R17
		if(channelnum!=Int(rn)) return true;
		else return false;
	}
	else return (RunNumber()<152166); //backwards compatibility, release 16 and before
}

