{
  TString dataDir = "~/Common/data/pulser_20140821/";

  STGainCheck *check = new STGainCheck();
  check -> SetParameterDir("~/geniejhang/SpiRITROOT.RIKEN-20140819to21/SPiRIT/parameters");
  check -> SetPedestalData("pedestal_pulser_20140821.root");
  check -> SetUAIndex(6);
  // If the line below is commented out, gain is not calibrated.
//  check -> SetGainCalibrationData("gainCalibration_pulser_20140821.root");
  check -> SetGainBase(-134.6, 535.05);
  check -> AddPulserData(dataDir + "pulser_1.0V.graw");
  check -> AddPulserData(dataDir + "pulser_2.0V.graw");
  check -> AddPulserData(dataDir + "pulser_2.5V.graw");
  check -> AddPulserData(dataDir + "pulser_3.0V.graw");
  check -> AddPulserData(dataDir + "pulser_4.0V.graw");
  check -> AddPulserData(dataDir + "pulser_5.0V.graw");
  TCanvas *cvs = check -> DrawSpectra();
}
