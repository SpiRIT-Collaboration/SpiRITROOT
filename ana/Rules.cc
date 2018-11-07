#include "Rules.hh"

/****************************
Rules: base abstract class for event cut and analysis
******************************/
void Rule::SetReader(TTreeReader& t_reader)
{
    this->SetMyReader(t_reader);
    if(NextRule_) NextRule_->SetReader(t_reader);
    if(RejectRule_) RejectRule_ -> SetReader(t_reader);
};

void Rule::Fill(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    //if(PreviousRule_) fill_ = PreviousRule_ -> fill_;
    this->Selection(t_hist, t_entry);
}


Rule* Rule::AddRule(Rule* t_rule)
{
    // avoid self referencing
    if(this == t_rule)
    {    std::cerr << "Try to add the same rule twice. Will be ignored\n";}
    if(NextRule_)
    {    std::cerr << "NextRule to this rule is alread set. Don't add again\n";}
    else
    {
        NextRule_ = t_rule;
        t_rule->PreviousRule_ = this;
    }
    return this;
}

Rule* Rule::AddRejectRule(Rule* t_rule)
{
    // avoid self referencing
    if(this == t_rule)
    {    std::cerr << "Try to add the same rule twice. Will be ignored\n";}
    if(RejectRule_)
    {    std::cerr << "RejectedRules have been set. You cannot add another RejectedRule\n";}
    else
    {
        RejectRule_ = t_rule;
        t_rule->PreviousRule_ = this;
    }
    return this;
}

/********************************
RuleBlock
Simply group a few rules into 1
Connect the rejected rule to the last rule within the block
********************************/
/*void RuleBlock::SetReader(TTreeReader& t_reader)
{
    InternalFirst_->SetReader(t_reader);
};

void RuleBlock::AddBlockRule(Rule* t_rule)
{
    // look for last/first rule in the internal rule
    while(t_rule->PreviousRule_)
        t_rule = t_rule->PreviousRule_;

    InternalFirst_ = t_rule;
    while(t_rule->NextRule_) 
        t_rule = t_rule->NextRule_;
    InternalLast_ = t_rule;
}

Rule* RuleBlock::AddRule(Rule* t_rule)
{
    if(!InternalFirst_ || !InternalLast_)
        std::cerr << "RuleBlock is empty. Please check again";
       
    InternalLast_->AddRule(t_rule);
    return InternalFirst_;
}

Rule* RuleBlock::AddRejectRule(Rule* t_rule)
{
    return InternalLast_->AddRejectRule(t_rule);
}*/

std::pair<Rule*, Rule*> RuleBlock(Rule* t_rule)
{
    // look for last/first rule in the internal rule
    while(t_rule->PreviousRule_)
        t_rule = t_rule->PreviousRule_;

    Rule *InternalFirst_ = t_rule;
    while(t_rule->NextRule_) 
        t_rule = t_rule->NextRule_;
    Rule *InternalLast_ = t_rule;
    return {InternalFirst_, InternalLast_};
}

/********************************
CheckPoint stores intermediate results
And allow user to retrieve it afterwards
*********************************/
void CheckPoint::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    temp_sink_.insert(temp_sink_.end(), t_hist.back().begin(), t_hist.back().end());
    this->FillData(t_hist, t_entry);
}

DataSink CheckPoint::GetData() 
{ 
    auto sink = temp_sink_;
    temp_sink_.clear();
    return sink;
};


/******************************
RecoTrackNumFilter
Skip events with track number > 1
Useful for mc single track data
******************************/
void RecoTrackNumFilter::SetMyReader(TTreeReader& t_reader)
{
    myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");
}

void RecoTrackNumFilter::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    if(this->compare_((*myTrackArray_)->GetEntries())) this->FillData(t_hist, t_entry);
    else this->RejectData(t_hist, t_entry);
}



/************************************
this draws output of the previous rules
will only work if it is DrawHit (or DrawTrack in the future). To do list
Can only work with x-z or y-z plane. To do list for other dimensions
*************************************/

GetHitOutline::GetHitOutline(const std::string& t_outputname) : 
    file_(t_outputname.c_str(), "RECREATE"),
    cutg_array_("TCutG", max_num_) {}

Rule* GetHitOutline::AddRule(Rule* t_rule)
{
    auto pt = this->Rule::AddRule(t_rule);
    auto prule = static_cast<DrawHit*>(PreviousRule_);
    if(!prule) std::cerr << "Only DrawHit class is allowed before GetHitOutline";
    return pt;
    //static_assert(prule->y_==2, "Only x-z plane can be used with GetHitOutline");
    //static_assert(prule->x_==0, "Only works if x-axis of the DrawHit Clasas is axis 0");
}

