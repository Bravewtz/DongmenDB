//
// Created by Sam on 2018/2/13.
//

#include <relationalalgebra/optimizer.h>
#include <iostream>
#include <algorithm>
#include <cstdio>

/**
 * 使用关于选择的等价变换规则将条件下推。
 *
 */

/*
 * 判断表达式的操作数链表中为 TOKEN_AND
 */
bool hasLegalKeyword(Expression *pExpression) {
    if(pExpression == nullptr) return false;
    else return pExpression->opType==TOKEN_AND;
}

/*
 * 当前标识符为 TOKEN_AND 时，获取当前的子表达式，也就是两个 AND
 * （或者只有一个 AND 另一个为 nullptr） 中间的子表达式，将其拆下
 * 长表达式，拆分成一个单独的 select 便于第二步中的 select 与 join
 * 的下推交换处理
 */
Expression *interceptSubexpression(Expression *pExpression) {
    if (pExpression == nullptr) return pExpression;
    else if (pExpression->term == nullptr) {
        if (pExpression->opType <= TOKEN_COMMA) {
            int cntOfOperators = 0;
            cntOfOperators = operators[pExpression->opType].numbers;
            pExpression = pExpression->nextexpr;

            while (cntOfOperators--) {
                pExpression = interceptSubexpression(pExpression);
            }
            return pExpression;
        }
    }
    else if (pExpression->term) {
        return pExpression->nextexpr;
    }

    return nullptr;
}

/*
 * 用于根据传入的表达式头部与表达式尾部找到当前子表达式结束的位置
 */
Expression *goSubexpressionEnd(Expression *pFront, Expression *pBack) {
    Expression * point = pFront;
    while (point != nullptr && point->nextexpr != pBack && point->nextexpr != nullptr) {
        point = point->nextexpr;
    }

    return point;
}

/*
 * 拆分一个 sra 表达式，将 AND 左右分拆成子 select 语句
 */
void splitSra(SRA_s **pS) {
    if (pS == nullptr) return ;

    SRA_s * sra = *pS;
    Expression* express = sra->select.cond;
    Expression* operandListFront = express->nextexpr;
    Expression* operandListBack = interceptSubexpression(operandListFront);

    Expression* frontEnd = goSubexpressionEnd(operandListFront, operandListBack);
    frontEnd->nextexpr = nullptr;

    sra->select.sra = SRASelect(sra->select.sra, operandListFront);
    sra->select.cond = operandListBack;
}

/*
    等价变换：将SRA_SELECT类型的节点进行条件串接
*/
void splitAndConcatenate(SRA_s **sra_point){
    // 获取传入关系代数的指针所指向的数据
    SRA_s *sra_data = *sra_point;
    if(sra_data == nullptr) return ;

    if(sra_data->t == SRA_SELECT){//对应语法树的SRA_SELECT结点
        if(hasLegalKeyword(sra_data->select.cond)){
            splitSra(sra_point);
        }
        splitAndConcatenate(&(sra_data->select.sra));
    }
    else if(sra_data->t == SRA_PROJECT){//对应语法树的SRA_PROJECT结点
        splitAndConcatenate(&(sra_data->project.sra));
    }
    else if(sra_data->t == SRA_JOIN){//对应语法树的SRA_JOIN结点
        splitAndConcatenate(&(sra_data->join.sra1));
        splitAndConcatenate(&(sra_data->join.sra2));
    }
}



/*
 * 检查 column 名是否存在于预定的 fieldsName 中
 */
bool hasColumnNameInFieldsName(const vector<char *> &fieldsName,const char *columnName) {
//    for (auto p : fieldsName) {
    for(int len = 0; len < fieldsName.size(); len++ ){
        if (strcmp(fieldsName[len], columnName) == 0) return true;
    }
    return false;
}


