/* 
 * File:   Graph.c
 * Author: Sabra
 *
 * Created on 15. únor 2013, 10:02
 */

#include <stdio.h>  //accesses the printf() function
#include <stdlib.h> //accesses the malloc funtion
#include <string.h>
#include "ApproxIface.h"
#include "Evolution.h"
#include "Graph.h"
#include "main.h"

#define PATH "A0904_GRAF.svg"

char IG_coordinates[MAX_COORDINATES];
char CalculatedBG_coordinates[MAX_COORDINATES];
char CalculatedIG_coordinates[MAX_COORDINATES];
char BG_coordinates[MAX_COORDINATES];
static char errorMessage[200];
static int imageSizeX = 1225;
static int imageSizeY = 770;
static int maxResolution_x = 1170; //maximalni souradnice x grafu
static int maxResolution_y = 730; //maximalni souradnice y grafu
static int top_padding = 30; //horní odsazení oblasti grafu
static int left_padding = 40; //levé odsazení oblasti grafu
static int min_Y_Value = 0;
static int max_Y_Value = 0;
static int divisor = 1; //určuje okolik se budou funkční hodnoty dělit aby se vešli do grafu
static double part_size_y = 0; // 70px velikost jednoho dílku na ose y (je jich 10)
static double part_size_label_y = 0; //textový přírustek na ose x
static double part_size_x = 0; // 130px velikost jednoho dílku na ose y (je jich 9)
static char time[15];
static int offsetY = 0; //zarovnani min. hodnoty y do pocatku. souradnic

static void initial();
static void getTime(int seconds);

//poits budu sdilet pres header

