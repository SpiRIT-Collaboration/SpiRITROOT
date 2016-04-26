////////////////////////////
//                        //
//   Configuration part   //
//                        //
////////////////////////////

// Put the parameter file name. Path is automatically concatenated.
TString fParameterFile = "ST.parameters.Commissioning_201604.par";

// Set use the meta data files
Bool_t fUseMetadata = kFALSE;

// Set the supplement path which contains data list and meta data
// Only works when fUseMetadata is kTRUE
TString supplementPath = "/data/Q16264/rawdataSupplement";

// Set use the gain calibration data file.
Bool_t fUseGainCalibration = kFALSE;

// FPN pedestal range selection threshold
Int_t fFPNThreshold = 5;

// Set the gating grid noise data. If left blank, it will use FPN pedestal.
TString fGGNoiseData = "";

//Set gg threshold for checking if noise worked
Int_t gatingg_thres=90;

//Number of events to analyze 
Int_t num_events=300;
  
//Set number of time buckets
Int_t numTbs =270;

//Set the expected number of Tbs the TPC volume represents
Int_t t_drifttime=220;

//Set the max ADC threshold 
Int_t adc_sat=3680;

//Beam location
Int_t tb_start=105;//tb start around beam locationp
Int_t tb_end=160;//tb end around beam location


//////////////////////////////////////////////////////////
//                                                      //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

//DONT CHANGE unless you change the bool condition in the check_ggsubtract() function
Int_t totalarea = 245;//total pads evaluated in check. Used for average prob calculation 

STCore *fCore = NULL;
STPlot *fPlot = NULL;

// histogram list

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"
#define cNORMAL "\033[0m"

