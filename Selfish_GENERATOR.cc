#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>
using namespace std;
int main(int argc, char* argv[])
{ 
  if(argc !=3)
   cout<<"USAGE: ./<PROGRAM> <Nº Selfish Nodes> <Node Interval (eg., 10, 20, 30, 50,...,n, n+1)>"<<endl;
  else {
    int NSelfish; //Selfish Number
    int INTERVALO; //Interval to Generate Selfish Nodes
    srand(time(NULL)); //Avoid Seed addiction 
    NSelfish = atoi(argv[1]);
    INTERVALO = atoi(argv[2]);
    int val[NSelfish]; //array to store the generated numbers
    ofstream grava("Selfish.tcl", ios::out);
    if(!grava){
        cerr<<"Error: Isn't possible open file!!!"<<endl;
        exit(1);}
    string s="$ns_ at 0.0 \"[$node(";
    string c=") set ragent_] egoista_on\"";//Selfish in Portuguese 
    for (int i=0;i<NSelfish;i++){
        if(NSelfish > INTERVALO){
            cout<<"The number of selfish nodes can not be greater than interval!!!"<<endl;
            break; 
        } else {
            bool check;//check if the number has already been used
            int n;//Save rand number
            do {
                n=rand()%INTERVALO;
                check=true;
                for (int j=0;j<i;j++)
                    if (n == val[j]) 
                        { check=false; break; }
            } while (!check); 
            val[i]=n;//save the number in the array
            grava<<s<<val[i]<<c<<endl;
            string Color1="$ns_ at 0.0 \"$node(";
            string Color2=") color red\"";
            grava<<Color1<<val[i]<<Color2<<endl;
        }
    } grava.close();
  }
return 0; }

