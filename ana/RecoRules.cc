#include "RecoRules.hh"

/******************************
RecoTrackRule
Base class that iterats through STRecoTracks
*******************************/
void RecoTrackRule::SetReader(TTreeReader& t_reader) 
{
    myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");
    this->Rule::SetReader(t_reader);
};

void RecoTrackRule::Fill(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    if(PreviousRule_) 
    {
        // look for previous rule recrusively to see if the loop on track has started
        // such that oridinay rule can be inserted in between RecoTrackRule without restarting the loop
        Rule* find_reco_rule = PreviousRule_;
        while(find_reco_rule)
        {
            if(auto prule = dynamic_cast<RecoTrackRule*>(find_reco_rule))
            {
                track_ = prule->track_;
                track_id_ = prule->track_id_;
                this->Selection(t_hist, t_entry);
                return;
            }else find_reco_rule = find_reco_rule->PreviousRule_;
        }
    }

    // if previous rule doesn't start looping on recotrack, it will initialize it
    for(track_id_ = 0; track_id_ < (*myTrackArray_)->GetEntries(); ++track_id_)
    {
        track_ = static_cast<STRecoTrack*>((*myTrackArray_) -> At(track_id_));
        this->Selection(t_hist, t_entry);
    }
}

/*******************************
If we enable BDC vertex
We may want to use VATracks instead of STRecoTrack
Everything comes after UseVATracks class will read data from STVATracks instead
********************************/
void UseVATracks::SetMyReader(TTreeReader& t_reader)
{
    myVATracksArray_ = std::make_shared<ReaderValue>(t_reader, "VATracks");
}

void UseVATracks::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    // look for va tracks that corresponds to the current track_id_
    for(unsigned i = 0; i < (*myVATracksArray_)->GetEntries(); ++i)
        if(static_cast<STRecoTrack*>((*myVATracksArray_) -> At(i))->GetRecoID() == track_id_)
        {
           track_id_ = i;
           track_ = static_cast<STRecoTrack*>((*myVATracksArray_) -> At(track_id_));
           this->FillData(t_hist, t_entry);
           return;
        }
    // reject if the track is not found
    this->RejectData(t_hist, t_entry);
}

/******************************
PID on tracks
******************************/
void PID::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    // find the average dE
    t_hist.push_back({{track_->GetMomentum().Mag(), track_->GetdEdxWithCut(0, 0.7)}});
    this->FillData(t_hist, t_entry);
}


/****************************
Observer: Pipe intermediate result to stdout
****************************/
void Observer::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    auto fill = t_hist.back().back();
    std::cout << title_ << " x: " << fill[0] << " y: " << fill[1] << " \n";
    this->FillData(t_hist, t_entry);
}

/******************************
Draw trackes from STRecoTrack
********************************/
void DrawTrack::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{    
    //t_hist.clear();
    DataSink localsink;
    for(const auto& point : (*track_->GetdEdxPointArray()))
        localsink.push_back({point.fPosition[x_], point.fPosition[y_]});
    t_hist.push_back(localsink);
    this->FillData(t_hist, t_entry);
}

/******************************
EmbedFilter
Only STRecoTracks corresponds to embeded track will be futher processed
******************************/
void EmbedFilter::SetMyReader(TTreeReader& t_reader)
{
    myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");
}

void EmbedFilter::Fill(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    embed_id_ = 1e3;
    embed_track_ = nullptr;
    if(PreviousRule_) 
    {
        // look for previous rule recrusively to see if embed id is found
        Rule* find_reco_rule = PreviousRule_;
        while(find_reco_rule)
        {
            if(auto prule = dynamic_cast<EmbedFilter*>(find_reco_rule))
            {
                embed_track_ = prule->embed_track_;
                embed_id_ = prule->embed_id_;
                break;
            }else find_reco_rule = find_reco_rule->PreviousRule_;
        }
    }

    this->RecoTrackRule::Fill(t_hist, t_entry);
}

void EmbedFilter::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    for(embed_id_ = 0; embed_id_ < (*myEmbedArray_)->GetEntries(); ++embed_id_)
    {
        embed_track_ = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(embed_id_));
        auto id = embed_track_->GetArrayID();
        if(id == track_id_) 
        {
            this->FillData(t_hist, t_entry);
            return;
        }
    }
    this->RejectData(t_hist, t_entry);
}

/******************************
EmbedExistence
Just to see if there are embeding tracks inside
if so pass the entire event donw
******************************/
void EmbedExistence::SetMyReader(TTreeReader& t_reader)
{
    myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");
}

void EmbedExistence::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    for(int i = 0; i < (*myEmbedArray_)->GetEntries(); ++i)
    {
        auto id = static_cast<STEmbedTrack*>((*myEmbedArray_)->At(i))->GetArrayID();
        if(id >= 0) 
        {
            this->FillData(t_hist, t_entry);
            return;
        }
    }
    this->RejectData(t_hist, t_entry);
}

/********************************
RecoTrackClusterNumFilter
Reject a track if the number of clusters it contains is too small
*********************************/
void RecoTrackClusterNumFilter::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    if(compare_((*track_->GetdEdxPointArray()).size())) this->FillData(t_hist, t_entry);
    else this->RejectData(t_hist, t_entry);
}

/*************************************
CompareMCPrimary
Compare momentum distribution event by event to momentum layout by the give file
Should only be used together with embed filter
*************************************/
CompareMCPrimary::CompareMCPrimary(CompareMCPrimary::Type t_x, 
                                   CompareMCPrimary::Type t_y) :
    x_(t_x),
    y_(t_y)
{}

