/**************************************************************************
 **
 **   File:         Particle.h
 **
 **   Description:  Generic analysis particle
 **
 **   Author:       B. Butler
 **   Created:      3-28-11
 **
 **************************************************************************/

#ifndef Particle_h
#define Particle_h

#include "MomentObj.h"
#include "TLorentzVector.h"

class Particle : public MomentObj {
	public:
	
		Particle() { }
      	Particle(const TLorentzVector& v) : p(v) { }
		Particle(const MomentObj& m) : MomentObj(m) { }
      	Particle(const TLorentzVector& v,const MomentObj& m) : MomentObj(m), p(v) { }
		virtual ~Particle() { }
		
		virtual TObject* Clone(const char*) const { //for deep copies
			Particle* obj = new Particle(*this);
			return obj;
		}
		
		//copy from obj to this, but do not clone sub-objects in ObjVecObj, just copy the pointers
		void ShallowClone(const Particle& obj) {
			Reset();
			m_keyMap = obj.m_keyMap;
			
			p = obj.p;
			
			map<MomKey, MapObj*>::iterator iter = m_keyMap.begin();
			for(; iter!=m_keyMap.end(); iter++) {
				if(iter->second->Type()==OBJ) {
					iter->second = new ObjVecObj(*(ObjVecObj*)iter->second);
				}
				else iter->second = iter->second->Clone(); //simple types (moments) copied
			}			
		}
		
		void Show() const {
			MomentObj::Show();
		
			//List Pt/Eta/Phi/M
			TString first = "Pt";
			TString second = "double";
			TString third = TString::Format("%f",p.Pt());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;
			
			first = "Eta";
			second = "double";
			third = TString::Format("%f",p.Eta());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;
			
			first = "Phi";
			second = "double";
			third = TString::Format("%f",p.Phi());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;
			
			first = "M";
			second = "double";
			third = TString::Format("%f",p.M());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;

			cout << "*******************************************************************" << endl;
		
		}
		
		TLorentzVector p;
		
		ClassDef(Particle,1);

	private:
};

#endif  /* Particle_h */
