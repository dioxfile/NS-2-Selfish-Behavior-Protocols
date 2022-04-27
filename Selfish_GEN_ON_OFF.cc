#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>
#include <stdlib.h> 
#include <sstream>

using namespace std;
int main(int argc, char* argv[])
{ 
  if(argc !=6)
		cout<<"USAGE: ./<PROGRAM> <Selfish Range> <Total Nodes (eg., 10, 20,...,n, n+1)> <Simul. Time> <Node Time offline> <Traffic File>!"<<endl;
  else {
		int NSelfish; //Selfish Node Number
		int INTERVALO; //Range to Generate Selfish Nodes;
		float time_ini;
		float off_time;
		float R_3;
		float Simulation_Time;
                std::stringstream ss; //Receive traffic file as a parameter
		srand(time(NULL)); //Avoids biased seed distribution 
		NSelfish = atoi(argv[1]);
		INTERVALO = atoi(argv[2]);
		Simulation_Time = atof(argv[3]);
		off_time = atof(argv[4]); 
		string traffic_file = argv[5];
		int val[NSelfish]; //matrix to store the selfish nodes generated
		ofstream grava("Selfish_On_Off.tcl", ios::out);
		if(!grava)
			{ cerr<<"Error: cannot open the file!!!"<<endl;
			  exit(1);}
		string s="$ns_ at "; 
		string n_=" \"$node_(";
		string c_off=") off\"";
		string c_on=") on\"";
        
        ///Verify who are the TX-RX nodes
        ss << "./SEL_DASH.sh " << traffic_file;
        system(ss.str().c_str());
	string line;
        int elements[100];
        int num_line = 0;
        ifstream myfile ("TX_RX_Nodes.txt");
        if (myfile.is_open())
        {
           while ( getline (myfile,line) )
           { 
             elements[num_line] = stoi(line);
             num_line++; 
	       }
        myfile.close();
        } else cout << "Unable to open file"; 
        ///
		
		for (int i=0;i<NSelfish;i++)
			{ bool check; //verify if this node already used, drawn.
			int n; //var for store the selfish node generated randomly.
			do
			  { n=rand()%INTERVALO;
				check=true;
				for (int j=0;j<i;j++){
					if (n == val[j]) //verify is this selfish node already drawn
						{ check=false; 
						break;}
				}
				///Test to verify that transmitting nodes
				for (int k=0;k< num_line ;k++){
					if(n==elements[k])
					{   check=false;
						break;
					}
				}
				///	
			  } while (!check); //continues the loop until a new selfish node is found   
				
			val[i]=n; //saves the new selfish node in the verification matrix
			R_3 = ((Simulation_Time*80)/100);
			time_ini = float(rand()%int(R_3));
			if(time_ini< 10)
				time_ini=10;
			grava<<s<<time_ini<<n_<<val[i]<<c_off<<endl;
			if((time_ini+off_time) > Simulation_Time)
				grava<<s<<Simulation_Time<< n_<<val[i]<<c_on<<endl;
			else
				grava<<s<<float(time_ini+off_time)<<n_<<val[i]<<c_on<<endl;
			string Color1="$ns_ at 0.0 \"$node_(";
			string Color2=") color red\"";
			grava<<Color1<<val[i]<<Color2<<endl;}	  
			grava.close();
			myfile.close();
	}
   return 0;
}
