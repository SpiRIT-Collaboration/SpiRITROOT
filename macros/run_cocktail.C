//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include "run_general.C"

const std::string config = 
"Run number	Anode 12 & 14 (V)\n"
"3202	1214	Cocktail100	 \n"
"3203	1214    Cocktail100	 \n"
"3204	1460    Cocktail100	 \n"
"3205	1460    Cocktail100	 \n"
"3187	1214	Cocktail300	 \n"
"3189	1214	Cocktail300	 \n"
"3190	1214	Cocktail300	 \n"
"3191	1214	Cocktail300	 \n"
"3193	1214	Cocktail300	 \n"
"3194	1214	Cocktail300	 \n"
"3195	1214	Cocktail300	 \n"
"3196	1214	Cocktail300	 ";       
  
const std::map<std::string, std::string> cocktail_info{ // copy and paste from google sheet on Cocktail momenta
{"Cocktail100",
"lower mom	high mom	Mean value	width	fraction %\n"
"p	900.7	906.3	903.5	5.6	0.619811843		  \n"
"d	895.6	901.4	898.5	5.8	0.645520312		  \n"
"t	883.9	890.1	887	6.2	0.698985344		  \n"
"3He	1789.4	1802	1795.7	12.6	0.701676227		  \n"
"4He	1776.2	1789.6	1782.9	13.4	0.751584497		  "},
{"Cocktail300",
"lower mom	high mom	Mean value	width	fraction %	\n"
"d	1613.7	1628.4	1621.05	14.7	0.906819654			\n"
"t	1604.7	1620	1612.35	15.3	0.948925481			\n"
"3He	3221.9	3250.9	3236.4	29	0.896057348			\n"
"4He	3211.7	3241	3226.35	29.3	0.908146977			\n"
"6Li	4792.6	4836.9	4814.75	44.3	0.920089309			\n"
"7Li	4788.8	4822.4	4805.6	33.6	0.699184285			"}};

const std::map<std::string, int> pname2id{ // tables for particle name to pdg_table id
{"p", 2212},
{"d", 1000010020},
{"t", 1000010030},
{"3He", 1000020030},
{"4He", 1000020040},
{"6Li", 1000030060},
{"7Li", 1000030070},
};

     
struct RunConfig
{
	int run_id;		// run number of the cocktail beam
	bool anode_12;		// whether anode 12 is used. if true different gain file will be loaded
	int num_run;		// number of events that needs to be generated
	std::string cocktail_type;
	TString GCData, GGData;
	// construct Run config from a line in the google sheet config
	RunConfig(const std::string& t_line)
	{
		// temp variables
		TString temp;
		int anode;

		// read line
		std::stringstream ss(t_line);
		ss >> run_id >> anode >> cocktail_type;

		// obtain information from run_info.txt
		//GetParameters(run_id, num_run, GCData, GGData);
		if(anode == 1214) anode_12 = true;
		else if(anode == 1460) anode_12 = false;
		else std::cerr << "Anode setting not recognized: " << anode << "\n";
	};
};

struct CocktailConfig // Calculated LISE momentum given run momentum
{	
	std::string 	pname;	// name of the outgoing particle
	int 		pid;	// corresponding pdg_table id of the particle
	double 		momentum;
	// construct Cocktail config from a line in the google sheet
	CocktailConfig(const std::string& t_line)
	{	
		std::string temp;
		std::stringstream ss(t_line);
		ss >> pname >> temp >> temp >> momentum >> temp >> temp;
		pid = pname2id.at(pname);
	};
	
};

class IOLoader
{
public:
	IOLoader(std::istream& t_stream) : stream_(t_stream) { for(int i = 0; i < 2; ++i) std::getline(stream_, line_);};// get rid of the header

	IOLoader Next()
	{
		if(!std::getline(stream_, line_))
			line_ = "";
		return *this;
	};

	bool IsEnd()
	{ return line_.empty(); };

	template<class T>
	T Get()
	{ return T(line_); };
		
		
private:
	std::istream& stream_;
	std::string line_;
};

void run_cocktail(int t_start_from = 1, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	std::vector<RunInfo> info_list;

	std::stringstream run_ss(config);
	for(IOLoader run_loader((run_ss)); !run_loader.IsEnd(); run_loader.Next())
	{
		RunConfig conf = run_loader.Get<RunConfig>();
		std::stringstream cocktail_ss(cocktail_info.at(conf.cocktail_type));
		for(IOLoader cocktail_loader((cocktail_ss)); !cocktail_loader.IsEnd(); cocktail_loader.Next())
		{
			CocktailConfig cocktail_conf = cocktail_loader.Get<CocktailConfig>();

			RunInfo info;
			// name of the output mc files
			info.filename = "Run_" + std::to_string(conf.run_id) + "_" + cocktail_conf.pname + "_junk";
			info.nevent = conf.num_run;
			info.momentum = cocktail_conf.momentum/1000.;
			info.particle = cocktail_conf.pid;
			info.phi = info.theta = 0.;

			info_list.push_back(info);
		}
	}

	std::string output_filename("Cocktail");

	RunListToFile(info_list, output_filename);
	run_general(output_filename);

}
