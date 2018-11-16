#ifndef DRAWERRULESSS_H
#define DRAWERRULESSS_H
#include "HistToCutG.hh"
#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include <functional>
#include <set>

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
#include "Rules.hh"


class DrawMultipleComplex
{
public:
    DrawMultipleComplex(const std::string& t_filenames, const std::string& t_treename, Rule* t_rule=0);
    DrawMultipleComplex(TChain *t_chain, Rule* t_rule=0);
    virtual ~DrawMultipleComplex(){};

    template<typename T, typename... ARGS>
    void DrawMultiple(Rule& t_rule, T& first_graph, ARGS&... args);
    template<typename T, typename... ARGS>
    void DrawMultiple(const std::vector<int>& t_entry_list, Rule& t_rule, T& first_graph, ARGS&... args);
    template<typename T>
    void DrawMultiple(Rule& t_rule, std::vector<T>& t_graphs);


    void SetRule(Rule* t_rule)
    {
        rule_ = t_rule; 
        this->GetCheckPoints(t_rule);
        std::cout << "CP size " << checkpoints_.size() << "\n";
        reader_.Restart(); 
        rule_->SetReader(reader_);
    };
protected:
    template<typename T, typename... ARGS>
    void FillHists(int t_ncp, Rule& t_rule, T& first_graph, ARGS&... args);
    template<typename T>
    void FillHists(int t_ncp, Rule& t_rule, T& graph);
    void GetCheckPointsSet(Rule *t_rule);
    void GetCheckPoints(Rule* t_rule);

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

    private:
        DrawMultipleComplex& fcomplex_;
        TTreeReader::Iterator_t it_;
    };
protected:
    Rule* rule_;
    TChain chain_;
    TTreeReader reader_;
    std::vector<CheckPoint*> checkpoints_;
    std::set<CheckPoint*> checkpointset_;
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
    { chain_.Add(t_filenames.c_str()); }

    DrawComplex(TChain& t_chain) : reader_(&t_chain){};

    template<class T, typename... ARGS>
    T FillRule(Rule& t_rule, ARGS... args);
    template<class T, typename... ARGS>
    T FillRule(const std::vector<int>& t_entry_list, Rule& t_rule, ARGS... args);
protected:
    TChain chain_;
    TTreeReader reader_;
};

#include "RulesDrawer.tcc"

#endif
