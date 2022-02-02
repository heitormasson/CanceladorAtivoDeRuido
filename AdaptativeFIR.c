/*
 * AdaptativeFIR.c
 *
 *  Created on: 7 de dez de 2021
 *      Author: heito
 */

#include "AdaptativeFIR.h"

void LMSFIR_Init(LMSFIR *fir){

    unsigned short n;
    for (n = 1; n < LMS_FILTER_LENGTH; n++){
        fir->W[n] = 0.0f;
    }

    fir->W[0] = 0.9f;
    fir->curr_error = 1.0f;
    fir->out = 0.0f;
}

void LMSFIR_Output(LMSFIR *fir, float *x, Uint16 sample_pointer){

    Uint16 sumIndex = sample_pointer;
    fir->out = 0.0f;                      //filter’output set to zero
    unsigned short i;
    for (i = 0; i < LMS_FILTER_LENGTH; i++){
        if (sumIndex>0){
            sumIndex --;
        }else{
            sumIndex = LMS_FILTER_LENGTH -1;
        }

        fir->out += (fir->W[i] * x[sumIndex]);         //calculate filter output
    }
}

void LMSFIR_UpdateW(LMSFIR *fir, float desired, float *X, Uint16 bufferPointer){
    Uint16 bufPoint = bufferPointer;

    float erro = desired - fir->out;                  //calculate error signal
    unsigned short i;
    for (i = 0; i < LMS_FILTER_LENGTH; i++){
        if (bufPoint>0){
            bufPoint --;
        }else{
            bufPoint = LMS_FILTER_LENGTH-1;
        }

        fir->W[i] = fir->W[i] + (mu * erro * X[bufPoint]);
    }
        fir->curr_error = erro;
}

void LMSFIR_UpdateW_acoustic(LMSFIR *fir, float erro, float *X, Uint16 bufferPointer){
    Uint16 bufPoint = bufferPointer;

    unsigned short i;
    for (i = 0; i < LMS_FILTER_LENGTH; i++){
        if (bufPoint>0){
            bufPoint --;
        }else{
            bufPoint = LMS_FILTER_LENGTH-1;
        }

        fir->W[i] = fir->W[i] + (mu * erro * X[bufPoint]);
    }
        fir->curr_error = erro;
}

