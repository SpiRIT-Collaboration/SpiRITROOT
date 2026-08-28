void YieldsCalculate(int spec = 211) {

    TString dataFileName = TString::Format("data/pid_data/pid_%d.root", spec);
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

    TString embedFileName = TString::Format("data/pidEmbed251022/pid_%d_251024.root", spec);
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

    double phiRatio = 125. / 360.;

    auto binsX = embedHist0->GetNbinsX();
    auto binsY = embedHist0->GetNbinsY();

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

    TFile *outFile =  new TFile("./data/pid_data/yields_211.root","RECREATE");

    corrHist0->Write();
    corrHist1->Write();
    corrHist->Write();

}