//*****************************************************
// Class   : SignalGrid1l2b
//
// Purpose : Make pretty 2D plots of 1l2b signal grid
//
// Author  : B. Butler
//
// Date    : 7-16-13 : Creation of first class
//*****************************************************
#ifndef SignalGrid1l2b_h
#define SignalGrid1l2b_h

#include "TString.h"
#include <TH2Poly.h>
#include <TStyle.h>
#include <map>
#include <vector>
#include <set>

using std::vector;
using std::map;
using std::set;

class TCanvas;

class SignalGrid1l2b {

	public:
		SignalGrid1l2b();
		~SignalGrid1l2b() {Reset(); delete m_histo; delete m_style;};

		void Reset();

		//massx, massy, weight
		void FillBin(float x, float y, float w);

		void SetATLASLabel(const TString& atlaslabel) {m_atlaslabel = atlaslabel;}
		void ShowATLASLabel(bool atlas = true) {m_atlas = atlas;}
		void SetLumi(const TString& lumi) {m_lumi = lumi;} //must include units, i.e. 165 pb^{-1}
		void ShowLumi(bool showlumi = true) {m_showlumi = showlumi;}
		void SetLabel(const TString& label) {m_label = label;}
		void SetAxisLabelSize(float labelsize) {m_labelsize = labelsize;}
		void SetTitleSize(float titlesize) {m_titlesize = titlesize;}
		void SetXTitleOffset(float titleoffset) {m_titleoffsetx = titleoffset;}
		void SetXLabelOffset(float labeloffset) {m_labeloffsetx = labeloffset;}
		void SetYTitleOffset(float titleoffset) {m_titleoffsety = titleoffset;}
		void SetYLabelOffset(float labeloffset) {m_labeloffsety = labeloffset;}
		void SetTextSize(float textsize) {m_textsize = textsize;}
		void SetTextOffset(float offset) {m_xoffset = offset;}
		void SetCenterXTitle(bool center) {m_xtitlecenter = center;}

		void SetXTitle(const TString& xtitle) {m_xtitle = xtitle;}
		void SetYTitle(const TString& ytitle) {m_ytitle = ytitle;}

		void SetCanvasX(int max) {m_canvasx = max;} //negative numbers in either X or Y mean default TCanvas constructor
		void SetCanvasY(int max) {m_canvasy = max;}

		void SetLogZ(bool logz) {m_logz = logz;}

		TH2Poly* GetHisto() { return m_histo; }

		TCanvas* Plot();

	private:

		bool m_atlas,m_showlumi,m_logz,m_xtitlecenter;
		float m_labelsize,m_textsize,m_titlesize,m_xoffset,m_titleoffsetx,
			m_labeloffsetx,m_titleoffsety,m_labeloffsety,m_canvasx,m_canvasy;
		TString m_label,m_atlaslabel,m_lumi,m_xtitle,m_ytitle;

		float Nm(float x, float y) { return (x-y)/2.; }
		float Cm(float x, float y) { return (x+y)/2.; }

		TCanvas* m_canvas;
		TStyle*  m_style;

		TH2Poly* m_histo;
};

#endif /* SignalGrid1l2b */
