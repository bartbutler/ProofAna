#include <TList.h>
#include <TString.h>
#include <TFileCollection.h>
#include <TDataSetManagerFile.h>
#include <TProof.h>

void runLocal(TString dataset, TString treename = "", Long64_t nentries=TChain::kBigNumber,  Long64_t firstentry = 0)
{
	ProofAna* ana = new ProofAna();

	//Need to provide a TList for local running
	TList* list = new TList();
	ana->SetInputList(list);

	// Load dataset manager stuff, uses same local datasets as PROOF-Lite by design
	TString defaultdir(gSystem->HomeDirectory());
	defaultdir.Append("/.proof");
	TString dsetdir(gEnv->GetValue("ProofLite.Sandbox",defaultdir.Data()));
	dsetdir.Append("/datasets");
	dsetdir.Prepend("dir:");
	TDataSetManagerFile mgr(dsetdir);

	TString dsetfile(dataset);
	dsetfile.Append(".txt");
	dsetfile.Prepend("../filelists/");

	//Register dataset, if non-existent
	if(!mgr.ExistsDataSet(dataset)) {
		TFileCollection* dset = new TFileCollection(dataset,"",dsetfile);
		mgr.RegisterDataSet(dataset,dset,"V"); //This seems to return true regardless of success or failure at the moment
		if(treename.CompareTo("")!=0) {
			cout << "Setting default tree to " << treename << " (local and lite running)" << endl;
			TProof* lite = TProof::Open("lite://");
			if(lite->SetDataSetTreeName(dataset,treename)) {
				cout << "Failure to set default tree to " << treename << endl;
				return;
			}
			delete lite;
		}
	}
	mgr.ShowDataSets();

	// And yes, all this dataset garbage was to get the default tree name
	TFileCollection* dset = mgr.GetDataSet(dataset);
	TString defaultTree = dset->GetDefaultTreeName();

	// Make TChain from TFileCollection...doesn't seem like there is a more direct way
	if(defaultTree.First("/")==0) defaultTree.Remove(0,1);
	TChain* T = new TChain(defaultTree);
	TList* filelist = (TList*)dset->GetList();
	TIter next(filelist);
	TFileInfo* fileinfo = 0;
	while ((fileinfo = (TFileInfo*)next())) T->AddFile(fileinfo->GetCurrentUrl()->GetUrl());

	// Process TChain
	T->Process(ana,"", nentries, firstentry);
}

