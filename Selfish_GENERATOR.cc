#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cstring>
using namespace std;

int main()
{
	int NSelfish; //numero de nós egoístas
	int INTERVALO; //intervalo de geração de nós egoístas;
	srand(time(NULL)); //evitar vício da semente 
	cout <<"\nDigite a quantidade de nós egoístas: ";
		cin>>NSelfish;
	cout <<"\nDigite o intervalo de distribuíção: ";
		cin>>INTERVALO;
		int val[NSelfish]; //matriz para armazenar os números gerados
		ofstream grava("Selfish.tcl", ios::out);
			if(!grava){
				cerr<<"Erro: não é possível abrir o arquivo!!!"<<endl;
				exit(1);
			}
		string s="$ns_ at 0.0 \"[$node(";
		string c=") set ragent_] egoista_on\"";
		for (int i=0;i<NSelfish;i++)
		{
			bool check; //verifica se o numero já foi usado
			int n; //var p/ armazenar o rand
			do
			{
				n=rand()%INTERVALO;
				check=true;
				for (int j=0;j<i;j++)
					if (n == val[j]) //se o número já foi sorteado
					{
						check=false; //seta verifica para falso
						break; //não precisa verificar outros elementos da matriz val[]
					}
			} while (!check); //continua o loop até um novo numero ser achado
		val[i]=n; //guarda o número na matriz
		grava<<s<<val[i]<<c<<endl;
		}
		grava.close();
		return 0;
}
