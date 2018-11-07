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

std::vector<CheckPoint*> DrawMultipleComplex::NewCheckPoints(int t_num)
{
    for(unsigned i = 0; i < t_num; ++i)
    {
        CheckPoint *cp = new CheckPoint;
        checkpoints_.push_back(cp);
    }
    return checkpoints_;
}

void DrawMultipleComplex::GetCheckPoints(Rule* t_rule)
{
    // Find all checkpoints along the rule tree
    // Things to be aware: SwitchCut doesn't use NextRule_
    // that class need to be handled differently
    auto current_rule = t_rule;
    if(current_rule)
    {
        if(auto checkpoint = dynamic_cast<CheckPoint*>(current_rule)) checkpoints_.push_back(checkpoint);
        else if(auto switch_ = dynamic_cast<SwitchCut*>(current_rule))
        {
            for(auto rule : switch_->execution_) this->GetCheckPoints(rule);
            return;
        }
        this->GetCheckPoints(current_rule->NextRule_);
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
