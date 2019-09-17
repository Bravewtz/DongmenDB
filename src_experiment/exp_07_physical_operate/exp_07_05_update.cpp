//
// Created by sam on 2018/9/18.
//

#include <physicalplan/ExecutionPlan.h>
#include <physicalplan/TableScan.h>
#include <physicalplan/Select.h>
#include <physicalplan/Project.h>
#include <physicalplan/Join.h>

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

    /*记录被更新的条数*/
    Scan *scan = generateScan(db,sqlStmtUpdate->where,tx);//产生扫描计划

    scan->beforeFirst();//遍历指针置前
    size_t updated_count = 0;

    while(scan->next()!=0){
        for(size_t i = 0; i<sqlStmtUpdate->fieldsExpr.size(); i++){
            char *currentFieldName = sqlStmtUpdate->fields.at(i);//获取当前字段名称
            variant *val = (variant *)calloc(sizeof(variant),1);//结构体存储where表达式
            enum data_type field_type = scan->getField(sqlStmtUpdate->tableName,currentFieldName)->type;//获取字段的数据类型

            scan->evaluateExpression(sqlStmtUpdate->fieldsExpr.at(i),scan,val);//计算赋值表达式

            if(val->type != field_type){
                fprintf(stdout,"data-type-error!!!");
                return -1;
            }
            if(val->type == DATA_TYPE_INT){
                scan->setInt(sqlStmtUpdate->tableName,currentFieldName,val->intValue);
            }
            else if(val->type == DATA_TYPE_CHAR){
                scan->setString(sqlStmtUpdate->tableName,currentFieldName,val->strValue);
            }
        }
        /*条数增加*/
        updated_count++;
    }
    /*关闭scan*/
    scan->close();
    return updated_count;

};


/*
 *
 * 第二种代码风格
 */
//
// Created by sam on 2018/9/18.
//

//#include <physicalplan/ExecutionPlan.h>
//
///*执行 update 语句的物理计划，返回修改的记录条数
// * 返回大于等于0的值，表示修改的记录条数；
// * 返回小于0的值，表示修改过程中出现错误。
// * */
///*TODO: plan_execute_update， update语句执行*/
//
//
//int ExecutionPlan::executeUpdate(DongmenDB *db, sql_stmt_update *sqlStmtUpdate, Transaction *tx){
//    /*删除语句以select的物理操作为基础实现。
//     * 1. 使用 sql_stmt_update 的条件参数，调用 physical_scan_select_create 创建select的物理计划并初始化;
//     * 2. 执行 select 的物理计划，完成update操作
//     * */
//
////    fprintf(stderr, "TODO: physical_update is not implemented yet. in plan_execute_update \n");
//
//    // 拆分参数
//    // 存储 数据表 名称，这个地方只能接收到原值char*，下面 string 强转
//    char *tableName = sqlStmtUpdate->tableName;
//
//    // 存储 键 集
//    auto fields = sqlStmtUpdate->fields;
//
//    // 存储 值 集
//    auto fieldsExpr = sqlStmtUpdate->fieldsExpr;
//
//    // 创建表扫描
//    auto *tableScan = generateScan(db, sqlStmtUpdate->where, tx);
//
//    // 移动扫描指针到前部
//    tableScan->beforeFirst();
//
//    // 更新计数
//    int updated = 0;
//
//    // 循环匹配
//    while (tableScan->next() != 0) {
////        vector<char *> FieldsName = tableScan->getFieldsName(tableName);
//        for (size_t i = 0;i < fields.size(); ++i) {
//            char *currFieldName = fields[i];
//            auto *val =
//                    (variant *) calloc(1, sizeof(variant));
//            enum data_type fieldType =
//                    tableScan->getField(tableName, currFieldName)->type;
//            tableScan->evaluateExpression(fieldsExpr[i], tableScan, val);
//
//            if (val->type != fieldType) {
//                printf("Update statement execution error: did not match the same data type \n");
//                return -1;
//            }
//
//            if (val->type == DATA_TYPE_INT) {
//                tableScan->setInt(tableName, currFieldName, val->intValue);
//            } else if (val->type == DATA_TYPE_CHAR) {
//                int maxLength = tableScan->getField(tableName, currFieldName)->length;
//                if (maxLength < strlen(val->strValue)) {
//                    val->strValue[maxLength] = '\0';
//                }
//                tableScan->setString(tableName, currFieldName, val->strValue);
//            } else if (val->type == DATA_TYPE_BOOLEAN) {
////                tableScan->setBoolean
//            }
//        }
//        ++updated;
//    }
//    tableScan->close();
//
//    return updated;
//};