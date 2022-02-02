// FILE:   Lab2_cpu01.c

#include "F28x_Project.h"     // Device Header File and Examples Include File
#include "FIRFilter.h"
#include "AdaptativeFIR.h"

#define OUT_BUFFER_SIZE 256
// Function Prototypes
void ConfigureADC(void);
void ConfigureEPWM(void);
void ConfigureDAC(void);
void SetupADCEpwm(void);

Uint16 calcMedia(Uint16 *vetor);
Uint16 randNum(Uint16 seed);
void ConfigureADC2(void);
//void ConfigureADC2(void);

interrupt void adca1_isr(void);
//interrupt void adcb1_isr(void);
// Variables
#define RESULTS_BUFFER_SIZE 256
Uint16 AdcaResults[RESULTS_BUFFER_SIZE];

float input_data_buffer[LMS_FILTER_LENGTH];
float last_value;
Uint16 inputsIndex;
Uint16 DACoutNumber;
float idealRef;
float lmsErro;

int AdcaResults2[RESULTS_BUFFER_SIZE];
float saida_filtro[OUT_BUFFER_SIZE];
Uint16 adaptWeights;
Uint16 countSaida;
Uint16 sin_index = 0;
Uint16 dest_index = 0;
Uint16 sin_divider = 2;
Uint16 sin_deph = 16;
Uint16 resultsIndex;
Uint16 resultsIndex2;
Uint16 ToggleCount = 0;
Uint16 dacOffset;
Uint16 dacOutput;
Uint16 dacOutput2;
Uint16 sineEnable = 0;
Uint16 sineEnable2 = 0;

FIRFilter lpfMic;
//LMSFIR secPathFilt;

extern int QuadratureTable[40];


void main(void)
{
	// Initialize System Control
    InitSysCtrl();

    FIRFilter_Init(&lpfMic);
//    LMSFIR_Init(&secPathFilt);

    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 1;
    EDIS;

    // Initialize GPIO
    InitGpio(); 							// Configure default GPIO
    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO18 = 1;		// Used as input to ADC
    GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;		// Drives LED LD2 on controlCARD
    EDIS;
    GpioDataRegs.GPADAT.bit.GPIO18 = 0; 	// Force GPIO18 output LOW
    GpioDataRegs.GPADAT.bit.GPIO31 = 1;		// Turn off LED

    // Clear all interrupts and initialize PIE vector table
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    // Map ISR functions
    EALLOW;
    PieVectTable.ADCA1_INT = &adca1_isr;    // Function for ADCA interrupt 1
//    PieVectTable.ADCB1_INT = &adcb1_isr;    // Function for ADCA interrupt 1
    EDIS;

    // Configure the ADC and power it up
    ConfigureADC();

    // Configure the ePWM
    ConfigureEPWM();

    // Configure DAC-B
    ConfigureDAC();

    // Setup the ADC for ePWM triggered conversions on channel 0
    SetupADCEpwm();

    // Initialize results buffer
    for(resultsIndex = 0; resultsIndex < RESULTS_BUFFER_SIZE; resultsIndex++)
    {
        AdcaResults[resultsIndex] = 0;
        AdcaResults2[resultsIndex] = 0;
    }
    resultsIndex = 0;

    for(inputsIndex = 0; inputsIndex < LMS_FILTER_LENGTH; inputsIndex++)
    {
        input_data_buffer[inputsIndex] = 0.0f;
    }
    inputsIndex = 0;
    DACoutNumber = 0;
    idealRef = 0;
    adaptWeights = 0;



    for(countSaida=0; countSaida < 64; countSaida++)
        saida_filtro[countSaida]= 0.0f;
    countSaida=0;
    // Enable PIE interrupt
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;

    // Enable global interrupts and higher priority real-time debug events
    IER |= M_INT1; 			// Enable group 1 interrupts
    EINT;  					// Enable Global interrupt INTM
    ERTM;  					// Enable Global real-time interrupt DBGM

    // Sync ePWM
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    // Start ePWM
    EPwm2Regs.TBCTL.bit.CTRMODE = 0; 			// Un-freeze and enter up-count mode

    do {

      	GpioDataRegs.GPADAT.bit.GPIO31 = 0;		// Turn on LED
      	DELAY_US(1000 * 500);					// ON delay
      	GpioDataRegs.GPADAT.bit.GPIO31 = 1;   	// Turn off LED
      	DELAY_US(1000 * 500);					// OFF delay

    } while(1);
}