GetHitOutline::~GetHitOutline()
{
    file_.cd();
    cutg_array_.Write();
}

void GetHitOutline::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    if(t_entry > max_num_)
    {
        std::cerr << "Reached maximum number of allowed cuts. Will skip\n";
        this->FillData(t_hist, t_entry);
        return;
    }

    const auto& hit_data = t_hist.back();
    TH2F hist("TEMP", "",pad_x, 
              -0.5*((double)pad_x*size_x), 
              0.5*((double)pad_x*size_x), 
              pad_y, 0, (double) pad_y*size_y); 

    for(const auto& row : hit_data) hist.Fill(row[0], row[1]);
    auto cutg = HistToCutG(hist);
    cutg.SetName(std::to_string(t_entry).c_str());
    new(cutg_array_[t_entry]) TCutG(cutg);
}


/************************************
DrawHit uses information from STHit class instead
Cannot iterate through each tracks
**************************************/
void DrawHit::SetMyReader(TTreeReader& t_reader) 
{
    myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STHit");
}

void DrawHit::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{    
    DataSink localsink;
    for(int j = 0; j < (*myHitArray_)->GetEntries(); ++j)
    {
        auto hit = static_cast<STHit*>((*myHitArray_) -> At(j));
        auto pos = hit->GetPosition();
        localsink.push_back({pos[x_], pos[y_]});
    }
    t_hist.push_back(localsink);
    this->FillData(t_hist, t_entry);
}

/*******************************
DrwaHitEmbed: similar but use info from STEmbedHit
*********************************/
void DrawHitEmbed::SetMyReader(TTreeReader& t_reader) 
{
    myHitArray_ = std::make_shared<ReaderValue>(t_reader, "STEmbedHit");
}

void DrawHitEmbed::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{    
    DataSink localsink;
    for(int j = 0; j < (*myHitArray_)->GetEntries(); ++j)
    {
        auto hit = static_cast<STHit*>((*myHitArray_) -> At(j));
        auto pos = hit->GetPosition();
        localsink.push_back({pos[x_], pos[y_]});
    }
    t_hist.push_back(localsink);
    this->FillData(t_hist, t_entry);
}


/*********************
ValueCut : 1-D value cut on the previous rule
************************/
void ValueCut::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    auto& fill = t_hist.back().back();
    if( upper_ > lower_)
    {
        if( upper_ >= fill[index_] && fill[index_] >= lower_) 
            this->FillData(t_hist, t_entry);
        else this->RejectData(t_hist, t_entry);
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

void EmbedCut::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    auto& fill = t_hist.back().back();
    if(cutg_->IsInside(fill[0], fill[1])) 
        this->FillData(t_hist, t_entry);
    else this->RejectData(t_hist, t_entry);
}

/**************************
EntryRecorder: Save all entries that have been looped through
Useful for when you want to know which entries satisfy all conditions
***************************/
void EntryRecorder::Selection(std::vector<DataSink>& t_hist, unsigned t_entry) 
{
    // do not fill repeated entry number
    if(list_.size() == 0) list_.push_back(t_entry);
    else if(t_entry != list_.back()) list_.push_back(t_entry);
    this->FillData(t_hist, t_entry);
};


void EntryRecorder::ToFile(const std::string& t_filename)
{
    std::ofstream file(t_filename.c_str());
    for(const auto& i : list_)
        file << i << "\n";
};

/*****************************
TrackZFilter
Use to see if the event contains tracks that goes forward
For some reasons some tracks will only register their hit in origin
Reject those
*****************************/
void TrackZFilter::SetMyReader(TTreeReader& t_reader)
{ myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");};

void TrackZFilter::Selection(std::vector<DataSink>& t_hist, unsigned t_entry)
{
    bool non_empty = false;
    for(unsigned track_id_ = 0; track_id_ < (*myTrackArray_)->GetEntries(); ++track_id_)
    {
        auto track_ = static_cast<STRecoTrack*>((*myTrackArray_) -> At(track_id_));
        for(const auto& point : (*track_->GetdEdxPointArray()))
            if(point.fPosition[2] > 10)
            {
                non_empty = true;
                break;
            }
        if(non_empty) break;
    }
    if(non_empty) this->FillData(t_hist, t_entry);
    else this->RejectData(t_hist, t_entry);
};
