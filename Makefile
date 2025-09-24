# Prosty Makefile do kompilacji naszej gry

# Kompilator C
CC = gcc
# Flagi kompilatora (włączamy wszystkie ostrzeżenia)
CFLAGS = -Wall -Wextra

# Nazwa pliku wykonywalnego
TARGET = gra

# Domyślny cel: kompilacja gry
all: $(TARGET)

# Jak zbudować plik wykonywalny
$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

# Cel do czyszczenia skompilowanych plików
clean:
	rm -f $(TARGET)

.PHONY: all clean