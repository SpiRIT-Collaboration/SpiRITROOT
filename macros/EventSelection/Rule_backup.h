#ifndef RULESSS_H
#define RULESSS_H

typedef std::vector<std::vector<double>> DataSink;

class Rule
{
public:
    Rule() : current_entry_(-1), fill_(2), NextRule_(0), PreviousRule_(0) {};
    virtual ~Rule(){};

    virtual void SetReader(TTreeReader& t_reader)
    {
        this->SetMyReader(t_reader);
        if(NextRule_) NextRule_->SetReader(t_reader);
    };
    
    virtual void SetMyReader(TTreeReader& t_reader){};
    virtual void Fill(DataSink& t_hist, unsigned t_entry)
    {
        if(PreviousRule_) fill_ = PreviousRule_ -> fill_;
        this->Selection(t_hist, t_entry);
    }

    virtual void Selection(DataSink& t_hist, unsigned t_entry) = 0;

    bool Repeated(unsigned t_entry) 
    { 
        if(t_entry == current_entry_) return true; 
        else 
        {
            current_entry_ = t_entry;
            return false;
        }
    };

    Rule* AddRule(Rule* t_rule)
    {
        // avoid self referencing
        if(this == t_rule)
            std::cerr << "Try to add the same rule twice. Will be ignored\n";
        else
        {
            if(NextRule_) NextRule_->AddRule(t_rule);
            else
            {
                NextRule_ = t_rule;
                t_rule->PreviousRule_ = this;
            }
        }
        return this;
    }

    std::vector<double> fill_;
protected:
    

    void FillData(DataSink& t_hist, unsigned t_entry)
    {
        if(NextRule_) NextRule_->Fill(t_hist, t_entry);
        else t_hist.push_back(fill_);
    };

    int current_entry_;
    Rule* NextRule_;
    Rule* PreviousRule_;
};

class ParallelRules : public Rule
{
public:
    virtual void SetReader(TTreeReader& t_reader) override
    {
        for(auto rule : rules_) rule->SetReader(t_reader);
    };
    
    virtual void Fill(DataSink& t_hist, unsigned t_entry) override
    {
        data_.clear();
        for(auto rule : rules_)
        {
            t_hist.clear();
            rule->Fill(t_hist, t_entry);
            data_.push_back(t_hist);
        }
    }

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override {};

    inline DataSink GetData(int t_i){ return data_[t_i]; };
    void AddParallelRule(Rule& t_rule){ rules_.push_back(&t_rule); };

protected:
    std::vector<Rule*> rules_;
    std::vector<DataSink> data_;
};

class RecoTrackRule : public Rule
{
public: 
    typedef TTreeReaderValue<TClonesArray> ReaderValue;

    virtual void SetReader(TTreeReader& t_reader) override
    {
        myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");
        this->SetMyReader(t_reader);
        if(NextRule_) NextRule_->SetReader(t_reader);
    };

    virtual void Fill(DataSink& t_hist, unsigned t_entry) override
    {
        if(PreviousRule_) 
        {
            fill_ = PreviousRule_->fill_;
            if(auto prule = dynamic_cast<RecoTrackRule*>(PreviousRule_))
            {
                track_ = prule->track_;
                track_id_ = prule->track_id_;
                this->Selection(t_hist, t_entry);
                return;
            }
        }

        // if previous rule doesn't start looping on recotrack, it will initialize it
        for(track_id_ = 0; track_id_ < (*myTrackArray_)->GetEntries(); ++track_id_)
        {
            track_ = static_cast<STRecoTrack*>((*myTrackArray_) -> At(track_id_));
            this->Selection(t_hist, t_entry);
        }
    }

protected:
    std::shared_ptr<ReaderValue> myTrackArray_;
    STRecoTrack* track_; 
    unsigned track_id_;
};

class CheckPoint : public RecoTrackRule
{
public:
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override 
    {
        temp_sink_.push_back(fill_);
        this->FillData(t_hist, t_entry);
    }
    
    DataSink GetData() 
    { 
        auto sink = temp_sink_;
        temp_sink_.clear();
        return sink;
    };
protected:
    DataSink temp_sink_;
};

class DrawMultipleComplex
{
public:
    DrawMultipleComplex(const std::string& t_filenames, const std::string& t_treename) : chain_(t_treename.c_str()), reader_(&chain_)
    { chain_.Add(t_filenames.c_str()); }

    virtual ~DrawMultipleComplex()
    { for(auto cp : checkpoints_) delete cp; }

