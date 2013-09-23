/**************************************************************************
 **
 **   File:         EventBuilderBase.h
 **
 **   Description:  EventBuilder base class
 **
 **
 **   Author:       B. Butler
 **
 **   Created:      2-23-11
 **   Modified:
 **
 **************************************************************************/

#ifndef EventBuilderBase_h
#define EventBuilderBase_h

#include <TSelector.h>
#include <TChain.h>
#include <TFile.h>
#include <TLeaf.h>
#include <TBranchElement.h>
//#include <TBranchObject.h>
#include "Error.h"
#include <map>
#include <set>
#include <iostream>
#include <limits>
#include "Key.h"

class AnalysisBase;
class SelectorBase;
class Config;

using namespace std;

class BranchContainer {
public :

	BranchContainer() : Type(0), Obj(0), Branch(0), Delete(0), Loaded(false), TBE(false) { }
	~BranchContainer() { if(Delete) Delete(Obj); }

	const type_info* Type;
	void* Obj;
	TBranch* Branch;
	void (*Delete)(void* ptr);
	bool Loaded;
	bool TBE;

};

class ThinContainer {
public :

	ThinContainer() : Thin(0) { }
	~ThinContainer() { }

	set<int> Indices;
	BranchKey Reference; //Used for index thinning
	void (*Thin)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices);

};

class SizeContainer {
public :

	SizeContainer() : Update(0) { }
	~SizeContainer() { }

	BranchKey Reference;
	void (*Update)(const BranchKey& key, const BranchKey& refkey);
};

class EventBuilderBase {
public :

	EventBuilderBase(TTree * /*tree*/ =0) { }
	virtual 			~EventBuilderBase() { }

	//access
	static TTree*		     Tree() { return fBaseChain->GetTree(); }
	static const set<BranchKey>& BranchNames() { return fBranchNames; }
	static const set<BranchKey>& UsedBranchNames() { return fUsedBranchNames; }
	static Bool_t                BranchExists(const BranchKey& key){ return ( fBranchNames.find(key)!=fBranchNames.end() ); }
	static void		     SetInputList(TList* input) { fInput = input; }

	static Long64_t		Entry() { return fEntry; }
	static void			SetSelector(SelectorBase* selector) { fSelector = selector; }
	static SelectorBase*	Selector() { return fSelector; }
	static void			SetConfig(Config* config);
	static Config*		Cfg() { return fConfig; };
	static const TString& Filename() { return fFilename; };

	static void     	InitBase(TTree* tree) { if (!tree) return; fBaseChain = tree; }
	static Bool_t		NotifyBase();
	static Bool_t  		LoadTree(Long64_t entry);
	static void			LoadAndThinBranches();
	static void			SetBranchStatusSlim(); //turns on only specific branches when slimming raw trees
	static void			SetBranchStatusOn(); //turns on all branches
	static void			CopyAddresses(TTree* tree); //copies addresses to target tree when writing raw trees
	static void			SlimBranch(const BranchKey& key) { fAllSlimBranchNames.insert(key); } //adds branch to slim, only call in EventBuilder::Initialize!

	virtual void     	Init(TTree* /*tree*/) { /* to be overridden by derived class if needed */ }
	virtual Bool_t   	Notify() {/* to be overridden by derived class if needed */ return kTRUE;}
	virtual Bool_t  	CopyEvent(AnalysisBase* /*evt */) {/* to be overridden by derived class */ return kTRUE;}
	virtual void		WriteAuxTrees(TDirectory* /*file */) {/* to be overridden by derived class, if needed */}
	virtual void		Initialize() {/* to be overridden by derived class, if needed */}

	//Dynamic load methods

	//Get a reference
	template <class T>
	static const T& Get(const BranchKey& key) {

		map<BranchKey,BranchContainer>::iterator branch = LoadBranch<T>(key);

		return *static_cast<T*>(branch->second.Obj);
	}

	//Get a pointer
	template <class T>
	static const T* GetP(const BranchKey& key) {

		map<BranchKey,BranchContainer>::iterator branch = LoadBranch<T>(key);

		return static_cast<T*>(branch->second.Obj);
	}

