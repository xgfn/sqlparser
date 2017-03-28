/*!
*  \brief     Collect the sql tables and table columns.
*
*  \details   This demo is used to demonstrate how to get the sql tables
*             and table columns.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser and extension library.
*  \copyright Gudu Software
*/

/*!
**  \file gettablecolumns.c
**
*/

#include "node_visitor.h"
#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static List* tableInfoList;
static List* fieldInfoList;

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

static void _process_table(gsp_node *node, struct gsp_visitor *visitor){
	char *str;
	int index;
	SqlTraverser *traverser = (SqlTraverser *)visitor->context;
	if(traverser->isBaseTable(traverser, (gsp_node*)node)){
		str = gsp_node_text((gsp_node *)_getTableName(node));
		if(!tableInfoList->contains(tableInfoList, str)){
			tableInfoList->add(tableInfoList, str);
		}
		{
			List *fields = (List *)traverser->getTableObjectNameReferences(traverser, node);
			if(fields!=NULL){
				Iterator fieldIter = fields->getIterator(fields);
				while(fields->hasNext(fields, &fieldIter)){
					gsp_objectname *field = (gsp_objectname *)fields->next(&fieldIter);
					char* fieldName = (char *)malloc((strlen(str) + field->partToken->nStrLen + 2 + 24)*sizeof(char));
					memset(fieldName,'\0', (strlen(str) + field->partToken->nStrLen + 2 + 24)*sizeof(char));
					strcat(fieldName,str);
					strcat(fieldName,".");
					strncat(fieldName,field->partToken->pStr, field->partToken->nStrLen);
					strcat(fieldName,"(table determined:");
					strcat(fieldName,traverser->isTableDetermined(traverser, field)?"true":"false");
					strcat(fieldName,")");
					if(!fieldInfoList->contains(fieldInfoList, fieldName))
						fieldInfoList->add(fieldInfoList, fieldName);
				}
			}
		}
	}
}

static struct gsp_visitor* _createVisitor(SqlTraverser *traverser){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = traverser;
	visitor->handle_node[t_gsp_table] = _process_table;
	visitor->handle_node[t_gsp_fromTable] = _process_table;
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

static void _printListInfo(List *list, FILE *infoResult ){
	Iterator infoIter = list->getIterator(list);
	while(fieldInfoList->hasNext(list,&infoIter))
	{
		char *str = (char *)list->next(&infoIter);
		_printInfo(infoResult,"%s\n", str);
		gsp_free(str);
	}
	_printInfo(infoResult,"\n");
}

int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser;
	List *nodeList, *argList;
	Iterator iter;
	struct gsp_visitor *visitor;
	SqlTraverser* traverser;
	FILE *sqlFile= NULL, *infoResult= NULL;
//	char *sqlText = "select employee_id,job_id\nfrom employees\nunion\nselect employee_id,job_id\nfrom job_history;";
	//gsp_dbvendor vendor = dbvoracle;

	//char *sqlText = "select revision from mailstore for update";
	char *sqlText = "INSERT into runoob_tbl (runoob_title, runoob_author, submission_date) VALUES (\"Learn Linux\", \"Da Fei\", Now())";

	gsp_dbvendor vendor = dbvpostgresql;//dbvoracle;
 
	//argList = createStringList(FALSE);

	//for(index=0;index<argc;index++){
	//	argList->add(argList, argv[index]);
	//}

	//argIndex = argList->indexOf(argList, "-o");
	//if(argIndex!=-1 && argList->size(argList) >argIndex+1){
	//	infoResult=fopen(argv[argIndex+1],"w");
	//}
	//else infoResult = NULL;

	//argIndex = argList->indexOf(argList, "-d");
	//if(argIndex!=-1 && argList->size(argList) >argIndex+1){
	//	if(strcmp("mssql",argv[argIndex+1])==0){
	//		vendor = dbvmssql;
	//	}
	//	else if(strcmp("db2",argv[argIndex+1])==0){
	//		vendor = dbvdb2;
	//	}	
	//	else if(strcmp("mysql",argv[argIndex+1])==0){
	//		vendor = dbvmysql;
	//	}	
	//	else if(strcmp("postgresql",argv[argIndex+1])==0){
	//		vendor = dbvpostgresql;
	//	}
	//	else if(strcmp("informix",argv[argIndex+1])==0){
	//		vendor = dbvinformix;
	//	}
	//	else if(strcmp("sybase",argv[argIndex+1])==0){
	//		vendor = dbvsybase;
	//	}
	//	else if(strcmp("teradata",argv[argIndex+1])==0){
	//		vendor = dbvteradata;
	//	}
	//}

	//argIndex = argList->indexOf(argList, "-f");
	//if(argIndex!=-1 && argList->size(argList) >argIndex+1){
	//	sqlFile=fopen(argv[argIndex+1],"r");
	//	if(sqlFile == NULL){
	//		fprintf(stderr,"script file %s doesn't exist!\n", argv[1] );
	//		if(infoResult!=NULL)
	//			fclose(infoResult);
	//		return 1;
	//	}
	//}
	//else sqlFile = NULL;

	//argList->dispose(argList);

	rc = gsp_parser_create(vendor,&parser);
	if (rc){
		_printErrorInfo(infoResult, "create parser error");
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
		fclose(sqlFile);
		if(infoResult!=NULL)
			fclose(infoResult);
		return 1;
	}

	tableInfoList = createStringList(TRUE);
	fieldInfoList = createStringList(TRUE);
	
	traverser = createSqlTraverser();
	visitor = _createVisitor(traverser);

	for(index=0;index<parser->nStatement;index++ ){
		nodeList = traverser->traverseSQL(traverser, &parser->pStatement[index]);
		iter = nodeList->getIterator(nodeList);
		while(nodeList->hasNext(nodeList,&iter))
		{
			gsp_node *node = (gsp_node*)nodeList->next(&iter);
			if(node->nodeType == t_gsp_table )
				visitor->handle_node[t_gsp_table](node, visitor);
			else if(node->nodeType == t_gsp_fromTable )
				visitor->handle_node[t_gsp_fromTable](node, visitor);
		}
	}

	if(!tableInfoList->isEmpty(tableInfoList)){
		tableInfoList->sort(tableInfoList);
		_printInfo(infoResult, "Tables:\n");
		_printListInfo(tableInfoList, infoResult);
	}

	if(!fieldInfoList->isEmpty(fieldInfoList)){
		fieldInfoList->sort(fieldInfoList);
		_printInfo(infoResult,"Columns:\n");
		_printListInfo(fieldInfoList, infoResult);
	}

	tableInfoList->dispose(tableInfoList);
	fieldInfoList->dispose(fieldInfoList);
	
	traverser->dispose(traverser);
	_disposeVisitor(visitor);
	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);
	gsp_parser_free(parser);
	return 0;
}
