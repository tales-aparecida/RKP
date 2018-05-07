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

int global_quantItens;
int* global_weigth;
int* global_value;
double** global_relation_array;
int* global_items_array;
int* global_items_array_partial;
int* global_items_array_partial_id;
int global_items_array_partial_count = 0;
int global_max_partial = 0;

void algE_rec(int i, int capacity, int partial_val) {
    int id;
    if (i == global_quantItens || capacity == 0 || got_interrupt)
        return;

    // Analise without me
    algE_rec(i+1, capacity, partial_val);

    // If I fit
    capacity -= global_weigth[i];
    if (capacity >= 0) {
        global_items_array_partial[i] = 1;
        global_items_array_partial_id[global_items_array_partial_count++] = i;

        // Adds the relation values between items in the partial_items array and `i`, and `i` value itself
        for (int j = 0; j < global_items_array_partial_count-1; j++) {
            id = global_items_array_partial_id[j];
            if (global_items_array_partial[id])
                partial_val += global_relation_array[i][id];
        }
        partial_val += global_value[i];

        // save if better
        if (partial_val > global_max_partial) {
            global_max_partial = partial_val;
            for (int j = 0; j < global_quantItens; j++) {
                global_items_array[j] = global_items_array_partial[j];
            }
        }

        // Analise with me
        algE_rec(i+1, capacity, partial_val);
        global_items_array_partial[i] = 0;
        global_items_array_partial_count--;
    }
}

// get the id for the biggest v[id] that fits
int get_best_id(int capacity) {
    double curr_v, max_v = 0;
    int max_v_i = -1;
    for (int i = 0; i < global_quantItens; i++) {
        // skip if already in solution
        if (global_items_array_partial[i])
            continue;

        curr_v = (double)global_value[i] / global_weigth[i];
        if (global_weigth[i] <= capacity && curr_v > max_v) {
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

        capacity -= global_weigth[best_id];
        global_max_partial += global_value[best_id];
        global_items_array_partial[best_id] = 1;

        // update v[j] for all j outside of the solution with relation[best_id][j] value
        for (j=0; j < global_quantItens; j++)
            if (!global_items_array_partial[j])
                global_value[j] += global_relation_array[best_id][j];
    }
}

int algE(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    got_interrupt = false;
    signal(SIGALRM, alarm_handler);
    alarm(maxTime);

    global_weigth = &s[0];
    global_value = &v[0];
    global_items_array = (int*)calloc(quantItens, sizeof(int));
    global_items_array_partial = (int*)calloc(quantItens, sizeof(int));
    global_items_array_partial_id = (int*)calloc(quantItens, sizeof(int));
    global_relation_array = (double**)malloc(quantItens * sizeof(double*));
    for (int i = 0; i < quantItens; i++) {
        global_relation_array[i] = &relation[i][0];
    }
    global_max_partial = 0;
    global_quantItens = quantItens;

    algE_rec(0, capacity, 0);
    // copy response
    for (int i = 0; i < quantItens; i++)
        itensMochila[i] = global_items_array[i];

    free(global_items_array);
    free(global_items_array_partial);
    free(global_items_array_partial_id);
    free(global_relation_array);
    return global_max_partial;
}

int algH(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    global_quantItens = quantItens;
    global_max_partial = 0;
    global_weigth = &s[0];
    global_value = &v[0];
    global_items_array_partial = (int*)calloc(quantItens, sizeof(int));
    global_relation_array = (double**)malloc(quantItens * sizeof(double*));
    for (int i = 0; i < quantItens; i++) {
        global_relation_array[i] = &relation[i][0];
    }

    got_interrupt = false;
    signal(SIGALRM, alarm_handler);
    alarm(maxTime);

    algH_array(capacity);

    for (int j = 0; j < global_quantItens; j++) {
        itensMochila[j] = global_items_array_partial[j];
    }
    free(global_items_array_partial);
    free(global_relation_array);

    return global_max_partial;
}

int algExato(int capacity, int quantItens, vector<int> s, vector<int> v, matriz &relation, vector<int>& itensMochila, int maxTime) {
    int verbose = true;
    bool with_heuristic = false;
    double grb_heuristics_time = 0.05;
    int presolve = 0;

    for (int i=0 ; i<quantItens ; i++)
        itensMochila[i] = 0;

    double valor_h;
    if (with_heuristic)
        valor_h = algH(capacity, quantItens, s, v, relation, itensMochila, maxTime);

    vector<GRBVar> x(quantItens);
    vector< vector<GRBVar> > y(quantItens);
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);
    try {

        // Quiet gurobi
        model.getEnv().set(GRB_IntParam_OutputFlag, verbose);

        GRBLinExpr expr;
        model.set(GRB_StringAttr_ModelName, "Relational Knapsack Problem"); // gives a name to the problem
        model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE); // says that lp is a maximization problem
        for (int i=0 ; i<quantItens ; i++) {
            x[i] = model.addVar(0.0, 1.0, v[i], GRB_BINARY, "");
            if (with_heuristic)
                x[i].set(GRB_DoubleAttr_Start, itensMochila[i]);
            expr += s[i]*x[i];
        }
        for (int i=0 ; i<quantItens ; i++) {
            for (int j=0 ; j<i ; j++) {
                y[i].push_back(model.addVar(0.0, 1.0, relation[i][j], GRB_BINARY, ""));
                if (with_heuristic)
                    y[i][j].set(GRB_DoubleAttr_Start, itensMochila[i] && itensMochila[j]);
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
        if (with_heuristic)
            model.getEnv().set(GRB_DoubleParam_Cutoff, valor_h);
        // bound heuristics time
        model.getEnv().set(GRB_DoubleParam_Heuristics, grb_heuristics_time);
        // set MIPFocus
        model.getEnv().set(GRB_IntParam_MIPFocus, 1);
        // set Presolve approach
        model.getEnv().set(GRB_IntParam_Presolve, presolve);
        model.update(); // Process any pending model modifications.
        model.optimize();

        double total_value = 0.0;
        for (int i=0 ; i<quantItens ; i++) {
            if (x[i].get(GRB_DoubleAttr_X) > 0.999) {
                total_value += v[i];
                itensMochila[i] = 1;
                for (int j=0 ; j<i ; j++) {
                    if (x[j].get(GRB_DoubleAttr_X) > 0.999) {
                        total_value += relation[i][j];
                    }
                }
            } else {
                itensMochila[i] = 0;
            }
        }

        return total_value;
    } catch(GRBException e) {
        // if it isn't a timelimit exception
        if (e.getErrorCode() != 10005) {
            cout << "Error code = " << e.getErrorCode() << endl;
            cout << e.getMessage() << endl;
            exit(1);
        } else {
            cout << "Time's up" << endl;
            double total_value = 0.0;
            double* xvals = model.get(GRB_DoubleAttr_X, model.getVars(), quantItens);
            for (int i=0 ; i<quantItens ; i++) {
                if (xvals[i] > 0.999) {
                    total_value += v[i];
                    itensMochila[i] = 1;
                    for (int j=0 ; j<i ; j++) {
                        if (xvals[j] > 0.999) {
                            total_value += relation[i][j];
                        }
                    }
                } else {
                    itensMochila[i] = 0;
                }
            }
            return total_value;
        }
    } catch (...) {
        printf("Exception...\n");
        exit(1);
    }
}
