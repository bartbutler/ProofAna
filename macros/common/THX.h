//*****************************************************
// Class   : THX
//
// Purpose : THist eXtended, initially for proper addition of systematics
//
// Author  : B. Butler
//
// Date    : 7-5-11 : Creation of class
//*****************************************************

#ifndef THX_h
#define THX_h

#include <iostream>
#include "TString.h"
#include <map>
#include <vector>

using std::vector;
using std::map;
using std::cout;
using std::endl;

class TF1;
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

class THSys {

	public:
		THSys() { }
		virtual ~THSys() { }

		//Relative errors
		virtual Double_t GetSys(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const = 0;
		virtual Double_t GetSysUp(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const = 0;
		virtual Double_t GetSysDown(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const = 0;
		virtual Double_t GetStat(Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const = 0;

		virtual void ClearStat() = 0;

		virtual void AddSys(const TString& name, float sys) = 0; //for flat systematics, 0.5 would be 50%, symmetrical
		virtual void AddSys(const TString& name, float up, float down) = 0; //at the moment symmetrized (greatest deviation)
		virtual void AddSys(const TString& name, TH1* sys) = 0; //symmetrical
		virtual void AddSys(const TString& name, TH1* up, TH1* down, bool suppress = false) = 0; //at the moment symmetrized (greatest deviation)

		virtual void ClearSys(TString name = "all") = 0; //all, none, or sys1,sys2,sys3 or !sys1,sys2,sys3
		virtual int GetNbins(TH1 const* h1 = 0) const = 0;

		virtual void Recompute(TString sys = "all") = 0;

		virtual TH1 const* GetNominal() const = 0;
		virtual const map<TString, TH1*>& GetSysUpMap() const = 0;
		virtual const map<TString, TH1*>& GetSysDownMap() const = 0;

		ClassDef(THSys,1)
};

TH1* MakeTHX(TH1* h);
THSys* CastTHSys(TH1* h);

template <class T>
class THX : public T, public THSys {

	public:
		THX() { m_nominal = 0; m_sys = "all"; m_sys_up.clear(); m_sys_down.clear();}
      	THX(const T& base) : T(base), THSys() { m_nominal = new T(base); m_nominal->SetDirectory(0); } //primary constructor, creating extended histogram from base class histogram
		THX(const THX& thx) : T(thx), THSys() {
			//custom copy constructor to ensure deep copy of histograms
			m_sys_up.clear();
			m_sys_down.clear();
			m_sys = thx.Sys();

			m_nominal = new T(*((T*)thx.GetNominal()));
			m_nominal->SetDirectory(0);

			map<TString, TH1*>::const_iterator iter = thx.GetSysUpMap().begin();
			for(; iter!=thx.GetSysUpMap().end(); iter++) {
				m_sys_up[iter->first] = new T(*((T*)iter->second));
				m_sys_up[iter->first]->SetDirectory(0);
			}
			iter = thx.GetSysDownMap().begin();
			for(; iter!=thx.GetSysDownMap().end(); iter++) {
				m_sys_down[iter->first] = new T(*((T*)iter->second));
				m_sys_down[iter->first]->SetDirectory(0);
			}
		}
		virtual ~THX();

		TObject* Clone(const char* newname = "") const { THX<T>* hist = new THX<T>(*this); if(strcmp(newname,"")) hist->SetName(newname); return hist; } //faster than TObject::Clone(), same result

		Bool_t Add(TF1 *h1, Double_t c1=1, Option_t *option="") {T::Add(h1,c1,option); return true;}
		Bool_t Add(const TH1 *h1, Double_t c1=1);
		Bool_t Add(const TH1 *h, const TH1 *h2, Double_t c1=1, Double_t c2=1);

		Long64_t Merge(TCollection* list);

		//Integrals with systematic treatment
		Double_t IntegralAndError(Int_t binx1, Int_t binx2, Double_t& err, Option_t* option = "") const;
		Double_t IntegralAndError(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Double_t& err, Option_t* option = "") const;
		Double_t IntegralAndError(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2, Double_t& err, Option_t* option = "") const;

		//Rebinning all sub-histograms
		TH1* Rebin(Int_t ngroup = 2, const char* newname = "", const Double_t* xbins = 0);
		TH2* Rebin2D(Int_t nxgroup = 2, Int_t nygroup = 2, const char* newname = "");
		TH3* Rebin3D(Int_t nxgroup = 2, Int_t nygroup = 2, Int_t nzgroup = 2, const char* newname ="");

		//Relative errors
		Double_t GetSys(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const;
		Double_t GetSysUp(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const;
		Double_t GetSysDown(TString sys = "all", Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const;
		Double_t GetStat(Int_t binx1 = 0, Int_t binx2 = -1, Int_t biny1 = 0, Int_t biny2 = -1, Int_t binz1 = 0, Int_t binz2 = -1) const;

		void ClearStat();

		void AddSys(const TString& name, float sys); //for flat systematics, 0.5 would be 50%, symmetrical
		void AddSys(const TString& name, float up, float down); //at the moment symmetrized (greatest deviation)
		void AddSys(const TString& name, TH1* sys); //symmetrical
		void AddSys(const TString& name, TH1* up, TH1* down, bool suppress = false); //at the moment symmetrized (greatest deviation)

		void ClearSys(TString name = "all"); //all, none, or sys1,sys2,sys3 or !sys1,sys2,sys3

		int GetNbins(TH1 const* h1 = 0) const;

		void Recompute(TString sys = "all"); //all, none, or sys1,sys2,sys3 or !sys1,sys2,sys3

		TH1 const* GetNominal() const {return m_nominal;}
		const map<TString, TH1*>& GetSysUpMap() const {return m_sys_up;}
		const map<TString, TH1*>& GetSysDownMap() const {return m_sys_down;}
		map<TString, TH1*>& GetSysUpMap() {return m_sys_up;}
		map<TString, TH1*>& GetSysDownMap() {return m_sys_down;}

		const TString& Sys() const {return m_sys;}

		ClassDef(THX,1)

	private:

		vector<TString> ParseSys(const TString& sys) const;

		TH1* m_nominal;
		TString m_sys;
		map<TString, TH1*> m_sys_up; //For holding systematics up
		map<TString, TH1*> m_sys_down; //For holding systematics down
};

#endif
