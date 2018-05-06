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
static void alarm_handler(int sig) {
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

void algE_rec(int i, int capacity, int partial_val) {
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

// get the id for the biggest v[id] that fits
int get_best_id(int capacity) {
    double curr_v, max_v = 0;
    int max_v_i = -1;
    for (int i = 0; i < quantItens_g; i++) {
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

int algExato(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    double total_value;

    double valor_h = algH(capacity, quantItens, s, v, relation, itensMochila, maxTime);
    cout << valor_h << endl;
    for (int i=0 ; i<quantItens ; i++)
        itensMochila[i] = 0;

    try {
        vector<GRBVar> x(quantItens);
        vector< vector<GRBVar> > y(quantItens);
        try {
            GRBEnv env = GRBEnv();
            GRBModel model = GRBModel(env);
            GRBLinExpr expr;
            model.set(GRB_StringAttr_ModelName, "Relational Knapsack Problem"); // gives a name to the problem
            model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE); // says that lp is a maximization problem
            for (int i=0 ; i<quantItens ; i++) {
                x[i] = model.addVar(0.0, 1.0, v[i], GRB_BINARY, "");
                expr += s[i]*x[i];
            }
            for (int i=0 ; i<quantItens ; i++) {
                for (int j=0 ; j<quantItens ; j++) {
                    y[i].push_back(model.addVar(0.0, 1.0, relation[i][j], GRB_BINARY, ""));
                    model.addConstr(x[i]+x[j] <= y[i][j]+1);
                    model.addConstr(x[i]+x[j] >= 2*y[i][j]);
                }
            }
            model.update(); // run update to use model inserted variables
            model.addConstr(expr <= capacity);
            model.update(); // Process any pending model modifications.

            // bound the execution time
            model.getEnv().set(GRB_DoubleParam_TimeLimit, maxTime);
            // bound the solution value
            model.getEnv().set(GRB_DoubleParam_Cutoff, valor_h);
            model.update(); // Process any pending model modifications.
            model.optimize();
        } catch(GRBException e) {
            // if it isn't a timelimit exception
            if (e.getErrorCode() != 10005) {
                cout << "Error code = " << e.getErrorCode() << endl;
                cout << e.getMessage() << endl;
                exit(1);
            }
        } catch (...) {
            printf("Exception...\n");
            exit(1);
        }

        total_value = 0.0;
        for (int i=0 ; i<quantItens ; i++) {
            if (x[i].get(GRB_DoubleAttr_X) > 0.999) {
                total_value += v[i];
                itensMochila[i] = 1;
            }
        }
        for (int i=0 ; i<quantItens ; i++)
        for (int j=i ; j<quantItens ; j++)
        if (x[i].get(GRB_DoubleAttr_X) > 0.999 && x[j].get(GRB_DoubleAttr_X) > 0.999)
        total_value += relation[i][j];
        return total_value;
    } catch(GRBException e) {
        return 0;
    }
}
