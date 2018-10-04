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
    else 
        t_hist.push_back(fill_);
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
    if(can_init_loop_)
    {
    	for(track_id_ = 0; track_id_ < (*myTrackArray_)->GetEntries(); ++track_id_)
    	{
    	    track_ = static_cast<STRecoTrack*>((*myTrackArray_) -> At(track_id_));
    	    this->Selection(t_hist, t_entry);
    	}
    }
    else this->Selection(t_hist, t_entry);
}


/******************************
PID on tracks
******************************/
void PID::Selection(DataSink& t_hist, unsigned t_entry)
{
    // find the average dE
    fill_[1] = track_->GetdEdxWithCut(0, 0.7);
    fill_[0] = track_->GetMomentum().Mag();
    this->FillData(t_hist, t_entry);
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
                                         const std::string& t_treename,
                                         Rule* t_rule) : 
    chain_(t_treename.c_str()), 
    reader_(&chain_),
    rule_(t_rule)
{ 
    chain_.Add(t_filenames.c_str()); 
    if(t_rule) t_rule->SetReader(reader_);
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

/********************************
Iterator for draw multiple complex
********************************/
inline std::vector<DataSink> DrawMultipleComplex::Iterator::operator*() const
{
    DataSink temp;
    if(!fcomplex_.rule_) std::cerr << "Rule is not set. Cannot use iterator\n";
    cout << "Start Fill\n";
    fcomplex_.rule_->Fill(temp, *it_);
    cout << "End Fill\n";

    cout << "Start fill datalist\n";
    std::vector<DataSink> datalist;
    for(auto cp : fcomplex_.checkpoints_)
    {
        datalist.push_back(cp->GetData());
        cout << datalist.back().size() << "\n";
    }
    cout << "Finished datalist\n";
    return datalist;
}

inline bool DrawMultipleComplex::Iterator::operator!=(const Iterator& rhs) const
{
    return this->it_ != rhs.it_;
}

inline DrawMultipleComplex::Iterator & DrawMultipleComplex::Iterator::operator++()
{
    it_++;
    return *this;
}

inline DrawMultipleComplex::Iterator DrawMultipleComplex::Iterator::operator++(int)
{
    return ++(*this);
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
        auto id = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(i))->GetArrayID();
        if(id == track_id_) this->FillData(t_hist, t_entry);
    }
}

/*************************************
CompareMCPrimary
Compare momentum distribution event by event to momentum layout by the give file
Should only be used together with embed filter
*************************************/
CompareMCPrimary::CompareMCPrimary(const std::string& t_filename, 
                                   CompareMCPrimary::Type t_x, 
                                   CompareMCPrimary::Type t_y) :
    x_(t_x),
    y_(t_y)
{
    std::ifstream file(t_filename.c_str());
    if(!file.is_open()) std::cerr << "File " << t_filename << " cannot be opened\n";
    
    int index;
    double px, py, pz, x, y, z;
    const double GeVToMeV = 1000;
    const double cmTomm = 10;
    // get rid of the header
    std::string line;
    std::getline(file, line);
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        if(!(ss >> index >> px >> py >> pz >> x >> y >> z))
        {
            std::cerr << "Error: cannot read line " << line << "\n";
            continue;
        }
        Px_.push_back(GeVToMeV*px);
        Py_.push_back(GeVToMeV*py);
        Pz_.push_back(GeVToMeV*pz);
        X_.push_back(cmTomm*x);
        Y_.push_back(cmTomm*y);
        Z_.push_back(cmTomm*z);
    }
}

void CompareMCPrimary::Selection(DataSink& t_hist, unsigned t_entry)
{
    if(t_entry > Px_.size())
    {
        std::cerr << "Entries in tree exceed number of entries in the primary text file\n";
        return;
    }

    auto reco_mom = track_->GetMomentum();
    auto reco_Vert = track_->GetPOCAVertex();

    // fill x and y respectively
    // that's why the range is 2
    for(int i = 0; i < 2; ++i)
    {
        auto type = (i==0)? x_ : y_;
        switch(type)
        {
            case MomX: fill_[i] = reco_mom[0] - Px_[t_entry]; break; 
            case MomY: fill_[i] = reco_mom[1] - Py_[t_entry]; break;
            case MomZ: fill_[i] = reco_mom[2] - Pz_[t_entry]; break;
            case MMag: {
                double mag = sqrt(Px_[t_entry]*Px_[t_entry] 
                                  + Py_[t_entry]*Py_[t_entry] 
                                  + Pz_[t_entry]*Pz_[t_entry]);
		fill_[i] = reco_mom.Mag() - mag;
		break;
            }
            case StartX: fill_[i] = reco_Vert[0] - X_[t_entry]; break;
            case StartY: fill_[i] = reco_Vert[1] - Y_[t_entry]; break;
            case StartZ: fill_[i] = reco_Vert[2] - Z_[t_entry]; break;
            case StartMag: {
                 double mag = sqrt(X_[t_entry]*X_[t_entry] 
                                 + Y_[t_entry]*Y_[t_entry] 
                                 + Z_[t_entry]*Z_[t_entry]);
                 fill_[i] = reco_Vert.Mag() - mag;
                 break;
            }
            case None: fill_[i] = 1; break;
        }
    }
    this->FillData(t_hist, t_entry);
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
    fill_[1] = 1;

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
    cout << "Draw Hit entry " << t_entry << "\n";
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

    fill_[1] = (double) num_cluster;
    fill_[0] = (double) db_num_cluster;

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
EmbedCut::EmbedCut(const std::string& t_file, const std::string& t_cutname) : file_(t_file.c_str()), cutg_(0)
{
    if(!file_.IsOpen()) std::cerr << "Cut File not found\n";
    cutg_ = (TCutG*) file_.Get(t_cutname.c_str());
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
