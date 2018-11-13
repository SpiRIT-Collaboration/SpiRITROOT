#include "STCorrection.hh"

#include <iostream>
using namespace std;

#include "STGlobal.hh"

ClassImp(STCorrection)

class containHits {
public:
  int index;
  std::vector<STHit*> sat;//saturated hits
  std::vector<STHit*> notsat;//not saturated hits
};

static void myFitFunction_helix(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  MyFitFunction* fitFunc_helix = MyFitFunction::Instance_Helix();
  f = fitFunc_helix -> Function_helix(npar, deriv, f, par, flag);
}

static void myFitFunction(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  MyFitFunction* fitFunc = MyFitFunction::Instance();
  f = fitFunc -> Function(npar, deriv, f, par, flag);
}

Double_t MyFitFunction::Function(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  double chisq =0;
  vector<double> stat_array = getmean(par);
  double mean      = stat_array.at(0);
  double total_chg = stat_array.at(1);
  
  int num_elem = hits_pos_ary->size();
  for (int i=0; i<num_elem; i++)
    {
      double v  = total_chg*PRF(hits_pos_ary->at(i)-mean,cluster_alpha);
      if ( v != 0.0 )
	{
	  double n = hits_chg_ary->at(i);
	  chisq += pow(n-v,2)/v;
	}
      else
	{
	  //	  cout << "WARNING -- pdf is negative!!!" << endl;
	}
    }
  
 return chisq;        
}

double MyFitFunction::PRF(double x, double alpha)
{

  double norm[18]  = {0.8423,0.848, 0.8417, 0.8338, 0.82, 0.8, 0.775, 0.7519, 0.723, 0.5312, 0.57825, 0.6197, 0.659546, 0.694289, 0.72305, 0.743942, 0.75782, 0.7664};
  double sigma[18] = {5.61, 5.49, 5.53, 5.601, 5.715, 5.86, 6.06, 6.29, 6.58, 6.1215, 5.6089, 5.2259, 4.904, 4.63829, 4.44797, 4.3118, 4.2215, 4.18};

  if(abs(alpha) < .0001)
    alpha = 0; //sometimes alpha is small and negative 
  int idx = floor(alpha/5.);
  if(idx >=18 || idx < 0)
    {
      cout<<"ERROR PRF_helix index out of range!!!!!!!!!!!! "<<endl;
      cout<<"IDX "<<idx<<" angle "<<alpha<<endl;
    }
  
  gaus_prf->SetParameters(norm[idx],sigma[idx]);

  return ( gaus_prf->Eval(x) );
};

std::vector<double> MyFitFunction::getmean(double par[])
{
  vector<double> output;
  double mean=0;
  double chg =0;
  //Here we loop over non-saturated hits
  int num_elem = hits_pos_ary->size();
  for(int i=0;i<num_elem;++i){
    mean += hits_pos_ary->at(i)*hits_chg_ary->at(i);
    chg += hits_chg_ary->at(i);
  }
  
  //Here we loop over saturated hits
  num_elem = s_hits_pos_ary->size();
  for(int i=0;i<num_elem;++i){
    mean += s_hits_pos_ary->at(i)*par[i];
    chg += par[i];
  }
  mean  = mean/chg;
  output.push_back(mean);
  output.push_back(chg);
  
  return output;
  
};

void MyFitFunction::SetAryPointers(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t)
{
  hits_pos_ary = a;
  hits_chg_ary = a_chg;
  s_hits_pos_ary = b;
  s_hits_chg_ary = b_chg;

  hit_ptrs   = hit_ptrs_t;
  s_hit_ptrs = s_hit_ptrs_t;
}

MyFitFunction* MyFitFunction::_instance = 0;
MyFitFunction* MyFitFunction::_instance_helix = 0;

MyFitFunction* MyFitFunction::Instance()
{
  if (!_instance) _instance = new MyFitFunction;
  return _instance;
}

MyFitFunction* MyFitFunction::Instance_Helix()
{
  if (!_instance_helix) _instance_helix = new MyFitFunction;
  return _instance_helix;
}


STCorrection::STCorrection()
{
}

