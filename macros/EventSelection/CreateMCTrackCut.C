void CreateMCTrackCut()
{
	DrawComplex mc_draw("data/Run2841_WithOffset/HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");
	DrawHit hist_xz;
	GetHitOutline gethit("HitsHighECutG.root");

	hist_xz.AddRule(&gethit);
	auto hist = mc_draw.FillRule<TH2F>(hist_xz, "temp", "", 100, -100, 100, 100, -100, 100);
}
