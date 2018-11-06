void GetMomentumListFromMC(const std::string& t_filename = "data/Run2841_WithProton/TrackDistComp/Mom_350.0_400.0_Theta_0.0_10.0.mc.root", const std::string& t_output_name = "MomDist.txt")
{
  TFile file(t_filename.c_str());
  if(!file.IsOpen()) std::cerr << "File cannot be opened\n";
  TTree *tree = (TTree*) file.Get("cbmsim");
  if(!tree) std::cerr << "Tree cbmsim is not found in the file\n";
  std::ofstream output(t_output_name.c_str());
  if(!output.is_open()) std::cerr << "Cannot open output file \n";
  output << "Entry\tPx\tPy\tPz\tStartX\tStartY\tStartZ\n";

  TTreeReader reader(tree);
  TTreeReaderValue<TClonesArray> Momentum(reader, "PrimaryTrack");

  while(reader.Next())
  {
    auto MomArray = *Momentum;
    for(unsigned i = 0; i < MomArray.GetEntries(); ++i)
    {
      auto ptrack = (STMCTrack*) MomArray.At(i);
      output << reader.GetCurrentEntry() << "\t" << ptrack->GetPx() << "\t" << ptrack->GetPy() << "\t" << ptrack->GetPz() << "\t"
             << ptrack->GetStartX() << "\t" << ptrack->GetStartY() << "\t" << ptrack->GetStartZ() << "\n";   
    }
  }
}
