void makeUrQMDGenFile()
{
  ofstream genfile("UrQMD_300AMeV_short.egen");
  
  const Int_t maxz=2;
  const Int_t maxa=4;
  Int_t ztemp;
  Int_t atemp;
  Double_t masspi[2] = {134.976, 139.570};
  Double_t mass[maxz+1][maxa+1];
  Double_t amu2mev = 931.4941;

  ifstream masses("etable.dat");

  for(Int_t line=0; line<92; line++) {
    masses >> ztemp >> atemp;
    if(ztemp<=maxz && atemp<=maxa) 
      masses >> mass[ztemp][atemp];
  }

  for(Int_t z=0; z<=maxz; z++) {
    for(Int_t a=0; a<=maxa; a++) {
      if(mass[z][a]!=0.0) mass[z][a] = (a*1.0+mass[z][a]*1.0E-6)*amu2mev;
      else                mass[z][a] =  a*1.0*amu2mev;
    }
  }

  Int_t beamA;
  Int_t beamZ;
  Int_t targA;
  Int_t targZ;
  Int_t beamEperA;
  const Int_t events;

  ifstream urqmd("132sn124sn300amev_urqmd_short.dat");
  urqmd >> beamA >> beamZ >> targA >> targZ >> beamEperA >> events;

  Double_t beamE  = 1.0*beamEperA*beamA+beamA*amu2mev;
  Double_t totalE = beamE+targA*amu2mev;
  Double_t beamP  = TMath::Sqrt(beamE*beamE-beamA*beamA*amu2mev*amu2mev);
  Double_t mBeta  = -beamP/totalE;

  TVector3 beta(0.,0.,-mBeta);
  TLorentzVector mLorentz;  

  Int_t    evtnum;
  Int_t    evtmult;
  Int_t    urA;
  Int_t    urZ;
  Int_t    pdg;
  Double_t px;
  Double_t py;
  Double_t pz;
  Double_t tempmass;
  Double_t totalC;

  Int_t nTracks;
  vector<Double_t> pxl;
  vector<Double_t> pyl;
  vector<Double_t> pzl;
  vector<Int_t>    pdgl;

  genfile<<10<<endl;
  for(Int_t i=0; i<10; i++){
    urqmd >> evtnum >> evtmult;

    nTracks=0;
    pxl.clear();
    pyl.clear();
    pzl.clear();
    pdgl.clear();
    for(Int_t j=1; j<=evtmult; j++){
      urqmd >> urA >> urZ >> px >> py >> pz;

      if(urA<=maxa && urZ<=maxz) 
      {
        if(urA==-1)
        {
          if(urZ==0) tempmass=masspi[0];
          else       tempmass=masspi[1];
        }
        else tempmass=mass[urZ][urA];

        totalC = TMath::Sqrt(px*px+py*py+pz*pz+tempmass*tempmass);
        mLorentz.SetPxPyPzE(px,py,pz,totalC);
        mLorentz.Boost(beta);
        pxl.push_back(mLorentz.Px()/1000.);
        pyl.push_back(mLorentz.Py()/1000.);
        pzl.push_back(mLorentz.Pz()/1000.);

        //Setting pdg
        if (urA==-1 && urZ==1)  pdg=211;
        if (urA==-1 && urZ==0)  pdg=111;
        if (urA==-1 && urZ==-1) pdg=-211;
        if (urA==1  && urZ==0)  pdg=2112;
        if (urA==1  && urZ==1)  pdg=2212;
        if (urA==2  && urZ==1)  pdg=1000010020;
        if (urA==3  && urZ==1)  pdg=1000010030;
        if (urA==3  && urZ==2)  pdg=1000020030;
        if (urA==4  && urZ==2)  pdg=1000020040;
        pdgl.push_back(pdg);

        nTracks++;
      }
    }
    genfile<<i<<" "<<nTracks<<endl;
    for(Int_t iTrack=0; iTrack<nTracks; iTrack++)
      genfile<<pdgl[iTrack]<<" "<<pxl[iTrack]<<" "<<pyl[iTrack]<<" "<<pzl[iTrack]<<endl;

  }
  genfile.close();
}