int writeGraph(LPAPPROXPOINT points, double p, double cg, double ecg, double c, double dt, double k) {
    initial();
    FILE *fw;
    if ((fw = fopen(PATH, "w")) == NULL) {
        sprintf(errorMessage, "Soubor pro zapis %s nelze otevrit ", PATH);
        perror(errorMessage);
        exit(1);
    }

    char x[10];
    char y[10];
    double resample = (double) count / (double) maxResolution_x; //načtený graf chci převzorkovat do oblasti velikosti maxResolution_x

    //výpočet souřadnic pro zadané ig
    int i;
    for (i = 0; i < maxResolution_x; i++) { //i je vlastne souradnice x + left_padding posun pac graf neni v pocatku
        int index = (int) (resample * i); //index funční hodnoty
        int ig = (int) ((((data[index].ig * part_size_y)) / divisor)); //osa y je rozdelena na 10 dilku, a i hodnoty jsou v techto deseti dilcich, pokud by se nevešli divisorem je zmenším aby se vešli

        sprintf(x, "%d,", (i + left_padding)); //zpusob jak prevest hodnotu na cele cislo
        sprintf(y, "%d ", (maxResolution_y - ig) + offsetY);
        strcat(IG_coordinates, x);
        strcat(IG_coordinates, y);
    }

    //výpočet souřadnic pro zadané bg
    for (i = 0; i < maxResolution_x; i++) {
        int index = (int) (resample * i);
        int bg = (int) ((((data[index].bg * part_size_y)) / divisor));

        sprintf(x, "%d,", (i + left_padding));
        sprintf(y, "%d ", (maxResolution_y - bg) + offsetY);
        strcat(BG_coordinates, x);
        strcat(BG_coordinates, y);
    }

    //!!!!!!ted je vse v poradku pac data beru po jedne vterine ale kdyby ne tak index musi ukazovat do pole points, ale asi nebudu resit, neni to v zadani
    // graf pro vypocitane bg ---provedl jsem zmenu 24.1.2013 predtim to bylo IG - leva strana rovnice se rovna ig v case t+dt+k*dt*ecg...takze vlastne pro cas t ig spocitam z prave strany 
    // double resample  =  (double) size_data / (double) maxResolution_x; //načtený graf chci převzorkovat do oblasti velikosti maxResolution_x
    for (i = 0; i < maxResolution_x; i++) { //i je vlastne souradnice x + nejaky posun pac graf neni v pocatku
        int index = (int) (resample * i); // index je vlastne cas t prepocitany podle resamble*i
        //funkcni hodnota bg
        double bg = points[index].bg;

        //vypocet leve strany rovnice
        double left_site = p * bg + cg * bg * ecg + c;
        int ig_calculated = (int) (((left_site * (double) part_size_y) / divisor)); //todo veskere ig prejmenovat na bg

        sprintf(x, "%d,", (i + left_padding));
        sprintf(y, "%d ", (maxResolution_y - ig_calculated) + offsetY);
        strcat(CalculatedBG_coordinates, x);
        strcat(CalculatedBG_coordinates, y);
    }
    double k_dt_ecg = k * dt * ecg;
    int k_dt_ecg_inSeconds = (int) (k_dt_ecg / OneSecond);
    printf("\n hodnota kecg v sekundach %d\n", k_dt_ecg_inSeconds);

    // graf pro vypocitane ig 
    // double resample  =  (double) size_data / (double) maxResolution_x; //načtený graf chci převzorkovat do oblasti velikosti maxResolution_x
    for (i = 0; i < maxResolution_x; i++) { //i je vlastne souradnice x + nejaky posun pac graf neni v pocatku 
        int t = (int) (resample * (i));

        //vypocet leve strany rovnice
        int right_site_parametr = (int) (t + (dt / OneSecond) + k_dt_ecg_inSeconds);
        if (right_site_parametr < 0 || right_site_parametr >= count) {
            continue;
        }
        int ig_calculated = (int) (((points[right_site_parametr].ig * (double) part_size_y) / divisor));

        sprintf(x, "%d,", (i + left_padding));
        sprintf(y, "%d ", (maxResolution_y - ig_calculated) + offsetY);
        strcat(CalculatedIG_coordinates, x);
        strcat(CalculatedIG_coordinates, y);
    }


    //osy
    fprintf(fw, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"1235\" height=\"770\">\n");
    fprintf(fw, "    <line id=\"osaX\" x1=\"35\" y1=\"730\" x2=\"1210\" y2=\"730\" style=\"stroke:black;stroke-width:1\"/>\n");
    fprintf(fw, "    <line id=\"osaY\" x1=\"40\" y1=\"735\" x2=\"40\" y2=\"30\" style=\"stroke:black;stroke-width:1\"/>\n");

    //graf zadaného ig
    fprintf(fw, "    <polyline fill=\"none\" stroke=\"blue\" stroke-width=\"1\"\n");
    fprintf(fw, "    points=\"");
    fprintf(fw, "%s\"", IG_coordinates);
    fprintf(fw, "/>\n");

    //graf vypočítaného ig
    fprintf(fw, "    <polyline fill=\"none\" stroke=\"blue\" stroke-dasharray=\"5,5\" stroke-width=\"1\"\n");
    fprintf(fw, "    points=\"");
    fprintf(fw, "%s\"", CalculatedIG_coordinates);
    fprintf(fw, "/>\n");


    //graf vypočítaného ig  //todo prejmenovat na bg
    fprintf(fw, "    <polyline fill=\"none\" stroke=\"green\" stroke-dasharray=\"5,5\" stroke-width=\"1\"\n");
    fprintf(fw, "    points=\"");
    fprintf(fw, "%s\"", CalculatedBG_coordinates);
    fprintf(fw, "/>\n");

    //graf zadaného bg
    fprintf(fw, "    <polyline fill=\"none\" stroke=\"red\" stroke-width=\"1\"\n");
    fprintf(fw, "    points=\"");
    fprintf(fw, "%s\"", BG_coordinates);
    fprintf(fw, "/>\n");


    //hodnoty na souřadnici y
    int max = 11;
    for (i = 0; i < max; i++) {
        fprintf(fw, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"16\">%d</text>\n", 11, (int) ((10 - i) * part_size_y + (top_padding)), ((min_Y_Value) + divisor * i));
        fprintf(fw, "    <line x1=\"35\" y1=\"%d\" x2=\"40\" y2=\"%d\" style=\"stroke:black;stroke-width:1\"/>\n", (int) (i * part_size_y + (top_padding)), (int) (i * part_size_y + (top_padding)));

    }

    //hodnoty na souřadnici x - čas
    fprintf(fw, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"16\">%s</text>\n", 24, 760, "0:00");
    int max_t = 9;
    for (i = 0; i < max_t; i++) {
        getTime((int) ((i + 1)*(count / (max_t))));
        fprintf(fw, "    <text x=\"%d\" y=\"%d\" font-family=\"Verdana\" font-size=\"16\">%s</text>\n", (int) (24 + ((i + 1) * part_size_x)), 760, time);
        fprintf(fw, "    <line x1=\"%d\" y1=\"730\" x2=\"%d\" y2=\"735\" style=\"stroke:black;stroke-width:1\"/>\n", (int) (left_padding + ((i + 1) * part_size_x)), (int) (left_padding + ((i + 1) * part_size_x)));
    }
    fprintf(fw, "</svg>");

    //todo odstranit
    printf("minvalue %d.\n", min_Y_Value);
    printf("maxvalue %d.\n", max_Y_Value);

    if (fclose(fw) == EOF) {
        printf("Soubor se nepodarilo uzavrit.\n");
        exit(1);
    }
}

static void getTime(int seconds) {
    int minutes = seconds % 3600;
    if (minutes > 60) {
        minutes = (int) (double) minutes / 60.0;
    }
    if (minutes < 10) {
        sprintf(time, "%d:0%d", (int) (seconds / 3600), minutes);

    } else {
        sprintf(time, "%d:%d", (int) (seconds / 3600), minutes);
    }
}

static void initial() {
    part_size_y = (maxResolution_y - top_padding) / 10.0;
    part_size_x = (maxResolution_x) / 9.0;
    //zjištení maximálniho rozsahu funkčních hodnot i a b a posunuto o 1 kvuli oriznuti pri prevodu na int
    min_Y_Value = (minI < minB) ? (minI - 1) : (minB - 1);
    max_Y_Value = (maxI > maxB) ? (maxI + 1) : (maxB + 1);

    int range = 10;
    while (max_Y_Value > (range + min_Y_Value)) {
        range += 10;
        divisor++;
    }
    //pozor offset se zadava v pixelech
    offsetY = (min_Y_Value * part_size_y) / divisor;
}
