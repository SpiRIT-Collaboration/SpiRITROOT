#include "RulesDrawer.hh"

/***********************************
DrawMultipleComplex allows one to output intermediate data to more than 1 histograms
It works by creating checkpoints
Insert them in the right step and this class with read and fill them to histograms
***********************************/

template<typename T, typename... ARGS>
void DrawMultipleComplex::DrawMultiple(Rule& t_rule, T& first_graph, ARGS&... args)
{
    checkpoints_.clear();
    this->GetCheckPoints(&t_rule);
    reader_.Restart();
    t_rule.SetReader(reader_);
    if(reader_.GetEntries(true) == 0)
    {
        std::cerr << "No entries from reader. Failed to load tree from file. Will ignore\n";
        return;
    }
    while( reader_.Next() )
    {
        std::vector<DataSink> result;
        t_rule.Fill(result, reader_.GetCurrentEntry());
        std::cout << "Processing Entry " << reader_.GetCurrentEntry() << "\t\r";
    }
    std::cout << "\n";
    FillHists(0, t_rule, first_graph, args...);
}

template<typename T, typename... ARGS>
void DrawMultipleComplex::DrawMultiple(const std::vector<int>& t_entry_list, Rule& t_rule, T& first_graph, ARGS&... args)
{
    checkpoints_.clear();
    this->GetCheckPoints(&t_rule);
    reader_.Restart();
    t_rule.SetReader(reader_);
    for(const auto& entry : t_entry_list)
    {
        reader_.SetEntry(entry);
        std::vector<DataSink> result;
        t_rule.Fill(result, entry);
        std::cout << "Processing Entry " << reader_.GetCurrentEntry() << "\t\r";
    }
    std::cout << "\n";
    FillHists(0, t_rule, first_graph, args...);
}

template<typename T>
void DrawMultipleComplex::DrawMultiple(Rule& t_rule, std::vector<T>& t_graphs)
{
    checkpoints_.clear();
    this->GetCheckPoints(&t_rule);
    if(t_graphs.size() != checkpoints_.size())
    {
        std::cerr << "Number of checkpoints and graphs doesn't match. Will abort\n";
        return;
    }

    reader_.Restart();
    t_rule.SetReader(reader_);
    if(reader_.GetEntries(true) == 0)
    {
        std::cerr << "No entries from reader. Failed to load tree from file. Will ignore\n";
        return;
    }
    while( reader_.Next() )
    {
        std::vector<DataSink> result;
        t_rule.Fill(result, reader_.GetCurrentEntry());
        std::cout << "Processing Entry " << reader_.GetCurrentEntry() << "\t\r";
    }
    std::cout << "\n";
    for(auto cp : checkpoints_)
    {
        auto data = cp->GetData();
        for(const auto& row : data) t_graphs[cp->id].Fill(row[0], row[1]);
    };
}

template<typename T, typename... ARGS>
void DrawMultipleComplex::FillHists(int t_ncp, Rule& t_rule, T& first_graph, ARGS&... args)
{
    auto data = checkpoints_[t_ncp]->GetData();
    for(const auto& row : data) first_graph.Fill(row[0], row[1]);
    FillHists(t_ncp + 1, t_rule, args...);
}

template<typename T>
void DrawMultipleComplex::FillHists(int t_ncp, Rule& t_rule, T& graph)
{
    auto data = checkpoints_[t_ncp]->GetData();
    for(const auto& row : data) graph.Fill(row[0], row[1]);
}

/********************************
DrawComplex
Just append a checkpoint at the end of a rule
only draw 1 result
********************************/
template<class T, typename... ARGS>
T DrawComplex::FillRule(Rule& t_rule, ARGS... args)
{
    CheckPoint cp(0);
    t_rule.AppendRule(&cp);
    T hist(args...);
    reader_.Restart();
    t_rule.SetReader(reader_);
    int index = 0;
    while( reader_.Next() )
    {
        std::vector<DataSink> result;
        t_rule.Fill(result, index);
        for(const auto& row : cp.GetData()) hist.Fill(row[0], row[1]);
        std::cout << "Processing Entry " << index << "\t\r";
        ++index;
    }
    t_rule.PopRule();
    std::cout << "\n";
    return hist;
}

template<class T, typename... ARGS>
T DrawComplex::FillRule(const std::vector<int>& t_entry_list, Rule& t_rule, ARGS... args)
{
    CheckPoint cp;
    t_rule.AppendRule(&cp);
    T hist(args...);
    reader_.Restart();
    t_rule.SetReader(reader_);
    for(const auto& entry : t_entry_list)
    {
        reader_.SetEntry(entry);
        std::vector<DataSink> result;
        t_rule.Fill(result, entry);
        for(const auto& row : cp.GetData()) hist.Fill(row[0], row[1]);
        std::cout << "Processing Entry " << entry << "\t\r";
    }
    t_rule.PopRule();
    std::cout << "\n";
    return hist;
}

