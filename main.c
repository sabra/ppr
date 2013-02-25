/* 
 * File:   main.c
 * Author: Sabra
 *
 * Created on 22. leden 2013, 23:56
 */

#include <stdio.h>
#include <stdlib.h>
#include "ApproxIface.h"
#include "main.h"

#define BUFFER_SIZE 1024
#define MAX_STRING 32
#define MAX_LINES 25000
#define BG "BG"
#define IG "IG"
#define TIME "Time"
#define MOD "Modifier"

FILE *f;
LPAPPROXPOINT data;
int count = 0;

int minB = 100000; //min funkční hodnota b(t)
int maxB = -100000; //maximální funkční hodnota b(t)
int minI = 100000; //min funkční hodnota i(t)
int maxI = -100000; //maximální funkční hodnota i(t), slouží k určení mezí grafu

void parseCSV() {

    char time[MAX_STRING];
    float ig = 0, bg = 0, mod = 0;
    int timePos = 0, igPos = 0, bgPos = 0, modPos = 0;
    int column = 0;
    int length = 0;
    char *pos = NULL;
    char *first = NULL;

    data = (LPAPPROXPOINT) malloc(sizeof (APPROXPOINT) * MAX_LINES);
    if (data == NULL) {
        printf("Memory could not be allocated!\n");
        exit(1);
    }

    if ((f = fopen("A0904.csv", "r")) == NULL) {
        printf("File could not be opened!\n");
        exit(1);
    }

    char line[1024];
    char tmp[500];
    while (fgets(line, BUFFER_SIZE, f) != NULL) {
        int i;
        if (count == 0) {
            first = line;
            pos = line;
            column = 0;

            for (i = 0; i < strlen(line); i++) {

                if (line[i] != ';' && line[i] != '\n') {
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

                if (line[i] != ';' && line[i] != '\n') {
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
                /*printf("%s %f %f %f\n", data[count - 1].time, data[count - 1].bg,
                         data[count - 1].ig, data[count - 1].mod);*/
                if (((int) ig) < minI) {
                    minI = (int) ig;
                }
                if (((int) ig) > maxI) {
                    maxI = (int) ig;
                }

                if (((int) bg) < minB) {
                    minB = (int) bg;
                }
                if (((int) bg) > maxB) {
                    maxB = (int) bg;
                }
                count++;
            }
        }

    }

    count--;
    printf("Nacteno %i radku\n", count);
    fclose(f);
}

HRESULT GetBuf(LPAPPROXPOINT points, floattype step, int len, int *filled) {
    // TODO: Upravit, aby sel pouzit i jinej krok
    if (len * step > count) {
        printf("You can't read more than you have! Decrease your step!\n");
        exit(1);
    } else {
        int i;
        for (i = 0; i < len; i++) {
            int stepTemp = (int) (step * i);
            points[i].bg = data[stepTemp].bg;
            points[i].ig = data[stepTemp].ig;
            *filled = i + 1;
        }
    }
    return 1;
}

/*
 * 
 */
int main(int argc, char** argv) {
    // TODO: Dodelat nacitani parametru z prikazovy radky
    parseCSV();
    evolution();
    return (EXIT_SUCCESS);
}

