/**************************************************************************
 **
 **   File:         Point.h
 **
 **   Description:  Generic analysis point
 **
 **   Author:       B. Butler
 **   Created:      3-28-11
 **
 **************************************************************************/

#ifndef Point_h
#define Point_h

#include "MomentObj.h"
#include "TVector3.h"

class Point : public MomentObj {
	public:
	
		Point() { }
      	Point(const TVector3& v) : x(v) { }
		Point(const MomentObj& m) : MomentObj(m) { }
      	Point(const TVector3& v,const MomentObj& m) : MomentObj(m), x(v) { }
		virtual ~Point() { }
		
		virtual TObject* Clone(const char*) const { //for (somewhat) deep copies
			Point* obj = new Point(*this);
			return obj;
		}
		
		//copy from obj to this, but do not clone sub-objects in ObjVecObj, just copy the pointers
		void ShallowClone(const Point& obj) {
			Reset();
			m_keyMap = obj.m_keyMap;
			
			x = obj.x;
			
			map<MomKey, MapObj*>::iterator iter = m_keyMap.begin();
			for(; iter!=m_keyMap.end(); iter++) {
				if(iter->second->Type()==OBJ) {
					iter->second = new ObjVecObj(*(ObjVecObj*)iter->second);
				}
				else iter->second = iter->second->Clone(); //simple types (moments) copied
			}			
		}
		
		TVector3 x;

		void Show() const {
			MomentObj::Show();
		
			//List X/Y/Z
			TString first = "X";
			TString second = "double";
			TString third = TString::Format("%f",x.X());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;
			
			first = "Y";
			second = "double";
			third = TString::Format("%f",x.Y());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;
			
			first = "Z";
			second = "double";
			third = TString::Format("%f",x.Z());
			while(first.Length()<18) {first = TString(first + " ");}
			while(second.Length()<12) {second = TString(second + " ");}	
			while(third.Length()<27) {third = TString(third + " ");}	
			cout << "* " << first << " | " << second << " | " << third << " *"<< endl;

			cout << "*******************************************************************" << endl;
		
		}
		
		ClassDef(Point,1);

	private:
};

#endif  /* Point_h */
