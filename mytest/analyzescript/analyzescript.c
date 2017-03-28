/*!
*  \brief     Analyze sql statements.
*
*  \details   This demo is used to demonstrate how to analyze sql statements
*             and get their informations.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser and extension library.
*  \copyright Gudu Software
*/

/*!
**  \file analyzescript.c
**
*/

#include "linked_list.h"
#include "cstring.h" 
#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>

static int constCount = 0;

static FILE *infoResult;

static void _printErrorInfo(FILE *file, const char * format, ...){
	va_list argp;
	char* arg;

	va_start(argp, format);
	arg = va_arg(argp, char*);

	if(file==NULL){
		fprintf(stderr, format, arg);
	}
	else
		fprintf(file, format, arg);
	va_end(argp);
}

static void _printInfo(FILE *file, const char * format, ...){
	va_list argp;
	char* arg;

	va_start(argp, format);
	arg = va_arg(argp, char*);

	if(file==NULL){
		fprintf(stdout, format, arg);
	}
	else
		fprintf(file, format, arg);
	va_end(argp);
}

/****************************************************************/
static char* getConstTypeName(EConstantType constType)
{
    char *constTypeName = NULL;
    switch (constType)
    {
        case ect_unknown: constTypeName = "unkonw";break;
        case ect_integer: constTypeName = "integer";break;
        case ect_float: constTypeName = "float";break;
        case ect_string: constTypeName = "string";break;
        case ect_boolean: constTypeName = "boolean";break;
        case ect_null: constTypeName = "null";break;
        defalut:
        {
            constTypeName="unkonw";
            break;
        }
    }

    return constTypeName;
}


static void parse_objectOperand (gsp_objectname* objName)
{
    if (NULL == objName)
    {
        return;
    }

    if (NULL == objName->partToken)
    {
        return;
    }
   
	CString* info = CStringNew();
    
    CStringAppend(info, "\n field:");
    CStringNAppend(info, objName->partToken->pStr, objName->partToken->nStrLen);
    _printInfo(infoResult, info->buffer);


    CStringDelete(info);

    return;
}

static void parse_constant (gsp_constant* constant)
{
    if (NULL == constant)
    {
        return;
    }

    if (NULL == constant->fragment.startToken)
    {
        return;
    }
        
   
	CString* info = CStringNew();
    CStringAppend(info, "\n const:");
    CStringNAppend(info, constant->fragment.startToken->pStr, constant->fragment.startToken->nStrLen);
    CStringAppend(info, "    ");

    CStringAppend(info, "type:");
    CStringAppend(info, getConstTypeName(constant->constantType));
    CStringAppend(info, "    ");

    _printInfo(infoResult, info->buffer);

    CStringDelete(info);

    return;
}

static void parse_expr (gsp_expr* expr)
{
    if (NULL == expr)
    {
        return;
    }

   // if (eet_simple_object_name == expr->expressionType)
    if (NULL != expr->objectOperand)
    {
        parse_objectOperand(expr->objectOperand);        
    }
    //else if (eet_simple_constant == expr->expressionType)
    if (NULL != expr->constantOperand)
    {
        parse_constant (expr->constantOperand);
    }
    //else if (eet_simple_source_token == expr->expressionType)
    {
        //printf ("********notice:this is eet_simple_source_token\n");
    }
    //else if (eet_simple_comparison == expr->expressionType)
    
    {
        gsp_expr* leftOperand = expr->leftOperand;
        gsp_expr* rightOperand = expr->rightOperand;

        if (NULL != leftOperand)
        {
            parse_expr (leftOperand);
        }
        
        if (NULL != leftOperand)
        {
            parse_expr (rightOperand);
        }
        
    }

    {
        gsp_selectSqlNode *subQuery = expr->subQueryNode;
        if (NULL != subQuery)
        {
           // parse_subQuery ();
        }
    }

    return;
}

