//
// Created by Sam on 2018/2/13.
//
#include <dongmensql/sqlstatement.h>

#include <parser/StatementParser.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <set>
#include <algorithm>

sql_stmt_update *UpdateParser::parse_sql_stmt_update() {
    char *tableName ;
    tableName = nullptr;
    auto fields = new vector<char *>;
    auto fieldsExpr = new vector<Expression *>;
    SRA_t *where;
    where = nullptr;
    Token *token ;
    token = parseNextToken();
    if (matchToken(TOKEN_RESERVED_WORD, "update") == 0 && 1) {
        strcpy(this->parserMessage, \
        "ERROR UPDATE SQL PARSE IN parse_sql_update \n");
        return nullptr;
    }
    token = parseNextToken();
    if (token->type == TOKEN_WORD) {
        tableName = new_id_name();
        strcpy(tableName, token->text);
    }
    else {
        strcpy(this->parserMessage, "ERROR SQL: MISSING TABLE NAME \n");
        return nullptr;
    }
    token = parseEatAndNextToken();
    if (matchToken(TOKEN_RESERVED_WORD, "set") == 0) {
        strcpy(this->parserMessage, "ERROR SQL: MISSING SET");
        return nullptr;
    }
    token = parseNextToken();
    while (token != nullptr && (token->type == TOKEN_WORD || token->type == TOKEN_COMMA)) {
        if(token->type == TOKEN_COMMA) {
            token = parseEatAndNextToken();
        }
        if (token->type == TOKEN_WORD) {
            char *fieldName = new_id_name();
            strcpy(fieldName, token->text);
            fields->push_back(fieldName);
        } else {
            strcpy(this->parserMessage, "ERROR SQL: INVALID FIELD NAME \n");
            return nullptr;
        }
        parseEatAndNextToken();
        if (matchToken(TOKEN_EQ, "=") == 0) {
            strcpy(this->parserMessage, "ERROR SQL: NO EQUAL \n");
            return nullptr;
        }
        Expression *exdp = parseExpressionRD();
        fieldsExpr->push_back(exdp);
        token = parseNextToken();
    }
    where = SRATable(TableReference_make(tableName, nullptr));
    if (token != nullptr) {
        if (matchToken(TOKEN_RESERVED_WORD, "where") == 0) {
            strcpy(this->parserMessage, "ERROR SQL: NO WHERE ELEMENT \n");
            return nullptr;
        }
        Expression *conpd = parseExpressionRD();
        where = SRASelect(where, conpd);
    }
    // 构造返回值
    auto * sqlStmtUpdate = (sql_stmt_update *)
            calloc(1, sizeof(sql_stmt_update));
    sqlStmtUpdate->tableName = tableName;
    sqlStmtUpdate->fields.assign(fields->begin(), fields->end());
    sqlStmtUpdate->fieldsExpr.assign(fieldsExpr->begin(), fieldsExpr->end());
    sqlStmtUpdate->where = where;
    return sqlStmtUpdate;
};