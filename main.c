/* 
 * File:   main.c
 * Author: Sabra
 *
 * Created on 22. leden 2013, 23:56
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_STRING 32

FILE *f;

struct data {
    char time[8];
    float bg;
    float ig;
    float mod;
};

char buf[BUFFER_SIZE];

void parseCSV() {
    float ig, bg, mod;
    int igPos, bgPos, modPos;
    int count = 0;
    int length = 0;
    char *pos = NULL;
    char *first = NULL;
    int previous = 0;
    int c;

    f = fopen("G:\\E\\School\\FAV\\PPR\\semdata\\data\\A0904.csv", "r");
    printf("Opened\n");

    char line[1024];
    char tmp[500];
    while (fgets(line, BUFFER_SIZE, f) != NULL) {
        //char* tmp = strdup(line);
        int i;

        //        int header_position[H_MAX] = {-1,-1};// header[0] BG, header[1] IG
        int h_count = 0; //počet nalezených sloupců
        char *delimiter = ";\r\n";
        char *first_char = NULL;
        char *last_char = NULL;
        char string[MAX_STRING];
        char string_bg[MAX_STRING];
        char string_ig[MAX_STRING];
        int string_length = 0;
        int isEmpty = 0; //jestli chybí hodnota IG nebo BG v csv
        int column = 0; //číslo sloupce
        int row = 0; //číslo řádku

        /*for (first_char = line; (last_char = strpbrk(first_char, delimiter)) != NULL; 
                first_char = ++last_char, column++) {
            string_length = last_char - first_char;
            strncpy(string, first_char, string_length);
            string[string_length] = '\0';
            printf(string);
            printf("\n");
        }*/

        if (count == 0) {
            first = line;
            pos = line;
            for (i = 0; i < strlen(line); i++) {
                if (line[i] != ';' && line[i] != '\n') {
                    pos++;
                } else {
                    length = pos - first;
                    strncpy(tmp, first, length);
                    if (tmp[length-1] == '\r') {
                        tmp[length-1] = '\0';
                    } else {
                        tmp[length] = '\0';
                    }
                    first = ++pos;
                    printf(tmp);
                }
            }
            count++;
        } else {
            for (i = 0; i < strlen(line); i++) {
                if (line[i] == ',') {
                    line[i] = '.';
                }

                if (line[i] != ';') {

                }
            }
        }



        /* FILE *fr;
        char *delimiter =   ";\r\n";
        char line[MAX_LINE];
        int header_position[H_MAX] = {-1,-1};// header[0] BG, header[1] IG
        int h_count =       0;               //počet nalezených sloupců

        char *first_char = NULL;
        char *last_char = NULL;
        char string[MAX_STRING];
        char string_bg[MAX_STRING];
        char string_ig[MAX_STRING];
        int string_length = 0;
        int isEmpty =       0;            //jestli chybí hodnota IG nebo BG v csv
        int column =        0;            //číslo sloupce
        int row =           0;            //číslo řádku

        if ((fr = fopen(PATH, "r")) == NULL) {
            sprintf(errorMessage, "Soubor %s nelze otevrit ", PATH);           
            perror(errorMessage);
            MPI_Finalize();
            exit(1);
        }

        while (fgets(line, MAX_LINE, fr) != NULL) {
            if (h_count != H_MAX) {
                //rozpoznání pozic sloupců z hlavičky csv
                for (first_char = line; (last_char = strpbrk(first_char, delimiter)) != NULL; first_char = ++last_char, column++) {
                    string_length = last_char - first_char;
                    strncpy(string, first_char, string_length);
                    string[string_length] = 0;               
                    //printf("%s\n", string);   //hodnoty zahlavi

                    if (strcmp(string, H_BG) == 0) {
                        header_position[0] = column;         //pozice 0 je BG
                        h_count++;
                        continue;
                    } else if (strcmp(string, H_IG) == 0) {
                        h_count++;
                        header_position[1] = column;
                    }
                }
            }*/
    }
    fclose(f);
}

/*
 * 
 */
int main(int argc, char** argv) {
    parseCSV();
    return (EXIT_SUCCESS);
}