void diagnoseTPC() {
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);

  TH1D *adcdist = new TH1D("adcdist","adc distribution",5000,-500,4500);    
  TH1D *driftdist = new TH1D("driftdist","raw drift time dist",270,0,270);
  TH2D *minadc = new TH2D("minadc","avg. minimum adc for each pad",112,0,112,108,0,108);
  TH2D *maxadc = new TH2D("maxadc","avg. maximum adc for each pad",112,0,112,108,0,108);
  TH2D *saturation_map = new TH2D("saturation","pads that saturated",112,0,112,108,0,108);

  if (!(gSystem -> Getenv("RUN"))) {
    cout << endl;
    cout << cYELLOW << "== Usage: " << cNORMAL << "RUN=" << cRED << "####" << cNORMAL << " root quickViewer.C" << endl;
    cout << "          There're some settings in the file. Open and check for yourself." << endl;
    cout << endl;
    gSystem -> Exit(0);
  }

  Int_t runNo = atoi(gSystem -> Getenv("RUN"));
  TString dataFile = "";
  TString metaFile = "";
  if (fUseMetadata) {
    dataFile = Form("%s/run_%04d/dataList.txt", supplementPath.Data(), runNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", supplementPath.Data(), runNo);
  } else {
    if (GETFileChecker::CheckFile(Form("list_run%04d.txt", runNo)))
      gSystem -> Exec(Form("./createList.sh %d", runNo));

    dataFile = Form("list_run%04d.txt", runNo);
  }

  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString parameterDir = workDir + "/parameters/";

  STParReader *fPar = new STParReader(parameterDir + fParameterFile);

  Bool_t fUseSeparatedData = kFALSE;
  if (dataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  if (!fUseSeparatedData) {
    fCore = new STCore(dataFile);
  } else {
    fCore = new STCore();
    fCore -> SetUseSeparatedData(fUseSeparatedData);

    TString dataFileWithPath = dataFile;
    std::ifstream listFile(dataFileWithPath.Data());
    TString buffer;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("s."))
        fCore -> AddData(dataFileWithPath, iCobo);
      else {
        iCobo++;
        fCore -> AddData(dataFileWithPath, iCobo);
      }
    }
  }

  if (fUseGainCalibration) {
    fCore -> SetGainCalibrationData(fPar -> GetFilePar(fPar -> GetIntPar("GainCalibrationDataFile")));
    fCore -> SetGainReference(fPar -> GetDoublePar("GCConstant"), fPar -> GetDoublePar("GCLinear"), fPar -> GetDoublePar("GCQuadratic"));
  }

  fCore -> SetUAMap(fPar -> GetFilePar(fPar -> GetIntPar("UAMapFile")));
  fCore -> SetAGETMap(fPar -> GetFilePar(fPar -> GetIntPar("AGETMapFile")));
  fCore -> SetFPNPedestal(fFPNThreshold);
  fCore -> SetData(0);

  if (!fGGNoiseData.IsNull()) {
    fCore -> SetGGNoiseData(fGGNoiseData);
    fCore -> InitGGNoiseSubtractor();
  }

  if (fUseMetadata) {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", supplementPath.Data(), runNo, dataFileWithPath.Data());
      fCore -> LoadMetaData(dataFileWithPath, iCobo);
    }
  }

  Int_t induced_thres=-25;
  //pads DONT CHANGE unless you change the bool condition below for gatinggarea1 and 2 in the code
  Int_t totalarea =245;
  
  Int_t count = 0; //event counter

  cout << endl << endl;
   while(STRawEvent *event = fCore->GetRawEvent()){
    
    if(count%100==0)
      cout << "Events analyzed: " << count << endl;
    if(count>num_events)
      break;

    std::vector<STPad> *padArray = event -> GetPads();
    
    for(Int_t iPad = 0; iPad < padArray -> size(); iPad++){
      STPad pad = padArray -> at(iPad);
      
      Int_t row = pad.GetRow();
      Int_t layer = pad.GetLayer();
      
      Int_t *rawadc = pad.GetRawADC(); // Raw signal
      Double_t *adc = pad.GetADC();    // Pedestal subtracted ADC using FPN channels

      Int_t min_adc=0;
      Int_t max_adc=0;
      Int_t maxadc_tb=0;
      
      for (Int_t iTb = 0; iTb < numTbs; iTb++){
	
	if(iTb>tb_start && iTb<tb_end){
	  if(min_adc>adc[iTb] && adc[iTb]<0)min_adc=adc[iTb];
	}
	
	if(max_adc<adc[iTb] && adc[iTb]>0){
	  max_adc=adc[iTb];
	  maxadc_tb=iTb;
	}
	
      }
      
      if(max_adc!=0)driftdist->Fill(maxadc_tb); // drift time distribution
      if(max_adc>=adc_sat)saturation_map->Fill(layer,row); // saturation map

      adcdist->Fill(max_adc);
      adcdist->Fill(min_adc);
      
      //Find the average min adc value for the events
      Double_t value = minadc->GetBinContent(layer,row);
      value += min_adc;
      minadc->SetBinContent(layer,row,value);
      value = 0;//re initizlize       

      value = maxadc->GetBinContent(layer,row);
      value += max_adc;
      maxadc->SetBinContent(layer,row,value);

    }//pad loop
    count++;
  }//event loop

  TH2D *prob_sat = (TH2D *)saturation_map->Clone("prob_sat");
  prob_sat->SetTitle("probability of a pad saturating");
  prob_sat->Scale(1./count);

  driftdist->Scale(1./driftdist->Integral());
  maxadc->Scale(1./count);
  minadc->Scale(1./count);

  TCanvas *c1 = new TCanvas("c1","summary",1200,1200);
  c1->Divide(2,3);

  c1->cd(1);
  prob_sat->Draw("colz");

  c1->cd(2);
  c1->cd(2)->SetLogy();
  TLine* l1 = new TLine(20,0,20,0.1);
  TLine* l2 = new TLine(250,0,250,0.1);
  driftdist->Draw();
  l1->SetLineWidth(2);
  l1->SetLineColor(2);
  l1->Draw("SAME");
  l2->SetLineWidth(2);
  l2->SetLineColor(2);
  l2->Draw("SAME");

  c1->cd(3);
  //  minadc->GetZaxis()->SetRangeUser(0,-200);
  minadc->Draw("colz");

  c1->cd(4);
  maxadc->Draw("colz");

  c1->cd(5);
  c1->cd(5)->SetLogy();
  adcdist->Draw();

}
