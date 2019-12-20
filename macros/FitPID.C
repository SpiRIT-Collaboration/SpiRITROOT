TH1F* ProfileX(TH2F* hist, TCutG* cutg);

TF1 *FitError(TH1F* t_hist)
{
  TCanvas c2("c2", "c2");
  TF1 fitError("ErrorFitTemp", "[0]/pow(x, [1]) + [2]");
  fitError.SetParameters(100,2,0);
  t_hist -> Fit(&fitError);

  TF1 *tf1 = nullptr;
  while(!tf1)
  {
    t_hist -> Draw("P");
    std::cout << "Please modify the fit with Fit Panel if needed. Then press Ctrl-C" << std::endl;
    c2.WaitPrimitive("CUTG", "[CUTG]");
    tf1 = t_hist -> GetFunction("ErrorFitTemp"); 
  }

  fitError.SetParameters(tf1->GetParameters());
  return (TF1*) fitError.Clone("ErrorFit");
}

void FitPID(const std::string& t_filename="data/RunSn132_ana.root")
{
  TFile output("PIDSigma.root", "RECREATE");

  TChain chain("cbmsim");
  chain.Add(t_filename.c_str());

  auto BBE = [](double *x, double *par)
  {
    double fitval;
    double beta = x[0]/sqrt(x[0]*x[0] + 931.5*par[0]);
    return par[1]/pow(beta, par[4])*(par[2] - pow(beta, par[4]) - log(par[3] + pow(931.5*par[0]/x[0], par[5])));
  };

  TF1 tBBE("BBE", BBE, 0, 3000, 6);

  int nbinsx = 100;
  int nbinsy = 500;

  double momMin = 0;
  double momMax = 3000;
  double minDeDx = 0;
  double maxDeDx = 1000;

  TCanvas c1;
  TH2F PID("PID", "PID", nbinsx, momMin, momMax, nbinsy, minDeDx, maxDeDx);
  PID.Sumw2();
  tBBE.SetParameters(1,-800,13,190000,-80,6);

  chain.Draw("STData[0].vadedx:STData[0].vaMom.Mag()>>PID", 
             "STData[0].vaNRowClusters + STData[0].vaNLayerClusters > 15 && STData[0].recodpoca.Mag() < 20", "colz");

  for(int pdg : std::vector<int>{2212,1000010020,1000010030,1000020030, 1000020040})
  {
    int A = 1;
    std::string pname;
    switch(pdg)
    {
      case 2212: A = 1; pname = "proton"; break;
      case 1000010020: A = 2; pname = "deuteron"; break;
      case 1000010030: A = 3; pname = "Triton"; break;
      case 1000020030: A = 3; pname = "He3"; break;
      case 1000020040: A = 4; pname = "He4"; break;
    }
    tBBE.FixParameter(0,A);

    std::cout << "Please draw cut for " << pname << std::endl;
    TCutG *cutg = nullptr;
    while(!cutg)
      cutg = (TCutG*) c1.WaitPrimitive("CUTG", "[CUTG]");
    std::cout << "Please modify the cutg if needed. Press Ctrl-C to continue" << std::endl;
    c1.WaitPrimitive("temp", "[CUTG]");

    auto profile = ProfileX(&PID, cutg);//PID.ProfileX("_px", 1, -1, "s [CUTG]");

    TH1F PIDMean("PIDMean", "Mean;momentum (MeV/c2);dEdX", nbinsx, momMin, momMax);
    TH1F PIDStd("PIDStd", "Std;momentum (MeV/c2);Std", nbinsx, momMin, momMax);

    for(int i = 1; i < PID.GetNbinsX(); ++i)
    {
      PIDMean.SetBinContent(i, profile->GetBinContent(i));
      PIDStd.SetBinContent(i, profile->GetBinError(i));
    }

    PIDMean.Fit(&tBBE);
    tBBE.SetNpx(1000);
    PID.Draw("colz");
    PIDMean.Draw("PE same");

    std::cout << "Please Inspect the result of the fit. You may adjust the parameters in the fit panel. Press Ctrl-C to continue" << std::endl;
    c1.WaitPrimitive("CUTG", "[CUTG]");

    // Obtain the tf1 after adjustment is made on fit panel
    auto newBBE = (TF1*) PIDMean.GetFunction("BBE");
    auto error = FitError(&PIDStd);

    output.cd();
    tBBE.SetParameters(newBBE->GetParameters());
  
    auto BBEClone = (TF1*) tBBE.Clone(TString::Format("BEE%d", pdg));
    BBEClone -> Write();
    error -> Clone(TString::Format("PIDSigma%d", pdg))->Write();
  }
}

TH1F* ProfileX(TH2F* hist, TCutG* cutg)
{
  // don't know why I can't get desirable error from default TH2::ProfileX
  // reimplement to avoid any probelms
  int ny = hist -> GetNbinsY();
  double ymin = hist -> GetYaxis() -> GetBinLowEdge(1);
  double ymax = hist -> GetYaxis() -> GetBinUpEdge(ny);

  int nx = hist -> GetNbinsX();
  double xmin = hist -> GetXaxis() -> GetBinLowEdge(1);
  double xmax = hist -> GetXaxis() -> GetBinUpEdge(nx);

  TH1F *profile = new TH1F(TString::Format("%s_px", hist->GetName()), "", nx, xmin, xmax);
  for(int i = 1; i < hist->GetNbinsX(); ++i)
  {
    TH1F temp("temp", "temp", ny, ymin, ymax);
    for(int j = 1; j < ny; ++j)
      if(cutg -> IsInside(hist->GetXaxis()->GetBinCenter(i), hist->GetYaxis()->GetBinCenter(j)))
        temp.SetBinContent(j, hist->GetBinContent(i, j));
    TF1 gaus("gaus", "gaus");
    if(temp.GetEntries() > 0)
      temp.Fit(&gaus, "Q");
    else gaus.SetParameters(0,0,0);
    profile->SetBinContent(i, gaus.GetParameter(1));
    profile->SetBinError(i, gaus.GetParameter(2));
  }
  return profile;
}
