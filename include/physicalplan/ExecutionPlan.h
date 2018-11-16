//
// Created by sam on 2018/11/16.
//

#ifndef DONGMENDB_EXECUTIONPLAN_H
#define DONGMENDB_EXECUTIONPLAN_H


#ifdef __cplusplus

#include <dongmendb/dongmendb.h>
#include <dongmensql/sra.h>
#include <parser/statement.h>
#include <physicalplan/Scan.h>

class ExecutionPlan {
    public :
    Scan *generateSelect(dongmendb *db, SRA_t *sra, transaction *tx);

    Scan *generateScan(dongmendb *db, SRA_t *sra, transaction *tx);

    int executeInsert(dongmendb *db, char *tableName, arraylist *fieldNames, arraylist *values, transaction *tx);
    int executeUpdate(dongmendb *db, sql_stmt_update *sqlStmtUpdate, transaction *tx);
    int executeDelete(dongmendb *db, sql_stmt_delete *sqlStmtDelete, transaction *tx);
};

#endif

#endif //DONGMENDB_EXECUTIONPLAN_H