	//Get a non-const reference, use only for thinning
	template <class T>
	static T& GetNonConst(const BranchKey& key) {

		map<BranchKey,BranchContainer>::iterator branch = LoadBranch<T>(key);

		return *static_cast<T*>(branch->second.Obj);
	}

	//Get a non-const pointer, use only for thinning
	template <class T>
	static T* GetNonConstP(const BranchKey& key) {

		map<BranchKey,BranchContainer>::iterator branch = LoadBranch<T>(key);

		return static_cast<T*>(branch->second.Obj);
	}

	//Get a pointer, you own it and must delete it
	template <class T>
	static T* GetOwnedP(const BranchKey& key) {

		map<BranchKey,BranchContainer>::iterator branch = LoadBranch<T>(key);

		static set<BranchKey> copy;

		//Copy if not owned by us, a basic type (not a TBranchElement), or flagged for copy
		if(!branch->second.Delete || !branch->second.TBE || copy.find(key)!=copy.end()) return new T(*static_cast<T*>(branch->second.Obj));

		//Set flag for detecting if copying would be better
		if(fCalledBefore.find(key)!=fCalledBefore.end()) copy.insert(key);
		else fCalledBefore.insert(key);

		//Remove ownerskip from EventBuilder
		void* obj = branch->second.Obj;
		branch->second.Obj = 0;
		branch->second.Loaded = false;

		return static_cast<T*>(obj);
	}

	//Load a branch, but do not set branch address
	//Intended for use with classes which do the SetBranchAddress themselves and expect it to be loaded
	static void Load(const BranchKey& key) {

		if(!Tree()) Abort("EventBuilderBase: ERROR Load() cannot be called without a loaded tree");

		map<BranchKey,BranchContainer>::iterator branch = fBranches.find(key);

		//if loading branch for the first time
		if(branch==fBranches.end()) {
			if(fBranchNames.find(key)==fBranchNames.end()) {
				TString msg("EventBuilderBase: ERROR no branch named \"");
				msg.Append(key.Data());
				msg.Append("\" found in tree. Problem file is: ");
				msg.Append(Tree()->GetTree()->GetCurrentFile()->GetName());
				Abort(msg);
			}

			branch = fBranches.insert(map<BranchKey,BranchContainer>::value_type(key,BranchContainer())).first;
			if(fUsed) fUsedBranchNames.insert(key);

			branch->second.Branch = Tree()->GetBranch(key.Data());
		}

		//if branch not loaded yet, load it
		if(!branch->second.Loaded) {
			branch->second.Branch->GetEntry(fEntry);
			branch->second.Loaded=true;
		}

		return;
	}

	//Remove all entries by default
	template <class T>
	static void ThinBranch(const BranchKey& key) {

		if(Active(key)) {
			ThinContainer* entry = &fThinBranches[key];
			if(!entry->Thin) { //don't overwrite existing setup
				entry->Thin = &Thin<T>;
				entry->Reference = key;
			}
		}
	}

	template <class T>
	static void ThinBranch(const BranchKey& key, int index) {

		if(Active(key)) {
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &Thin<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinBranch type template parameter or ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = key;
			entry->Indices.insert(index);
		}
	}

	template <class T>
	static void ThinBranch(const BranchKey& key, const vector<int>& indices) {

		if(Active(key)) {
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &Thin<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinBranch type template parameter or ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = key;
			vector<int>::const_iterator iter = indices.begin();
			for(; iter!=indices.end(); ++iter) entry->Indices.insert(*iter);
		}
	}

	template <class T>
	static void ThinIndexBranch(const BranchKey& key, int index) {

		if(Active(key)) {
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &ThinIndex<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinIndexBranch type template parameter or ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = key;
			entry->Indices.insert(index);
		}
	}

	template <class T>
	static void ThinIndexBranch(const BranchKey& key, const vector<int>& indices) {

		if(Active(key)) {
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &ThinIndex<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinIndexBranch type template parameter or mixed ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = key;
			vector<int>::const_iterator iter = indices.begin();
			for(; iter!=indices.end(); ++iter) entry->Indices.insert(*iter);
		}
	}

