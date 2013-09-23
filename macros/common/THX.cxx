//*****************************************************
// Class   : THX
//
// Purpose : THist eXtended, initially for proper addition of systematics
//
// Author  : B. Butler
//
// Date    : 7-5-11 : Creation of class
//*****************************************************

#ifndef THX_cxx
#define THX_cxx

#include <set>
#include "THX.h"
#include "TMath.h"
#include "TCollection.h"
#include "TList.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TClass.h"

using std::set;

//template<class T>
//THX<T>::THX(const THX<T>& thx) : T(thx)


template<class T>
THX<T>::~THX<T>()
{
	if(m_nominal) delete m_nominal;
	this->ClearSys("all");
}

template<class T>
void THX<T>::ClearSys(TString sys)
{
	vector<TString> inputs = ParseSys(sys);

	vector<TString>::iterator iter = inputs.begin();
	for(; iter!=inputs.end(); iter++) {
		delete m_sys_up[*iter];
		delete m_sys_down[*iter];
		m_sys_up.erase(*iter);
		m_sys_down.erase(*iter);
	}
}

template<class T>
Bool_t THX<T>::Add(const TH1 *h1, const TH1 *h2, Double_t c1, Double_t c2)
{
	//Special case of h1->Add(h1,h1,c1,c2), h1->Add(h1,h2,c1,c2) or h2->Add(h1,h2,c1,c2)
	const THX<T>* X = dynamic_cast<const THX<T>*>(h1);
	const THX<T>* Y = dynamic_cast<const THX<T>*>(h2);
	if((this==X)&&(this==Y)) {
		if((c1+c2)==1.) return true; //nothing to be done

		THX<T>* clone = (THX<T>*)this->Clone(); //save copy of histogram, probably less than efficient

		//Clear systematics
		this->ClearSys("all");

		//Reset inherited and nominal histograms, if exists in nominal case
		this->Reset("M");
		if(m_nominal) m_nominal->Reset("M");

		this->Add(clone,c1+c2);

		delete clone;
		return true;
	}
	else if(this==X) {
		this->Scale(c1); //recursively calls this function
		if(c2) Add(h2,c2);
		return true;
	}
	else if(this==Y) {
		this->Scale(c2); //recursively calls this function
		if(c1) Add(h1,c1);
		return true;
	}

	//h1 and h2 must both be != this, safe to clear things

	//Clear systematics
	this->ClearSys("all");

	//Reset inherited and nominal histograms, if exists in nominal case
	this->Reset("M");
	if(m_nominal) m_nominal->Reset("M");

	//use THX<T>::Add(h1,c1) sequentially
	if(c1) Add(h1,c1);
	if(c2) Add(h2,c2);

	return true;
}

template<class T>
Bool_t THX<T>::Add(const TH1 *h1, Double_t c1)
{
	const THX<T>* X = dynamic_cast<const THX<T>*>(h1);

	if(X) {
		//Add nominal histogram to inherited histogram class and map
		if(this->fNcells!=GetNbins(h1)) {
			cout << "Histogram dimension/nbin mismatch" << endl;
			return false;
		}
		else T::Add(X->GetNominal(),c1);
		if(!m_nominal) {
			m_nominal = new T(*((T*)X->GetNominal()));
			m_nominal->SetDirectory(0);
			m_nominal->Scale(c1);
		}
		else m_nominal->Add(X->GetNominal(),c1);

		//Add systematics histograms to map
		map<TString, TH1*>::const_iterator iter = X->GetSysUpMap().begin();
		for(; iter!=X->GetSysUpMap().end(); iter++) {
			if(m_sys_up.find(iter->first)==m_sys_up.end()) {
				m_sys_up[iter->first] = new T(*((T*)iter->second));
				m_sys_up[iter->first]->SetDirectory(0);
				m_sys_up[iter->first]->Scale(c1);
			}
			else {
				m_sys_up[iter->first]->Add(iter->second,c1);
			}
		}

		iter = X->GetSysDownMap().begin();
		for(; iter!=X->GetSysDownMap().end(); iter++) {
			if(m_sys_down.find(iter->first)==m_sys_down.end()) {
				m_sys_down[iter->first] = new T(*((T*)iter->second));
				m_sys_down[iter->first]->SetDirectory(0);
				m_sys_down[iter->first]->Scale(c1);
			}
			else {
				m_sys_down[iter->first]->Add(iter->second,c1);
			}
		}
	}
	else {
		const T* Y = dynamic_cast<const T*>(h1);
		if(Y) {
			if(this->fNcells!=GetNbins(h1)) {
				cout << "Histogram dimension/nbin mismatch" << endl;
				return false;
			}
			else T::Add(h1,c1);
			if(!m_nominal) {
				m_nominal = new T(*Y);
				m_nominal->SetDirectory(0);
				m_nominal->Scale(c1);
			}
			else m_nominal->Add(h1,c1);
		}
		else {
			cout << "Type mismatch, fix your code" << endl;
			return false;
		}
	}
	return true;
}

