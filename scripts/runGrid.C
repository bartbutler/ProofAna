#include <TList.h>
#include <TString.h>
#include <TFileCollection.h>
#include <TDataSetManagerFile.h>
#include <fstream>
#include <iostream>

void runGrid(TString outDS, TString extraFlags = "--nGBPerJob=10", bool noSubmit = false)
{
	if(outDS.Length() > 131) outDS.Resize(131);

	if(!gSystem->Which(gSystem->Getenv("PATH"),"prun")) {
		cout << "prun not found in path. Have you loaded the GRID tools? Exiting." << endl;
		return;
	}

	TString symString = "ln -sf ../lib/libProofAna.par libProofAna.par";
	TString tarString = "tar -zchf ";
	tarString += outDS;
	tarString += ".tar.gz libProofAna.par options.root";
	gSystem->Exec(symString);
	gSystem->Exec(tarString);
	gSystem->Unlink("libProofAna.par");

	TString baseCommand = "prun --rootVer=\"5.34.07\" --cmtConfig=\"x86_64-slc5-gcc43-opt\" --inTarBall=";
	baseCommand += outDS;
	baseCommand += ".tar.gz --bexec \"tar -xvzf libProofAna.par; rm libProofAna.par; mkdir libProofAna/run; mv options.root libProofAna/run; mv libProofAna/scripts/runGridRuntime.C libProofAna/run; cd libProofAna; make solib\" --exec \"cd libProofAna; cd run; echo %IN > input.txt; root -l -b -q runGridRuntime.C\" ";
	baseCommand += extraFlags;

	// get the options.root file with options in it
	TFile* optionsFile = new TFile("options.root");

	TKey *key;
	Config* options = 0;
	TIter nextkey(optionsFile->GetListOfKeys());
	while ((key = (TKey*)nextkey())) {
		TString name = key->GetName();
		if(name.CompareTo("ProofAna")==0){
			Config* options = (Config*) key->ReadObj();
			break;
		}
	}

	if(!options) {
		cout << "No ProofAna configuration object found. Exiting." << endl;
		return;
	}
	if(!options->Exists("TREENAME")) {
		cout << "TREENAME missing in ProofAna config object, this likely means it is missing in your dataset config file. Exiting." << endl;
		return;
	}
	bool merge = true;
	options->Get("MERGE",merge);
	if(!merge) {
		cout << "MERGE must be set to true for the Grid. Exiting." << endl;
		return;
	}

	vector<TString> datasets = GetDatasets(options);

	bool onefailure = false;

	vector<TString>::iterator iter = datasets.begin();
	for(; iter != datasets.end(); iter++) {
		TString inDS = *iter;

		vector<TString> moreOutputs = GetMoreOutputs(options,inDS);

		TString outputsString = options->String("IDENTIFIER") + "." + options->String("DATASET") + ".root";
		vector<TString>::iterator iterMore = moreOutputs.begin();
		for(; iterMore != moreOutputs.end(); iterMore++) {
			outputsString += "," + options->String("IDENTIFIER") + "." + *iterMore + ".root";
		}
		cout << "outputs are " << outputsString << endl;

		TString command = baseCommand + " --outputs="+outputsString + " --inDS="+inDS + " --outDS="+outDS;

		cout << "--------------------------------------------------------------------------------------" << endl;
		cout << command << endl;
		cout << "--------------------------------------------------------------------------------------" << endl;

		bool failure = false;
		if(!noSubmit) {
			failure = gSystem->Exec(command);
			if(failure) {
				cout << "Submission failed once, retrying:" << endl;
				failure = gSystem->Exec(command);
			}
			if(failure) {
				cout << "Submission failed twice, retrying:" << endl;
				failure = gSystem->Exec(command);
			}
		}
		if(failure||noSubmit) {
			if(failure) cout << "3 submission failures, giving up and writing failed command to " << outDS << "_cmd.txt" << endl;

			ofstream afh;
			afh.open(outDS+"_cmd.txt",ofstream::app);
    		if (!afh.is_open()) {
      			cout << "ERROR opening " << outDS << "_cmd.txt for writing failed!" << endl;
      			return;
    		}

			afh << command << endl;
			afh << endl;

    		afh.close();

			if(failure) onefailure = true;
		}
	}
	//if(!onefailure && !noSubmit) gSystem->Unlink(outDS+".tar.gz");

	if(!noSubmit) cout << "Finished submitting all! Check Panda to see if things went through properly!" << endl;
	else cout << "Submission commands written to " << outDS << "_cmd.txt" << endl;

	if(onefailure) cout << "You had a least one persistent submission failure after 3 tries: see " << outDS << "_cmd.txt" << endl;
}