	template <class T>
	static void ThinIndexBranch(const BranchKey& key, const BranchKey& refkey, int index) {

		if(Active(key)) {
			if(key!=refkey && !Active(refkey)) {
				TString msg("EventBuilderBase: ERROR branch ");
				msg.Append(key.Data());
				msg.Append(" is designated as an index branch referring to branch ");
				msg.Append(refkey.Data());
				msg.Append(", but the latter is not present in the output slim");
				Abort(msg);
			}
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &ThinIndex<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinIndexBranch type template parameter or ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = refkey;
			entry->Indices.insert(index);
		}
	}

	template <class T>
	static void ThinIndexBranch(const BranchKey& key, const BranchKey& refkey, const vector<int>& indices) {

		if(Active(key)) {
			if(key!=refkey && !Active(refkey)) {
				TString msg("EventBuilderBase: ERROR branch ");
				msg.Append(key.Data());
				msg.Append(" is designated as an index branch referring to branch ");
				msg.Append(refkey.Data());
				msg.Append(", but the latter is not present in the output slim");
				Abort(msg);
			}
			ThinContainer* entry = &fThinBranches[key];
			void (*f)(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) = &ThinIndex<T>;
			if(entry->Thin != f && entry->Indices.size()) {
				TString msg("EventBuilderBase: ERROR Inconsistent ThinIndexBranch type template parameter or mixed ThinBranch/ThinIndexBranch calls for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			entry->Thin = f;
			entry->Reference = refkey;
			vector<int>::const_iterator iter = indices.begin();
			for(; iter!=indices.end(); ++iter) entry->Indices.insert(*iter);
		}
	}

	template <class T>
	static void SetSizeBranch(const BranchKey& key, const BranchKey& refkey) {

		if(Active(key)) {
			SizeContainer* entry = &fSizeBranches[key];
			entry->Reference = refkey;
			entry->Update = &UpdateSize<T>;
		}
	}

	static TList* fInput;

	ClassDef(EventBuilderBase,0);

protected :

	static Bool_t fAddressChange;

private :

	static Config*	fConfig;
	static TTree* fBaseChain;
	static Long64_t fEntry;
	static SelectorBase* fSelector;
	static TString fFilename;

	static Bool_t fCache;
	static Bool_t fUsed;

	static set<BranchKey> fBranchNames;
	static set<BranchKey> fAllSlimBranchNames;
	static set<BranchKey> fSlimBranchNames;
	static set<BranchKey> fUsedBranchNames;
	static map<BranchKey, ThinContainer> fThinBranches;
	static map<BranchKey, SizeContainer> fSizeBranches;
	static map<BranchKey,BranchContainer> fBranches;

	static set<BranchKey> fCalledBefore;

	static bool Active(const BranchKey& key);

	template <class T>
	static map<BranchKey,BranchContainer>::iterator LoadBranch(const BranchKey& key) {

		static const type_info* type = &typeid(T*);

		if(!Tree()) Abort("EventBuilderBase: ERROR LoadBranch() cannot be called without a loaded tree");

		map<BranchKey,BranchContainer>::iterator branch = fBranches.find(key);
		bool checkAddress = false;
		//if loading branch for the first time
		if(branch==fBranches.end()) {
			if(fBranchNames.find(key)==fBranchNames.end()) {
				TString msg("EventBuilderBase: ERROR no branch named \"");
				msg.Append(key.Data());
				msg.Append("\" found in tree. Problem file is: ");
				msg.Append(Tree()->GetTree()->GetCurrentFile()->GetName());
				Abort(msg);
			}

			branch = fBranches.insert(map<BranchKey,BranchContainer>::value_type(key,BranchContainer())).first;

			if(fUsed) fUsedBranchNames.insert(key);
			checkAddress = true;
		}
		else if(!branch->second.Delete) { //previously loaded with the Load() function, now we need the object itself
			checkAddress = true;
		}

		bool newMemory = false;
		if(checkAddress) {
			branch->second.Type = type; //This doesn't necessarily mean the type is correct, but means that the type will be consistent
			branch->second.Branch = Tree()->GetBranch(key.Data());
			if(!branch->second.Branch->GetAddress()) newMemory = true;
		}

		if(newMemory) {
			branch->second.Loaded = false; //force reload at new address, if loaded with Load() previously
			//Use SetBranchAddress rather than TBranch::SetAddress so address changes are forwarded properly to clone trees

			//Type check, unfortunately have to call SetBranchAddress twice not to break strict-aliasing rules
			T* ptr = 0;
			if(Tree()->SetBranchAddress(key.Data(), &ptr, &branch->second.Branch)!=0) {
				TString msg("EventBuilderBase: ERROR incorrect template type supplied for branch ");
				msg.Append(key.Data());
				Abort(msg);
			}
			Tree()->SetBranchAddress(key.Data(), (void*)&branch->second.Obj, &branch->second.Branch);

			if(branch->second.Branch->IsA() == TBranchElement::Class()) {
				branch->second.TBE = true;
				TClass* tclass = TClass::GetClass(((TBranchElement*)branch->second.Branch)->GetTypeName());
				if(!tclass) {
					TString msg("EventBuilderBase: ERROR no dictionary found for class ");
					msg.Append(((TBranchElement*)branch->second.Branch)->GetTypeName());
					Abort(msg);
				}
				branch->second.Delete = &DeletePointer<T*>;
			}
			else {
				int size = branch->second.Branch->GetLeaf(key.Data())->GetNdata();
				if(size == 1) {
					branch->second.Obj = new T();
					branch->second.Delete = &DeletePointer<T*>;
				}
				else { // It is an array
					branch->second.Obj = new T[size]();
					branch->second.Delete = &DeletePointerArray<T*>;
				}
				Tree()->SetBranchAddress(key.Data(), (T*)branch->second.Obj, &branch->second.Branch);
			}
		}

		if(!branch->second.Delete) { //an object not owned by us
			if(!newMemory && (branch->second.Branch->GetReadEntry()==fEntry)) branch->second.Loaded = true; //already loaded
			if(branch->second.Branch->IsA() == TBranchElement::Class()) { //recopy pointer every call as it could change and the operation is fast
				branch->second.TBE = true;
				branch->second.Obj = *(void**)branch->second.Branch->GetAddress();
			}
			else branch->second.Obj = (void*)branch->second.Branch->GetAddress();
		}

		//type check (should be fast...)
		if(branch->second.Type != type) {
			TString msg("EventBuilderBase: ERROR branch ");
			msg.Append(key.Data());
			msg.Append(" previously requested using type ");
			msg.Append(branch->second.Type->name());
			msg.Append(", requested type is ");
			msg.Append(type->name());
			Abort(msg);
		}

		//if branch not loaded yet, load it
		if(!branch->second.Loaded) {
			branch->second.Branch->GetEntry(fEntry);
			branch->second.Loaded=true;
		}

		return branch;
	}

	template <class T>
	static void DeletePointer(void* ptr) { delete static_cast<T*>(ptr); }
	template <class T>
	static void DeletePointerArray(void* ptr) { delete [] static_cast<T*>(ptr); }

	template <class T>
	static void Thin(const BranchKey& key, const BranchKey& /*refkey*/, const set<int>& indices) {
		//vectors only
		T& cont = GetNonConst<T>(key);
		T temp(cont);
		cont.clear();
		set<int>::const_iterator iter = indices.begin();
		for(; iter!=indices.end(); ++iter) cont.push_back(temp.at(*iter));
	}

	//This base template should never be used. Do not add any other base templates for ThinIndex!
	template <class T>
	static void ThinIndex(const BranchKey& /*key*/, const BranchKey& /*refkey*/, const set<int>& /*indices*/) {
		T obj;
		obj.DoNotUseThisBaseTemplate.CheckYourThinIndexBranchType();
	}

	template <class T>
	static void UpdateSize(const BranchKey& key, const BranchKey& refkey) {
		T& size = GetNonConst<T>(key);
		map<BranchKey,ThinContainer>::iterator iter = fThinBranches.find(refkey);
		if(iter != fThinBranches.end()) {
			size = iter->second.Indices.size();
		}
	}
};

//ThinIndex template specializations...not the best way to do this, but keeps the ThinIndex interface intact
template <>
inline void EventBuilderBase::ThinIndex<vector<int> >(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) {
	vector<int>& cont = GetNonConst<vector<int> >(key);
	vector<int> temp(cont);
	cont.clear();

	//index reference
	const set<int>* refindices = &indices;
	bool indexupdate = true;
	if(key!=refkey) {
		map<BranchKey,ThinContainer>::iterator iter = fThinBranches.find(refkey);
		if(iter != fThinBranches.end()) {
			refindices = &iter->second.Indices;
		}
		else indexupdate = false; //not a thinned branch
	}

	set<int>::const_iterator iter = indices.begin();
	for(; iter!=indices.end(); ++iter) {
		int index = temp.at(*iter);
		if(indexupdate) {
			set<int>::const_iterator target = refindices->find(index);
			if(target != refindices->end()) index = distance(refindices->begin(),target);
			else index = -1;
		}
		cont.push_back(index);
	}
}

template <>
inline void EventBuilderBase::ThinIndex<vector<vector<int> > >(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) {
	vector<vector<int> >& cont = GetNonConst<vector<vector<int> > >(key);
	vector<vector<int> > temp(cont);
	cont.clear();

	//index reference
	const set<int>* refindices = &indices;
	bool indexupdate = true;
	if(key!=refkey) {
		map<BranchKey,ThinContainer>::iterator iter = fThinBranches.find(refkey);
		if(iter != fThinBranches.end()) {
			refindices = &iter->second.Indices;
		}
		else indexupdate = false; //not a thinned branch
	}

	set<int>::const_iterator iter = indices.begin();
	for(; iter!=indices.end(); ++iter) {
		if(indexupdate) {
			vector<int>::iterator iter2 = temp.at(*iter).begin();
			for(; iter2 != temp.at(*iter).end(); ++iter2) {
				set<int>::const_iterator target = refindices->find(*iter2);
				if(target != refindices->end()) (*iter2) = distance(refindices->begin(),target);
				else (*iter2) = -1;
			}
		}
		cont.push_back(temp.at(*iter));
	}
}

template <>
inline void EventBuilderBase::ThinIndex<vector<UInt_t> >(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) {
	vector<unsigned int>& cont = GetNonConst<vector<unsigned int> >(key);
	vector<unsigned int> temp(cont);
	cont.clear();

	//index reference
	const set<int>* refindices = &indices;
	bool indexupdate = true;
	if(key!=refkey) {
		map<BranchKey,ThinContainer>::iterator iter = fThinBranches.find(refkey);
		if(iter != fThinBranches.end()) {
			refindices = &iter->second.Indices;
		}
		else indexupdate = false; //not a thinned branch
	}

	set<int>::const_iterator iter = indices.begin();
	for(; iter!=indices.end(); ++iter) {
		unsigned int index = temp.at(*iter);
		if(indexupdate) {
			set<int>::const_iterator target = refindices->find(index);
			if(target != refindices->end()) index = distance(refindices->begin(),target);
			else index = numeric_limits<unsigned int>::max();
		}
		cont.push_back(index);
	}
}

template <>
inline void EventBuilderBase::ThinIndex<vector<vector<UInt_t> > >(const BranchKey& key, const BranchKey& refkey, const set<int>& indices) {
	vector<vector<unsigned int> >& cont = GetNonConst<vector<vector<unsigned int> > >(key);
	vector<vector<unsigned int> > temp(cont);
	cont.clear();


	//index reference
	const set<int>* refindices = &indices;
	bool indexupdate = true;
	if(key!=refkey) {
		map<BranchKey,ThinContainer>::iterator iter = fThinBranches.find(refkey);
		if(iter != fThinBranches.end()) {
			refindices = &iter->second.Indices;
		}
		else indexupdate = false; //not a thinned branch
	}

	set<int>::const_iterator iter = indices.begin();
	for(; iter!=indices.end(); ++iter) {
		if(indexupdate) {
			vector<unsigned int>::iterator iter2 = temp.at(*iter).begin();
			for(; iter2 != temp.at(*iter).end(); ++iter2) {
				set<int>::const_iterator target = refindices->find(*iter2);
				if(target != refindices->end()) (*iter2) = distance(refindices->begin(),target);
				else (*iter2) = numeric_limits<unsigned int>::max();
			}
		}
		cont.push_back(temp.at(*iter));
	}
}


#endif
