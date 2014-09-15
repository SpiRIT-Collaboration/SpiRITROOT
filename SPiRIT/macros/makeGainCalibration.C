{
  TString dataDir = "~/Common/data/pulser_20140821/";

  STGenerator *g = new STGenerator("gain");
  g -> SetParameterDir("~/geniejhang/SpiRITROOT.RIKEN-20140819to21/SPiRIT/parameters");
  g -> SetPedestalData("pedestal_pulser_20140821.root");
  g -> SetPersistence();

  g -> AddData(1.0, dataDir + "pulser_1.0V.graw");
  g -> AddData(2.0, dataDir + "pulser_2.0V.graw");
  g -> AddData(2.5, dataDir + "pulser_2.5V.graw");
  g -> AddData(3.0, dataDir + "pulser_3.0V.graw");
  g -> AddData(4.0, dataDir + "pulser_4.0V.graw");
  g -> AddData(5.0, dataDir + "pulser_5.0V.graw");

  g -> SetOutputFile("gainCalibration_pulser_20140821.root");

  g -> Print();
  g -> StartProcess();
}
