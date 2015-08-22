#include "STHoughSpaceLine.hh"



ClassImp(STHoughSpaceLine);

STHoughSpaceLine::STHoughSpaceLine(){
  // Default constructor
 Initialize();
}



STHoughSpaceLine::STHoughSpaceLine(STEvent *event){
 
     //if (event->IsClustered()) std::cout<<" SpiRITROOT : Clustered event "<<std::cout; 

}


STHoughSpaceLine::~STHoughSpaceLine(){
}


void STHoughSpaceLine::Initialize(){
  // Default constructor
  fStandardMode = kFALSE;
  fGUIMode = kFALSE;
  fDrawHist = kFALSE;
  fDebugMode = 0;
  fClusteredMode = kTRUE;
}

void STHoughSpaceLine::SetGUIMode(Int_t guimode){
 
  fDrawHist = guimode;
  std::cout<<" ****** GUI Mode to Visualize Hough Space and Tracks ******* "<<std::endl;

  fGUIMode = kTRUE;
  HistHoughXZ = new TH2F("HistHoughXZ","HistHoughXZ",1000,0,4,1000,-400,400);
  //ClusterProjXZ = new TH2F("ClusterProjXZ","ClusterProjXZ",1000,-300,300,1000,0,800);
  ClusterProjXZ = new TH2F("ClusterProjXZ","ClusterProjXZ",100,-400,400,100,0,800);
  HistHoughXY = new TH2F("HistHoughXY","HistHoughXY",1000,0,4,1000,-400,400);
  ClusterProjXY = new TH2F("ClusterProjXY","ClusterProjXY",100,-400,400,100,-0,800);
  if(fDrawHist){
       	  HC = new TCanvas("HC","HC",200,10,1000,750);
	  HC->Divide(2,1);
	  HC->Draw();
  }

}

void STHoughSpaceLine::SetDebugMode(Int_t verbosity){

  std::cout<<" ****** Debug Mode ******* "<<std::endl;

  fDebugMode = verbosity;
  

}

void STHoughSpaceLine::SetMode(TString mode){

      
        mode.ToLower();
	  if (mode.EqualTo("clustered")) {
		fClusteredMode = kTRUE;
		std::cout<<" Data mode set to Clustered "<<std::endl;
	  }  else if (mode.EqualTo("standard")){
		 fStandardMode = kTRUE;
                 fClusteredMode = kFALSE;
		std::cout<<" Data mode set to Standard "<<std::endl; 
 	  } else {
          std::cout<<" Wrong mode. Please set a mode (Default is Clustered). Exiting... "<<std::endl; 
          exit(1);
	  }

}


void STHoughSpaceLine::GetClusterSpace(STEvent *event){

      if(fGUIMode) ResetHoughSpace();

      if(fClusteredMode){

       fClustersNum = event -> GetNumClusters();
         for (Int_t iCluster = 0; iCluster <fClustersNum; iCluster++){ 
                      SingleCluster = event -> GetCluster(iCluster);
                      fClusterPos= SingleCluster -> GetPosition();
                      //fClusterPos.Print();
                      SetSingle(fClusterPos.X(),fClusterPos.Y(),fClusterPos.Z());
		      
	}

      }


      if(fGUIMode){
	         if(fDrawHist) DrawHoughSpace();
	         Int_t maxbinx;
                 Int_t maxbiny;
                 Int_t maxbinz;  
                 HistHoughXZ->GetMaximumBin(maxbinx,maxbiny,maxbinz);
                 std::cout<<" **********    Hough Space XZ   ************"<<std::endl;
                 std::cout<<" maxbinx : "<<maxbinx<<"   maxbiny  : "<<maxbiny<<"  maxbinz : "<<maxbinz<<std::endl;
  	         Double_t x_coord = HistHoughXZ->GetXaxis()->GetBinCenter(maxbinx);
                 Double_t y_coord = HistHoughXZ->GetYaxis()->GetBinCenter(maxbiny);
		 std::cout<<"  First Maximum of the Hough Space - Distance of approach : "<<std::dec<<y_coord<<"   Angle : "<<std::dec<<x_coord<<std::endl;
		 Int_t locmaxbinx = maxbinx;
                 Int_t locmaxbiny = maxbiny;
                 Int_t locmaxbinz = maxbinz;
                 GetNextMaximumBin(locmaxbinx,locmaxbiny,locmaxbinz,HistHoughXZ);
		 std::cout<<" ********************************************"<<std::endl;
		 std::cout<<" locmaxbinx : "<<locmaxbinx<<"   locmaxbiny  : "<<locmaxbiny<<"  locmaxbinz : "<<locmaxbinz<<std::endl;
  	         x_coord = HistHoughXZ->GetXaxis()->GetBinCenter(locmaxbinx);
                 y_coord = HistHoughXZ->GetYaxis()->GetBinCenter(locmaxbiny);
		 std::cout<<" Second Maximum of the Hough Space - Distance of approach : "<<std::dec<<y_coord<<"   Angle : "<<std::dec<<x_coord<<std::endl;

      }



}

