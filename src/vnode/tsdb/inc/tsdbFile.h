/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#if !defined(_TD_TSDB_FILE_H_)
#define _TD_TSDB_FILE_H_

#include <stdint.h>

#include "taosdef.h"
#include "tglobalcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define tsdbGetKeyFileId(key, daysPerFile, precision) ((key) / tsMsPerDay[(precision)] / (daysPerFile))
#define tsdbGetMaxNumOfFiles(keep, daysPerFile) ((keep) / (daysPerFile) + 3)

typedef enum {
  TSDB_FILE_TYPE_HEAD = 0,  // .head file type
  TSDB_FILE_TYPE_DATA,      // .data file type
  TSDB_FILE_TYPE_LAST,      // .last file type
  TSDB_FILE_TYPE_MAX
} TSDB_FILE_TYPE;

extern const char *tsdbFileSuffix[];

typedef struct {
  int64_t size;
  int64_t tombSize;
} SFileInfo;

typedef struct {
  int8_t  type;
  char    fname[128];
  int64_t size;      // total size of the file
  int64_t tombSize;  // unused file size
} SFile;

typedef struct {
  int32_t fileId;
  SFile   files[TSDB_FILE_TYPE_MAX];
} SFileGroup;

// TSDB file handle
typedef struct {
  int32_t    daysPerFile;
  int32_t    keep;
  int32_t    minRowPerFBlock;
  int32_t    maxRowsPerFBlock;
  int32_t    maxTables;
  SFileGroup fGroup[];
} STsdbFileH;

/**
 * if numOfSubBlocks == -1, then the SCompBlock is a sub-block
 * if numOfSubBlocks == 1, then the SCompBlock refers to the data block, and offset/len refer to
 *                         the data block offset and length
 * if numOfSubBlocks > 1, then the offset/len refer to the offset of the first sub-block in the
 * binary
 */
typedef struct {
  int64_t last : 1;          // If the block in data file or last file
  int64_t offset : 63;       // Offset of data block or sub-block index depending on numOfSubBlocks
  int32_t algorithm : 8;     // Compression algorithm
  int32_t numOfPoints : 24;  // Number of total points
  int32_t sversion;          // Schema version
  int32_t len;               // Data block length or nothing
  int16_t numOfSubBlocks;    // Number of sub-blocks;
  int16_t numOfCols;
  TSKEY   keyFirst;
  TSKEY   keyLast;
} SCompBlock;

#define IS_VALID_TSDB_FILE_TYPE(type) ((type) >= TSDB_FILE_TYPE_HEAD && (type) < TSDB_FILE_TYPE_MAX)

STsdbFileH *tsdbInitFile(char *dataDir, int32_t daysPerFile, int32_t keep, int32_t minRowsPerFBlock,
                         int32_t maxRowsPerFBlock, int32_t maxTables);

void  tsdbCloseFile(STsdbFileH *pFileH);
int   tsdbCreateFileGroup(char *dataDir, int fileId, SFileGroup *pFGroup, int maxTables);
void  tsdbGetKeyRangeOfFileId(int32_t daysPerFile, int8_t precision, int32_t fileId, TSKEY *minKey, TSKEY *maxKey);
#ifdef __cplusplus
}
#endif

#endif  // _TD_TSDB_FILE_H_