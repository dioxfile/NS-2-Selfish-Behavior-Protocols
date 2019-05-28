#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>
using namespace std;

int main()
{
	int NSelfish; //Selfish Number
	int INTERVALO; //Interval to Generate Selfish Nodes
	srand(time(NULL)); //Avoid Seed addiction 
	cout <<"\nEnter the number of selfish nodes: ";
		cin>>NSelfish;
	cout <<"\nType the distribution interval: ";
		cin>>INTERVALO;
		int val[NSelfish]; //array to store the generated numbers
		ofstream grava("Selfish.tcl", ios::out);
			if(!grava){
				cerr<<"Error: Isn't possible open file!!!"<<endl;
				exit(1);
			}
		string s="$ns_ at 0.0 \"[$node(";
		string c=") set ragent_] egoista_on\""; //'Egoísta' is selfish in Portuguese
		for (int i=0;i<NSelfish;i++)
		{
			bool check; //check if the number has already been used
			int n; //Save rand number
			do
			{
				n=rand()%INTERVALO;
				check=true;
				for (int j=0;j<i;j++)
					if (n == val[j]) //if the number has already been seeded
					{
						check=false;
						break;
					}
			} while (!check); //continue the loop until a new number is found
		val[i]=n; //save the number in the array
		grava<<s<<val[i]<<c<<endl;
		}
		grava.close();
		return 0;
}
