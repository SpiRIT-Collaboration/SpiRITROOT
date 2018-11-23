#ifndef RECORULESSS_H
#define RECORULESSS_H
#include "HistToCutG.hh"
#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>

#include "TMath.h"
#include "TH1.h"
#include "TGraph.h"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TClonesArray.h"
#include "TCutG.h"
#include "TChain.h"

#include "STRecoTrack.hh"
#include "STEmbedTrack.hh"
#include "STVertex.hh"
#include "ST_ClusterNum_DB.hh"
#include "Rules.hh"

class RecoTrackRule : public Rule
{
public: 
    RecoTrackRule() : can_init_loop_(true) {};
    virtual void SetReader(TTreeReader& t_reader) override;
    virtual void Fill(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    STRecoTrack* track_; 
    int track_id_;
    bool can_init_loop_;
};

class UseVATracks : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myVATracksArray_;
    std::vector<int> idx_map;
    int current_entry_ = 2;
};

class EmbedFilter : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Fill(std::vector<DataSink>& t_hist, int t_entry) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
    STEmbedTrack* embed_track_;
    int embed_id_;
};

class EmbedExistence : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class RecoTrackClusterNumFilter : public RecoTrackRule
{
public: 
    RecoTrackClusterNumFilter(const std::function<bool(int)>& t_compare = [](int t_tracks){return t_tracks > 5;}) : compare_(t_compare){};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::function<bool(int)> compare_;
};

class TrackShapeFilter : public RecoTrackRule
{
public:
    TrackShapeFilter(const std::string& t_cutfilename, double t_threshold);
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    inline TCutG GetCurrentCut() { if(cutg_) return TCutG(*cutg_); else return TCutG();}
protected:
    double threshold_;
    TFile cut_file_;
    TCutG* cutg_;
};

class PID : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override ;
};

class Observer : public RecoTrackRule
{
public:
	Observer(const std::string t_title = "") : title_(t_title) {};
        virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
	std::string title_;
};

class DrawTrack : public RecoTrackRule
{
public: 
    DrawTrack(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    const int x_, y_;
};

class CompareMCPrimary : public EmbedFilter
{
public:
    enum Type{MomX, MomY, MomZ, MMag, StartX, StartY, StartZ, StartMag, None};
    CompareMCPrimary(Type t_x, Type t_y);
    virtual void SetMyReader(TTreeReader& t_reader) override;
    inline void ChangeAxis(Type t_x, Type t_y) {x_ = t_x; y_ = t_y;};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    Type x_, y_;    
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class DistToVertex : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myVertexArray_;
};

class MomentumTracks : public RecoTrackRule
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MomentumTracks(int t_axis=0) : axis_(t_axis) {};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    inline void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
};

class RenshengCompareData : public RecoTrackRule
{
public:
    RenshengCompareData();
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
protected:
    ST_ClusterNum_DB db;
};

class ClusterNum : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
};

class ThetaPhi : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
};

class TrackIDRecorder : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<std::pair<int, int>> GetList() { return list_; };
protected:
    std::vector<std::pair<int, int>> list_;// entry num, track id
};

class MCThetaPhi : public EmbedFilter
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
};

class MCMomentumTracks : public EmbedFilter
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MCMomentumTracks(int t_axis=0) : axis_(t_axis) {};
    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override;
    inline void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
};

#endif

