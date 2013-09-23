#include <fstream>
#include <string>

void WriteObject(TFile* options, const TString& filename, const TString& objname, TString name = "")
{
	TFile* myfile = new TFile(filename,"READ");
	TNamed* obj = (TNamed*)myfile->Get(objname);
	if(obj) {
		if(name.CompareTo("")) obj->SetName(name);
		options->cd();
		obj->Write();
	}
	else cout << "Object " << objname << " not found in file " << filename << endl;
	myfile->Close();
	delete myfile;
}

void ReadDatasetInfo(TString dataset, TObject* conf)
{
	TString filelist(dataset);
	filelist.Append(".txt").Prepend("../filelists/");

    ifstream fi(filelist);
    if (!fi.is_open()) {
      cout << "helperFunc: ERROR opening dataset file list " << filelist << endl;
    }
	else fi.close();
	
	TString filelistconfig("../filelists/");
	filelistconfig.Append(dataset).Append(".config");
	if(((Config*)conf)->ReadFile(filelistconfig)) return;
	
	cout << "If this sample is Monte Carlo, please create " << filelistconfig << " manually or use the makeFileListConfig.sh script to generate it automatically using AMI for proper event weighting." << endl;
}

