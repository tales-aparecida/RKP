/*******************************************************************************
 * MC658 - Projeto e Análise de Algoritmos III - 1s2018
 * Prof: Flavio Keidi Miyazawa
 * PED: Francisco Jhonatas Melo da Silva
 * Usa ideias e código de Mauro Mulati e Flávio Keidi Miyazawa 
 ******************************************************************************/

/*******************************************************************************
 * ATENÇÃO: NÃO ALTERE ESTE ARQUIVO
 ******************************************************************************/

#include "pmr.h"
#include "pmr_e_h.h"
#include <signal.h>
#include<fstream>

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	// Variables to be obtained from parameters
	int    exec    = 0;      // 0: not set, 1: e, 2: h
	int    maxTime = 0;      // 0: not set
   bool verbose   = false;
   string inputFile_name;   // Input test file
   string outputFile_name;  // Output sol file
	
	// Reading program arguments
   for(int i = 1; i < argc; i++){
      const string arg(argv[i]);
      string next;
      if((i+1) < argc){
         next = string(argv[i+1]);
		}
      else{
         next = string("");
		}
		
      if( exec != 0 && (arg.find("-k") == 0 || arg.find("-a") == 0 ) ){
         cout << "Erro ao ler parametro \"" << arg << "\", somente pode haver um parametro de modo de execucao." << endl;
         showUsage();
         exit(1);
      }
      else if( arg.find("-e") == 0 ){
         exec = 1;
      }
      else if( arg.find("-h") == 0 ){
         exec = 2;
      }
      else if( arg.find("-v") == 0 ){
        verbose = true;
      }
      else if( arg.find("-t") == 0 && next.size() > 0){
         maxTime = atoi(next.c_str()); i++; continue;
      }
      else if( arg.find("-i") == 0 && next.size() > 0){
         inputFile_name = next; i++; continue;
      }
      else if( arg.find("-o") == 0 && next.size() > 0){
         outputFile_name = next; i++; continue;
      }
      else{
         cout << "Parametro invalido: \"" << arg << "\"" << " (ou faltando argumento)" << endl;
         showUsage();
         exit(1);
      }
   }

   // Required parameters
   if( exec == 0 || exec > 2){
      cout << "Nenhum modo de execucao selecionado dentre: -e ou -h" << endl;
      showUsage(); 
		exit(1);
   }
   
   if( inputFile_name.size() < 1 ){
      cout << ((inputFile_name.size() < 1)? "nome do arq, ":"") 
			  << endl;
      showUsage(); 
		exit(1);
   }

   if( maxTime == 0 ){
		maxTime = 60;  // Default of 60s = 1m
   }

	// int seed=1;     // mhmulati
	// srand48(seed);  // mhmulati
	
	// Variables that represent the input of the problem
	int capacity;
	int quantItens;
	vector<int> s;	//sizes
    	vector<int> v;	//values	
	matriz relation;

	read_input(inputFile_name, &capacity, &quantItens, s, v, relation);
	
	vector<int> itensMochila(quantItens);
	
	show_input(capacity, quantItens, s, v, relation, exec);
	
   double elapsed_time = numeric_limits<double>::max();
   clock_t before = clock();
   int OptimalSolution = 0;
	
	switch(exec){
		case 1:{
			OptimalSolution = algE(capacity, quantItens, s, v, relation, itensMochila, maxTime);
			break;
		}
		case 2:{
			OptimalSolution = algH(capacity, quantItens, s, v, relation, itensMochila, maxTime);
			break;
		}
	}
	
	cout << "valor encontrado: "<< OptimalSolution << endl;
   clock_t after = clock();
   elapsed_time = (double) (after-before) / CLOCKS_PER_SEC;
    cout << elapsed_time << endl;
    
	// Verificar se é de fato uma solução para a instância
	if( !is_feasible_solution(OptimalSolution, itensMochila, capacity, quantItens, s, v, relation))
	    cout << "Infeasible solution" << endl;
	
	return 0;
}


void read_input(string input_file, int* C, int* quantItens, vector<int>& s, vector<int>& v, matriz &relation){
	ifstream kinput;
	kinput.open(input_file.c_str()); if (!kinput) return ;
	kinput >> quantItens[0] >> C[0];
	int i, j;
	for(i=0; i<quantItens[0]; i++){
		int x, y;
		kinput >> x >> y;
		s.push_back(x); v.push_back(y);
	}
	//int relation[quantItens[0]][quantItens[0]];
	relation.resize(quantItens[0]);
	for(i=0; i< quantItens[0]; i++){
	    relation[i].resize(quantItens[0]);
	    for(j=0; j<quantItens[0];j++){
	        kinput >> relation[i][j];  
	    }
	}
	
}


void showUsage()
// Usage information
{
	cout << "Usage:"
	     << "./pmr -i <nome_arquivo_entrada> -o <nome_arquivo_saida> <modo_operacao> (um dentre: -e exato, -h heuristica) -t <tempo_max_em_segundos>"
	     << endl;
}


void show_input(int C, int n, vector<int> s, vector<int> v, matriz &relation, int exec){
//	cout << "Input file: " << input_file;
    int i,j;
	cout << n << " " << C << endl;
	for(i=0; i<n; i++)
		cout<<s[i]<<" "<<v[i]<<endl;
	
	for(i=0; i<n; i++){
	    for(j=0; j<n; j++){
	        cout << relation[i][j] <<  " ";
	    }
	    cout << endl;
	}
	cout << "Using " << (exec==1?"exato":"heuristica") << endl;
	
}

// returns true if sol is feasible
bool is_feasible_solution(int valueOpt, vector<int> itensMochila, int C, int n, vector<int> s, vector<int> v, matriz &relation){
	//set<int> nc;
	int weight = 0;
	int val = 0;
	
	for(int i=0; i<n; i++){
		if ( itensMochila[i] == 1){ 
		    weight += s[i];
		    val += v[i];
		    for(int j=i; j<n; j++){
		        if(itensMochila[j] == 1){
		            val += relation[i][j];
		        }
		    }
		}
		
	}
	
	if ( (weight > C) || (val != valueOpt)){
		cout << weight << " " << val << endl;
		return false;
	}
	cout << "Feasible solution" << endl;
	return true;
}
