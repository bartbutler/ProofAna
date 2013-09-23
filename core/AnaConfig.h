/**************************************************************************
 **
 **   File:         AnaConfig.h
 **
 **   Description:  Generic persistable configuration class
 **
 **   Author:       B. Butler
 **   Created:      2-24-11
 **
 **************************************************************************/

#ifndef AnaConfig_h
#define AnaConfig_h

#include "TString.h"
#include "MomentObj.h"
#include "TNamed.h"

class Config : public MomentObj {
	public:
		Config() { }	
		Config(const TString& name) : m_name(name) { }	
		Config(const TString& name, const TString& cfgfile) : m_name(name) {ReadFile(cfgfile);}  
		bool ReadFile(const TString& cfgfile);
		
		const char* GetName() const { return m_name.Data(); }
		void SetName(const TString& name) { m_name = name; }

		Int_t Write(const char* name = 0, Int_t option = 0, Int_t bufsize = 0) { 
			if(!name) return this->TObject::Write(m_name.Data(),option,bufsize);
			else return this->TObject::Write(name,option,bufsize);
		}
		Int_t Write(const char* name = 0, Int_t option = 0, Int_t bufsize = 0) const {
			if(!name) return this->TObject::Write(m_name.Data(),option,bufsize);
			else return this->TObject::Write(name,option,bufsize);
		}

		ClassDef(Config,1);
	
	private:

		TString m_name;
};

#endif  /* Config_h */
