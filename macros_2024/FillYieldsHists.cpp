void FillYieldsHists(int spec = 211) {

    TString dataFileName = TString::Format("../pid_data_wPhi_noWin_pT/pid_124Xe_%d.root", spec);
    TFile *dataFile = new TFile(dataFileName.Data());

    auto dataHist0 = (TH2D *)dataFile->Get("momTheta0");
    dataHist0->SetName("dataHist0");
    dataHist0->SetTitle("dataHist0");
    auto dataHist1 = (TH2D *)dataFile->Get("momTheta1");
    dataHist1->SetName("dataHist1");
    dataHist1->SetTitle("dataHist1");
    auto dataHist = (TH2D *)dataFile->Get("momTheta");
    dataHist->SetName("dataHist");
    dataHist->SetTitle("dataHist");

    auto dataPT0 = (TH2D *)dataFile->Get("pTTheta0");
    dataPT0->SetName("dataPT0");
    dataPT0->SetTitle("dataPT0");
    auto dataPT1 = (TH2D *)dataFile->Get("pTTheta1");
    dataPT1->SetName("dataPT1");
    dataPT1->SetTitle("dataPT1");
    auto dataPT = (TH2D *)dataFile->Get("pTTheta");
    dataPT->SetName("dataPT");
    dataPT->SetTitle("dataPT");

    TString embedFileName = TString::Format("data_1222/pid_%d_noWin.root", spec);
    TFile *embedFile = new TFile(embedFileName.Data());

    auto embedHist0 = (TH2D *)embedFile->Get("momTheta0Rat");
    embedHist0->SetName("embedHist0");
    embedHist0->SetTitle("embedHist0");
    auto embedHist1 = (TH2D *)embedFile->Get("momTheta1Rat");
    embedHist1->SetName("embedHist1");
    embedHist1->SetTitle("embedHist1");
    auto embedHist = (TH2D *)embedFile->Get("momThetaRat");
    embedHist->SetName("embedHist");
    embedHist->SetTitle("embedHist");

    auto embedPT0 = (TH2D *)embedFile->Get("pTTheta0Rat");
    embedPT0->SetName("embedPT0");
    embedPT0->SetTitle("embedPT0");
    auto embedPT1 = (TH2D *)embedFile->Get("pTTheta1Rat");
    embedPT1->SetName("embedPT1");
    embedPT1->SetTitle("embedPT1");
    auto embedPT = (TH2D *)embedFile->Get("pTThetaRat");
    embedPT->SetName("embedPT");
    embedPT->SetTitle("embedPT");

    TH2D *corrHist0 = new TH2D(*dataHist0);
    corrHist0->Divide(embedHist0);
    corrHist0->SetName("corrHist0");
    corrHist0->SetTitle("corrHist0");
    TH2D *corrHist1 = new TH2D(*dataHist1);
    corrHist1->Divide(embedHist1);
    corrHist1->SetName("corrHist1");
    corrHist1->SetTitle("corrHist1");
    TH2D *corrHist = new TH2D(*dataHist);
    corrHist->Divide(embedHist);
    corrHist->SetName("corrHist");
    corrHist->SetTitle("corrHist");

    TH2D *corrPT0 = new TH2D(*dataPT0);
    corrPT0->Divide(embedPT0);
    corrPT0->SetName("corrPT0");
    corrPT0->SetTitle("corrPT0");
    TH2D *corrPT1 = new TH2D(*dataPT1);
    corrPT1->Divide(embedPT1);
    corrPT1->SetName("corrPT1");
    corrPT1->SetTitle("corrPT1");
    TH2D *corrPT = new TH2D(*dataPT);
    corrPT->Divide(embedPT);
    corrPT->SetName("corrPT");
    corrPT->SetTitle("corrPT");

    double phiRatio = 125. / 360.;

    auto binsX = embedHist0->GetNbinsX();
    auto binsY = embedHist0->GetNbinsY();

    TH1D *piHistRaw = new TH1D("piHistRaw", "piHistRaw", binsX, 50, 560);
    TH1D *piHist = new TH1D("piHist", "piHist", binsX, 50, 560);

    for(int i = 1; i < binsX; i++) {
        Double_t valErr;
        auto value = dataHist->IntegralAndError(i, i, 1, binsY, valErr);
        value /= phiRatio;
        valErr /= phiRatio;

        piHistRaw->SetBinContent(i, value);
        piHistRaw->SetBinError(i, valErr);
    }

    for(int i = 1; i < binsX; i++) {
        Double_t valErr;
        auto value = corrHist->IntegralAndError(i, i, 1, binsY, valErr);
        value /= phiRatio;
        valErr /= phiRatio;

        piHist->SetBinContent(i, value);
        piHist->SetBinError(i, valErr);
    }

    Double_t rawError0;
    auto rawYield0 = dataHist0->IntegralAndError(1, binsX, 1, binsY, rawError0);
    Double_t rawError1;
    auto rawYield1 = dataHist1->IntegralAndError(1, binsX, 1, binsY, rawError1);
    Double_t rawError;
    auto rawYield = dataHist->IntegralAndError(1, binsX, 1, binsY, rawError);

    Double_t corrError0;
    auto corrYield0 = corrHist0->IntegralAndError(1, binsX, 1, binsY, corrError0);
    Double_t corrError1;
    auto corrYield1 = corrHist1->IntegralAndError(1, binsX, 1, binsY, corrError1);
    Double_t corrError;
    auto corrYield = corrHist->IntegralAndError(1, binsX, 1, binsY, corrError);

    auto rawYield01 = (rawYield0 + rawYield1) / phiRatio;
    auto rawError01 = sqrt(pow(rawError0, 2) + pow(rawError1, 2)) / phiRatio;

    auto rawYieldNet = rawYield / phiRatio;
    auto rawErrorNet = rawError / phiRatio;

    auto corrYield01 = (corrYield0 + corrYield1) / phiRatio;
    auto corrError01 = sqrt(pow(corrError0, 2) + pow(corrError1, 2)) / phiRatio;

    auto corrYieldNet = corrYield / phiRatio;
    auto corrErrorNet = corrError / phiRatio;

    cout << rawYield0 << "  " << rawYield1 << "  " << rawYield << endl;

    cout << endl;

    cout << "rawYield01: " << rawYield01 << " +/- " << rawError01 << "; Ratio: " << rawError01 / rawYield01 << endl;
    cout << "rawYield: " << rawYieldNet << " +/- " << rawErrorNet << "; Ratio: " << rawErrorNet / rawYieldNet << endl;

    cout << endl;

    cout << "corrYield01: " << corrYield01 << " +/- " << corrError01 << "; Ratio: " << corrError01 / corrYield01 << endl;
    cout << "corrYield: " << corrYieldNet << " +/- " << corrErrorNet << "; Ratio: " << corrErrorNet / corrYieldNet << endl;

    TString outFileName = TString::Format("./dataYields_1230/yields_%d_noWin.root", spec);

    TFile *outFile =  new TFile(outFileName.Data(),"RECREATE");

    corrHist0->Write();
    corrHist1->Write();
    corrHist->Write();
    corrPT0->Write();
    corrPT1->Write();
    corrPT->Write();

    piHistRaw->Write();
    piHist->Write();

}