    CheckPoint* NewCheckPoint()
    {
        CheckPoint *cp = new CheckPoint;
        checkpoints_.push_back(cp);
        return checkpoints_.back();
    }

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
        inline DataSink operator*() const
        {
            DataSink result;
            fcomplex_.rule_.Fill(result, *it_);
            return result;
        }
    
        inline bool operator!=(const Iterator& rhs) const
        {
            return this->it_ != rhs.it_;
        }
    
        inline Iterator & operator++()
        {
            it_++;
            return *this;
        }
        inline Iterator operator++(int)
        {
            return ++(*this);
        }
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
                Rule& t_rule) : rule_(t_rule),
                            chain_(t_treename.c_str()),
                            reader_(&chain_)
    {
        chain_.Add(t_filenames.c_str());
        rule_.SetReader(reader_);
    }

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

    inline void ChangeRule(Rule& t_rule) 
    {
        rule_ = t_rule; 
        reader_.Restart(); 
        rule_.SetReader(reader_);
    };
    inline void Restart() { reader_.Restart();};
};




class DrawDiff
{
public:
    DrawDiff(const std::string& t_filenames1,
                 const std::string& t_treename1,
                 const std::string& t_filenames2,
                 const std::string& t_treename2) :
                 chain1_(t_treename1.c_str()),
                 chain2_(t_treename2.c_str()),
                 reader1_(&chain1_),
                 reader2_(&chain2_)
    {
        chain1_.Add(t_filenames1.c_str());
        chain2_.Add(t_filenames2.c_str());
    }

    template<class M>
    void FillRules(Rule& t_rule1, Rule& t_rule2, M t_metric, TH1& t_hist)
    {
        reader1_.Restart();
        reader2_.Restart();
        t_rule1.SetReader(reader1_);
        t_rule2.SetReader(reader2_);
        while( reader1_.Next() && reader2_.Next() )
        {
            DataSink result1, result2;
            t_rule1.Fill(result1, reader1_.GetCurrentEntry());
            t_rule2.Fill(result2, reader2_.GetCurrentEntry());
            auto result = t_metric(result1, result2);
            for(const auto& row : result) t_hist.Fill(row[0], row[1]);
        }
    }

protected:
    TChain chain1_, chain2_;
    TTreeReader reader1_, reader2_;
};

class Observer : public RecoTrackRule
{
public:
        virtual void Selection(DataSink& t_hist, unsigned t_entry) override
        {
                std::cout << " Momentum is " << fill_[1] << "\n";
                this->FillData(t_hist, t_entry);
        }
};

class DrawTrack : public RecoTrackRule
{
public: 
    DrawTrack(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {    
        for(const auto& point : (*track_->GetdEdxPointArray()))
        {
            fill_[0] = point.fPosition[x_];
            fill_[1] = point.fPosition[y_];
            this->FillData(t_hist, t_entry);
        }
    }

protected:
    const int x_, y_;
};

class DrawTrackEmbed : public RecoTrackRule
{
public: 
    DrawTrackEmbed(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};
    virtual void SetMyReader(TTreeReader& t_reader) override
    {myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");}

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        if((*myEmbedArray_)->GetEntries() == 0) return;
        auto id = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(0))->GetArrayID();    
        if(id == track_id_)
        {
            for(const auto& point : (*track_->GetdEdxPointArray()))
            {
                fill_[0] = point.fPosition[x_];
                fill_[1] = point.fPosition[y_];
                this->FillData(t_hist, t_entry);
            }

        }
    }
protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myEmbedArray_;
};


class MomentumTracks : public RecoTrackRule
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MomentumTracks(int t_axis=0) : axis_(t_axis) { fill_[1] = 1; };

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        auto mom = track_->GetMomentum();

        // return magnitude if axis_ = 3
        if(axis_ < 3) fill_[0] = mom[axis_];
        else fill_[0] = mom.Mag();

        this->FillData(t_hist, t_entry);
    }

    void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
};

class MomentumEmbedTracks : public RecoTrackRule
{
public:
    // which axis x y z or magnitude of the momentum do you want to plot
    MomentumEmbedTracks(int t_axis=0) : axis_(t_axis) { fill_[1] = 1; };