/*void STCorrection::SetSaturationOption(Int_t opt)
{
  fSaturationOption = opt;
  std::cout << "== [STCorrection] DeSaturation process is turned " << (opt ? "on" : "off") << "!" << std::endl;
}
*/
std::vector<double> STCorrection::minimize(const int npar)
{
  // Initialize minuit, set initial values etc. of parameters.
  vector<double> f_par;
  TMinuit minuit(npar);
  minuit.SetPrintLevel(-1);
  minuit.SetFCN(myFitFunction);
  
  double par[npar];               // the start values
  double stepSize[npar];          // step sizes
  double minVal[npar];            // minimum bound on parameter
  double maxVal[npar];            // maximum bound on parameter
  string parName[npar];
  
  for( int i =0;i < npar; ++i)
    {
      par[i] = 4000.;            // a guess at the true value.
      stepSize[i] = 1.;       // take e.g. 0.1 of start value
      minVal[i] = 3500;   // if min and max values = 0, parameter is unbounded.  Only set bounds if you really think it's right!
      maxVal[i] = 100000;
      parName[i] = "miss charge";
    }
  
  for (int i=0; i<npar; i++)
    {
      minuit.DefineParameter(i, parName[i].c_str(),
			     par[i], stepSize[i], minVal[i], maxVal[i]);
    }
  
  // Do the minimization!
  
  minuit.Migrad();       // Minuit's best minimization algorithm
  double outpar[npar], err[npar];
  for (int i=0; i<npar; i++){
    minuit.GetParameter(i,outpar[i],err[i]);
    f_par.push_back(outpar[i]);
  }
  //       cout << endl << endl << endl;
  //       cout << "*********************************" << endl;
  //       cout << "   "<<parName[0]<<": " << outpar[0] << " +/- " << err[0] << endl;
  
  return f_par;
};


std::vector<double> STCorrection::minimize_helix(const int npar)
{
  // Initialize minuit, set initial values etc. of parameters.
  vector<double> f_par;
  TMinuit minuit(npar);
  minuit.SetPrintLevel(-1);
  minuit.SetFCN(myFitFunction_helix);
  
  double par[npar];               // the start values
  double stepSize[npar];          // step sizes
  double minVal[npar];            // minimum bound on parameter
  double maxVal[npar];            // maximum bound on parameter
  string parName[npar];
  
  for( int i =0;i < npar; ++i)
    {
      par[i] = 4000.;            // a guess at the true value.
      stepSize[i] = 1.;       // take e.g. 0.1 of start value
      minVal[i] = 3500;   // if min and max values = 0, parameter is unbounded.  Only set bounds if you really think it's right!
      maxVal[i] = 100000;
      parName[i] = "miss charge";
    }
  
  for (int i=0; i<npar; i++)
    {
      minuit.DefineParameter(i, parName[i].c_str(),
			     par[i], stepSize[i], minVal[i], maxVal[i]);
    }
  
  // Do the minimization!
  
  minuit.Migrad();       // Minuit's best minimization algorithm
  double outpar[npar], err[npar];
  for (int i=0; i<npar; i++){
    minuit.GetParameter(i,outpar[i],err[i]);
    f_par.push_back(outpar[i]);
  }
  //       cout << endl << endl << endl;
  //       cout << "*********************************" << endl;
  //       cout << "   "<<parName[0]<<": " << outpar[0] << " +/- " << err[0] << endl;
  
  return f_par;
};



