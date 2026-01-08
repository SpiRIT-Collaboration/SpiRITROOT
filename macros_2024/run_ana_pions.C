#include "pionSelect.h"

void run_ana_pions(Int_t fRunNo = 1191, Int_t fSplitNo = 0) {

    string spec = "136Xe";
    if(fRunNo < 1600) {
        spec = "124Xe";
    }
    else {
        piSelect::targetZpos = -32.8;
    }

    auto inputFile = TString::Format("/mnt/cephfs/hira/SPIRIT_2024/July2025Recon/%s/run%04d_s%02d.reco.bulk.root", spec.c_str(), fRunNo, fSplitNo);
    auto outputFile = TString::Format("./data_reduc_noWin/%s/run%04d_noWin_s%02d.root", spec.c_str(), fRunNo, fSplitNo);

    if(gSystem->AccessPathName(inputFile.Data()))
	    gApplication->Terminate();

    // FairRun
    FairRunAna *fRun = new FairRunAna();

    fRun->SetSource(new FairFileSource(inputFile.Data()));
    fRun->SetOutputFile(outputFile.Data());

    STMacroTask *macro = new STMacroTask();
    macro -> AddInitFunction(piSelect::Init);
    macro -> AddFunction(piSelect::Exec);

    fRun->AddTask(macro);

    fRun->Init();
    fRun->Run();

	gApplication->Terminate();
}