    virtual void SetMyReader(TTreeReader& t_reader) override
    {myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");}

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        if((*myEmbedArray_)->GetEntries() == 0) return;
        auto id = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(0))->GetArrayID();    
        if(id == track_id_)
        {
            auto mom = track_->GetMomentum();
            if(axis_ < 3) fill_[0] = mom[axis_];
            else fill_[0] = mom.Mag();

            this->FillData(t_hist, t_entry);
        }
    }

    void SetAxis(int t_axis) { axis_ = t_axis; };
protected:
    int axis_;
    std::shared_ptr<ReaderValue> myTrackArray_, myEmbedArray_;
};



class DrawHit : public Rule
{
public: 
    typedef TTreeReaderValue<TClonesArray> ReaderValue;

    DrawHit(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};

    virtual void SetMyReader(TTreeReader& t_reader) override
    {myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STHit");}

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {    
        if(this->Repeated(t_entry)) return;
        for(int j = 0; j < (*myHitArray_)->GetEntries(); ++j)
        {
            auto hit = static_cast<STHit*>((*myHitArray_) -> At(j));
            auto pos = hit->GetPosition();
            fill_[0] = pos[x_];
            fill_[1] = pos[y_];
            this->FillData(t_hist, t_entry);
        }
    }

protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};

class DrawHitEmbed : public Rule
{
public: 
    typedef TTreeReaderValue<TClonesArray> ReaderValue;

    DrawHitEmbed(int t_x=0, int t_y=2) : x_(t_x), y_(t_y) {};

    virtual void SetMyReader(TTreeReader& t_reader) override
    {myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedHit");}

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {    
        if(this->Repeated(t_entry)) return;
        for(int j = 0; j < (*myHitArray_)->GetEntries(); ++j)
        {
            auto hit = static_cast<STHit*>((*myHitArray_) -> At(j));
            auto pos = hit->GetPosition();
            fill_[0] = pos[x_];
            fill_[1] = pos[y_];
            this->FillData(t_hist, t_entry);
        }
    }

protected:
    const int x_, y_;
    std::shared_ptr<ReaderValue> myHitArray_;
};


class RenshengCompareEmbed : public RecoTrackRule
{
public:
    typedef TTreeReaderValue<TClonesArray> ReaderValue;
    RenshengCompareEmbed()
    {    
        db.Initial_Config("/mnt/spirit/analysis/user/tsangc/Renshen/Momentum_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.config");
        db.ReadDB("/mnt/spirit/analysis/user/tsangc/Renshen/f1_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.root");
        double Momentum_Range_Plus[2] = {50,3000};
        double Momentum_Range_Minus[2] = {50,1000};
        db.Set_MomentumRange_Plus(Momentum_Range_Plus);
        db.Set_MomentumRange_Minus(Momentum_Range_Minus);
    };

    virtual void SetMyReader(TTreeReader& t_reader) override
    {myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");};

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        for(int i = 0; i < (*myEmbedArray_)->GetEntries(); ++i)
        {
            auto embed = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(i));
            auto id = embed->GetArrayID();
            if(id == track_id_)
            {
                int embed_num_cluster = embed->GetNumLayerClusters90() + embed->GetNumRowClusters90();
                auto momVec = embed -> GetFinalMom();
                
                auto mom = momVec.Mag();
                momVec = momVec.Unit();
                auto phiL = momVec.Phi()*180./TMath::Pi();
                phiL = (phiL < 0 ? phiL + 360 : phiL);
                
                auto thetaL = momVec.Theta()*180./TMath::Pi();
                //for 90 layers
                int db_num_cluster = db.GetClusterNum(1., thetaL, (phiL > 180 ? phiL - 360 : phiL), mom);

                fill_[0] = (double) embed_num_cluster;
                fill_[1] =  (double) db_num_cluster;

                this->FillData(t_hist, t_entry);
            }
        }
    };

protected:
    ST_ClusterNum_DB db;
    std::shared_ptr<ReaderValue> myEmbedArray_;
    
};

class RenshengCompareData : public RenshengCompareEmbed
{
public:
    virtual void SetMyReader(TTreeReader& t_reader) override
    {myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");}

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {

        // find number of tracks for the embeded pions
        auto nrc90 = track_->GetNumRowClusters90();
        auto nlc90 = track_->GetNumLayerClusters90();
        int num_cluster = nrc90 + nlc90;
        //if(num_cluster < 20) continue;

        auto mom = track_ -> GetMomentum().Mag();
        auto charge = track_ -> GetCharge();
        auto momVec = track_ -> GetMomentum();
        
        momVec = momVec.Unit();
        auto phiL = momVec.Phi()*180./TMath::Pi();
        phiL = (phiL < 0 ? phiL + 360 : phiL);
        
        auto thetaL = momVec.Theta()*180./TMath::Pi();
        
        //for 90 layers
        int db_num_cluster = db.GetClusterNum(charge, thetaL, (phiL > 180 ? phiL - 360 : phiL), mom);

        fill_[0] = (double) num_cluster;
        fill_[1] = (double) db_num_cluster;

        this->FillData(t_hist, t_entry);
    };
};    

class ValueCut : public RecoTrackRule
{
public:
    ValueCut(double t_lower=0, double t_upper=0) : lower_(t_lower), upper_(t_upper)
    {}
    
