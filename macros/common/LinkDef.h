#ifndef __COMMON__
#define __COMMON__

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all defined_ines;
#pragma link off all functions;
#pragma link C++ nestedclasses;

//Classes
#pragma link C++ function MakeTHX;
#pragma link C++ function CastTHSys;

#pragma link C++ class THSys+;

//This is actually required for Python iteration...wow
#pragma link C++ class map<TString, TH1*>+;
#pragma link C++ class pair<TString, TH1*>+;
#pragma link C++ class map<TString, TH1*>::*;
#pragma link C++ operators map<TString, TH1*>::iterator;
#pragma link C++ operators map<TString, TH1*>::const_iterator;
#pragma link C++ operators map<TString, TH1*>::reverse_iterator;

#pragma link C++ class THX<TH1I>+;
#pragma link C++ class THX<TH1F>+;
#pragma link C++ class THX<TH1D>+;

#pragma link C++ class THX<TH2I>+;
#pragma link C++ class THX<TH2F>+;
#pragma link C++ class THX<TH2D>+;

#pragma link C++ class THX<TH3I>+;
#pragma link C++ class THX<TH3F>+;
#pragma link C++ class THX<TH3D>+;

#pragma link C++ class Stacked+;
#pragma link C++ class SignalGrid1l2b+;

#endif

#endif

