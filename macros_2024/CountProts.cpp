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

void CountProts() {
    TFile *f2 = new TFile("data/pid_prots/pid_15.root");

    auto hist2 = (TH2D*)f2->Get("pidFlatLog");

    //hist->Draw("COLZ");
    //hist->GetYaxis()->SetRangeUser(0, 50);

    TH1D *piProts[40];

    cout << "making hists" << endl;

    for(int i = 0; i < 40; i++) {
        piProts[i] = hist2->ProjectionY(TString::Format("piElecs_%02d", i).Data(), i * 400 / 40 + 1, (i + 1) * 400 / 40 + 1);
        piProts[i]->SetTitle(TString::Format("piElecs_%.0f-%.0f", 200. + i * 400. / 40., 200. + (i + 1) * 400. / 40.).Data());
    }

    TH1D *elecMean = new TH1D("elecMean", "elecMean", 50, 0, 50);
    TH1D *elecMeanPi = new TH1D("elecMeanPi", "elecMeanPi", 50, -3, 0.5);
    TH1D *piMean = new TH1D("piMean", "piMeanPi", 50, -0.5, 0.5);

    TH1D *elecSig = new TH1D("elecSig", "elecSig", 10, 0, 20);
    TH1D *elecSigPi = new TH1D("elecSigPi", "elecSigPi", 10, 0, 0.5);
    TH1D *piSig = new TH1D("piSig", "piSig", 10, 0, 0.5);

    cout << "done with pi prots" << endl;

    TF1 *fitM = new TF1("fitM", "gaus(0) + gaus(3)", -3, .5);
    fitM->SetParameters(1, 0, .06, 5, -.5, .06);
    fitM->SetParLimits(0, 0, 1000);
    fitM->SetParLimits(1, -0.05, 0.05);
    fitM->SetParLimits(2, 1e-2, 0.06);
    //fitM->SetParLimits(3, 0, 100000);
    fitM->SetParLimits(4, 0, 3);
    //fitM->SetParLimits(5, 0, 1);

    TF1 *fitMPi[40];
    for(int i = 0; i < 40; i++) {
        fitMPi[i] = new TF1(TString::Format("FitMPi_%02d", i).Data(), "gaus", -3, .5);
        fitMPi[i]->SetLineColor(6);
    }

    TF1 *fitMPro[40];
    for(int i = 0; i < 40; i++) {
        fitMPro[i] = new TF1(TString::Format("FitMPro_%02d", i).Data(), "gaus", -3, .5);
        fitMPro[i]->SetLineColor(8);
    }

    double IntMPro[40] = {};
    double IntMProErr[40] = {};

    double dedxMPro[40] = {};
    double dedxMProErr[40] = {};

    cout << "fits setup" << endl;

    TCanvas *cMPro = new TCanvas("cMPro", "cMPro", 1);
    cMPro->Divide(5, 4);
    TCanvas *cMPro2 = new TCanvas("cMPro2", "cMPro2", 1);
    cMPro2->Divide(5, 4);
    for(int i = 0; i < 40; i++) {
        //cout << i << " now processing." << endl;
        if(i < 20)
            cMPro->cd(i + 1);
        else
            cMPro2->cd(i + 1 - 20);
        piProts[i]->Draw();
        piProts[i]->GetYaxis()->SetRangeUser(0, 50);
        piProts[i]->GetXaxis()->SetRangeUser(-1, 0.5);
        auto max = piProts[i]->GetMaximum();
        auto maxBin = piProts[i]->GetBinCenter(piProts[i]->GetMaximumBin());
        if(maxBin < 0.1) {
            maxBin = 0.25;
        }
        double maxPi = 0;
        double maxPiBin = 0;
        for(int r = 1; r < piProts[i]->GetNbinsX() + 1; r++) {
            if(piProts[i]->GetBinCenter(r) < 0.06 && piProts[i]->GetBinCenter(r) > -0.06) {
                if(piProts[i]->GetBinContent(r) > maxPi) {
                    maxPi = piProts[i]->GetBinContent(r);
                    maxPiBin = piProts[i]->GetBinCenter(r);
                }
            }
        }
        fitM->SetParameters(maxPi, 0, .05, max, maxBin, .1);
        auto res = piProts[i]->Fit("fitM", "S");
        auto pars = fitM->GetParameters();
        //elecMeanPi->Fill(fitM->GetParameter(4));
        //elecSigPi->Fill(fitM->GetParameter(5));
        //piMean->Fill(fitM->GetParameter(1));
        //piSig->Fill(fitM->GetParameter(2));
        fitMPi[i]->SetParameters(pars[0], pars[1], pars[2]);
        fitMPro[i]->SetParameters(pars[3], pars[4], pars[5]);
        fitMPi[i]->Draw("SAME");
        fitMPro[i]->Draw("SAME");
        Double_t parsPro[3] = {pars[3], pars[4], pars[5]};
        Double_t covPro[9] = {};
        if(res.Get() != nullptr) {
            //cout << i << " is good" << endl;
            //cout << "(" << endl;
            auto covBase = res->GetCovarianceMatrix().GetMatrixArray();
            for(int x = 3; x < 6; x++) {
                for(int y = 3; y < 6; y++) {
                    int index = x * 6 + y;
                    double val = covBase[index];
                    covPro[(x - 3) * 3 + y - 3] = val;
                    //cout << covEle[(x - 3) * 3 + y - 3] << " ";
                }
                //cout << endl;
            }
        }
        //cout << ")" << endl;
        IntMPro[i] = fitMPro[i]->Integral(-3, 0.5);
        IntMProErr[i] = fitMPro[i]->IntegralError(-3, 0.5, parsPro, covPro);
        dedxMPro[i] = fitM->GetParameter(4);
        dedxMProErr[i] = fitM->GetParError(4);
        //cout << "IntMEl" << i << ": " << IntMEl[i] << "; IntMElErr" << i << ": " << IntMElErr[i] << endl;
        //cout << i << " has finished" << endl;
    }

    TF1 *fitPi = new TF1("fitPi", fpFunc, -500, 500, 5);
    fitPi->SetParameters(1.64217, 7.82343, 0.903356, 2.05422, -3.50896);

    fitPi->SetParLimits(0, 0.1, 20);
    fitPi->SetParLimits(1, 0.5, 50);
    fitPi->SetParLimits(2, 1e-6, 1e3);
    fitPi->SetParLimits(3, 1.0, 8.0);
    fitPi->SetParLimits(4, -8.0, 8.0);

    double MMome[40] = {};
    double MMomeEr[40] = {};
    double MProt[40] = {};
    double MProtEr[40] = {};
    double dedxMProt[40] = {};
    double dedxMProtEr[40] = {};


    for(int i = 0; i < 40; i++) {
        MMome[i] = 200. / 20. * (i + 0.5) + 200;
        MProt[i] = IntMPro[i] / (200. / 20.) / piProts[0]->GetBinWidth(1);
        MProtEr[i] = IntMProErr[i] / (200. / 20.) / piProts[0]->GetBinWidth(1);
        //dedxMElec[i] = pow(10, dedxMEl[19 - i]) * fitPi->Eval(MMome[i]);
        //dedxMElecEr[i] = dedxMElec[i] * log(10) * dedxMElErr[19 - i];
        //dedxMPosi[i] = pow(10, dedxMEl[20 + i]) * fitPi->Eval(MMome[i]);
        //dedxMPosiEr[i] = dedxMPosi[i] * log(10) * dedxMElErr[20 + i];
    }

    TGraphErrors *protMG = new TGraphErrors(20, MMome, MProt, MMomeEr, MProtEr);     

    TCanvas *cGrph = new TCanvas("cGrph", "cGrph", 1);
    protMG->Draw("AP");
    protMG->SetLineColor(4);
    protMG->SetLineWidth(3);
    protMG->SetMarkerColor(4);
    protMG->SetMarkerSize(3);
    protMG->SetMarkerStyle(20);

    //TGraphErrors *elecGdedx = new TGraphErrors(7, Mome, dedxElec, MomeEr, dedxElecEr);     
    //TGraphErrors *posiGdedx = new TGraphErrors(7, Mome, dedxPosi, MomeEr, dedxPosiEr);     
    
    //TGraphErrors *elecMGdedx = new TGraphErrors(20, MMome, dedxMElec, MMomeEr, dedxMElecEr);     
    //TGraphErrors *posiMGdedx = new TGraphErrors(20, MMome, dedxMPosi, MMomeEr, dedxMPosiEr);     

    //TGraphErrors *elecGdedx2 = new TGraphErrors(14, Mome2, dedxElec2, MomeEr2, dedxElecEr2);     

    //TCanvas *cGrphE = new TCanvas("cGrphE", "cGrphE", 1);
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
    //elecGdedx2->Draw("AP");
    //elecGdedx2->SetMarkerStyle(3);
    //elecGdedx2->SetMarkerSize(3);
    //elecGdedx2->SetLineWidth(3);
    //elecGdedx2->Fit(fitPi);
    //elecGdedx2->Fit(fitPi);
    //elecGdedx2->Fit(fitPi);

    //TFile *f3 = new TFile("data/pid_250805/pid.root");
    //auto pidHist = (TH2D*)f3->Get("pidVAHist");

    //TCanvas *cPID = new TCanvas("cPID", "cPID", 1);
    //pidHist->Draw("COLZ");
    //fitPi->Draw("SAME");

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