template<class T>
void THX<T>::Recompute(TString sys)
{
	m_sys = sys; //save for future IntegralAndError calls
	vector<TString> input = ParseSys(sys);

	this->Reset("M");

	//Replace existing histogram with nominal histogram stored in the map
	T::Add(m_nominal,m_nominal,1.,0.);

	//Bin by bin addition of errors (quadrature, add procedure should take care of correlated systematics)
	for(int bin = 0; bin<this->fNcells; bin++) {
		double relstaterror = (this->GetBinContent(bin)>0.) ? this->GetBinError(bin)/this->GetBinContent(bin) : 0;
		double error2 = relstaterror*relstaterror;

		vector<TString>::iterator iter = input.begin();
		for(; iter!=input.end(); iter++) {

			//Pick biggest deviation for now (asymmetric errors later?)
			double abserror = TMath::Abs(m_sys_up.find(*iter)->second->GetBinContent(bin));

			TH1* down = m_sys_down.find(*iter)->second;
			//if(down) {
				double downerror = TMath::Abs(down->GetBinContent(bin));
				if(downerror>abserror) abserror=downerror;
			//}

			//Add in systematic
			double relerror = (this->GetBinContent(bin)>0.) ? abserror/this->GetBinContent(bin) : 0.;
			error2+=relerror*relerror;
		}

		this->SetBinError(bin,TMath::Sqrt(error2)*this->GetBinContent(bin));
	}
}

template<class T>
int THX<T>::GetNbins(TH1 const* h1) const
{
	const TH1* hist = h1;
	if(!h1) hist = this;
	int nbins = 1;
	switch(hist->GetDimension()) {
		case 3:
			nbins*=hist->GetNbinsZ()+2;
		case 2:
			nbins*=hist->GetNbinsY()+2;
		case 1:
			nbins*=hist->GetNbinsX()+2;
	}
	return nbins;
}

template<class T>
void THX<T>::ClearStat() //Clear statistical errors
{
	if(m_nominal) m_nominal = new T(*this);

	int nbins = this->GetNbins(m_nominal);
	for(int i = 0; i<nbins; i++) m_nominal->SetBinError(i,0.);
}

template<class T>
void THX<T>::AddSys(const TString& name, float sys) //for flat systematics, 0.5 would be 50%, symmetrical
{
	if(m_sys_up.find(name)!=m_sys_up.end()) {
		cout << "Systematic already exists, fix your code" << endl;
		return;
	}
	if(sys<0) {
		cout << "Systematic cannot be less than zero" << endl;
		return;
	}

	T* sysup = new T(*((T*)m_nominal));
	T* sysdown = new T(*((T*)m_nominal));
	sysup->SetDirectory(0);
	sysdown->SetDirectory(0);
	sysup->Scale(sys);
	if(sys>1.) sys=1.; //Sys down, number of events can't go below zero, irrelevant when symmetrizing anyway
	sysdown->Scale(-sys);

	m_sys_up[name] = sysup;
	m_sys_down[name] = sysdown;
}

