#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void printProgress (double percentage)
{
  int val = (int) (percentage * 100);
  int lpad = (int) (percentage * PBWIDTH);
  int rpad = PBWIDTH - lpad;
  printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
  fflush (stdout);
}

struct track {
  int run,event;
  double vx,vy,vz;
};
  
const Int_t fNumSn108 = 90;
Int_t fSn108[fNumSn108] = {2261, 2262, 2264, 2270, 2271, 2272, 2273, 2274, 2275, 2276,
                           2283, 2284, 2285, 2286, 2288, 2289, 2291, 2310, 2311, 2314,
                           2315, 2320, 2322, 2323, 2324, 2325, 2331, 2332, 2333, 2334,
                           2335, 2336, 2337, 2340, 2341, 2362, 2363, 2368, 2369, 2370,
                           2371, 2372, 2373, 2374, 2375, 2378, 2379, 2380, 2381, 2382,
                           2383, 2384, 2385, 2386, 2387, 2388, 2389, 2391, 2392, 2393,
                           2394, 2395, 2396, 2397, 2398, 2399, 2400, 2401, 2402, 2429,
                           2432, 2433, 2434, 2437, 2438, 2439, 2440, 2442, 2453, 2461,
                           2462, 2463, 2501, 2502, 2503, 2505, 2506, 2507, 2508, 2509};

const Int_t fNumSn132 = 113;
Int_t fSn132[fNumSn132] = {2841, 2843, 2844, 2845, 2846, 2848, 2849, 2850, 2851, 2852,
                           2855, 2856, 2857, 2858, 2859, 2860, 2861, 2875, 2877, 2878,
                           2879, 2880, 2881, 2882, 2883, 2884, 2887, 2888, 2889, 2890,
                           2891, 2892, 2893, 2894, 2896, 2898, 2899, 2900, 2901, 2902,
                           2903, 2904, 2905, 2907, 2914, 2916, 2917, 2919, 2920, 2921,
                           2922, 2924, 2925, 2926, 2927, 2929, 2930, 2931, 2932, 2933,
                           2934, 2935, 2936, 2939, 2940, 2941, 2942, 2943, 2944, 2945,
                           2946, 2948, 2955, 2956, 2958, 2959, 2960, 2961, 2962, 2964,
                           2965, 2966, 2968, 2969, 2970, 2971, 2972, 2973, 2975, 2976,
                           2977, 2978, 2979, 2980, 2981, 2982, 2983, 2984, 2985, 2986,
                           2988, 2989, 2990, 2991, 2992, 2993, 2997, 2999, 3000, 3002, 
                           3003, 3007, 3039};

const Int_t fNumSn124 = 68;
Int_t fSn124[fNumSn124] = {3059, 3061, 3062, 3065, 3066, 3068, 3069, 3071, 3074, 3075,
                           3076, 3077, 3078, 3080, 3081, 3082, 3083, 3084, 3085, 3087,
                           3088, 3089, 3090, 3091, 3092, 3093, 3094, 3095, 3097, 3098,
                           3102, 3103, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145,
                           3146, 3148, 3149, 3150, 3151, 3152, 3153, 3154, 3155, 3156,
                           3157, 3158, 3159, 3165, 3166, 3167, 3168, 3169, 3170, 3171,
                           3172, 3177, 3179, 3180, 3181, 3182, 3183, 3184};



std::map<Int_t, Int_t> fSn108Index;
std::map<Int_t, Int_t> fSn132Index;

