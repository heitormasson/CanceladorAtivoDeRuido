// FILE:   Lab2_cpu01.c

#include "F28x_Project.h"     // Device Header File and Examples Include File
#include "FIRFilter.h"
#include "AdaptativeFIR.h"

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

#define MEAN_BUFFER_SIZE 256
int increment = 1;
unsigned long soma_vetor1;
unsigned long soma_vetor2;
float last_erro = 50000;
//Uint16 RefMicReadings[MEAN_BUFFER_SIZE]; // x(n) - leitura bruta;
Uint16 ErrorMicReadings[MEAN_BUFFER_SIZE]; // y(n)
float normRef[LMS_FILTER_LENGTH]; // xx(n) - leitura normalizada e sem DC
Uint16 buffer_saida[MEAN_BUFFER_SIZE];
Uint16 delay = 0;
//float forwardRef[LMS_FILTER_LENGTH]; // x*(n)
Uint16 readingsIndex;
Uint16 buffersIndex;
float lmsErro;
Uint16 adaptWeights = 0;
Uint16 cancelSignal = 0;
Uint16 correction_val;

//FIRFilter forwardPath;
LMSFIR adaptativeFilter;

extern int QuadratureTable[40];
Uint16 sineEnable= 0;
Uint16 sin_index = 0;
Uint16 sin_divider = 1;
Uint16 dacOffset = 0;
Uint16 dacOutput;


