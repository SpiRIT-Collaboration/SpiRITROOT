void DrawGaus()
{
  TrackZFilter min_z_filter;
  EmbedFilter filter;
  UseVATracks va_tracks;
  PID pid;
  EmbedCut pid_cut("ProtonCut.root");
  ThetaPhi theta_phi;

  min_z_filter.AddRule(filter.AddRule(va_tracks.AddRule(pid.AddRule(pid_cut.AddRule(&theta_phi)))));

  std::string location = "data/Run2841_WithProton/GausPhi/";
  for(const auto& name : listdir(location))
  {
    if(IsDirectory(location + "/" + name))
    {
      std::string fullname = location + "/" + name + "/run2841_s*.bc8ea52.root"; 
      std::cout << "Drawing " << fullname << std::endl;
      DrawComplex drawer(fullname, "cbmsim");  //"data/Run2841_WithProton/GausTheta/Mom_350.0_450.0_GausTheta_20.0_5.0/run2841_s*.reco.tommy_branch.1751.bc8ea52.root", "cbmsim");
      auto hist = drawer.FillRule<TH2F>(filter, "hist", ";#theta (deg);#phi (deg)", 50,0,50,180,-180,180);

      DailyLogger logger(name);
      logger.SaveClass(hist);
    }
  }
}
