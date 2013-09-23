//*****************************************************
// Class   : Stacked
//
// Purpose : Stacked 1D plots, memory-leak free!
//
// Author  : B. Butler
//
// Date    : 6-13-11 : Creation of first class
//*****************************************************

#include "THX.h" //make use of proper systematics, should be transparent for regular histograms with only statistical errors
#include "TLine.h"
#include "TCanvas.h"
#include "TPad.h"
#include "THStack.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TMath.h"
#include "AtlLabels.h"
#include <iostream>

#include "Stacked.h"

//Constructor
Stacked::Stacked() {

	m_canvas = 0;
	m_upperPad = 0;
	m_lowerPad = 0;
	m_ratio = 0;
	m_mc_sum = 0;
	m_temp = 0;
	m_line = 0;
	m_ratioerrors = 0;
	m_ratioerrorlow = 0;
	m_ratioerrorhigh = 0;
	m_errors = 0;
	m_legend = 0;
	m_style = 0;

	m_mc_stackedhisto.clear();

	m_mc_histo.clear();
	m_mc_legend.clear();
	m_mc_color.clear();
	m_mc_style.clear();

	m_data_histo.clear();
	m_data_legend.clear();

	m_signal_histo.clear();
	m_signal_legend.clear();

	m_errorlines.clear();

	Reset();
}

//Clean up non-input-histogram memory usage
void Stacked::ClearMemory() {

	if(m_canvas) {
		delete m_canvas;
		m_canvas = 0;
		m_upperPad = 0;
		m_lowerPad = 0;
	}
	//if(m_upperPad) {delete m_upperPad; m_upperPad = 0;}
	//if(m_lowerPad) {delete m_lowerPad; m_lowerPad = 0;}
	if(m_ratio) {delete m_ratio; m_ratio = 0;}
	if(m_mc_sum) {delete m_mc_sum; m_mc_sum = 0;}
	if(m_temp) {delete m_temp; m_temp = 0;}
	if(m_ratioerrors) {delete m_ratioerrors; m_ratioerrors = 0;}
	if(m_ratioerrorlow) {delete m_ratioerrorlow; m_ratioerrorlow = 0;}
	if(m_ratioerrorhigh) {delete m_ratioerrorhigh; m_ratioerrorhigh = 0;}
	if(m_errors) {delete m_errors; m_errors = 0;}
	if(m_legend) {delete m_legend; m_legend = 0;}
	if(m_style) {delete m_style; m_style = 0;}
	if(m_line) {delete m_line; m_line = 0;}

	vector<TH1*>::iterator th1 = m_mc_stackedhisto.begin();
	for(; th1!=m_mc_stackedhisto.end(); th1++) delete *th1;
	m_mc_stackedhisto.clear();

	vector<TLine*>::iterator line = m_errorlines.begin();
	for(; line!=m_errorlines.end(); line++) delete *line;
	m_errorlines.clear();

	vector<TGraphAsymmErrors*>::iterator graph = m_data_graph.begin();
	for(; graph<m_data_graph.end(); graph++) delete *graph;
	m_data_graph.clear();

	graph = m_data_ratio_graph.begin();
	for(; graph<m_data_ratio_graph.end(); graph++) delete *graph;
	m_data_ratio_graph.clear();

}

void Stacked::Reset() {

	ClearMemory();

	m_doSMTotal = true;
	m_doOnlySMTotal = false;
	m_showEventCount = false;
	m_logx = false;
	m_logy = true;
	m_doPoisson = true;
	m_ratioerroredges = false;
	m_units = "";
	m_lumi = "";
	m_label = "";
	m_ratiolabel = "Data/MC";
	m_atlaslabel = "";
	m_atlas = true;
	m_showlumi = true;
	m_xtitle = "";
	m_ytitle = "";
	m_ymax = -1.;
	m_ymin = -1.;
	m_xmax = -1.;
	m_xmin = -1.;
	m_automaxy = -1.;
	m_errorstyle = 3001;
	m_ratioerrorstyle = -1;
	m_errorcolor = kYellow;
	m_smtotalratiocolor = kRed;
	m_datalegstyle = "elp";
	m_ndiv = 510;
	m_forcendiv = false;
	m_linewidth = 2;
	m_labelsize = 0.05;
	m_textsize = 0.05;
	m_titlesize = 0.05;
	m_titleoffsetx = 0.95;
	m_labeloffsetx = 0.001;
	m_titleoffsety = 1.3;
	m_labeloffsety = 0.001;
	m_xoffset = 0.2;
	m_ratiomax = 2.5;
	m_ndivratio = 510;
	m_xtitlecenter = false;
	m_canvasx = 650;
	m_canvasy = 650;
	m_legx = 0.64;
	m_legy = 0.96;

	//delete histograms here, we own them
	vector<TH1*>::iterator iter;

	for(iter = m_mc_histo.begin(); iter!=m_mc_histo.end(); iter++) delete *iter;
	m_mc_histo.clear();
	m_mc_legend.clear();
	m_mc_color.clear();
	m_mc_style.clear();

	for(iter = m_data_histo.begin(); iter!=m_data_histo.end(); iter++) delete *iter;
	m_data_histo.clear();
	m_data_legend.clear();

	for(iter = m_signal_histo.begin(); iter!=m_signal_histo.end(); iter++) delete *iter;
	m_signal_histo.clear();
	m_signal_legend.clear();
}

void Stacked::AddMC(TH1* histo, const TString& legend, int color, int rebin, int style) {
	TH1* hist = (TH1*)histo->Clone();
	hist->SetName(TString::Format("mc_%i",(int)m_mc_histo.size()));
	hist->Rebin(rebin);
	m_mc_histo.push_back(hist);
	m_mc_legend.push_back(legend);
	m_mc_color.push_back(color);
	m_mc_style.push_back(style);
}