void main(void)
{
	// Initialize System Control
    InitSysCtrl();


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

//    FIRFilter_Init(&forwardPath);
    LMSFIR_Init(&adaptativeFilter);
    soma_vetor1 = 0;
    soma_vetor2 = 0;

    for (readingsIndex=0; readingsIndex < LMS_FILTER_LENGTH; readingsIndex++){

        normRef[readingsIndex] = 0.0f;
//        forwardRef[readingsIndex] = 0.0f;
    }
    for (buffersIndex=0; buffersIndex < MEAN_BUFFER_SIZE; buffersIndex++){
//        RefMicReadings[buffersIndex] = 0;
        ErrorMicReadings[buffersIndex] = 0;
    }
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

//Uint16 calcMedia(Uint16 *vetor){
//    unsigned long soma = 0;
//    unsigned short i;
//    for(i=0; i<RESULTS_BUFFER_SIZE; i++)
//        soma = soma + vetor[i];
//    soma = soma/RESULTS_BUFFER_SIZE;
//    return (Uint16) soma;
//
//}

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

float calc_erro(float *vec){
    Uint16 i;
    float erro=0;
    for(i=0; i<LMS_FILTER_LENGTH; i++){
        erro += vec[i]*vec[i];
    }

    return erro;

}

interrupt void adca1_isr(void)
{

//    soma_vetor1 += AdcaResultRegs.ADCRESULT1- RefMicReadings[buffersIndex];
//	int temporario = (int) AdcaResultRegs.ADCRESULT1 - (int) soma_vetor1/LMS_FILTER_LENGTH; // remocao DC
//	float temp_ = (float) temporario;
//    temp_ = temp_ / 2045.0f; // normalizacao
//    normRef[readingsIndex] = temp_;  // sinal de referencia normalizado
    Uint16 last = ErrorMicReadings[buffersIndex];
    ErrorMicReadings[buffersIndex] = AdcaResultRegs.ADCRESULT1;
    soma_vetor2 += ErrorMicReadings[buffersIndex] - last;
    int temporario = (int) ErrorMicReadings[buffersIndex] - (int) (soma_vetor2/MEAN_BUFFER_SIZE);
    float temp_ = (float) temporario;
    temp_ = temp_ / 2045.0f;  // sinal de erro normalizado

    // Correcao dos pesos
//    if (cancelSignal  != 0){
//        LMSFIR_UpdateW_acoustic(&adaptativeFilter, temp_, forwardRef, readingsIndex);
//        LMSFIR_UpdateW_acoustic(&adaptativeFilter, temp_, normRef, readingsIndex);

//    }

//    soma_vetor1 += dacOutput- RefMicReadings[buffersIndex];
    temporario = (int) dacOutput - 1024;//(int) soma_vetor1/MEAN_BUFFER_SIZE; // remocao DC
    temp_ = (float) temporario;
    temp_ = temp_ / 1024.0f; // normalizacao
    normRef[readingsIndex] = temp_;  // sinal de refe

    temp_ = calc_erro(normRef);
    if (temp_> last_erro){
            increment = increment*(-1);
        }
    last_erro = temp_;


      if(cancelSignal != 0){
          if(adaptWeights != 0){
              delay = delay + increment;
              if (delay>MEAN_BUFFER_SIZE-1){
                  delay = MEAN_BUFFER_SIZE-1;

              }
          }
          int newIndex = buffersIndex-delay;
          if (newIndex < 0){
              newIndex = MEAN_BUFFER_SIZE-1 + newIndex;
          }
          correction_val = buffer_saida[buffersIndex-delay];
      }else{
          correction_val = 0;
      }


    // Atualiza buffer de entradas para calculo da media (remocao DC)
//    ErrorMicReadings[buffersIndex] = AdcaResultRegs.ADCRESULT1;
//    RefMicReadings[buffersIndex++] = AdcaResultRegs.ADCRESULT1;
//    RefMicReadings[buffersIndex++] = dacOutput;


//    forwardRef[readingsIndex] = FIRFilter_UpdateLite(normRef, readingsIndex, LMS_FILTER_LENGTH);


    // Calculo da saida
//    LMSFIR_Output(&adaptativeFilter, normRef, readingsIndex); //filtro adaptativo
//    LMSFIR_Output(&adaptativeFilter, normRef, readingsIndex); //filtro adaptativo



//    if (cancelSignal != 0){
//        temp_ = 1024*adaptativeFilter.out+1024;
//        correction_val = (Uint16) temp_;
//    }else{
//        correction_val = 0;
//    }

//    lmsErro = adaptativeFilter.curr_error;

    readingsIndex ++;

    if(LMS_FILTER_LENGTH <= readingsIndex)
     {
        readingsIndex = 0;
     }


	if (buffersIndex % sin_divider == 0){
	    sin_index ++;
	}
	if(32 <= sin_index){
	    sin_index = 0;
	}

	// Toggle GPIO18 so we can read it with the ADC
//	if (ToggleCount++ >= 15)
//	{
//		GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1;
//		ToggleCount = 0;
//	}

	// Write to DACB to create input to ADC-A0
	if (sineEnable != 0)
	{
		dacOutput = dacOffset + ((QuadratureTable[sin_index % 0x20] ^ 0x8000) >> 5);
	}
	else
	{
		dacOutput = dacOffset;
	}

	buffer_saida[buffersIndex] = dacOutput;
	buffersIndex++;

    if(MEAN_BUFFER_SIZE <= buffersIndex){
        buffersIndex = 0;
    }

	dacOutput = dacOffset + ((QuadratureTable[sin_index % 0x20] ^ 0x8000) >> 5);


//	if (cancelSignal != 0){
//	    temp_ = 1024*adaptativeFilter.out+1023;
//	    correction_val = (Uint16) temp_;
//	}else{
//	    correction_val = 0;
//	}

	if (cancelSignal != 0){
	    DacaRegs.DACVALS.all =dacOutput;
	}else{
	    DacaRegs.DACVALS.all = 0;
	}
	if (sineEnable != 0){
	    DacbRegs.DACVALS.all = dacOutput;
	}else{
	    DacbRegs.DACVALS.all = 0;
	}


//        DacbRegs.DACVALS.all = DACoutNumber;
	DacbRegs.DACVALS.all = correction_val;

    DacaRegs.DACVALS.all = dacOutput;

//    	DacbRegs.DACVALS.all = dacOutput;
//	DacaRegs.DACVALS.all = (temporario>>5);




	// Return from interrupt
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; 		// Clear ADC INT1 flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;		// Acknowledge PIE group 1 to enable further interrupts
}
