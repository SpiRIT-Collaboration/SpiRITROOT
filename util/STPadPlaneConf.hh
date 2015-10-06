#pragma once

class STPadPlaneConf
{
  public:
    STPadPlaneConf();
    ~STPadPlaneConf();
    static STPadPlaneConf* Instance();

    Int_t GetPadRowWidth();
    Int_t GetPadLayerWidth();
    Int_t GetNumRows();
    Int_t GetNumLayers();
    Int_t GetRow(Double_t x);
    Int_t GetLayer(Double_t z);
    void  GetRowLayer(Double_t x, Double_t z, Int_t &row, Int_t &layer);
    void  GetPadCenter(Int_t row, Int_t layer, Double_t& x, Double_t& z);

  private:
    Int_t fXPadPlane; //!< x width of pad plane (mm)
    Int_t fZPadPlane; //!<  z width of pad plane (mm)

    Int_t fPadWidthRow;   //!< width of row (mm)
    Int_t fPadWidthLayer; //!< width of layer (mm)

    Int_t fNRows;   //!< number of rows
    Int_t fNLayers; //!< number of layers

    static STPadPlaneConf* fInstance;

  ClassDef(STPadPlaneConf,1);
};
