#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    char name[300];
    int hitpoints;
    char weapon[300];
    int damage;
    int exp;
} Character;


//return the size of the database array
//by searching the marker of last character
int size_of_array(Character *characters) {
    int count = 0;
    while(characters[count].damage != -1) {
        count++;
    }
    return count;
}


//searches the database by character name
//return the index of the character or -1 if was not found
int find_character_index(Character* characters, char *name) {
    int size = size_of_array(characters);
    int index = -1;
    for(int i = 0; i < size; i++) {  
        if (strcmp(characters[i].name, name) == 0) {
            index = i;
        }
    }
    return index;
}


//adds character into the database
Character * add_character(Character *characters, char *name, int hitpoints, int exp, char *weapon, int damage, int print_suc) {
    if (find_character_index(characters, name) != -1) {
        printf("Fighter \"%s\" is already in the database.\n", name);
        return characters;
    }
    int size = size_of_array(characters);
    characters = realloc(characters, (size + 2) * sizeof(Character)); // +2 to have space for the marker of last character
    snprintf(characters[size].name, 300, name);
    snprintf(characters[size].weapon, 300, weapon);
    characters[size].hitpoints = hitpoints;
    characters[size].damage = damage;
    characters[size].exp = exp;
    characters[size + 1].damage = -1; // initialize the marker to new position
    if (print_suc) {
        printf("SUCCESS\n");
    }
    return characters;
}


// one character attacks another
void attack(Character *characters, char* attacker, char* attacked) {
    if (strcmp(attacker, attacked) == 0) {    // cannot attack self
        printf("Attacker \"%s\" cannot attack to itself.\n", attacker);
        return;
    }
    int atckr = find_character_index(characters, attacker);
    int atckd = find_character_index(characters, attacked);
    if (atckr == -1) {      
        printf("Attacker \"%s\" is not in the database.\n", attacker);
        return;
    }
    if (atckd == -1){
        printf("Attacker \"%s\" is not in the database.\n", attacked);
        return;
    }
    int array[3];  //index 0: damage done, index 1: experience gained, index 2: hitpoints left
    int damage = 0;
    while(damage == 0) {
        damage = rand() % characters[atckr].damage + 1;
    }
    if (damage >= characters[atckd].hitpoints) {
        array[0] = characters[atckd].hitpoints;
        array[1] = characters[atckd].hitpoints + 2;  //gain exp from damage and 2 extra from killing a character
        characters[atckd].hitpoints = 0;
        array[2] = 1;
    }
    else {
        array[0] = damage;
        array[1] = damage;
        characters[atckd].hitpoints -= damage;
        array[2] = characters[atckd].hitpoints;
    }
    characters[atckr].exp = array[1];
    printf("%s attacked %s with %s by %d damage.\n", attacker, attacked, characters[atckr].weapon, array[0]);
    printf("%s has %d hit points remaining.\n%s gained %d experience points.\n", attacked, array[2], attacker, array[1]);
    printf("SUCCESS\n");
}


//compares characters for qsort
//characters with 0 hp are sorted to the bottom of the list
//after that they are sorted by exp
int character_compare(const void* a, const void* b) {
    Character *c = (Character*) a;
    Character *d = (Character*) b;
    if (c->hitpoints == 0) {
        return 1;
    }
    if (d->hitpoints == 0) {
        return -1;
    }
    if (c->exp > d->exp) {
        return -1;
    }
    if (c->exp < d->exp) {
        return 1;
    }
    return 1;
}


//prints characters in order and sorts the character array while doing it
void print_characters(Character *characters) {
    int size = size_of_array(characters);
    qsort(characters, size, sizeof(Character), character_compare);
    for (int i = 0; i < size; i++) {
        Character ch = characters[i];
        printf("%s %d %d %s %d\n", ch.name, ch.hitpoints, ch.exp, ch.weapon, ch.damage);
    }
    printf("SUCCESS\n");
}


//saves the database into a file
//one character/line
void save_into_file(Character *characters, char *file_name) {
    FILE *file_ptr;
    file_ptr = fopen(file_name, "w");
    int size = size_of_array(characters);
    for (int i = 0; i < size; i++) {
        Character ch = characters[i];
        fprintf(file_ptr, "%s %d %d %s %d\n", ch.name, ch.hitpoints, ch.exp, ch.weapon, ch.damage);
    }
    fclose(file_ptr);
    printf("SUCCESS\n");
}


//replaces the current database-array of characters with a new one from a file
//the file's formatting is the one that the save_into_file uses
Character* load_from_file(Character *characters, char *file_name) {
    FILE *file_ptr = NULL;
    file_ptr = fopen(file_name, "r");
    if (file_ptr == NULL) {
        printf("Cannot open file %s for reading.\n", file_name);
        return characters;
    }
    free(characters);
    Character *new_characters;
    new_characters = malloc(sizeof(Character));
    new_characters[0].damage = -1; //initialize the marker for last character
    char name_1[300];
    int hitpoints;
    char weapon[300];
    int damage;
    int exp;
    char buffer[1000];
    
    while (fgets(buffer, 1000, file_ptr) != NULL) {
        if (sscanf(buffer, "%s %d %d %s %d", name_1, &hitpoints, &exp, weapon, &damage) != 5 ) {
            printf("Invalid line in file.\n");
        }
        else {
            new_characters = add_character(new_characters, name_1, hitpoints, exp, weapon, damage, 0);
        }
    }
    printf("SUCCESS\n");
    return new_characters;
}

//has the main loop for completing tasks and quits the program when the Q command is given
int main(void) {
    char user_input[1000];
    char command;
    int loop = 1;
    char name_1[300];
    int hitpoints;
    char name_2[300]; 
    int damage;

    Character *characters;
    characters = malloc(sizeof(Character));
    characters[0].damage = -1;  //set the damage of the last character in the list as -1 as a marker for the last character

    while (loop) {
        fgets(user_input, 1000, stdin);
        command = user_input[0];
        switch (command)
        {
        case 'A':  //adds a character to the database
            if (sscanf(user_input, "%c %s %d %s %d", &command, name_1, &hitpoints, name_2, &damage) != 5 ) {
                printf("A should be followed by exactly 4 arguments.\n");
            }
            else {
                if (hitpoints <= 0) {
                    printf("HP cannot be lower than 1\n");
                }
                else if (damage <= 0) {
                    printf("Max damage cannot be lower than 1\n");
                }
                else {
                characters = add_character(characters, name_1, hitpoints, 0, name_2, damage, 1);
                }
            }
            break;
        
        case 'H':   //one character from the database attacks another character from the database
            if (sscanf(user_input, "%c %s %s", &command, name_1, name_2) != 3 ) {
                printf("H should be followed by exactly 2 arguments.\n");
            }
            else {
                attack(characters, name_1, name_2);
            }
            break;

        case 'L':   //prints the characters from the database
            print_characters(characters);
            break;

        case 'W':   //saves the characters into a file
            if (sscanf(user_input, "%c %s", &command, name_1) != 2 ) {
                printf("W should be followed by exactly 1 argument.\n");
            }
            else {
                save_into_file(characters, name_1);
            }
            break;

        case 'O':   //replaces the current database with data from a file
            if (sscanf(user_input, "%c %s", &command, name_1) != 2 ) {
                printf("O should be followed by exactly 1 argument.\n");
            }
            else {
                characters = load_from_file(characters, name_1);
            }
            break;

        case 'Q': //quits the program
            loop = 0;
            free(characters);
            printf("SUCCESS\n");
            break;

        default:
            printf("Invalid command %s\n", user_input);
            break;
        }
    }
    return 0;
}