void CompareMCPrimary::SetMyReader(TTreeReader& t_reader)
{
    myEmbedArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedTrack");
}

void CompareMCPrimary::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    auto mc_mom = embed_track_->GetInitialMom();
    auto mc_Vert = embed_track_->GetInitialVertex();
    auto reco_mom = track_->GetMomentum();
    auto reco_Vert = track_->GetPOCAVertex();
    double xval, yval;

    // fill x and y respectively
    // that's why the range is 2
    const double GEVTOMEV = 1e3;
    for(int i = 0; i < 2; ++i)
    {
        auto type = (i==0)? x_ : y_;
        auto& val = (i==0)? xval : yval;
        switch(type)
        {
            case MomX: val = reco_mom[0] - GEVTOMEV*mc_mom[0]; break; 
            case MomY: val = reco_mom[1] - GEVTOMEV*mc_mom[1]; break;
            case MomZ: val = reco_mom[2] - GEVTOMEV*mc_mom[2]; break;
            case MMag: val = (reco_mom - GEVTOMEV*mc_mom).Mag(); break;
            case StartX: val = reco_Vert[0] - mc_Vert[0]; break;
            case StartY: val = reco_Vert[1] - mc_Vert[1]; break;
            case StartZ: val = reco_Vert[2] - mc_Vert[2]; break;
            case StartMag: val = (reco_Vert - mc_Vert).Mag(); break;
            case None: val = 1; break;
        }
    }
    t_hist.push_back({{xval, yval}});
    this->FillData(t_hist, t_entry);
}


/************************************
MomentumTracks
Draw the momentum associated with each track
Set axis to plot different momentum direction
*************************************/
void MomentumTracks::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    auto mom = track_->GetMomentum();
    double val;

    // return magnitude if axis_ = 3
    if(axis_ < 3) val = mom[axis_];
    else val = mom.Mag();
    t_hist.push_back({{val, 1}});

    this->FillData(t_hist, t_entry);
}

/************************************
TrackShapeFilter
Compare the tracks with cut given by cut file
See if the embed correlator is functioning
*************************************/
TrackShapeFilter::TrackShapeFilter(const std::string& t_cutfilename, double t_threshold) : 
    cut_file_(t_cutfilename.c_str()),
    threshold_(t_threshold){}

void TrackShapeFilter::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    cutg_ = (TCutG*) cut_file_.Get(std::to_string(t_entry).c_str());
    if(! cutg_ ) 
    {
        this->RejectData(t_hist, t_entry);
        return;// if no cut for such entry, treat as if it is rejected
    }
    auto& track_pts = t_hist.back();

    unsigned num_inside = 0;
    for(const auto& row : track_pts)
        if(cutg_->IsInside(row[0], row[1])) ++num_inside;
    double percentage = (double) num_inside/ (double) track_pts.size();
    if(percentage < threshold_) this->FillData(t_hist, t_entry);
    else this->RejectData(t_hist, t_entry);
    
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

void RenshengCompareData::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
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

    t_hist.push_back({{(double) num_cluster, (double) db_num_cluster}});
    this->FillData(t_hist, t_entry);
};

/***********************************
ClusterNum: simply fill with number of clusters of each tracks
***********************************/
void ClusterNum::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    auto num_cluster = track_->GetNumRowClusters90() + track_->GetNumLayerClusters90();
    t_hist.push_back({{(double) num_cluster, 1.}});
    this->FillData(t_hist, t_entry);
}

    
/*********************
ThetaPhi : angle of the initial momentum of each track
*********************/
void ThetaPhi::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    //if(num_cluster < 20) continue;
    auto momVec = track_ -> GetMomentum();
    auto phiL = momVec.Phi()*180./TMath::Pi();
    auto thetaL = momVec.Theta()*180./TMath::Pi();

    t_hist.push_back({{thetaL, phiL}});
    this->FillData(t_hist, t_entry);
}

/**************************
TrackIDRecorder: Save all entries and its track id that have been looped through
Useful for when you want to know which track satisfy all conditions
***************************/
void TrackIDRecorder::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    list_.push_back({t_entry, track_id_});
    this->FillData(t_hist, t_entry);
}

void TrackIDRecorder::ToFile(const std::string& t_filename)
{
    std::ofstream file(t_filename.c_str());
    file << "Entry\tTrack_id\n";
    for(const auto& result : list_)
        file << result.first << "\t" << result.second << "\n";
}

/****************************
MCThetaPhi
Plot initial MC theta vs phi instead of reco direction
Must have EmbedFilter in front
Only work for embedded data
******************************/
void MCThetaPhi::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    auto momVec = embed_track_->GetInitialMom();
    auto phiL = momVec.Phi()*180./TMath::Pi();
    auto thetaL = momVec.Theta()*180./TMath::Pi();

    t_hist.push_back({{thetaL, phiL}});
    this->FillData(t_hist, t_entry);
}

/************************************
MCMomentumTracks
Initial momentum of the mc data 
*************************************/
void MCMomentumTracks::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    auto mom = embed_track_->GetInitialMom();
    double val;

    // return magnitude if axis_ = 3
    if(axis_ < 3) val = mom[axis_];
    else val = mom.Mag();
    t_hist.push_back({{val, 1}});

    this->FillData(t_hist, t_entry);
}
