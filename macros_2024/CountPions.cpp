void CountPions() {
    TFile *f1 = new TFile("data/pid_250812_Mult50/pid.root");

    auto hist = (TH2D*)f1->Get("pidFlatLog");

    hist->Draw("COLZ");
    hist->GetYaxis()->SetRangeUser(-3, 0.5);

    TH1D *piMinus[20];
    TH1D *piPlus[20];

    for(int i = 0; i < 20; i++) {
        piMinus[i] = hist->ProjectionY(TString::Format("piMinus_%02d", i).Data(), 500 / 20 * i + 1, 500 / 20 * (i + 1) + 1);
        piMinus[i]->SetTitle(TString::Format("piMinus_%d-%d", 700 - 700 / 20 * i, 700 - 700 / 20 * (i + 1)).Data());
        piPlus[i] = hist->ProjectionY(TString::Format("piPlus_%02d", i).Data(), 500 / 20 * i + 501, 500 / 20 * (i + 1) + 501);
        piPlus[i]->SetTitle(TString::Format("piPlus_%d-%d", 700 / 20 * i, 700 / 20 * (i + 1)).Data());
    }

    TF1 *fitM = new TF1("fitM", "gaus(0) + gaus(3)", -3, .5);
    fitM->SetParameters(1, 0, .05, .5, -.5, .05);
    fitM->SetParLimits(1, -0.1, 0.1);
    fitM->SetParLimits(2, -0.5, 0.5);
    fitM->SetParLimits(4, -3, 0.5);
    fitM->SetParLimits(5, -0.5, 0.5);

    TCanvas *cMin = new TCanvas("cMin", "cMin", 1);
    cMin->Divide(5, 4);
    for(int i = 0; i < 5; i++) {
        for(int r = 0; r< 4; r++) {
            cMin->cd(5 * r + i + 1);
            piMinus[5 * r + i]->Draw();
            auto max = piMinus[5 * r + i]->GetMaximum();
            fitM->SetParameters(max, 0, .1, .5, -.5, .05);
            piMinus[5 * r + i]->Fit("fitM");
        }
    }

    /*TCanvas *cPlu = new TCanvas("cPlu", "cPlu", 1);
    cPlu->Divide(5, 4);
    for(int i = 0; i < 5; i++) {
        for(int r = 0; r< 4; r++) {
            cPlu->cd(5 * r + i + 1);
            piPlus[5 * r + i]->Draw();
        }
    }*/
}