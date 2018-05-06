/*******************************************************************************
 * MC658 - Projeto e Análise de Algoritmos III - 1s2018
 * Prof: Flavio Keidi Miyazawa
 * PED: Francisco Jhonatas Melo da Silva
 * Usa ideias e código de Mauro Mulati e Flávio Keidi Miyazawa
 ******************************************************************************/

/*******************************************************************************
 * EDITE ESTE ARQUIVO APENAS ONDE INDICADO
 * DIGITE SEU RA: 177312
 * SUBMETA SOMENTE ESTE ARQUIVO
 ******************************************************************************/
 #include <iostream>
 #include <float.h>
#include "pmr_e_h.h"
#include <signal.h>
#include <unistd.h>
#include "gurobi_c++.h"
//typedef vector<vector<double> > matriz;

volatile sig_atomic_t got_interrupt = 0;
static void alarm_handler(int sig)
{
	got_interrupt = 1;
}

int quantItens_g;
int* s_g;
int* v_g;
double** relation_array;
int* itensMochila_array;
int* itensMochila_array_partial;
int* itensMochila_array_partial_id;
int itensMochila_array_partial_count = 0;
int max_partial = 0;

/* Parâmetros das funções
	capacity 		capacidade da mochila
	quantItens		N itens
	relation[i][j] 	valor de relações
	v[i] 			valor do item
	s[i] 			peso do item
	itensMochila[] 	resposta
	maxTime 		timeout em segundos
*/

void algE_rec(int i, int capacity, int partial_val)
{
    int id;
    if (i == quantItens_g || capacity == 0 || got_interrupt)
        return;

    // Analise without me
    algE_rec(i+1, capacity, partial_val);

    // If I fit
    capacity -= s_g[i];
    if (capacity >= 0) {
        itensMochila_array_partial[i] = 1;
        itensMochila_array_partial_id[itensMochila_array_partial_count++] = i;

        // Adds the relation values between items in the partial_items array and `i`, and `i` value itself
        for (int j = 0; j < itensMochila_array_partial_count-1; j++) {
            id = itensMochila_array_partial_id[j];
            if (itensMochila_array_partial[id])
                partial_val += relation_array[i][id];
        }
        partial_val += v_g[i];

        // save if better
        if (partial_val > max_partial) {
            max_partial = partial_val;
            for (int j = 0; j < quantItens_g; j++) {
                itensMochila_array[j] = itensMochila_array_partial[j];
            }
        }

        // Analise with me
        algE_rec(i+1, capacity, partial_val);
        itensMochila_array_partial[i] = 0;
        itensMochila_array_partial_count--;
    }
}

int algE(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    got_interrupt = false;
    signal(SIGALRM, alarm_handler);
    alarm(maxTime);

    s_g = &s[0];
    v_g = &v[0];
    itensMochila_array = (int*)calloc(quantItens, sizeof(int));
    itensMochila_array_partial = (int*)calloc(quantItens, sizeof(int));
    itensMochila_array_partial_id = (int*)calloc(quantItens, sizeof(int));
    relation_array = (double**)malloc(quantItens * sizeof(double*));
    for (int i = 0; i < quantItens; i++) {
        relation_array[i] = &relation[i][0];
    }
    max_partial = 0;
    quantItens_g = quantItens;

    algE_rec(0, capacity, 0);
    // copy response
    for (int i = 0; i < quantItens; i++)
        itensMochila[i] = itensMochila_array[i];

    free(itensMochila_array);
    free(itensMochila_array_partial);
    free(itensMochila_array_partial_id);
    free(relation_array);
    return max_partial;
}

// get the id for the biggest v[id] that fits
int get_best_id(int capacity) {
	double curr_v, max_v = 0;
	int max_v_i = -1;
	for	(int i = 0; i < quantItens_g; i++) {
		// skip if already in solution
		if (itensMochila_array_partial[i])
			continue;

        curr_v = (double)v_g[i] / s_g[i];
        if (s_g[i] <= capacity && curr_v > max_v) {
            max_v = curr_v;
            max_v_i = i;
        }		
	}

	return max_v_i;
}

void algH_array(int &capacity) {
	int best_id, j;
	while (capacity) {
		if (got_interrupt)
			return;

		// Put the best into solution
		best_id = get_best_id(capacity);
		if (best_id == -1)
			return;

		capacity -= s_g[best_id];
		max_partial += v_g[best_id];
		itensMochila_array_partial[best_id] = 1;

		// update v[j] for all j outside of the solution with relation[best_id][j] value
        for (j=0; j < quantItens_g; j++)
            if (!itensMochila_array_partial[j])
                v_g[j] += relation_array[best_id][j];
	}
}

int algH(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    quantItens_g = quantItens;
	s_g = &s[0];
    v_g = &v[0];
    itensMochila_array_partial = (int*)calloc(quantItens, sizeof(int));
    relation_array = (double**)malloc(quantItens * sizeof(double*));
    for (int i = 0; i < quantItens; i++) {
        relation_array[i] = &relation[i][0];
    }

    got_interrupt = false;
    signal(SIGALRM, alarm_handler);
    alarm(maxTime);

	algH_array(capacity);

	for (int j = 0; j < quantItens_g; j++) {
		itensMochila[j] = itensMochila_array_partial[j];
	}
    free(itensMochila_array_partial);
    free(relation_array);
    

	return max_partial;
}

int algExato(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime)
/*******************************************************************************
 * SUBSTITUIA O CONTEÚDO DESTE MÉTODO POR SUA IMPLEMENTAÇÃO EXATA USANDO O GUROBI.
 * ENTRETANTO, NÃO ALTERE A ASSINATURA DO MÉTODO.
 ******************************************************************************/
{
	return 0;
}
