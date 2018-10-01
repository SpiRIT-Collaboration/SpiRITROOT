#include "Rules.h"

/****************************
Rules: base abstract class for event cut and analysis
******************************/
void Rule::SetReader(TTreeReader& t_reader)
{
    this->SetMyReader(t_reader);
    if(NextRule_) NextRule_->SetReader(t_reader);
};

void Rule::Fill(DataSink& t_hist, unsigned t_entry)
{
    if(PreviousRule_) fill_ = PreviousRule_ -> fill_;
    this->Selection(t_hist, t_entry);
}


bool Rule::Repeated(unsigned t_entry) 
{ 
    if(t_entry == current_entry_) return true; 
    else 
    {
        current_entry_ = t_entry;
        return false;
    }
};

Rule* Rule::AddRule(Rule* t_rule)
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

void Rule::FillData(DataSink& t_hist, unsigned t_entry)
{
    if(NextRule_) NextRule_->Fill(t_hist, t_entry);
    else t_hist.push_back(fill_);
};

/****************************
ParallelRules: Rules that runs in parallel
Get data to obtain data from each step
With the design of DrawMultipleComplex, the use of this class is not recommended
****************************/

void ParallelRules::SetReader(TTreeReader& t_reader) 
{
    for(auto rule : rules_) rule->SetReader(t_reader);
};

void ParallelRules::Fill(DataSink& t_hist, unsigned t_entry) 
{
    data_.clear();
    for(auto rule : rules_)
    {
        t_hist.clear();
        rule->Fill(t_hist, t_entry);
        data_.push_back(t_hist);
    }
}

/******************************
RecoTrackRule
Base class that iterats through STRecoTracks
*******************************/
void RecoTrackRule::SetReader(TTreeReader& t_reader) 
{
    myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");
    this->SetMyReader(t_reader);
    if(NextRule_) NextRule_->SetReader(t_reader);
};

void RecoTrackRule::Fill(DataSink& t_hist, unsigned t_entry) 
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

/********************************
CheckPoint stores intermediate results
And allow user to retrieve it afterwards
*********************************/
void CheckPoint::Selection(DataSink& t_hist, unsigned t_entry) 
{
    temp_sink_.push_back(fill_);
    this->FillData(t_hist, t_entry);
}

DataSink CheckPoint::GetData() 
{ 
    auto sink = temp_sink_;
    temp_sink_.clear();
    return sink;
};

/***********************************
DrawMultipleComplex allows one to output intermediate data to more than 1 histograms
It works by creating checkpoints
Insert them in the right step and this class with read and fill them to histograms
***********************************/

DrawMultipleComplex::DrawMultipleComplex(const std::string& t_filenames, 
                                         const std::string& t_treename) : 
    chain_(t_treename.c_str()), 
    reader_(&chain_)
{ 
    chain_.Add(t_filenames.c_str()); 
}

DrawMultipleComplex::~DrawMultipleComplex()
{ 
    for(auto cp : checkpoints_) delete cp; 
}

CheckPoint* DrawMultipleComplex::NewCheckPoint()
{
    CheckPoint *cp = new CheckPoint;
    checkpoints_.push_back(cp);
    return checkpoints_.back();
}


/*********************************
Experimental drawer
where an iterator can be used
**********************************/

inline DataSink FillComplex::Iterator::operator*() const
{
    DataSink result;
    fcomplex_.rule_.Fill(result, *it_);
    return result;
}

inline bool FillComplex::Iterator::operator!=(const Iterator& rhs) const
{
    return this->it_ != rhs.it_;
}

inline FillComplex::Iterator & FillComplex::Iterator::operator++()
{
    it_++;
    return *this;
}

inline FillComplex::Iterator FillComplex::Iterator::operator++(int)
{
    return ++(*this);
}

FillComplex::FillComplex(const std::string& t_filenames, 
                         const std::string& t_treename,
                         Rule& t_rule) : rule_(t_rule),
                                     chain_(t_treename.c_str()),
                                     reader_(&chain_)
{
    chain_.Add(t_filenames.c_str());
    rule_.SetReader(reader_);
}

inline void FillComplex::ChangeRule(Rule& t_rule) 
{
    rule_ = t_rule; 
    reader_.Restart(); 
    rule_.SetReader(reader_);
};

/****************************
Observer: Pipe intermediate result to stdout
****************************/
void Observer::Selection(DataSink& t_hist, unsigned t_entry) 
{
        std::cout << " Momentum is " << fill_[1] << "\n";
        this->FillData(t_hist, t_entry);
}

/******************************
Draw trackes from STRecoTrack
********************************/
void DrawTrack::Selection(DataSink& t_hist, unsigned t_entry) 
{    
    for(const auto& point : (*track_->GetdEdxPointArray()))
    {
        fill_[0] = point.fPosition[x_];
        fill_[1] = point.fPosition[y_];
        this->FillData(t_hist, t_entry);
    }
}

