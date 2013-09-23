/**************************************************************************
 **
 **   File:         AnaConfig.cxx
 **
 **   Description:  Generic persistable configuration class
 **
 **   Author:       B. Butler, J. Sjolin
 **   Created:      2-24-11
 **
 **************************************************************************/

#define AnaConfig_cxx

#include "AnaConfig.h"
#include <iostream>
#include <fstream>
#include <string>

using std::cout;
using std::endl;

bool Config::ReadFile(const TString& cfgfile) {
    string buffer;
    ifstream fh(cfgfile);
    if (!fh.is_open()) {
      cout << "Config: ERROR opening config file " << cfgfile << endl;
      return false;
    }
    getline(fh,buffer);    
	while (!fh.eof()) {
		TString n(buffer);

		n.ReplaceAll(" ",""); //get rid of spaces
		n.ReplaceAll("	",""); //get rid of tabs

		while (n.Contains("#")) {
			int comment = n.Index("#");
			n = TString(n(0,comment));
		}
		while (n.Contains("//")) {
			int comment = n.Index("//");
			n = TString(n(0,comment));
		}
		if(n.IsNull()) {
			getline(fh,buffer);      		
      		continue;				
		}
      	if (!n.Contains("=")) {
      		cout << "Config: Problem parsing line:" << endl << n << endl << "Fix your config file!" << endl;
			getline(fh,buffer);      		
      		continue;
      	}

		int equal = n.Index("=");
		TString key(n(0,equal));
		TString value(n(equal+1,n.Length()));

		/* On second thought, I don't see why special characters need to be forbidden

		//These are special characters which are allowed. If I left some out, feel free to add
		TString m(n);
		m.Remove(equal,1);
		m.ReplaceAll("-","");
		m.ReplaceAll("_","");
		m.ReplaceAll("/","");
		m.ReplaceAll(".","");
		
      	if (!m.IsAlnum()) {
      		cout << "Config: Problem parsing line:" << endl << n << endl << "Fix your config file!" << endl;
      		cout << "Config: If you need special characters in a key or value, add them as exceptions in AnaConfig.cxx." << endl;
			getline(fh,buffer);      		
      		continue;
      	}
      	
		*/
		
		Set(key,value);

		getline(fh,buffer);
    }
    fh.close();
    
    Show();
    return true;
}