void STCorrection::Desaturate(TClonesArray *clusterArray)
{
  //*******
  //Section to do extrapolation
  //and non saturated hits we can now loop over the hits and extrapolate
  //the saturated hits by using the non saturated hits and doing a ChiSq min
  //*******
  
  //MAYBE add some checks here
  //Check ideas:
  //Check that there are more non-saturated pads than saturated for fit sake
  //Check if there are multiple sat pad they are next to each other
  //If not this might indicate delta electrons which of course dont follow
  //the pad response function
  //Check to see distribution is falling off from saturated pads


  std::vector<double> *hits_pos_ary   = new ::std::vector<double>; 
  std::vector<double> *hits_chg_ary   = new ::std::vector<double>;
  std::vector<double> *s_hits_pos_ary = new ::std::vector<double>; 
  std::vector<double> *s_hits_chg_ary = new ::std::vector<double>; 

  std::vector<STHit *> *s_hit_ptrs = new ::std::vector<STHit *>; 
  std::vector<STHit *> *hit_ptrs   = new ::std::vector<STHit *>; 

  MyFitFunction* fitFunc = MyFitFunction::Instance();
  fitFunc -> SetAryPointers(hits_pos_ary,hits_chg_ary,s_hits_pos_ary,s_hits_chg_ary,hit_ptrs,s_hit_ptrs);

  for(int iCluster = 0; iCluster < clusterArray -> GetEntries(); iCluster++)
    {
      hits_pos_ary   -> clear();
      hits_chg_ary   -> clear();
      s_hits_pos_ary -> clear();
      s_hits_chg_ary -> clear();
      
      hit_ptrs       -> clear();
      s_hit_ptrs     -> clear();
      
      auto cluster  = (STHitCluster *) clusterArray -> At(iCluster);
      auto hit_ary = cluster -> GetHitPtrs();
      Bool_t byRow_cl  = cluster -> IsRowCluster();
      fitFunc -> cluster_alpha = cluster -> GetChi()*TMath::RadToDeg();
      fitFunc -> byRow = byRow_cl;

      for(auto hit : *hit_ary)
	  {
	    int max_adc = 3499;       //may have to put this info into the minimizer max and min
	    if( (hit -> GetLayer() <=98 && hit-> GetLayer() >=91) || hit->GetLayer()>=108)
	      max_adc *= 9.8;
	    
	    double chg = hit -> GetCharge();
	    double x   = hit -> GetX();
	    double z   = hit -> GetZ();
	    
	    if(byRow_cl)
	      {
		//by row clustering so look along z
		if(chg >= max_adc)
		  {
		    s_hit_ptrs -> push_back(hit);
		    s_hits_pos_ary -> push_back(z);		
		    s_hits_chg_ary -> push_back(chg);				    
		  }
		else
		  {
		    hit_ptrs -> push_back(hit);
		    hits_pos_ary -> push_back(z);		
		    hits_chg_ary -> push_back(chg);				    
		  }
	      }
	    
	    else
	      {
		//by row clustering so look along x
		if(chg >= max_adc)
		  {
		    s_hit_ptrs -> push_back(hit);
		    s_hits_pos_ary -> push_back(x);		
		    s_hits_chg_ary -> push_back(chg);				    
		  }
		else
		  {
		    hit_ptrs -> push_back(hit);
		    hits_pos_ary -> push_back(x);		
		    hits_chg_ary -> push_back(chg);				    
		  }
	      }
	  }

      //no saturated hits to desaturate, too many to desaturate,or no non saturated hits to extrapolate
      if(s_hits_pos_ary->size() == 0 || s_hits_pos_ary->size() > 4 || hits_pos_ary->size() ==0) 
	continue;
      
      const int npar = s_hits_pos_ary->size();;
      vector<double>outpar = minimize(npar);
      
      if(outpar.size() != s_hit_ptrs->size())
	cout<<"[STCorrection] Ouput parameter of MINUIT is different than number of saturated hits"<<endl;
      for(int iSatHit = 0; iSatHit < s_hit_ptrs -> size(); iSatHit++)
	{
	  double new_chg = outpar.at(iSatHit);
	  if( abs(new_chg-3500)<.001 || abs(new_chg-100000)<.001 )//set max min of MINUIT
	    continue;
	  
	  auto hit = s_hit_ptrs -> at(iSatHit);
	  hit -> SetCharge(new_chg);
	}
      cluster -> ApplyModifiedHitInfo(); //update cluster position,charge, covariance, etc...
    }


  return;
}

Double_t MyFitFunction::Function_helix(Int_t& npar, Double_t* deriv, Double_t &f, Double_t* par, Int_t flag)
{
  double chisq =0;
  double total_chg = 0.;
  int num_hits = hits_lambda_ary->size();
  int num_s_hits = s_hits_lambda_ary->size();

  for(int i = 0; i < num_s_hits; ++i)
    total_chg += par[i]; //add saturated chg estimate 

  for(int i = 0; i < num_hits; ++i)
    total_chg += hits_chg_ary->at(i);//add measured hits
  
  for (int i=0; i < num_hits; i++)
    {
      double v  = total_chg*PRF_helix(hits_lambda_ary->at(i),cluster_alpha);
      if ( v != 0.0 )
	{
	  double n = hits_chg_ary->at(i);
	  chisq += pow(n-v,2)/v;
	}
      else
	{
	  //	  cout << "WARNING -- pdf is negative!!!" << endl;
	}
    }
  
 return chisq;        
}

