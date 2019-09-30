//
// Created by Sam on 2018/2/13.
//

#include <dongmensql/sqlstatement.h>
#include <parser/StatementParser.h>

/**
 * 在现有实现基础上，实现delete from子句
 *
 *  支持的delete语法：
 *
 *  DELETE FROM <table_nbame>
 *  WHERE <logical_expr>
 *
 * 解析获得 sql_stmt_delete 结构
 */

sql_stmt_delete *DeleteParser::parse_sql_stmt_delete(){

    //存储数据表名称
    char *table_name = nullptr;

    // 匹配 where 条件
    SRA_t *where = nullptr;

    //匹配delete
    Token *token = parseNextToken();
    if(matchToken(TOKEN_RESERVED_WORD, "delete") == 0){
        strcpy(this->parserMessage, \
               "ERROR DELETE SQL PARSE IN parse_sql_delete \n");
        return nullptr;
    }
    //获取表名
    token = parseNextToken();
    if(token->type == TOKEN_WORD){
        table_name = new_id_name();
        strcpy(table_name,token->text);
    }
    else{
        strcpy(this->parserMessage, \
               "ERROR SQL: MISSING TABLE NAME \n");
        return nullptr;
    }
    //匹配where
    // 此时 currToken 肯定不为 NULL 且并非我们所需要的数据，所以吃掉读取下一个
    token = parseEatAndNextToken();
    if(matchToken(TOKEN_RESERVED_WORD,"where") == 0){
        strcpy(this->parserMessage, "ERROR SQL: MISSING WHERE");
        return nullptr;
    }
    // 解析 where 表达式
    where = SRATable(TableReference_make(table_name, nullptr));
    Expression *cond = parseExpressionRD();
    where = SRASelect(where,cond);

    // 构造返回值
    auto * sqlStmtDelete = (sql_stmt_delete *)
            calloc(sizeof(sql_stmt_delete),1);
    sqlStmtDelete->tableName = table_name;
    sqlStmtDelete->where = where;
    return sqlStmtDelete;

};