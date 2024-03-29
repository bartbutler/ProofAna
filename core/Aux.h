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

#ifndef Aux_h
#define Aux_h

#include "TString.h"
#include <vector>
#include "MomentObj.h"
#include "TProof.h"

vector<TString> GetRegexp(MomentObj* obj, const char* regexp);
vector<TString> GetMoreOutputs(MomentObj* obj, const TString& inDS);
vector<TString> GetDatasets(MomentObj* obj);
void 			PrintMissing(TProof* proof, TString dataset);

#endif
