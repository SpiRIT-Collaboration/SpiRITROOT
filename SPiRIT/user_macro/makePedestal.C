{
  TString dataDir = "data/DATADIR/";

  STGenerator *g = new STGenerator("pedestal");
  g -> SetParameterDir("parameters");
  g -> AddData(dataDir + "INPUT_RAW_DATA_FILENAME.graw");
  g -> SetOutputFile("OUTPUT_FILENAME.root");
  g -> SetStoreRMS();   // comment out for disabling
  g -> SetSumRMSCut(0); // comment out or 0 will disable cut

  g -> Print();

  g -> StartProcess();
}
