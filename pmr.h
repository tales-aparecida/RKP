/*******************************************************************************
 * MC658 - Projeto e Análise de Algoritmos III - 1s2018
 * Prof: Flavio Keidi Miyazawa
 * PED: Francisco Jhonatas Melo da Silva
 * Usa ideias e código de Mauro Mulati e Flávio Keidi Miyazawa 
 ******************************************************************************/


/* ATENÇÃO: NÃO ALTERE ESTE ARQUIVO */

#ifndef PMR_H
#define PMR_H



#include<string>
#include<vector>
#include<algorithm>
#include<set>
#include<numeric>
#include<iostream>
#include<map>
#include<limits>
using namespace std;
typedef vector<vector<double> > matriz;

// Usage information
void showUsage();
void show_input(int C, int n, vector<int> s, vector<int> v, matriz &relation, int exec);
bool is_feasible_solution(int valueOpt, vector<int> itensMochila, int C, int n, vector<int> s, vector<int> v, matriz &relation);
void read_input(string input_file, int* C, int* quantItens, vector<int>& s, vector<int>& v, matriz &relation);

#endif