Uint16 calcMedia(Uint16 *vetor){
    unsigned long soma = 0;
    unsigned short i;
    for(i=0; i<RESULTS_BUFFER_SIZE; i++)
        soma = soma + vetor[i];
    soma = soma/RESULTS_BUFFER_SIZE;
    return (Uint16) soma;

}

Uint16 randNum(Uint16 seed){
    return (161*seed + 62)%2048;
}


// Write ADC configurations and power up the ADC for both ADC A and ADC B
void ConfigureADC(void)
{
	EALLOW;
	AdcaRegs.ADCCTL2.bit.PRESCALE = 6; 			// Set ADCCLK divider to /4
	AdcaRegs.ADCCTL2.bit.RESOLUTION = 0; 		// 12-bit resolution
	AdcaRegs.ADCCTL2.bit.SIGNALMODE = 0; 		// Single-ended channel conversions (12-bit mode only)
	AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;		// Set pulse positions to late
	AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;			// Power up the ADC

//    AdcbRegs.ADCCTL2.bit.PRESCALE = 6;          // Set ADCCLK divider to /4
//    AdcbRegs.ADCCTL2.bit.RESOLUTION = 0;        // 12-bit resolution
//    AdcbRegs.ADCCTL2.bit.SIGNALMODE = 0;        // Single-ended channel conversions (12-bit mode only)
//    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;       // Set pulse positions to late
//    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;          // Power up the ADC


	DELAY_US(1000);								// Delay for 1ms to allow ADC time to power up



	EDIS;
}


void ConfigureEPWM(void)
{
	EALLOW;
	// Assumes ePWM clock is already enabled
	EPwm2Regs.TBCTL.bit.CTRMODE = 3;            // Freeze counter
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;			// TBCLK pre-scaler = /1
	EPwm2Regs.TBPRD = 0x07D0;			        // Set period to 2000 counts (50kHz)
	EPwm2Regs.ETSEL.bit.SOCAEN	= 0;	        // Disable SOC on A group
	EPwm2Regs.ETSEL.bit.SOCASEL	= 2;	        // Select SOCA on period match
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; 			// Enable SOCA
//    EPwm2Regs.ETPS.bit.SOCAPRD = 1;             // Generate pulse on 1st event
    EPwm2Regs.ETPS.bit.SOCAPRD = ET_1ST;             // Generate pulse on 1st event
	EDIS;
}


void ConfigureDAC(void)
{
    EALLOW;
    DacbRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DacbRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
    DacbRegs.DACVALS.all = 0x0800;              // Set mid-range
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC

    DacaRegs.DACCTL.bit.DACREFSEL = 1;          // Use ADC references
    DacaRegs.DACCTL.bit.LOADMODE = 0;           // Load on next SYSCLK
    DacaRegs.DACVALS.all = 0x0800;              // Set mid-range
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DAC


    EDIS;
}


void SetupADCEpwm(void)
{
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 2;  		// SOC0 will convert pin A0
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 14; 		// Sample window is 100 SYSCLK cycles
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 7; 		// Trigger on ePWM2 SOCA/C

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 3;          // SOC0 will convert pin A0
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0x01;      // End of SOC0 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        // Enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Make sure INT1 flag is cleared


//	AdcbRegs.ADCSOC1CTL.bit.CHSEL = 2;          // SOC0 will convert pin A0
//	AdcbRegs.ADCSOC1CTL.bit.ACQPS = 14;         // Sample window is 100 SYSCLK cycles
//	AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 7;        // Trigger on ePWM2 SOCA/C
//
//    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 0;      // End of SOC0 will set INT1 flag
//    AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;        // Enable INT1 flag
//    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Make sure INT1 flag is cleared

	EDIS;
}


