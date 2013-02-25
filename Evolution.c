/* 
 * File:   Evolution.c
 * Author: Sabra
 *
 * Created on 3. únor 2013, 20:43
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#include <windef.h>
//#include <rpcndr.h>
//#include <winioctl.h>
#include "ApproxIface.h"
#include "Evolution.h"
#include "main.h"
#include "Graph.h"

int length;
int filled;
int *p_filled;
floattype step;
static LPAPPROXPOINT points;

static double F; //mutační konstanta
static double CR; //práh křížení
static int NP; //počet jedinců v populaci
static int D = 6; //počet dimenzí (rozměr jedince - argumenty funkce)
static PINDIVIDUAL population;
static PINDIVIDUAL mutatedPopulation;

const floattype OneMinute = 1.0 / (24.0 * 60.0);
const floattype OneSecond = 1.0 / (24.0 * 60.0 * 60.0);
const int dtMax = 40 * 60;
static double fiveMinNeg;
static double fiveMinPos;
static double fortyMinPos;
static int maxDt = 2400;
int minIndex = -1;
double minValue = 1000;
int maxIndex = -1;
double maxValue = 0;


static void evolve();
static void generateIndividuals();
static void generateIndividual();
static void mutatePopulation();
static int penalization(INDIVIDUAL vector);
static double probability();

static void evolve() {
    generateIndividuals();
    mutatePopulation();
}

static void generateIndividuals() {
    srand(time(NULL));
    int i;
    for (i = 0; i < NP; i++) {
        generateIndividual(i);
    }
}

static void generateIndividual(int i) {
    int t = 0;
    double ecg = 0.0;
    double p = 0.0;
    double cg = 0.0;
    double c = 0.0;
    double dt = 0.0;
    double k = 0.0;
    double kDtEcg = 0.0;
    double cMax = 0.0;
    int indexI = 0;

    do {
        t = rand() % filled;
        if (points[t].bg < points[t].ig) {
            cMax = points[t].bg;
        } else {
            cMax = points[t].ig;
        }
        ecg = points[t].bg - points[t].ig;
        p = 0.94;
        cg = (-cMax)*((double) rand() / (double) RAND_MAX);
        c = (cMax)*((double) rand() / (double) RAND_MAX);
        dt = (double) ((rand() % maxDt) * OneSecond);
        k = (double) ((rand() % 5000) * OneSecond);
        kDtEcg = k * dt*ecg;

        while (kDtEcg < fiveMinNeg || kDtEcg > fiveMinPos) {
            dt = (double) ((rand() % maxDt) * OneSecond);
            k = (double) ((rand() % 5000) * OneSecond);
            kDtEcg = k * dt*ecg;
        }

        int time = (int) ((dt + kDtEcg) / OneSecond);
        indexI = t + time;
    } while (indexI < 0 || indexI > filled);

    double bt = points[t].bg;
    double leftSide = p * bt + cg * bt * ecg + c;
    double rightSide = points[indexI].ig;

    double vof = fabs(rightSide - leftSide); // hodnota ucelove funkce (value of objective function)

    population[i].t = t;
    population[i].c = c;
    population[i].cg = cg;
    population[i].dt = dt;
    population[i].ecg = ecg;
    population[i].k = k;
    population[i].p = p;
    population[i].vof = vof;

}

static void mutatePopulation() {
    int i;
    int a, b, c;
    for (i = 0; i < NP; i++) {
        do {
            a = rand() % NP;
        } while (a == i);
        do {
            b = rand() % NP;
        } while (b == i || b == a);
        do {
            c = rand() % NP;
        } while (c == i || c == b || c == a);

        INDIVIDUAL noiseVector;
        noiseVector.c = (population[a].c - population[b].c) * F + population[c].c;
        noiseVector.cg = (population[a].cg - population[b].cg) * F + population[c].cg;
        noiseVector.dt = (population[a].dt - population[b].dt) * F + population[c].dt;
        noiseVector.ecg = (population[a].ecg - population[b].ecg) * F + population[c].ecg;
        noiseVector.k = (population[a].k - population[b].k) * F + population[c].k;
        noiseVector.p = (population[a].p - population[b].p) * F + population[c].p;
        noiseVector.vof = (population[a].vof - population[b].vof) * F + population[c].vof;
        noiseVector.t = (population[a].t - population[b].t) * F + population[c].t;

        //int penale = penalization(noiseVector);
        //double prob = probability();
        if (probability() > CR) {
            noiseVector.c = population[i].c;
        }
        if (probability() > CR) {
            noiseVector.cg = population[i].cg;
        }
        if (probability() > CR) {
            noiseVector.dt = population[i].dt;
        }
        if (probability() > CR) {
            noiseVector.ecg = population[i].ecg;
        }
        if (probability() > CR) {
            noiseVector.k = population[i].k;
        }
        if (probability() > CR) {
            noiseVector.t = population[i].t;
        }
        if (probability() > CR) {
            noiseVector.vof = population[i].vof;
        }

        double kDtEcq = noiseVector.k * noiseVector.dt * noiseVector.ecg;
        int time = (int) ((noiseVector.dt + kDtEcq) / OneSecond);
        int indexI = population[i].t + time;

        if (indexI < 0 || indexI >= filled || noiseVector.t < 0 || noiseVector.t >= filled) {
            mutatedPopulation[i].t = population[i].t;
            mutatedPopulation[i].c = population[i].c;
            mutatedPopulation[i].cg = population[i].cg;
            mutatedPopulation[i].dt = population[i].dt;
            mutatedPopulation[i].ecg = population[i].ecg;
            mutatedPopulation[i].k = population[i].k;
            mutatedPopulation[i].p = population[i].p;
            mutatedPopulation[i].vof = population[i].vof;
            continue;
        }

        double bt = points[noiseVector.t].bg;
        double leftSide = noiseVector.p * bt + noiseVector.cg * bt * noiseVector.ecg + noiseVector.c;
        double rightSide = points[indexI].ig;

        double vof = fabs(rightSide - leftSide); // hodnota ucelove funkce (value of objective function)

        if (vof < minValue) {
            minValue = vof;
            minIndex = i;
        }
        if(vof > maxValue){
            maxValue = vof;
            maxIndex = i;
        }
        
        if (population[i].vof > vof) {
            //printf("%d huf %f\n", i,  huf );
            mutatedPopulation[i].t = noiseVector.t;
            mutatedPopulation[i].c = noiseVector.c;
            mutatedPopulation[i].cg = noiseVector.cg;
            mutatedPopulation[i].dt = noiseVector.dt;
            mutatedPopulation[i].ecg = noiseVector.ecg;
            mutatedPopulation[i].k = noiseVector.k;
            mutatedPopulation[i].p = noiseVector.p;
            mutatedPopulation[i].vof = vof;
        } else {
            mutatedPopulation[i].t = population[i].t;
            mutatedPopulation[i].c = population[i].c;
            mutatedPopulation[i].cg = population[i].cg;
            mutatedPopulation[i].dt = population[i].dt;
            mutatedPopulation[i].ecg = population[i].ecg;
            mutatedPopulation[i].k = population[i].k;
            mutatedPopulation[i].p = population[i].p;
            mutatedPopulation[i].vof = population[i].vof;
        }

    }
}

static double probability() {
    double prob = (double) (rand() % 101) / 100.0;
    return prob;
}

static int penalization(INDIVIDUAL vector) {
    int penalty = 0;
    double kDtEcg = vector.k * vector.dt * vector.ecg;

    if (vector.c <= 0) {
        penalty += 1000;
    }
    if (vector.cg >= 0) {
        penalty += 1000;
    }
    if (vector.dt < 0 || vector.dt > maxDt * OneSecond) {
        penalty += 1000;
    }
    if (vector.p < 0 || vector.p > 1) {
        penalty += 1000;
    }
    if (fiveMinNeg > kDtEcg || kDtEcg > fiveMinPos) {
        penalty += 1000;
    }

    return penalty;
}

void evolution() {
    length = count;
    filled;
    p_filled = &filled;
    step = 1;

    D = 8;
    NP = D * 100;
    F = 0.6;
    CR = 0.6;

    fiveMinNeg = OneMinute * -5.0;
    fiveMinPos = OneMinute * 5.0;
    fortyMinPos = OneMinute * 40;
    population = (PINDIVIDUAL) malloc(sizeof (INDIVIDUAL) * NP);
    mutatedPopulation = (PINDIVIDUAL) malloc(sizeof (INDIVIDUAL) * NP);

    points = (LPAPPROXPOINT) malloc(sizeof (APPROXPOINT) * length);
    if (points == NULL) {
        printf("Nepodarilo se pridelit pamet");
        exit(1);
    }

    GetBuf(points, step, length, p_filled);

    evolve();
    writeGraph(points, population[minIndex].p, population[minIndex].cg,
            population[minIndex].ecg, population[minIndex].c, population[minIndex].dt,
            population[minIndex].k);
}