static void parse_whereCondition (gsp_whereClause *pWhereClause)
{
    if (NULL == pWhereClause)
    {
        return;
    }
    
    if (NULL == pWhereClause->condition)
    {
        return;
    }

    gsp_expr* leftOperand = pWhereClause->condition->leftOperand;
    gsp_expr* rightOperand = pWhereClause->condition->rightOperand;
    
    parse_expr (leftOperand);
    parse_expr (rightOperand);
    
    return;
}


/*****************************************************************/

static void analyzestmt( gsp_sql_statement * stmt ); 

static char* selectStmtInfo( gsp_selectStatement * pSqlstmt ) 
{
	char* result;
	CString* info = CStringNew();
	gsp_listcell *cell;

	switch (pSqlstmt->setOperator){
	case eso_none:
		CStringAppend(info, "\n Select set type: none");
		break;
	case eso_union:
		CStringAppend(info, "\n Select set type: union");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	case eso_unionall:
		CStringAppend(info, "\n Select set type: union all");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	case eso_minus:
		CStringAppend(info, "\n Select set type: minus");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	case eso_minusall:
		CStringAppend(info, "\n Select set type: minus all");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	case eso_intersect:
		CStringAppend(info, "\n Select set type: intersect");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	case eso_intersectall:
		CStringAppend(info, "\n Select set type: intersect all");
		CStringAppend(info, "\n left statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->leftStmt));
		CStringAppend(info, "\n right statement: ");
		CStringAppend(info, selectStmtInfo(pSqlstmt->rightStmt));
		break;
	}

	if (pSqlstmt->setOperator != eso_none)
	{
		if(pSqlstmt->orderbyClause!=NULL){
			CStringAppend(info, "\n order by clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->orderbyClause));
		}

		if(pSqlstmt->forupdateClause!=NULL){
			CStringAppend(info, "\n for update clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->forupdateClause));
		}
	}
	else{
		if (pSqlstmt->selectDistinct!=NULL)
		{
			CStringAppend(info, "\n select distinct: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->selectDistinct));
		}

		CStringAppend(info, "\n select clause: ");
		if(pSqlstmt->resultColumnList!=NULL){

			CStringAppend(info, "\n columns");

			foreach(cell, pSqlstmt->resultColumnList){
				gsp_resultColumn *field = (gsp_resultColumn *)gsp_list_celldata(cell);
				CStringAppend(info, "\n\tcolumn: ");
				CStringAppend(info, gsp_node_text((gsp_node*)field->expr));
				if(field->aliasClause!=NULL){
					CStringAppend(info, "\talias: ");
					CStringAppend(info, gsp_node_text((gsp_node*)field->aliasClause));
				}
			}
		}

		if(pSqlstmt->fromTableList!=NULL){
			CStringAppend(info, "\n from clause: from ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->fromTableList));
			foreach(cell, pSqlstmt->fromTableList){
				gsp_fromTable *join = (gsp_fromTable *)gsp_list_celldata(cell);
				if(join->tableName!=NULL){
					CStringAppend(info, "\n table: ");
					CStringAppend(info, gsp_node_text((gsp_node*)join->tableName));
				}
				if(join->aliasClause!=NULL){
					CStringAppend(info, "\talias: ");
					CStringAppend(info, gsp_node_text((gsp_node*)join->aliasClause));
				}
				if(join->joinExpr!=NULL){
					if(join->joinExpr->leftOperand!=NULL){
						gsp_fromTable *joinTable = join->joinExpr->leftOperand;
						if(joinTable->tableName!=NULL){
							CStringAppend(info, "\n table: ");
							CStringAppend(info, gsp_node_text((gsp_node*)joinTable->tableName));
						}
						if(joinTable->aliasClause!=NULL){
							CStringAppend(info, "\talias: ");
							CStringAppend(info, gsp_node_text((gsp_node*)joinTable->aliasClause));
						}
					}
				}
			}
		}

		if(pSqlstmt->whereCondition!=NULL){
			CStringAppend(info, "\n where clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->whereCondition));
            parse_whereCondition (pSqlstmt->whereCondition);
		}

		if(pSqlstmt->groupByClause!=NULL){
			CStringAppend(info, "\n group by clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->groupByClause));
			if(pSqlstmt->groupByClause->havingClause!=NULL){
				CStringAppend(info, "\n having clause: having ");
				CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->groupByClause->havingClause));
			}
		}

		if(pSqlstmt->orderbyClause!=NULL){
			CStringAppend(info, "\n order by clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->orderbyClause));
		}

		if(pSqlstmt->forupdateClause!=NULL){
			CStringAppend(info, "\n for update clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->forupdateClause));
		}

		if(pSqlstmt->topClause!=NULL){
			CStringAppend(info, "\n top clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->topClause));
		}

		if(pSqlstmt->limitClause!=NULL){
			CStringAppend(info, "\n limit clause: ");
			CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->limitClause));
		}
	}

	result = info->buffer;
	CStringDeleteWithoutBuffer(info);
	return result;
}

static void analyzeselect( gsp_selectStatement * psql ) 
{
	_printInfo(infoResult, "Select statement:");
	_printInfo(infoResult, selectStmtInfo(psql));
	_printInfo(infoResult, "\n");
}

static char* deleteStmtInfo( gsp_deleteStatement * pSqlstmt ) 
{
	char* result;
	CString* info = CStringNew();

	if(pSqlstmt->targetTableNode!=NULL){
		CStringAppend(info, "\n table:");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->targetTableNode));
	}

	if(pSqlstmt->whereCondition!=NULL){
		CStringAppend(info, "\n where clause: ");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->whereCondition));
        parse_whereCondition(pSqlstmt->whereCondition);
	}

	if(pSqlstmt->returningClause!=NULL){
		CStringAppend(info, "\n returning clause: ");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->returningClause));
	}

	result = info->buffer;
	CStringDeleteWithoutBuffer(info);
	return result;
}

