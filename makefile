CC=gcc
CFLAGS=-std=c11 -Wall -g -O -pthread 
LDLIBS=-lm -lrt -pthread

SRC = src
OBJ = obj

TARGET = pagerank

# Regola per la creazione dell'eseguibile pagerank utilizzando i file oggetto
pagerank: $(OBJ)/main.o $(OBJ)/xerrori.o $(OBJ)/grafo.o $(OBJ)/buffer.o $(OBJ)/pagerank_utils.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Regola per la creazione dei file oggetto
$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

# Regola per pulire i file generati

clean: 
	$(RM) -r $(OBJ)
	$(RM) $(TARGET)