double MyFitFunction::PRF_helix(double x, double alpha)
{

  double norm[18]  = {0.8968,0.9206,0.916,0.9,0.8595,0.8244,0.794,0.7559,0.509,0.5609,0.6101,0.6602,0.69689,0.7358,0.7607,0.7794,0.788};
  double sigma[18] = {5.4998, 5.3209, 5.34, 5.427, 5.5159, 5.663, 5.874, 6.126, 6.446, 6.367, 5.7938, 5.3636, 4.909, 4.645, 4.4248, 4.302, 4.1824, 4.108};
  
  if(abs(alpha) < .0001)
    alpha = 0; //sometimes alpha is small and negative 
  int idx = floor(alpha/5.);
  if(idx >=18 || idx < 0)
    {
      cout<<"ERROR PRF_helix index out of range!!!!!!!!!!!! "<<endl;
      cout<<"IDX "<<idx<<" angle "<<alpha<<endl;
    }
  
  gaus_prf_helix->SetParameters(norm[idx],sigma[idx]);

  return ( gaus_prf_helix->Eval(x) );
};


void MyFitFunction::SetLambdaChg(std::vector<double> *a, std::vector<double> *a_chg, std::vector<double> *b, std::vector<double> *b_chg, std::vector<STHit*> *hit_ptrs_t, std::vector<STHit*> *s_hit_ptrs_t)
{
  hits_lambda_ary = a;
  hits_chg_ary = a_chg;
  s_hits_lambda_ary = b;
  s_hits_chg_ary = b_chg;

  hit_ptrs   = hit_ptrs_t;
  s_hit_ptrs = s_hit_ptrs_t;
}

void STCorrection::Desaturate_byHelix(TClonesArray *helixArray, TClonesArray *clusterArray)
{

  std::vector<double> *hits_lambda_ary   = new ::std::vector<double>; 
  std::vector<double> *hits_chg_ary   = new ::std::vector<double>;
  std::vector<double> *s_hits_lambda_ary = new ::std::vector<double>; 
  std::vector<double> *s_hits_chg_ary = new ::std::vector<double>; 

  std::vector<STHit *> *s_hit_ptrs = new ::std::vector<STHit *>; 
  std::vector<STHit *> *hit_ptrs   = new ::std::vector<STHit *>; 

  MyFitFunction* fitFunc = MyFitFunction::Instance_Helix();
  fitFunc -> SetLambdaChg(hits_lambda_ary,hits_chg_ary,s_hits_lambda_ary,s_hits_chg_ary,hit_ptrs,s_hit_ptrs);

  for(int iHelix = 0; iHelix < helixArray->GetEntries(); iHelix++)
    {
      auto helix = (STHelixTrack *) helixArray->At(iHelix);
      auto cluster_ary = helix -> GetClusterArray();
      
      for(auto cluster : *cluster_ary)
	{
	  hits_lambda_ary   -> clear();
	  hits_chg_ary   -> clear();
	  s_hits_lambda_ary -> clear();
	  s_hits_chg_ary -> clear();
	  
	  hit_ptrs       -> clear();
	  s_hit_ptrs     -> clear();

	  auto hit_ary = cluster -> GetHitPtrs();
	  Bool_t byRow_cl  = cluster -> IsRowCluster();
	  fitFunc -> cluster_alpha = cluster -> GetChi()*TMath::RadToDeg();
	  fitFunc -> byRow = byRow_cl;
	    
	  for(auto hit : *hit_ary)
	    {
	      TVector3 pointOnHelix;
	      Double_t alpha;
	      auto hit_vector = hit -> GetPosition();

	      if(byRow_cl)
		helix->ExtrapolateToX(hit_vector.X(),pointOnHelix);
	      else
		helix->ExtrapolateToZ(hit_vector.Z(),pointOnHelix);
	      
	      pointOnHelix = pointOnHelix - hit_vector;

	      int max_adc = 3499;       //may have to put this info into the minimizer max and min
	      if( (hit -> GetLayer() <=98 && hit-> GetLayer() >=91) || hit->GetLayer()>=108)
		max_adc *= 9.8;

	      double chg = hit -> GetCharge();

	      double z = pointOnHelix.Z();
	      double x = pointOnHelix.X();

	      if(byRow_cl)
		{
		  //by row clustering so look along z
		  if(chg >= max_adc)
		    {
		      s_hit_ptrs -> push_back(hit);
		      s_hits_lambda_ary -> push_back(z);
		      s_hits_chg_ary -> push_back(chg);
		    }
		  else
		    {
		      hit_ptrs -> push_back(hit);
		      hits_lambda_ary -> push_back(z);
		      hits_chg_ary -> push_back(chg);
		    }
		}

	      else
		{
		  //by row clustering so look along x
		  if(chg >= max_adc)
		    {
		      s_hit_ptrs -> push_back(hit);
		      s_hits_lambda_ary -> push_back(x);
		      s_hits_chg_ary -> push_back(chg);
		    }
		  else
		    {
		      hit_ptrs -> push_back(hit);
		      hits_lambda_ary -> push_back(x);
		      hits_chg_ary -> push_back(chg);
		    }
		}

	    }

	  //no saturated hits to desaturate, too many to desaturate,or no non saturated hits to extrapolate
	  if(s_hits_lambda_ary->size() == 0 || s_hits_lambda_ary->size() > 4 || hits_lambda_ary->size() ==0)
	    continue;

	  const int npar = s_hits_lambda_ary->size();;
	  vector<double>outpar = minimize_helix(npar);

	  if(outpar.size() != s_hit_ptrs->size())
	    cout<<"[STCorrection] Ouput parameter of MINUIT is different than number of saturated hits"<<endl;

	  for(int iSatHit = 0; iSatHit < s_hit_ptrs -> size(); iSatHit++)
	    {
	      double new_chg = outpar.at(iSatHit);
	      if( abs(new_chg-3500)<.001 || abs(new_chg-100000)<.001 )//set max min of MINUIT
		continue;

	      auto hit = s_hit_ptrs -> at(iSatHit);
	      hit -> SetCharge(new_chg);
	    }
	  cluster -> ApplyModifiedHitInfo(); //update cluster position,charge, covariance, etc...

	}
    }

  return;
}

