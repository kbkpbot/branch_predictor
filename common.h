///////////////////////////////////////////////////////////////////////
////  Copyright 2023 by mars.                                        //
///////////////////////////////////////////////////////////////////////

#ifndef __BT9_COMMON_H__
#define __BT9_COMMON_H__

#ifdef _WIN32
#define strncpy(a,b,c) strcpy_s(a,c,b)
#else
#define _strcmpi strcasecmp
#endif

typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef int                INT32;
typedef unsigned long long UINT64;

typedef enum {
    OPTYPE_OP = 2,

    OPTYPE_RET_UNCOND,
    OPTYPE_JMP_DIRECT_UNCOND,
    OPTYPE_JMP_INDIRECT_UNCOND,
    OPTYPE_CALL_DIRECT_UNCOND,
    OPTYPE_CALL_INDIRECT_UNCOND,

    OPTYPE_RET_COND,
    OPTYPE_JMP_DIRECT_COND,
    OPTYPE_JMP_INDIRECT_COND,
    OPTYPE_CALL_DIRECT_COND,
    OPTYPE_CALL_INDIRECT_COND,

    OPTYPE_ERROR,

    OPTYPE_MAX
} OpType;

struct BT9_NODE {
    UINT64 virtual_address;
    UINT64 physical_address;
    UINT64 opcode;
    OpType optype;
    UINT32 size;
};

// EDGE id不能超过2^32-1，否则UINT32类型放不下
struct BT9_EDGE {
    UINT32 src_id;
    UINT32 dest_id;
    char taken;
    UINT64 br_virt_target;
    UINT64 br_phy_target;
    UINT64 inst_cnt;
    UINT64 traverse_cnt;
};

struct BT9_struct {
    UINT32 bt9_minor_version;
    UINT32 has_physical_address;
    UINT32 md5_checksum;
    UINT32 conversion_date;
    char original_stf_input_file[256];
    UINT64 total_instruction_count;
    UINT64 branch_instruction_count;
    UINT64 invalid_physical_branch_target_count;
    UINT64 A32_instruction_count;
    UINT64 A64_instruction_count;
    UINT64 T32_instruction_count;
    UINT64 unidentified_instruction_count;
    UINT32 BT9_NODE_count;
    UINT32 BT9_EDGE_count;
    UINT32 BT9_TRACE_count;
    struct BT9_NODE *NODE;
    struct BT9_EDGE *EDGE;
    UINT32 *TRACE;
};

typedef enum {
    PROCESS_START,
    PROCESS_NODE,
    PROCESS_EDGE,
    PROCESS_TRACE
} PROCESS_STATE_ENUM;

void PREDICTOR_init(void);
char GetPrediction(UINT64 PC);
void UpdatePredictor(UINT64 PC, OpType opType, char resolveDir, char predDir, UINT64 branchTarget);
void PREDICTOR_free(void);

int LoadBT9(struct BT9_struct *BT, char *filename);
void FreeBT9(struct BT9_struct *BT);
double SimBT9(struct BT9_struct *BT);

#endif