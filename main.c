#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

// --- Struktury Danych Gry ---
struct Location; // Forward declaration

typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int gold;
    struct Location* current_location;
} Player;

typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
} Monster;

typedef struct Location {
    char* name;
    char* description;
    struct Location* north;
    struct Location* south;
    struct Location* east;
    struct Location* west;
    Monster* monster;
} Location;


// --- Zmienne Globalne ---
WINDOW *main_win, *status_win, *input_win;
const char* commands[] = {"/pomoc", "/status", "/rozgladnij", "/idz", "/walcz", "/wyjdz"};
const int num_commands = sizeof(commands) / sizeof(commands[0]);


// --- Funkcje Interfejsu ---
void initialize_ncurses() {
    setlocale(LC_ALL, "");
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
}

void create_windows() {
    int h, w;
    getmaxyx(stdscr, h, w);
    main_win = newwin(h - 6, w, 0, 0);
    status_win = newwin(3, w, h - 6, 0);
    input_win = newwin(3, w, h - 3, 0);
    box(main_win, 0, 0); mvwprintw(main_win, 0, 2, " DZIENNIK ");
    box(status_win, 0, 0); mvwprintw(status_win, 0, 2, " STATUS ");
    box(input_win, 0, 0); mvwprintw(input_win, 0, 2, " KOMENDA ");
    refresh();
    wrefresh(main_win); wrefresh(status_win); wrefresh(input_win);
}

void log_message(const char* message) {
    wclear(main_win);
    box(main_win, 0, 0);
    mvwprintw(main_win, 0, 2, " DZIENNIK ");
    mvwprintw(main_win, 1, 2, "%s", message);
    wrefresh(main_win);
}

void update_status(Player player) {
    wclear(status_win);
    box(status_win, 0, 0);
    mvwprintw(status_win, 0, 2, " STATUS ");
    mvwprintw(status_win, 1, 2, "Lokacja: %-20s | HP: %d/%d | Złoto: %d",
              player.current_location->name, player.hp, player.max_hp, player.gold);
    wrefresh(status_win);
}

void get_command(char *buffer, int size) {
    int i = 0;
    buffer[0] = '\0';
    wmove(input_win, 1, 2);
    wclrtoeol(input_win);
    box(input_win, 0, 0); mvwprintw(input_win, 0, 2, " KOMENDA ");
    wrefresh(input_win);
    wmove(input_win, 1, 2);

    int ch;
    while ((ch = wgetch(input_win)) != '\n') {
        if (ch == '\t') {
            char *match = NULL; int match_count = 0;
            for (int j = 0; j < num_commands; j++) {
                if (strncmp(buffer, commands[j], i) == 0) {
                    match = (char*)commands[j];
                    match_count++;
                }
            }
            if (match_count == 1) {
                strcpy(buffer, match);
                i = strlen(buffer);
                mvwprintw(input_win, 1, 2, "%s", buffer);
                wmove(input_win, 1, 2 + i);
            }
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (i > 0) { i--; buffer[i] = '\0'; mvwprintw(input_win, 1, 2 + i, " "); wmove(input_win, 1, 2 + i); }
        } else if (i < size - 1 && ch >= 32 && ch <= 126) {
            buffer[i++] = ch; buffer[i] = '\0';
            mvwprintw(input_win, 1, 2, "%s", buffer);
        }
        wrefresh(input_win);
    }
}

void cleanup_ncurses() {
    delwin(main_win); delwin(status_win); delwin(input_win);
    endwin();
}

// --- Logika Gry ---
int start_combat(Player *player, Monster *monster) {
    log_message("Rozpoczyna się walka!");
    update_status(*player);
    getch(); // pauza

    while (player->hp > 0 && monster->hp > 0) {
        int player_dmg = player->attack - monster->defense;
        if (player_dmg < 1) player_dmg = 1;
        monster->hp -= player_dmg;
        log_message("Zadajesz obrażenia! Potwór traci %d HP.");
        update_status(*player);
        getch();

        if (monster->hp <= 0) break;

        int monster_dmg = monster->attack - player->defense;
        if (monster_dmg < 0) monster_dmg = 0;
        player->hp -= monster_dmg;
        log_message("Otrzymujesz obrażenia! Tracisz %d HP.");
        update_status(*player);
        getch();
    }

    if (player->hp > 0) {
        log_message("Zwycięstwo!");
        getch();
        return 1;
    } else {
        log_message("Porażka... Koniec gry.");
        getch();
        return 0;
    }
}


int main() {
    initialize_ncurses();
    create_windows();

    // --- Inicjalizacja Świata Gry ---
    Location entrance = {"Wejście", "Stoisz przed wrotami lochu.", NULL, NULL, NULL, NULL, NULL};
    Location hallway = {"Korytarz", "Ciemny korytarz. Coś słychać na wschodzie.", NULL, NULL, NULL, NULL, NULL};
    Location armory = {"Zbrojownia", "Stojaki na broń i... szkielet!", NULL, NULL, NULL, NULL, NULL};
    entrance.north = &hallway;
    hallway.south = &entrance;
    hallway.east = &armory;
    armory.west = &hallway;

    Monster skeleton = {"Szkielet", 12, 12, 4, 1};
    armory.monster = &skeleton;

    Player knight = {"Rycerz", 20, 20, 5, 2, 0, &entrance};

    // --- Główna Pętla Gry ---
    char command[100];
    log_message(knight.current_location->description);
    update_status(knight);

    while (1) {
        get_command(command, sizeof(command));

        if (strcmp(command, "/wyjdz") == 0) break;

        if (strcmp(command, "/pomoc") == 0) {
            log_message("Komendy: /pomoc, /status, /rozgladnij, /idz [kierunek], /walcz, /wyjdz");
        } else if (strcmp(command, "/rozgladnij") == 0) {
            log_message(knight.current_location->description);
        } else if (strncmp(command, "/idz ", 5) == 0) {
            char* dir = command + 5;
            struct Location* next = NULL;
            if (strcmp(dir, "polnoc") == 0) next = knight.current_location->north;
            if (strcmp(dir, "poludnie") == 0) next = knight.current_location->south;
            if (strcmp(dir, "wschod") == 0) next = knight.current_location->east;
            if (strcmp(dir, "zachod") == 0) next = knight.current_location->west;

            if (next) {
                knight.current_location = next;
                log_message(knight.current_location->description);
            } else {
                log_message("Nie możesz tam iść.");
            }
        } else if (strcmp(command, "/walcz") == 0) {
            if (knight.current_location->monster) {
                if(start_combat(&knight, knight.current_location->monster)) {
                    knight.gold += 10;
                    knight.current_location->monster = NULL; // Potwór pokonany
                    log_message("Zdobywasz 10 złota. W komnacie jest już bezpiecznie.");
                } else {
                    break; // Przegrana kończy grę
                }
            } else {
                log_message("Nie ma tu z kim walczyć.");
            }
        } else {
            log_message("Nieznana komenda. Wpisz /pomoc.");
        }
        update_status(knight);
    }

    cleanup_ncurses();
    return 0;
}