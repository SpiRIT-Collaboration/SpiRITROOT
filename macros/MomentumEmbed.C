#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

void MomentumEmbed()
{
	gStyle->SetPalette(kBird);
	TCanvas c1;
	c1.Divide(2,2);

	//for(int i = 0; i < 3; ++i) filename.push_back("data/run2841_s"+std::to_string(i)+".reco.develop.1737.f55eaf6.root");//"Run2841_WithOffset/Run_2841_full.reco.mc";//mc0_s0.reco.v1.04";
	DailyLogger logger("MomentumLow");
	DrawComplex mc_drawer("data/Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc.root", "cbmsim");
        DrawComplex embed_drawer("data/Run2841_WithOffset/LowEnergy/run2841_s*.reco.develop.1737.f55eaf6.root", "cbmsim");


        std::vector<TH1F> hist_mc_arr;
	std::vector<TH1F> hist_embed_arr;
	auto t1 = Clock::now();

	MomentumTracks mc_mom;
	MomentumTracks embed_mom_core;
	EmbedFilter embed_mom;
	embed_mom.AddRule(&embed_mom_core);
	for(int i = 0; i < 4; ++i)
	{
		mc_mom.SetAxis(i);
		embed_mom_core.SetAxis(i);
		hist_mc_arr.push_back(mc_drawer.FillRule<TH1F>(mc_mom, 
                                                         TString::Format("mc_momentum_%d", i).Data(),
                                                         TString::Format("Momentum distribution in axis %d;Momentum (MeV/c)", i).Data(),
                                                         1000, -1000, 1000));
		hist_embed_arr.push_back(embed_drawer.FillRule<TH1F>(embed_mom, 
                                                                     TString::Format("embed_momentum_%d", i).Data(),
                                                                     TString::Format("Momentum distribution in axis %d;Momentum (MeV/c)", i).Data(),
                                                                     1000, -1000, 1000));
	}

	for(auto& hist: hist_mc_arr) logger.SaveClass(hist);
	for(auto& hist: hist_embed_arr) logger.SaveClass(hist);

	for(int i = 0; i < hist_mc_arr.size(); ++i)
	{
		c1.cd(i + 1);
		hist_mc_arr[i].SetLineColor(kBlue);
		hist_mc_arr[i].Scale(1./(double) hist_mc_arr[i].GetEntries());
		hist_mc_arr[i].Draw("hist");

		hist_embed_arr[i].SetLineColor(kRed);
		hist_embed_arr[i].Scale(1./(double) hist_embed_arr[i].GetEntries());
		hist_embed_arr[i].Draw("same hist");
	}

	auto t2 = Clock::now();
	std::cout << "Delta t2-t1: " 
              << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count()
              << " seconds" << std::endl;

	c1.Modified();
	c1.Update();
	while(true && gPad) gPad->WaitPrimitive();

}
