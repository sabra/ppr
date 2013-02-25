/* 
 * File:   Evolution.h
 * Author: Sabra
 *
 * Created on 3. únor 2013, 21:21
 */

#ifndef EVOLUTION_H
#define	EVOLUTION_H

typedef struct _INDIVIDUAL {
    int t;
    double ecg;
    double p;
    double cg;
    double c;
    double k;
    double dt;
    double vof;
} INDIVIDUAL, *PINDIVIDUAL;

extern void evolution();

#endif	/* EVOLUTION_H */

