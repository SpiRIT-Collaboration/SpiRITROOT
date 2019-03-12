#ifndef CONFIGLISTPARSER
#define CONFIGLISTPARSER
#include <stdio.h>

class ConfigListParser
{
public:
  void ParseLine(const std::string& t_line)
  {
    // reset stringstream to accept new line
    ss_.str("");
    ss_.clear();
    ss_ << t_line;
  };

  bool Next(){ if(ss_ >> line_) return true; else return false;};

  std::string GetElement()
  {
    // ignore # sign
    if(line_[0] == '#')
      line_ = line_.substr(1, line_.size() - 1);

    std::stringstream element;
    element << line_;
    // read with space delimited if there's no quote
    // otherwise read the entire quote as one
    if(line_[0] == '"')
    {
      while((ss_ >> line_))
      {  
        element << " " << line_; 
        if(line_.back() == '"') break;
      }
      auto ans = element.str();
      return ans.substr(1, ans.size() - 2);
    }
    return element.str();
  };

private:
  std::stringstream ss_;
  std::string line_;
  bool inside_quote_;
};

class ConfigListIO
{
public:
  void ToText(const std::string& t_filename)
  {
    std::ofstream file(t_filename.c_str());
    if(!file.is_open())  std::cerr << "Error configlist cannot be created\n";
    
    // first write header
    file << "#";
    for(const auto& key: header_) file << key << " ";

    // write content one by one
    file << "\n";
    for(const auto& line: data_)
    {
      for(const auto& element: line)
        file << element << " ";
      file << "\n";  
    }
  };

  void FillFromText(const std::string& t_filename)
  {
    data_.clear();
    header_.clear();

    // read file
    std::string line;
    std::ifstream file(t_filename.c_str());

    // parse input
    ConfigListParser parser;
    // first line is header
    std::getline(file, line);
    parser.ParseLine(line);
    while(parser.Next()) header_.push_back(parser.GetElement());

    // get the rest
    while(std::getline(file, line))
    {
      std::vector<std::string> row;
      parser.ParseLine(line); 
      while(parser.Next()) row.push_back(parser.GetElement());
      data_.push_back(row);
    }
  };

  void SetHeader(const std::vector<std::string> t_header){ header_ = t_header;};
  template<class T>
  void SetElement(const std::string& t_key, const T& t_value)
  { 
    std::stringstream ss; 
    ss << t_value; 
    elements_[t_key] = ss.str();
  };

  template<class T>
  void SetElement(const std::string& t_key, const std::vector<T>& t_value)
  { 
    std::stringstream ss; 
    ss << "\"";
    for(const auto& val: t_value) 
      ss << val << " "; 
    ss << "\"";
    elements_[t_key] = ss.str();
  }
  void Fill()
  {
    std::vector<std::string> ordered_line;
    for(const auto& key: header_)
      ordered_line.push_back(elements_[key]);
    data_.push_back(ordered_line);
    elements_.clear();
  };

  void ToConfig(int t_line, const std::string& t_filename)
  {
    std::ofstream config(t_filename.c_str());
    if(!config.is_open())  std::cerr << "Cannot open file " << t_filename << "\n";

    const auto& row = data_[t_line];
    for(unsigned i = 0; i < row.size(); ++i)
      config << header_[i] << " " << row[i] << "\n";
  };

  std::string GetElement(int t_line, const std::string& t_key)
  {
    auto iter = std::find(header_.begin(), header_.end(), t_key);
    if(iter == header_.end())
    {
      std::cerr << "Key " << t_key << " not found\n";
      return "";
    }

    int index = std::distance(header_.begin(), iter);
    return data_[t_line][index];
  }

  unsigned Size() { return data_.size(); };
private:
  std::vector<std::string> header_;
  std::vector<std::vector<std::string>> data_;
  std::map<std::string, std::string> elements_;
  ConfigListParser parser_;
};

#endif
