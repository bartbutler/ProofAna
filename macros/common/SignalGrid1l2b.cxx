//*****************************************************
// Class   : SignalGrid1l2b
//
// Purpose : Make pretty 2D plots of 1l2b signal grid
//
// Author  : B. Butler
//
// Date    : 7-16-13 : Creation of first class
//*****************************************************

#include "TCanvas.h"
#include "TLatex.h"
#include "TSystem.h"
#include "TMath.h"
#include "TColor.h"
#include "AtlLabels.h"
#include <iostream>

#include "SignalGrid1l2b.h"

//Constructor
SignalGrid1l2b::SignalGrid1l2b() {

	m_canvas = 0;

	m_histo = new TH2Poly("poly","poly",115.,270.,-10.,125.);
	map<int, set<int> > points;
	points[130].insert(130);
	points[150].insert(130);
	points[150].insert(150);
	points[175].insert(175);
	points[175.0].insert(130.0);
	points[175.0].insert(150.0);
	points[200].insert(130);
	points[200].insert(150);
	points[200].insert(200);
	points[200.0].insert(175.0);
	points[225].insert(175);
	points[225].insert(225);
	points[225.0].insert(130.0);
	points[225.0].insert(150.0);
	points[225.0].insert(200.0);
	points[250].insert(130);
	points[250].insert(150);
	points[250].insert(200);
	points[250].insert(250);
	points[250.0].insert(175.0);
	points[250.0].insert(225.0);
	points[275].insert(175);
	points[275].insert(225);
	points[275.0].insert(130.0);
	points[275.0].insert(150.0);
	points[275.0].insert(200.0);
	points[300].insert(130);
	points[300].insert(150);
	points[300].insert(200);
	points[300.0].insert(175.0);
	points[325].insert(175);
	points[325.0].insert(130.0);
	points[325.0].insert(150.0);
	points[350].insert(130);
	points[350].insert(150);

	map<int, set<int> >::iterator outer = points.begin();
	set<int>::iterator inner;
	for(; outer!=points.end(); ++outer) {
		int x = outer->first;
		inner = outer->second.begin();
		for(; inner!=outer->second.end(); ++inner) {
			int y = (*inner);

			float upxgap = 12.5;
			float upygap = 12.5;
			float downxgap = 12.5;
			float downygap = 12.5;
			if(x==130) {
				upxgap = 10.;
				downxgap = 5.;
			}
			if(y==130) {
				upygap = 10.;
				downygap = 5.;
			}
			if(x==150) {
				downxgap = 10.;
			}
			if(y==150) {
				downygap = 10.;
			}

			int nvtx = 5;
			double ycoords[5] = {Nm(x-downxgap,y-downygap),Nm(x+upxgap,y-downygap),Nm(x+upxgap,y+upygap),Nm(x-downxgap,y+upygap),Nm(x-downxgap,y-downygap)};
			double xcoords[5] = {Cm(x-downxgap,y-downygap),Cm(x+upxgap,y-downygap),Cm(x+upxgap,y+upygap),Cm(x-downxgap,y+upygap),Cm(x-downxgap,y-downygap)};

			if(Nm(x,y)<0.1) {
				nvtx = 4;
				ycoords[3] = Nm(x-downxgap,y-downygap);
				xcoords[3] = Cm(x-downxgap,y-downygap);
			}

			m_histo->AddBin(nvtx,xcoords,ycoords);
		}
	}

	m_style = new TStyle();

	const Int_t NRGBs = 5;
	const Int_t NCont = 255;
	Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
	Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
	Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
	Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
	TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
	m_style->SetNumberContours(NCont);

	m_style->SetPaintTextFormat(".1f");

	Reset();
}

void SignalGrid1l2b::Reset() {

	m_logz = false;
	m_lumi = "";
	m_label = "";
	m_atlaslabel = "";
	m_atlas = true;
	m_showlumi = true;
	m_xtitle = "m_{#chi^{0}_{2}}/m_{#chi^{#pm}_{1}} [GeV]";
	m_ytitle = "m_{#chi^{0}_{1}} [GeV]";
	m_labelsize = 0.05;
	m_textsize = 0.05;
	m_titlesize = 0.05;
	m_titleoffsetx = 0.95;
	m_labeloffsetx = 0.001;
	m_titleoffsety = 1.3;
	m_labeloffsety = 0.001;
	m_xoffset = 0.2;
	m_xtitlecenter = false;
	m_canvasx = 650;
	m_canvasy = 650;

	if(m_canvas) {
		delete m_canvas;
		m_canvas = 0;
	}
	m_histo->Reset("M");
}

void SignalGrid1l2b::FillBin(float x, float y, float w) {
	if(y<0.1) y = 0.1;
	m_histo->Fill(x,y,w);
}

TCanvas* SignalGrid1l2b::Plot() {

//	TStyle* backup = gStyle;
	gStyle = m_style;
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
	m_style->SetPadRightMargin(0.15);

	/////////////// TWO PADS //////////////////////
	if(m_canvas) delete m_canvas;
	if((m_canvasx<=0)||(m_canvasy<=0)) m_canvas   = new TCanvas();
	else m_canvas = new TCanvas("c", "c",0,0,m_canvasx,m_canvasy);

	if(m_logz) m_canvas->SetLogz();

	m_histo->GetYaxis()->SetTitle(m_ytitle);
	m_histo->GetYaxis()->SetLabelSize(m_labelsize);
	m_histo->GetYaxis()->SetLabelOffset(m_labeloffsety);
	m_histo->GetYaxis()->SetTitleSize(m_titlesize);
	m_histo->GetYaxis()->SetTitleOffset(m_titleoffsety);

	m_histo->GetXaxis()->SetLabelSize(m_labelsize);
	m_histo->GetXaxis()->SetTitleSize(m_titlesize);
	m_histo->GetXaxis()->SetLabelOffset(m_labeloffsetx);
	m_histo->GetXaxis()->SetTitleOffset(m_titleoffsetx);
	m_histo->GetXaxis()->SetTitle(m_xtitle);
	m_histo->GetXaxis()->CenterTitle(m_xtitlecenter);

	m_histo->SetMarkerColor(kWhite);
	m_histo->Draw("colz text");

	// add ATLAS label, lumi info and channel

	double ypos = 0.88;

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
		n1.DrawLatex(m_xoffset,ypos-1.9*m_style->GetTextSize(),TString::Format("#intL dt ~ %s",m_lumi.Data()).Data());
	}

	TLatex n2;
	n2.SetNDC();
	n2.SetTextFont(42);
	n2.SetTextColor(kBlack);
	n2.DrawLatex(m_xoffset,ypos-0.02-4.*m_style->GetTextSize(),m_label.Data());

	m_canvas->Modified();
	m_canvas->Update();

	//gStyle = backup;

	return m_canvas;
}