template<class T>
void THX<T>::AddSys(const TString& name, float up, float down) //at the moment symmetrized (greatest deviation)
{
	if(m_sys_up.find(name)!=m_sys_up.end()) {
		cout << "Systematic already exists, fix your code" << endl;
		return;
	}
	if((up*down)>0) {
		cout << "Are both up and down scalar systematics supposed to be the same sign?" << endl;
		//return;
	}

	T* sysup = new T(*((T*)m_nominal));
	T* sysdown = new T(*((T*)m_nominal));
	sysup->SetDirectory(0);
	sysdown->SetDirectory(0);
	if(up<-1.) up=-1.; //number of events can't go below zero, irrelevant when symmetrizing anyway
	if(down<-1.) down=-1.;
	sysup->Scale(up);
	sysdown->Scale(down);

	m_sys_up[name] = sysup;
	m_sys_down[name] = sysdown;
}

template<class T>
void THX<T>::AddSys(const TString& name, TH1* sys) //symmetrical
{
	if(m_sys_up.find(name)!=m_sys_up.end()) {
		cout << "Systematic already exists, fix your code" << endl;
		return;
	}

	TH1* sysup = 0;
	if(sys==0) {
		cout << "Adding 100% " << name << " systematic due to null pointer passed" << endl;
		sysup = new T(*((T*)m_nominal));
		sysup->SetDirectory(0);
		sysup->Scale(2.);
	}
	else {
		sysup = new T(*((T*)sys));
		sysup->SetDirectory(0);
	}

	sysup->Add(m_nominal,-1.);

	TH1* sysdown = new T(*((T*)sysup));
	sysdown->SetDirectory(0);
	sysdown->Scale(-1.);

	m_sys_up[name] = sysup;
	m_sys_down[name] = sysdown;
}

template<class T>
void THX<T>::AddSys(const TString& name, TH1* up, TH1* down, bool suppress) //at the moment symmetrized (greatest deviation)
{
	if(m_sys_up.find(name)!=m_sys_up.end()) {
		cout << "Systematic already exists, fix your code" << endl;
		return;
	}
	if((up==0)&&(down==0)) {
		if(!suppress) cout << "Both systematic histograms are null pointers, this is likely a problem" << endl;
		return;
	}

	TH1* sysup = 0;
	TH1* sysdown = 0;
	if(up==0) {
		cout << "Adding 100% " << name << " up systematic due to null pointer passed" << endl;
		sysup = new T(*((T*)m_nominal));
		sysup->SetDirectory(0);
		sysup->Scale(2.);
	}
	else {
		sysup = new T(*((T*)up));
		sysup->SetDirectory(0);
	}

	if(down==0) {
		cout << "Adding 100% " << name << " down systematic due to null pointer passed" << endl;
		sysdown = new T(*((T*)m_nominal));
		sysdown->SetDirectory(0);
		sysdown->Scale(0.);
	}
	else {
		sysdown = new T(*((T*)down));
		sysdown->SetDirectory(0);
	}

	sysup->Add(m_nominal,-1.);
	sysdown->Add(m_nominal,-1.);

	m_sys_up[name] = sysup;
	m_sys_down[name] = sysdown;
}

template<class T>
Double_t THX<T>::IntegralAndError(Int_t binx1, Int_t binx2, Double_t& err, Option_t* option) const
{
	if(this->GetDimension()!=1) {
		cout << "Wrong integration function for this histogram dimension" << endl;
		err = 0.;
		return 0.;
	}

	double absstaterror = 0.;
	double integral = m_nominal->IntegralAndError(binx1,binx2,absstaterror,option);
	if(integral<=0.) {
		cout << "Attempt to integrate empty histogram!" << endl;
		err = 0.;
		return 0.;
	}
	double relstaterror = absstaterror/integral;

	double error2 = relstaterror*relstaterror;

	vector<TString> input = ParseSys(m_sys);
	vector<TString>::iterator iter = input.begin();
	for(; iter!=input.end(); iter++) {
		//Pick biggest deviation for now (asymmetric errors later?)
		double abserror = TMath::Abs(m_sys_up.find(*iter)->second->Integral(binx1,binx2,option));

		TH1* down = m_sys_down.find(*iter)->second;
		//if(down) {
			double downerror = TMath::Abs(down->Integral(binx1,binx2,option));
			if(downerror>abserror) abserror=downerror;
		//}

		//Add in systematic
		double relerror = abserror/integral;
		error2+=relerror*relerror;
	}

	err = TMath::Sqrt(error2)*integral;
	return integral;
}

