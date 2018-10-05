#ifndef RULESSS_H
#define RULESSS_H
#include "HistToCutG.h"

typedef std::vector<std::vector<double>> DataSink;
typedef TTreeReaderValue<TClonesArray> ReaderValue;

class Rule
{
public:
    Rule() : current_entry_(-1), NextRule_(0), PreviousRule_(0) {};
    virtual ~Rule(){};

    virtual void SetReader(TTreeReader& t_reader);
    virtual void SetMyReader(TTreeReader& t_reader){};
    virtual void Fill(std::vector<DataSink>& t_hist, unsigned t_entry);
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) = 0;

    bool Repeated(unsigned t_entry);
    virtual Rule* AddRule(Rule* t_rule);
    inline void AppendRule(Rule* t_rule) { if(NextRule_) NextRule_->AppendRule(t_rule); else this->AddRule(t_rule);};
protected:
    void FillData(std::vector<DataSink>& t_hist, unsigned t_entry);
    int current_entry_;
    Rule* NextRule_;
    Rule* PreviousRule_;
};

class ParallelRules : public Rule
{
public:
    virtual void SetReader(TTreeReader& t_reader) override;
    virtual void Fill(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override {};

    inline DataSink GetData(int t_i){ return data_[t_i].back(); };
    inline void AddParallelRule(Rule& t_rule){ rules_.push_back(&t_rule); };
protected:
    std::vector<Rule*> rules_;
    std::vector<std::vector<DataSink>> data_;
};

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

class RecoTrackNumFilter : public Rule
{
public: 
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
};

class EmbedFilter : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class TrackShapeFilter : public RecoTrackRule
{
public:
    TrackShapeFilter(const std::string& t_cutfilename, double t_threshold);
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    double threshold_;
    TFile cut_file_;
};

class CheckPoint : public RecoTrackRule
{
public:
    CheckPoint() {can_init_loop_ = false;};
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
    virtual ~DrawMultipleComplex();
    CheckPoint* NewCheckPoint();

    template<typename T, typename... ARGS>
    void DrawMultiple(Rule& t_rule, T& first_graph, ARGS&... args)
    {
        reader_.Restart();
        t_rule.SetReader(reader_);
        while( reader_.Next() )
        {
            std::vector<DataSink> result;
            t_rule.Fill(result, reader_.GetCurrentEntry());
        }
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

    class Iterator : public std::iterator<std::output_iterator_tag, DataSink>
    {
    public:
        explicit Iterator(DrawMultipleComplex& t_complex, 
                              TTreeReader::Iterator_t t_it) : fcomplex_(t_complex),
                                                          it_(t_it){};
        inline std::vector<DataSink> operator*() const;
        inline bool operator!=(const Iterator& rhs) const;
        inline Iterator & operator++();
        inline Iterator operator++(int);
    private:
        DrawMultipleComplex& fcomplex_;
        TTreeReader::Iterator_t it_;
    };

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
        return hist;
    }
protected:
    TChain chain_;
    TTreeReader reader_;
};



class Observer : public RecoTrackRule
{
public:
        virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
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

class ValueCut : public RecoTrackRule
{
public:
    ValueCut(double t_lower=0, double t_upper=0) : lower_(t_lower), upper_(t_upper){};
    inline void SetCut(double t_lower, double t_upper) { lower_ = t_lower; upper_ = t_upper; };
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
protected:
    double lower_, upper_;
};
    
class EmbedCut : public RecoTrackRule
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

class EntryRecorder : public RecoTrackRule
{
public:
    EntryRecorder() {};
    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<int> GetList() { return list_; };
protected:
    std::vector<int> list_;
    std::vector<int> event_id_, db_num_, exp_num_;
};

#endif
