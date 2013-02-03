/* 
 * File:   ApproxIface.h
 * Author: Sabra
 *
 * Created on 3. únor 2013, 20:41
 */

#ifndef APPROXIFACE_H
#define	APPROXIFACE_H
#define floattype double
typedef long HRESULT;

typedef struct _APPROXPOINT {
    char time[9];
    floattype bg;
    floattype ig;
    floattype mod;
} APPROXPOINT, *LPAPPROXPOINT;

extern LPAPPROXPOINT data;
extern int size_data;
extern const floattype OneMinute;
extern const floattype OneSecond;
extern int max_i;
extern int min_i;
extern int max_b;
extern int min_b;

extern HRESULT GetBuf(LPAPPROXPOINT points, floattype step, int len, int *filled);

#endif	/* APPROXIFACE_H */

