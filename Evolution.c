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
#include <pthread.h>
#include "ApproxIface.h"
#include "Evolution.h"
#include "main.h"
#include "Graph.h"

int length;
int filled;
int *pFilled;
floattype step;
static LPAPPROXPOINT points;

static double F;
static double CR;
static int NP;
static int D = 8;

static PINDIVIDUAL population;
static PINDIVIDUAL mutatedPopulation;
static pthread_mutex_t barrierLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condBarrier = PTHREAD_COND_INITIALIZER;

const floattype OneMinute = 1.0 / (24.0 * 60.0);
const floattype OneSecond = 1.0 / (24.0 * 60.0 * 60.0);
static double fiveMinNeg;
static double fiveMinPos;
static int maxDt = 2400;
int vofMinIndex = -1;
double vofMin = 1000;
int vofMaxIndex = -1;
double vofMax = 0;
static int generations = 10000;


static int barrierReadCounter = 0; // číslo jedince, který bude vláknem počítán
static int barrierWriteCounter = 0; // počet zpracovaných - vlákno které dosáhne max. počtu probudí ostatní, maximum určuje NP - počet jedinců v populaci 
static int end = 0; // slouží k ukončení výpočtu
static double sum = 0; // průběžný výpočet průměrného absolutního rozdílu
static double sumFinal = 0; // celkový průměrný absolutní rozdíl

static int penalization(INDIVIDUAL vector);
static void generateIndividual();
static void *threaded();
static void mutatePopulation();
static double probability();

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
    double left = p * bt + cg * bt * ecg + c;
    double right = points[indexI].ig;

    double vof = fabs(right - left); // hodnota ucelove funkce (value of objective function)

    population[i].t = t;
    population[i].c = c;
    population[i].cg = cg;
    population[i].dt = dt;
    population[i].ecg = ecg;
    population[i].k = k;
    population[i].p = p;
    population[i].vof = vof;

}

static void *threaded() {
    while (1) {
        pthread_mutex_lock(&barrierLock);
        while (barrierReadCounter == NP) {
            pthread_cond_wait(&condBarrier, &barrierLock); // uspi se a pri probuzeni se opet testuje podminka
            if (generations == 0) { // po dosažení počtu generací ukonči vlákno
                pthread_mutex_unlock(&barrierLock);
                return NULL;
            }
        }
        if (end) { // po dosažení počtu generací ukonči vlákno
            pthread_mutex_unlock(&barrierLock);
            return NULL;
        }
        int i = barrierReadCounter;
        barrierReadCounter++; // počet přečtených jedinců
        pthread_mutex_unlock(&barrierLock);

        mutatePopulation(i);

        pthread_mutex_lock(&barrierLock);
        barrierWriteCounter++; //počet prošlých jedinců
        if (barrierWriteCounter == NP) {

            sumFinal += (sum / NP);
            generations--;
            population = mutatedPopulation; //nahrazení populace po skončení generace

            barrierReadCounter = 0;
            barrierWriteCounter = 0;

            if (generations == 0) {
                end = 1;
            }
            pthread_cond_broadcast(&condBarrier);
        }
        pthread_mutex_unlock(&barrierLock);
        if (end) {
            return (NULL);
        }
    }
}

static void mutatePopulation(int i) {
    int a = 0, b = 0, c = 0;
    double vof = 0.0;

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

    int penale = penalization(noiseVector);
    
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
        /*mutatedPopulation[i].t = population[i].t;
        mutatedPopulation[i].c = population[i].c;
        mutatedPopulation[i].cg = population[i].cg;
        mutatedPopulation[i].dt = population[i].dt;
        mutatedPopulation[i].ecg = population[i].ecg;
        mutatedPopulation[i].k = population[i].k;
        mutatedPopulation[i].p = population[i].p;
        mutatedPopulation[i].vof = population[i].vof;*/
        penale = 100000000;
    } else {
        double bt = points[noiseVector.t].bg;
        double left = noiseVector.p * bt + noiseVector.cg * bt * noiseVector.ecg + noiseVector.c;
        double right = points[indexI].ig;

        double vof = fabs(right - left); // hodnota ucelove funkce (value of objective function)

        if (vof > vofMax) {
            vofMax = vof;
            vofMaxIndex = i;
        }
        vof += penale;
    }
   
    if (vof < vofMin) {
        vofMin = vof;
        vofMinIndex = i;
    }
    
    if (population[i].vof > vof) {
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

static double probability() {
    double prob = (double) (rand() % 101) / 100.0;
    return prob;
}

void evolution() {
    srand(time(NULL));
    int processes = 1;
    length = count;
    filled;
    pFilled = &filled;
    step = 1;
    int i, rc;
    pthread_t threads[processes];

    D = 8;
    NP = D * 100;
    F = 0.6;
    CR = 0.6;

    fiveMinNeg = OneMinute * -5.0;
    fiveMinPos = OneMinute * 5.0;

    population = (PINDIVIDUAL) malloc(sizeof (INDIVIDUAL) * NP);
    mutatedPopulation = (PINDIVIDUAL) malloc(sizeof (INDIVIDUAL) * NP);

    points = (LPAPPROXPOINT) malloc(sizeof (APPROXPOINT) * length);
    if (points == NULL) {
        printf("Nepodarilo se pridelit pamet");
        exit(1);
    }

    GetBuf(points, step, length, pFilled);

    for (i = 0; i < NP; i++) {
        generateIndividual(i);
    }

    for (i = 0; i < processes; ++i) {
        if (rc = pthread_create(&threads[i], NULL, threaded, NULL)) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
        }
    }

    for (i = 0; i < processes; ++i) {
        if (rc = pthread_join(threads[i], NULL)) {
            fprintf(stderr, "error: pthread_join, rc: %d\n", rc);
        }
    }

    writeGraph(points, population[vofMinIndex].p, population[vofMinIndex].cg,
            population[vofMinIndex].ecg, population[vofMinIndex].c, population[vofMinIndex].dt,
            population[vofMinIndex].k);
}
