#define bGREEN  "\033[92m"
#define bYELLOW "\033[93m"
#define bRED    "\033[91m"
#define bENDC   "\033[0m"

void spirit_configuration() {
  cout << endl;
  cout << "  ====================================" << endl;
  cout << "        SpiRITROOT Configuration      " << endl;
  cout << "  ====================================" << endl;
  cout << endl; 

  // ------------------------------------------------
  //  FairSoft environment settings and version
  // ------------------------------------------------
  cout <<  " - Checking FairSoft       ";
  if (TString(gSystem -> Getenv("SIMPATH")).EqualTo("")) {
    cout << bRED << "   [ERROR]" << bENDC << " FairSoft not found!" << endl;
    cout << "           Please set environments for FairSoft." << endl;
    cout << "           See https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages for detail." << endl;

    return;
  }

  TString FairSoft_version = gSystem -> GetFromPipe("fairsoft-config --version");
  FairSoft_version.ReplaceAll("\n", "");
  cout << "your FairSoft version is " << bGREEN << FairSoft_version << bENDC << endl;


  // ------------------------------------------------
  //  FairROOT environment settings and version
  // ------------------------------------------------
  cout << " - Checking FairROOT       ";
  if (TString(gSystem -> Getenv("FAIRROOTPATH")).EqualTo("")) {
    cout << bRED << "  [ERROR]" << bENDC << " FairROOT not found!" << endl;
    cout << "          Please set environments for FairROOT." << endl;
    cout << "          See \"https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages\" for detail." << endl;

    return;
  }

  TString FairROOT_version = gSystem -> GetFromPipe("$FAIRROOTPATH/bin/fairroot-config --version");
  FairROOT_version.ReplaceAll("\n", "");
  cout << "your FairROOT version is " << bGREEN << FairROOT_version << bENDC << endl;

  // ------------------------------------------------
  //  SpiRITROOT environment setting and version.
  // ------------------------------------------------
  cout << " - Checking SpiRITROOT     ";
  if (TString(gSystem -> Getenv("VMCWORKDIR")).EqualTo("")) {
    cout << bRED << "  [ERROR]" << bENDC << " SpiRITROOT not found!" << endl;
    cout << "              Please build SpiRITROOT." << endl;
    cout << "              See \"https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/How-to-install-packages\" for detail." << endl;

    return;
  }

  cout << "your SpiRITROOT version is " << bGREEN << "-" << bENDC << endl; //#SpiRITROOT_version

  // ------------------------------------------------
  //  Define functions for reading parameters
  // ------------------------------------------------
  TString dirWork = gSystem -> Getenv("VMCWORKDIR");

  // ------------------------------------------------
  //  Mapping Files
  // ------------------------------------------------
  cout << endl;
  Bool_t exist_file = kFALSE;
  while (!exist_file) {
    cout << "  ";
    cout << bYELLOW << "[Q]" << bENDC;
    cout << " Please enter name of the parameter file without the path. ex) ST.parameters.par" << endl;
    cout << "      The file should be inside the SpiRITROOT/parameters directory." << endl;
    cout << "      For ST.parameter.par, just press [Enter] : ";
    TString name_parfile_full;
    REENTER:
    name_parfile_full = cin.get();
    name_parfile_full.ReplaceAll("\n", "");
    if (name_parfile_full.EqualTo("")) {
      exist_file = kTRUE;
      name_parfile_full = dirWork + "/parameters/ST.parameters.par";
    } else {
      TString path = dirWork + "/parameters/";
      exist_file = !TString(gSystem -> Which(path, name_parfile_full)).EqualTo("");

      if (!exist_file) {
        cout << "  " << bRED << "    [ERROR]" << bENDC << " File does not exist! Please enter again: ";

        cin.clear();
        cin.ignore(1000,'\n');
        goto REENTER;
      }

      name_parfile_full = dirWork + "/parameters/" + name_parfile_full;
    }

    STParReader parReader;
    streambuf *old = cout.rdbuf(0);
    parReader.SetParFile(name_parfile_full);
    cout.rdbuf(old);
    TString UA_mapfile = parReader.GetFilePar(parReader.GetIntPar("UAMapFile"));
    TString AGET_mapfile = parReader.GetFilePar(parReader.GetIntPar("AGETMapFile"));

    cout << endl;
    cout << " - Checking Map            AGET.map     -> " << bGREEN << UA_mapfile << bENDC << endl;
    cout << "                           UnitAsAd.map -> " << bGREEN << AGET_mapfile << bENDC << endl;
  }

  // ------------------------------------------------
  //  Geometry
  // ------------------------------------------------
  cout << " - Checking Geometry       ";
  TString dirGeom = dirWork + "/geometry/";
  TString pathGeomTop = "geomSpiRIT.root";
  TString pathGeomMgr = "geomSpiRIT.man.root";
  TString pathGeomMcr = "geomSpiRIT.C";
  Bool_t geomFlag;
  if (TString(gSystem -> Which(dirGeom, pathGeomTop)).EqualTo("") || TString(gSystem -> Which(dirGeom, pathGeomMgr)).EqualTo(""))
    geomFlag = kFALSE;
  else
    geomFlag = kTRUE;

  if (!geomFlag) {
    cout << endl;
    cout << bYELLOW << "   [INFO]" << bENDC << " Geometry not found!" << endl;
    cout << "            Creating geometry file... running macro " << pathGeomMcr << endl;
    gSystem -> Exec(Form("root -q -l %s", (dirGeom + pathGeomMcr).Data()));
    cout << endl;
  } else {
    cout << "Top     : " << bGREEN << pathGeomTop << bENDC << endl;
    cout << "                           Manager : " << bGREEN << pathGeomMgr << bENDC << endl;
  }

  // ------------------------------------------------
  //  Directories
  // ------------------------------------------------
  cout << " - Checking Directories    ";
  if ((gSystem -> AccessPathName(TString(gSystem -> Getenv("PWD")) + "/data", kWritePermission))) {
    cout << endl;
    cout << bYELLOW << "   [INFO]" << bENDC << " data directory not found!";
    cout << " Creating data directory..." << endl;
    gSystem -> Exec("mkdir data");
  } else
    cout << bGREEN << "data" << bENDC << endl;

  cout << endl;

  // ------------------------------------------------
  //  Cosmic reconstruction test
  // ------------------------------------------------
  cout << " - Cosmics Reconstruction Test";
  cout << "  ";
  cout << bYELLOW << "[Q]" << bENDC;
  cout << " Run cosmics reconstruction test? (~ 30 sec.) (y/n): ";

  TString kbInput = cin.get();
  kbInput.ToLower();
  if (kbInput.EqualTo("y")) {
    TString nameCosmicShort = "run_0457_event4n10.dat";
    TString pathCosmicShort = dirWork + "/input/";
    TString urlCosmicShort = "http://ribf.riken.jp/~genie/box/run_0457_event4n10.dat";
    Bool_t existCosmicShort = !(TString(gSystem -> Which(pathCosmicShort, nameCosmicShort)).EqualTo(""));

    if (!existCosmicShort)
      cout << bYELLOW << "   [INFO] " << bENDC << nameCosmicShort << " not found in input directory!" << endl;
    else {
      ifstream file(pathCosmicShort + nameCosmicShort, std::ios::end);
      Int_t file_size = file.tellg(); //("../input/run_0457_event4n10.dat"))
      if (file_size == 0) {
        cout << bYELLOW << "   [INFO] " << bENDC << nameCosmicShort << " is empty!" << endl;
        existCosmicShort = kFALSE;
      }
      file.close();
    }

    if (!existCosmicShort) {
      cout << "          Downloading " << nameCosmicShort << " Bytes) from " << urlCosmicShort << endl;
      cout << endl;
      gSystem -> Exec("curl -OL http://ribf.riken.jp/~genie/box/run_0457_event4n10.dat");
      gSystem -> Exec("mv run_0457_event4n10.dat " + pathCosmicShort);
      cout << endl;
      cout << "          Complete." << endl;
    }
    TString sim_name = "\"cosmic_short\"";
    TString event_name = Form("\"%s\"", pathCosmicShort.Data());
    TString command_reco = "source " + dirWork + "/build/config.sh; root -l -q -b 'run_reco.C(" + sim_name + "," + event_name + ")'";

    TStopwatch timer;
    timer.Start();
    gSystem -> Exec(command_reco);
    timer.Stop();
    Double_t time_diff = timer.RealTime();

    cout << endl;
    cout << "   " << bGREEN << "[Summary]" << bENDC << " Cosmics reconstruction test" << endl;
    cout << "   - Number of events : 2" << endl;
    cout << Form("   - Reconstruction time : %.2f seconds  (%.2f seconds per event)", time_diff, time_diff/2.) << endl;
  }

  cout << endl;

  // ------------------------------------------------
  //  MC simulation test
  // ------------------------------------------------
  cout << " - MC Reconstruction Test";
  cout << "  ";
  cout << bYELLOW << "[Q]" << bENDC;
  cout << " Run MC reconstruction test? (~ 3 min.) (y/n): ";

  cin.clear();
  cin.ignore(1000,'\n');

  kbInput = cin.get();
  kbInput.ToLower();
  if (kbInput.EqualTo("y")) {
    TString sim_name = "\"urqmd_short\"";
    TString gen_name = "\"UrQMD_300AMeV_short.egen\"";
    TString use_map  = "kFALSE";

    TString command_mc   = "source " + dirWork + "/build/config.sh; root -q -l 'run_mc.C("   + sim_name + ',' + gen_name + ',' + use_map + ")'";
    TString command_digi = "source " + dirWork + "/build/config.sh; root -q -l 'run_digi.C(" + sim_name + ")'";
    TString command_reco = "source " + dirWork + "/build6/config.sh; root -q -l 'run_reco.C(" + sim_name + ")'";

    TStopwatch timer;
    timer.Start();
    gSystem -> Exec(command_mc);
    timer.Stop();
    Double_t time_diff_mc = timer.RealTime();

    timer.Reset();
    timer.Start();
    gSystem -> Exec(command_digi);
    timer.Stop();
    Double_t time_diff_digi = timer.RealTime();

    timer.Reset();
    timer.Start();
    gSystem -> Exec(command_reco);
    timer.Stop();
    Double_t time_diff_reco = timer.RealTime();

    cout << endl; 
    cout << "   " << bGREEN << "[Summary]" << bENDC << " MC reconstruction test" << endl;
    cout << "   - Number of events : 10" << endl;
    cout << Form("   - Geant4 simulation time : %.2f seconds  (%.2f seconds per event)", time_diff_mc, time_diff_digi) << endl;
    cout << Form("   - Digitization      time : %.2f seconds  (%.2f seconds per event)", time_diff_digi, time_diff_digi) << endl;
    cout << Form("   - Reconstruction    time : %.2f seconds  (%.2f seconds per event)", time_diff_reco, time_diff_digi) << endl;
  }

  cout << endl;
  cout << "  Ending configuration." << endl;
  cout << endl;

  exit(0);
}