static void analyzedelete( gsp_deleteStatement * psql ) 
{
	_printInfo(infoResult, "Delete statement:");
	_printInfo(infoResult, deleteStmtInfo(psql));
	_printInfo(infoResult, "\n");
}

static char* insertAllStmtInfo( gsp_insertStatement * pSqlstmt ) 
{
	char* result;
	CString* info = CStringNew();
	gsp_listcell *cell, *element, *element1, *element2;

	if(pSqlstmt->insertIntoValues!=NULL){
		foreach(cell, pSqlstmt->insertIntoValues){
			gsp_insertIntoValue *value = (gsp_insertIntoValue *)gsp_list_celldata(cell);
			if(value->fromTable!=NULL){
				CStringAppend(info, "\n insert into table: ");
				CStringAppend(info, gsp_node_text((gsp_node*)value->fromTable));
			}
			if(value->columnList!=NULL){
				CStringAppend(info, "\n columns: ");
				foreach(element, value->columnList){
					gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
					CStringAppend(info, "\n\t");
					CStringAppend(info, gsp_node_text((gsp_node*)column));
				}
			}
			if(value->valuesClause!=NULL && value->valuesClause->multiTargetList!=NULL){
				CStringAppend(info, "\n values: ");
				foreach(element,  value->valuesClause->multiTargetList){
					gsp_multiTarget *targetValue = (gsp_multiTarget *)gsp_list_celldata(element);
					foreach(element1, targetValue->resultColumnList){
						gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element1);
						CStringAppend(info, "\n\t");
						CStringAppend(info, gsp_node_text((gsp_node*)column));
					}
				}
			}
		}
	}

	if(pSqlstmt->insertConditions!=NULL){
		foreach(cell, pSqlstmt->insertConditions){
			gsp_insertCondition *condition = (gsp_insertCondition *)gsp_list_celldata(cell);
			if(condition->condition!=NULL){
				CStringAppend(info, "\n condition: ");
				CStringAppend(info, gsp_node_text((gsp_node*)condition->condition));
			}
			if(condition->insertIntoValues!=NULL && condition->insertIntoValues->length>0){
				foreach(element, condition->insertIntoValues){
					gsp_insertIntoValue *value = (gsp_insertIntoValue *)gsp_list_celldata(element);
					if(value->fromTable!=NULL){
						CStringAppend(info, "\n insert into table: ");
						CStringAppend(info, gsp_node_text((gsp_node*)value->fromTable));
					}
					if(value->columnList!=NULL){
						CStringAppend(info, "\n columns: ");
						foreach(element1, value->columnList){
							gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element1);
							CStringAppend(info, "\n\t");
							CStringAppend(info, gsp_node_text((gsp_node*)column));
						}
					}
					if(value->valuesClause!=NULL && value->valuesClause->multiTargetList!=NULL){
						CStringAppend(info, "\n values: ");
						foreach(element1,  value->valuesClause->multiTargetList){
							gsp_multiTarget *targetValue = (gsp_multiTarget *)gsp_list_celldata(element1);
							foreach(element2, targetValue->resultColumnList){
								gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element2);
								CStringAppend(info, "\n\t");
								CStringAppend(info, gsp_node_text((gsp_node*)column));
							}
						}
					}
				}
			}
		}
	}

	if(pSqlstmt->subQueryNode!=NULL){
		CStringAppend(info, "\n select query details: ");
		CStringAppend(info, selectStmtInfo((gsp_selectStatement*)pSqlstmt->subQueryNode));
	}

	result = info->buffer;
	CStringDeleteWithoutBuffer(info);
	return result;
}