template<class T>
Double_t THX<T>::IntegralAndError(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Double_t& err, Option_t* option) const
{
	if(this->GetDimension()!=2) {
		cout << "Wrong integration function for this histogram dimension" << endl;
		err = 0.;
		return 0.;
	}

	double absstaterror = 0.;
	double integral = ((TH2*)m_nominal)->IntegralAndError(binx1,binx2,biny1,biny2,absstaterror,option);
	if(integral<=0.) {
		cout << "Attempt to integrate empty histogram!" << endl;
		err = 0.;
		return 0.;
	}
	double relstaterror = absstaterror/integral;

	double error2 = relstaterror*relstaterror;

	vector<TString> input = ParseSys(m_sys);
	vector<TString>::iterator iter = input.begin();
	for(; iter!=input.end(); iter++) {

		//Pick biggest deviation for now (asymmetric errors later?)
		double abserror = TMath::Abs(((TH2*)m_sys_up.find(*iter)->second)->Integral(binx1,binx2,biny1,biny2,option));

		TH1* down = m_sys_down.find(*iter)->second;
		//if(down) {
			double downerror = TMath::Abs(((TH2*)down)->Integral(binx1,binx2,biny1,biny2,option));
			if(downerror>abserror) abserror=downerror;
		//}

		//Add in systematic
		double relerror = abserror/integral;
		error2+=relerror*relerror;
	}

	err = TMath::Sqrt(error2)*integral;
	return integral;
}

template<class T>
Double_t THX<T>::IntegralAndError(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2, Double_t& err, Option_t* option) const
{
	if(this->GetDimension()!=3) {
		cout << "Wrong integration function for this histogram dimension" << endl;
		err = 0.;
		return 0.;
	}

	double absstaterror = 0.;
	double integral = ((TH3*)m_nominal)->IntegralAndError(binx1,binx2,biny1,biny2,binz1,binz2,absstaterror,option);
	if(integral<=0.) {
		cout << "Attempt to integrate empty histogram!" << endl;
		err = 0.;
		return 0.;
	}
	double relstaterror = absstaterror/integral;

	double error2 = relstaterror*relstaterror;

	vector<TString> input = ParseSys(m_sys);
	vector<TString>::iterator iter = input.begin();
	for(; iter!=input.end(); iter++) {

		//Pick biggest deviation for now (asymmetric errors later?)
		double abserror = TMath::Abs(((TH3*)m_sys_up.find(*iter)->second)->Integral(binx1,binx2,biny1,biny2,binz1,binz2,option));

		TH1* down = m_sys_down.find(*iter)->second;
		//if(down) {
			double downerror = TMath::Abs(((TH3*)down)->Integral(binx1,binx2,biny1,biny2,binz1,binz2,option));
			if(downerror>abserror) abserror=downerror;
		//}

		//Add in systematic
		double relerror = abserror/integral;
		error2+=relerror*relerror;
	}

	err = TMath::Sqrt(error2)*integral;
	return integral;
}

template<class T>
Double_t THX<T>::GetSys(TString name, Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2) const
{
	vector<TString> input = ParseSys(name);

	float relerror2 = 0.;
	vector<TString>::iterator iter = input.begin();
	for(; iter!=input.end(); iter++) {
		float sysup = fabs(this->GetSysUp(*iter, binx1, binx2, biny1, biny2, binz1, binz2));
		float sysdown = fabs(this->GetSysDown(*iter, binx1, binx2, biny1, biny2, binz1, binz2));
		if(sysup>sysdown) relerror2 += sysup*sysup;
		else relerror2 += sysdown*sysdown;
	}
	return TMath::Sqrt(relerror2);
}

