//*****************************************************
// Class   : EventDisplay2D
// 
// Purpose : 2D event displays, initially for tracks
//                      
// Author  : B. Butler, N. Sennett
//
// Date    : 7-8-11 : Creation of class
//*****************************************************

#ifndef EventDisplay2D_cxx
#define EventDisplay2D_cxx

#include "EventDisplay2D.h"
#include "TCanvas.h"
#include "TH2.h"

EventDisplay2D::~EventDisplay2D() 
{
	Reset();
}

void EventDisplay2D::Reset() 
{
	if(m_frame) {
		delete m_frame;
		m_frame = 0;
	}

	vector<Track*>::iterator iter = m_tracks.begin();
	for(; iter!=m_tracks.end(); iter++) delete (*iter);	
	
	m_tracks.clear();
}

TCanvas* EventDisplay2D::PlotXY(TString option)
{
		float zoom = -1.;
		float ptscale = -1.;
		bool ptview = false;
		
		TObjArray* optionarray = option.Tokenize(" ");
		for(int i = 0; i<optionarray->GetEntries(); i++) {
			TObjString* obj = (TObjString*)optionarray->At(i);
			if(obj->GetString().Contains("zoom=")) {
				zoom = TString(obj->GetString()).ReplaceAll("zoom=","").Atof();
			}
			else if(obj->GetString().Contains("ptscale=")) {
				ptscale = TString(obj->GetString()).ReplaceAll("ptscale=","").Atof();
			}
			else if(obj->GetString().Contains("ptview")) {
				ptview = true;
			}
		}
		if(zoom<0) {
			zoom = 10.; //10mm default
			option.Append(" zoom=10");	
		}
		if(ptscale<0) {
			if(ptview) {
				option.Append(" ptscale=1");	
				ptscale = 1;
			}
			else {
				option.Append(" ptscale=5"); //1 mm/5 GeV track pT	
				ptscale = 5;
			}
		}

	    TCanvas *c1 = (TCanvas*)gROOT->FindObject("c1");
    	if (c1) {delete c1;} 
    	c1 = new TCanvas("c1","PlotXY",0,0,600,600);

		c1->SetGrid();
		c1->cd();
		
		if(m_frame) delete m_frame;
		if(ptview) { 
			m_frame = new TH2F("frame","frame",2,-zoom,zoom,2,-zoom,zoom);
			m_frame->GetXaxis()->SetTitle("Px (GeV)");
			m_frame->GetYaxis()->SetTitle("Py (GeV)");
		}
		else {
			m_frame = new TH2F("frame","frame",2,-zoom,zoom,2,-zoom,zoom);
			m_frame->GetXaxis()->SetTitle("X (mm)");
			m_frame->GetYaxis()->SetTitle("Y (mm)");			
		}
		m_frame->Draw();

		c1->AbsCoordinates(true);
		
		vector<Track*>::iterator iter = m_tracks.begin();
		for(; iter!=m_tracks.end(); iter++) (*iter)->DrawXY(option);	
		
		return c1;
}

#endif