static char* insertStmtInfo( gsp_insertStatement * pSqlstmt ) 
{
	char* result;
	CString* info = CStringNew();
	gsp_listcell *cell, *element;

	if(pSqlstmt->targetTableNode!=NULL){
		CStringAppend(info, "\n table:");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->targetTableNode));
	}

	if(pSqlstmt->columnList!=NULL && pSqlstmt->columnList->length>0){
		CStringAppend(info, "\n columns: ");
		foreach(cell, pSqlstmt->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			CStringAppend(info, gsp_node_text((gsp_node*)column));
		}
	}

	if(pSqlstmt->multiTargetList!=NULL && pSqlstmt->multiTargetList->length>0){
		CStringAppend(info, "\n values: ");
		foreach(cell, pSqlstmt->multiTargetList){
			gsp_multiTarget *targetValue = (gsp_multiTarget *)gsp_list_celldata(cell);
			if(targetValue->resultColumnList!=NULL){
				foreach(element, targetValue->resultColumnList){
					gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
					CStringAppend(info, "\n\t");
					CStringAppend(info, gsp_node_text((gsp_node*)column));
				}
			}
		}
	}

	if(pSqlstmt->returningClause!=NULL){
		CStringAppend(info, "\n returning clause: ");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->returningClause));
	}

	result = info->buffer;
	CStringDeleteWithoutBuffer(info);
	return result;
}

static void analyzeinsert( gsp_insertStatement * psql ) 
{
	_printInfo(infoResult, "insert statement: ");
	if (psql->insertIntoValues != NULL && psql->insertIntoValues->length > 0)
	{
		_printInfo(infoResult, insertAllStmtInfo(psql));
	}
	else if (psql->insertConditions != NULL && psql->insertConditions->length > 0)
	{
		_printInfo(infoResult, insertAllStmtInfo(psql));
	}
	else
	{
		_printInfo(infoResult, insertStmtInfo(psql));
	}
	_printInfo(infoResult, "\n");
}

