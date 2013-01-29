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
#define MAX_LINES 25000
#define BG "BG"
#define IG "IG"
#define TIME "Time"
#define MOD "Modifier"

FILE *f;

typedef struct {
    char time[9];
    float bg;
    float ig;
    float mod;
} DATA;

void parseCSV(DATA data[]) {
    float ig = -1, bg = -1, mod = -1;
    int timePos = 0, igPos = 0, bgPos = 0, modPos = 0;
    int count = 0;
    int column = 0;
    int length = 0;
    char *pos = NULL;
    char *first = NULL;
    char time[MAX_STRING];

    f = fopen("G:\\E\\School\\FAV\\PPR\\semdata\\data\\A0904.csv", "r");
    printf("Opened\n");

    char line[1024];
    char tmp[500];
    while (fgets(line, BUFFER_SIZE, f) != NULL) {
        int i;
        if (count == 0) {
            first = line;
            pos = line;
            column = 0;

            for (i = 0; i < strlen(line); i++) {

                if (line[i] != ';' && line[i] != '\n' && line[i] != ';\r\n') {
                    pos++;
                } else {
                    length = pos - first;
                    strncpy(tmp, first, length);

                    if (tmp[length - 1] == '\r') {
                        tmp[length - 1] = '\0';
                    } else {
                        tmp[length] = '\0';
                    }

                    if (strcmp(tmp, TIME) == 0) {
                        timePos = column;
                    } else if (strcmp(tmp, BG) == 0) {
                        bgPos = column;
                    } else if (strcmp(tmp, IG) == 0) {
                        igPos = column;
                    } else if (strcmp(tmp, MOD) == 0) {
                        modPos = column;
                    } else {
                    }

                    first = ++pos;
                    column++;
                    /*printf("%s\n", tmp);*/
                }
            }
            count++;
        } else {
            first = line;
            pos = line;
            column = 0;
            for (i = 0; i < strlen(line); i++) {

                if (line[i] == ',') {
                    line[i] = '.';
                }

                if (line[i] != ';' && line[i] != '\n' && line[i] != ';\r\n') {
                    pos++;
                } else {
                    length = pos - first;
                    strncpy(tmp, first, length);
                    if (tmp[length - 1] == '\r') {
                        tmp[length - 1] = '\0';
                    } else {
                        tmp[length] = '\0';
                    }
                    first = ++pos;

                    if (column == timePos) {
                        strncpy(time, tmp, strlen(tmp) + 1);
                    } else if (column == bgPos) {
                        bg = atoff(tmp);
                    } else if (column == igPos) {
                        ig = atoff(tmp);
                    } else if (column == modPos) {
                        mod = atoff(tmp);
                    } else {
                    }
                    column++;
                }
            }

            if (bg != 0 && ig != 0) {
                strncpy(data[count - 1].time, time, strlen(time));
                data[count - 1].bg = bg;
                data[count - 1].ig = ig;
                data[count - 1].mod = mod;
               /* printf("%s %f %f %f\n", data[count - 1].time, data[count - 1].bg,
                        data[count - 1].ig, data[count - 1].mod);*/
                count++;
            }
        }

    }
    printf("Nacteno %i radku\n", count);
    fclose(f);
    printf("File closed\n");
}

/*
 * 
 */
int main(int argc, char** argv) {
    DATA data[MAX_LINES];
    parseCSV(data);
    return (EXIT_SUCCESS);
}

