// =================================================
//  GETMath Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 10. 24
//      Start writing class
// =================================================

#include <cmath>

#include "GETMath.hh"
#include "Riostream.h"

#include "GETDecoder.hh"
#include "GETFrame.hh"

ClassImp(GETMath);

GETMath::GETMath()
{
  fDecoder = 0;
  Reset();
}

GETMath::GETMath(GETDecoder *decoder)
{
  Reset();
  fDecoder = decoder;
}

GETMath::~GETMath()
{
}

void GETMath::Add(Double_t value)
{
  /**
    * This method uses the formulae below to calculate mean and RMS. <br>
    * \f$\text{mean}\Rightarrow \mu_{n+1} = \mu_n + \displaystyle\frac{x_{n+1} - \mu_n}{n + 1},\quad(n\geq0)\f$ <br>
    * \f$\text{RMS}\Rightarrow\sigma^2_{n+1} = \displaystyle\frac{n}{n + 1}\sigma^2_n + \displaystyle\frac{(x_{n+1} - \mu_{n+1})^2}{n},\quad(n>0)\f$ 
   **/
  fMean += (value - fMean)/(Double_t)(fNumValues + 1);
  
  if (fNumValues > 0)
    fRms = fNumValues*fRms/(Double_t)(fNumValues + 1) + pow(value - fMean, 2)/(Double_t)(fNumValues);

  fNumValues++;
}

Double_t GETMath::GetMean()
{
  return fMean;
}

Double_t GETMath::GetRMS()
{
  return sqrt(fRms);
}

void GETMath::Set(Int_t numValues, Double_t mean, Double_t rms)
{
  fNumValues = numValues;
  fMean = mean;
  fRms = rms;
}

Double_t **GETMath::GetAverage(Int_t numChannels, Int_t *chList, Int_t frameNo)
{
  /**
    * This method calculates the average value of channels listed in **chList** for each time bucket in the frame **frameNo**.
    * It returns two dimentional **Double_t** type array. The first dimension is the AGET number which runs from 0 to 3 and
    * the second dimension is the time bucket number which runs from 0 to **fDecoder -> GetNumTbs()**.
    * Returned array is **mean**, for example, then array[2][50] is the averaged value of the time bucket 50 in AGET 2.
    * 
    * If the **numChannels** is negative number, the method calculates average value of all the channels except the channels in **chList**.
    * If the **numChannels** is positive number, the method calculates average value of the channels in **chList**.
    *
    * When **frameNo** is omitted or given as -1, calling the method will give you the average value of the next frame continueously.
   **/

  if (fDecoder == NULL) {
    std::cout << "== GETDecoder is not set!" << std::endl;

    return 0;
  }

  Int_t maxTb = fDecoder -> GetNumTbs();

  for (Int_t iAget = 0; iAget < 4; iAget++) {
    fAdc[iAget] = new Double_t[512];

    for (Int_t iTb = 0; iTb < 512; iTb++)
      fAdc[iAget][iTb] = 0;
  }


  if (frameNo == -1)
    fFrame = fDecoder -> GetFrame();
  else
    fFrame = fDecoder -> GetFrame(frameNo);

  if (!fFrame)
    return 0;

  std::cout << "== Calculating average of frame " << fDecoder -> GetCurrentFrameID();
  if (numChannels == 0)
    std::cout << " of all channels" << std::endl;
  else {
    if (numChannels > 0)
      std::cout << " of channels" << std::endl;
    else
      std::cout << " of all channels except" << std::endl;
    std::cout << " > ";

    for (Int_t iCh = 0; iCh < abs((Double_t)numChannels); iCh++)
      std::cout << chList[iCh] << " ";
    std::cout << std::endl;
  }

  for (Int_t iAget = 0; iAget < 4; iAget++) {
    Int_t divider = 0;
    Double_t tb[512] = {0};

    for (Int_t iCh = 0; iCh < 68; iCh++) {
      Bool_t isSkip = 0;
      if (numChannels < 0) {
        for (Int_t iMask = 0; iMask < -numChannels; iMask++)
          if (iCh == chList[iMask]) {
            isSkip = 1;
            break;
          }
      } else if (numChannels > 0) {
        isSkip = 1;

        for (Int_t iDraw = 0; iDraw < numChannels; iDraw++)
          if (iCh == chList[iDraw]) {
            isSkip = 0;
            break;
          }
      }

      if (isSkip)
        continue;

      Int_t *rawadc = fFrame -> GetRawADC(iAget, iCh);

      for (Int_t iTb = 0; iTb < maxTb; iTb++) {
        tb[iTb] = iTb;
        fAdc[iAget][iTb] += rawadc[iTb];
      }

      divider++;
    }

    for (Int_t iTb = 0; iTb < maxTb; iTb++) 
      fAdc[iAget][iTb] /= (Double_t) divider;
  }

  return fAdc;
}

void GETMath::Reset()
{
  fFrame = 0;

  fNumValues = 0;
  fMean = 0;
  fRms = 0;
}
