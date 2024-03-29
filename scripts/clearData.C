// Macro to clear all unregistered PROOF-created datasets (non-merged output) which are no longer register. Use clearDatasets.C first.
// Usage root -l 'clearData.C("username@atlprf01")'
#include <TEnv.h>
#include <TProof.h>
#include <TString.h>
#include "getProof.C"

void clearData(TString url, TString dataset = "")
{
  	// Temp dir
  	TString tempdir = Form("%s/.proofanalysis", gSystem->TempDirectory());
  	if (gSystem->AccessPathName(tempdir)) {
    	Printf("runProof: creating the temporary directory"
	   		" (%s) ... ", tempdir.Data());
    	if (gSystem->mkdir(tempdir, kTRUE) != 0) {
      		Printf("runProof: could not assert / create the temporary directory"
	    		" (%s)", tempdir.Data());
      		return;
    	}
  	}
  
	// Get the PROOF Session
	TProof *proof = getProof(url, -1, tempdir.Data(), "ask");
	if (!proof) {
		Printf("runProof: could not start/attach a PROOF session");	
		return;
	}
  if(dataset=="")
    proof->ClearData();
  else
    proof->ClearData(TProof::kDataset, dataset);


	proof->ShowData();
}
