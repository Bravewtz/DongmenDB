#include <physicalplan/ExecutionPlan.h>
#include <physicalplan/TableScan.h>
#include <physicalplan/Select.h>
#include <physicalplan/Project.h>
#include <physicalplan/Join.h>
#include <parser/StatementParser.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <set>
#include <algorithm>

/*执行 update 语句的物理计划，返回修改的记录条数
 * 返回大于等于0的值，表示修改的记录条数；
 * 返回小于0的值，表示修改过程中出现错误。
 * */
/*TODO: plan_execute_update， update语句执行*/



int ExecutionPlan::executeUpdate(DongmenDB *db, sql_stmt_update *sqlStmtUpdate, Transaction *tx){
    /*删除语句以select的物理操作为基础实现。
     * 1. 使用 sql_stmt_update 的条件参数，调用 physical_scan_select_create 创建select的物理计划并初始化;
     * 2. 执行 select 的物理计划，完成update操作
     * */

    Scan * scan;
    scan = generateScan(db,sqlStmtUpdate->where,tx);
    size_t num = 0;
    scan->beforeFirst();

    while(scan->next()){

        for(size_t i = 0; i<sqlStmtUpdate->fieldsExpr.size(); i++){
            char *currentFieldName = sqlStmtUpdate->fields.at(i);
            variant *val = (variant *)calloc(sizeof(variant),1);
            enum data_type field_type = scan->getField(sqlStmtUpdate->tableName,currentFieldName)->type;//获取字段的数据类型

            scan->evaluateExpression(sqlStmtUpdate->fieldsExpr.at(i),scan,val);//计算赋值表达式

            if( field_type!=val->type ){
                fprintf(stdout,"type-error!!!");
                return -1;
            }
            if(val->type == DATA_TYPE_INT){
                scan->setInt(sqlStmtUpdate->tableName,currentFieldName,val->intValue);
            }   else if(val->type == DATA_TYPE_CHAR){
                scan->setString(sqlStmtUpdate->tableName,currentFieldName,val->strValue);
            }
        }
        num++;
    }
    scan->close();
    return num;
};