    void SetCut(double t_lower, double t_upper) { lower_ = t_lower; upper_ = t_upper; };

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        if( upper_ > lower_)
        {
            if( upper_ >= fill_[0] && fill_[0] >= lower_) 
                this->FillData(t_hist, t_entry);
        }
        else this->FillData(t_hist, t_entry);
    }
protected:
    double lower_, upper_;
};
    
class EmbedCut : public RecoTrackRule
{
public:
    EmbedCut() : cutg_(0) {};
    EmbedCut(const std::string& t_file) : file_(t_file.c_str()), cutg_(0)
    {
        if(!file_.IsOpen()) std::cerr << "Cut File not found\n";
        cutg_ = (TCutG*) file_.Get("CUTG");
        if(!cutg_) std::cerr << "TCutG is not found in file!\n";
    }
    
    void SetCut(TCutG* t_cutg) { cutg_ = t_cutg; };

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        if(cutg_->IsInside(fill_[0], fill_[1])) 
            this->FillData(t_hist, t_entry);
    }

    TCutG *GetCut() { return cutg_; };

protected:
    TFile file_;
    TCutG* cutg_;
};

class ThetaPhi : public RecoTrackRule
{
public:
    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        //if(num_cluster < 20) continue;
        auto momVec = track_ -> GetMomentum();
        auto phiL = momVec.Phi()*180./TMath::Pi();
        auto thetaL = momVec.Theta()*180./TMath::Pi();

        fill_[0] = thetaL;
        fill_[1] = phiL;
        this->FillData(t_hist, t_entry);
    }
};

class EntryRecorder : public RecoTrackRule
{
public:
    EntryRecorder() {};

    virtual void Selection(DataSink& t_hist, unsigned t_entry) override
    {
        // do not fill repeated entry number
        if(list_.size() == 0) list_.push_back(t_entry);
        else if(t_entry != list_.back()) list_.push_back(t_entry);

        if(NextRule_) NextRule_->Fill(t_hist, t_entry);
        else t_hist.push_back(fill_);

        // it is a new event if the entry number doesn't match
        event_id_.push_back(list_.size() - 1);
        db_num_.push_back((int) (fill_[1] + 0.5));
        exp_num_.push_back((int) (fill_[0] + 0.5));
    };


    void ToFile(const std::string& t_filename)
    {
        std::ofstream file(t_filename.c_str());
        for(const auto& i : list_)
            file << i << "\n";

        std::ofstream record(("Record_" + t_filename).c_str());
        record << "EventID\tExpNum\tDBNum\n";
        for(unsigned i = 0; i < event_id_.size(); ++i)
            record << event_id_[i] << "\t" << exp_num_[i] << "\t" << db_num_[i] << "\n";
    };

    void Clear() { list_.clear(); };
    std::vector<int> GetList() { return list_; };

protected:
    std::vector<int> list_;
    std::vector<int> event_id_, db_num_, exp_num_;
};

#endif
/*
void MCClusterNum(const std::string& t_cut_file = "")
{
    std::cout << "start" << std::endl;
    TH2F hist_real("num_cluster_real_data", "num_cluster", 150, 0, 150, 150, 0, 150);

    //RenshengCompareEmbed rc_embed;
    RenshengCompareData rc_data;
    EmbedCut cut("CUTG.root");
    EntryRecorder recorder;

    rc_data.AddRule(&cut);
    rc_data.AddRule(&recorder);
    DrawComplex drawer("mc0_s0.reco.v1.04.root", "cbmsim");

    drawer.FillRule(rc_data, hist_real);

    TCanvas c2;
    c2.SetBottomMargin(0.15);
    c2.SetLeftMargin(0.15);
    ImproveGraph(&hist_real, "Number of clusters for real data", "Number of DB clusters");
    hist_real.Draw("colz");

    while(true) gPad->WaitPrimitive();
}

*/
