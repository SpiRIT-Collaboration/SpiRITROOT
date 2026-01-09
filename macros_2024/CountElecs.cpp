auto mpi = 139.57018;
auto me = 0.5109989461;
auto mp = 938.2720813;
auto mn = 939.565346;
auto md = 1875.612762;
auto mt = 2808.921112;
auto mhe3 = 2808.39132;
auto mal = 3727.379378;

//Double_t particleMass = mpi; // Select particle you'll going to analyze
Double_t particleMass = me; // Select particle you'll going to analyze

Double_t fpFunc(Double_t *x, Double_t *p) {
  if (x[0] < 0)
    x[0] = -x[0];

  const Double_t m = particleMass;
  Double_t totalE = TMath::Sqrt(m*m + x[0]*x[0]);
  Double_t beta = x[0]/totalE;
  Double_t gamma = 1./TMath::Sqrt(1 - beta*beta);

  Double_t betaP3 = TMath::Power(beta, p[3]);

  return p[0]/betaP3*(p[1] - betaP3 + TMath::Log(p[2] + TMath::Power(1./(beta*gamma), p[4])));
}

void CountElecs() {
    TFile *f1 = new TFile("data/pid_elecs/pid_15.root");

    auto hist = (TH2D*)f1->Get("pidVAHist");
    //auto hist = (TH2D*)f1->Get("pidFlatLog");

    TFile *f2 = new TFile("data/pid_piElec/pid_15.root");

    auto hist2 = (TH2D*)f2->Get("pidFlatLog");

    //hist->Draw("COLZ");
    //hist->GetYaxis()->SetRangeUser(0, 50);

    TH1D *elecs[20];
    TH1D *piElecs[40];

    cout << "making hists" << endl;

    for(int i = 0; i < 20; i++) {
        elecs[i] = hist->ProjectionY(TString::Format("elecs_%02d", i).Data(), i * 160 / 20 + 1, (i + 1) * 160 / 20 + 1);
        elecs[i]->SetTitle(TString::Format("elecs_%.0f-%.0f", -80. + i * 160. / 20., -80. + (i + 1) * 160. / 20.).Data());
    }

    cout << "done with elecs" << endl;

    for(int i = 0; i < 40; i++) {
        piElecs[i] = hist2->ProjectionY(TString::Format("piElecs_%02d", i).Data(), i * 400 / 40 + 1, (i + 1) * 400 / 40 + 1);
        piElecs[i]->SetTitle(TString::Format("piElecs_%.0f-%.0f", -200. + i * 400. / 40., -200. + (i + 1) * 400. / 40.).Data());
    }

    TH1D *elecMean = new TH1D("elecMean", "elecMean", 50, 0, 50);
    TH1D *elecMeanPi = new TH1D("elecMeanPi", "elecMeanPi", 50, -3, 0.5);
    TH1D *piMean = new TH1D("piMean", "piMeanPi", 50, -0.5, 0.5);

    TH1D *elecSig = new TH1D("elecSig", "elecSig", 10, 0, 20);
    TH1D *elecSigPi = new TH1D("elecSigPi", "elecSigPi", 10, 0, 0.5);
    TH1D *piSig = new TH1D("piSig", "piSig", 10, 0, 0.5);

    cout << "done with pi elecs" << endl;

    TF1 *fitEl;
        fitEl = new TF1("fitEl", "gaus", 0, 50);
        //fitEl = new TF1("fitEl", "gaus", -3, 0.5);

    TF1 *fitM = new TF1("fitM", "gaus(0) + gaus(3)", -3, .5);
    fitM->SetParameters(1, 0, .05, 5, -.5, .05);
    fitM->SetParLimits(0, 0, 1000);
    fitM->SetParLimits(1, -0.1, 0.1);
    fitM->SetParLimits(2, 0, 0.1);
    fitM->SetParLimits(3, 0, 1000);
    fitM->SetParLimits(4, -3, 0.5);
    fitM->SetParLimits(5, 0, 0.1);

    TF1 *fitMPi[40];
    for(int i = 0; i < 40; i++) {
        fitMPi[i] = new TF1(TString::Format("FitMPi_%02d", i).Data(), "gaus", -3, .5);
        fitMPi[i]->SetLineColor(6);
    }

    TF1 *fitMEl[40];
    for(int i = 0; i < 40; i++) {
        fitMEl[i] = new TF1(TString::Format("FitMEl_%02d", i).Data(), "gaus", -3, .5);
        fitMEl[i]->SetLineColor(8);
    }

    double IntEl[20] = {};
    double IntElErr[20] = {};

    double IntMEl[40] = {};
    double IntMElErr[40] = {};

    double dedxEl[20] = {};
    double dedxElErr[20] = {};

    double dedxMEl[40] = {};
    double dedxMElErr[40] = {};

    cout << "fits setup" << endl;

    TCanvas *cEle = new TCanvas("cEle", "cEle", 1);
    cEle->Divide(5, 4);
    for(int i = 0; i < 20; i++) {
        cEle->cd(i + 1);
        elecs[i]->Draw();
        auto max = elecs[i]->GetMaximum();
        auto maxBin = elecs[i]->GetBinCenter(elecs[i]->GetMaximumBin());
        fitEl->SetParameters(max, maxBin, 10);
        //fitEl->SetParameters(max, 0.5, 0.1);
        elecs[i]->Fit("fitEl", "Q");
        elecMean->Fill(fitEl->GetParameter(1));
        elecSig->Fill(fitEl->GetParameter(2));
        IntEl[i] = fitEl->Integral(0, 50);
        IntElErr[i] = fitEl->IntegralError(0, 50);
        dedxEl[i] = fitEl->GetParameter(1);
        dedxElErr[i] = fitEl->GetParError(1);
        //IntEl[i] = fitEl->Integral(-3, 0.5);
        //IntElErr[i] = fitEl->IntegralError(-3, 0.5);
    }

    TCanvas *cMEle = new TCanvas("cMEle", "cMEle", 1);
    cMEle->Divide(5, 4);
    for(int i = 0; i < 20; i++) {
        //cout << i << " now processing." << endl;
        cMEle->cd(i + 1);
        piElecs[i]->Draw();
        auto max = piElecs[i]->GetMaximum();
        auto maxBin = piElecs[i]->GetBinCenter(piElecs[i]->GetMaximumBin());
        double maxEl = 0;
        double maxElBin = 0;
        for(int r = 1; r < piElecs[i]->GetNbinsX() + 1; r++) {
            if(piElecs[i]->GetBinCenter(r) < -0.25) {
                if(piElecs[i]->GetBinContent(r) > maxEl) {
                    maxEl = piElecs[i]->GetBinContent(r);
                    maxElBin = piElecs[i]->GetBinCenter(r);
                }
            }
        }
        fitM->SetParameters(max, 0, .05, maxEl, maxElBin, .05);
        auto res = piElecs[i]->Fit("fitM", "SQ");
        auto pars = fitM->GetParameters();
        elecMeanPi->Fill(fitM->GetParameter(4));
        elecSigPi->Fill(fitM->GetParameter(5));
        piMean->Fill(fitM->GetParameter(1));
        piSig->Fill(fitM->GetParameter(2));
        fitMPi[i]->SetParameters(pars[0], pars[1], pars[2]);
        fitMEl[i]->SetParameters(pars[3], pars[4], pars[5]);
        fitMPi[i]->Draw("SAME");
        fitMEl[i]->Draw("SAME");
        Double_t parsEl[3] = {pars[3], pars[4], pars[5]};
        Double_t covEle[9] = {};
        if(res.Get() != nullptr) {
            //cout << i << " is good" << endl;
            //cout << "(" << endl;
            auto covBase = res->GetCovarianceMatrix().GetMatrixArray();
            for(int x = 3; x < 6; x++) {
                for(int y = 3; y < 6; y++) {
                    int index = x * 6 + y;
                    double val = covBase[index];
                    covEle[(x - 3) * 3 + y - 3] = val;
                    //cout << covEle[(x - 3) * 3 + y - 3] << " ";
                }
                //cout << endl;
            }
        }
        //cout << ")" << endl;
        IntMEl[i] = fitMEl[i]->Integral(-3, 0.5);
        IntMElErr[i] = fitMEl[i]->IntegralError(-3, 0.5, parsEl, covEle);
        dedxMEl[i] = fitM->GetParameter(4);
        dedxMElErr[i] = fitM->GetParError(4);
        //cout << "IntMEl" << i << ": " << IntMEl[i] << "; IntMElErr" << i << ": " << IntMElErr[i] << endl;
        //cout << i << " has finished" << endl;
    }

    TCanvas *cMPos = new TCanvas("cMPos", "cMPos", 1);
    cMPos->Divide(5, 4);
    for(int i = 20; i < 40; i++) {
        cMPos->cd(i - 20 + 1);
        piElecs[i]->Draw();
        auto max = piElecs[i]->GetMaximum();
        auto maxBin = piElecs[i]->GetBinCenter(piElecs[i]->GetMaximumBin());
        //cout << "MaxBin: " << maxBin - 0.25 << "; NBins: " << piElecs[i]->GetNbinsX() << endl;
        double maxEl = 0;
        double maxElBin = 0;
        for(int r = 1; r < piElecs[i]->GetNbinsX() + 1; r++) {
            if(piElecs[i]->GetBinCenter(r) < -0.25) {
                //cout << piElecs[i]->GetBinCenter(r) << endl;
                if(piElecs[i]->GetBinContent(r) > maxEl) {
                    maxEl = piElecs[i]->GetBinContent(r);
                    maxElBin = piElecs[i]->GetBinCenter(r);
                }
            }
        }
        fitM->SetParameters(max, 0, .05, maxEl, maxElBin, .05);
        auto res = piElecs[i]->Fit("fitM", "SQ0");
        auto pars = fitM->GetParameters();
        auto parErrs = fitM->GetParErrors();
        elecMeanPi->Fill(fitM->GetParameter(4));
        elecSigPi->Fill(fitM->GetParameter(5));
        piMean->Fill(fitM->GetParameter(1));
        piSig->Fill(fitM->GetParameter(2));
        double piErr[3] = {parErrs[0], parErrs[1], parErrs[2]};
        double elErr[3] = {parErrs[3], parErrs[4], parErrs[5]};
        fitMPi[i]->SetParameters(pars[0], pars[1], pars[2]);
        fitMPi[i]->SetParErrors(piErr);
        fitMEl[i]->SetParameters(pars[3], pars[4], pars[5]);
        fitMEl[i]->SetParErrors(elErr);
        fitMPi[i]->Draw("SAME");
        fitMEl[i]->Draw("SAME");
        Double_t parsEl[3] = {pars[3], pars[4], pars[5]};
        Double_t covEle[9] = {};
        if(res.Get() != nullptr) {
            //cout << i << " is good" << endl;
            //cout << "(" << endl;
            auto covBase = res->GetCovarianceMatrix().GetMatrixArray();
            for(int x = 3; x < 6; x++) {
                for(int y = 3; y < 6; y++) {
                    int index = x * 6 + y;
                    double val = covBase[index];
                    covEle[(x - 3) * 3 + y - 3] = val;
                    //cout << covEle[(x - 3) * 3 + y - 3] << " ";
                }
                //cout << endl;
            }
        }
        //cout << ")" << endl;
        IntMEl[i] = fitMEl[i]->Integral(-3, 0.5);
        IntMElErr[i] = fitMEl[i]->IntegralError(-3, 0.5, parsEl, covEle);
        dedxMEl[i] = fitM->GetParameter(4);
        dedxMElErr[i] = fitM->GetParError(4);
        //cout << maxEl << " " << maxElBin << endl;
    }

    cout << "=====//Elecs//===== " << endl;
    for(int i = 0; i < 20; i++) {
        cout << -80 + i * 160. / 20 + 160. / 20. / 2 << ": " << IntEl[i]  << "; " << IntElErr[i] << "; " << elecs[i]->GetEntries() << endl;
    }

    TF1 *fitPi = new TF1("fitPi", fpFunc, -500, 500, 5);
    fitPi->SetParameters(1.64217, 7.82343, 0.903356, 2.05422, -3.50896);

    fitPi->SetParLimits(0, 0.1, 20);
    fitPi->SetParLimits(1, 0.5, 50);
    fitPi->SetParLimits(2, 1e-6, 1e3);
    fitPi->SetParLimits(3, 1.0, 8.0);
    fitPi->SetParLimits(4, -8.0, 8.0);

    double Mome[7] = {};
    double MomeEr[7] = {};
    double Elec[7] = {};
    double ElecEr[7] = {};
    double Posi[7] = {};
    double PosiEr[7] = {};
    double dedxElec[7] = {};
    double dedxElecEr[7] = {};
    double dedxPosi[7] = {};
    double dedxPosiEr[7] = {};

    double MMome[20] = {};
    double MMomeEr[20] = {};
    double MElec[20] = {};
    double MElecEr[20] = {};
    double MPosi[20] = {};
    double MPosiEr[20] = {};
    double dedxMElec[20] = {};
    double dedxMElecEr[20] = {};
    double dedxMPosi[20] = {};
    double dedxMPosiEr[20] = {};

    double Mome2[14] = {};
    double MomeEr2[14] = {};
    double dedxElec2[14] = {};
    double dedxElecEr2[14] = {};

    for(int i = 0; i < 7; i++) {
        Mome[i] = 80. / 10. * (i + 3 + 0.5);
        Mome2[i * 2] = 80. / 10. * (i + 3 + 0.5);
        Mome2[i * 2 + 1] = 80. / 10. * (i + 3 + 0.5);
        Elec[i] = IntEl[6 - i] / (80. / 10.) / elecs[0]->GetBinWidth(1);
        ElecEr[i] = IntElErr[6 - i] / (80. / 10.) / elecs[0]->GetBinWidth(1);
        Posi[i] = IntEl[12 + i] / (80. / 10.) / elecs[0]->GetBinWidth(1);
        PosiEr[i] = IntElErr[12 + i] / (80. / 10.) / elecs[0]->GetBinWidth(1);
        dedxElec[i] = dedxEl[6 - i];
        dedxElecEr[i] = dedxElErr[6 - i];
        dedxElec2[i * 2] = dedxEl[6 - i];
        dedxElecEr2[i * 2] = dedxElErr[6 - i];
        dedxPosi[i] = dedxEl[12 + i];
        dedxPosiEr[i] = dedxElErr[12 + i];
        dedxElec2[i * 2 + 1] = dedxEl[12 + i];
        dedxElecEr2[i * 2 + 1] = dedxElErr[12 + i];
    }

    for(int i = 0; i < 20; i++) {
        MMome[i] = 200. / 20. * (i + 0.5);
        MElec[i] = IntMEl[19 - i] / (200. / 20.) / piElecs[0]->GetBinWidth(1);
        MElecEr[i] = IntMElErr[19 - i] / (200. / 20.) / piElecs[0]->GetBinWidth(1);
        MPosi[i] = IntMEl[20 + i] / (200. / 20.) / piElecs[0]->GetBinWidth(1);
        MPosiEr[i] = IntMElErr[20 + i] / (200. / 20.) / piElecs[0]->GetBinWidth(1);
        //dedxMElec[i] = pow(10, dedxMEl[19 - i]) * fitPi->Eval(MMome[i]);
        //dedxMElecEr[i] = dedxMElec[i] * log(10) * dedxMElErr[19 - i];
        //dedxMPosi[i] = pow(10, dedxMEl[20 + i]) * fitPi->Eval(MMome[i]);
        //dedxMPosiEr[i] = dedxMPosi[i] * log(10) * dedxMElErr[20 + i];
    }

    TGraphErrors *elecG = new TGraphErrors(7, Mome, Elec, MomeEr, ElecEr);     
    TGraphErrors *posiG = new TGraphErrors(7, Mome, Posi, MomeEr, PosiEr);     
    
    TGraphErrors *elecMG = new TGraphErrors(20, MMome, MElec, MMomeEr, MElecEr);     
    TGraphErrors *posiMG = new TGraphErrors(20, MMome, MPosi, MMomeEr, MPosiEr);     

    TCanvas *cGrph = new TCanvas("cGrph", "cGrph", 1);
    elecMG->Draw("AP");
    elecMG->SetLineColor(4);
    elecMG->SetLineWidth(3);
    elecMG->SetMarkerColor(4);
    elecMG->SetMarkerSize(3);
    elecMG->SetMarkerStyle(20);
    posiMG->Draw("SAMEP");
    posiMG->SetLineColor(2);
    posiMG->SetLineWidth(3);
    posiMG->SetMarkerColor(2);
    posiMG->SetMarkerSize(3);
    posiMG->SetMarkerStyle(21);
    elecG->Draw("SAMEP");
    elecG->SetLineColor(4);
    elecG->SetLineWidth(3);
    elecG->SetMarkerColor(4);
    elecG->SetMarkerSize(3);
    elecG->SetMarkerStyle(24);
    posiG->Draw("SAMEP");
    posiG->SetLineColor(2);
    posiG->SetLineWidth(3);
    posiG->SetMarkerColor(2);
    posiG->SetMarkerSize(3);
    posiG->SetMarkerStyle(25);

    TGraphErrors *elecGdedx = new TGraphErrors(7, Mome, dedxElec, MomeEr, dedxElecEr);     
    TGraphErrors *posiGdedx = new TGraphErrors(7, Mome, dedxPosi, MomeEr, dedxPosiEr);     
    
    TGraphErrors *elecMGdedx = new TGraphErrors(20, MMome, dedxMElec, MMomeEr, dedxMElecEr);     
    TGraphErrors *posiMGdedx = new TGraphErrors(20, MMome, dedxMPosi, MMomeEr, dedxMPosiEr);     

    TGraphErrors *elecGdedx2 = new TGraphErrors(14, Mome2, dedxElec2, MomeEr2, dedxElecEr2);     

    TCanvas *cGrphE = new TCanvas("cGrphE", "cGrphE", 1);
    //elecMGdedx->Draw("AP");
    //elecMGdedx->SetMarkerStyle(20);
    //posiMGdedx->Draw("SAMEP");
    //posiMGdedx->SetMarkerStyle(21);
    //elecGdedx->Draw("AP");
    //elecGdedx->SetMarkerStyle(24);
    //elecGdedx->Fit(fitPi);
    //posiGdedx->Draw("SAMEP");
    //posiGdedx->SetMarkerStyle(25);
    //posiGdedx->Fit(fitPi);
    elecGdedx2->Draw("AP");
    elecGdedx2->SetMarkerStyle(3);
    elecGdedx2->SetMarkerSize(3);
    elecGdedx2->SetLineWidth(3);
    elecGdedx2->Fit(fitPi);
    elecGdedx2->Fit(fitPi);
    elecGdedx2->Fit(fitPi);

    TFile *f3 = new TFile("data/pid_250805/pid.root");
    auto pidHist = (TH2D*)f3->Get("pidVAHist");

    TCanvas *cPID = new TCanvas("cPID", "cPID", 1);
    pidHist->Draw("COLZ");
    fitPi->Draw("SAME");

    /*TCanvas *cEMean = new TCanvas("cEMean", "cEMean", 1);
    elecMean->Draw();
    TCanvas *cESig = new TCanvas("cESig", "cESig", 1);
    elecSig->Draw();*/

    /*TCanvas *cEPMean = new TCanvas("cEPMean", "cEPMean", 1);
    elecMeanPi->Draw();
    TCanvas *cEPSig = new TCanvas("cEPSig", "cEPSig", 1);
    elecSigPi->Draw();

    TCanvas *cPMean = new TCanvas("cPMean", "cPMean", 1);
    piMean->Draw();
    TCanvas *cPSig = new TCanvas("cPSig", "cPSig", 1);
    piSig->Draw();*/

}