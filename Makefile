# Prosty Makefile do kompilacji naszej gry

# Kompilator C
CC = gcc
# Flagi kompilatora (włączamy wszystkie ostrzeżenia i obsługę Unicode)
CFLAGS = -Wall -Wextra -D_XOPEN_SOURCE_EXTENDED
# Flagi linkera (dołączamy szerokoznakową bibliotekę ncursesw)
LDFLAGS = -lncursesw

# Nazwa pliku wykonywalnego
TARGET = gra

# Domyślny cel: kompilacja gry
all: $(TARGET)

# Jak zbudować plik wykonywalny
$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) $(LDFLAGS)

# Cel do czyszczenia skompilowanych plików
clean:
	rm -f $(TARGET)

.PHONY: all clean