/******************************
EmbedFilter
Only STRecoTracks corresponds to embeded track will be futher processed
******************************/
void EmbedFilter::SetMyReader(TTreeReader& t_reader)
{
    myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");
}

void EmbedFilter::Selection(DataSink& t_hist, unsigned t_entry)
{
    if((*myEmbedArray_)->GetEntries() == 0) return;
    for(int i = 0; i < (*myEmbedArray_)->GetEntries(); ++i)
    {
        auto id = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(0))->GetArrayID();
        if(id == track_id_) this->FillData(t_hist, t_entry);
    }
}


/************************************
MomentumTracks
Draw the momentum associated with each track
Set axis to plot different momentum direction
*************************************/
void MomentumTracks::Selection(DataSink& t_hist, unsigned t_entry) 
{
    auto mom = track_->GetMomentum();

    // return magnitude if axis_ = 3
    if(axis_ < 3) fill_[0] = mom[axis_];
    else fill_[0] = mom.Mag();

    this->FillData(t_hist, t_entry);
}

/************************************
DrawHit uses information from STHit class instead
Cannot iterate through each tracks
**************************************/
void DrawHit::SetMyReader(TTreeReader& t_reader) 
{
    myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STHit");
}

void DrawHit::Selection(DataSink& t_hist, unsigned t_entry) 
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

/*******************************
DrwaHitEmbed: similar but use info from STEmbedHit
*********************************/
void DrawHitEmbed::SetMyReader(TTreeReader& t_reader) 
{
    myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedHit");
}

void DrawHitEmbed::Selection(DataSink& t_hist, unsigned t_entry) 
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

/**********************************
RenshengCompareData
Compare track cluster numbers with Rensheng's database
*************************************/
RenshengCompareData::RenshengCompareData()
{    
    db.Initial_Config("/mnt/spirit/analysis/user/tsangc/Renshen/Momentum_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.config");
    db.ReadDB("/mnt/spirit/analysis/user/tsangc/Renshen/f1_tb_edge_ellipsoid_90layer_cut_clusternum_DB_theta90_phi180.root");
    double Momentum_Range_Plus[2] = {50,3000};
    double Momentum_Range_Minus[2] = {50,1000};
    db.Set_MomentumRange_Plus(Momentum_Range_Plus);
    db.Set_MomentumRange_Minus(Momentum_Range_Minus);
};

void RenshengCompareData::Selection(DataSink& t_hist, unsigned t_entry) 
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

/*********************
ValueCut : 1-D value cut on the previous rule
************************/
void ValueCut::Selection(DataSink& t_hist, unsigned t_entry) 
{
    if( upper_ > lower_)
    {
        if( upper_ >= fill_[0] && fill_[0] >= lower_) 
            this->FillData(t_hist, t_entry);
    }
    else this->FillData(t_hist, t_entry);
}
    
/***********************
EmbedCut: Graphical 2D cut
*************************/
EmbedCut::EmbedCut(const std::string& t_file) : file_(t_file.c_str()), cutg_(0)
{
    if(!file_.IsOpen()) std::cerr << "Cut File not found\n";
    cutg_ = (TCutG*) file_.Get("CUTG");
    if(!cutg_) std::cerr << "TCutG is not found in file!\n";
}

void EmbedCut::Selection(DataSink& t_hist, unsigned t_entry) 
{
    if(cutg_->IsInside(fill_[0], fill_[1])) 
        this->FillData(t_hist, t_entry);
}

/*********************
ThetaPhi : angle of the initial momentum of each track
*********************/
void ThetaPhi::Selection(DataSink& t_hist, unsigned t_entry) 
{
    //if(num_cluster < 20) continue;
    auto momVec = track_ -> GetMomentum();
    auto phiL = momVec.Phi()*180./TMath::Pi();
    auto thetaL = momVec.Theta()*180./TMath::Pi();

    fill_[0] = thetaL;
    fill_[1] = phiL;
    this->FillData(t_hist, t_entry);
}

/**************************
EntryRecorder: Save all entries that have been looped through
Useful for when you want to know which entries satisfy all conditions
***************************/
void EntryRecorder::Selection(DataSink& t_hist, unsigned t_entry) 
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


void EntryRecorder::ToFile(const std::string& t_filename)
{
    std::ofstream file(t_filename.c_str());
    for(const auto& i : list_)
        file << i << "\n";

    std::ofstream record(("Record_" + t_filename).c_str());
    record << "EventID\tExpNum\tDBNum\n";
    for(unsigned i = 0; i < event_id_.size(); ++i)
        record << event_id_[i] << "\t" << exp_num_[i] << "\t" << db_num_[i] << "\n";
};
