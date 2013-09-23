//*****************************************************
// Class   : EventDisplay2D
// 
// Purpose : 2D event displays, initially for tracks
//                      
// Author  : B. Butler, N. Sennett
//
// Date    : 7-8-11 : Creation of class
//*****************************************************

#ifndef EventDisplay2D_h
#define EventDisplay2D_h

#include <iostream>
#include "TString.h"
#include <map>
#include <vector>

using std::vector;
using std::map;
using std::cout;
using std::endl;

class TCanvas;
class TH2F;

#include "TLine.h"
#include "TMath.h"
#include "TROOT.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TObjArray.h"
#include "TObjString.h"

class Track {

	public:
		
		Track() { m_color = 1; m_charge = 1; m_style = 1; m_line = 0;}
		Track(const TLorentzVector& tlv) : p(tlv) { m_color = 1; m_charge = 1; m_style = 1; m_line = 0; }
		Track(const TLorentzVector& tlv, const TVector3& vtx) : p(tlv), x(vtx) { m_color = 1; m_charge = 1; m_style = 1; m_line = 0; }
		virtual ~Track() { if(m_line) delete m_line; }
		
		void DrawXY(TString option) {
			float zoom = -1.;
			float trkptscale = -1.; 
		
			TObjArray* optionarray = option.Tokenize(" ");
			for(int i = 0; i<optionarray->GetEntries(); i++) {
				TObjString* obj = (TObjString*)optionarray->At(i);
				if(obj->GetString().Contains("zoom=")) {
					zoom = TString(obj->GetString()).ReplaceAll("zoom=","").Atof();
				}
				else if(obj->GetString().Contains("ptscale=")) {
					trkptscale = TString(obj->GetString()).ReplaceAll("ptscale=","").Atof();
				}
			}
			delete optionarray;
			
			if(zoom<0) { 
				cout << "Error, no zoom parameter" << endl;
				return;
			}
			if(trkptscale<0) { 
				cout << "Error, no ptscale parameter" << endl;
				return;
			}
		
			if(m_line) delete m_line;
			
			float endx = p.Px()/trkptscale+x.X();
			float endy = p.Py()/trkptscale+x.Y();
		
			if(TMath::Abs(endx)>zoom) {
				endx = (endx*zoom)/TMath::Abs(endx);
				trkptscale = p.Px()/(endx-x.X());
				endy = p.Py()/trkptscale+x.Y();
			}
			if(TMath::Abs(endy)>zoom) {
				endy = (endy*zoom)/TMath::Abs(endy);
				trkptscale = p.Py()/(endy-x.Y());
				endx = p.Px()/trkptscale+x.X();
			}			
			
			m_line = new TLine(x.X(),x.Y(),endx,endy);
			m_line->SetLineColor(m_color);
			m_line->SetLineStyle(m_style);
			m_line->Draw();
		}
		
		int Charge() const {return m_charge;}
		int Color() const {return m_color;}
		int Style() const {return m_style;}

		void Charge(const int& charge) {m_charge = charge;}
		void Color(const int& color) {m_color = color;}
		void Style(const int& style) {m_style = style;}
		
		TLorentzVector p;
		TVector3 x;
		
	private:
	
		int m_color, m_charge, m_style;
		TLine* m_line;

};

class EventDisplay2D  {

	public:
		
		EventDisplay2D() { m_frame = 0; }
		virtual ~EventDisplay2D();
		
		void AddTrack(Track track) {m_tracks.push_back(new Track(track));}
		
		TCanvas* PlotXY(TString option = "");
		
		void Reset(); 
		
		ClassDef(EventDisplay2D,0)
   
	private:
		
		vector<Track*> m_tracks;
		TH2F* m_frame;

};

#endif
