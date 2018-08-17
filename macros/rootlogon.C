{
  SysInfo_t info;
  gSystem -> GetSysInfo(&info);
  TString osString = info.fOS;

  TString libString;
  if (osString.Index("Darwin") >= 0)
    libString = TString("/Users/ejungwoo/spirit/spiritroot.readonly/build/libSpiRIROOT-ReadOnly.dylib");
  else if (osString.Index("Linux") >= 0)
    libString = TString("/Users/ejungwoo/spirit/spiritroot.readonly/build/libSpiRIROOT-ReadOnly.so");

  if (gSystem -> Load(libString) != -1)
    cout << "spiritroot read-only loaded" << endl;
  else
    cout << "*** cannot load spiritroot read-only" << endl;
}
