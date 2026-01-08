{
    TFile *f1 =  new TFile("/mnt/spirit/analysis/user/curtis/newDaq/SpiRITROOT/macros_2024/data/HokusaiEmbed/data_1222/pid_-211_noWin.root");

    auto momTheta =  (TH2D *)f1->Get("momTheta");
    auto momThetaNet =  (TH2D *)f1->Get("momThetaNet");


    ofstream outFile;
    outFile.open("outFiles/momTheta.csv");

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

    ofstream outFileNet;
    outFileNet.open("outFiles/momThetaNet.csv");

    auto xAxisNet = momThetaNet->GetXaxis();
    auto yAxisNet = momThetaNet->GetYaxis();

    outFileNet << "p/Z(MeV)";

    for(int xbin = 1; xbin < momThetaNet->GetNbinsX() + 1; xbin++) {
        outFileNet << "  " << xAxisNet->GetBinCenter(xbin);
    }

    outFileNet << endl;
    outFileNet << "theta(deg)" << endl;

    for(int ybin = 1; ybin < momThetaNet->GetNbinsY() + 1; ybin++) {
        outFileNet << yAxisNet->GetBinCenter(ybin);
        for(int xbin = 1; xbin < momThetaNet->GetNbinsX() + 1; xbin++) {
            outFileNet << "  " << momThetaNet->GetBinContent(xbin, ybin);
        }
        outFileNet << endl;
    }

    outFileNet.close();

}