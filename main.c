#include <ncurses.h>
#include <locale.h>
#include <wchar.h>
#include <stdlib.h>
#include <wctype.h>

// --- Struktury Danych Gry (z wchar_t) ---
struct Location;

typedef struct {
    wchar_t* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int gold;
    struct Location* current_location;
} Player;

typedef struct {
    wchar_t* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
} Monster;

typedef struct Location {
    wchar_t* name;
    wchar_t* description;
    struct Location* north;
    struct Location* south;
    struct Location* east;
    struct Location* west;
    Monster* monster;
} Location;


// --- Zmienne Globalne ---
WINDOW *main_win, *status_win, *input_win;
const wchar_t* commands[] = {L"/pomoc", L"/status", L"/rozglądnij", L"/idz", L"/walcz", L"/wyjdz"};
const int num_commands = sizeof(commands) / sizeof(commands[0]);
const wchar_t* directions[] = {L"północ", L"południe", L"wschód", L"zachód"};
const int num_directions = sizeof(directions) / sizeof(directions[0]);


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
    box(main_win, 0, 0); mvwaddwstr(main_win, 0, 2, L" DZIENNIK ");
    box(status_win, 0, 0); mvwaddwstr(status_win, 0, 2, L" STATUS ");
    box(input_win, 0, 0); mvwaddwstr(input_win, 0, 2, L" KOMENDA ");
    refresh();
    wrefresh(main_win); wrefresh(status_win); wrefresh(input_win);
}

void log_message(const wchar_t* message) {
    wclear(main_win);
    box(main_win, 0, 0);
    mvwaddwstr(main_win, 0, 2, L" DZIENNIK ");
    mvwaddwstr(main_win, 1, 2, message);
    wrefresh(main_win);
}

void update_status(Player player) {
    wclear(status_win);
    box(status_win, 0, 0);
    mvwaddwstr(status_win, 0, 2, L" STATUS ");
    mvwprintw(status_win, 1, 2, "Lokacja: %-20ls | HP: %d/%d | Złoto: %d",
              player.current_location->name, player.hp, player.max_hp, player.gold);
    wrefresh(status_win);
}

