#root -l -q 'run_mc_singleTrack.C("test0")';
root -l -q 'run_mc.C("test0")';
root -l -q 'run_digi.C("test0")';
root -l -q 'run_reco_mc.C("test0")';
root -l 'run_eve.C("test0")';
#root -l 'newEventDisplay_mc.C("test0")';