void STHoughSpaceLine::SetSingle(Float_t x, Float_t y,Float_t z){

                     

                   // for(Int_t itheta = -900; itheta < 900; itheta++){
		     for(Int_t itheta = 0; itheta <1023; itheta++){
                                 
                              //double startTime = getRealTime();
                              //fDistO.push_back(TMath::Cos(itheta)*x  +  TMath::Sin(itheta)*y); //Create a HoughSpaceContainer!!
                             // fTheta.push_back(itheta); // Test
                             // Float_t angle = (  (itheta*TMath::Pi()) /(1800.0)  );
			      Float_t angle = TMath::Pi()*(static_cast<Float_t>(itheta)/1023);
  			      Float_t d0_XZ = (TMath::Cos(angle)*x)  +  (TMath::Sin(angle)*z);
                              Float_t d0_XY = (TMath::Cos(angle)*x)  +  (TMath::Sin(angle)*y);
                              
			      if(fGUIMode){	
					HistHoughXZ->Fill(angle,d0_XZ);
                              		ClusterProjXZ->Fill(z,x);
                                        HistHoughXY->Fill(angle,d0_XY);
                              		ClusterProjXY->Fill(y,x);
                                        
			        }

				   
			
			
			        else {fMapKey = EncodeMapKey(d0_XZ,angle);
                                //if(!HoughMap.count(fMapKey)) HoughMap.insert(std::pair<Int_t,Int_t>(fMapKey,1)); //Starts in 1 instead of 0
                                 HoughMap[fMapKey]++;
				}
                               

				//char c;
				//std::cin>>c;  

			}

		
	              // fHoughSpace_enc = std::max_element(HoughMap.begin(),HoughMap.end()); // For the moment we only return one pair of values. In other words, we only consider one track
		      // std::cout<<fHoughSpace_enc->first<<" => "<<fHoughSpace_enc->second <<std::endl;
		
		     

}


void STHoughSpaceLine::DrawHoughSpace(){
       
       HC->cd(1);
       ClusterProjXZ->Draw();
       HC->cd(2);
       HistHoughXZ->Draw("zcol");
       //  HC->cd(3);
       //ClusterProjXY->Draw();
       //HC->cd(4);
       //HistHoughXY->Draw("zcol");
       
       HC->Modified();
       HC->Update();

}

void STHoughSpaceLine::ResetHoughSpace(){
       
       HistHoughXZ->Reset();
       ClusterProjXZ->Reset();
       HistHoughXY->Reset();
       ClusterProjXY->Reset();
      

}



Int_t STHoughSpaceLine::EncodeMapKey(Float_t d, Float_t ang){
     
     Int_t HMapKey=0;
     
     Int_t d_enc = static_cast<Int_t>(d*10);
     Int_t ang_enc = static_cast<Int_t>(ang*1000); 
     
    
     if(d_enc<0.0 && (abs(d_enc)<4096) ){
       d_enc = abs(d_enc);
       HMapKey =( (d_enc&0x00007fff) + 0x8000);  
     }  
     else  HMapKey = (d_enc&0x00007fff);

     HMapKey = HMapKey + ((ang_enc&0x0000ffff)<<16);
     
     // Debugging info
     if(fDebugMode == 1){
     std::cout<<" Distance of approach : "<<d<<" Angle : "<<ang<<std::endl;
     std::cout<<" Distance of approach (dec) : "<<std::dec<<d_enc<<std::endl;
     std::cout<<" Angle (dec) : "<<std::dec<<ang_enc<<std::endl;
     std::cout<<" Distance of approach (hex) : "<<std::hex<<d_enc<<std::endl;
     std::cout<<" Angle (hex) : "<<std::hex<<ang_enc<<std::endl;
     std::cout<<" HMapKey (hex) : "<<std::hex<<HMapKey<<std::endl;
     }

     return HMapKey;

}

Int_t STHoughSpaceLine::GetNextMaximumBin(Int_t &locmax, Int_t &locmay, Int_t &locmaz, TH2F* HoughSpace){

       Double_t dist;

       Float_t loc_x_bin = locmax;
       Float_t loc_y_bin = locmay;

       Int_t bin, binx, biny, binz;
       Int_t locm;
       Int_t xfirst  = HoughSpace->GetXaxis()->GetFirst();
       Int_t xlast   = HoughSpace->GetXaxis()->GetLast();
       Int_t yfirst  = HoughSpace->GetYaxis()->GetFirst();
       Int_t ylast   = HoughSpace->GetYaxis()->GetLast();
       Int_t zfirst  = HoughSpace->GetZaxis()->GetFirst();
       Int_t zlast   = HoughSpace->GetXaxis()->GetLast();

	Double_t maximum = -FLT_MAX, value;
	locm = locmax = locmay = locmaz = 0;

           for (binz=zfirst;binz<=zlast;binz++) {
     		 for (biny=yfirst;biny<=ylast;biny++) {
        		 for (binx=xfirst;binx<=xlast;binx++) {
           			 bin = HoughSpace->GetBin(binx,biny,binz);
            			 value = HoughSpace->GetBinContent(bin);
                                 dist = TMath::Sqrt( pow(loc_x_bin-binx,2) + pow(loc_y_bin-biny,2) );
                                // std::cout<<" Dist : "<<dist<<std::endl;
                                // std::cout<<" Maximum Local coord X :" <<loc_x_bin<<"  Maximum Local coord Y :"<<loc_y_bin<<"  Bin x :"<<binx<<"  Bin y :"<<biny<<std::endl;
            		   if( dist > 60 ){	
			      if (value > maximum) {
				 maximum = value;
               			 locm    = bin;
              			 locmax  = binx;
               			 locmay  = biny;
               			 locmaz  = binz;
				}
			     }	
			 }
		 }
	    }
 
         return locm;


}