bool hasColumnNameInSra(SRA_t *sra, Expression *pExpression, TableManager *tableManager, Transaction *transaction) {
    auto sraType = sra->t;
    if (sraType == SRA_SELECT) {
        return hasColumnNameInSra(sra->select.sra, pExpression, tableManager, transaction);
    }
    else if (sraType == SRA_JOIN) {
        return (hasColumnNameInSra(sra->join.sra1, pExpression, tableManager, transaction)
                | hasColumnNameInSra(sra->join.sra2, pExpression, tableManager, transaction));
    }
    else if (sraType == SRA_TABLE) {
        for (auto point = pExpression; point != nullptr; point = point->nextexpr) {
            if (point->term != nullptr && point->term->t == TERM_COLREF) {
                if (point->term->ref->tableName != nullptr) {
                    if (strcmp(point->term->ref->tableName,
                               sra->table.ref->table_name) == 0) {
                        return true;
                    }
                }
                else {
                    auto fields = tableManager->table_manager_get_tableinfo(
                            sra->table.ref->table_name, transaction);
                    if (hasColumnNameInFieldsName(fields->fieldsName,
                                                  point->term->ref->columnName)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    return false;
}

void conditionalExchange(SRA_t **pS, TableManager *tableManager, Transaction *transaction) {
    auto sra = *pS;
    if (sra == nullptr) return ;
    else if (sra->t == SRA_SELECT) {
        conditionalExchange(&(sra->select.sra), tableManager, transaction);
        auto selectSra = sra->select.sra;
        if (selectSra->t == SRA_SELECT) {
            sra->select.sra = selectSra->select.sra;
            selectSra->select.sra = sra;
            *pS = selectSra;
            conditionalExchange(&(selectSra->select.sra), tableManager, transaction);
        }
        else if (selectSra->t == SRA_JOIN) {
            auto leftBranch = hasColumnNameInSra(selectSra->join.sra1,sra->select.cond, tableManager, transaction),
                    rightBranch = hasColumnNameInSra(selectSra->join.sra2,
                                                     sra->select.cond, tableManager, transaction);
            if (leftBranch && !rightBranch) {
                sra->select.sra = selectSra->join.sra1;
                selectSra->join.sra1 = sra;
                *pS = selectSra;
                conditionalExchange(&(selectSra->join.sra1), tableManager, transaction);
            }
            else if (!leftBranch && rightBranch) {
                sra->select.sra = selectSra->join.sra2;
                selectSra->join.sra2 = sra;
                *pS = selectSra;
                conditionalExchange(&(selectSra->join.sra2), tableManager, transaction);
            }
        }
    }
    else if (sra->t == SRA_PROJECT) {
        conditionalExchange(&(sra->project.sra), tableManager, transaction);
    }
    else if (sra->t == SRA_JOIN) {
        conditionalExchange(&(sra->join.sra1), tableManager, transaction);
        conditionalExchange(&(sra->join.sra2), tableManager, transaction);
    }
}

/*输入一个关系代数表达式，输出优化后的关系代数表达式
 * 要求：在查询条件符合合取范式的前提下，根据等价变换规则将查询条件移动至合适的位置。
 * */
SRA_t *dongmengdb_algebra_optimize_condition_pushdown(SRA_t *sra, TableManager *tableManager, Transaction *transaction) {

    /*初始关系代数语法树sra由三个操作构成：SRA_PROJECT -> SRA_SELECT -> SRA_JOIN，即对应语法树中三个节点。*/

    /*第一步：.等价变换：将SRA_SELECT类型的节点进行条件串接*/

    /*1.1 在sra中找到每个SRA_Select节点 */
    /*1.2 检查每个SRA_Select节点中的条件是不是满足串接条件：多条件且用and连接*/
    /*1.3 若满足串接条件则：创建一组新的串接的SRA_Select节点，等价替换当前的SRA_Select节点*/

    /*第二步：等价变换：条件交换*/
    /*2.1 在sra中找到每个SRA_Select节点*/
    /*2.2 对每个SRA_Select节点做以下处理：
     * 在sra中查找 SRA_Select 节点应该在的最优位置：
     *     若子操作也是SRA_Select，则可交换；
     *     若子操作是笛卡尔积，则可交换，需要判断SRA_Select所包含的属性属于笛卡尔积的哪个子操作
     * 最后将SRA_Select类型的节点移动到语法树的最优位置。
     * */
    cout<<endl<<"================BEGIN================"<<endl;
    SRA_print(sra);

    //等价变换
    splitAndConcatenate(&sra);

    cout<<endl<<"================SPLIT================"<<endl;
    SRA_print(sra);

    //条件交换
    conditionalExchange(&sra,tableManager,transaction);
    cout<<endl<<"================EXCHANGE================"<<endl;
    SRA_print(sra);

    cout<<endl<<"================END================"<<endl;

    return sra;
}