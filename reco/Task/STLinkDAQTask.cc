#include "STLinkDAQTask.hh"

#include <FairLogger.h>
#include <FairRootManager.h>
#include <FairRunAna.h>
#include <FairTask.h>

#include <TChain.h>
#include <TClonesArray.h>
#include <TFile.h>
#include <TGraph.h>
#include <TMathBase.h>
#include <TObject.h>

#include <algorithm>
#include <iostream>
#include <memory>

bool STLinkDAQTask::SetInputTree(TString fileName, TString treeName)
{
   if (bdcTree != nullptr) {
      fLogger -> Error(MESSAGE_ORIGIN, "BDC Tree was already set! Add more trees using the function AddInputTree.");
      return false;
   }
   // Add the tree to the input
   bdcTree = new TChain(treeName);

   // If there is a tree with the correct name in the file
   return AddInputTree(fileName);
}

bool STLinkDAQTask::AddInputTree(TString fileName)
{
   auto logger = FairLogger::GetLogger();
   if (bdcTree == nullptr) {
      logger -> Fatal(MESSAGE_ORIGIN, "BDC TChain was not initialized  using the function SetInputTree.");
      return false;
   }

   return (bdcTree->Add(fileName, -1) == 1);
}

InitStatus STLinkDAQTask::Init()
{
   fLogger = FairLogger::GetLogger();
   auto *run = FairRunAna::Instance();
   if (run == nullptr)
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RunAna");


   // Register the input and output branches with the IO manager
   FairRootManager *ioMan = FairRootManager::Instance();
   if (ioMan == nullptr) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RootManager!");
      return kFATAL;
   }

   fInputAuxHeader = dynamic_cast<STAuxHeader *>(ioMan->GetObject(fInputBranchName));
   if (fInputAuxHeader == nullptr) {
      fLogger -> Fatal(MESSAGE_ORIGIN, TString::Format("Cannot find STAuxHeader array in branch %s !", fInputBranchName).Data());
      return kFATAL;
   }

   // Set the branch addresses for the BDC file
   if (bdcTree == nullptr) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "BDC tree was never initialized!");
      return kFATAL;
   }
   bdcTree->SetBranchAddress(fBdcTimeName.Data(), &fBdcTS);

   // Create the new Aux Header
   fOutputAuxHeader = new STAuxHeader();
   ioMan -> Register(fOutputBranchName, "SpiRIT", fOutputAuxHeader, kPersistent);

   return kSUCCESS;
}

void STLinkDAQTask::DoFirstEvent()
{
   bdcTree->GetEntry(0);
   fBdcTimestamp = fBdcTS;
   fTpcTimestamp = fInputAuxHeader->GetTpcTime();

   fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Initial timestamps: %d  %d", fTpcTimestamp, fBdcTimestamp).Data());
   kFirstEvent = false;
   FairRunAna::Instance()->MarkFill(false);
   fBdcTreeIndex = 1;
   fTpcTreeIndex = 1;
}

Int_t STLinkDAQTask::SyncStart()
{
   fIntervalBdc = fBdcTimestamp - fOldBdcTimestamp;
   fIntervalTpc = fTpcTimestamp - fOldTpcTimestamp;

   //std::cout << "Intervals: " << fIntervalTpc << "  " << fIntervalBdc << std::endl;

   fBdcIntervals.push_back(fIntervalBdc);
   fTpcIntervals.push_back(fIntervalTpc);

   if(fTpcTreeIndex < 3)
      return -1;

   double scaledIntervals[2][2] = {};

   for(int i = 0; i < 2; i++) {
      for(int r = 0; r < 2; r++) {
         scaledIntervals[i][r] = GetScaledInterval(fBdcIntervals[r], fTpcIntervals[i]);
         //std::cout << i << ", " << r << ": " << scaledIntervals[i][r] << std::endl;
      }
   }


   if (abs(scaledIntervals[0][0] - 1) < fSearchRadius && abs(scaledIntervals[1][1] - 1) < fSearchRadius) {
      std::cout << "No DAQ offset found. Continuing DAQ Linking." << std::endl;
      return 0;
   }

   if(abs(scaledIntervals[1][0] - 1) < fSearchRadius) {
      std::cout << "DAQ offset found. Extra TPC event at start." << std::endl;
      fBdcTreeIndex--;
      std::cout << "BDC index reduced by 1. Continuing DAQ Linking." << std::endl;
      bdcTree->GetEntry(fBdcTreeIndex -1);
      fBdcTimestamp = fBdcTS;
      return 0;
   }

   if(abs(scaledIntervals[0][1] - 1) < fSearchRadius) {
      std::cout << "DAQ offset found. Extra BDC event at start." << std::endl;
      fBdcTreeIndex++;
      std::cout << "BDC index increased by 1. Continuing DAQ Linking." << std::endl;
      bdcTree->GetEntry(fBdcTreeIndex - 1);
      fBdcTimestamp = fBdcTS;
      return 0;
   }

   std::cout << "FATAL: DAQ linking failed!" << std::endl;
   FairRunAna::Instance() -> TerminateRun();
   return 1;
   
}

bool STLinkDAQTask::UpdateTimestamps()
{
   if (bdcTree->GetEntry(fBdcTreeIndex) <= 0)
      return false;

   fBdcTreeIndex++;
   fTpcTreeIndex++;
   fOldBdcTimestamp = fBdcTimestamp;
   fOldTpcTimestamp = fTpcTimestamp;
   fBdcTimestamp = fBdcTS;
   fTpcTimestamp = fInputAuxHeader->GetTpcTime();

   return true;
}

