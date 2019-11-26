#include "STUnfoldingTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STUnfoldingTask);

STUnfoldingTask::STUnfoldingTask() 
{}

STUnfoldingTask::~STUnfoldingTask()
{}

void STUnfoldingTask::SetMomBins(double t_min, double t_max, int t_det_bins, int t_gen_bins) {fMinMom = t_min; fMaxMom = t_max; fNDetMom = t_det_bins; fNGenMom = t_gen_bins; }

void STUnfoldingTask::SetThetaBins(double t_min, double t_max, int t_det_bins, int t_gen_bins) {fMinTheta = t_min; fMaxTheta = t_max; fNDetTheta = t_det_bins; fNGenMom = t_gen_bins; }

void STUnfoldingTask::LoadMCData(const std::string& t_treename, const std::string& t_filename)
{
  fMCTreeName = t_treename;
  fMCFileName = t_filename;
}

void STUnfoldingTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

InitStatus STUnfoldingTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fPDG = (TClonesArray*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  fDetectorBinning = new TUnfoldBinning("detector");
  fGeneratorBinning = new TUnfoldBinning("generator");

  fDetectorBinning -> AddAxis("P", fNDetMom, fMinMom, fMaxMom, true, true);
  fDetectorBinning -> AddAxis("Theta", fNDetTheta, fMinTheta, fMaxTheta, true, true);
 
  fGeneratorBinning -> AddAxis("P", fNGenMom, fMinMom, fMaxMom, true, true);
  fGeneratorBinning -> AddAxis("Theta", fNGenTheta, fMinTheta, fMaxTheta, true, true);

  fProtonHist = fDetectorBinning -> CreateHistogram("hist_data_recTUnfold");
  fResponseMatrix = TUnfoldBinning::CreateHistogramOfMigrations(fGeneratorBinning, fDetectorBinning, "hist_mcl_recgen");

  // create migration histogram
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Creating response matrix");
    TChain chain(fMCTreeName.c_str());
    chain.Add(fMCFileName.c_str());
    TTreeReader reader(&chain);
    TTreeReaderValue<std::vector<TVector3>> recodpoca(reader, "recodpoca");
    TTreeReaderValue<std::vector<TVector3>> recoMom(reader, "recoMom");
    TTreeReaderValue<std::vector<bool>> recoEmbedTag(reader, "recoEmbedTag");
    TTreeReaderValue<std::vector<double>> recodedx(reader, "recodedx");
    TTreeReaderValue<std::vector<int>> recoNRowClusters(reader, "recoNRowClusters"), recoNLayerClusters(reader, "recoNLayerClusters");
    TTreeReaderValue<TVector3> embedMom(reader, "embedMom");

    while(reader.Next())
    {
      int iBinGen = fGeneratorBinning -> GetGlobalBinNumber(embedMom -> Mag(), embedMom -> Theta());

      // find the embedded track
      double min_dpoca = 9999;
      int min_idx = -1;
      int ntracks = recoMom->size();
      for(int i = 0; i < ntracks; ++i)
        if(recoEmbedTag->at(i))
          if(min_dpoca > recodpoca->at(i).Mag())
            min_idx = i;

      // plot the tracks once the embedded track is found
      bool satisfy_cut = false;
      if(min_idx >= 0)
        if(recoNRowClusters->at(min_idx) + recoNLayerClusters->at(min_idx) > fMinClusters && recodpoca->at(min_idx).Mag() > fMaxDPoca)
          satisfy_cut = true;


      double detMom = fMinMom - 10;
      double detTheta = fMinTheta-10;
      if(satisfy_cut)
      {
        detMom = recoMom -> at(min_idx).Mag();
        detTheta = recoMom -> at(min_idx).Theta();
      }
      int iBinDet = fDetectorBinning -> GetGlobalBinNumber(detMom, detTheta);
      fResponseMatrix -> Fill(iBinGen, iBinDet);
    }
    fLogger -> Info(MESSAGE_ORIGIN, "Response matrix is created");
  }

  return kSUCCESS;
}


void STUnfoldingTask::Exec(Option_t *opt)
{
  auto data = (STData*) fData -> At(0);
  auto pdg = (STVectorI*) fPDG -> At(0);

  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    if(data->vaNRowClusters[part] + data->vaNLayerClusters[part] > fMinClusters && data->recodpoca[part].Mag() < fMaxDPoca)
      if(pdg->fElements[part] == 2212)
      {
        int iBinRec = fDetectorBinning -> GetGlobalBinNumber(data -> recoMom[part].Mag(), data -> recoMom[part].Theta());
        fProtonHist -> Fill(iBinRec);
      }
  }
}

void STUnfoldingTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  TUnfoldDensity unfold(fResponseMatrix,
                        TUnfold::kHistMapOutputHoriz,
                        TUnfold::kRegModeSize,
                        TUnfold::kEConstraintArea,
                        TUnfoldDensity::kDensityModeNone,
                        fGeneratorBinning,
                        fDetectorBinning);
  unfold.SetInput(fProtonHist,1.,1.);
  TGraph *lCurve=0;
  unfold.ScanLcurve(100,0.,0.,&lCurve);
  unfold.GetOutput("ProtonUnfold",
                   "data (TUnfolded);P_{TUNFOLD};#theta_{TUNFOLD}");

  outfile -> cd();
  outfile -> Write();

}

void STUnfoldingTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STUnfoldingTask::SetCutConditions(int min_clusters, double max_dpoca)
{ fMinClusters = min_clusters; fMaxDPoca = max_dpoca; }