void GetVertexList() {

  for (auto iIndex = 0; iIndex < fNumSn132; iIndex++)
    fSn132Index[fSn132[iIndex]] = iIndex;

  for (auto iIndex = 0; iIndex < fNumSn108; iIndex++)
    fSn108Index[fSn108[iIndex]] = iIndex;

  TString path = "/mnt/spirit/analysis/changj/SpiRITROOT.develop.latest/macros/data/analysisCode-JustinPRF-pion/";
  
  double sigma = 1.69675;
  double mean  = -11.9084;
  double low_vz = mean - 3*sigma;
  double high_vz = mean + 3*sigma;

  vector<track> track_vec;
  Double_t vx,vy,vz;
  Bool_t sigma20;
  Int_t eventid,run;

  for(int iRun = 0; iRun < 60; iRun++)
    {
      auto t = new TChain("mult");

      //TString filename = Form(path+"dedxROOT/dedxSn132-LC112-%d.root", iRun);
      TString filename = Form("/mnt/spirit/analysis/changj/SpiRITROOT.latest.forTommy/macros/data/analysisCode-analysis-pion-Sn124/singles/multSn124-%d.root", iRun);
      cout<<"Loading file "<<filename<<endl;
      t -> AddFile(filename);

      run = fSn124[iRun];
      //t -> SetBranchAddress("run", &run);
      t -> SetBranchAddress("eventid", &eventid);
      t -> SetBranchAddress("vx",&vx);
      t -> SetBranchAddress("vy",&vy);
      t -> SetBranchAddress("vz",&vz);
      t -> SetBranchAddress("sn124s20",&sigma20);

      for(int iEntry = 0; iEntry < t->GetEntries(); iEntry++)
      //  for(int iEntry = 0; iEntry < t->GetEntries(); iEntry++)
        {
          t->GetEntry(iEntry);
          if(iEntry%100000==0)
            std::cout << iEntry << " over " << t->GetEntries() << std::endl;//printProgress((1.*iEntry)/t->GetEntries());

          track t_entry;
          t_entry.run = run;
          t_entry.event = eventid;
          t_entry.vx = vx;
          t_entry.vy = vy;
          t_entry.vz = vz;

          if( !( (vz < high_vz && vz > low_vz) && sigma20 == true) ) //vertex && beamcut
            continue;

          if(track_vec.size() == 0)
              track_vec.push_back(t_entry);
          else if(eventid != track_vec.back().event)
            track_vec.push_back(t_entry);

          if(track_vec.size() > 1e4) break;
        }
    }

  //  for(int i =0 ;i<track_vec.size(); i++)
  //    cout<<track_vec.at(i).run<<" "<<track_vec.at(i).event<<" "<<track_vec.at(i).vz<<endl;

  ofstream outfile;
  outfile.open("VertexLocation.txt");
  outfile<<"#RunNum   EventNum    x(mm)    y(mm)    z(mm)"<<endl;

  TRandom3 *ran = new TRandom3(12345);
  int num_picked = 1e4; //number of picked events

  if(num_picked > track_vec.size())
    {
      cout<<endl;
      cout<<"ERROR Size of data events smaller than number of picked events you want " << track_vec.size() <<endl;
      return;
    }
  
  //Pick events from a non repeating uniform distribution
  vector<int> entry ={}, picked_entries = {};
  for(int i = 0; i < track_vec.size(); i++)
    entry.push_back(i);

  for(int iPick = 0; iPick < num_picked; iPick++)
    {
      int idx = ran -> Integer(entry.size()-iPick-1);

      int a_i   = entry.at(idx);
      int a_j = entry.at(entry.size()-iPick-1);
      entry.at(idx) = a_j;
      entry.at(entry.size()-iPick-1)= a_i;

      picked_entries.push_back(a_i);
    }

  std::sort(picked_entries.begin(),picked_entries.end());
  
  for(int iPick = 0; iPick < num_picked; iPick++)
    {
      int idx = picked_entries.at(iPick);
      outfile<<std::fixed<<track_vec.at(idx).run<<'\t'
	     <<std::fixed<<track_vec.at(idx).event<<'\t'
 	     <<std::fixed<<std::setprecision(3)<<track_vec.at(idx).vx<<'\t'
	     <<std::fixed<<std::setprecision(3)<<track_vec.at(idx).vy<<'\t'
 	     <<std::fixed<<std::setprecision(3)<<track_vec.at(idx).vz<<'\n';
    }
  
  outfile.close();

}