void get_command(wchar_t *buffer, int size) {
    int i = 0;
    wint_t ch;
    buffer[0] = L'\0';

    wmove(input_win, 1, 2);
    wclrtoeol(input_win);
    box(input_win, 0, 0); mvwaddwstr(input_win, 0, 2, L" KOMENDA ");
    wrefresh(input_win);
    wmove(input_win, 1, 2);

    while (wget_wch(input_win, &ch) != ERR && ch != L'\n') {
        if (ch == L'\t') {
            wchar_t *space = wcschr(buffer, L' ');
            if (space == NULL) { // Uzupełnianie komendy
                const wchar_t *match = NULL; int match_count = 0;
                for (int j = 0; j < num_commands; j++) {
                    if (wcsncmp(buffer, commands[j], i) == 0) {
                        match = commands[j];
                        match_count++;
                    }
                }
                if (match_count == 1) {
                    wcscpy(buffer, match);
                    i = wcslen(buffer);
                    mvwaddwstr(input_win, 1, 2, buffer);
                    wmove(input_win, 1, 2 + i);
                }
            } else { // Uzupełnianie argumentu (np. kierunku)
                if (wcsncmp(buffer, L"/idz ", 5) == 0) {
                    const wchar_t* arg_start = space + 1;
                    int arg_len = wcslen(arg_start);
                    const wchar_t *match = NULL; int match_count = 0;
                    for (int j = 0; j < num_directions; j++) {
                        if (wcsncmp(arg_start, directions[j], arg_len) == 0) {
                            match = directions[j];
                            match_count++;
                        }
                    }
                    if (match_count == 1) {
                        wcscpy((wchar_t*)arg_start, match);
                        i = wcslen(buffer);
                        mvwaddwstr(input_win, 1, 2, buffer);
                        wmove(input_win, 1, 2 + i);
                    }
                }
            }
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (i > 0) { i--; buffer[i] = L'\0'; wmove(input_win, 1, 2); wclrtoeol(input_win); box(input_win, 0, 0); mvwaddwstr(input_win, 0, 2, L" KOMENDA "); mvwaddwstr(input_win, 1, 2, buffer); }
        } else if (i < size - 1 && iswprint(ch)) {
            buffer[i++] = ch; buffer[i] = L'\0';
            waddwstr(input_win, (const wchar_t*)&(buffer[i-1]));
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
    wchar_t combat_log[100];
    wint_t pause_char;

    swprintf(combat_log, 100, L"Rozpoczyna się walka z %ls!", monster->name);
    log_message(combat_log);
    update_status(*player);
    wget_wch(main_win, &pause_char);

    while (player->hp > 0 && monster->hp > 0) {
        int player_dmg = player->attack - monster->defense;
        if (player_dmg < 1) player_dmg = 1;
        monster->hp -= player_dmg;
        swprintf(combat_log, 100, L"Zadajesz %d obrażeń!", player_dmg);
        log_message(combat_log);
        update_status(*player);
        wget_wch(main_win, &pause_char);

        if (monster->hp <= 0) break;

        int monster_dmg = monster->attack - player->defense;
        if (monster_dmg < 0) monster_dmg = 0;
        player->hp -= monster_dmg;
        swprintf(combat_log, 100, L"%ls kontratakuje, tracisz %d HP.", monster->name, monster_dmg);
        log_message(combat_log);
        update_status(*player);
        wget_wch(main_win, &pause_char);
    }

    if (player->hp > 0) {
        swprintf(combat_log, 100, L"Zwycięstwo! Pokonałeś %ls.", monster->name);
        log_message(combat_log);
        wget_wch(main_win, &pause_char);
        return 1;
    } else {
        log_message(L"Porażka... Koniec gry.");
        wget_wch(main_win, &pause_char);
        return 0;
    }
}

int main() {
    initialize_ncurses();
    create_windows();

    Location entrance = {L"Wejście", L"Stoisz przed wrotami lochu. Czujesz chłód.", NULL, NULL, NULL, NULL, NULL};
    Location hallway = {L"Korytarz", L"Ciemny korytarz. Coś słychać na wschodzie.", NULL, NULL, NULL, NULL, NULL};
    Location armory = {L"Zbrojownia", L"Stojaki na broń i... szkielet! Coś tu śmierdzi.", NULL, NULL, NULL, NULL, NULL};
    entrance.north = &hallway;
    hallway.south = &entrance;
    hallway.east = &armory;
    armory.west = &hallway;

    Monster skeleton = {L"Szkielet", 12, 12, 4, 1};
    armory.monster = &skeleton;

    Player knight = {L"Rycerz", 20, 20, 5, 2, 0, &entrance};

    wchar_t command[100];
    log_message(knight.current_location->description);
    update_status(knight);

    while (1) {
        get_command(command, sizeof(command) / sizeof(wchar_t));

        if (wcscmp(command, L"/wyjdz") == 0) break;

        if (wcscmp(command, L"/pomoc") == 0) {
            log_message(L"Komendy: /pomoc, /status, /rozglądnij, /idz [kierunek], /walcz, /wyjdz");
        } else if (wcscmp(command, L"/rozglądnij") == 0) {
            log_message(knight.current_location->description);
        } else if (wcsncmp(command, L"/idz ", 5) == 0) {
            wchar_t* dir = command + 5;
            struct Location* next = NULL;
            if (wcscmp(dir, L"północ") == 0) next = knight.current_location->north;
            else if (wcscmp(dir, L"południe") == 0) next = knight.current_location->south;
            else if (wcscmp(dir, L"wschód") == 0) next = knight.current_location->east;
            else if (wcscmp(dir, L"zachód") == 0) next = knight.current_location->west;

            if (next) {
                knight.current_location = next;
                log_message(knight.current_location->description);
            } else {
                log_message(L"Nie możesz tam iść.");
            }
        } else if (wcscmp(command, L"/walcz") == 0) {
            if (knight.current_location->monster) {
                if(start_combat(&knight, knight.current_location->monster)) {
                    knight.gold += 10;
                    knight.current_location->monster = NULL;
                    log_message(L"Zdobywasz 10 złota. W komnacie jest już bezpiecznie.");
                } else {
                    break;
                }
            } else {
                log_message(L"Nie ma tu z kim walczyć.");
            }
        } else {
            log_message(L"Nieznana komenda. Wpisz /pomoc.");
        }
        update_status(knight);
    }

    cleanup_ncurses();
    return 0;
}