void STCorrection::LoadPRFCut(TString filename)
{
  TFile f(filename);
  prf_layer = (TCutG*)f.Get("prf_layer");
  prf_row   = (TCutG*)f.Get("prf_row");

  return;
}

void STCorrection::CheckClusterPRF(TClonesArray *clusterArray, TClonesArray *helixArray, TClonesArray *hitArray)
{


  for(auto iHelix = 0; iHelix < helixArray -> GetEntries(); iHelix++)
    {
      auto helix = (STHelixTrack *) helixArray ->At(iHelix);
      auto cl_id = helix-> GetClusterIDArray();
      for( auto iCluster = 0; iCluster < cl_id->size(); iCluster++)
	{
	  auto cluster = (STHitCluster *)clusterArray -> At(cl_id->at(iCluster));
	  auto hit_ary = cluster -> GetHitIDs();
	  vector<double> lambda,fract;
	  bool byRow = cluster -> IsRowCluster();

	  for(auto idx : *hit_ary)
	    {
	      TVector3 pointOnHelix;
	      Double_t alpha;
	      auto hit = (STHit *) hitArray -> At(idx);
	      auto hit_vector = hit -> GetPosition();
	      helix -> ExtrapolateToPointAlpha(hit->GetPosition(), pointOnHelix, alpha);
	      pointOnHelix = pointOnHelix - hit_vector;

	      if(byRow)
		{
		  lambda.push_back(pointOnHelix.Z());
		  fract.push_back(hit->GetCharge()/cluster ->GetCharge());
		}
	      else
		{
		  lambda.push_back(pointOnHelix.X());
		  fract.push_back(hit->GetCharge()/cluster ->GetCharge());
		}
	    }

	  int num_in = 0;// number of hits inside prf cuts
	  for(int i = 0; i< lambda.size(); i++)
	    {
	      if(byRow)
		{
		  if(prf_row -> IsInside(lambda.at(i),fract.at(i)))
		    num_in++;
		}
	      else
		{
		  if(prf_layer -> IsInside(lambda.at(i),fract.at(i)))
		    num_in++;
		}
	    }

	  if((1.*num_in)/hit_ary->size() < .5)
	    {
	      cluster -> SetIsStable(false);
	    }
	}
    }

  return;
}
