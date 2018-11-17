/*******************************************
EEDG6375 Design Automation of VLSI systems
Programming assignment 1, FM partitioner
by,
		Team-9
Akash Anil tadmare		aat170130
Akshay Nandkumar Patil	anp170330
********************************************/

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdlib>
using namespace std;


//global declarations

int sumA = 0;
int sumB = 0;
int sumA_mincut = 0;
int sumB_mincut = 0;
int pmaxA =0;
int pminA =0;
int pmaxB =0;
int pminB =0;
int cutset =0;
int new_cutset = 0;
int new_index = 0; //index for selecting next cell if balance_check not satisfied
int indexA = 0; //index for gainA
int indexB = 0; //index for gainB
int i =0;
int j =0;
int k =0;
int mincut = 0;
int startcut = 0;
int old_partition =0;
bool deadA = false;
bool deadB = false;

vector <string> setA;
vector <string> setB;
vector <string> setA_mincut;
vector <string> setB_mincut;
vector <int> gainA;
vector <int> gainB;
map <int, vector <string> > bucketA;
map <int, vector <string> > bucketB;

//ofstream logfile ("FM.log");

vector<string> split(string strToSplit, char delimeter)
{
    stringstream ss(strToSplit);
    string item;
    vector<string> splittedStrings;
    while (getline(ss, item, delimeter))
    {
       splittedStrings.push_back(item);
    }
    return splittedStrings;
}

class cell
{
	public:
		int cell_size;
		int FS;
		int TE;
		int cell_gain;
		int lock_status;// 1=locked, 0=unlocked
		string cell_type;
		int cell_partition;  //A = 0,  B = -1
		//string cell_partition;
		vector <string> net_list;
		vector <string> cell_neighbours;
};

class net
{
	public:
	vector <string> cell_list;	
	vector <string> cell_listA;		
	vector <string> cell_listB;
	vector <string> cell_listA_mincut;
	vector <string> cell_listB_mincut;
	int cutstate; //0 = uncut, -1 = cut
	int Asize;
	int Bsize;
	int Asize_mincut;
	int Bsize_mincut;
	int lock_count;
	bool critical;
};

//class type global declaration 
map <string, cell> cell_map;
map <string, net> net_map;

 
//balance check function
bool balance_check(string cell_to_move)
{
	int temp_sumA = sumA;
	int temp_sumB = sumB;
	if (cell_map[cell_to_move].cell_partition == 0)
	{
		temp_sumA = temp_sumA - cell_map[cell_to_move].cell_size;
		temp_sumB = temp_sumB + cell_map[cell_to_move].cell_size;
	}
	else if (cell_map[cell_to_move].cell_partition == -1)
	{
		temp_sumA = temp_sumA + cell_map[cell_to_move].cell_size;
		temp_sumB = temp_sumB - cell_map[cell_to_move].cell_size;
	}
	
	if(abs(temp_sumA - temp_sumB) <= 3600 )
	{
		sumA = temp_sumA;
		sumB = temp_sumB;
		//logfile<<"later: "<<sumA<<"\t"<<sumB<<"\n";
		return true;
	}
	else
	{
		//logfile<<cell_to_move<<" violating balance_check\n";
		return false;
	}
}

void update_buckets(string &curr_cell, int &prev_gain)
{
	int new_gain;
	
	new_gain = cell_map[curr_cell].cell_gain;
	
	if(cell_map[curr_cell].cell_partition == 0)
	{
		bucketA[prev_gain].erase(remove(bucketA[prev_gain].begin(),bucketA[prev_gain].end(),curr_cell),bucketA[prev_gain].end());
		if(bucketA[prev_gain].empty())
		{
			gainA.erase(remove(gainA.begin(),gainA.end(),prev_gain),gainA.end());
			//logfile<<curr_cell<<" was LAST element while changing gains AND REMOVED, hence erasing key "<<prev_gain<<" from bucketA. New gain of "<<curr_cell<<" is "<<new_gain<<"\n";
			//logfile<<"new size of bucketA :"<<gainA.size()<<"\n";
		}
		//logfile<<curr_cell<<"'s gain was changed from "<<prev_gain<<" to "<<new_gain<<"\n";
		bucketA[new_gain].push_back(curr_cell);
		gainA.push_back(new_gain);
	}
	else if (cell_map[curr_cell].cell_partition == -1)
	{
		bucketB[prev_gain].erase(remove(bucketB[prev_gain].begin(),bucketB[prev_gain].end(),curr_cell),bucketB[prev_gain].end());
		if(bucketB[prev_gain].empty())
		{
			gainB.erase(remove(gainB.begin(),gainB.end(),prev_gain),gainB.end());
			//logfile<<curr_cell<<" was LAST element while changing gains AND REMOVED, hence erasing key "<<prev_gain<<" from bucketB. New gain of "<<curr_cell<<" is "<<new_gain<<"\n";
			//logfile<<"new size of bucketB :"<<gainB.size()<<"\n";
		}
		//logfile<<curr_cell<<"'s gain was changed from "<<prev_gain<<" to "<<new_gain<<"\n";
		bucketB[new_gain].push_back(curr_cell);
		gainB.push_back(new_gain);
	}
}

