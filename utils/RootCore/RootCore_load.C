#include <iostream>
#include <fstream>
#include <string>
#include "TSystem.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TUUID.h"

int RootCore_load()
{
	TString rootcoreenv("RootCoreEnv.");
	rootcoreenv.Append(TUUID().AsString());
	rootcoreenv.Append(".txt");

	cout << "Loading RootCore packages..." << endl;
	if(!gSystem->Getenv("ROOTCOREDIR")) {
		if(!gSystem->AccessPathName(rootcoreenv.Data())) gSystem->Unlink(rootcoreenv.Data());
		TString cmd("source utils/RootCore/scripts/setup.sh; echo ROOTCOREDIR=$ROOTCOREDIR >> RootCoreEnv.txt; echo ROOTCOREBIN=$ROOTCOREBIN >> RootCoreEnv.txt; echo PATH=$PATH >> RootCoreEnv.txt; echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH >> RootCoreEnv.txt; echo DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH >> RootCoreEnv.txt; echo PYTHONPATH=$PYTHONPATH >> RootCoreEnv.txt;");
		cmd.ReplaceAll("RootCoreEnv.txt", rootcoreenv.Data());
		gSystem->Exec(cmd.Data());
	
		string buffer;
		ifstream fh(rootcoreenv.Data());
    	if (!fh.is_open()) {
      		cout << "RootCore_load ERROR opening temporary RootCore environment variable dump file " << rootcoreenv.Data() << endl;
      		return 1;
    	}

		getline(fh,buffer);
		while (!fh.eof()) {
			TString line(buffer);
			if(line.IsNull()) {
				getline(fh,buffer);
				continue;
			}
			
			TObjArray* arr = line.Tokenize("=");
			if(arr->GetEntries()!=2) {
				cout << "RootCore_load ERROR parsed line into !=2 entries" << endl;
				return 1;
			}
	
			gSystem->Setenv(((TObjString*)arr->At(0))->String(),((TObjString*)arr->At(1))->String().Data());
		
			delete arr;
			getline(fh,buffer);	
		}
    	fh.close();
    	
    	gSystem->Unlink(rootcoreenv.Data());
    }

	return 0;	
}