template<class T>
Double_t THX<T>::GetSysUp(TString name, Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2) const
{
	vector<TString> input = ParseSys(name);

	if(input.size()>1) {
		float relerror2 = 0.;
		vector<TString>::iterator iter = input.begin();
		for(; iter!=input.end(); iter++) {
			float sys = this->GetSysUp(*iter);
			relerror2 += sys*sys;
		}
		return TMath::Sqrt(relerror2);
	}

	if(m_sys_up.find(name)==m_sys_up.end()) {
		//cout << "Systematic " << name << " not found" << endl;
		return 0.;
	}

	double error = 0.;
	double integral = 0.;

	switch(this->GetDimension()) {
		case 1:
			integral = ((TH1*)m_nominal)->Integral(binx1,binx2);

			error = ((TH1*)m_sys_up.find(name)->second)->Integral(binx1,binx2);

			break;
		case 2:
			integral = ((TH2*)m_nominal)->Integral(binx1,binx2,biny1,biny2);

			error = ((TH2*)m_sys_up.find(name)->second)->Integral(binx1,binx2,biny1,biny2);

			break;
		case 3:
			integral = ((TH3*)m_nominal)->Integral(binx1,binx2,biny1,biny2,binz1,binz2);

			error = ((TH3*)m_sys_up.find(name)->second)->Integral(binx1,binx2,biny1,biny2,binz1,binz2);

			break;
		default:
			cout << "Dimension != 1, 2, or 3? What?" << endl;
			exit(1);
	}
	return error/integral;
}

template<class T>
Double_t THX<T>::GetSysDown(TString name, Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2) const
{
	vector<TString> input = ParseSys(name);

	if(input.size()>1) {
		float relerror2 = 0.;
		vector<TString>::iterator iter = input.begin();
		for(; iter!=input.end(); iter++) {
			float sys = this->GetSysDown(*iter);
			relerror2 += sys*sys;
		}
		return -TMath::Sqrt(relerror2);
	}

	if(m_sys_down.find(name)==m_sys_down.end()) {
		//cout << "Systematic " << name << " not found" << endl;
		return 0.;
	}

	double error = 0.;
	double integral = 0.;

	switch(this->GetDimension()) {
		case 1:
			integral = ((TH1*)m_nominal)->Integral(binx1,binx2);

			error = ((TH1*)m_sys_down.find(name)->second)->Integral(binx1,binx2);

			break;
		case 2:
			integral = ((TH2*)m_nominal)->Integral(binx1,binx2,biny1,biny2);

			error = ((TH2*)m_sys_down.find(name)->second)->Integral(binx1,binx2,biny1,biny2);

			break;
		case 3:
			integral = ((TH3*)m_nominal)->Integral(binx1,binx2,biny1,biny2,binz1,binz2);

			error = ((TH3*)m_sys_down.find(name)->second)->Integral(binx1,binx2,biny1,biny2,binz1,binz2);

			break;
		default:
			cout << "Dimension != 1, 2, or 3? What?" << endl;
			exit(1);
	}
	return error/integral;
}

template<class T>
Double_t THX<T>::GetStat(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2) const
{
	double absstaterror = 0.;
	double integral = 0.;
	switch(this->GetDimension()) {
		case 1:
			integral = ((TH1*)m_nominal)->IntegralAndError(binx1,binx2,absstaterror,"");
			break;
		case 2:
			integral = ((TH2*)m_nominal)->IntegralAndError(binx1,binx2,biny1,biny2,absstaterror,"");
			break;
		case 3:
			integral = ((TH3*)m_nominal)->IntegralAndError(binx1,binx2,biny1,biny2,binz1,binz2,absstaterror,"");
			break;
		default:
			cout << "Dimension != 1, 2, or 3? What?" << endl;
			return -1.;
	}
	return absstaterror/integral;
}

