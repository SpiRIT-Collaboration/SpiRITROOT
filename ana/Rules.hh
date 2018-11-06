#ifndef RULESSS_H
#define RULESSS_H
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
#include "ST_ClusterNum_DB.hh"

typedef std::vector<std::vector<double>> DataSink;
typedef TTreeReaderValue<TClonesArray> ReaderValue;

class Rule
{
public:
    friend class RecoTrackRule;
    friend std::pair<Rule*, Rule*> RuleBlock(Rule* t_rule);
    Rule() : NextRule_(nullptr), PreviousRule_(nullptr), RejectRule_(nullptr) {};
    virtual ~Rule(){};

    virtual void SetReader(TTreeReader& t_reader); // unless you know what you are doing, dont override SetReader
    virtual void SetMyReader(TTreeReader& t_reader){}; // only override SetMyReader such that all inheriented class will set parent's reader
    virtual void Fill(std::vector<DataSink>& t_hist, unsigned t_entry);
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) = 0;

    virtual Rule* AddRule(Rule* t_rule);
    virtual Rule* AddRejectRule(Rule* t_rule);
    inline void AppendRule(Rule* t_rule) { if(NextRule_) NextRule_->AppendRule(t_rule); else this->AddRule(t_rule);};
    inline void PopRule() { if(NextRule_) NextRule_->PopRule(); else {this->PreviousRule_->NextRule_ = nullptr; this->PreviousRule_ = nullptr;}};
protected:
    inline void FillData(std::vector<DataSink>& t_hist, unsigned t_entry) {if(NextRule_) NextRule_->Fill(t_hist, t_entry);};
    inline void RejectData(std::vector<DataSink>& t_hist, unsigned t_entry) {if(RejectRule_) RejectRule_->Fill(t_hist, t_entry);};
    Rule* NextRule_;
    Rule* RejectRule_;
    Rule* PreviousRule_;
};

std::pair<Rule*, Rule*> RuleBlock(Rule* t_rule);


class RecoTrackRule : public Rule
{
public: 
    RecoTrackRule() : can_init_loop_(true) {};
    virtual void SetReader(TTreeReader& t_reader) override;
    virtual void Fill(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    STRecoTrack* track_; 
    unsigned track_id_;
    bool can_init_loop_;
};

class EmptyRule : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override {this->FillData(t_hist, t_entry);};
};

/*class RuleBlock : public Rule
{
public:
    RuleBlock() : InternalFirst_(0), InternalLast_(0){};
    RuleBlock(Rule* t_rule){this->AddBlockRule(t_rule);};
    virtual void SetReader(TTreeReader& t_reader) override;
    void AddBlockRule(Rule* t_rule);
    virtual void Fill(std::vector<DataSink>& t_hist, unsigned t_entry) override {InternalFirst_->Fill(t_hist, t_entry);};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override {}; // actually it doesnlty matter as Selection is only called by FillData, which is in turn called by Fill. We overwrite our Fill Method so it won't be called at any given time.
    virtual Rule* AddRule(Rule* t_rule) override;
    virtual Rule* AddRejectRule(Rule* t_rule) override;
protected:
    Rule* InternalFirst_;
    Rule* InternalLast_;
};*/

class RecoTrackNumFilter : public Rule
{
public: 
    RecoTrackNumFilter(const std::function<bool(int)>& t_compare = [](int t_tracks){return t_tracks < 2;}) : compare_(t_compare){};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    std::function<bool(int)> compare_;
};

class EmbedFilter : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class EmbedExistence : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class RecoTrackClusterNumFilter : public RecoTrackRule
{
public: 
    RecoTrackClusterNumFilter(const std::function<bool(int)>& t_compare = [](int t_tracks){return t_tracks > 5;}) : compare_(t_compare){};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::function<bool(int)> compare_;
};

class TrackShapeFilter : public RecoTrackRule
{
public:
    TrackShapeFilter(const std::string& t_cutfilename, double t_threshold);
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    inline TCutG GetCurrentCut() { if(cutg_) return TCutG(*cutg_); else return TCutG();}
protected:
    double threshold_;
    TFile cut_file_;
    TCutG* cutg_;
};

