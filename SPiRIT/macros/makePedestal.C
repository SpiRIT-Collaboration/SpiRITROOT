{
  TString dataDir = "~/Common/data/pulser_20140821/";

  STGenerator *g = new STGenerator("pedestal");
  g -> SetParameterDir("~/geniejhang/SpiRITROOT.RIKEN-20140819to21/SPiRIT/parameters");
  g -> AddData(dataDir + "/pedestal.graw");
  g -> SetData(0);
  g -> SetOutputFile("pedestal_pulser_20140821.root");

  g -> Print();

  Int_t fEventList[] = {0, 1, 2, 3, 7, 8, 11, 13, 14, 15, 18, 19, 20, 22, 23};
  g -> SelectEvents(15, fEventList);

  g -> StartProcess();
}