template<class T>
TH1* THX<T>::Rebin(Int_t ngroup, const char* newname, const Double_t* xbins)
{
	if(this->GetDimension()!=1) {
		cout << "Wrong rebin function for this histogram dimension" << endl;
		return 0;
	}

	((TH1*)this)->TH1::Rebin(ngroup,newname,xbins);
	if(m_nominal) ((TH1*)m_nominal)->Rebin(ngroup,newname,xbins);
	map<TString, TH1*>::iterator mapiter = m_sys_up.begin();
	for(; mapiter!=m_sys_up.end(); mapiter++) {
		((TH1*)mapiter->second)->Rebin(ngroup,newname,xbins);
	}
	mapiter = m_sys_down.begin();
	for(; mapiter!=m_sys_down.end(); mapiter++) {
		((TH1*)mapiter->second)->Rebin(ngroup,newname,xbins);
	}

	Recompute();
	return (TH1*)this;
}

template<class T>
TH2* THX<T>::Rebin2D(Int_t nxgroup, Int_t nygroup, const char* newname)
{
	if(this->GetDimension()!=2) {
		cout << "Wrong rebin function for this histogram dimension" << endl;
		return 0;
	}

	((TH2*)this)->TH2::Rebin2D(nxgroup,nygroup,newname);
	if(m_nominal) ((TH2*)m_nominal)->Rebin2D(nxgroup,nygroup,newname);
	map<TString, TH1*>::iterator mapiter = m_sys_up.begin();
	for(; mapiter!=m_sys_up.end(); mapiter++) {
		((TH2*)mapiter->second)->Rebin2D(nxgroup,nygroup,newname);
	}
	mapiter = m_sys_down.begin();
	for(; mapiter!=m_sys_down.end(); mapiter++) {
		((TH2*)mapiter->second)->Rebin2D(nxgroup,nygroup,newname);
	}

	Recompute();
	return (TH2*)this;
}

template<class T>
TH3* THX<T>::Rebin3D(Int_t nxgroup, Int_t nygroup, Int_t nzgroup, const char* newname)
{
	if(this->GetDimension()!=3) {
		cout << "Wrong rebin function for this histogram dimension" << endl;
		return 0;
	}

	((TH3*)this)->TH3::Rebin3D(nxgroup,nygroup,nzgroup,newname);
	if(m_nominal) ((TH3*)m_nominal)->Rebin3D(nxgroup,nygroup,nzgroup,newname);
	map<TString, TH1*>::iterator mapiter = m_sys_up.begin();
	for(; mapiter!=m_sys_up.end(); mapiter++) {
		((TH3*)mapiter->second)->Rebin3D(nxgroup,nygroup,nzgroup,newname);
	}
	mapiter = m_sys_down.begin();
	for(; mapiter!=m_sys_down.end(); mapiter++) {
		((TH3*)mapiter->second)->Rebin3D(nxgroup,nygroup,nzgroup,newname);
	}

	Recompute();
	return (TH3*)this;
}

template<class T>
Long64_t THX<T>::Merge(TCollection* li)
{
	if (!li) return 0;
	if (li->IsEmpty()) return (Int_t) this->GetEntries();

	TList inlist;
	inlist.AddAll(li);

	TIter next(&inlist);
	while (TObject *o = next()) {
		THX<T>* h = dynamic_cast<THX<T>*> (o);
		if (!h) {
			Error("Add","Attempt to add object of class: %s to a %s", o->ClassName(),this->ClassName());
			return -1;
		}
		this->Add(h);
	}
	return (Int_t) this->GetEntries();
}

template<class T>
vector<TString> THX<T>::ParseSys(const TString& sys) const
{
	bool invert = sys.Contains("!");
	vector<TString> output;

	TString temp(sys);
	temp.ReplaceAll("!","");

	TObjArray* arr = temp.Tokenize(",");
	arr->SetOwner(kTRUE);
	set<TString> list;
	for(int i = 0; i<arr->GetEntries(); i++) {
		list.insert(((TObjString*)arr->At(i))->GetString());
	}
	delete arr;

	bool all = (list.find("all")!=list.end()) ? true : false;
	bool none = (list.find("none")!=list.end()) ? true : false;

	if(all) {
		map<TString,TH1*>::const_iterator iter = m_sys_up.begin();
		for(; iter!=m_sys_up.end(); iter++) output.push_back(iter->first);
	}
	else if(!none) {
		map<TString,TH1*>::const_iterator iter = m_sys_up.begin();
		for(; iter!=m_sys_up.end(); iter++) {
			bool inlist = (list.find(iter->first)!=list.end());
			if((inlist&&(!invert))||((!inlist)&&(invert))) output.push_back(iter->first);
		}
	}
	return output;
}