interrupt void adca1_isr(void)
{
	// Read the ADC result and store in circular buffer
    //Uint16 var = AdcaResultRegs.ADCRESULT0 - 1700;
    //var = var*20;
    AdcaResults2[resultsIndex] = AdcaResultRegs.ADCRESULT1;
	AdcaResults[resultsIndex++] = AdcaResultRegs.ADCRESULT0;
	int temporario = (int) AdcaResultRegs.ADCRESULT0 - (int) calcMedia(AdcaResults);

	float temp_ = (float) temporario;
    temp_ = temp_ / 2045.0f;

    FIRFilter_Update(&lpfMic, temp_);
//    temp_ = lpfMic.out*2045.0;

//    input_data_buffer[inputsIndex++] = lpfMic.out;


//    input_data_buffer[inputsIndex++] = temp_;
////
//    LMSFIR_Output(&secPathFilt, input_data_buffer, inputsIndex);
//    lmsErro = secPathFilt.out - idealRef;
////
//    saida_filtro[countSaida++] = secPathFilt.out;
    saida_filtro[countSaida++] = lpfMic.out;
////
//    if (adaptWeights  != 0){
//        LMSFIR_UpdateW(&secPathFilt, idealRef, input_data_buffer, inputsIndex);
//    }

 //    lmsErro = secPathFilt.curr_error;

    if (countSaida>=OUT_BUFFER_SIZE){
        countSaida=0;
    }

//    sinal aleatorio para identificacao do sistema
//    DACoutNumber = randNum(DACoutNumber);

//  exemplo de filtro digital passa baixa para display
//    FIRFilter_Update(&lpfMic, temp_);
//    temp_ = lpfMic.out*2045.0;
//    temporario = (int) temp_;
//    AdcaResults2[resultsIndex] = temporario;


    if(LMS_FILTER_LENGTH <= inputsIndex)
     {
            inputsIndex = 0;
     }

	if(RESULTS_BUFFER_SIZE <= resultsIndex)
	{
		resultsIndex = 0;
	}

	if (resultsIndex % sin_divider == 0){
	    sin_index ++;
	}
	if(RESULTS_BUFFER_SIZE <= sin_index){
	    sin_index = 0;
	}

	// Toggle GPIO18 so we can read it with the ADC
	if (ToggleCount++ >= 15)
	{
		GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1;
		ToggleCount = 0;
	}

	// Write to DACB to create input to ADC-A0
	if (sineEnable != 0)
	{
		dacOutput = dacOffset + ((QuadratureTable[sin_index % 0x20] ^ 0x8000) >> 5);
	}
	else
	{
		dacOutput = dacOffset;
	}

    if (sineEnable2 != 0)
    {
        dacOutput2 = dacOffset + ((QuadratureTable[(sin_index +sin_deph )% 0x20] ^ 0x8000) >> 5);
    }
    else
    {
        dacOutput2 = dacOffset;
    }
    if (sineEnable == 0){

        DACoutNumber = 0;
    }

//        DacbRegs.DACVALS.all = DACoutNumber;
        DacbRegs.DACVALS.all = dacOutput;
    	int normalizado =  DACoutNumber-1023;
    	idealRef = (float) normalizado/1024.0f;

//    	DacbRegs.DACVALS.all = dacOutput;
//	DacaRegs.DACVALS.all = dacOutput2;
//	DacaRegs.DACVALS.all = (temporario>>5);




	// Return from interrupt
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 		// Clear ADC INT1 flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;		// Acknowledge PIE group 1 to enable further interrupts
}

//interrupt void adcb1_isr(void)
//{
//    // Read the ADC result and store in circular buffer
//    //Uint16 var = AdcaResultRegs.ADCRESULT0 - 1700;
//    //var = var*20;
//    AdcaResults2[resultsIndex2++] = AdcbResultRegs.ADCRESULT0;
////  AdcaResults2[resultsIndex] = AdcaResultRegs.ADCRESULT1;
//    if(RESULTS_BUFFER_SIZE <= resultsIndex2)
//    {
//        resultsIndex2 = 0;
//    }
//
//
//    // Return from interrupt
//    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      // Clear ADC INT1 flag
//    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;     // Acknowledge PIE group 1 to enable further interrupts
//}

 // end of file
