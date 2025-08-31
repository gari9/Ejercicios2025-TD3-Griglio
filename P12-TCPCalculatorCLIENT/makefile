# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Iinc -pthread

# Directorios
SRC_DIR = src
INC_DIR = inc
BIN_DIR = bin

# Listado de archivos
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

# Regla para generar el ejecutable
EXECUTABLE = application

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(EXECUTABLE)