void update_gains(string &cell_to_move)
{
	int prev_gain;
	int new_gain;
	string curr_net;
	string curr_cell;
	
	for (i=0; i < cell_map[cell_to_move].net_list.size(); i++)
	{ 
		curr_net = cell_map[cell_to_move].net_list[i];
		/*net_map[curr_net].lock_count++;
		//checking criticality before the move
		if(net_map[curr_net].lock_count == net_map[curr_net].cell_list.size())
		{
		//		continue;
		}*/
		if(old_partition == 0)
		{
			cell_map[cell_to_move].FS = net_map[curr_net].Asize;
			cell_map[cell_to_move].TE = net_map[curr_net].Bsize;
		}
		else if(old_partition == -1)
		{
			cell_map[cell_to_move].FS = net_map[curr_net].Bsize;
			cell_map[cell_to_move].TE = net_map[curr_net].Asize;
		}
		if(cell_map[cell_to_move].TE == 0)
		{
			for(j =0; j<net_map[curr_net].cell_list.size(); j++)
			{
				curr_cell = net_map[curr_net].cell_list[j];
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain + 1;
					update_buckets(curr_cell, prev_gain);
				}
				
			}
		}
		else if(cell_map[cell_to_move].TE == 1)
		{
			if(cell_map[cell_to_move].cell_partition == 0)
			{
				curr_cell = net_map[curr_net].cell_listA[0];
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain - 1;
					update_buckets(curr_cell, prev_gain);
				}
			}
			else if(cell_map[cell_to_move].cell_partition == -1)
			{
				curr_cell = net_map[curr_net].cell_listB[0];
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain - 1;
					update_buckets(curr_cell, prev_gain);
				}
			}
		}
		//making the move
		cell_map[cell_to_move].FS = cell_map[cell_to_move].FS - 1;
		cell_map[cell_to_move].TE = cell_map[cell_to_move].TE + 1;
		//after moving
			//updating Asize and Bsize and cell lists for current net
		if(old_partition == -1)
		{
			net_map[curr_net].cell_listA.push_back(cell_to_move);
			net_map[curr_net].cell_listB.erase(remove(net_map[curr_net].cell_listB.begin(),net_map[curr_net].cell_listB.end(),cell_to_move),net_map[curr_net].cell_listB.end());
			net_map[curr_net].Asize = cell_map[cell_to_move].TE;
			net_map[curr_net].Bsize = cell_map[cell_to_move].FS;
			if(new_cutset < mincut)
			{
				net_map[curr_net].Asize_mincut = net_map[curr_net].Asize;
				net_map[curr_net].Bsize_mincut = net_map[curr_net].Bsize;
				net_map[curr_net].cell_listA_mincut.clear();
				net_map[curr_net].cell_listB_mincut.clear();
				net_map[curr_net].cell_listA_mincut = net_map[curr_net].cell_listA;
				net_map[curr_net].cell_listB_mincut = net_map[curr_net].cell_listB;
				//mincut = new_cutset;//happens only once for first net and then makes the condition false
			}
		}
		else if(old_partition == 0)
		{
			net_map[curr_net].cell_listB.push_back(cell_to_move);
			net_map[curr_net].cell_listA.erase(remove(net_map[curr_net].cell_listA.begin(),net_map[curr_net].cell_listA.end(),cell_to_move),net_map[curr_net].cell_listA.end());
			net_map[curr_net].Bsize = cell_map[cell_to_move].TE;
			net_map[curr_net].Asize = cell_map[cell_to_move].FS;
			if(new_cutset < mincut)
			{
				net_map[curr_net].Asize_mincut = net_map[curr_net].Asize;
				net_map[curr_net].Bsize_mincut = net_map[curr_net].Bsize;
				net_map[curr_net].cell_listA_mincut.clear();
				net_map[curr_net].cell_listB_mincut.clear();
				net_map[curr_net].cell_listA_mincut = net_map[curr_net].cell_listA;
				net_map[curr_net].cell_listB_mincut = net_map[curr_net].cell_listB;
				//mincut = new_cutset;
			}
		}
		
		//checking criticality after the move
		if (cell_map[cell_to_move].FS == 0)
		{
			for(j =0; j<net_map[curr_net].cell_list.size(); j++)
			{
				curr_cell = net_map[curr_net].cell_list[j];
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain - 1;
					update_buckets(curr_cell, prev_gain);
				}
			}
		}
		else if (cell_map[cell_to_move].FS == 1)
		{
			if(old_partition == 0)
			{
				curr_cell = net_map[curr_net].cell_listA[0]; //confirmation pending
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain + 1;
					update_buckets(curr_cell, prev_gain);
				}
			}
			else if(old_partition == -1)
			{
				curr_cell = net_map[curr_net].cell_listB[0];  //confirmation pending
				if(cell_map[curr_cell].lock_status == 0)
				{
					prev_gain = cell_map[curr_cell].cell_gain;
					cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain + 1;
					update_buckets(curr_cell, prev_gain);
				}
			}
			
		}
		
	}
	sort(gainA.rbegin(),gainA.rend());
	sort(gainB.rbegin(),gainB.rend());
	gainA.erase(unique(gainA.begin(),gainA.end()),gainA.end());
	gainB.erase(unique(gainB.begin(),gainB.end()),gainB.end());
	
	//logfile<<"size of buckeA: "<<gainA.size()<<" Size of gainA: "<<gainA.size()<<"\n";

	
}


