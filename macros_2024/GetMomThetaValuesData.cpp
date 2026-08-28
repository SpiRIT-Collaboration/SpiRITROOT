{
    TFile *f1 =  new TFile("/mnt/spirit/analysis/user/curtis/newDaq/SpiRITROOT/macros_2024/data/pid_data_wPhi_noWin_pT/pid_124Xe_-211.root");

    auto momTheta =  (TH2D *)f1->Get("momTheta");


    ofstream outFile;
    outFile.open("outFiles/momThetaData.csv");

    auto xAxis = momTheta->GetXaxis();
    auto yAxis = momTheta->GetYaxis();

    outFile << "p/Z(MeV)";

    for(int xbin = 1; xbin < momTheta->GetNbinsX() + 1; xbin++) {
        outFile << "  " << xAxis->GetBinCenter(xbin);
    }

    outFile << endl;
    outFile << "theta(deg)" << endl;

    for(int ybin = 1; ybin < momTheta->GetNbinsY() + 1; ybin++) {
        outFile << yAxis->GetBinCenter(ybin);
        for(int xbin = 1; xbin < momTheta->GetNbinsX() + 1; xbin++) {
            outFile << "  " << momTheta->GetBinContent(xbin, ybin);
        }
        outFile << endl;
    }

    outFile.close();


}