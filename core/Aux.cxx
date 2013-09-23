/**************************************************************************
 **
 **   File:         Aux.h
 **
 **   Description:  Auxiliary functions that are problematic for CINT
 **                 
 **
 **   Author:       B. Butler
 **
 **   Created:      5-23-12
 **   Modified:
 **
 **************************************************************************/

#ifndef Aux_cxx
#define Aux_cxx

#include "Aux.h"
#include "Error.h"
#include "TRegexp.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TProof.h"
#include "TFileCollection.h"
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include "THashList.h"
#include "TFileInfo.h"

vector<TString> GetRegexp(MomentObj* obj, const char* regexp) {

	TRegexp re(regexp);

	set<TString> setds;
	vector<TString> ds;

	map<MomKey,MapObj*>::const_iterator iter = obj->Keys().begin();
	for(; iter!=obj->Keys().end();++iter) {
		TString key(iter->first.Data());
		if(key.Contains(re)) {
			if(iter->second->Type()!=STRING) {
				TString msg("Value for key ");
				msg.Append(iter->first);
				msg.Append(" not of type STRING, exiting.");
				Abort(msg);
			}
			setds.insert(iter->second->String());
		}
	}
	
	set<TString>::iterator setiter = setds.begin();
	for(; setiter!=setds.end(); ++setiter) ds.push_back(*setiter);
	return ds;
}

vector<TString> GetMoreOutputs(MomentObj* obj, const TString& inDS) {
	vector<TString> alloutputs = GetRegexp(obj,"^output_.+$");
	
	if(alloutputs.size()==0) return alloutputs;
	
	bool forceoutputs = false;
	obj->Get("FORCEOUTPUTS",forceoutputs);
	if(forceoutputs) {
		return alloutputs;
	}
				
	vector<TString> dsoutputs = GetRegexp(obj,"^output_[0-9]+$");
	
	if( alloutputs.size() == dsoutputs.size() ) {

		TObjArray* arr = inDS.Tokenize(".");
		TRegexp re("^[0-9]+$");
		vector<TString> numbers;
		for(int i = 0; i<arr->GetEntries(); ++i) {
			TString str(((TObjString*)arr->At(i))->String());
			if(str.Contains(re)) {
				str.Prepend("output_");
				numbers.push_back(str);
			}
		}
		arr->SetOwner(true);
		delete arr;
		
		int nds = dsoutputs.size();
		dsoutputs.clear();
		vector<TString>::iterator iter = numbers.begin();
		for(; iter!=numbers.end(); ++iter) {
			if(obj->Exists(*iter)) {
				dsoutputs.push_back(obj->String(*iter));
			}
		}
		
		if((dsoutputs.size()==1)&&(nds==1)) {
			return dsoutputs;
		}
		else if(dsoutputs.size()==1) {
			cout << "GetMoreOutputs: INFO You have two or more additional output files targeted to a specific dataset number." << endl;
			cout << "GetMoreOutputs: INFO Only the one consistent with your Grid input dataset name will be added to the output dataset." << endl;
			return dsoutputs;
		}
		else if((!dsoutputs.size())&&(!numbers.size())) {
			cout << "GetMoreOutputs: INFO You have one or more additional output files targeted to a specific dataset number." << endl;
			cout << "GetMoreOutputs: INFO Your Grid input dataset name is not consistent with any specific dataset number." << endl;
			Abort("GetMoreOutputs: ERROR You must set FORCEOUTPUTS=true in your ProofAna config object before running on the Grid.");	
		}
		else if(!dsoutputs.size()) {
			cout << "GetMoreOutputs: INFO You have one or more additional output files targeted to a specific dataset number." << endl;
			cout << "GetMoreOutputs: INFO Your Grid input dataset name is not consistent with any of your targeted output files." << endl;		
			cout << "GetMoreOutputs: INFO The dataset-targeted output files will not be added to the output dataset." << endl;
			return dsoutputs;
		}
		else { //dsoutputs.size()>1
			cout << "GetMoreOutputs: INFO You have one or more additional output files targeted to a specific dataset number." << endl;
			cout << "GetMoreOutputs: INFO Your Grid input dataset name is consistent with more than one of these dataset numbers." << endl;
			Abort("GetMoreOutputs: ERROR You must set FORCEOUTPUTS=true in your ProofAna config object before running on the Grid.");	
		}
	}
	else { //At least one non-DS-number-based output
		cout << "GetMoreOutputs: INFO You have one or more additional output files not targeted to a specific dataset number." << endl;
		Abort("GetMoreOutputs: ERROR You must set FORCEOUTPUTS=true in your ProofAna config object before running on the Grid");
	}
	return alloutputs; //never reach here
}

vector<TString> GetDatasets(MomentObj* obj) {
	return GetRegexp(obj,"^name_.+$");
}

void PrintMissing(TProof* proof, TString dataset){
    TFileCollection* fileCol = proof->GetMissingFiles();

    if(!fileCol)
      return;

    cout << "printing missing files! " << fileCol->GetNFiles() << " missing files " << endl;

    if(fileCol->GetNFiles() > 0 ){

      //Opening output file list for writing
      TString misslist(dataset);
      misslist.Append("_missing.txt");

      ofstream afh(misslist.Data());
      if (!afh.is_open()) {
      	cout << "ERROR opening missing files list " << misslist << endl;
      	return;
      }
      else {
      	cout << "Writing file list " << misslist << " which contains " <<
	      	fileCol->GetNFiles() << " missing files " << endl;
      }

      TObjLink* sublnk = fileCol->GetList()->FirstLink();
      while (sublnk) {
      	TFileInfo* info = (TFileInfo*)sublnk->GetObject();
      	afh << info->GetFirstUrl()->GetUrl() << endl;
      	sublnk = sublnk->Next();
      }

      afh.close();
  }
}
#endif
