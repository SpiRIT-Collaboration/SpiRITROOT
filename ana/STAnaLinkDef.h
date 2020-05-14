#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Add namespaces
#pragma link C++ namespace STAnaParticleDB+;

// Add the class names you created
#pragma link C++ class ST_ProduceDB_ClusterNum+;
#pragma link C++ class ST_ClusterNum_DB+;
#pragma link C++ class ST_TrackCut+;
#pragma link C++ class STConcReaderTask+;
#pragma link C++ class STPIDAnalysisTask+;
#pragma link C++ class STPIDCutTask+;
#pragma link C++ class STPIDProbTask+;
#pragma link C++ class STPIDMachineLearningTask+;
#pragma link C++ class STEfficiencyTask+;
#pragma link C++ class STTransformFrameTask+;
#pragma link C++ class STMatchImQMDTask+;
#pragma link C++ class STFilterTask+;
#pragma link C++ class STFilterEventTask+;
#pragma link C++ class STAddBDCInfoTask+;
#pragma link C++ class STSimpleGraphsTask+;

#pragma link C++ class EfficiencyFactory+;
#pragma link C++ class EfficiencyFromConcFactory+;
#pragma link C++ class EfficiencyInCMFactory+;

#endif
