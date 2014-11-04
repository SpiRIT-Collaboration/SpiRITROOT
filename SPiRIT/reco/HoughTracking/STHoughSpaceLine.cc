#include "STHoughSpaceLine.hh"



ClassImp(STHoughSpaceLine);

STHoughSpaceLine::STHoughSpaceLine(){
  // Default constructor
 Initialize();
}



STHoughSpaceLine::STHoughSpaceLine(STEvent *event){
 
     if (!event->IsClustered()){}

}


STHoughSpaceLine::~STHoughSpaceLine(){
}


void STHoughSpaceLine::Initialize(){
  // Default constructor
  fGUIMode = kFALSE;
  fDebugMode = 0;
}

void STHoughSpaceLine::SetGUIMode(){

  std::cout<<" ****** GUI Mode to Visualize Hough Space and Tracks ******* "<<std::endl;

  fGUIMode = kTRUE;
  HistHough = new TH2F("HistHough","HistHough",1000,0,4,1000,-400,400);
  ClusterProjXZ = new TH2F("ClusterProjXZ","ClusterProjXZ",100,-400,400,100,-400,400);
  HC = new TCanvas("HC","HC",200,10,1000,1000);
  HC->Divide(1,2);
  HC->Draw();

}

void STHoughSpaceLine::SetDebugMode(Int_t verbosity){

  std::cout<<" ****** Debug Mode ******* "<<std::endl;

  fDebugMode = verbosity;
  

}


void STHoughSpaceLine::GetClusterSpace(STEvent *event){

       if(fGUIMode) ResetHoughSpace();

       fClustersNum = event -> GetNumClusters();
         for (Int_t iCluster = 0; iCluster <fClustersNum; iCluster++){ 
                      SingleCluster = event -> GetCluster(iCluster);
                      fClusterPos= SingleCluster -> GetPosition();
                      //fClusterPos.Print();
                      SetXZSingle(fClusterPos.X(),fClusterPos.Z());
		      
	}
        if(fGUIMode){
		 DrawHoughSpace();
                 Int_t maxbinx;
                 Int_t maxbiny;
                 Int_t maxbinz;  
                 HistHough->GetMaximumBin(maxbinx,maxbiny,maxbinz);
                 std::cout<<" maxbinx : "<<maxbinx<<"   maxbiny  : "<<maxbiny<<"  maxbinz : "<<maxbinz<<std::endl;
  	         Double_t x_coord = HistHough->GetXaxis()->GetBinCenter(maxbinx);
                 Double_t y_coord = HistHough->GetYaxis()->GetBinCenter(maxbiny);
		 std::cout<<"  Maximum of Hough Space - Distance of approach : "<<std::dec<<y_coord<<"   Angle : "<<std::dec<<x_coord<<std::endl;
	}


}

void STHoughSpaceLine::SetXZSingle(Float_t x,Float_t z){

                     

                   // for(Int_t itheta = -900; itheta < 900; itheta++){
		     for(Int_t itheta = 0; itheta <1023; itheta++){
                                 
                              //double startTime = getRealTime();
                              //fDistO.push_back(TMath::Cos(itheta)*x  +  TMath::Sin(itheta)*y); //Create a HoughSpaceContainer!!
                             // fTheta.push_back(itheta); // Test
                             // Float_t angle = (  (itheta*TMath::Pi()) /(1800.0)  );
			      Float_t angle = TMath::Pi()*(static_cast<Float_t>(itheta)/1023);
  			      Float_t d0 = (TMath::Cos(angle)*x)  +  (TMath::Sin(angle)*z);
                              
			      if(fGUIMode){	
					HistHough->Fill(angle,d0);
                              		ClusterProjXZ->Fill(x,z);
                                        
			        }

				   
			
			
			        else {fMapKey = EncodeMapKey(d0,angle);
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
       HistHough->Draw("zcol");
       
       HC->Modified();
       HC->Update();

}

void STHoughSpaceLine::ResetHoughSpace(){
       
       HistHough->Reset();
       ClusterProjXZ->Reset();
      

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
     if(fDebugMode > 1){
     std::cout<<" Distance of approach : "<<d<<" Angle : "<<ang<<std::endl;
     std::cout<<" Distance of approach (dec) : "<<std::dec<<d_enc<<std::endl;
     std::cout<<" Angle (dec) : "<<std::dec<<ang_enc<<std::endl;
     std::cout<<" Distance of approach (hex) : "<<std::hex<<d_enc<<std::endl;
     std::cout<<" Angle (hex) : "<<std::hex<<ang_enc<<std::endl;
     std::cout<<" HMapKey (hex) : "<<std::hex<<HMapKey<<std::endl;
     }

     return HMapKey;

}















