//*****************************************************
// Class   : Stacked
//
// Purpose : Stacked 1D plots, memory-leak free!
//
// Author  : B. Butler
//
// Date    : 6-13-11 : Creation of first class
//*****************************************************
#ifndef Stacked_h
#define Stacked_h

#include "TString.h"
#include <map>
#include <vector>

using std::vector;
using std::map;

class TH1;
class TCanvas;
class TPad;
class THStack;
class TLine;
class TGraphAsymmErrors;
class TLegend;
class TStyle;

class Stacked {

	public:
		Stacked();
		~Stacked() {Reset();};

		void Reset();

		void AddMC(TH1* histo, const TString& legend, int color, int rebin = 1, int style = 1001);
		void AddData(TH1* histo, const TString& legend, int color = 1, int rebin = 1);
		void AddSignal(TH1* histo, const TString& legend, int color, int rebin = 1);

		void SetATLASLabel(const TString& atlaslabel) {m_atlaslabel = atlaslabel;}
		void ShowATLASLabel(bool atlas = true) {m_atlas = atlas;}
		void SetLumi(const TString& lumi) {m_lumi = lumi;} //must include units, i.e. 165 pb^{-1}
		void ShowLumi(bool showlumi = true) {m_showlumi = showlumi;}
		void SetLabel(const TString& label) {m_label = label;}
		void SetRatioLabel(const TString& label) {m_ratiolabel = label;}
		void SetBinWidthUnits(const TString& units) {m_units = units;}
		void SetSMErrorColor(int color) {m_errorcolor = color;}
		void SetSMErrorStyle(int style) {m_errorstyle = style;}
		void SetSMRatioErrorStyle(int style) {m_ratioerrorstyle = style;}
		void SetSMTotalRatioColor(int color) {m_smtotalratiocolor = color;}
		void SetLineWidth(int width) {m_linewidth = width;}
		void SetDataLegStyle(const TString& style) {m_datalegstyle = style;}
		void SetNdivisions(int ndiv) {m_ndiv = ndiv;}
		void ForceNdivisions(bool force) {m_forcendiv = force;}
		void SetAxisLabelSize(float labelsize) {m_labelsize = labelsize;}
		void SetTitleSize(float titlesize) {m_titlesize = titlesize;}
		void SetXTitleOffset(float titleoffset) {m_titleoffsetx = titleoffset;}
		void SetXLabelOffset(float labeloffset) {m_labeloffsetx = labeloffset;}
		void SetYTitleOffset(float titleoffset) {m_titleoffsety = titleoffset;}
		void SetYLabelOffset(float labeloffset) {m_labeloffsety = labeloffset;}
		void SetTextSize(float textsize) {m_textsize = textsize;}
		void SetTextOffset(float offset) {m_xoffset = offset;}
		void SetCenterXTitle(bool center) {m_xtitlecenter = center;}
		void DoRatioErrorEdges(bool edges) {m_ratioerroredges = edges;}
		void SetRatioMax(float ratiomax) {m_ratiomax = ratiomax;}
		void SetNdivisionsRatio(int ndiv) {m_ndivratio = ndiv;}
		void SetLegX(float legx) {m_legx = legx;}
		void SetLegY(float legy) {m_legy = legy;}

		void SetXRange(float min, float max) {m_xmax = max; m_xmin = min;}
		void SetYRange(float min, float max) {m_ymax = max; m_ymin = min;}

		void SetXTitle(const TString& xtitle) {m_xtitle = xtitle;}
		void SetYTitle(const TString& ytitle) {m_ytitle = ytitle;}

		void SetCanvasX(int max) {m_canvasx = max;} //negative numbers in either X or Y mean default TCanvas constructor
		void SetCanvasY(int max) {m_canvasy = max;}

		void SetLogX(bool logx) {m_logx = logx;}
		void SetLogY(bool logy) {m_logy = logy;}
		void SetAutoMaxY(float automaxy) {m_automaxy = automaxy;}

		void DoSMTotal(bool doSM) {m_doSMTotal = doSM;}
		void DoOnlySMTotal(bool doOnlySM) {m_doOnlySMTotal = doOnlySM;}
		void ShowEventCount(bool showEventCount) {m_showEventCount = showEventCount;}

		void DoPoisson(bool doPoisson) {m_doPoisson = doPoisson;}

		TCanvas* Plot(bool ratio = true, bool stack = true, TString sys = "all");

	private:

		void ClearMemory();

		bool m_doSMTotal,m_doOnlySMTotal,m_showEventCount,m_atlas,m_showlumi,m_logx,m_logy,m_doPoisson,m_xtitlecenter,m_ratioerroredges,m_forcendiv;
		int m_errorstyle,m_ratioerrorstyle,m_errorcolor,m_ndiv,m_ndivratio,m_linewidth,m_smtotalratiocolor,m_canvasx,m_canvasy;
		float m_ymax,m_ymin,m_xmax,m_xmin,m_automaxy,m_labelsize,m_textsize,m_titlesize,m_xoffset,m_ratiomax,m_titleoffsetx,
			m_labeloffsetx,m_titleoffsety,m_labeloffsety,m_legx,m_legy;
		TString m_label,m_atlaslabel,m_lumi,m_xtitle,m_ytitle,m_ratiolabel,m_units,m_datalegstyle;

		TCanvas* m_canvas;
		TPad*    m_upperPad;
		TPad*    m_lowerPad;
		TLegend* m_legend;
		TStyle*  m_style;

		TH1* m_mc_sum;
		TH1* m_temp;
		vector<TH1*> m_mc_histo;
		vector<TH1*> m_mc_stackedhisto;
		vector<TString> m_mc_legend;
		vector<int> m_mc_color;
		vector<int> m_mc_style;

		vector<TH1*> m_data_histo;
		vector<TString> m_data_legend;
		vector<TGraphAsymmErrors*> m_data_graph;
		vector<TGraphAsymmErrors*> m_data_ratio_graph;

		vector<TH1*> m_signal_histo;
		vector<TString> m_signal_legend;

		TH1* m_ratio;
		TLine* m_line;
		vector<TLine*> m_errorlines;
		TGraphAsymmErrors* m_ratioerrors;
		TH1* m_ratioerrorlow;
		TH1* m_ratioerrorhigh;
		TGraphAsymmErrors* m_errors;
};

#endif /* Stacked */
