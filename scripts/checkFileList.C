// Macro to check config file/file list consistency
#include <TString.h>
#include <fstream>
#include <iostream>
#include <string>

void checkFileList(TString filelist, TString treename)
{
	TString configfile(filelist);
	configfile.ReplaceAll(".txt",".config");

    //Change this when we get to run/channel 350000 (lol)
    TH1D* runnumbers = new TH1D("runnumbers","runnumbers",250000,100000.5,350000.5);  
    TH1D* runnumbers_temp = new TH1D("runnumbers_temp","runnumbers_temp",250000,100000.5,350000.5);     

	TString filename("");		
	string buffer;
	ifstream fh(filelist);
    if (!fh.is_open()) {
      	cout << "ERROR opening filelist" << filelist << endl;
      	return;
    }
    getline(fh,buffer);
    filename = TString(buffer);
	int filecounter = 0;
	while (!fh.eof()) {
		++filecounter;
		TFile file(buffer);
		TTree* tree = (TTree*)file.Get(treename);
		if(!tree) {
			cout << "Tree " << treename << " not found in file " << buffer <<", skipping file." << endl;
			getline(fh,buffer);
			continue;
		}
		
		if(!(filecounter%100)) {
			cout << "File #" << filecounter << endl;
			cout << buffer << endl;
		}
		//cout << buffer << endl;
		
		gROOT->cd();

		runnumbers_temp->Reset();

		if(tree->FindBranch("mc_channel_number")) {
			if((treename.CompareTo("susy")==0)&&(filename.Contains("McAtNlo"))) tree->Draw("mc_channel_number>>runnumbers_temp","mcevt_weight[0][0]"); //R17 McAtNlo
			else tree->Draw("mc_channel_number>>runnumbers_temp"); //R17 MC
		}
		else {
			if((treename.CompareTo("susy")==0)&&(filename.Contains("McAtNlo"))) tree->Draw("RunNumber>>runnumbers_temp","mcevt_weight[0][0]"); //R16 McAtNlo		
			else tree->Draw("RunNumber>>runnumbers_temp");	 //R17 data and R16 data & MC
		}
		
		runnumbers->Add(runnumbers_temp);
		file.Close();
		getline(fh,buffer);
    }
    fh.close();  
	
	//A lot of this code is repeated from AnaConfig.cxx because CINT's handling of maps sucks
	
    string abuffer;
    ifstream afh(configfile);
    if (!afh.is_open()) {
      cout << "ERROR opening config file " << configfile << endl;
      return;
    }
    
    getline(afh,abuffer);    
	while (!afh.eof()) {
		TString n(abuffer);

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
		    cout << n << endl;
			getline(afh,abuffer);      		
      		continue;				
		}
      	if (!n.Contains("=")) {
      		cout << "Problem parsing line:" << endl << n << endl << "Fix your config file!" << endl;
			getline(afh,abuffer);      		
      		continue;
      	}

		int equal = n.Index("=");
		TString key(n(0,equal));
		TString value(n(equal+1,n.Length()));
 
 		if(!key.Contains("n_")) {
			getline(afh,abuffer); 		
 			continue;		
		}
		
		TString process = TString(key).ReplaceAll("n_","");
		int num = value.Atoi();
    	
    	if(filename.Contains("McAtNlo")&&treename.CompareTo("susy")) {
    		cout << "Potential problem found: " << filelist << ", tree " << treename << endl;
    		cout << "MC generated with MC@Nlo, make sure config file reflects weighted total events" << endl;
    	}
    	
    	if(num!=runnumbers->GetBinContent(runnumbers->FindBin(process.Atoi()))) {
    		cout << "Potential problem found: " << filelist << ", tree " << treename << endl;
    		cout << "Process/RunNumber " << process << ": config total = " << num << ", file total = " << (int)runnumbers->GetBinContent(runnumbers->FindBin(process.Atoi())) << endl;
    		cout << "This may not be a problem if the dataset is a skim" << endl;
    	}

		getline(afh,abuffer);
    }
    
    afh.close();
}
