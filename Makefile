CC=g++
#utilize aqui o path completo para a pasta do gurobi em seu pc
CFLAGS=-I /opt/gurobi752/linux64/include/ -L /opt/gurobi752/linux64/lib/ -lgurobi_c++ -lgurobi75 -lm -lstdc++
DEPS = pmr.h
OBJ = pmr.o pmr_e_h.o 

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pmr: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

#OBJECTS=knapsack.o knapsack_bt_bnb.o
#CPP=g++

#all: knapsack

#knapsack: $(OBJECTS) knapsack.h
#	$(CPP) $(CFLAGS) $(OBJECTS) -o knapsack 
#
#knapsack.o: knapsack.cpp knapsack.h
#	$(CPP) $(CFLAGS) -c knapsack.cpp
#	
#knapsack_bt_bnb.o: knapsack_bt_bnb.cpp knapsack.h
#	$(CPP) $(CFLAGS) -c knapsack_bt_bnb.cpp
#
clean:
	rm -f $(OBJ) *~ pmr
