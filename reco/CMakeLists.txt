# Create a library called "libSTReco" which includes the source files given in the array .

set(INCLUDE_DIRECTORIES
${ROOT_INCLUDE_DIR} 
${EIGEN3_INCLUDE_DIR}
${GENFIT2_INCLUDE_DIR}
${BASE_INCLUDE_DIRECTORIES}
# Add all the folders having header files you used in the code.
${CMAKE_SOURCE_DIR}/global
${CMAKE_SOURCE_DIR}/mc
${CMAKE_SOURCE_DIR}/stack
${CMAKE_SOURCE_DIR}/par
${CMAKE_SOURCE_DIR}/format
${CMAKE_SOURCE_DIR}/tool
${CMAKE_SOURCE_DIR}/util
${CMAKE_SOURCE_DIR}/field
${CMAKE_SOURCE_DIR}/reco
${CMAKE_SOURCE_DIR}/reco/Corrections
${CMAKE_SOURCE_DIR}/reco/GETDecoder
${CMAKE_SOURCE_DIR}/reco/STConverter
${CMAKE_SOURCE_DIR}/reco/PulseShapeAnalyzer
${CMAKE_SOURCE_DIR}/reco/HitClustering
${CMAKE_SOURCE_DIR}/reco/SystemManipulator
${CMAKE_SOURCE_DIR}/reco/RiemannTracking
${CMAKE_SOURCE_DIR}/reco/HoughTracking
${CMAKE_SOURCE_DIR}/reco/Genfit
${CMAKE_SOURCE_DIR}/reco/Tracking
${CMAKE_SOURCE_DIR}/reco/LinearTracking
${CMAKE_SOURCE_DIR}/reco/CurveTracking
${CMAKE_SOURCE_DIR}/reco/HelixTracking
${CMAKE_SOURCE_DIR}/reco/Task
${CMAKE_SOURCE_DIR}/reco/PID
${CMAKE_SOURCE_DIR}/reco/BDCMagneticProjection
${CMAKE_SOURCE_DIR}/reco/SpaceCharge
)

include_directories(${INCLUDE_DIRECTORIES})

include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${GENFIT2_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
) 

link_directories(${LINK_DIRECTORIES})

set(SRCS
# Add all the source files below this line. Those must have cc for their extension.
Task/STRecoTask.cc
Task/STEventPreviewTask.cc
Task/STPSAETask.cc
Task/STCurveTrackingETask.cc
Task/STHitClusteringCTTask.cc
Task/STSMETask.cc
Task/STRiemannTrackingETask.cc
Task/STGenfitETask.cc
Task/STHelixTrackingTask.cc
Task/STMCTruthTask.cc
Task/STPIDCorrelatorTask.cc
Task/STRiemannToHelixTask.cc
Task/STGenfitSinglePIDTask.cc
Task/STGenfitPIDTask.cc
Task/STGenfitVATask.cc
Task/STEmbedCorrelatorTask.cc
Task/STCorrectionTask.cc
Task/STMCEventTask.cc
Task/ST_VertexShift.cc
Task/STSpaceChargeCorrectionTask.cc
Task/STSmallOutputTask.cc

Corrections/STCorrection.cc

PID/STPIDTest.cc

GETDecoder/GETDecoder.cc
GETDecoder/GETFrameInfo.cc

GETDecoder/GETHeaderBase.cc
GETDecoder/GETBasicFrameHeader.cc
GETDecoder/GETLayerHeader.cc

GETDecoder/GETTopologyFrame.cc
GETDecoder/GETBasicFrame.cc
GETDecoder/GETCoboFrame.cc
GETDecoder/GETLayeredFrame.cc
GETDecoder/GETMutantFrame.cc

GETDecoder/GETMath.cc
GETDecoder/GETFileChecker.cc

STConverter/STCore.cc
STConverter/STPedestal.cc
STConverter/STGainCalibration.cc
STConverter/STGenerator.cc
STConverter/STGainCheck.cc
STConverter/STMap.cc
STConverter/STMapTest.cc
STConverter/STPlot.cc
STConverter/STParReader.cc
STConverter/STStatic.cc
STConverter/STGGNoiseSubtractor.cc
STConverter/STGainMatching.cc

PulseShapeAnalyzer/STPSA.cc
PulseShapeAnalyzer/STPSASimple.cc
PulseShapeAnalyzer/STPSAAll.cc
PulseShapeAnalyzer/STPSALayer.cc
PulseShapeAnalyzer/STPSALayerOPTICS.cc
PulseShapeAnalyzer/STPSADF.cc
PulseShapeAnalyzer/STPSAFast.cc
PulseShapeAnalyzer/STPSAFastFit.cc
PulseShapeAnalyzer/STPulse.cc

HitClustering/STClusterizer.cc
HitClustering/STClusterizerScan.cc
HitClustering/STClusterizerScan2.cc
HitClustering/STClusterizerLinearTrack.cc
HitClustering/STClusterizerCurveTrack.cc

SystemManipulator/STSystemManipulator.cc

RiemannTracking/STRiemannTrackFinder.cc
RiemannTracking/STDipTTCorrelator.cc
RiemannTracking/STRiemannTTCorrelator.cc
RiemannTracking/STProximityHTCorrelator.cc
RiemannTracking/STHelixHTCorrelator.cc

HoughTracking/STHoughSpaceLine.cc
HoughTracking/STHoughSpaceHelix.cc

Genfit/STSpacepointMeasurement.cc
Genfit/STGFBField.cc
Genfit/STGenfitTest.cc
Genfit/STGenfitTestE.cc
Genfit/STGenfitTest2.cc

Tracking/STVTrackFinder.cc

LinearTracking/STCorrLinearTHProx.cc
LinearTracking/STCorrLinearTHPerp.cc
LinearTracking/STCorrLinearTHRMS.cc
LinearTracking/STCorrLinearTTGeo.cc
LinearTracking/STCorrLinearTTPerp.cc
LinearTracking/STCorrLinearTTProx.cc
LinearTracking/STCorrLinearTTRMS.cc
LinearTracking/STLinearTrackFinder.cc
LinearTracking/STLinearTrackFinder2.cc
LinearTracking/STLinearTrackFitter.cc

STDecoderTask.cc
STEmbedTask.cc

STPSATask.cc
STHitClusteringTask.cc
STSMTask.cc
STRiemannTrackingTask.cc
STGenfitTask.cc
STSource.cc

STLinearTrackingTask.cc

CurveTracking/STCurveTrackFinder.cc
CurveTracking/STCurveTrackFitter.cc
CurveTracking/STCorrCurveTHProx.cc
CurveTracking/STCorrCurveTHPerp.cc

STCurveTrackingTask.cc

HelixTracking/STHelixTrackFinder.cc

BDCMagneticProjection/STBeamEnergy.cc
BDCMagneticProjection/STBDCProjection.cc

SpaceCharge/STSpaceCharge.cc
)

CHANGE_FILE_EXTENSION(*.cc *.hh HEADERS "${SRCS}")

Set(LINKDEF STRecoLinkDef.h)
Set(LIBRARY_NAME STReco)
Set(DEPENDENCIES
    Base
    Stack
# Add all library names your code depending on.
# Basically those are the libraries in the header files' directory.
    STGlobal
    STFormat
    STTool
    STPar
    STUtil
    STField
    Spectrum
    Minuit
    genfit2
)

GENERATE_LIBRARY()
