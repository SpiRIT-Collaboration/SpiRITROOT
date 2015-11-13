// =================================================
//  GETFileChecker Class
// 
//  Description:
//    Check if the file exists or not
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 09. 01
// =================================================

// GETDecoder
#include "GETFileChecker.hh"

// ROOT
#include "TString.h"
#include "TSystem.h"
#include "TObjArray.h"
#include "TObjString.h"

// STL
#include <iostream>

TString
GETFileChecker::CheckFile(TString filename)
{
  if (filename(0, 1) == "~")
    filename.Replace(0, 1, gSystem -> HomeDirectory());

  TString nextData = filename;

  TObjArray *pathElements = 0;
  pathElements = nextData.Tokenize("/");

  Int_t numElements = pathElements -> GetLast();

  TString path = "";
  if (numElements == 0)
    path = gSystem -> pwd();
  else {
    if (filename(0, 1) == "/")
      path.Append("/");

    for (Int_t i = 0; i < numElements; i++) {
      path.Append(((TObjString *) pathElements -> At(i)) -> GetString());
      path.Append("/");
    }
  }

  TString tempDataFile = ((TObjString *) pathElements -> Last()) -> GetString();
  delete pathElements;

  nextData = gSystem -> Which(path, tempDataFile);
  if (!nextData.EqualTo("")) {
    std::cout << "== [GETFileChecker] File exist: " << filename << std::endl;

    return nextData.Data();
  } else {
    std::cout << "== [GETFileChecker] File does not exist: " << filename << std::endl;

    return "";
  }

  return "";
}
