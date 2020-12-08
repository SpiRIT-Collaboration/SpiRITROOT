#include "STUrQMDFormatWriterTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"

#include <cmath>

ClassImp(STUrQMDFormatWriterTask);

STUrQMDFormatWriterTask::STUrQMDFormatWriterTask(const std::string& output_name, const std::string& urqmd_raw, bool simple_format) : fOutput(output_name), fSimpleFormat(simple_format)
{
  fLogger = FairLogger::GetLogger(); 
  if(!fOutput.is_open())
    fLogger -> Info(MESSAGE_ORIGIN, "UrQMDFormatWriter cannot write to output. Will not write to output");
  else if(fSimpleFormat)
    fOutput << "dEdX,Px,Py,Pz,Charge,NClus,Type\n";
  if(!urqmd_raw.empty())
  {
    fLogger -> Info(MESSAGE_ORIGIN, ("Will load additional meta data from " + urqmd_raw).c_str());
    fUrQMDReader = new STUrQMDReader(urqmd_raw);
  } 
 
  fEventHeader = 
"UQMD   version:       -99999   -9999  -9999  output_file  -9999\n"
"projectile:  (mass, char)  %d  %d   target:  (mass, char)  %d  %d\n"
"transformation betas (NN,lab,pro)     -9999 -9999 -9999\n"
"impact_parameter_real/min/max(fm):    %.2f  0.00 10.00  total_cross_section(mbarn):    -9999\n"
"equation_of_state:    1  E_lab(GeV/u): -9999  sqrt(s)(GeV): -9999  p_lab(GeV/u): -9999\n"
"event#         %d random seed:  -9999 (auto)   total_time(fm/c):      -9999 Delta(t)_O(fm/c):      -9999\n"
"op  0    0    0    0 *  1    0    0    0    0    0    1    0    0    0    0\n"
"op  0    0    0    0    0    0    1    0    0 *  0    0    0    0    2    0\n"
"op  0    0    0    1    1    0    0    0    0    0    0    0    0    1    0\n"
"op  0    5    0    0    0    0    0    0    0    0    0    1    0    0    0\n"
"pa 0.E+01   0.E+00   0.E+01   0.E+00   0.E+00   0.E+00   0.E+00   0.E-01   0.E+00   0.E+00   0.E+00   0.E+00\n"
"pa 0.E+00   0.E+00   0.E+00   0.E+01   0.E+01   0.E+00   0.E+01   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00\n"
"pa 0.E+00   0.E+02   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+00   0.E+01   0.E+00   0.E+01   0.E+00\n"
"pa 0.E+00   0.E+01   0.E+00   0.E+00   0.E+00   0.E+00   0.E+01   0.E+00   0.E+00   0.E+06   0.E+01   0.E+01\n"
"pa 0.E+00   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+01   0.E+00   0.E+01\n"
"pa 0.E+00   0.E+03   0.E+01   0.E+01   0.E+01   0.E+11   0.E+01   0.E+01   0.E+00   0.E+11   0.E+01   0.E+00\n"
"pa 0.E+01   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00   0.E+00\n"
"pvec: r0              rx              ry              rz              p0              px              py              pz              m          ityp 2i3 chg lcl#  ncl or\n"
"          %d         -9999   -9999        -9999\n"
"      0      0       0      0       0       0       0       0\n";
  fSkip = new STVectorI;
  fSkip -> fElements.push_back(0);

}

STUrQMDFormatWriterTask::~STUrQMDFormatWriterTask()
{ 
  if(fUrQMDReader) delete fUrQMDReader;
}

InitStatus STUrQMDFormatWriterTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fEventID = (TClonesArray*) ioMan -> GetObject("EventID");

  // fSkip could have been provided by STFilterEventTask
  // if so we just graph the existing Skip
  if(STVectorI* skip = (STVectorI*) ioMan -> GetObject("Skip"))
  {
    delete fSkip;
    fSkip = skip; 
  }
  else ioMan -> Register("Skip", "ST", fSkip, false); // Skip flag is only used internally

  return kSUCCESS;
}

void STUrQMDFormatWriterTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STUrQMDFormatWriterTask::Exec(Option_t *opt)
{
  if(!fOutput.is_open()) return;
  auto data = static_cast<STData*>(fData -> At(0));

  if(fSimpleFormat)
  {
    for(int j = 0; j < data -> multiplicity; ++j)
      if(data -> recodpoca[j].Mag() < 20 && data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] > 15)
        fOutput << data -> vadedx[j] << "," << data -> vaMom[j].x() << "," << data -> vaMom[j].y() << "," 
                << data -> vaMom[j].z() << "," << data -> recoCharge[j] << "," 
                << data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] << ",Null\n";
  }
  else
  {
    double b = -9999;
    int id = static_cast<STVectorI*>(fEventID -> At(0)) -> fElements[0];
    if(fUrQMDReader) 
    {
      fUrQMDReader -> SetEntry(id - 1);
      b = fUrQMDReader -> GetB();
    }

    if(fBMin >= 0 && fBMax >= 0)
    {
      if(!(fBMin <= b && b <= fBMax)) 
      {
        fSkip -> fElements[0] = 1;
        return;
      }
      else fSkip -> fElements[0] = 0;
    }

    int mult_identified = 0;
    std::stringstream ss;

    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      auto& cm_vector = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements;
      auto& prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements;
      int pdg = fSupportedPDG[i];
      auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
      for(int j = 0; j < data -> multiplicity; ++j)
        if(prob[j] > 0.5)
        {
          ++mult_identified;
          int A = particle -> Mass()/STAnaParticleDB::kAu2Gev + 0.5;
          int Z = particle -> Charge()/3 + 0.5;
          int ityp;
          if(std::fabs(pdg) == 211)
          {
            ityp = 101;
            A = Z = 0; 
          }
          else if(pdg == 2212 || pdg == 2112) ityp = 1;
          else ityp = 500 + A;
          int N = A - Z;

          double p0 = std::sqrt(cm_vector[j].Mag2()/1000000 + particle -> Mass()*particle -> Mass());
          ss << std::scientific << "0 0 0 0 " << p0 << " " << cm_vector[j].x()/1000. << " " << cm_vector[j].y()/1000. << " " << cm_vector[j].z()/1000.;
          ss << " " << particle -> Mass() << " " << ityp << " " << Z - N << " " << Z << " 0 0 0" << std::endl;
        }
    }
    
    fOutput << TString::Format(fEventHeader, 132, 50, 124, 50, b, 
                               id, mult_identified);
    fOutput << ss.str();
  }
}

void STUrQMDFormatWriterTask::FinishTask()
{
  fOutput << std::flush;
}