//Create specific types and typedef them

//1D
template class THX<TH1I>;
template class THX<TH1F>;
template class THX<TH1D>;
typedef THX<TH1I> TH1IX;
typedef THX<TH1F> TH1FX;
typedef THX<TH1D> TH1DX;
//2D
template class THX<TH2I>;
template class THX<TH2F>;
template class THX<TH2D>;
typedef THX<TH2I> TH2IX;
typedef THX<TH2F> TH2FX;
typedef THX<TH2D> TH2DX;
//3D
template class THX<TH3I>;
template class THX<TH3F>;
template class THX<TH3D>;
typedef THX<TH3I> TH3IX;
typedef THX<TH3F> TH3FX;
typedef THX<TH3D> TH3DX;

TH1* MakeTHX(TH1* h) {

	TClass* th1Class = h->Class();
	TClass* derivedClass = th1Class->GetActualClass(h);
	TString derivedName(derivedClass->GetName());

	if(derivedName.Contains("THX<")) return (TH1*)(h->Clone());

	if(derivedName.CompareTo("TH1I")==0) return new TH1IX(*(TH1I*)h);
	else if(derivedName.CompareTo("TH1F")==0) return new TH1FX(*(TH1F*)h);
	else if(derivedName.CompareTo("TH1D")==0) return new TH1DX(*(TH1D*)h);
	else if(derivedName.CompareTo("TH2I")==0) return new TH2IX(*(TH2I*)h);
	else if(derivedName.CompareTo("TH2F")==0) return new TH2FX(*(TH2F*)h);
	else if(derivedName.CompareTo("TH2D")==0) return new TH2DX(*(TH2D*)h);
	else if(derivedName.CompareTo("TH3I")==0) return new TH3IX(*(TH3I*)h);
	else if(derivedName.CompareTo("TH3F")==0) return new TH3FX(*(TH3F*)h);
	else if(derivedName.CompareTo("TH3D")==0) return new TH3DX(*(TH3D*)h);
	else { cout << "Unsupported base histogram type " << derivedName << endl; return 0; }
}

THSys* CastTHSys(TH1* h) {

	TClass* th1Class = h->Class();
	TClass* derivedClass = th1Class->GetActualClass(h);
	TString derivedName(derivedClass->GetName());

	if(!derivedName.Contains("THX<")) return 0;

	if(derivedName.CompareTo("THX<TH1I>")==0) return (THSys*)((THX<TH1I>*)h);
	else if(derivedName.CompareTo("THX<TH1F>")==0) return (THSys*)((THX<TH1F>*)h);
	else if(derivedName.CompareTo("THX<TH1D>")==0) return (THSys*)((THX<TH1D>*)h);
	else if(derivedName.CompareTo("THX<TH2I>")==0) return (THSys*)((THX<TH2I>*)h);
	else if(derivedName.CompareTo("THX<TH2F>")==0) return (THSys*)((THX<TH2F>*)h);
	else if(derivedName.CompareTo("THX<TH2D>")==0) return (THSys*)((THX<TH2D>*)h);
	else if(derivedName.CompareTo("THX<TH3I>")==0) return (THSys*)((THX<TH3I>*)h);
	else if(derivedName.CompareTo("THX<TH3F>")==0) return (THSys*)((THX<TH3F>*)h);
	else if(derivedName.CompareTo("THX<TH3D>")==0) return (THSys*)((THX<TH3D>*)h);
	else { cout << "Unsupported histogram type " << derivedName << endl; return 0; }
}

#endif
