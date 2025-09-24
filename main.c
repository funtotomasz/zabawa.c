#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration dla struktury Location
struct Location;

// Struktura przechowująca dane naszego rycerza
typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int gold;
    struct Location* current_location; // Aktualna pozycja gracza
} Player;

// Struktura dla przeciwników
typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
} Monster;

// Struktura dla lokacji w grze
typedef struct Location {
    char* name;
    char* description;
    struct Location* north;
    struct Location* south;
    struct Location* east;
    struct Location* west;
    Monster* monster; // Wskaźnik na potwora w lokacji (lub NULL)
} Location;

// Prototypy funkcji
void print_prompt(Player player);
void print_combat_status(Player player, Monster monster);
int start_combat(Player *player, Monster *monster); // Zwraca 1 jeśli gracz wygra, 0 jeśli przegra

int main() {
    // --- Tworzenie świata gry ---
    Location entrance = { "Wejście do lochu", "Stoisz przed masywnymi, kamiennymi wrotami. Chłodne powietrze owiewa Twoją twarz.", NULL, NULL, NULL, NULL, NULL };
    Location hallway = { "Mroczny korytarz", "Wąski, kamienny korytarz ginie w ciemnościach. Na wschód widać uchylone drzwi.", NULL, NULL, NULL, NULL, NULL };
    Location armory = { "Zbrojownia", "Pomieszczenie wypełnione jest stojakami na broń. Większość jest pusta, ale w kącie coś się porusza.", NULL, NULL, NULL, NULL, NULL };

    // Łączenie lokacji
    entrance.north = &hallway;
    hallway.south = &entrance;
    hallway.east = &armory;
    armory.west = &hallway;

    // Inicjalizacja gracza z początkowymi statystykami i pozycją
    Player knight = {
        .name = "Rycerz",
        .hp = 20,
        .max_hp = 20,
        .attack = 5,
        .defense = 2,
        .gold = 0,
        .current_location = &entrance
    };


    // Stworzenie naszego pierwszego przeciwnika
    Monster skeleton = {
        .name = "Szkielet",
        .hp = 12,
        .max_hp = 12,
        .attack = 4,
        .defense = 1
    };
    // Umieszczenie potwora w lokacji
    armory.monster = &skeleton;

    char command[50];
    int game_is_running = 1;

    printf("Witaj w grze, potężny rycerzu!\n\n");
    printf("%s\n\n", knight.current_location->description);
    printf("Co robisz? (Wpisz 'pomoc' aby zobaczyć listę komend)\n");


    // Główna pętla gry
    while (game_is_running && knight.hp > 0) {
        print_prompt(knight);
        // Wczytaj komendę od gracza
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Usuń znak nowej linii z końca komendy
            command[strcspn(command, "\n")] = 0;

            // Prosta obsługa komend
            if (strcmp(command, "wyjdz") == 0) {
                printf("Opuszczasz grę. Do zobaczenia!\n");
                game_is_running = 0; // Zakończ pętlę
            } else if (strcmp(command, "pomoc") == 0) {
                printf("\n--- Dostępne komendy ---\n");
                printf("pomoc          - wyświetla tę listę\n");
                printf("status         - pokazuje statystyki rycerza\n");
                printf("rozgladnij sie - opisuje obecną lokację\n");
                printf("idz [kierunek] - idź na polnoc, poludnie, wschod lub zachod\n");
                printf("walcz          - zaatakuj przeciwnika w lokacji\n");
                printf("wyjdz          - kończy grę\n");
                printf("------------------------\n\n");
            } else if (strcmp(command, "rozgladnij sie") == 0) {
                printf("\n%s\n", knight.current_location->description);
            } else if (strncmp(command, "idz ", 4) == 0) {
                char* direction = command + 4;
                struct Location* next_location = NULL;

                if (strcmp(direction, "polnoc") == 0) next_location = knight.current_location->north;
                else if (strcmp(direction, "poludnie") == 0) next_location = knight.current_location->south;
                else if (strcmp(direction, "wschod") == 0) next_location = knight.current_location->east;
                else if (strcmp(direction, "zachod") == 0) next_location = knight.current_location->west;

                if (next_location != NULL) {
                    knight.current_location = next_location;
                    printf("\nPrzechodzisz do lokacji: %s\n", knight.current_location->name);
                    printf("%s\n", knight.current_location->description);
                } else {
                    printf("Nie możesz iść w tym kierunku.\n");
                }
            }
            else if (strcmp(command, "status") == 0) {
                printf("\n--- Twoje statystyki ---\n");
                printf("HP:      %d/%d\n", knight.hp, knight.max_hp);
                printf("Atak:    %d\n", knight.attack);
                printf("Obrona:  %d\n", knight.defense);
                printf("Złoto:   %d\n", knight.gold);
                printf("------------------------\n\n");
            } else if (strcmp(command, "walcz") == 0) {
                Monster* monster = knight.current_location->monster;
                if (monster != NULL) {
                    // Przywracamy potworowi pełne HP przed walką
                    monster->hp = monster->max_hp;
                    if (start_combat(&knight, monster)) {
                        printf("W nagrodę znajdujesz 10 sztuk złota!\n");
                        knight.gold += 10;
                        // Usuwamy potwora z lokacji po pokonaniu
                        knight.current_location->monster = NULL;
                    } else {
                        printf("\nZostałeś pokonany... KONIEC GRY\n");
                        game_is_running = 0;
                    }
                } else {
                    printf("W tej komnacie nie ma z kim walczyć.\n");
                }
            }
            else {
                printf("Nieznana komenda: '%s'\n", command);
            }
        }
    }

    return 0;
}

// Funkcja do wyświetlania znaku zachęty
void print_prompt(Player player) {
    printf("\n[%s] HP: %d/%d > ", player.current_location->name, player.hp, player.max_hp);
}

void print_combat_status(Player player, Monster monster) {
    printf("%s: %d/%d HP | %s: %d/%d HP\n",
           player.name, player.hp, player.max_hp,
           monster.name, monster.hp, monster.max_hp);
}

// Funkcja obsługująca walkę
int start_combat(Player *player, Monster *monster) {
    printf("\n--- Rozpoczyna się walka! ---\n");

    while (1) { // Pętla będzie przerywana przez break
        print_combat_status(*player, *monster);

        // Tura gracza
        int player_damage = player->attack - monster->defense;
        if (player_damage < 1) player_damage = 1; // Gracz zawsze zadaje min 1 obrażenie

        monster->hp -= player_damage;
        printf("Zadajesz %sowi %d obrażeń!\n", monster->name, player_damage);

        if (monster->hp <= 0) {
            printf("Pokonałeś %s!\n", monster->name);
            printf("--- Walka zakończona ---\n\n");
            return 1; // Gracz wygrał
        }

        // Tura potwora
        int monster_damage = monster->attack - player->defense;
        if (monster_damage < 0) monster_damage = 0; // Potwór może nie zadać obrażeń

        player->hp -= monster_damage;
        printf("%s kontratakuje, zadając Ci %d obrażeń!\n", monster->name, monster_damage);

        if (player->hp <= 0) {
            player->hp = 0; // Zapobiegamy ujemnym HP
            printf("Zostałeś pokonany przez %s...\n", monster->name);
            printf("--- Walka zakończona ---\n\n");
            return 0; // Gracz przegrał
        }
        printf("--------------------------------\n");
    }
}