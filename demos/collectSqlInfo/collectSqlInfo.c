/*!
*  \brief     Collection the sql information.
*
*  \details   This demo is used to demonstrate how to get the sql detail information
*             f from a sql script, such as table names, table columns, database schemas and built-in functions.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser and extension library.
*  \copyright Gudu Software
*/

/*!
**  \file collectSqlInfo.c
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
static List* schemaInfoList;
static List* functionInfoList;

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
		index = (strrchr(str, '.') - str)/sizeof(char);
		if(index>0){
			char* schema = (char*)malloc( (index + 1)*sizeof(char));
			memset(schema,'\0', (index + 1)*sizeof(char));
			strncat(schema, str, index);
			if(!schemaInfoList->contains(schemaInfoList, schema))
				schemaInfoList->add(schemaInfoList, schema);
			else free(schema);
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
					else gsp_free(fieldName);
				}
			}
		}
		if(!tableInfoList->contains(tableInfoList, str)){
			tableInfoList->add(tableInfoList, str);
		}
		else gsp_free(str);
	}
}

static void _process_func(gsp_node *node, struct gsp_visitor *visitor){
	char *str = gsp_node_text((gsp_node *)((gsp_functionCall *)node)->functionName);
	if(string_comparer_ignoreCase(str, "SYSDATE") == 0){
		gsp_free(str);
		return;
	}
	else{
		int index = (strrchr(str, '.') - str)/sizeof(char);
		if(index>0){
			char* schema = (char*)malloc( (index + 1)*sizeof(char));
			memset(schema,'\0', (index + 1)*sizeof(char));
			strncat(schema, str, index);
			if(!schemaInfoList->contains(schemaInfoList, schema))
				schemaInfoList->add(schemaInfoList, schema);
			else free(schema);
		}

		if(!functionInfoList->contains(functionInfoList, str))
			functionInfoList->add(functionInfoList, str);
		else gsp_free(str);
	}
}

static struct gsp_visitor* _createVisitor(SqlTraverser *traverser){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = traverser;
	visitor->handle_node[t_gsp_functionCall] = _process_func;
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

//collectsqlinfo_main
int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser;
	List *nodeList, *argList;
	Iterator iter;
	struct gsp_visitor *visitor;
	SqlTraverser* traverser;
	FILE *sqlFile, *infoResult;
//	char *sqlText = "select employee_id,job_id\nfrom employees\nunion\nselect employee_id,job_id\nfrom job_history;";
	gsp_dbvendor vendor = dbvoracle;

	char *sqlText = "SELECT wp_term_taxonomy.term_taxonomy_id FROM wp_term_taxonomy  JOIN wp_terms USING (term_id) WHERE taxonomy = 'post_tag' AND wp_terms.slug IN ('eurovisie-songfestival');";
	
//	char *sqlText = "SELECT 2 AS nsptyp,\n"
//       "nsp.nspname, nsp.oid, pg_get_userbyid(nspowner) AS namespaceowner, nspacl, description,       FALSE as cancreate\n"
//  "FROM pg_namespace nsp\n"
//  "LEFT OUTER JOIN pg_description des ON (des.objoid=nsp.oid AND des.classoid='pg_namespace'::regclass)\n"
// "WHERE ((nspname = 'pg_catalog' AND EXISTS (SELECT 1 FROM pg_class WHERE relname = 'pg_class' AND relnamespace = nsp.oid )) OR\n"
//"(nspname = 'pgagent' AND EXISTS (SELECT 1 FROM pg_class WHERE relname = 'pga_job' AND relnamespace = nsp.oid )) OR\n"
//"(nspname = 'information_schema' AND EXISTS (SELECT 1 FROM pg_class WHERE relname = 'tables' AND relnamespace = nsp.oid LIMIT 1)) OR\n"
//"(nspname LIKE '_%' AND EXISTS (SELECT 1 FROM pg_proc WHERE proname='slonyversion' AND pronamespace = nsp.oid ))\n"
//")";
//
//	gsp_dbvendor vendor = dbvpostgresql;

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
			fprintf(stderr,"script file %s doesn't exist!\n", argv[argIndex+1] );
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
		getchar();
		return 1;
	}

	tableInfoList = createStringList(TRUE);
	fieldInfoList = createStringList(TRUE);
	schemaInfoList = createStringList(TRUE);
	functionInfoList = createStringList(TRUE);

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
			else if(node->nodeType == t_gsp_functionCall)
				visitor->handle_node[node->nodeType](node, visitor);
		}
	}

	if(!tableInfoList->isEmpty(tableInfoList)){
		tableInfoList->sort(tableInfoList);
		_printInfo(infoResult, "Tables:\n");
		_printListInfo(tableInfoList, infoResult);
	}

	if(!fieldInfoList->isEmpty(fieldInfoList)){
		fieldInfoList->sort(fieldInfoList);
		_printInfo(infoResult,"Fields:\n");
		_printListInfo(fieldInfoList, infoResult);
	}

	if(!schemaInfoList->isEmpty(schemaInfoList)){
		schemaInfoList->sort(schemaInfoList);
		_printInfo(infoResult,"Schemas:\n");
		_printListInfo(schemaInfoList, infoResult);
	}


	if(!functionInfoList->isEmpty(functionInfoList)){
		functionInfoList->sort(functionInfoList);
		_printInfo(infoResult,"Functions:\n");
		_printListInfo(functionInfoList, infoResult);
	}

	tableInfoList->dispose(tableInfoList);
	fieldInfoList->dispose(fieldInfoList);
	schemaInfoList->dispose(schemaInfoList);
	functionInfoList->dispose(functionInfoList);

	traverser->dispose(traverser);
	_disposeVisitor(visitor);
	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);
	gsp_parser_free(parser);
	
	//getchar();

	return 0;
}