void STLinkDAQTask::ResetFlags()
{
}

// return < 0 -> Extra evt event
// return == 0 -> match
// return > 0 -> extra TPC
Int_t STLinkDAQTask::CheckMatch()
{
   fIntervalBdc = fBdcTimestamp - fOldBdcTimestamp;
   fIntervalTpc = fTpcTimestamp - fOldTpcTimestamp;

   double scaledInterval = GetScaledInterval(fIntervalBdc, fIntervalTpc);

   if (abs(scaledInterval - 1) < fSearchRadius)
      return 0;

   std::cout << "TPC Timestamp: %d" << fTpcTimestamp << "BDC Timestamp: " << fBdcTimestamp << std::endl
             << "TPC Interval: " << fIntervalTpc << std::endl
             << "BDC Interval: " << fIntervalBdc << std::endl
             << "Scaled Interval: " << scaledInterval << std::endl;

   if (fSearchRadius > 1)
      return -1;
   else
      return 1;
}

void STLinkDAQTask::Exec(Option_t *opt)
{
   fOutputAuxHeader -> Clear();
   fOutputAuxHeader -> SetTpcEventNum(fInputAuxHeader -> GetTpcEventNum());
   fOutputAuxHeader -> SetTpcTime(fInputAuxHeader -> GetTpcTime());
   ResetFlags();
   // Should already have loaded the event from iomanager. If this is the
   // first event then set the old timestamp and continue without filling

   if (kFirstEvent) {
      DoFirstEvent();
      return;
   }

   // Grab both timestamps for this event, set old timestamp and update counter
   if (!UpdateTimestamps()) {
      fLogger -> Warning(MESSAGE_ORIGIN, "Failed to update timestamps. Skipping event");
      FairRunAna::Instance()->MarkFill(false);

      return;
   }

   // Compare both timestamps with their old ones. If it is negative, we must
   // have taken an event before the clocks cleared so reset the old TS
   // and continue without filling
   if (fBdcTimestamp < fOldBdcTimestamp ||
       fTpcTimestamp < fOldTpcTimestamp) {
      std::cout << "Timestamp was reset between BDC event " << fBdcTreeIndex - 2 << " and " << fBdcTreeIndex - 1
                << " reseting old timestamps and "
                << " continuing." << std::endl;
      std::cout << "BDC TS old: " << fOldBdcTimestamp << " BDC TS new: " << fBdcTimestamp << std::endl;
      std::cout << "TPC TS old: " << fOldTpcTimestamp << std::endl
                << " TPC TS new: " << fTpcTimestamp << std::endl;
      FairRunAna::Instance()->MarkFill(false);
      return;
   }

   if(fTpcTreeIndex < 4) {
      SyncStart();
      FairRunAna::Instance()->MarkFill(false);
      return;
   }

   // Compare intervalTPC/intervalBDC to the search mean and radius
   // If it lies within, then fill the event, upadate the old timestamps,
   // and return
   auto match = CheckMatch();
   // fLogger -> Info(MESSAGE_ORIGIN, match;
   if (match == 0) {
      fOutputAuxHeader -> SetBdcID(fBdcTreeIndex - 1);
      Fill();
      return;
   }

   /***** If we have an extra BDC event *****/
   fLogger -> Warning(MESSAGE_ORIGIN, TString::Format("Extra event, match number: %d", match).Data());
   if (match < 0) {

      fLogger -> Info(MESSAGE_ORIGIN, "Extra BDC event found, searching for matching event");

      // Loop through until we hit the last entry of the tree, or until
      // we can match the event interval,
      auto currentIndex = fBdcTreeIndex;
      auto maxIndex = currentIndex + 3;
      if (maxIndex > bdcTree->GetEntries())
         maxIndex = bdcTree->GetEntries();

      // Change to a loop until EVT timestamp is further along
      while (currentIndex < maxIndex) {
         // Get the next NSCL event and associated interval
         bdcTree->GetEntry(currentIndex++);
         fBdcTimestamp = fBdcTS;

         // Check for a match
         if (CheckMatch() == 0) {
            fLogger -> Info(MESSAGE_ORIGIN, "Found match!");
            fBdcTreeIndex = currentIndex;
            fOutputAuxHeader -> SetBdcID(fBdcTreeIndex - 1);
            Fill();

            return;
         }
      } // while(currentIndex < maxIndex)

      fLogger -> Error(MESSAGE_ORIGIN, "Did not find match in next three events");
      FairRunAna::Instance()->MarkFill(false);

   } // if we had an extra EVT event

   /**** We have an extra TPC event *******/
   else {
      fLogger -> Info(MESSAGE_ORIGIN, "Extra TPC  event found, searching for matching event!");

      FairRunAna::Instance()->MarkFill(false);
      // Reset the bdcIndex so we're still looking at the same event
      fBdcTreeIndex--;
      bdcTree->GetEntry(fBdcTreeIndex -1);
      fBdcTimestamp = fBdcTS;
      return;
   }
}
   

void STLinkDAQTask::Fill()
{
}

Double_t STLinkDAQTask::GetScaledInterval(ULong64_t intervalBDC, ULong64_t intervalTPC)
{
   return TMath::Abs(static_cast<double>(intervalTPC) / static_cast<double>(intervalBDC));
}
