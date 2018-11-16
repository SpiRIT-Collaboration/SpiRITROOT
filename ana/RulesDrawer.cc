#include "RulesDrawer.hh"

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

DrawMultipleComplex::DrawMultipleComplex(TChain* t_chain, Rule* t_rule) : 
    reader_(t_chain),
    rule_(t_rule)
{
    if(t_rule) t_rule->SetReader(reader_);
}


void DrawMultipleComplex::GetCheckPoints(Rule* t_rule)
{
    checkpointset_.clear();
    this->GetCheckPointsSet(t_rule);
    // sort the vector according to checkpoint id
    checkpoints_.resize(checkpointset_.size());
    for(auto cp : checkpointset_)
        checkpoints_[cp->id] = cp;
}

void DrawMultipleComplex::GetCheckPointsSet(Rule* t_rule)
{
    // Find all checkpoints along the rule tree
    // Things to be aware: SwitchCut doesn't use NextRule_
    // that class need to be handled differently
    auto current_rule = t_rule;
    if(current_rule)
    {
        // search for rejected rules to see if there are checkpoints there
        if(current_rule->RejectRule_) this->GetCheckPointsSet(current_rule->RejectRule_);
        if(auto checkpoint = dynamic_cast<CheckPoint*>(current_rule)) 
            checkpointset_.insert(checkpoint);
        else if(auto switch_ = dynamic_cast<SwitchCut*>(current_rule))
        {
            for(auto rule : switch_->execution_) this->GetCheckPointsSet(rule);
            return;
        }
        this->GetCheckPointsSet(current_rule->NextRule_);
    }
}

/********************************
Iterator for draw multiple complex
********************************/
std::vector<DataSink> DrawMultipleComplex::Iterator::operator*() const
{
    std::vector<DataSink> temp;
    if(!fcomplex_.rule_) std::cerr << "Rule is not set. Cannot use iterator\n";
    fcomplex_.rule_->Fill(temp, *it_);

    std::vector<DataSink> datalist;
    for(auto cp : fcomplex_.checkpoints_)
        datalist.push_back(cp->GetData());
    return datalist;
}