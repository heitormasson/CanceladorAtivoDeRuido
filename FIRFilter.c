/*
 * FIRFilter.c
 *
 *  Created on: 30 de nov de 2021
 *      Author: heito
 */

#include "FIRFilter.h"

static float FIR_IMPULSE_RESPONSE[FIR_FILTER_LENGTH] = {
2.93782234, -0.135437876, -0.350849032, -0.316569805, -0.690199196, 0.0251029693, 0.0110815233, -0.102686539,
0.0181034543, -0.163026437, 0.0855707154, -0.141179964, 0.0288666263, -0.162541494, -0.134327516,
0.247993156,
0.043644242, 0.0594193526, -0.256612718, -0.296213031, -0.0801055357, -0.0961941779, -0.0385826528,
-0.143233404,-0.0978824422, -0.00401659636, 0.14050509, 0.372264892, -0.376340508, -0.279895693, -0.171941191,
0.127244622};


/*
static float FIR_IMPULSE_RESPONSE[FIR_FILTER_LENGTH] =
{2.06012917 , -0.00617272267 , -0.306013137 ,   -0.70566237,0.163583666,0.145536363,0.0314579681, 0.0230406038,
-0.176970363, 0.168195456, -0.032874193, 0.0759650096, 0.085573554, 0.0334629416,  0.133711174, 0.00483287359,
0.348988324, -0.0783319846,  0.164093241, 0.187646627, 0.0451050736,  0.270289093, -0.0417997576, 0.0237649754,
0.167631298, 0.195928454,0.0408395864, 0.37751323, 0.133312255, -0.0902424231,0.222006291,0.602908611,
-2.41486049, -0.126760617, -0.0398559272, 0.509338558,-0.0809198543, -0.117315814,  0.0262808502,0.12974456,
-0.0401457138, -0.372732341, 0.16077739, -0.178415284,   0.15229176,  -0.149976388,   0.0159026571,   -0.0372109637,
0.241691649, -0.0282396954, -0.170504317, 0.0649427772, -0.163204968, -0.122647852, -0.101265542, -0.266129732,
-0.0827431306, -0.180465028, -0.114147335, -0.155480981, -0.288098037, -0.191652402, -0.105208836, -0.0276638009};
*/
/*
static float FIR_IMPULSE_RESPONSE[FIR_FILTER_LENGTH] = {
                                                        -0.00019839343930446154,
                                                        0.0020314614771604115,
                                                        0.004221027497300727,
                                                        0.0032812966005886573,
                                                        -0.0011350750793052637,
                                                        -0.0036611233698619545,
                                                        -0.00019346762995983935,
                                                        0.00454330066442472,
                                                        0.0024637259116995435,
                                                        -0.004634538082613447,
                                                        -0.005456733254380434,
                                                        0.0032434628629257173,
                                                        0.008546930072990348,
                                                        0.000045770959053117864,
                                                        -0.010802290459596794,
                                                        -0.005268220908718177,
                                                        0.011105277041270085,
                                                        0.011966312971017794,
                                                        -0.008320128969087131,
                                                        -0.019138144032920603,
                                                        0.0014554600505982786,
                                                        0.025227171732624717,
                                                        0.010215851577508062,
                                                        -0.028173661476429593,
                                                        -0.027373768814328212,
                                                        0.02511218596788476,
                                                        0.05151295917490789,
                                                        -0.010842466030898326,
                                                        -0.08912561600909188,
                                                        -0.03254418594242589,
                                                        0.18846416251018272,
                                                        0.4004149704314988,
                                                        0.4004149704314988,
                                                        0.18846416251018272,
                                                        -0.03254418594242589,
                                                        -0.08912561600909188,
                                                        -0.010842466030898326,
                                                        0.05151295917490789,
                                                        0.02511218596788476,
                                                        -0.027373768814328212,
                                                        -0.028173661476429593,
                                                        0.010215851577508062,
                                                        0.025227171732624717,
                                                        0.0014554600505982786,
                                                        -0.019138144032920603,
                                                        -0.008320128969087131,
                                                        0.011966312971017794,
                                                        0.011105277041270085,
                                                        -0.005268220908718177,
                                                        -0.010802290459596794,
                                                        0.000045770959053117864,
                                                        0.008546930072990348,
                                                        0.0032434628629257173,
                                                        -0.005456733254380434,
                                                        -0.004634538082613447,
                                                        0.0024637259116995435,
                                                        0.00454330066442472,
                                                        -0.00019346762995983935,
                                                        -0.0036611233698619545,
                                                        -0.0011350750793052637,
                                                        0.0032812966005886573,
                                                        0.004221027497300727,
                                                        0.0020314614771604115,
                                                        -0.00019839343930446154
                                                      };
*/

void FIRFilter_Init(FIRFilter *fir){

    unsigned short n;
    for (n = 0; n < FIR_FILTER_LENGTH; n++){
        fir->buf[n] = 0.0f;
    }

    fir->bufIndex = 0;

    fir->out = 0.0f;
}

float FIRFilter_Update(FIRFilter *fir, float inp){

    fir->buf[fir->bufIndex] = inp;

    fir->bufIndex++;

    if (fir->bufIndex == FIR_FILTER_LENGTH) {

        fir->bufIndex = 0;
    }

    fir->out = 0.0f;
    unsigned short sumIndex = fir->bufIndex;

    unsigned short n;
    for(n=0; n < FIR_FILTER_LENGTH; n++){
        if (sumIndex>0){

            sumIndex--;
        } else {
            sumIndex = FIR_FILTER_LENGTH -1;
        }

        fir->out += FIR_IMPULSE_RESPONSE[n]*fir->buf[sumIndex];


    }
    return fir->out;
}

float FIRFilter_UpdateLite(float *inp, Uint16 sumIndex, Uint16 buffer_size){
    unsigned short n;
    float out_ = 0;
    Uint16 sumIndex_ = sumIndex;
    for(n=0; n < FIR_FILTER_LENGTH; n++){
        if (sumIndex_>0){

            sumIndex_--;
        } else {
            sumIndex_ = buffer_size -1;
        }

        out_ += FIR_IMPULSE_RESPONSE[n]*inp[sumIndex_];
    }
    return out_;
}