static char* updateStmtInfo( gsp_updateStatement * pSqlstmt ) 
{
	char* result;
	CString* info = CStringNew();
	gsp_listcell *cell;

	if(pSqlstmt->targetTableNode!=NULL){
		CStringAppend(info, "\n table:");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->targetTableNode));
	}

	if(pSqlstmt->resultColumnList!=NULL && pSqlstmt->resultColumnList->length>0){
		CStringAppend(info, "\n set clause:");
		foreach(cell, pSqlstmt->resultColumnList){
			gsp_resultColumn *field = (gsp_resultColumn *)gsp_list_celldata(cell);
			if(field->expr!=NULL && field->expr->leftOperand!=NULL && field->expr->rightOperand!=NULL ){
				CStringAppend(info, "\n\tcolumn: ");
				CStringAppend(info, gsp_node_text((gsp_node*)field->expr->leftOperand));
				CStringAppend(info, "\tvalue: ");
				CStringAppend(info, gsp_node_text((gsp_node*)field->expr->rightOperand));
                CStringAppend(info, "\ttype: ");
                if (eet_simple_constant == field->expr->rightOperand->expressionType)
                {
                    if (NULL != field->expr->rightOperand->constantOperand)
                    {
                        CStringAppend(info, getConstTypeName (field->expr->rightOperand->constantOperand->constantType));
                    }
                }
                else
                {
                    CStringAppend(info, "unkonw");
                }
			}
		}
	}

	if(pSqlstmt->whereCondition!=NULL){
		CStringAppend(info, "\n where clause: ");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->whereCondition));
	}

	if(pSqlstmt->returningClause!=NULL){
		CStringAppend(info, "\n returning clause: ");
		CStringAppend(info, gsp_node_text((gsp_node*)pSqlstmt->returningClause));
	}

	result = info->buffer;
	CStringDeleteWithoutBuffer(info);
	return result;
}

static void analyzeupdate( gsp_updateStatement * psql ) 
{
	_printInfo(infoResult, "Update statement:");
	_printInfo(infoResult, updateStmtInfo(psql));
	_printInfo(infoResult, "\n");
}

static void analyzeColumn( gsp_objectname * column ) 
{
	if(column == NULL)
		return;
	_printInfo(infoResult, "column name:");
	_printInfo(infoResult, gsp_node_text((gsp_node*)column));
	_printInfo(infoResult, "\n");
}

static void analyzeColumnDefinition( gsp_columnDefinition * column ) 
{
	if(column == NULL)
		return;
	if(column->columnName!=NULL){
		_printInfo(infoResult, "column name:");
		_printInfo(infoResult, gsp_node_text((gsp_node*)column->columnName));
		_printInfo(infoResult, "\n");
	}
	if(column->datatype!=NULL){
		_printInfo(infoResult, "column type:");
		_printInfo(infoResult, gsp_node_text((gsp_node*)column->datatype));
		_printInfo(infoResult, "\n");
	}
}

static void analyzealtertable( gsp_alterTableStatement * psql ) 
{
	gsp_listcell *cell, *element;
	_printInfo(infoResult, "Alter table: ");
	_printInfo(infoResult, gsp_node_text((gsp_node*)psql->tableName));
	_printInfo(infoResult, "\n");

	if(psql->alterTableOptionList!=NULL && psql->alterTableOptionList->length>0){
		foreach(cell, psql->alterTableOptionList){
			gsp_alterTableOption *option = (gsp_alterTableOption *)gsp_list_celldata(cell);
			switch(option->optionType){
			case eat_AddColumn:
				_printInfo(infoResult, "add columns\n");
				foreach (element, option->columnDefinitionList)
				{
					gsp_columnDefinition *column = (gsp_columnDefinition *)gsp_list_celldata(cell);
					analyzeColumnDefinition(column);
				}
				break; 
			case eat_ModifyColumn:
				_printInfo(infoResult, "modify columns\n");
				foreach (element, option->columnDefinitionList)
				{
					gsp_columnDefinition *column = (gsp_columnDefinition *)gsp_list_celldata(cell);
					analyzeColumnDefinition(column);
				}
				break;
			case eat_AlterColumn:
				_printInfo(infoResult, "alter column\n");
				analyzeColumn(option->columnName);
				break;
			case eat_DropColumn:
				_printInfo(infoResult, "drop columns\n");
				foreach (element, option->columnNameList)
				{
					gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
					analyzeColumn(column);
				}
				break;
			case eat_SetUnUsedColumn:
				_printInfo(infoResult, "set unused columns\n");
				foreach (element, option->columnNameList)
				{
					gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
					analyzeColumn(column);
				}
				break;
			case eat_DropUnUsedColumn:
				_printInfo(infoResult, "drop unused columns\n");
				break;
			case eat_RenameColumn:
				_printInfo(infoResult, "rename column from ");
				_printInfo(infoResult, gsp_node_text((gsp_node*)option->columnName));
				_printInfo(infoResult, " to ");
				_printInfo(infoResult, gsp_node_text((gsp_node*)option->newColumnName));
				_printInfo(infoResult, "\n");
				break;
			case eat_ChangeColumn:
				_printInfo(infoResult, "change column ");
				_printInfo(infoResult, gsp_node_text((gsp_node*)option->columnName));
				_printInfo(infoResult, "\n");
				_printInfo(infoResult, "new column definition: ");
				analyzeColumnDefinition((gsp_columnDefinition*)gsp_list_first(option->columnDefinitionList));
				_printInfo(infoResult, "\n");
				break;
			default:
				_printInfo(infoResult, "alter table option not handled yet\n");
				break;
			}
		}
	}
}

