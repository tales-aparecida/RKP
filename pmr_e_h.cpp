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
#include "pmr_e_h.h"
#include <signal.h>
#include <unistd.h>
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

void sum(int &partial_val, int i) {
    // Sum relations
    // for (int j = 0; j < i; j++) {
    //     if (itensMochila_array_partial[j])
    //         partial_val += relation_array[i][j];
    // }
    // partial_val += v_g[i];
    int id;
    for (int j = 0; j < itensMochila_array_partial_count; j++) {
        id = itensMochila_array_partial_id[j];
        if (itensMochila_array_partial[id])
            partial_val += relation_array[i][id];
    }
    partial_val += v_g[i];
}

void copy(int &partial_val) {
    if (partial_val > max_partial) {
        max_partial = partial_val;
        for (int j = 0; j < quantItens_g; j++) {
            itensMochila_array[j] = itensMochila_array_partial[j];
        }
    }
}

void algH_array(int i, int capacity, int partial_val)
{
    if (i == quantItens_g)
        return;

    // Analise without me
    algH_array(i+1, capacity, partial_val);

    // If I fit
    capacity -= s_g[i];
    if (capacity >= 0) {
        itensMochila_array_partial[i] = 1;
        itensMochila_array_partial_id[itensMochila_array_partial_count++] = i;

        sum(partial_val, i);

        copy(partial_val);

        // Analise with me
        algH_array(i+1, capacity, partial_val);
        itensMochila_array_partial[i] = 0;
        itensMochila_array_partial_count--;
    }
}

int algH(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
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

    algH_array(0, capacity, 0);
    // copy response
    for (int i = 0; i < quantItens; i++)
        itensMochila[i] = itensMochila_array[i];

    free(itensMochila_array);
    free(itensMochila_array_partial);
    return max_partial;
}

int _algH(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime)
{
	/*
	capacity 		capacidade da mochila
	quantItens		N itens
	relation[i][j] 	valor de relações
	v[i] 			valor do item
	s[i] 			peso do item
	itensMochila[] 	resposta
	maxTime 		timeout
	*/
	bool carry_in, carry_out = 0, s_overflow;
	int s_sum, max_v_sum = 0, v_sum;
	vector<int> items_try = vector<int>(quantItens);

	got_interrupt = false;
    signal(SIGALRM, alarm_handler);
  	alarm(maxTime);

	// Clear
	for (int i=quantItens-1; i >= 0; i--) {
		itensMochila[i] = 0;
		items_try[i] = 0;
	}

	// For each combination
	while (!carry_out && !got_interrupt) {
		// Act like a ripple carry adder to generate a new combination
		carry_in = true;
		for (int i=quantItens-1; i >= 0 && carry_in; i--) {
			carry_out = items_try[i] & carry_in;
			items_try[i] = items_try[i] ^ carry_in;
			carry_in = carry_out;
		}
		if (carry_out)
			return max_v_sum;

		// Try the combination
		s_sum = 0;
		v_sum = 0;
		s_overflow = false;
		for (int i=quantItens-1; i >= 0 && !s_overflow; i--) {
			// Return partial result on timeout
			if (got_interrupt)
				return max_v_sum;

			// If item is on set, add its weight and its relations
			if (items_try[i]) {
				s_sum += s[i];
				s_overflow = s_sum > capacity;
				// If it didn't overflown yet
				if (!s_overflow) {
					// Add value
					v_sum += v[i];

					// Add relation values
					for (int j=quantItens-1; j > i; j--) {
						if (items_try[j])
							v_sum += relation[i][j];
					}
				}
			}

		}

		// save the last best
		if (!s_overflow && v_sum > max_v_sum) {
			max_v_sum = v_sum;
			for (int i=quantItens-1; i >= 0; i--) {
				itensMochila[i] = items_try[i];
			}
		}

		// Return partial result on timeout
		if (got_interrupt)
			return max_v_sum;
	}

	return max_v_sum;
}

int _algE(int &capacity, int &quantItens, int s[], int v[], double** relation, int* itensMochila, int &maxTime)
{
	/*
	capacity 		capacidade da mochila
	quantItens		N itens
	relation[i][j] 	valor de relações
	v[i] 			valor do item
	s[i] 			peso do item
	itensMochila[] 	resposta
	maxTime 		timeout
	*/
	int s_sum, max_v_sum = 0, v_sum, i, j;
	int items_try[quantItens];

	got_interrupt = false;
    signal(SIGALRM, alarm_handler);
  	alarm(maxTime);

	// Clear
	for (i=quantItens-1; i >= 0; i--) {
		itensMochila[i] = 0;
		items_try[i] = 0;
	}

	i = 0;
	s_sum = 0;
	v_sum = 0;
	while (i >= 0) {
		// Try to toggle one bit 1 forward
		for (; i < quantItens; i++) {
			// Return partial result on timeout
			if (got_interrupt)
				return max_v_sum;

			if (s_sum + s[i] <= capacity) {
				items_try[i] = true;

				s_sum += s[i];
				// Add product value
				v_sum += v[i];
				// Add relation values
				for (j=i-1; j >= 0; j--) {
					if (items_try[j])
						v_sum += relation[i][j];
				}

				// Save if better
				if (v_sum > max_v_sum) {
					max_v_sum = v_sum;
                    for (j = 0; j < quantItens; j++) {
                        itensMochila[j] = items_try[j];
                    }
				}
			}
		}

		// Return to the last 1
		for (i=i-1; i >= 0 && !items_try[i]; i--);

		// Toggle to 0
		if (i >= 0) {
			items_try[i] = false;

			// Update values without it
			s_sum -= s[i];
			v_sum -= v[i];
			for (j=i-1; j >= 0; j--) {
				if (items_try[j])
					v_sum -= relation[i][j];
			}
			i++;
		}
	}

	return max_v_sum;
}

int algE(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime)
{
    double* mat[quantItens];
    int res[quantItens];
    for (int i = 0; i < quantItens; i++)
        mat[i] = &relation[i][0];
    int r = _algE(capacity, quantItens, (int*)&s[0], (int*)&v[0], mat, res, maxTime);
    for (int i = 0; i < quantItens; i++) {
        itensMochila[i] = res[i];
    }
    return r;
}
