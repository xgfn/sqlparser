/*!
*  \brief     Decode the merge statement.
*
*  \details   This demo is used to demonstrate how to decode the merge statement
*             and output the sub statements.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser library.
*  \copyright Gudu Software
*/

/*!
**  \file mergeSqlDecode.c
**
*/

#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include "linked_list.h"

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
	List *argList;
	FILE *sqlFile;
	FILE *infoResult;
	gsp_dbvendor vendor = dbvoracle;
	char *sqlText = "MERGE INTO t_target A using\n"
		"(select key1, key2, value1, value2 from t_base where key1='aaa') B\n"
		"ON\n"
		"(A.key1=B.key1 and A.key2=B.key2)\n"
		"WHEN MATCHED THEN\n"
		"UPDATE SET A.value1=B.value1, A.value2=B.value2\n"
		"WHEN NOT MATCHED THEN\n"
		"INSERT(A.key1, A.key2, A.value1, A.value2) values(B.key1, B.key2, B.value1, B.value2);\n";


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


	printf("The result of merge statement decoding:\n");

	if(parser->pStatement[0].stmtType == sstmerge){
		gsp_mergeStatement* merge = (gsp_mergeStatement*)parser->pStatement[0].stmt;
		if(merge->usingTableNode !=NULL && merge->usingTableNode->subQueryNode!=NULL){
			char* str = gsp_node_text((gsp_node*)merge->usingTableNode->subQueryNode);
			printf("%s\n", str);
			gsp_free(str);
		}
		if(merge->whenClauses!=NULL){
			gsp_listcell *cell;
			foreach(cell, merge->whenClauses){
				gsp_mergeWhenClause  *item = (gsp_mergeWhenClause *)gsp_list_celldata(cell);
				gsp_node *node;
				if(item->insertClause!=NULL){
					gsp_insertStatement *stmt = (gsp_insertStatement *)item->insertClause;
					node = (gsp_node*)item->insertClause;
				}
				else if(item->updateClause!=NULL){
					node = (gsp_node*)item->updateClause;
				}
				else if(item->deleteClause!=NULL){
					node = (gsp_node*)item->deleteClause;
				}
				else{
					node = NULL;
				}
				if(node!=NULL){
					char* str = gsp_node_text(node);
					_printInfo(infoResult, "%s\n", str);
					gsp_free(str);
				}
			}
		}
	}

	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);

	gsp_parser_free(parser);
	return 0;
}
