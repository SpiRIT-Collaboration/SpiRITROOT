#include "/user/tsangc/ImproveGraph.cxx"

void ExampleDB2()
{
	gStyle->SetPalette(kBird);

	ST_ClusterNum_DB* db = new ST_ClusterNum_DB();
	db->Initial_Config("/mnt/spirit/analysis/user/tsangc/Renshen/Momentum_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.config");
	db->ReadDB("/mnt/spirit/analysis/user/tsangc/Renshen/f1_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.root");
	double Momentum_Range_Plus[2] = {50,3000};
	double Momentum_Range_Minus[2] = {50,1000};
	db->Set_MomentumRange_Plus(Momentum_Range_Plus);
	db->Set_MomentumRange_Minus(Momentum_Range_Minus);
	

	// load embed files
	TChain chain("cbmsim");
	chain.Add("mc0_s0*");
	TTreeReader reader(&chain);

	TTreeReaderValue<TClonesArray> myTrackArray(reader, "STRecoTrack");
                                       //myEmbedArray(reader, "STEmbedTrack");

	// histogram that stores everything
	TH2F hist_embed("num_cluster", "num_cluster", 150, 0, 150, 150, 0, 150);
	TH2F hist_real("num_cluster_real_data", "num_cluster", 150, 0, 150, 150, 0, 150);

	while( reader.Next() )
	{
	
		// look the the embeded track with PID
	/*	for(int i = 0; i < myEmbedArray->GetEntries(); ++i)
		{
			auto embed = static_cast<STEmbedTrack*>(myEmbedArray->At(i));
			int embed_num_cluster = embed->GetNumLayerClusters90() + embed->GetNumRowClusters90();
			auto momVec = embed -> GetFinalMom();
			
			auto mom = momVec.Mag();
			momVec = momVec.Unit();
			auto phiL = momVec.Phi()*180./TMath::Pi();
			phiL = (phiL < 0 ? phiL + 360 : phiL);
			
			auto thetaL = momVec.Theta()*180./TMath::Pi();
			//for 90 layers
			int t_num_cluster = db -> GetClusterNum(1., thetaL, (phiL > 180 ? phiL - 360 : phiL), mom);
			hist_embed.Fill((double) embed_num_cluster, (double) t_num_cluster);
		}*/

		for(int i = 0; i < myTrackArray->GetEntries(); ++i)
		{
			auto track = static_cast<STRecoTrack*>(myTrackArray->At(i));

			// find number of tracks for the embeded pions
			auto nrc90 = track->GetNumRowClusters90();
			auto nlc90 = track->GetNumLayerClusters90();
			int num_cluster = nrc90 + nlc90;
			//if(num_cluster < 20) continue;

			auto mom = track -> GetMomentum().Mag();
			auto charge = track -> GetCharge();
			auto momVec = track -> GetMomentum();
			
			momVec = momVec.Unit();
			auto phiL = momVec.Phi()*180./TMath::Pi();
			phiL = (phiL < 0 ? phiL + 360 : phiL);
			
			auto thetaL = momVec.Theta()*180./TMath::Pi();
			
			//for 90 layers
			int t_num_cluster = db -> GetClusterNum(charge, thetaL, (phiL > 180 ? phiL - 360 : phiL), mom);
			hist_real.Fill((double) num_cluster, (double) t_num_cluster);
		}
	}
	

	TCanvas c1;
	c1.SetBottomMargin(0.15);
	c1.SetLeftMargin(0.15);
	ImproveGraph(&hist_embed, "Number of clusters for embeded pions", "Number of DB clusters");
	hist_embed.Draw("colz");

	TCanvas c2;
	c2.SetBottomMargin(0.15);
	c2.SetLeftMargin(0.15);
	ImproveGraph(&hist_real, "Number of clusters for real data", "Number of DB clusters");
	hist_real.Draw("colz");
	
	TCanvas c3;
	c3.SetBottomMargin(0.15);
	c3.SetLeftMargin(0.15);
	auto embed_copy = static_cast<TH2F*>(hist_embed.Clone());
	auto real_copy = static_cast<TH2F*>(hist_real.Clone());
	embed_copy->Divide(real_copy);
	embed_copy->SetTitle("Ratio of embedding to read data");
	ImproveGraph(embed_copy, "Number of clusters for embedded pions or real data", "Number of DB clusters");
	embed_copy->Draw("colz");

	while(true) gPad->WaitPrimitive();
}
