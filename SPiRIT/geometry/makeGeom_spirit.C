void makeGeom_spirit()
{
  //--- Definition of a simple geometry


   gSystem->Load("libGeom");
  
   TGeoManager *tpc = new TGeoManager("SPiRIT", "SPiRIT");


   unsigned int medInd(0);

// some parameters which could be used later but not necessary for our tpc

   Double_t mPar[10];
   //TAG sollte wieder 0 werden sens flag
   mPar[0]=0.;//sensitive volume flag
   mPar[1]=1.;//magnetic field flag
   mPar[2]=30.;//max fiel in kGauss
   mPar[3]=0.1;//maximal angular dev. due to field
   mPar[4]=0.01;//max step allowed (in cm)
   mPar[5]=1.e-5;//max fractional energy loss
   mPar[6]=1.e-3;//boundary crossing accuracy
   mPar[7]=1.e-5;//minimum step
   mPar[8]=0.;//not defined
   mPar[9]=0.;//not defined


// Materials and Gases used in the SPiRIT TPC

   TGeoMaterial *_siliconMat = new TGeoMaterial("siliconMat",28.0855,14.,2.33);
   _siliconMat->SetRadLen(1.);//calc automatically, need this for elemental mats.
   TGeoMedium *_silicon = new TGeoMedium("silicon",medInd++,_siliconMat,mPar);

   TGeoMaterial *_copperMat = new TGeoMaterial("copperMat",63.54,29,8.96);
   _copperMat->SetRadLen(1.);//calc automatically, need this for elemental mats.
   TGeoMedium *_copper = new TGeoMedium("copper",medInd++,_copperMat,mPar);

   TGeoMaterial *_aluminumMat = new TGeoMaterial("aluminumMat",26.98,13,2.7);
   _aluminumMat->SetRadLen(1.);//calc automatically, need this for elemental mats.
   TGeoMedium *_aluminum = new TGeoMedium("aluminum",medInd++,_aluminumMat,mPar);


   TGeoMixture *_airMat = new TGeoMixture("airMat",3);
   _airMat->AddElement(14.01,7.,.755);
   _airMat->AddElement(16.00,8.,.231);
   _airMat->AddElement(39.95,18.,.014);
   _airMat->SetDensity(1.205e-3);
   TGeoMedium *_air = new TGeoMedium("air",medInd++,_airMat,mPar);

   TGeoMixture *_P10Mat = new TGeoMixture("P10Mat",3);
   _P10Mat->AddElement(39.95,18.,.957);
   _P10Mat->AddElement(12.011,6.,.032);
   _P10Mat->AddElement(1.008,1.,.011);
   _P10Mat->SetDensity(1.59e-3);

   TGeoMedium *_P10 = new TGeoMedium("p10",medInd++,_P10Mat,mPar);
   

    TGeoMixture *_vacuumMat = new TGeoMixture("vacuumMat",3);
   _vacuumMat->AddElement(14.01,7.,.78);
   _vacuumMat->AddElement(16.00,8.,.21);
   _vacuumMat->AddElement(39.95,18.,.01);
   _vacuumMat->SetDensity(1.2e-15);
   TGeoMedium *_vacuum = new TGeoMedium("vacuum",medInd++,_vacuumMat,mPar);


   TGeoMedium *vacuum = gGeoManager->GetMedium("vacuum");
   assert(vacuum!=NULL);
   TGeoMedium *air = gGeoManager->GetMedium("air");
   assert(air!=NULL);
   TGeoMedium *P10 = gGeoManager->GetMedium("p10");
   assert(P10!=NULL);
   TGeoMedium *sil = gGeoManager->GetMedium("silicon");
   assert(sil!=NULL);
   TGeoMedium *copp = gGeoManager->GetMedium("copper");
   assert(copp!=NULL);
   TGeoMedium *alum = gGeoManager->GetMedium("aluminum");
   assert(alum!=NULL);

// World volume or the Master/Mother volume used in the SPiRIT TPC, I am setting it to be filled with P10 gas could be changed to Vacuum or air by just changing the parameter

   TGeoVolume *top = gGeoManager->MakeBox("TOPPER", P10, 1000., 1000., 1000.);
   gGeoManager->SetTopVolume(top); // mandatory !

   Double_t pad_x=86.40;
   Double_t drift_y=51.29;
   Double_t beam_z=134.40;
   
// Field cage made up of copper, prepared from the composite structure, origin of the detector volumes starts from the upstream center of pad plane.

   TGeoVolume *field_cage_out = gGeoManager->MakeBox("field_cage_out", copp, (pad_x+10.30)/2, (drift_y+0.18)/2, (beam_z+5)/2);
   field_cage_out->SetLineColor(kBlue);
//     top->AddNode(field_cage_out, 1, gGeoIdentity);

 
   TGeoVolume *field_cage_in = gGeoManager->MakeBox("field_cage_in", copp, pad_x/2, drift_y/2, beam_z/2);
   field_cage_in->SetLineColor(kRed);
//     top->AddNode(field_cage_in, 1, gGeoIdentity);

// Some translation and rotation definitions to make the geometry by translating and rotatimg

   TGeoTranslation *t1 = new TGeoTranslation("t1",0,0,+2.6);
   t1->RegisterYourself();
 
   TGeoRotation *r1 = new TGeoRotation("r1",90,0,0,180,90,90);
   r1->RegisterYourself();
 
   TGeoTranslation *t2 = new TGeoTranslation("t2",0,0,5);
   t2->RegisterYourself();        

   TGeoCompositeShape *cs1 = new TGeoCompositeShape("cs1","(field_cage_out)-(field_cage_in)");     
//   TGeoCompositeShape *cs1 = new TGeoCompositeShape("cs1","(field_cage_out)-(field_cage_in:t1)");     
   TGeoVolume *comp1 = new TGeoVolume("comp1",cs1,copp);
   TGeoVolume *comp1 = new TGeoVolume("comp1",cs1,copp);
   comp1->SetLineColor(kBlue);
   top->AddNode(comp1, 1, new TGeoTranslation(0,-drift_y/2,(beam_z+5)/2));       
//	top->Raytrace();


// Enclosure  made up of Aluminum but not used, prepared from the composite structure

   TGeoVolume *enclosure_out = gGeoManager->MakeBox("enclosure_out", alum, (pad_x+20)/2, (drift_y+10)/2, (beam_z+30)/2);
   enclosure_out->SetLineColor(kBlue);
// top->AddNode(enclosure_out, 1, gGeoIdentity);

   TGeoVolume *enclosure_in = gGeoManager->MakeBox("enclosure_in", alum, (pad_x+18)/2, (drift_y+8)/2, (beam_z+28)/2);
   enclosure_in->SetLineColor(kRed);
//  top->AddNode(enclosure_in, 1, gGeoIdentity);

   TGeoCompositeShape *cs2 = new TGeoCompositeShape("cs2","(enclosure_out)-(enclosure_in:t1)");
   TGeoVolume *comp2 = new TGeoVolume("comp2",cs2,alum);
   comp2->SetLineColor(kRed);
// top->AddNode(comp2, 1, gGeoIdentity);
// top->Raytrace();

// Top primeter of the TPC , not used 

   TGeoVolume *top_plate = gGeoManager->MakeBox("top_plate", alum, (pad_x+30)/2, (8)/2, (beam_z+25)/2);
   top_plate->SetLineColor(kGreen);
//     top->AddNode(top_plate, 1, new TGeoTranslation(0,29.5,0));

// Pad plane made up of copper and 2 cm thick

   TGeoVolume *pad_plane = gGeoManager->MakeBox("pad_plane", copp, (pad_x)/2, 2/2, (beam_z+4)/2);
   pad_plane->SetLineColor(kRed);
   top->AddNode(pad_plane, 1, new TGeoTranslation(0,-(drift_y-49)/2,(beam_z+4)/2));

// Thin Ground wire plane made up of copper which is 8 mm thick

   TGeoVolume *wire_plane_ground = gGeoManager->MakeBox("wire_plane_ground", copp, (pad_x)/2, 0.8/2, (beam_z+4)/2);
   wire_plane_ground->SetLineColor(kBlack);
   top->AddNode(wire_plane_ground, 1, new TGeoTranslation(0,-(drift_y-45.5)/2,(beam_z+4)/2));


   //--- close the geometry
   gGeoManager->CloseGeometry();
   //--- draw the ROOT box
   gGeoManager->SetVisLevel(10);
  top->Draw("ogl");

   TFile *outfile_spirit = TFile::Open("Spirit_Geom.root","RECREATE");
   gGeoManager->Write();
   outfile_spirit->Close();
}
