#include "STHoughSpaceHelix.hh"



ClassImp(STHoughSpaceHelix);

STHoughSpaceHelix::STHoughSpaceHelix(){
  // Default constructor
     Initialize();
}

STHoughSpaceHelix::~STHoughSpaceHelix(){
}


void STHoughSpaceHelix::Initialize(){
  // Default constructor
  
  fGUIMode = kFALSE;
  fDebugMode = 0;
  
}

void STHoughSpaceHelix::SetGUIMode(){

  std::cout<<" ****** GUI Mode to Visualize Hough Space and Tracks ******* "<<std::endl;

  fGUIMode = kTRUE;
  HistHoughXZ = new TH2F("HistHoughXZ","HistHoughXZ",1000,0,4,1000,-400,400);
  HelixProjXZ = new TH2F("HelixProjXZ","HelixrProjXZ",100,-400,400,100,0,400);
  HC = new TCanvas("HC","HC",200,10,1000,1000);
  HC->Divide(2,2);
  HC->Draw();

}

void STHoughSpaceHelix::SetDebugMode(Int_t verbosity){

  std::cout<<" ****** Debug Mode ******* "<<std::endl;

  fDebugMode = verbosity;
  

}

void STHoughSpaceHelix::GetClusterSpace(STEvent *event){

      if(fGUIMode) ResetHoughSpace();

      

       fClustersNum = event -> GetNumClusters();
         for (Int_t iCluster = 0; iCluster <fClustersNum; iCluster++){ 
                      SingleCluster = event -> GetCluster(iCluster);
                      fClusterPos= SingleCluster -> GetPosition();
                      //fClusterPos.Print();
                      SetSingle(fClusterPos.X(),fClusterPos.Y(),fClusterPos.Z());
		      
	}

      


        if(fGUIMode){
		 DrawHoughSpace();
                 Int_t maxbinx;
                 Int_t maxbiny;
                 Int_t maxbinz;  
                 HistHoughXZ->GetMaximumBin(maxbinx,maxbiny,maxbinz);
                 std::cout<<" **********    Hough Space XZ   ************"<<std::endl;
                 std::cout<<" maxbinx : "<<maxbinx<<"   maxbiny  : "<<maxbiny<<"  maxbinz : "<<maxbinz<<std::endl;
  	         Double_t x_coord = HistHoughXZ->GetXaxis()->GetBinCenter(maxbinx);
                 Double_t y_coord = HistHoughXZ->GetYaxis()->GetBinCenter(maxbiny);
		 std::cout<<"  Maximum of Hough Space - Distance of approach : "<<std::dec<<y_coord<<"   Angle : "<<std::dec<<x_coord<<std::endl;
	}



}


void STHoughSpaceHelix::SetSingle(Float_t x, Float_t y,Float_t z){

                     

                   // for(Int_t itheta = -900; itheta < 900; itheta++){
		     for(Int_t itheta = 0; itheta <1023; itheta++){
                                 
                              //double startTime = getRealTime();
                              //fDistO.push_back(TMath::Cos(itheta)*x  +  TMath::Sin(itheta)*y); //Create a HoughSpaceContainer!!
                             // fTheta.push_back(itheta); // Test
                             // Float_t angle = (  (itheta*TMath::Pi()) /(1800.0)  );
			     // Float_t angle = TMath::Pi()*(static_cast<Float_t>(itheta)/1023);
  			     // Float_t d0_XZ = (TMath::Cos(angle)*x)  +  (TMath::Sin(angle)*z);
                            //  Float_t d0_YZ = (TMath::Cos(angle)*y)  +  (TMath::Sin(angle)*z);
                              
			/*      if(fGUIMode){	
					HistHoughXZ->Fill(angle,d0_XZ);
                              		HelixProjXZ->Fill(z,x);
                                       
                                        
			        }

				   
			
			
			        else {fMapKey = EncodeMapKey(d0_XZ,angle);
                                //if(!HoughMap.count(fMapKey)) HoughMap.insert(std::pair<Int_t,Int_t>(fMapKey,1)); //Starts in 1 instead of 0
                                 HoughMap[fMapKey]++;
				}  */
                               

				//char c;
				//std::cin>>c;  

			}

		
	              // fHoughSpace_enc = std::max_element(HoughMap.begin(),HoughMap.end()); // For the moment we only return one pair of values. In other words, we only consider one track
		      // std::cout<<fHoughSpace_enc->first<<" => "<<fHoughSpace_enc->second <<std::endl;
		
		     

}

Int_t STHoughSpaceHelix::EncodeMapKey(Float_t d, Float_t ang){
     
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

void STHoughSpaceHelix::ResetHoughSpace(){
       
       HistHoughXZ->Reset();
       HelixProjXZ->Reset();
       
      

}

void STHoughSpaceHelix::DrawHoughSpace(){
       
       HC->cd(1);
       HelixProjXZ->Draw();
       HC->cd(2);
       HistHoughXZ->Draw("zcol");
       
       
       HC->Modified();
       HC->Update();

}





