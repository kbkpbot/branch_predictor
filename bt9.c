#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "cbsl.h"

#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); }}



int LoadBT9(struct BT9_struct *BT9, char *filename)
{
    printf("[%s] Reading trace file[%s], please wait...\n", __func__, filename);
    cbsl_ctx *ctx = cbsl_open(cbsl_load_mode, filename);
    if (ctx == NULL) {
        printf("[%s] Can't open %s for reading.\n", __func__, filename);
        return -1;
    }
    CBSL_ERROR_CHECK(cbsl_read(ctx, BT9, sizeof(struct BT9_struct)));
    strncpy(BT9->original_stf_input_file, filename,255);
    BT9->NODE = (struct BT9_NODE *)malloc(BT9->BT9_NODE_count*sizeof(struct BT9_NODE));
    BT9->EDGE = (struct BT9_EDGE *)malloc(BT9->BT9_EDGE_count*sizeof(struct BT9_EDGE));
    BT9->TRACE = (UINT32 *)malloc(BT9->BT9_TRACE_count*sizeof(UINT32));
    CBSL_ERROR_CHECK(cbsl_read(ctx, BT9->NODE, BT9->BT9_NODE_count * sizeof(struct BT9_NODE)));
    CBSL_ERROR_CHECK(cbsl_read(ctx, BT9->EDGE, BT9->BT9_EDGE_count * sizeof(struct BT9_EDGE)));
    CBSL_ERROR_CHECK(cbsl_read(ctx, BT9->TRACE, BT9->BT9_TRACE_count * sizeof(UINT32)));
    CBSL_ERROR_CHECK(cbsl_close(ctx));

    return 0;
}

void FreeBT9(struct BT9_struct *BT9)
{
    if (BT9->BT9_NODE_count) {
        free(BT9->NODE);
    }
    if (BT9->BT9_EDGE_count) {
        free(BT9->EDGE);
    }
    if (BT9->BT9_TRACE_count) {
        free(BT9->TRACE);
    }
}

double SimBT9(struct BT9_struct *BT9)
{
    UINT64 i;
    UINT64 PC;
    OpType optype;
    char predDir;
    char resolveDir;
    UINT64 branchTarget;
    UINT32 EDGE_id;
    struct BT9_EDGE *EDGE;
    struct BT9_NODE *src_NODE;
    UINT64 MISS_Count = 0LL;
    UINT64 cond_br_Count = 0LL;
    UINT64 uncond_br_Count = 0LL;
    double MISPRED_PER_1K_INST = 1000;

    printf("[%s] Start branch simulation, please wait...\n", __func__);
    PREDICTOR_init();

    // skip the first record, it is invalid
    for (i = 1; i < BT9->BT9_TRACE_count; i++) {
        EDGE_id = BT9->TRACE[i];
        EDGE = &(BT9->EDGE[EDGE_id]);
        src_NODE = &(BT9->NODE[EDGE->src_id]);
        PC = src_NODE->virtual_address;
        resolveDir = EDGE->taken;
        branchTarget = EDGE->br_virt_target;
        optype = src_NODE->optype;

        if (optype == OPTYPE_RET_COND || optype == OPTYPE_JMP_DIRECT_COND  || optype == OPTYPE_JMP_INDIRECT_COND || optype == OPTYPE_CALL_DIRECT_COND || optype == OPTYPE_CALL_INDIRECT_COND) {
            cond_br_Count++;
            predDir = GetPrediction(PC);
            UpdatePredictor(PC, optype, resolveDir, predDir, branchTarget);
            if (resolveDir != predDir) {
                MISS_Count++;
            }
        } else {
            uncond_br_Count++;
        }
    }
    PREDICTOR_free();

    MISPRED_PER_1K_INST = 1000.0*(double)(MISS_Count) / (double)(BT9->total_instruction_count);
    printf("  NUM_INSTRUCTIONS            \t : %10llu\n", BT9->total_instruction_count);
    printf("  NUM_BR                      \t : %10llu\n", BT9->branch_instruction_count-1);
    printf("  NUM_UNCOND_BR               \t : %10llu\n", uncond_br_Count);
    printf("  NUM_CONDITIONAL_BR          \t : %10llu\n", cond_br_Count);
    printf("  NUM_MISPREDICTIONS          \t : %10llu\n", MISS_Count);
    printf("  MISPRED_PER_1K_INST         \t : %10.4f\n", MISPRED_PER_1K_INST);
    return MISPRED_PER_1K_INST;
}
