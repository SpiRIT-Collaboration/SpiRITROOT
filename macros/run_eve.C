void run_eve
(
  TString name = "run3000_s0", 
  TString pathToData = "/Users/ejungwoo/spiritroot/macros/data/",
  TString parname = "ST.parameters.Commissioning_201604.par",
  /*
   * - If dataList is "", deactivate single pad data,
   * - If dataList is set, activate single pad data (independent of reco file).
   *   XXX This may cause serious speed problem if meta data is not set.
   *   (depending on the system) if startEventID is not correct, pad may not match.
  */
  TString dataList = "list_run3000.txt",
    Int_t runNo = 3000,
    Int_t startEventID = 0,
   Bool_t useMeta = false,
  TString supplePath = "/data/Q16264/rawdataSupplement"
)
{
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (pathToData.IsNull())
    pathToData = spiritroot+"macros/data/";

  TString input     = pathToData + name + ".reco.root";
  TString output    = pathToData + name + ".eve.root";
  TString parameter = spiritroot + "parameters/"  + parname;
  TString geomety   = spiritroot + "geometry/geomSpiRIT.man.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  STEveManager *eve = new STEveManager();
  eve -> SetInputFile(input);         // Set input file (string)
  eve -> SetParInputFile(parameter);  // Set parameter file (string)
  eve -> SetOutputFile(output);       // Set output file (string)
  eve -> SetBackgroundColor(kWhite);  // Set background color (Color_t) 
  eve -> SetGeomFile(geomety);        // Set geometry file (string)
  eve -> SetVolumeTransparency(80);   // Set geometry transparency (integer, 0~100)
  eve -> SetViewerPoint(-0.7, 1.1);   // Set camera angle (theta, phi)

  STEveDrawTask *draw = new STEveDrawTask();
  draw -> SetRendering("mc",         false);
  draw -> SetRendering("digi",       false);
  draw -> SetRendering("hit",        true);
  draw -> SetRendering("hitbox",     false);
  draw -> SetRendering("helixhit",   true);
  draw -> SetRendering("helix",      true);
  draw -> SetRendering("cluster",    true);
  draw -> SetRendering("recotrack",  true);

  if (dataList.IsNull() == false) {
    if (useMeta)
      dataList = Form("%s/run_%04d/dataList.txt", supplePath.Data(), runNo);

    STDecoderTask *decoder = new STDecoderTask();
    decoder -> SetUseSeparatedData(true);
    decoder -> SetPersistence(false);
    decoder -> SetUseGainCalibration(false);
    decoder -> SetGGNoiseData("");
    decoder -> SetDataList(dataList);
    decoder -> SetEventID(startEventID);

    if (useMeta) {
      TString metaFile = Form("%s/run_%04d/metadataList.txt", supplePath.Data(), runNo);
      std::ifstream metalistFile(metaFile.Data());
      TString dataFileWithPath;
      for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
        dataFileWithPath.ReadLine(metalistFile);
        dataFileWithPath = Form("%s/run_%04d/%s", supplePath.Data(), runNo, dataFileWithPath.Data());
        decoder -> SetMetaData(dataFileWithPath, iCobo);
      }
    }
    eve -> AddTask(decoder);
  }

  eve -> AddEveTask(draw);
  eve -> Init();
}