//main function
int main (int argc, char* argv[]) 
{

  int partition = -1;
  int passes;
  int pass;
  string line;
  string curr_net;
  string curr_cell;
  string next_cell;
  string cell_to_move;
  string cell_to_check;
  vector <string> locked_cells;
  vector <string> all_cells;
  vector <string> all_nets;
  vector <string> files;
  vector <string> node;
  vector <string> net_line;
  vector <string> inst_line;
	
  srand(time(NULL));
	
	
	if(argc < 2)
	{
	  cout<<"please specify the input file name\n";
	  return 0;
	}
	cout<<"Enter the number of passes you want to make: ";
	cin>>passes;
	
	const char* arg = argv[1];
	ifstream inputfile(arg);
	if(inputfile.is_open())
	{
		while(getline (inputfile,line))
		{
		  if(line.find("design") != string::npos)
		  {
			  files = split(line, ' ');
		  }
		  
		}
		inputfile.close();
	}
  else cout<<"unable to open the inputfile";
	const char* file1 = files[2].c_str();
	const char* file2 = files[3].c_str();
		
  //cout<<"making "<<passes<<" passes on "<<files[2]<<" and "<<files[3]<<"\n";
  
  ifstream nodesfile (file1);
  //ofstream testfile	("testfile.csv");
  //system("date");
  ofstream resultfile ("Team09_output.txt");
  if (nodesfile.is_open())
  {
    while ( getline (nodesfile,line) )
    {
	node = split(line,' ');
	cell_map[node[0]].cell_type = node[1]; //updating cell type
	all_cells.push_back(node[0]);
	
	/*****update cell size*****/
	if(cell_map[node[0]].cell_type.compare("FDRE") == 0){cell_map[node[0]].cell_size = 5;}
	else if(cell_map[node[0]].cell_type.compare("LUT6") == 0){cell_map[node[0]].cell_size = 7;}
	else if(cell_map[node[0]].cell_type.compare("LUT5")== 0){cell_map[node[0]].cell_size = 6;}
	else if(cell_map[node[0]].cell_type.compare("LUT4") == 0){cell_map[node[0]].cell_size = 5;}
	else if(cell_map[node[0]].cell_type.compare("LUT3") == 0){cell_map[node[0]].cell_size = 4;}
	else if(cell_map[node[0]].cell_type.compare("LUT2") == 0){cell_map[node[0]].cell_size = 3;}
	else if(cell_map[node[0]].cell_type.compare("LUT1") == 0){cell_map[node[0]].cell_size = 2;}
	else if(cell_map[node[0]].cell_type.compare("CARRY8") == 0){cell_map[node[0]].cell_size = 34;}
	else if(cell_map[node[0]].cell_type.compare("DSP48E2") == 0){cell_map[node[0]].cell_size = 429;}
	else if(cell_map[node[0]].cell_type.compare("RAMB36E2") == 0){cell_map[node[0]].cell_size = 379;}
	else if(cell_map[node[0]].cell_type.compare("BUFGCE") == 0){cell_map[node[0]].cell_size = 3;}
	else if(cell_map[node[0]].cell_type.compare("IBUF") == 0){cell_map[node[0]].cell_size = 2;}
	else if(cell_map[node[0]].cell_type.compare("OBUF") == 0){cell_map[node[0]].cell_size = 2;}
	
	/*****Random Partitioning*****/
	
	if((rand()%2) == 1)
	{
		partition = 0;
	}
	else
	{
		partition = -1;
	}
	if(partition == 0) 
	{
		if((sumB + cell_map[node[0]].cell_size - sumA) <= 3600 )
		{
			partition = ~partition;
			partB:sumB = sumB + cell_map[node[0]].cell_size;
			setB.push_back(node[0]);
		}
		else
		{
			goto partA;
		}
		//partitionB <<node[0]<<"\n";
	}
	else if (partition == -1) 
	{
		if((sumA + cell_map[node[0]].cell_size - sumB) <= 3600 )
		{
			partition = ~partition;
			partA:sumA = sumA + cell_map[node[0]].cell_size;
			setA.push_back(node[0]);
		}
		else
		{
			goto partB;
		}
			//partitionA <<node[0]<<"\n";
	}
	
	cell_map[node[0]].cell_partition = partition;
	
	/*****Random Partitioning end****/
	
	
	cell_map[node[0]].lock_status = 0;				//Lock status intialization (All cells unlocked)
	
    }
    nodesfile.close();
  }
  else cout << "Unable to open file"; 
  
  setA_mincut = setA;
  setB_mincut = setB;
  
  ifstream netsfile (file2);
  if(netsfile.is_open())
  {
	  while (getline(netsfile,line))
	  {
		  if(line.find("net net_") != string::npos)
		  {
			net_line = split(line, ' ');
			all_nets.push_back(net_line[1]);
		  }
		  else if(line.find("net clk") != string::npos)
		  {
			 net_line = split(line, ' ');
			 all_nets.push_back(net_line[1]);
		  }
		  else if (line.find("net controlSig") != string::npos)
		  {
			 net_line = split(line, ' ');
			 all_nets.push_back(net_line[1]);
		  }
		  else if(line.find("endnet") != string::npos)
		  {
			  net_map[net_line[1]].Asize = net_map[net_line[1]].cell_listA.size();
			  net_map[net_line[1]].Bsize = net_map[net_line[1]].cell_listB.size();
			  //net_map[net_line[1]].lock_count = 0;
			  if((net_map[net_line[1]].Asize == 1)||(net_map[net_line[1]].Bsize == 1))
			  {
				  net_map[net_line[1]].critical = true;
			  }
			  if((net_map[net_line[1]].Asize != 0) && (net_map[net_line[1]].Bsize != 0))
			  {
				  cutset++;
				  net_map[net_line[1]].cutstate = -1;
			  }
			  else
			  {
				  net_map[net_line[1]].critical = true;
				  net_map[net_line[1]].cutstate = 0;
			  }
		  }
		  else
		  {
			 inst_line = split(line,' ');
			 inst_line = split(inst_line[0],'\t');
			 cell_map[inst_line[1]].net_list.push_back(net_line[1]);
			 net_map[net_line[1]].cell_list.push_back(inst_line[1]);
			 
			 if(cell_map[inst_line[1]].cell_partition == 0)
				{
					net_map[net_line[1]].cell_listA.push_back(inst_line[1]);
				}
			 else if (cell_map[inst_line[1]].cell_partition == -1)
				{
					net_map[net_line[1]].cell_listB.push_back(inst_line[1]);
				}
			 
		  }
		  
	  }
	  netsfile.close();

  }
  else cout << "Unable to open file"; 
  
  //resultfile<<"Starting cut: "<<cutset<<endl;
	startcut = cutset;
	mincut = cutset;
for(pass =1; pass<=passes; pass++)
{	
	
	if(pass>1)
	{
		cutset = mincut;
		sumA =0;
		sumB =0;
		locked_cells.clear();
		setA = setA_mincut;
		setB = setB_mincut;
		
		for(i=0; i<setA.size(); i++)
		{
			sumA = sumA + cell_map[setA[i]].cell_size;
			cell_map[setA[i]].cell_partition = 0;
			cell_map[setA[i]].lock_status = 0;
		}
		for(i=0; i<setB.size(); i++)
		{
			sumB = sumB + cell_map[setB[i]].cell_size;
			cell_map[setB[i]].cell_partition = -1;
			cell_map[setB[i]].lock_status = 0;
		}
		for(i=0; i<all_nets.size(); i++)
		{
			net_map[all_nets[i]].Asize = net_map[all_nets[i]].Asize_mincut;
			net_map[all_nets[i]].Bsize = net_map[all_nets[i]].Bsize_mincut;
			net_map[all_nets[i]].cell_listA = net_map[all_nets[i]].cell_listA_mincut;
			net_map[all_nets[i]].cell_listB = net_map[all_nets[i]].cell_listB_mincut;
			//net_map[all_nets[i]].lock_count =0;
		}
	}
	
/********Gain calculation*********/
 bucketA.clear();
 bucketB.clear();
 gainA.clear();
 gainB.clear();
 for(i=0;i<all_cells.size();i++)
 {
	curr_cell = all_cells[i];
	cell_map[curr_cell].cell_gain = 0;
	for (j=0; j < cell_map[curr_cell].net_list.size(); j++)
	{ 
		curr_net = cell_map[curr_cell].net_list[j];
		if(cell_map[curr_cell].cell_partition == 0)
		{
			cell_map[curr_cell].FS = net_map[curr_net].Asize;
			cell_map[curr_cell].TE = net_map[curr_net].Bsize;
		}
		else if(cell_map[curr_cell].cell_partition == -1)
		{
			cell_map[curr_cell].FS = net_map[curr_net].Bsize;
			cell_map[curr_cell].TE = net_map[curr_net].Asize;
		}
		if(cell_map[curr_cell].FS == 1)
		{
			cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain + 1;
		}
		if(cell_map[curr_cell].TE == 0)
		{
			cell_map[curr_cell].cell_gain = cell_map[curr_cell].cell_gain - 1;
		}
	}
	//----------------UPDATING BUCKETS-------------
	if(cell_map[curr_cell].cell_partition == 0)
	{
		bucketA[cell_map[curr_cell].cell_gain].push_back(curr_cell);
		gainA.push_back(cell_map[curr_cell].cell_gain);
	}
	else if (cell_map[curr_cell].cell_partition == -1)
	{
		bucketB[cell_map[curr_cell].cell_gain].push_back(curr_cell);
		gainB.push_back(cell_map[curr_cell].cell_gain);
	}
	//----------------UPDATING BUCKETS END---------
 }
/************Gain calculation end******************/ 
  
 sort(gainA.rbegin(),gainA.rend());
 sort(gainB.rbegin(),gainB.rend());
 gainA.erase(unique(gainA.begin(),gainA.end()),gainA.end());
 gainB.erase(unique(gainB.begin(),gainB.end()),gainB.end());
 indexA = 0;
 indexB = 0;
 deadA = false;
 deadB = false;
 
 /*******printing initial values********/
 /*
 resultfile<<"***********Start of pass "<<pass<<"*********\n";
 resultfile<<"sumA: "<<sumA<<"\t"<<"sumB: "<<sumB;
 resultfile<<"\n cutset: "<<cutset<<"\n";
 resultfile<<"size of bucketA: "<<bucketA.size()<<"\n";
 resultfile<<"size of bucketB: "<<bucketB.size()<<"\n";
*/
  LOOP:while(!deadA||!deadB)
  {
	if(gainA.empty())
	{
		deadA = true;
		//cout<<"bucket A is empty hence dead\n";
		if(indexB == gainB.size())
		{
			//cout<<"buckeA empty and bucketB is dead.. exiting.....\n";
			break;
		}
		cell_to_move = bucketB[gainB[indexB]].back(); 
	}
	else if(gainB.empty())
	{
		deadB = true;
		//cout<<"bucket B is empty hence dead\n";
		if(indexA == gainA.size())
		{
			//cout<<"buckeB empty and bucketA is dead.. exiting.....\n";
			break;
		}
		cell_to_move = bucketA[gainA[indexA]].back();
	}
	else
	{
		if(gainA[indexA] > gainB[indexB])
		{ 	
		  A:if(deadA)//last key of bucket A
			{
				if(!deadB)
				{
					goto B;
				}
				else
				{
					//cout<<"buckeA dead and bucketB dead.. exiting.....\n";
					break;
				}
			}
			cell_to_move = bucketA[gainA[indexA]].back();
		}
		else
		{
		  B:if(deadB)//last key of bucket B
			{
				if(!deadA)
				{
					goto A;
				}
				else
				{
					//cout<<"buckeB dead and bucketA dead.. exiting.....\n";
					break;
				}
			}
			cell_to_move = bucketB[gainB[indexB]].back();
		}
	}
	
	while(!balance_check(cell_to_move))
	{
		//logfile<<cell_to_move<<" of size "<<cell_map[cell_to_move].cell_size<<" and gain "<<cell_map[cell_to_move].cell_gain<<", picked and not moved from "<<cell_map[cell_to_move].cell_partition<<"\n";
		if(cell_map[cell_to_move].cell_partition == 0)
		{
			if(cell_to_move.compare(bucketA[gainA[indexA]].front()) == 0 )
			{
				//increamnet index for gainA (select next 2nd highest and so on)
				indexA++;
				//logfile<<"Incrementing indexA. Index A: "<<indexA<<"\n";
				new_index = 0;
				if(indexA == gainA.size())
				{
					//cout<<"last element from bucket A cannot move hence dead\n";
					deadA = true;
					indexA--;
					//logfile<<"*************\n\ndecremented indexA to avoid SEGMENTATION FAULT\n*************";
				}
				goto LOOP;
			}
			new_index++;
			//logfile<<"new_index = "<<new_index<<"\n";
			cell_to_move = *(&bucketA[gainA[indexA]].back()-new_index);
			
		}
		else if (cell_map[cell_to_move].cell_partition == -1)
		{
			
			if(cell_to_move.compare(bucketB[gainB[indexB]].front()) == 0 )
			{
				//increamnet index for gainB (select next 2nd highest and so on)
				indexB++;
				//logfile<<"Incrementing indexB. index B: "<<indexB<<"\n";
				new_index = 0;
				if(indexB == gainB.size())
				{
					//cout<<"last element from bucket B cannot move hence dead\n";
					deadB = true;
					indexB--;
					//logfile<<"*************\ndecremented indexB to avoid SEGMENTATION FAULT\n*************";
				}
				goto LOOP;
			}
			new_index++;
			//logfile<<"new_index = "<<new_index<<"\n";
			cell_to_move = *(&bucketB[gainB[indexB]].back()-new_index);
		}
	}
	
	//logfile<<cell_to_move<<" of size "<<cell_map[cell_to_move].cell_size<<" and gain "<<cell_map[cell_to_move].cell_gain<<", picked\n";
	new_index = 0;
	indexA = 0;
	indexB = 0;
	cell_map[cell_to_move].lock_status = 1; 
	locked_cells.push_back(cell_to_move);
	new_cutset = cutset - cell_map[cell_to_move].cell_gain;
	//cout<<cell_to_move<<"\t"<<"|"<<"\t"<<new_cutset<<"\n";
	//testfile<<cell_to_move<<","<<new_cutset<<"\n";
	if(cell_map[cell_to_move].cell_partition == 0)
		{
			//logfile<<"it was moved from partition A\n";
			//Remove the cell_to_move from bucketA
			bucketA[cell_map[cell_to_move].cell_gain].erase(remove(bucketA[cell_map[cell_to_move].cell_gain].begin(),bucketA[cell_map[cell_to_move].cell_gain].end(),cell_to_move),bucketA[cell_map[cell_to_move].cell_gain].end());
			setA.erase(remove(setA.begin(),setA.end(),cell_to_move), setA.end());
			setB.push_back(cell_to_move);
			//logfile<<"size of bucketA["<<cell_map[cell_to_move].cell_gain<<"]: "<<bucketA[cell_map[cell_to_move].cell_gain].size()<<"\n";
			if(bucketA[cell_map[cell_to_move].cell_gain].empty())
			{
				gainA.erase(remove(gainA.begin(),gainA.end(),cell_map[cell_to_move].cell_gain),gainA.end());
				//logfile<<cell_to_move<<" was LAST element AND REMOVED, hence erasing key "<<cell_map[cell_to_move].cell_gain<<" from bucketA\n";
				//logfile<<"new size of bucketA :"<<gainA.size()<<"\n";
			}
			if(new_cutset<mincut)
			{
				setA_mincut = setA;
				setB_mincut = setB;
				//mincut = new_cutset;
			}
			
		}
	else if (cell_map[cell_to_move].cell_partition == -1)
		{
			//logfile<<"it was moved from partition B\n";
			//Remove the cell_to_move from bucketB
			bucketB[cell_map[cell_to_move].cell_gain].erase(remove(bucketB[cell_map[cell_to_move].cell_gain].begin(),bucketB[cell_map[cell_to_move].cell_gain].end(),cell_to_move),bucketB[cell_map[cell_to_move].cell_gain].end());
			setB.erase(remove(setB.begin(),setB.end(),cell_to_move), setB.end());
			setA.push_back(cell_to_move);
			//logfile<<"size of bucketB["<<cell_map[cell_to_move].cell_gain<<"]: "<<bucketB[cell_map[cell_to_move].cell_gain].size()<<"\n";
			if(bucketB[cell_map[cell_to_move].cell_gain].empty())
			{
				gainB.erase(remove(gainB.begin(),gainB.end(),cell_map[cell_to_move].cell_gain),gainB.end());
				//logfile<<cell_to_move<<" was LAST element AND REMOVED, hence erasing key "<<cell_map[cell_to_move].cell_gain<<" from bucketB\n";
				//logfile<<"new size of bucketB :"<<gainB.size()<<"\n";
			}
			if(new_cutset<mincut)
			{
				
				setA_mincut = setA;
				setB_mincut = setB;
				//mincut = new_cutset;
			}
		}
		
	cutset = new_cutset;
	//logfile <<"cutset: "<<cutset<<"\n";
	//testfile <<cutset<<"\n";
	old_partition = cell_map[cell_to_move].cell_partition;
	cell_map[cell_to_move].cell_partition = ~cell_map[cell_to_move].cell_partition;
	
	if(cell_map[cell_to_move].cell_partition == 0)
	{
		deadA = false;
	}
	else if (cell_map[cell_to_move].cell_partition == -1)
	{
		deadB = false;
	}	
	
	update_gains(cell_to_move); //update gains of the neigbours of cell_to_move
	
	if(new_cutset < mincut)
	{
		mincut = new_cutset;
		sumA_mincut = sumA;
		sumB_mincut = sumB;
	}
	//logfile<<"number of locked cells: "<<locked_cells.size()<<"\n";
  }
  /*
  resultfile<<"****************End of pass "<<pass<<"********************\n";
  resultfile<<"number of locked cells: "<<locked_cells.size()<<"\n";
  resultfile<<"best cutset corresponds to: "<<mincut<<"\n";
  resultfile<<"sumA_mincut: "<<sumA_mincut<<"\tsumB_mincut: "<<sumB_mincut<<endl;
  resultfile <<"sizes: "<<sumA<<"\t"<<sumB<<"\n";
  */
  //system("date");
  //logfile.close();
}
//testfile.close();
float per = ((startcut-mincut)/(float)startcut)*100;
  resultfile<<"Starting cut: "<<startcut<<endl;
  resultfile<<"Final cut: "<<mincut<<endl;
  resultfile<<"percentage change: "<<per<<endl;
  resultfile.close();
  return 0;
}

