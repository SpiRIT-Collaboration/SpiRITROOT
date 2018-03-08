#include <iostream>
#include <fstream>
#include <random>
Double_t GenerateGauss(Double_t mu, Double_t sigma);
Double_t GenerateUniform(Double_t min, Double_t max);
  
void mc_generator_PID
(
 Int_t fNoEvents = 5000,
 // protons
 Double_t fMomCutMin = 0.,
 Double_t fMomCutMax = 2.5,
 // deuterium/triton
 // Double_t fMomCutMin = 0.1,
 // Double_t fMomCutMax = 1.1, 
 Double_t fMomStep = 2.,
 //
 // Double_t fMomCutMin = 0.025,
 // Double_t fMomCutMax = 0.5, 
 // Double_t fMomStep = 0.475,
 Int_t fNoProton = 1,
 Int_t fNoDeuterium = 0,
 Int_t fNoTriton = 0,
 Int_t fNo3Helium = 0, 
 Int_t fNo4Helium = 0, 
 Int_t fNoPiMinus = 0,
 Int_t fNoPiPlus = 0
 )
{

  ofstream outFile;
  TString fOutput;
  Int_t step = (fMomCutMax - fMomCutMin) / fMomStep;
  for (Int_t i=0; i<step; i++){
    //    fOutput.Form("../test_%.6g-%.6g.egen",fMomCutMin+i*fMomStep,fMomCutMin+(i+1)*fMomStep);
    //    fOutput.Form("../test_d_%.6g-%.6g.egen",fMomCutMin+i*fMomStep,fMomCutMin+(i+1)*fMomStep);
    //    fOutput.Form("../pion_t_%.6g-%.6g.egen",fMomCutMin+i*fMomStep,fMomCutMin+(i+1)*fMomStep);
    //    fOutput.Form("../two_proton.egen"); //,fMomCutMin+i*fMomStep,fMomCutMin+(i+1)*fMomStep);
    fOutput.Form("one_proton_600_15th_45ph.egen"); //,fMomCutMin+i*fMomStep,fMomCutMin+(i+1)*fMomStep);
    cout << fOutput << endl;
  
    outFile.open(fOutput);
    outFile << fNoEvents << endl;
    Int_t fNTracks = fNoProton+fNoDeuterium+fNoTriton+fNo3Helium+fNo4Helium+fNoPiMinus+fNoPiPlus;
  
    Double_t px,py,pz,ptot;
    Int_t counter = 0;
    
    while (counter < fNoEvents){
      
      outFile << counter << " " << fNTracks << " " << 99 << endl; // 99 is for no impact parameter information
      counter++;
      // protons
      for (Int_t j = 0; j < fNoProton; j++){
	if (j==0){
    // theta 15, phi 45
	  px=0.109808;
    py=0.109808;
    pz=0.579555;
    // 15 deg
//    py=0.241996;
//    pz=0.903141;
    // 30 deg
//	  py=0.467500;
//	  pz=0.809734; //GenerateGauss(0.805,0.05);
	} else {
	  px=0.2;
	  py=0;
	  pz=GenerateGauss(1.2,0.05);
	}
	// px, py, pz
	/*
	px = GenerateUniform(-0.1,0.1);
	py = GenerateUniform(-0.1,0.1);
	pz = GenerateUniform(fMomCutMin,fMomCutMax);
	ptot = sqrt(px*px+py*py+pz*pz);
	*/
	//	if ((ptot > fMomCutMin+i*fMomStep) && (ptot < fMomCutMin+(i+1)*fMomStep)){
	outFile << 2212 << " " << px << " " << py << " " << pz << endl;
	//	counter++;
	//	}
      }
      // deuterium 
      /*
      for (Int_t j = 0; j < fNoDeuterium; j++){
	// px, py, pz
	px = GenerateUniform(-0.1,0.1);
	py = GenerateUniform(-0.1,0.1);
	pz = GenerateUniform(fMomCutMin,fMomCutMax);
	ptot = sqrt(px*px+py*py+pz*pz);
	if ((ptot > fMomCutMin+i*fMomStep) && (ptot < fMomCutMin+(i+1)*fMomStep)){
	  outFile << counter << " " << fNTracks << endl;
	  outFile << 1000010020 << " " << px << " " << py << " " << pz << endl;
	  counter++;
	}
      }
      */
      // triton 
      /*
      for (Int_t j = 0; j < fNoTriton; j++){
	// px, py, pz
	px = GenerateUniform(-0.1,0.1);
	py = GenerateUniform(-0.1,0.1);
	pz = GenerateUniform(fMomCutMin,fMomCutMax);
	ptot = sqrt(px*px+py*py+pz*pz);
	if ((ptot > fMomCutMin+i*fMomStep) && (ptot < fMomCutMin+(i+1)*fMomStep)){
	  outFile << counter << " " << fNTracks << endl;
	  outFile << 1000010030 << " " << px << " " << py << " " << pz << endl;
	  counter++;
	}
      }
      */
      /*
      // triton // not correct values for generator
      for (Int_t j = 0; j < fNoTriton; j++)
      // px, py, pz
      outFile << 1000010030 << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateGauss(0.64,0.22) << endl;

      // pi+
      */
      /*
      for (Int_t j = 0; j < fNoPiMinus; j++){
	// px, py, pz
	px = GenerateUniform(-0.05,0.05);
        py = GenerateUniform(-0.05,0.05);
	pz = GenerateUniform(fMomCutMin,fMomCutMax);
	ptot = sqrt(px*px+py*py+pz*pz);
	if ((ptot > fMomCutMin+i*fMomStep) && (ptot < fMomCutMin+(i+1)*fMomStep)){
	  outFile << counter << " " << fNTracks << endl;
	  outFile << -211 << " " << px << " " << py << " " << pz << endl;
	  counter++;
	}
      }
      */
      /*
      // 3He // not correct values for generator
      for (Int_t j = 0; j < fNo3Helium; j++)
      // px, py, pz
      outFile << 1000020030 << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateGauss(0.64,0.22) << endl;
      // 4He // not correct values for generator
      for (Int_t j = 0; j < fNo4Helium; j++)
      // px, py, pz
      outFile << 1000020040 << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateUniform(-0.05,0.05) << " " <<
      GenerateGauss(0.64,0.22) << endl;
      // pi-
      for (Int_t j = 0; j < fNoPiMinus; j++)
      // px, py, pz
      outFile << -211 << " " <<
      GenerateGauss(0.0,0.084) << " " <<
      GenerateGauss(0.0,0.087) << " " <<
      GenerateGauss(0.077,0.098) << endl;
      */
    }
    
    outFile.close();
  }
  
}

Double_t GenerateGauss(Double_t mu, Double_t sigma){

  std::random_device rd;
  std::mt19937 e2(rd());
  std::normal_distribution<> dist(mu, sigma);
  
  return dist(e2);

}
Double_t GenerateUniform(Double_t min, Double_t max){

  if (min > max){
    Int_t temp = max;
    max = min;
    min = temp;
  }
  
  std::random_device rd;
  std::mt19937 e2(rd());
  std::uniform_real_distribution<> dist(min, max);
  
  return dist(e2);

}