static void analyzecreatetable( gsp_createTableStatement * psql ) 
{
	gsp_listcell *cell;

	_printInfo(infoResult, "Create table, table name: ");
	_printInfo(infoResult, gsp_node_text((gsp_node*)psql->table));
	_printInfo(infoResult, "\n");

	if(psql->columnList!=NULL && psql->columnList->length>0){
		foreach(cell, psql->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			_printInfo(infoResult, "\n column name: ");
			_printInfo(infoResult, gsp_node_text((gsp_node*)column));
		}
	}
}

static void analyzeExec( gsp_mssql_executeSqlNode * psql ) 
{
	gsp_listcell *cell;

	if(psql->moduleName!=NULL){
		_printInfo(infoResult, "procedure name: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->moduleName));
		_printInfo(infoResult, "\n");
	}

	_printInfo(infoResult, "exec type: ");

	switch(psql->execType){
	case eet_exec_noExecKeyword:
		_printInfo(infoResult, "eet_exec_noExecKeyword");
		break;
	case eet_exec_sp:
		_printInfo(infoResult, "eet_exec_sp");
		break;
	case eet_exec_stringCmd:
		_printInfo(infoResult, "eet_exec_stringCmd");
		break;
	case eet_exec_stringCmdLinkServer:
		_printInfo(infoResult, "eet_exec_stringCmdLinkServer");
		break;
	}
	_printInfo(infoResult, "\n");

	if(psql->parameterList!=NULL && psql->parameterList->length>0){
		int i=0;
		_printInfo(infoResult, "parameters: ");
		_printInfo(infoResult, "%d", psql->parameterList->length);
		_printInfo(infoResult, "\n");

		foreach(cell, psql->parameterList){
			gsp_mssql_execParameter *parameter = (gsp_mssql_execParameter *)gsp_list_celldata(cell);
			_printInfo(infoResult, "param %d : ", i++);
			_printInfo(infoResult, gsp_node_text((gsp_node*)parameter->parameterValue));
			_printInfo(infoResult, "\n");
		}
	}


}

static void analyzeCreateView( gsp_createViewStatement * psql ) 
{
	gsp_listcell *cell;

	if(psql->viewName!=NULL){
		_printInfo(infoResult, "create view statement, view name: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->viewName));
		_printInfo(infoResult, "\n");
	}

	if(psql->viewAliasClause!=NULL && psql->viewAliasClause->viewAliasItemList!=NULL){
		foreach(cell, psql->viewAliasClause->viewAliasItemList){
			gsp_viewAliasItem* vai = (gsp_viewAliasItem *)gsp_list_celldata(cell);
			if(vai->alias!=NULL){
				_printInfo(infoResult, "view alias: ");
				_printInfo(infoResult, gsp_node_text((gsp_node*)vai->alias));
				_printInfo(infoResult, "\n");
			}
		}
	}

	if(psql->selectSqlNode!=NULL){
		_printInfo(infoResult, "subquery: ");
		analyzeselect((gsp_selectStatement*)psql->selectSqlNode);
	}
}

static void analyzePlsql_CreateProcedure( gsp_createProcedureStatement * psql ) 
{
	_printInfo(infoResult, "create procedure statement: ");
	_printInfo(infoResult, "\n");

	if(psql->procedureName!=NULL){
		_printInfo(infoResult, "procedure name: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->procedureName));
		_printInfo(infoResult, "\n");
	}
}

static void analyzePlsql_Block( gsp_blockStatement * psql ) 
{
	gsp_listcell *cell;

	if(psql->declareStmts!=NULL && psql->declareStmts->length>0){
		_printInfo(infoResult, "declare section: ");
		_printInfo(infoResult, "\n");

		foreach(cell, psql->declareStmts){
			gsp_sql_statement* stmt = (gsp_sql_statement *)gsp_list_celldata(cell);
			analyzestmt(stmt);
		}
	}
}

static void analyzePlsql_IfStmt( gsp_plsqlIfStmt * psql ) 
{
	gsp_listcell *cell;

	if(psql->condition!=NULL){
		_printInfo(infoResult, "if condition: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->condition));
		_printInfo(infoResult, "\n");
	}

	if(psql->thenStmts!=NULL && psql->thenStmts->length>0){
		foreach(cell, psql->thenStmts){
			gsp_sql_statement* stmt = (gsp_sql_statement *)gsp_list_celldata(cell);
			analyzestmt(stmt);
		}
	}
}

static void analyzePlsql_AssignStmt( gsp_plsqlAssignStmt * psql ) 
{
	if(psql->left!=NULL && psql->right!=NULL){
		_printInfo(infoResult, "plsql assignment: left: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->left));
		_printInfo(infoResult, ", right: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->right));
		_printInfo(infoResult, "\n");
	}
}

static void analyzePlsql_ForallStmt( gsp_plsqlForallStmt * psql ) 
{
	if(psql->stmt!=NULL){
		_printInfo(infoResult, "plsql for statement: ");
		_printInfo(infoResult, "\n");
		analyzestmt(psql->stmt);
		_printInfo(infoResult, "\n");
	}
}

static void analyzePlsql_ProcBasicStmt( gsp_plsqlBasicStmt * psql ) 
{
	if(psql->expr!=NULL){
		_printInfo(infoResult, "basic statement: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->expr));
		_printInfo(infoResult, "\n");
	}
}

static void analyzePlsql_CreateTrigger( gsp_createTriggerStatement * psql ) 
{
	if(psql->triggerName!=NULL){
		_printInfo(infoResult, "plsql trigger name: ");
		_printInfo(infoResult, gsp_node_text((gsp_node*)psql->triggerName));
		_printInfo(infoResult, "\n");
	}
	if(psql->stmt!=NULL){
		analyzestmt(psql->stmt);
	}
}

static void analyzestmt( gsp_sql_statement * stmt ) 
{
	if(stmt == NULL || (stmt->stmt == NULL && stmt->parseTree == NULL))
		return;
	else{
		gsp_node* node = (gsp_node*)stmt->stmt;
		if(node == NULL){
			node = stmt->parseTree;
		}

		switch(stmt->stmtType){
		case sstselect:
			analyzeselect((gsp_selectStatement *)node);
			break;
		case sstdelete:
			analyzedelete((gsp_deleteStatement *)node);
			break;
		case sstinsert:
			analyzeinsert((gsp_insertStatement *)node);
			break;
		case sstupdate:
			analyzeupdate((gsp_updateStatement *)node);
			break;
		case sstaltertable:
			analyzealtertable((gsp_alterTableStatement *)node);
			break;
		case sstcreatetable:
			analyzecreatetable((gsp_createTableStatement *)node);
			break;
		case sstmssqlexec:
			analyzeExec((gsp_mssql_executeSqlNode *)node);
			break;
		case sstcreateview:
			analyzeCreateView((gsp_createViewStatement *)node);
			break;
		case sstplsql_createprocedure:
			analyzePlsql_CreateProcedure((gsp_createProcedureStatement *)node);
			break;
		case sstplsql_block:
			analyzePlsql_Block((gsp_blockStatement *)node);
			break;
		case sstplsql_ifstmt:
			analyzePlsql_IfStmt((gsp_plsqlIfStmt *)node);
			break;
		case sstplsql_assignstmt:
			analyzePlsql_AssignStmt((gsp_plsqlAssignStmt *)node);
			break;
		case sstplsql_forallstmt:
			analyzePlsql_ForallStmt((gsp_plsqlForallStmt *)node);
			break;
		case sstplsql_procbasicstmt:
			analyzePlsql_ProcBasicStmt((gsp_plsqlBasicStmt *)node);
			break;
		case sstplsql_createtrigger:
			analyzePlsql_CreateTrigger((gsp_createTriggerStatement *)node);
			break;
		case sstplsql_proceduredecl:
			analyzePlsql_CreateProcedure((gsp_createProcedureStatement *)node);
			break;
		default:
			_printInfo(infoResult, "This type of sql statement not analyzed in this demo, contact info@sqlparser.com for an improved demo. \n %s \n", gsp_node_text(node));
			break;
		}
	}
}

int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser = NULL;
	List *argList = NULL;
	FILE *sqlFile = NULL;
	FILE *infoResult = NULL;
	gsp_dbvendor vendor = dbvmysql;
	int i;

#if 0
	char *sqlText = "SELECT ProductID, ListPrice \nFROM Product, Product1 \nleft join Product1 t2 on t1.f1 = t2.f2 \nWHERE ListPrice!=1 AND ListPrice!=3 \nGROUP BY ListPrice \nHAVING SUM(ListPrice)>1000 \nORDER BY ListPrice ASC";

	char *sqlText = "select last_name,job_id,salary from employees"
			" where job_id=(select job_id from employees where employee_id=141)"
			" and salary > (select salary from employees where employeename='gjc');";
#endif
	char *sqlText = argv[1];


#if 0
	argList = createStringList(FALSE);

	for(index=0;index<argc;index++){
		argList->add(argList, argv[index]);
	}

	argIndex = argList->indexOf(argList, "-o");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		infoResult=fopen(argv[argIndex+1],"w");
	}
	else infoResult = NULL;

	argIndex = argList->indexOf(argList, "-d");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		if(strcmp("mssql",argv[argIndex+1])==0){
			vendor = dbvmssql;
		}
		else if(strcmp("db2",argv[argIndex+1])==0){
			vendor = dbvdb2;
		}	
		else if(strcmp("mysql",argv[argIndex+1])==0){
			vendor = dbvmysql;
		}	
		else if(strcmp("postgresql",argv[argIndex+1])==0){
			vendor = dbvpostgresql;
		}
		else if(strcmp("informix",argv[argIndex+1])==0){
			vendor = dbvinformix;
		}
		else if(strcmp("sybase",argv[argIndex+1])==0){
			vendor = dbvsybase;
		}
		else if(strcmp("teradata",argv[argIndex+1])==0){
			vendor = dbvteradata;
		}
	}

	argIndex = argList->indexOf(argList, "-f");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		sqlFile=fopen(argv[argIndex+1],"r");
		if(sqlFile == NULL){
			fprintf(stderr,"script file %s doesn't exist!\n", argv[1] );
			if(infoResult!=NULL)
				fclose(infoResult);
			return 1;
		}
	}
	else sqlFile = NULL;

	argList->dispose(argList);
#endif

	rc = gsp_parser_create(vendor,&parser);
	if (rc){
		fprintf(stderr,"create parser error");
		return 1;
	}

	if(sqlFile!=NULL){
		rc= gsp_check_syntax_file(parser, sqlFile);
	}
	else{
		_printInfo(infoResult, "Origin SQL:\n");
		_printInfo(infoResult, sqlText);
		_printInfo(infoResult, "\n\n");
		rc= gsp_check_syntax(parser, sqlText);
	}
	if (rc != 0){
		fprintf(stderr,"parser error:%s",gsp_errmsg(parser));
		return 1;
	}

	for(i=0;i<parser->nStatement;i++){
		gsp_sql_statement *stmt = &parser->pStatement[i];
		analyzestmt(stmt);
		if(i<parser->nStatement-1){
			_printInfo(infoResult, "\n\n");
		}
	}

	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);

	gsp_parser_free(parser);
	return 0;
}

