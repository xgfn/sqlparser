/*!
*  \brief     	Traverse the gsp expression.
*
*  \details   	This demo is used to demonstrate how to use the visitor pattern to traverse the gsp expression,
*  			and it also demonstrates how to use the pre-order, in-order and post-order ways to traverse the
*  			gsp expression.
*  \author    	cnfree2000@hotmail.com
*  \version   	1a
*  \date      	2013
*  \pre       	need to compile with core parser and extension library.
*  \copyright 	Gudu Software
*/

/*!
**  \file expressionTraverser.c
**
*/

#include "expr_traverse.h"
#include "linked_list.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static gsp_walking_result __exprVisit(ExprTraverser *traverser, gsp_expr *pExpr, BOOL isLeafNode);

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

static FILE *infoResult;

int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser;
	ExprTraverser visitor;
	gsp_expr *pExpr;
	List *argList;
	FILE *sqlFile;
	char *sqlText = "select col1, col2,sum(col3) from table1, table2 where col4 > col5 and col6= 1000 or c1 = 1 and not sal";
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
		fprintf(stderr,"create parser error");
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
		fprintf(stderr,"parser error:%s",gsp_errmsg(parser));
		gsp_parser_free(parser);
		if(sqlFile!=NULL)
			fclose(sqlFile);
		if(infoResult!=NULL)
			fclose(infoResult);
		return 1;
	}

	visitor.exprVisit = __exprVisit;
	if(parser->pStatement[0].stmtType == sstselect && ((gsp_selectStatement *)parser->pStatement[0].stmt)->whereCondition!=NULL){
		pExpr =  ((gsp_selectStatement *)parser->pStatement[0].stmt)->whereCondition->condition;

		fprintf(stdout,"pre order\n");
		preOrderTraverse(&visitor, pExpr);

		fprintf(stdout,"\nin order\n");
		inOrderTraverse(&visitor, pExpr);

		fprintf(stdout,"\npost order\n");
		postOrderTraverse(&visitor, pExpr);
	}
	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);

	gsp_parser_free(parser);
	return 0;
}

static gsp_walking_result __exprVisit(ExprTraverser *traverser, gsp_expr *pExpr, BOOL isLeafNode){
	char *ch = " ";
	char *str = gsp_node_text((gsp_node*)pExpr);
	if (isLeafNode){
		ch = "*";
	}
	_printInfo(infoResult, "%s ", ch);
	_printInfo(infoResult, "%s\n", str);
	gsp_free(str);
	return gsp_walking_continue;
}
