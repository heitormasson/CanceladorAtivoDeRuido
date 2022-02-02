/*
 * AdaptativeFIR.h
 *
 *  Created on: 7 de dez de 2021
 *      Author: heito
 */

#ifndef ADAPTATIVEFIR_H_
#define ADAPTATIVEFIR_H_

#include "F28x_Project.h"

#define LMS_FILTER_LENGTH 128
#define mu 0.00005

typedef struct {
    float W[LMS_FILTER_LENGTH];
    float curr_error;
    float out;
}LMSFIR;

void LMSFIR_Init(LMSFIR *fir);
void LMSFIR_Output(LMSFIR *fir, float *x, Uint16 sample_pointer);

void LMSFIR_UpdateW(LMSFIR *fir, float desired, float *X, Uint16 bufferPointer);
void LMSFIR_UpdateW_acoustic(LMSFIR *fir, float erro, float *X, Uint16 bufferPointer);

#endif /* ADAPTATIVEFIR_H_ */
