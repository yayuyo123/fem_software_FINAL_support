#ifndef MODELING_RCS_H
#define MODELING_RCS_H

typedef enum {
	MODELING_RCS_SUCCESS = 0,  // 成功
    MODELING_RCS_ERROR = 1     // 失敗
} ModelingRcsResult;

ModelingRcsResult modeling_rcs(const char *inputFileName, const char *outputFileName);

#endif