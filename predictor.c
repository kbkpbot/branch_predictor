#include <stdio.h>
#include <stdlib.h>

#include "common.h"

// sat counter increase
static inline UINT8 SatIncrement(UINT8 x, UINT8 max)
{
    if (x<max) { return x + 1; }
    return x;
}

// sat counter decrease
static inline UINT8 SatDecrement(UINT8 x)
{
    if (x>0) { return x - 1; }
    return x;
}

// The state is defined for Gshare, change for your design
UINT32 ghr;             // global history register
UINT8  *pht;            // pattern history table
UINT32 historyLength;   // history length
UINT32 numPhtEntries;   // entries in pht

#define PHT_CTR_MAX  3
#define PHT_CTR_INIT 3

#define HIST_LEN   17   // global history register length, 17 bits

#define TAKEN       'T'
#define NOT_TAKEN   'N'


void PREDICTOR_init(void)
{

    historyLength = HIST_LEN;
    ghr = 0;
    numPhtEntries = (1 << HIST_LEN); 

    pht = (UINT8 *)malloc(numPhtEntries * sizeof(UINT8));

    for (UINT32 ii = 0; ii< numPhtEntries; ii++) {
        pht[ii] = PHT_CTR_INIT;
    }

}

// Gshare branch predictor
char GetPrediction(UINT64 PC)
{

    UINT32 phtIndex = (PC^ghr) % (numPhtEntries);
    UINT8  phtCounter = pht[phtIndex];

    if (phtCounter > (PHT_CTR_MAX / 2)) {
        return TAKEN;
    } else {
        return NOT_TAKEN;
    }
}

void  UpdatePredictor(UINT64 PC, OpType opType, char resolveDir, char predDir, UINT64 branchTarget)
{

    opType = opType;
    predDir = predDir;
    branchTarget = branchTarget;

    UINT32 phtIndex = (PC^ghr) % (numPhtEntries);
    UINT8  phtCounter = pht[phtIndex];

    if (resolveDir == TAKEN) {
        pht[phtIndex] = SatIncrement(phtCounter, PHT_CTR_MAX);
    } else {
        pht[phtIndex] = SatDecrement(phtCounter);
    }

    // update the GHR
    ghr = (ghr << 1);

    if (resolveDir == TAKEN) {
        ghr = ghr | 0x1;
    }
}

void PREDICTOR_free(void)
{
    free(pht);
}
