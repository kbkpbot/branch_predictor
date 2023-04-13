#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "getopt.h"
#include "cbsl.h"
#include "common.h"
#define MAX_TRACE_NUM 30

#define MIN_MISS_PER_1K 30
#define MAX_MISS_PER_1K 100

#define CBSL_ERROR_CHECK(X)  {if ((X) == cbsl_error) { fprintf(stderr, "error: %s\n", (#X)); }}
void Trim(char *src);
char *trim(char *str);

extern struct BT9_struct BT9;
void DisplayHelp(char *argv[])
{
    printf("[%s] \t\t%s <trace>.bin\n", __func__, argv[0]);
}

int main(int argc, char *argv[])
{
    int ret_val = -1;
    char *pfilename;
    struct BT9_struct BT9[MAX_TRACE_NUM];
    char linebuf[256];
    char *ptr;
    char real_file_name[512];
    double miss_per_1k[MAX_TRACE_NUM];
    int i;
    clock_t tick1, tick2;

    tick1 = clock();
    memset(BT9,0,MAX_TRACE_NUM*sizeof(struct BT9_struct));
    memset(miss_per_1k,0,sizeof(miss_per_1k));
    printf("Branch Predictor Framework by kbkpbot, 2023\n");
    pfilename = NULL;

    /* check arguments */
    while (1) {
        int c = getopt(argc, argv, "-h");
        if (c == -1) { break; }

        switch (c) {
            case 'h': DisplayHelp(argv); return 1;
            case 1: pfilename = optarg; break;
        }
    }

    if (pfilename) {
        ret_val = LoadBT9(&BT9[0], pfilename);
        if (ret_val == -1) {
            FreeBT9(&BT9[0]);
            return ret_val;
        }
        printf("[%s] Load trace done.\n", __func__);

        SimBT9(&BT9[0]);
        FreeBT9(&BT9[0]);
        return 0;
    } else {
        FILE *fp = fopen("traces/TRACE_LIST.txt","r");
        if (fp == NULL) {
            printf("Open traces/TRACE_LIST.txt fail.\n");
            return -1;
        }
        i = 0;
        while (fgets(linebuf, sizeof(linebuf), fp)) {
            if (linebuf[0] == '#') {
                continue;
            }
            if (i+2 > MAX_TRACE_NUM) {
                printf("Error! Too many traces in traces/TRACE_LIST.txt, max is %d.\n", MAX_TRACE_NUM);
                return -1;
            }
            ptr = trim(linebuf);
            sprintf(real_file_name,"traces/%s", ptr);
            ret_val = LoadBT9(&BT9[i], real_file_name);
            if (ret_val == -1) {
                FreeBT9(&BT9[i]);
                return ret_val;
            }
            printf("[%s] Load trace done.\n", __func__);

            miss_per_1k[i] = SimBT9(&BT9[i]);
            FreeBT9(&BT9[i]);
            i++;
        }
        fclose(fp);
    }

    printf("====================Summary====================\n");
    double total_miss_per_1k = 0;
    for (i=0; i<MAX_TRACE_NUM; i++) {
        if (miss_per_1k[i] == 0) { break; }
        else {
            total_miss_per_1k += miss_per_1k[i];
            printf("%s\t\t= %f\n",BT9[i].original_stf_input_file, miss_per_1k[i]);
        }
    }
    tick2 = clock();
    float seconds = (float)(tick2 - tick1) / CLOCKS_PER_SEC;
    printf("===============================================\n");
    printf("Total MISPRED_PER_1K_INST\t= %0.1f\n", total_miss_per_1k);
    if (total_miss_per_1k > MAX_MISS_PER_1K) { total_miss_per_1k = MAX_MISS_PER_1K; }
    if (total_miss_per_1k < MIN_MISS_PER_1K) { total_miss_per_1k = MIN_MISS_PER_1K; }

    int score = (int)(((MAX_MISS_PER_1K - total_miss_per_1k) / (MAX_MISS_PER_1K - MIN_MISS_PER_1K)) * 100.0);
    printf("\nScore = %d(%0.1f) time cost = %0.1f seconds\n",score,total_miss_per_1k,seconds);
}

void Trim(char *src)
{
    char *begin  = src;
    char *end    = src;

    while (*end++);

    if (begin  == end ) { return; }

    while (*begin  == ' ' || *begin  == '\t') {
        ++begin;
    }
    while ((*end) == '\0' || *end  == ' ' || *end  == '\t' || *end == '\n' || *end == '\r') {
        --end;
    }

    if (begin > end ) {
        *src  = '\0';  return;
    }
    while (begin  != end ) {
        *src++ = *begin++;
    }

    *src++ = *end;
    *src  = '\0';

    return;
}

char *rtrim(char *str)
{
    if (str == NULL || *str == '\0') {
        return str;
    }
    int len = strlen(str);
    char *p = str + len - 1;
    while (p >= str && isspace(*p)) {
        *p = '\0'; --p;
    }
    return str;
}


char *ltrim(char *str)
{
    if (str == NULL || *str == '\0') {
        return str;
    }
    int len = 0;
    char *p = str;
    while (*p != '\0' && isspace(*p)) {
        ++p; ++len;
    }
    memmove(str, p, strlen(str) - len + 1);
    return str;
}


char *trim(char *str)
{
    str = rtrim(str);
    str = ltrim(str);
    return str;
}

