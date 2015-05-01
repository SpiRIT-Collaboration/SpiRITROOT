{
  TString dataDir = "data/DATADIR/";

  STGenerator *g = new STGenerator("gain");
  g -> SetParameterDir("parameters");
  g -> SetPedestalData("PEDESTAL_DATA.root", 0);
  g -> SetPositivePolarity(); // comment out or add kFALSE for negative polarity
  g -> SetPersistence();      // comment out for disabling store raw data
  g -> SetOutputFile("OUTPUT_FILENAME.root");

  g -> AddData(VOLTAGE, dataDir + "CORRESPONDING_DATA.graw");

  g -> Print();
  g -> StartProcess();
}
