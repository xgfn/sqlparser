/*!
*  \brief     	Iterate the statements in the sql script.
*
*  \details   	This demo is used to demonstrate how to iterate the statements in the sql script and get
*  			the statement detail informations, such as statement type, clauses, columns and tables.
*  \author    	cnfree2000@hotmail.com
*  \version   	1a
*  \date      	2013
*  \pre       	need to compile with core parser and extension library.
*  \copyright 	Gudu Software
*/

/*!
**  \file iterateStatement.c
**
*/

#include "node_visitor.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FILE *infoResult;

static void _printInfo(FILE *file, const char * format, ...);
static void _printErrorInfo(FILE *file, const char * format, ...);

static gsp_objectname* _getTableName(gsp_node* table){
	if(table->nodeType == t_gsp_table){
		gsp_table *normalTable = (gsp_table*)table;
		return normalTable->tableName;
	}
	else if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return fromTable->tableName;
	}
	return NULL;
}

static ETableSource _getTableSource(gsp_node* table){
	if(table->nodeType == t_gsp_table){
		gsp_table *normalTable = (gsp_table*)table;
		return normalTable->tableSource;
	}
	else if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return fromTable->fromtableType;
	}
	return ets_unknown;
}

static gsp_aliasClause* _getTableAlias(gsp_node* table){
	if(table->nodeType == t_gsp_table){
		gsp_table *normalTable = (gsp_table*)table;
		return normalTable->aliasClause;
	}
	else if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return fromTable->aliasClause;
	}
	return NULL;
}

static void _process_select(gsp_node *node, struct gsp_visitor *visitor){
	gsp_base_statement *stmt = (gsp_base_statement*)node;
	SqlTraverser* traverser = (SqlTraverser*)visitor->context;
	Map* infoMap = traverser->getSQLInfo(traverser, stmt);

	if(infoMap == NULL)
		return;
	else{
		List *columns = (List*)infoMap->get(infoMap, (void *)sft_resultColumn);
		List *tables = (List*)infoMap->get(infoMap, (void *)sft_table);
		List *wheres = (List*)infoMap->get(infoMap, (void *)sft_whereClause);
		List *groupBys = (List*)infoMap->get(infoMap, (void *)sft_groupByClause);
		List *orderBys = (List*)infoMap->get(infoMap, (void *)sft_orderByClause);

		int i = 0;

		if(columns!=NULL && !columns->isEmpty(columns)){
			_printInfo(infoResult, "Columns:\n");
			for(i = 0; i<columns->size(columns); i++){
				char* nodeText = gsp_node_text((gsp_node *)columns->get(columns, i));
				_printInfo(infoResult, "%d: ", i);
				_printInfo(infoResult, "%s\n", nodeText);
				gsp_free(nodeText);
			}
		}

		if(tables!=NULL && !tables->isEmpty(tables)){
			_printInfo(infoResult, "\n");
			for(i = 0; i<tables->size(tables); i++){
				gsp_node *table = (gsp_node *)tables->get(tables, i);
				if(_getTableSource(table) == ets_objectname){
					char* nodeText = gsp_node_text((gsp_node *)table);
					_printInfo(infoResult, "table name: %s\n", nodeText);
					gsp_free(nodeText);
				}
				else{
					if(_getTableSource(table) == ets_subquery){
						_printInfo(infoResult, "table source: %s", "ets_subquery");
					}
					else if(_getTableSource(table) == ets_join){
						_printInfo(infoResult, "table source: %s", "ets_join");
					}
					else if(_getTableSource(table) == ets_tableExpr){
						_printInfo(infoResult, "table source: %s", "ets_tableExpr");
					}
					if(_getTableAlias(table)!=NULL && _getTableAlias(table)->aliasName!=NULL){
						char* nodeText = gsp_node_text((gsp_node *)_getTableAlias(table)->aliasName);
						_printInfo(infoResult, " %s", nodeText);
						gsp_free(nodeText);
					}
					_printInfo(infoResult, "\n");
				}
			}
		}

		if(wheres!=NULL && !wheres->isEmpty(wheres)){
			_printInfo(infoResult, "\n");
			for(i = 0; i<wheres->size(wheres); i++){
				gsp_whereClause *whereClause = (gsp_whereClause *)wheres->get(wheres, i);
				char* nodeText = gsp_node_text((gsp_node *)whereClause);
				_printInfo(infoResult, "where clause:\n");
				_printInfo(infoResult, "%s\n", nodeText);
				gsp_free(nodeText);
			}
		}

		if(groupBys!=NULL && !groupBys->isEmpty(groupBys)){
			_printInfo(infoResult, "\n");
			for(i = 0; i<groupBys->size(groupBys); i++){
				gsp_groupBy *groupByClause = (gsp_groupBy *)groupBys->get(groupBys, i);
				char* nodeText = gsp_node_text((gsp_node *)groupByClause);
				_printInfo(infoResult, "group by clause:\n");
				_printInfo(infoResult, "%s\n", nodeText);
				gsp_free(nodeText);
				if(groupByClause->havingClause!=NULL){
					nodeText = gsp_node_text((gsp_node *)groupByClause->havingClause);
					_printInfo(infoResult, "having clause:\n");
					_printInfo(infoResult, "%s\n", nodeText);
					gsp_free(nodeText);
				}
			}
		}

		if(orderBys!=NULL && !orderBys->isEmpty(orderBys)){
			_printInfo(infoResult, "\n");
			for(i = 0; i<orderBys->size(orderBys); i++){
				gsp_orderBy *orderByClause = (gsp_orderBy *)orderBys->get(orderBys, i);
				char* nodeText = gsp_node_text((gsp_node *)orderByClause);
				_printInfo(infoResult, "order by clause:\n");
				_printInfo(infoResult, "%s\n", nodeText);
				gsp_free(nodeText);
			}
		}
	}
}

