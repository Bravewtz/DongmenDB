//
// Created by sam on 2018/11/16.
//

#ifndef DONGMENDB_EXECUTIONPLAN_H
#define DONGMENDB_EXECUTIONPLAN_H

#include <dongmendb/dongmendb.h>
#include <dongmensql/sra.h>
#include <parser/statement.h>
#include <physicalplan/Scan.h>

/*执行计划实现*/
class ExecutionPlan {
    public :
    Scan* generateSelect(dongmendb *db, SRA_t *sra, Transaction *tx);

    Scan* generateScan(dongmendb *db, SRA_t *sra, Transaction *tx);

    int executeInsert(dongmendb *db, char *tableName,  vector<char*> fieldNames,  vector<variant*> values, Transaction *tx);
    int executeUpdate(dongmendb *db, sql_stmt_update *sqlStmtUpdate, Transaction *tx);
    int executeDelete(dongmendb *db, sql_stmt_delete *sqlStmtDelete, Transaction *tx);
};


#endif //DONGMENDB_EXECUTIONPLAN_H
