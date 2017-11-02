all: raduls
	
BIN_DIR = bin
MAIN_DIR = Example


CC 	= g++
CFLAGS	= -IRaduls -Wall -fopenmp -O3 -mavx2 -fno-ipa-ra -fno-tree-vrp -fno-tree-pre -m64 -std=c++14 -pthread
CLINK	= -lm -fopenmp -O3 -mavx2 -fno-ipa-ra -fno-tree-vrp -fno-tree-pre  -std=c++14 -lpthread


OBJS = \
$(MAIN_DIR)/main.o \


$(OBJS): %.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Raduls/sorting_network.o : Raduls/sorting_network.cpp 
	$(CC) -O1 -m64 -std=c++14 -mavx2 -pthread -c $< -o $@

raduls: $(OBJS) Raduls/sorting_network.o
	-mkdir -p $(BIN_DIR)
	$(CC) $(CLINK) -o $(BIN_DIR)/$@ $^

clean:
	-rm Raduls/sorting_network.o
	-rm $(MAIN_DIR)/*.o
	-rm -rf bin

all: raduls