class CheckPoint : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override ;
    DataSink GetData();
protected:
    DataSink temp_sink_;
};

class PID : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override ;
};

class DrawMultipleComplex
{
public:
    DrawMultipleComplex(const std::string& t_filenames, const std::string& t_treename, Rule* t_rule=0);
    DrawMultipleComplex(TChain *t_chain, Rule* t_rule=0);
    virtual ~DrawMultipleComplex();
    CheckPoint* NewCheckPoint();
    std::vector<CheckPoint*> NewCheckPoints(int t_num);

    template<typename T, typename... ARGS>
    void DrawMultiple(Rule& t_rule, T& first_graph, ARGS&... args)
    {
        reader_.Restart();
        t_rule.SetReader(reader_);
        if(reader_.GetEntries(true) == 0)
        {
            std::cerr << "No entries from reader. Failed to load tree from file. Will ignore\n";
            return;
        }
        while( reader_.Next() )
        {
            std::vector<DataSink> result;
            t_rule.Fill(result, reader_.GetCurrentEntry());
            std::cout << "Processing Entry " << reader_.GetCurrentEntry() << "\t\r";
        }
        std::cout << "\n";
        FillHists(0, t_rule, first_graph, args...);
    }

    template<typename T, typename... ARGS>
    void DrawMultiple(const std::vector<int>& t_entry_list, Rule& t_rule, T& first_graph, ARGS&... args)
    {
        reader_.Restart();
        t_rule.SetReader(reader_);
        for(const auto& entry : t_entry_list)
        {
            reader_.SetEntry(entry);
            std::vector<DataSink> result;
            t_rule.Fill(result, entry);
            std::cout << "Processing Entry " << reader_.GetCurrentEntry() << "\t\r";
        }
        std::cout << "\n";
        FillHists(0, t_rule, first_graph, args...);
    }

    void SetRule(Rule* t_rule){rule_ = t_rule; reader_.Restart(); rule_->SetReader(reader_);};
    
protected:
    template<typename T, typename... ARGS>
    void FillHists(int t_ncp, Rule& t_rule, T& first_graph, ARGS&... args)
    {
        auto data = checkpoints_[t_ncp]->GetData();
        for(const auto& row : data) first_graph.Fill(row[0], row[1]);
        FillHists(t_ncp + 1, t_rule, args...);
    }

    template<typename T>
    void FillHists(int t_ncp, Rule& t_rule, T& graph)
    {
        auto data = checkpoints_[t_ncp]->GetData();
        for(const auto& row : data) graph.Fill(row[0], row[1]);
    }

public:
    class Iterator : public std::iterator<std::output_iterator_tag, DataSink>
    {
    public:
        explicit Iterator(DrawMultipleComplex& t_complex, 
                              TTreeReader::Iterator_t t_it) : fcomplex_(t_complex),
                                                          it_(t_it){};
        std::vector<DataSink> operator*() const;
        inline bool operator!=(const Iterator& rhs) const {return this->it_ != rhs.it_;};
        inline Iterator & operator++(){ it_++; return *this;};
        inline Iterator operator++(int){ return ++(*this);};
        inline int GetCurrentEntry() { return fcomplex_.reader_.GetCurrentEntry(); };

    private:
        DrawMultipleComplex& fcomplex_;
        TTreeReader::Iterator_t it_;
    };
protected:
    Rule* rule_;
    TChain chain_;
    TTreeReader reader_;
    std::vector<CheckPoint*> checkpoints_;
public:
    Iterator begin() { reader_.Restart(); return Iterator(*this, reader_.begin()); };
    Iterator end() { return Iterator(*this, reader_.end());};
};


class DrawComplex
{
public:
    DrawComplex(const std::string& t_filenames, 
                const std::string& t_treename) :
        chain_(t_treename.c_str()),
        reader_(&chain_)
    {
        chain_.Add(t_filenames.c_str());
    }

    template<class T, typename... ARGS>
    T FillRule(Rule& t_rule, ARGS... args)
    {
        CheckPoint cp;
        t_rule.AppendRule(&cp);
        T hist(args...);
        reader_.Restart();
        t_rule.SetReader(reader_);
        unsigned index = 0;
        while( reader_.Next() )
        {
            std::vector<DataSink> result;
            t_rule.Fill(result, index);
            for(const auto& row : cp.GetData()) hist.Fill(row[0], row[1]);
            std::cout << "Processing Entry " << index << "\t\r";
            ++index;
        }
        t_rule.PopRule();
        std::cout << "\n";
        return hist;
    }