static struct gsp_visitor* _createVisitor(SqlTraverser* traverser){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = traverser;
	visitor->handle_node[t_gsp_selectStatement] = _process_select;
	return visitor;
}

static void _disposeVisitor(struct gsp_visitor* visitor){
	free(visitor);
}

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

int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser;
	List *nodeList, *argList;
	Iterator iter;
	struct gsp_visitor *visitor;
	SqlTraverser* traverser;
	FILE *sqlFile;
	char *sqlText = "select employee_id,job_id\nfrom employees\nunion\nselect employee_id,job_id\nfrom job_history;";
	gsp_dbvendor vendor = dbvoracle;

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

	rc = gsp_parser_create(vendor,&parser);
	if (rc){
		_printErrorInfo(infoResult, "create parser error");
		fclose(sqlFile);
		if(sqlFile!=NULL)
			fclose(sqlFile);
		if(infoResult!=NULL)
			fclose(infoResult);
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
		_printErrorInfo(infoResult, "parser error:%s\n",gsp_errmsg(parser));
		gsp_parser_free(parser);
		if(sqlFile!=NULL)
			fclose(sqlFile);
		if(infoResult!=NULL)
			fclose(infoResult);
		return 1;
	}

	traverser = createSqlTraverser();
	nodeList = traverser->traverseSQL(traverser, &parser->pStatement[0]);
	visitor = _createVisitor(traverser);

	iter = nodeList->getIterator(nodeList);
	while(nodeList->hasNext(nodeList,&iter))
	{
		gsp_node *node = (gsp_node*)nodeList->next(&iter);
		if(node->nodeType == t_gsp_deleteStatement)
			_printInfo(infoResult, "\nstmt type = %s\n\n", "t_gsp_deleteStatement");
		if(node->nodeType == t_gsp_updateStatement)
			_printInfo(infoResult, "\nstmt type = %s\n\n", "t_gsp_updateStatement");
		if(node->nodeType == t_gsp_insertStatement)
			_printInfo(infoResult, "\nstmt type = %s\n\n", "t_gsp_insertStatement");
		if(node->nodeType == t_gsp_selectStatement){
			_printInfo(infoResult, "\nstmt type = %s\n\n", "t_gsp_selectStatement");
			visitor->handle_node[t_gsp_selectStatement](node, visitor);
		}
	}

	traverser->dispose(traverser);
	_disposeVisitor(visitor);
	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);
	gsp_parser_free(parser);
	return 0;
}
