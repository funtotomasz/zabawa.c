#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struktura przechowująca dane naszego rycerza
typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int gold;
} Player;

// Struktura dla przeciwników
typedef struct {
    char* name;
    int hp;
    int max_hp;
    int attack;
    int defense;
} Monster;

// Prototypy funkcji
void print_prompt(Player player);
void print_combat_status(Player player, Monster monster);
int start_combat(Player *player, Monster *monster); // Zwraca 1 jeśli gracz wygra, 0 jeśli przegra

int main() {
    // Inicjalizacja gracza z początkowymi statystykami
    Player knight = {
        .name = "Rycerz",
        .hp = 20,
        .max_hp = 20,
        .attack = 5,
        .defense = 2,
        .gold = 0
    };

    // Stworzenie naszego pierwszego przeciwnika
    Monster skeleton = {
        .name = "Szkielet",
        .hp = 12,
        .max_hp = 12,
        .attack = 4,
        .defense = 1
    };
    int skeleton_alive = 1;

    char command[50];
    int game_is_running = 1;

    printf("Witaj w grze, potężny rycerzu!\n");
    printf("Stoisz u wrót pradawnego lochu, gdy nagle z mroku wyłania się %s!\n", skeleton.name);
    printf("Co robisz?\n");
    printf("(Wpisz 'pomoc' aby zobaczyć listę komend)\n\n");


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
                printf("pomoc  - wyświetla tę listę\n");
                printf("status - pokazuje statystyki rycerza\n");
                printf("walcz  - zaatakuj przeciwnika\n");
                printf("wyjdz  - kończy grę\n");
                printf("------------------------\n\n");
            } else if (strcmp(command, "status") == 0) {
                printf("\n--- Twoje statystyki ---\n");
                printf("HP:      %d/%d\n", knight.hp, knight.max_hp);
                printf("Atak:    %d\n", knight.attack);
                printf("Obrona:  %d\n", knight.defense);
                printf("Złoto:   %d\n", knight.gold);
                printf("------------------------\n\n");
            } else if (strcmp(command, "walcz") == 0) {
                if (skeleton_alive) {
                    // Przywracamy potworowi pełne HP przed walką
                    skeleton.hp = skeleton.max_hp;
                    if (start_combat(&knight, &skeleton)) {
                        skeleton_alive = 0;
                        printf("W nagrodę znajdujesz 10 sztuk złota!\n");
                        knight.gold += 10;
                        printf("Po pokonaniu przeciwnika możesz kontynuować eksplorację lub wyjść.\n");
                    } else {
                        printf("\nZostałeś pokonany... KONIEC GRY\n");
                        game_is_running = 0;
                    }
                } else {
                    printf("W tej komnacie nie ma już z kim walczyć.\n");
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
    printf("\nHP: %d/%d > ", player.hp, player.max_hp);
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