void Stacked::AddData(TH1* histo, const TString& legend, int color, int rebin) {
	TH1* hist = (TH1*)histo->Clone();
	hist->SetName(TString::Format("data_%i",(int)m_data_histo.size()));
	hist->Rebin(rebin);
	hist->SetMarkerColor(color);
	hist->SetLineColor(color);
	m_data_histo.push_back(hist);
	m_data_legend.push_back(legend);
}

void Stacked::AddSignal(TH1* histo, const TString& legend, int color, int rebin) {
	TH1* hist = (TH1*)histo->Clone();
	hist->SetName(TString::Format("signal_%i",(int)m_signal_histo.size()));
	hist->Rebin(rebin);
	hist->SetLineColor(color);
	m_signal_histo.push_back(hist);
	m_signal_legend.push_back(legend);
}

TCanvas* Stacked::Plot(bool ratio, bool stack, TString sys) {

	ClearMemory();

	if((!m_data_histo.size())||(!m_mc_histo.size())) ratio = false; //Don't do ratio if only MC or only data
	if(m_doOnlySMTotal) m_doSMTotal = true;
	if(m_doOnlySMTotal) stack = false; //stack doesn't make sense for this
	if((m_signal_histo.size())&&(!m_mc_histo.size()||(!stack&&!m_doSMTotal))) {
		cout << "AddSignal is for overlaying signal samples on top of MC, use AddMC instead if you have no background MC or are not stacking/showing total MC" << endl;
		return 0;
	}

	//Add together MC histograms
	if(m_mc_histo.size()) {
		//detemine input mc histogram type
		m_mc_sum = MakeTHX(m_mc_histo.at(0));
		m_mc_sum->SetName("mcsum");

		vector<TH1*>::iterator histiter = m_mc_histo.begin();
		histiter++;
		for(; histiter!=m_mc_histo.end(); histiter++) m_mc_sum->Add(*histiter);

		//Recompute systematic errors
		CastTHSys(m_mc_sum)->Recompute(sys);
	}

	vector<float> signalcount;
	vector<float> signalerror;
	//Signal overlay
	if(m_signal_histo.size()) {
		for(unsigned int i = 0; i<m_signal_histo.size(); i++) {
			THSys* thsys = CastTHSys(m_signal_histo.at(i));
			if(thsys) thsys->Recompute(sys);
			if(m_showEventCount) {
				double error;
				double integral = m_signal_histo.at(i)->IntegralAndError(0,-1,error);
				signalcount.push_back(integral);
				signalerror.push_back(error);
			}
			//m_signal_histo.at(i)->Add(m_mc_sum);
		}
	}

	//If AutoMaxY set, set the Y max automatically
	if(m_automaxy>0.) {
		double max = 0;

		vector<TH1*>::iterator iter;
		//Loop over signals, if exist, else MC
		if(m_signal_histo.size()) {
			for(iter = m_signal_histo.begin(); iter!=m_signal_histo.end(); iter++) {
				TH1* clone = (TH1*)(*iter)->Clone();
				clone->Add(m_mc_sum);
				double tempmax = clone->GetBinContent(clone->GetMaximumBin());
				if(tempmax>max) max = tempmax;
				delete clone;
			}
		}
		else if(m_mc_histo.size()) {
			if(stack||m_doSMTotal) {
				double tempmax = m_mc_sum->GetBinContent(m_mc_sum->GetMaximumBin());
				if(tempmax>max) max = tempmax;
			}
			else {
				for(iter = m_mc_histo.begin(); iter!=m_mc_histo.end(); iter++) {
					double tempmax = (*iter)->GetBinContent((*iter)->GetMaximumBin());
					if(tempmax>max) max = tempmax;
				}
			}
		}

		//Loop over data
		if(m_data_histo.size()) {
			for(iter = m_data_histo.begin(); iter!=m_data_histo.end(); iter++) {
				double tempmax = (*iter)->GetBinContent((*iter)->GetMaximumBin());
				if(tempmax>max) max = tempmax;
			}
		}

		if(m_logy) {
			if(m_ymin<0.) m_ymin = 0.35;
			m_ymax = TMath::Power(10.,TMath::Log10(max)/m_automaxy)*2.;
		}
		else {
			if(m_ymin<0.) m_ymin = 0.;
			m_ymax = max/m_automaxy;
		}
	}

	m_style = new TStyle();
	m_style->SetOptStat(0);
	Int_t icol=0;
	m_style->SetFrameBorderMode(icol);
	m_style->SetFrameFillColor(icol);
	m_style->SetCanvasBorderMode(icol);
	m_style->SetCanvasColor(icol);
	m_style->SetPadBorderMode(icol);
	m_style->SetPadColor(icol);
	m_style->SetStatColor(icol);
	m_style->SetPaperSize(20,26);
	m_style->SetPadTopMargin(0.05);
	m_style->SetPadRightMargin(0.05);
	m_style->SetPadBottomMargin(0.16);
	m_style->SetPadLeftMargin(0.16);
	m_style->SetTitleXOffset(1.4);
	m_style->SetTitleYOffset(1.4);
	Int_t font=42;
	m_style->SetTextFont(font);
	m_style->SetTextSize(m_textsize);
	m_style->SetLabelFont(font,"x");
	m_style->SetTitleFont(font,"x");
	m_style->SetLabelFont(font,"y");
	m_style->SetTitleFont(font,"y");
	m_style->SetLabelFont(font,"z");
	m_style->SetTitleFont(font,"z");
	m_style->SetLabelSize(m_labelsize,"x");
	m_style->SetTitleSize(m_titlesize,"x");
	m_style->SetLabelSize(m_labelsize,"y");
	m_style->SetTitleSize(m_titlesize,"y");
	m_style->SetLabelSize(m_labelsize,"z");
	m_style->SetTitleSize(m_titlesize,"z");
	m_style->SetMarkerStyle(20);
	m_style->SetMarkerSize(1.2);
	m_style->SetHistLineWidth(2);
	m_style->SetLineStyleString(2,"[12 12]");
	m_style->SetEndErrorSize(0.);
	m_style->SetOptTitle(0);
	m_style->SetOptFit(0);
	m_style->SetPadTickX(1);
	m_style->SetPadTickY(1);

//	TStyle* backup = gStyle;
	gStyle = m_style;

	/////////////// TWO PADS //////////////////////

	if((m_canvasx<=0)||(m_canvasy<=0)) m_canvas   = new TCanvas();
	else m_canvas = new TCanvas("c", "c",0,0,m_canvasx,m_canvasy);
	//m_canvas   = new TCanvas("c", "c",0,0,500,500);

	double upperPady = ((m_ratiomax-2.5)/0.5)*(.27/3.+0.01)+0.15;
	double lowerPady = ((m_ratiomax-2.5)/0.5)*(.27/3.)+0.27;

	if(ratio) m_upperPad = new TPad("upperPad", "upperPad", .001, upperPady, .995, .995);
	if(!ratio) m_upperPad = new TPad("upperPad", "upperPad", .001, .001, .995, .995);
	if(ratio) m_lowerPad = new TPad("lowerPad", "lowerPad", .001, .001, .995, lowerPady);

	//rescale
	float uppertitlesize = m_titlesize;
	float lowertitlesize = m_titlesize;
	float upperlabelsize = m_labelsize;
	float lowerlabelsize = m_labelsize;
	float uppertitleoffsety = m_titleoffsety;
	float lowertitleoffsety = m_titleoffsety;
	float upperlabeloffsety = m_labeloffsety;
	float lowerlabeloffsety = m_labeloffsety;
	float titleoffsetx = m_titleoffsetx;
	float labeloffsetx = m_labeloffsetx;
	if(ratio) {
		uppertitlesize = m_titlesize*(.995-0.001)/(.995-upperPady);
		lowertitlesize = m_titlesize*(.995-0.001)/(lowerPady-0.001);
		upperlabelsize = m_labelsize*(.995-0.001)/(.995-upperPady);
		lowerlabelsize = m_labelsize*(.995-0.001)/(lowerPady-0.001);
		uppertitleoffsety = m_titleoffsety*(.995-upperPady)/(.995-0.001);
		lowertitleoffsety = m_titleoffsety*(lowerPady-0.001)/(.995-0.001);
		upperlabeloffsety = m_labeloffsety*(.995-upperPady)/(.995-0.001);
		lowerlabeloffsety = m_labeloffsety*(.995-0.001)/(lowerPady-0.001)*2;

		titleoffsetx = m_titleoffsetx;
		labeloffsetx = m_labeloffsetx*(.995-0.001)/(lowerPady-0.001);
	}


	m_upperPad->Draw();
	if(ratio) m_lowerPad->Draw();

	m_upperPad->cd();
	m_upperPad->Range(-189.6512,-4.901712,1045.574,3.671233);
	m_upperPad->SetFillColor(0);
	m_upperPad->SetBorderMode(0);
	m_upperPad->SetBorderSize(2);
	if(m_logx) m_upperPad->SetLogx();
	if(m_logy) m_upperPad->SetLogy();
	m_upperPad->SetTickx(1);
	m_upperPad->SetTicky(1);
	m_upperPad->SetLeftMargin(0.14);
	m_upperPad->SetRightMargin(0.05);
	m_upperPad->SetTopMargin(0.01);
	m_upperPad->SetBottomMargin(0.1534613);
	m_upperPad->SetFrameBorderMode(0);
	m_upperPad->SetFrameBorderMode(0);

	//Convert data histograms to TGraph (and do Poisson errors if desired)
	if(m_data_histo.size()) {
		vector<TH1*>::iterator hist = m_data_histo.begin();
		for(; hist!=m_data_histo.end(); hist++) {
			bool firstnonzero = true;
			double scale = -1.;

			TArrayD* sumw2 = (*hist)->GetSumw2();

			int nbins = (*hist)->GetNbinsX();
  			float x[nbins];
 			float y[nbins];
 			float yrel[nbins];
 			float errorXl[nbins];
 			float errorXh[nbins];
 			float errorYl[nbins];
  			float errorYh[nbins];
 			float errorYlrel[nbins];
  			float errorYhrel[nbins];

			int j = 0; //number of bins in graph, only keep nonzero points!
			for(int i = 0; i<nbins; i++) {
				x[j] = (*hist)->GetBinCenter(i+1);
				y[j] = (*hist)->GetBinContent(i+1);

				if(y[j]==0.) continue; //skip, zero events

				errorXl[j] = (*hist)->GetBinWidth(i+1)/2.;
 				errorXh[j] = (*hist)->GetBinWidth(i+1)/2.;
 				errorYl[j] = (*hist)->GetBinError(i+1);
				errorYh[j] = (*hist)->GetBinError(i+1);
				if(ratio&&m_mc_sum) {
					yrel[j] = (*hist)->GetBinContent(i+1)/m_mc_sum->GetBinContent(i+1);
 					errorYlrel[j] = errorYl[j]/m_mc_sum->GetBinContent(i+1);
					errorYhrel[j] = errorYh[j]/m_mc_sum->GetBinContent(i+1);
				}

				//Poisson errors
				if(m_doPoisson) {
					if((y[j]!=0.)&&(firstnonzero)) {
						firstnonzero = false;
						scale = sumw2->GetAt(i+1)/y[j];
					}
					else if((!firstnonzero)&&(TMath::Abs(sumw2->GetAt(i+1)/y[j]-scale)>0.01)) {
						cout << "Poisson errors requested for a histogram which is not unscaled or scaled by a constant, the errors will be wrong. Turn off Poisson errors." << endl;
						exit(1);
					}

					if((y[j]/scale)>1000.) {
						errorYh[j] = scale*(TMath::Sqrt(y[j]/scale+0.25) + 0.5);
						errorYl[j] = errorYh[j];
					}
					else {
						errorYh[j] = scale*(TMath::ChisquareQuantile(1. - (1. - 0.68269)/2. , 2.* (y[j]/scale + 1.)) / 2. - y[j]/scale); //Remove "- 1.", 3-21-12 B.B.
						errorYl[j] = scale*(y[j]/scale - TMath::ChisquareQuantile((1. - 0.68629)/2., 2.*y[j]/scale) / 2.);
					}

					if(ratio&&m_mc_sum) {
 						errorYlrel[j] = errorYl[j]/m_mc_sum->GetBinContent(i+1);
						errorYhrel[j] = errorYh[j]/m_mc_sum->GetBinContent(i+1);
					}
				}
				j++;
			}

			TGraphAsymmErrors* graph = new TGraphAsymmErrors(j,x,y,errorXl,errorXh,errorYl,errorYh);

			graph->SetLineColor((*hist)->GetLineColor());
			graph->SetLineWidth(m_linewidth); //graph->SetLineWidth((*hist)->GetLineWidth());
			graph->SetMarkerColor((*hist)->GetMarkerColor());
			graph->SetMarkerSize((*hist)->GetMarkerSize());

			m_data_graph.push_back(graph);

			if(ratio&&m_mc_sum) {
				TGraphAsymmErrors* ratio_graph = new TGraphAsymmErrors(j,x,yrel,errorXl,errorXh,errorYlrel,errorYhrel);

				ratio_graph->SetLineColor((*hist)->GetLineColor());
				ratio_graph->SetLineWidth(m_linewidth); //(*hist)->GetLineWidth());
				ratio_graph->SetMarkerColor((*hist)->GetMarkerColor());
				ratio_graph->SetMarkerSize((*hist)->GetMarkerSize());

				m_data_ratio_graph.push_back(ratio_graph);
			}
		}
	}

 	//MC total error bars
  	int nbins = 1;
  	if(m_mc_histo.size()) nbins = m_mc_sum->GetNbinsX();
  	float x[nbins];
 	float y[nbins];
  	float one[nbins];
  	float zero[nbins];
 	float errorXl[nbins];
 	float errorXh[nbins];
 	float errorYl[nbins];
  	float errorYh[nbins];
 	float errorYlrel[nbins];
  	float errorYhrel[nbins];
 	if(m_mc_histo.size()) {
 		for(int i = 0; i<nbins; i++) {
 			x[i] = m_mc_sum->GetBinCenter(i+1);
 			y[i] = m_mc_sum->GetBinContent(i+1);;
 			one[i] = 1.;
 			zero[i] = 0.;
 			errorXl[i] = m_mc_sum->GetBinWidth(i+1)/2.;
 			errorXh[i] = errorXl[i];
  			errorYl[i] = m_mc_sum->GetBinError(i+1);
 			errorYh[i] = errorYl[i];
 			if(m_mc_sum->GetBinContent(i+1)) {
  				errorYlrel[i] = m_mc_sum->GetBinError(i+1)/m_mc_sum->GetBinContent(i+1);
 				errorYhrel[i] = errorYlrel[i];
  			}
  			else {
  				errorYlrel[i] = 0.;
 				errorYhrel[i] = 0.;
  			}
 		}
 	}

 	//Determine bin size append
 	TString binsize = "";
	if(m_units!="") {
		float binwidth;
		if(m_mc_histo.size()) binwidth = m_mc_histo.at(0)->GetXaxis()->GetBinUpEdge(1)-m_mc_histo.at(0)->GetXaxis()->GetBinLowEdge(1);
		else binwidth = m_data_histo.at(0)->GetXaxis()->GetBinUpEdge(1)-m_data_histo.at(0)->GetXaxis()->GetBinLowEdge(1);

		if(binwidth==(int)binwidth) binsize = TString::Format(" / %i %s",(int)binwidth,m_units.Data());
		else binsize = TString::Format(" / %0.1f %s",binwidth,m_units.Data());
	}

	//MC
	if(m_mc_histo.size()) {
		for(unsigned int i = 0; i<m_mc_histo.size(); i++) {
			if(i==0) {
				if(m_xtitle=="") m_xtitle = m_mc_histo.at(i)->GetXaxis()->GetTitle();
				if(m_ytitle=="") m_ytitle = m_mc_histo.at(i)->GetYaxis()->GetTitle();
				if( (m_units!="") && (!m_ytitle.Contains(binsize)) ) m_ytitle.Append(binsize);
			}

			if(stack) {
				m_mc_histo.at(i)->SetFillColor(m_mc_color.at(i));
				m_mc_histo.at(i)->SetFillStyle(m_mc_style.at(i));
				m_mc_histo.at(i)->SetLineWidth(m_linewidth);

				//Add to previous histograms
				vector<TH1*>::iterator th1 = m_mc_stackedhisto.begin();
				for(; th1!=m_mc_stackedhisto.end(); th1++) (*th1)->Add(m_mc_histo.at(i));

				if(m_mc_style.at(i)!=1001) { //Make a white background histogram so underlying histogram fill colors don't bleed through
					TH1* extrahist = (TH1*)m_mc_histo.at(i)->Clone();
					extrahist->SetName(TString::Format("stacked_%i",(int)m_mc_stackedhisto.size()));
					extrahist->SetFillColor(10); //non-transparent white
					extrahist->SetFillStyle(1001);
					m_mc_stackedhisto.push_back(extrahist);
				}

				TH1* extrahist = (TH1*)m_mc_histo.at(i)->Clone();
				extrahist->SetName(TString::Format("stacked_%i",(int)m_mc_stackedhisto.size()));
				m_mc_stackedhisto.push_back(extrahist); //push back clone
			}
			else {
				m_mc_histo.at(i)->SetLineColor(m_mc_color.at(i));
				m_mc_histo.at(i)->SetLineWidth(m_linewidth);
				if(m_mc_style.at(i)>1000) m_mc_histo.at(i)->SetLineStyle(m_mc_style.at(i)-1000);
				else m_mc_histo.at(i)->SetLineStyle(m_mc_style.at(i));
				if(i==0) {
					if(m_xmin!=m_xmax) m_mc_histo.at(i)->GetXaxis()->SetRangeUser(m_xmin,m_xmax);
					if(m_ymax!=m_ymin) {
						m_mc_histo.at(i)->SetMaximum(m_ymax);
						m_mc_histo.at(i)->SetMinimum(m_ymin);
					}
					m_mc_histo.at(i)->GetYaxis()->SetTitle(m_ytitle);
					m_mc_histo.at(i)->GetYaxis()->SetLabelSize(upperlabelsize);
					m_mc_histo.at(i)->GetYaxis()->SetLabelOffset(upperlabeloffsety);
					m_mc_histo.at(i)->GetYaxis()->SetTitleSize(uppertitlesize);
					m_mc_histo.at(i)->GetYaxis()->SetTitleOffset(uppertitleoffsety);
					m_mc_histo.at(i)->GetXaxis()->SetNdivisions(m_ndiv);
					if(m_forcendiv) m_mc_histo.at(i)->GetXaxis()->SetNdivisions(m_ndiv,false);
					if(!ratio) {
						m_mc_histo.at(i)->GetXaxis()->SetLabelSize(upperlabelsize);
						m_mc_histo.at(i)->GetXaxis()->SetTitleSize(uppertitlesize);

						m_mc_histo.at(i)->GetXaxis()->SetLabelOffset(labeloffsetx);
						m_mc_histo.at(i)->GetXaxis()->SetTitleOffset(titleoffsetx);

						m_mc_histo.at(i)->GetXaxis()->SetTitle(m_xtitle);
						m_mc_histo.at(i)->GetXaxis()->CenterTitle(m_xtitlecenter);
					}
					else {
						m_mc_histo.at(i)->GetXaxis()->SetTitle("");
						for(int j = 1; j<m_mc_histo.at(i)->GetXaxis()->GetNbins()+1; j++) m_mc_histo.at(i)->GetXaxis()->SetBinLabel(j,"");
					}
 					if(!m_doOnlySMTotal) m_mc_histo.at(i)->Draw("hist");
 				}
				else if (!m_doOnlySMTotal) m_mc_histo.at(i)->Draw("hist same");
			}
		}
		m_mc_histo.at(0)->Draw("sameaxis");
	}


	if(m_mc_histo.size()&&stack) {
		if(m_ymax!=m_ymin) {
			(*m_mc_stackedhisto.begin())->SetMaximum(m_ymax);
			(*m_mc_stackedhisto.begin())->SetMinimum(m_ymin);
		}

		if(m_xmin!=m_xmax) (*m_mc_stackedhisto.begin())->GetXaxis()->SetRangeUser(m_xmin,m_xmax);
		(*m_mc_stackedhisto.begin())->GetYaxis()->SetTitle(m_ytitle);
		(*m_mc_stackedhisto.begin())->GetYaxis()->SetLabelSize(upperlabelsize);
		(*m_mc_stackedhisto.begin())->GetYaxis()->SetLabelOffset(upperlabeloffsety);
		(*m_mc_stackedhisto.begin())->GetYaxis()->SetTitleSize(uppertitlesize);
		(*m_mc_stackedhisto.begin())->GetYaxis()->SetTitleOffset(uppertitleoffsety);
		(*m_mc_stackedhisto.begin())->GetXaxis()->SetNdivisions(m_ndiv);
		if(m_forcendiv) (*m_mc_stackedhisto.begin())->GetXaxis()->SetNdivisions(m_ndiv,false);
		if(!ratio) {
			(*m_mc_stackedhisto.begin())->GetXaxis()->SetLabelSize(upperlabelsize);
			(*m_mc_stackedhisto.begin())->GetXaxis()->SetTitleSize(uppertitlesize);
			(*m_mc_stackedhisto.begin())->GetXaxis()->SetTitle(m_xtitle);
			(*m_mc_stackedhisto.begin())->GetXaxis()->CenterTitle(m_xtitlecenter);
			(*m_mc_stackedhisto.begin())->GetXaxis()->SetLabelOffset(labeloffsetx);
			(*m_mc_stackedhisto.begin())->GetXaxis()->SetTitleOffset(titleoffsetx);
		}
		else {

			for(int i = 1; i<(*m_mc_stackedhisto.begin())->GetXaxis()->GetNbins()+1; i++) (*m_mc_stackedhisto.begin())->GetXaxis()->SetBinLabel(i,"");
		}

		vector<TH1*>::iterator th1 = m_mc_stackedhisto.begin();
		(*th1)->Draw("hist");
		th1++;
		for(; th1!=m_mc_stackedhisto.end(); th1++) (*th1)->Draw("hist same");
		(*m_mc_stackedhisto.begin())->Draw("sameaxis");
		m_upperPad->Update();
	}

	//SM total
	if(m_mc_histo.size()) {
		m_temp=(TH1*)m_mc_sum->Clone();
		m_temp->SetName("temp");

		if(m_doSMTotal) {
			m_temp->SetLineColor(2);
			m_temp->SetLineStyle(1);
			m_temp->SetLineWidth(m_linewidth);
			m_temp->SetFillColor(m_errorcolor);
			m_temp->SetFillStyle(m_errorstyle);
			m_temp->GetXaxis()->SetNdivisions(m_ndiv);
			if(m_forcendiv) m_temp->GetXaxis()->SetNdivisions(m_ndiv,false);

			m_mc_sum->SetLineColor(2);
			m_mc_sum->SetLineStyle(1);
			m_mc_sum->SetFillColor(0);
			m_mc_sum->SetFillStyle(0);
			m_mc_sum->SetLineWidth(m_linewidth);
			m_mc_sum->GetXaxis()->SetNdivisions(m_ndiv);
			if(m_forcendiv) m_mc_sum->GetXaxis()->SetNdivisions(m_ndiv,false);

			if(m_doOnlySMTotal) {
				if(m_xmin!=m_xmax) m_mc_sum->GetXaxis()->SetRangeUser(m_xmin,m_xmax);
				if(m_ymax!=m_ymin) {
					m_mc_sum->SetMaximum(m_ymax);
					m_mc_sum->SetMinimum(m_ymin);
				}
				m_mc_sum->GetYaxis()->SetTitle(m_ytitle);
				m_mc_sum->GetYaxis()->SetLabelSize(upperlabelsize);
				m_mc_sum->GetYaxis()->SetLabelOffset(upperlabeloffsety);
				m_mc_sum->GetYaxis()->SetTitleSize(uppertitlesize);
				m_mc_sum->GetYaxis()->SetTitleOffset(uppertitleoffsety);
				if(!ratio) {
					m_mc_sum->GetXaxis()->SetLabelSize(upperlabelsize);
					m_mc_sum->GetXaxis()->SetTitleSize(uppertitlesize);
					m_mc_sum->GetXaxis()->SetTitle(m_xtitle);
					m_mc_sum->GetXaxis()->CenterTitle(m_xtitlecenter);
					m_mc_sum->GetXaxis()->SetLabelOffset(labeloffsetx);
					m_mc_sum->GetXaxis()->SetTitleOffset(titleoffsetx);
				}
				else {
					m_mc_sum->GetXaxis()->SetTitle("");
					for(int j = 1; j<m_mc_sum->GetXaxis()->GetNbins()+1; j++) m_mc_sum->GetXaxis()->SetBinLabel(j,"");
				}
				m_mc_sum->Draw("hist");
			}

			// setup yellow error band:
			m_errors = new TGraphAsymmErrors(nbins,x,y,errorXl,errorXh,errorYl,errorYh);
			m_errors->SetFillColor(m_errorcolor);
			m_errors->SetFillStyle(m_errorstyle);
			m_errors->Draw("2 same");

			m_mc_sum->Draw("hist same"); //Double draw in case of m_doOnlySMTotal, but necessary to get histogram on top of error band.
			m_mc_sum->Draw("sameaxis");
		}
	}

	//Signal overlay
	if(m_signal_histo.size()) {
		for(unsigned int i = 0; i<m_signal_histo.size(); i++) {
			m_signal_histo.at(i)->SetLineStyle(2+i);
			m_signal_histo.at(i)->SetLineWidth(m_linewidth);

			TH1* extrahist = (TH1*)m_signal_histo.at(i)->Clone();
			extrahist->SetName(TString::Format("stacked_%i",(int)m_mc_stackedhisto.size()));
			m_mc_stackedhisto.push_back(extrahist); //push back clone

			extrahist->Add(m_mc_sum);
			extrahist->Draw("hist same");
		}
	}

	//Legend
	int leg_entries = m_mc_histo.size()+m_data_histo.size()+m_signal_histo.size();
	if(m_doSMTotal&&m_mc_histo.size()) leg_entries++;
	if(m_doOnlySMTotal) leg_entries = leg_entries - m_mc_histo.size();
	if(leg_entries>7) leg_entries=7;
	float leg_ylower = m_legy-0.04-((float)leg_entries)*0.06;
	m_legend = new TLegend(m_legx,leg_ylower,0.93,m_legy);
	m_legend->SetFillStyle(0);
	m_legend->SetBorderSize(0);
	TLegendEntry* entry = 0;

	//Data
	if(m_data_histo.size()) {
		for(unsigned int i = 0; i<m_data_histo.size(); i++) {
			if((i==0)&&(!m_mc_histo.size())) {
				if(m_xtitle=="") m_xtitle = m_data_histo.at(i)->GetXaxis()->GetTitle();
				if(m_ytitle=="") m_ytitle = m_data_histo.at(i)->GetYaxis()->GetTitle();
				if( (m_units!="") && (!m_ytitle.Contains(binsize)) ) m_ytitle.Append(binsize);
			}

			if(!m_mc_histo.size()&&(i==0)) {
				if(m_xmin!=m_xmax) m_data_histo.at(i)->GetXaxis()->SetRangeUser(m_xmin,m_xmax);
				if(m_ymax!=m_ymin) {
					m_data_histo.at(i)->SetMaximum(m_ymax);
					m_data_histo.at(i)->SetMinimum(m_ymin);
				}
				m_data_histo.at(i)->GetYaxis()->SetTitle(m_ytitle);
				m_data_histo.at(i)->GetYaxis()->SetLabelSize(upperlabelsize);
				m_data_histo.at(i)->GetYaxis()->SetLabelOffset(upperlabeloffsety);
				m_data_histo.at(i)->GetYaxis()->SetTitleSize(uppertitlesize);
				m_data_histo.at(i)->GetYaxis()->SetTitleOffset(uppertitleoffsety);
				m_data_histo.at(i)->GetXaxis()->SetNdivisions(m_ndiv);
				if(m_forcendiv) m_data_histo.at(i)->GetXaxis()->SetNdivisions(m_ndiv,false);
				if(!ratio) {
					m_data_histo.at(i)->GetXaxis()->SetLabelSize(upperlabelsize);
					m_data_histo.at(i)->GetXaxis()->SetTitleSize(uppertitlesize);
					m_data_histo.at(i)->GetXaxis()->SetTitle(m_xtitle);
					m_data_histo.at(i)->GetXaxis()->CenterTitle(m_xtitlecenter);
					m_data_histo.at(i)->GetXaxis()->SetLabelOffset(labeloffsetx);
					m_data_histo.at(i)->GetXaxis()->SetTitleOffset(titleoffsetx);
				}
				else {
					m_data_histo.at(i)->GetXaxis()->SetTitle("");
					for(int j = 1; j<m_data_histo.at(i)->GetXaxis()->GetNbins()+1; j++) m_data_histo.at(i)->GetXaxis()->SetBinLabel(j,"");
				}
				m_temp=(TH1*)m_data_histo.at(i)->Clone();
				m_temp->SetName("temp");
				m_temp->Reset();
				m_temp->Draw();
			}
			m_data_graph.at(i)->Draw("ep same");

			//legend
			TString count = "";
			if(m_showEventCount) {
				double integral = m_data_histo.at(i)->Integral(0,-1);
				count = TString::Format(", %i",(int)integral);
			}
			entry = m_legend->AddEntry(m_data_graph.at(i),TString(m_data_legend.at(i)+count),m_datalegstyle);
			entry->SetTextAlign(12);
			entry->SetTextColor(1);
		}
		m_temp->Draw("sameaxis");
	}

	//SM total legend
	if(m_mc_histo.size()&&m_doSMTotal) {
		TString count = "";
		if(m_showEventCount) {
			double error;
			double integral = m_temp->IntegralAndError(0,-1,error);
			if(error<0.1) count = TString::Format(", %.1f #pm %.2f",integral,error);
			else count = TString::Format(", %.1f #pm %.1f",integral,error);
		}
		entry = m_legend->AddEntry(m_temp,TString("SM Total"+count),"lf");
		entry->SetTextAlign(12);
		entry->SetTextColor(1);
	}

	//MC components legend
	if(m_mc_histo.size()&&(!m_doOnlySMTotal)) {
		for(unsigned int i = 0; i<m_mc_histo.size(); i++) {
			THSys* thsys = CastTHSys(m_mc_histo.at(i));
			if(thsys) thsys->Recompute(sys);
			TString count = "";
			if(m_showEventCount) {
				double error;
				double integral = m_mc_histo.at(i)->IntegralAndError(0,-1,error);
				if(error<0.1) count = TString::Format(", %.1f #pm %.2f",integral,error);
				else count = TString::Format(", %.1f #pm %.1f",integral,error);
			}
			if(stack) entry = m_legend->AddEntry(m_mc_histo.at(i),TString(m_mc_legend.at(i)+count),"f");
			else entry = m_legend->AddEntry(m_mc_histo.at(i),TString(m_mc_legend.at(i)+count),"l");
			entry->SetTextAlign(12);
			entry->SetTextColor(1);
		}
	}

	//Signals legend
	if(m_signal_histo.size()) {
		for(unsigned int i = 0; i<m_signal_histo.size(); i++) {
			TString count = "";
			if(m_showEventCount) {
				if(signalerror.at(i)<0.1) count = TString::Format(", %.1f #pm %.2f",signalcount.at(i),signalerror.at(i));
				else count = TString::Format(", %.1f #pm %.1f",signalcount.at(i),signalerror.at(i));
			}
			entry = m_legend->AddEntry(m_signal_histo.at(i),m_signal_legend.at(i)+count,"l");
			entry->SetTextAlign(12);
			entry->SetTextColor(1);
		}
	}

	//Draw legend
	m_legend->Draw();
	m_canvas->Update();

	// add ATLAS label, lumi info and channel

	double ypos = 0.92;

	char* atlabel = const_cast<char*>(m_atlaslabel.Data());
	if(m_atlas) ATLASLabel(m_xoffset,ypos,atlabel,1);

	if(m_showlumi) {
		TLatex n1;
		n1.SetNDC();
		n1.SetTextFont(42);
		//if(m_lumi.Length()>14) n1.SetTextSize(0.04);
		//if((m_lumi.Length()>14)&&(!ratio)) n1.SetTextSize(0.035);
		//n1.SetTextSize(0.03);
		n1.SetTextColor(kBlack);
		n1.DrawLatex(m_xoffset,ypos-2.*m_style->GetTextSize(),TString::Format("#intL dt ~ %s",m_lumi.Data()).Data());
	}

	TLatex n2;
	n2.SetNDC();
	n2.SetTextFont(42);
	n2.SetTextColor(kBlack);
	n2.DrawLatex(m_xoffset,ypos-4.*m_style->GetTextSize(),m_label.Data());


	if(ratio) {
		m_lowerPad->cd();
		m_lowerPad->SetGridy();
		m_lowerPad->Range(-187.9722,-0.7128,1040.361,2.7432);
		m_lowerPad->SetFillColor(0);
		m_lowerPad->SetBorderMode(0);
		m_lowerPad->SetBorderSize(2);
		if(m_logx) m_lowerPad->SetLogx();
		m_lowerPad->SetTickx(1);
		m_lowerPad->SetTicky(1);
		m_lowerPad->SetLeftMargin(0.14);
		m_lowerPad->SetRightMargin(0.05);
		m_lowerPad->SetTopMargin(0.05);
		m_lowerPad->SetBottomMargin(0.33);
		m_lowerPad->SetFrameBorderMode(0);
		m_lowerPad->SetFrameBorderMode(0);

		// setup histograms to be divided ///
		if(m_data_histo.size()&&m_mc_histo.size()) {
			m_ratio = (TH1*)m_mc_sum->Clone();
			m_ratio->SetName("ratio");
			m_ratio->Reset();

 			if(m_xmin!=m_xmax) m_ratio->GetXaxis()->SetRangeUser(m_xmin,m_xmax);
 			m_ratio->GetXaxis()->SetTitle(m_xtitle);
 			m_ratio->GetXaxis()->SetNdivisions(m_ndiv);
 			if(m_forcendiv) m_ratio->GetXaxis()->SetNdivisions(m_ndiv,false);
			m_ratio->GetXaxis()->CenterTitle(m_xtitlecenter);

			m_ratio->GetYaxis()->SetRangeUser(0,m_ratiomax);
			m_ratio->GetYaxis()->SetLabelSize(lowerlabelsize);
			m_ratio->GetYaxis()->SetLabelOffset(lowerlabeloffsety);
			m_ratio->GetYaxis()->SetTitleSize(lowertitlesize);
			m_ratio->GetXaxis()->SetLabelSize(lowerlabelsize);
			m_ratio->GetXaxis()->SetTitleSize(lowertitlesize);
			m_ratio->GetXaxis()->SetLabelOffset(labeloffsetx);
			m_ratio->GetXaxis()->SetTitleOffset(titleoffsetx);
			m_ratio->GetYaxis()->SetTitle(m_ratiolabel);
			m_ratio->GetYaxis()->SetTitleOffset(lowertitleoffsety);
			m_ratio->GetYaxis()->SetNdivisions(m_ndivratio);

			m_ratio->SetLineColor(kBlack);
			m_ratio->SetLineWidth(0);
			m_ratio->Draw();

			// setup yellow error band:
			m_ratioerrors = new TGraphAsymmErrors(nbins,x,one,errorXl,errorXh,errorYlrel,errorYhrel);
			m_ratioerrors->SetFillColor(m_errorcolor);
			if(m_ratioerrorstyle<0) m_ratioerrors->SetFillStyle(m_errorstyle);
			else m_ratioerrors->SetFillStyle(m_ratioerrorstyle);
			m_ratioerrors->Draw("2 same");

			//if desired, draw some ridiculous dashed black lines around the error bands
			if(m_ratioerroredges) {
				m_ratioerrorlow = (TH1*)m_mc_sum->Clone();
				m_ratioerrorhigh = (TH1*)m_mc_sum->Clone();

				m_ratioerrorlow->Reset();
				m_ratioerrorhigh->Reset();

				double x,y;
				for(int i = 0; i<m_ratioerrors->GetN(); i++) {
					m_ratioerrors->GetPoint(i,x,y);
					m_ratioerrorlow->SetBinContent(m_ratioerrorlow->FindBin(x),1.-m_ratioerrors->GetErrorYlow(i));
					m_ratioerrorhigh->SetBinContent(m_ratioerrorhigh->FindBin(x),1.+m_ratioerrors->GetErrorYhigh(i));
				}

				m_ratioerrorlow->SetLineColor(kBlack);
				m_ratioerrorhigh->SetLineColor(kBlack);

				m_ratioerrorlow->SetLineStyle(2);
				m_ratioerrorhigh->SetLineStyle(2);

				m_ratioerrorhigh->Draw("hist same");
				m_ratioerrorlow->Draw("hist same");
			}

 			float xlow = m_ratio->GetXaxis()->GetBinLowEdge(1);
 			float xhigh = m_ratio->GetXaxis()->GetBinUpEdge(m_ratio->GetNbinsX());
 			if(m_xmin!=m_xmax) {
  				float xlow_new = m_ratio->GetXaxis()->GetBinLowEdge(m_ratio->FindFixBin(m_xmin));
 				float xhigh_new = m_ratio->GetXaxis()->GetBinUpEdge(m_ratio->FindFixBin(m_xmax-0.00001));
 				if(xhigh>xhigh_new) xhigh = xhigh_new;
 				if(xlow<xlow_new) xlow = xlow_new;
			}

			float halves = 0.5;
			if(m_ndivratio%100<6) {
			 	m_lowerPad->SetGridy(false);

				while(halves<m_ratiomax) {

					if(halves==1.) {
						halves = halves + 0.5;
						continue;
					}

 					TLine* line = new TLine(xlow,halves,xhigh,halves);
					line->SetLineColor(kBlack);
					line->SetLineStyle(7);
					line->SetLineWidth(m_linewidth);
					line->Draw();

					m_errorlines.push_back(line);

					halves = halves + 0.5;
				}
			}

 			m_line = new TLine(xlow,1.,xhigh,1.);
			m_line->SetLineColor(m_smtotalratiocolor);
			m_line->SetLineStyle(1);
			m_line->SetLineWidth(m_linewidth);
			m_line->Draw();

			//draw data points
			vector<TGraphAsymmErrors*>::iterator ratio_graph = m_data_ratio_graph.begin();
			for(; ratio_graph!=m_data_ratio_graph.end(); ratio_graph++) {
				(*ratio_graph)->Draw("ep same");

				//deal with data ratio points above m_ratiomax
				for(int i = 0; i<(*ratio_graph)->GetN(); i++) {
					double a;
					double b;
					(*ratio_graph)->GetPoint(i,a,b);

					if(b<=m_ratiomax) continue;

					double error = (*ratio_graph)->GetErrorYlow(i);
					if((b-error)>m_ratiomax) continue;

					TLine* line = new TLine(a,b-error,a,m_ratiomax);
					line->SetLineColor((*ratio_graph)->GetLineColor());
					line->SetLineWidth((*ratio_graph)->GetLineWidth());

					line->Draw("same");
					m_errorlines.push_back(line);
				}
 			}
		}

		m_lowerPad->Modified();
		m_lowerPad->Update();
	}

	m_canvas->Modified();
	m_canvas->Update();

	//gStyle = backup;

	return m_canvas;
}
