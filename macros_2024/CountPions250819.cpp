void CountPions250819() {
    TFile *f1 = new TFile("data/pid_250819/pid_15.root");

    auto hist = (TH2D*)f1->Get("pidFlatLog");

    hist->Draw("COLZ");
    hist->GetYaxis()->SetRangeUser(-3, 0.5);

    TH1D *piMinus[20];
    TH1D *piPlus[20];

    for(int i = 0; i < 20; i++) {
        piMinus[i] = hist->ProjectionY(TString::Format("piMinus_%02d", i).Data(), 200 / 20 * i + 301, 200 / 20 * (i + 1) + 301);
        piMinus[i]->SetTitle(TString::Format("piMinus_%.0f-%.0f", 700 - 700. / 500. * 300 - 700. / 500. * 200. / 20 * i, 700 - 700. / 500. * 400 - 300. / 500. * 200. / 20 * (i + 1)).Data());
        piPlus[i] = hist->ProjectionY(TString::Format("piPlus_%02d", i).Data(), 200 / 20 * i + 801, 200 / 20 * (i + 1) + 801);
        piPlus[i]->SetTitle(TString::Format("piPlus_%.0f-%.0f", 700. / 500. * 200 / 20 * i + 700. / 500. * 300, 700 / 500. * 200. / 20 * (i + 1) + 700. / 500. * 300).Data());
    }

    TF1 *fitM = new TF1("fitM", "gaus(0) + gaus(3)", -3, .5);
    fitM->SetParameters(1, 0, .05, .5, -.5, .05);
    fitM->SetParLimits(1, -0.1, 0.1);
    fitM->SetParLimits(2, -0.5, 0.5);
    fitM->SetParLimits(4, -3, 0.5);
    fitM->SetParLimits(5, -0.5, 0.5);

    TF1 *fitPi[20];
    for(int i = 0; i < 20; i++) {
        fitPi[i] = new TF1("FitPi", "gaus", -3, .5);
        fitPi[i]->SetLineColor(6);
    }
    TF1 *fitEl[20];
    for(int i = 0; i < 20; i++) {
        fitEl[i] = new TF1("FitEl", "gaus", -3, .5);
        fitEl[i]->SetLineColor(8);
    }

    double IntPi[20] = {};
    double IntEl[20] = {};

    /*TCanvas *cMin = new TCanvas("cMin", "cMin", 1);
    cMin->Divide(5, 4);
    for(int i = 0; i < 5; i++) {
        for(int r = 0; r< 4; r++) {
            cMin->cd(5 * r + i + 1);
            piMinus[5 * r + i]->Draw();
            auto max = piMinus[5 * r + i]->GetMaximum();
            fitM->SetParameters(max, 0, .1, .5, -.5, .05);
            piMinus[5 * r + i]->Fit("fitM");
            auto pars = fitM->GetParameters();
            fitPi[5 * r + i]->SetParameters(pars[0], pars[1], pars[2]);
            fitEl[5 * r + i]->SetParameters(pars[3], pars[4], pars[5]);
            fitPi[5 * r + i]->Draw("SAME");
            fitEl[5 * r + i]->Draw("SAME");
            IntPi[5 * r + i] = fitPi[5 * r + i]->Integral(-3, 0.5);
            IntEl[5 * r + i] = fitEl[5 * r + i]->Integral(-3, 0.5);
        }
    }*/

    /*TCanvas *cMin = new TCanvas("cMin", "cMin", 1);
    cMin->Divide(2, 3);
    for(int i = 12; i < 17; i++) {
        cMin->cd(i - 12 + 1);
        piMinus[i]->Draw();
        auto max = piMinus[i]->GetMaximum();
        fitM->SetParameters(max, 0, .1, .5, -.5, .05);
        piMinus[i]->Fit("fitM");
        auto pars = fitM->GetParameters();
        fitPi[i]->SetParameters(pars[0], pars[1], pars[2]);
        fitEl[i]->SetParameters(pars[3], pars[4], pars[5]);
        fitPi[i]->Draw("SAME");
        fitEl[i]->Draw("SAME");
        IntPi[i] = fitPi[i]->Integral(-3, 0.5);
        IntEl[i] = fitEl[i]->Integral(-3, 0.5);
    }*/

    TCanvas *cPlu = new TCanvas("cPlu", "cPlu", 1);
    cPlu->Divide(5, 4);
    for(int i = 0; i < 5; i++) {
        for(int r = 0; r< 4; r++) {
            cPlu->cd(5 * r + i + 1);
            piPlus[5 * r + i]->Draw();
            gPad->SetLogy(1);
            /*auto max = piPlus[5 * r + i]->GetMaximum();
            fitM->SetParameters(max, 0, .1, .5, -.5, .05);
            piPlus[5 * r + i]->Fit("fitM");
            auto pars = fitM->GetParameters();
            fitPi[5 * r + i]->SetParameters(pars[0], pars[1], pars[2]);
            fitEl[5 * r + i]->SetParameters(pars[3], pars[4], pars[5]);
            fitPi[5 * r + i]->Draw("SAME");
            fitEl[5 * r + i]->Draw("SAME");
            IntPi[5 * r + i] = fitPi[5 * r + i]->Integral(-3, 0.5);
            IntEl[5 * r + i] = fitEl[5 * r + i]->Integral(-3, 0.5);*/
        }
    }

    /*TCanvas *cPlu = new TCanvas("cPlu", "cPlu", 1);
    cPlu->Divide(2, 3);
    for(int i = 3; i < 8; i++) {
        cPlu->cd(i - 3 + 1);
        piPlus[i]->Draw();
        auto max = piPlus[i]->GetMaximum();
        fitM->SetParameters(max, 0, .1, .5, -.5, .05);
        piPlus[i]->Fit("fitM");
        auto pars = fitM->GetParameters();
        fitPi[i]->SetParameters(pars[0], pars[1], pars[2]);
        fitEl[i]->SetParameters(pars[3], pars[4], pars[5]);
        fitPi[i]->Draw("SAME");
        fitEl[i]->Draw("SAME");
        IntPi[i] = fitPi[i]->Integral(-3, 0.5);
        IntEl[i] = fitEl[i]->Integral(-3, 0.5);
    }*/

    cout << "=====//Pions//===== " << endl;
    for(int i = 0; i < 20; i++) {
        cout << i << ": " << IntPi[i] << endl;
    }
    cout << "=====//Elecs//===== " << endl;
    for(int i = 0; i < 20; i++) {
        cout << i << ": " << IntEl[i] << endl;
    }

}