/*
 * FIRfilter.h
 *
 *  Created on: 30 de nov de 2021
 *      Author: heito
 */

#ifndef FIRFILTER_H_
#define FIRFILTER_H_

#include "F28x_Project.h"

#define FIR_FILTER_LENGTH 32

typedef struct {
    float buf[FIR_FILTER_LENGTH];
    unsigned short bufIndex;

    float out;
}FIRFilter;

void FIRFilter_Init(FIRFilter *fir);
float FIRFilter_Update(FIRFilter *fir, float inp);

float FIRFilter_UpdateLite(float *inp, Uint16 sumIndex, Uint16 buffer_size);
#endif /* FIRFILTER_H_ */
