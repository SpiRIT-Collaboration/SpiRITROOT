#ifndef RULESSS_H
#define RULESSS_H

typedef std::vector<std::vector<double>> DataSink;
typedef TTreeReaderValue<TClonesArray> ReaderValue;

class Rule
{
public:
    Rule() : current_entry_(-1), fill_(2), NextRule_(0), PreviousRule_(0) {};
    virtual ~Rule(){};

    virtual void SetReader(TTreeReader& t_reader);
    virtual void SetMyReader(TTreeReader& t_reader){};
    virtual void Fill(DataSink& t_hist, unsigned t_entry);
    virtual void Selection(DataSink& t_hist, unsigned t_entry) = 0;

    bool Repeated(unsigned t_entry);
    Rule* AddRule(Rule* t_rule);
    std::vector<double> fill_;
protected:
    void FillData(DataSink& t_hist, unsigned t_entry);
    int current_entry_;
    Rule* NextRule_;
    Rule* PreviousRule_;
};

class ParallelRules : public Rule
{
public:
    virtual void SetReader(TTreeReader& t_reader) override;
    virtual void Fill(DataSink& t_hist, unsigned t_entry) override;
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override {};

    inline DataSink GetData(int t_i){ return data_[t_i]; };
    inline void AddParallelRule(Rule& t_rule){ rules_.push_back(&t_rule); };
protected:
    std::vector<Rule*> rules_;
    std::vector<DataSink> data_;
};

class RecoTrackRule : public Rule
{
public: 
    virtual void SetReader(TTreeReader& t_reader) override;
    virtual void Fill(DataSink& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    STRecoTrack* track_; 
    unsigned track_id_;
};

class EmbedFilter : public RecoTrackRule
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    std::shared_ptr<ReaderValue> myEmbedArray_;
};

class CheckPoint : public RecoTrackRule
{
public:
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override ;
    DataSink GetData();
protected:
    DataSink temp_sink_;
};

class DrawMultipleComplex
{
public:
    DrawMultipleComplex(const std::string& t_filenames, const std::string& t_treename);
    virtual ~DrawMultipleComplex();
    CheckPoint* NewCheckPoint();

    template<typename T, typename... ARGS>
    void DrawMultiple(Rule& t_rule, T& first_graph, ARGS&... args)
    {
        reader_.Restart();
        t_rule.SetReader(reader_);
        while( reader_.Next() )
        {
            DataSink result;
            t_rule.Fill(result, reader_.GetCurrentEntry());
        }
        FillHists(0, t_rule, first_graph, args...);
    }
    
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

    TChain chain_;
    TTreeReader reader_;
    std::vector<CheckPoint*> checkpoints_;
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
        T hist(args...);
        reader_.Restart();
        t_rule.SetReader(reader_);
        while( reader_.Next() )
        {
            DataSink result;
            t_rule.Fill(result, reader_.GetCurrentEntry());
            for(const auto& row : result) hist.Fill(row[0], row[1]);
        }
        return hist;
    }
protected:
    TChain chain_;
    TTreeReader reader_;
};

class FillComplex
{
protected:
    class Iterator : public std::iterator<std::output_iterator_tag, DataSink>
    {
    public:
        explicit Iterator(FillComplex& t_complex, 
                              TTreeReader::Iterator_t t_it) : fcomplex_(t_complex),
                                                          it_(t_it){};
        inline DataSink operator*() const;
        inline bool operator!=(const Iterator& rhs) const;
        inline Iterator & operator++();
        inline Iterator operator++(int);
    private:
        FillComplex& fcomplex_;
        TTreeReader::Iterator_t it_;
    };


    Rule& rule_;
    TChain chain_;
    TTreeReader reader_;
public:
    FillComplex(const std::string& t_filenames, 
                const std::string& t_treename,
                Rule& t_rule); 

    Iterator begin() { return Iterator(*this, reader_.begin()); };
    Iterator end() { return Iterator(*this, reader_.end());};

    template<class T, typename... ARGS>
    T Fill(ARGS... args)
    {
        T hist(args...);
        for(const auto& result : *this)
            for(const auto& row : result) hist.Fill(row[0], row[1]);
        return hist;
    }

    template<class T, typename... ARGS>
    T FillRule(Rule& t_rule, ARGS... args)
    {
        this->ChangeRule(t_rule);
        return this->Fill<T>(args...);
    }

    inline void ChangeRule(Rule& t_rule); 
    inline void Restart() { reader_.Restart();};
};


class Observer : public RecoTrackRule
{
public:
        virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
};

class DrawTrack : public RecoTrackRule
{
public: 
    DrawTrack(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
};

class MomentumTracks : public RecoTrackRule
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MomentumTracks(int t_axis=0) : axis_(t_axis) { fill_[1] = 1; };
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
    inline void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
};


class DrawHit : public Rule
{
public: 
    DrawHit(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};

class DrawHitEmbed : public Rule
{
public: 
    DrawHitEmbed(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override;
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};


class RenshengCompareData : public RecoTrackRule
{
public:
    RenshengCompareData();
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    ST_ClusterNum_DB db;
};

class ValueCut : public RecoTrackRule
{
public:
    ValueCut(double t_lower=0, double t_upper=0) : lower_(t_lower), upper_(t_upper){};
    inline void SetCut(double t_lower, double t_upper) { lower_ = t_lower; upper_ = t_upper; };
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
protected:
    double lower_, upper_;
};
    
class EmbedCut : public RecoTrackRule
{
public:
    EmbedCut() : cutg_(0) {};
    EmbedCut(const std::string& t_file);    
    inline void SetCut(TCutG* t_cutg) { cutg_ = t_cutg; };
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
    TCutG *GetCut() { return cutg_; };

protected:
    TFile file_;
    TCutG* cutg_;
};

class ThetaPhi : public RecoTrackRule
{
public:
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
};

class EntryRecorder : public RecoTrackRule
{
public:
    EntryRecorder() {};
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override;
    void ToFile(const std::string& t_filename);
    inline void Clear() { list_.clear(); };
    std::vector<int> GetList() { return list_; };
protected:
    std::vector<int> list_;
    std::vector<int> event_id_, db_num_, exp_num_;
};

#endif