    template<class T, typename... ARGS>
    T FillRule(const std::vector<int>& t_entry_list, Rule& t_rule, ARGS... args)
    {
        CheckPoint cp;
        t_rule.AppendRule(&cp);
        T hist(args...);
        reader_.Restart();
        t_rule.SetReader(reader_);
        for(const auto& entry : t_entry_list)
        {
            reader_.SetEntry(entry);
            std::vector<DataSink> result;
            t_rule.Fill(result, entry);
            for(const auto& row : cp.GetData()) hist.Fill(row[0], row[1]);
            std::cout << "Processing Entry " << entry << "\t\r";
        }
        t_rule.PopRule();
        std::cout << "\n";
        return hist;
    }
protected:
    TChain chain_;
    TTreeReader reader_;
};



class Observer : public RecoTrackRule
{
public:
	Observer(const std::string t_title = "") : title_(t_title) {};
        virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
	std::string title_;
};

class DrawTrack : public RecoTrackRule
{
public: 
    DrawTrack(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
};

class CompareMCPrimary : public RecoTrackRule
{
public:
    enum Type{MomX, MomY, MomZ, MMag, StartX, StartY, StartZ, StartMag, None};
    CompareMCPrimary(const std::string& t_filename, Type t_x, Type t_y);
    inline void ChangeAxis(Type t_x, Type t_y) {x_ = t_x; y_ = t_y;};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    Type x_, y_;    
    std::vector<double> Px_, Py_, Pz_, X_, Y_, Z_;
};

class MomentumTracks : public RecoTrackRule
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MomentumTracks(int t_axis=0) : axis_(t_axis) {};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    inline void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
};


class DrawHit : public Rule
{
public: 
    DrawHit(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};

// this draws output of the previous rules
// will only work if it is DrawHit (or DrawTrack in the future). To do list
// Can only work with x-z or y-z plane. To do list for other dimensions
class GetHitOutline : public Rule
{
public: 
    GetHitOutline(const std::string& t_outputname);
    virtual ~GetHitOutline();
    virtual Rule* AddRule(Rule* t_rule) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
private:
    const int pad_x = 108;              
    const int pad_y = 112;
    const double size_x = 8;
    const double size_y = 12;
    const int max_num_ = 10000; // maximum number of cuts to be stored
    TFile file_;
    TClonesArray cutg_array_;                        // to do list: by pass this number
};

class DrawHitEmbed : public Rule
{
public: 
    DrawHitEmbed(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};


class RenshengCompareData : public RecoTrackRule
{
public:
    RenshengCompareData();
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    ST_ClusterNum_DB db;
};

class ClusterNum : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
};

class ValueCut : public Rule
{
public:
    ValueCut(double t_lower=0, double t_upper=0, bool t_yaxis=false) : lower_(t_lower), upper_(t_upper)
    { index_ = (t_yaxis)? 1 : 0;};
    inline void SetCut(double t_lower, double t_upper) { lower_ = t_lower; upper_ = t_upper; };
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    double lower_, upper_;
    int index_;
};

    
class EmbedCut : public Rule
{
public:
    EmbedCut() : cutg_(0) {};
    EmbedCut(const std::string& t_file, const std::string& t_cutname = "CUTG");    
    inline void SetCut(TCutG* t_cutg) { cutg_ = t_cutg; };
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    TCutG *GetCut() { return cutg_; };

protected:
    TFile file_;
    TCutG* cutg_;
};

class ThetaPhi : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
};

class EntryRecorder : public Rule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<int> GetList() { return list_; };
protected:
    std::vector<int> list_;
};

class TrackIDRecorder : public RecoTrackRule
{
public:
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<std::pair<int, int>> GetList() { return list_; };
protected:
    std::vector<std::pair<int, int>> list_;// entry num, track id
};

class TrackZFilter : public Rule
{
public:
    void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
};

#endif

