/*!
 *  \brief     Find and replace constant.
 *
 *  \details   This demo is used to demonstrate how to find and replace constant
 *             in sql script
 *  \author    cnfree2000@hotmail.com
 *  \version   1a
 *  \date      2013
 *  \pre       need to compile with core parser and extension library.
 *  \copyright Gudu Software
*/

/*!
**  \file collectConst.c
**
*/

#include "node_visitor.h"

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

static void _process_expr(gsp_node *node, struct gsp_visitor *visitor){
	char *str = gsp_node_text(node);
	if(((gsp_expr*)node)->expressionType == eet_simple_constant){
		if(constCount == 0){
			_printInfo(infoResult," %s", str);
		}
		else{
			_printInfo(infoResult,", %s", str);
		}
		gsp_node_set_text(node,"?");
		constCount++;
	}
	gsp_free(str);
}

static struct gsp_visitor* createVisitor(){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = 0;
	visitor->handle_node[t_gsp_expr] = _process_expr;
	return visitor;
}

static void disposeVisitor(struct gsp_visitor* visitor){
	free(visitor);
}

int main(int argc,char *argv[])
{
	int rc, argIndex, index;
	gsp_sqlparser *parser;
	List *nodeList;
	Iterator iter;
	struct gsp_visitor *visitor;
	SqlTraverser *traverser;
	List *argList;
	FILE *sqlFile;
	FILE *infoResult;
	gsp_dbvendor vendor = dbvoracle;

	char *sqlText = "with SET0 as( select 'x' from dual union all select 'x' from dual)\n"
		"   , SET1 as ( select 'a' from SET0 s1, SET0 s2)\n"
		"   , SET2 as ( select 'b' from SET1 s1, SET1 s2)\n"
		"   , SET3 as ( select 'c' from SET3 s1, SET3 s2)\n"
		"   , SET4 as ( select 'd' from SET4 s1, SET4 s2)\n"
		"   , ControlSet AS (SELECT ROW_NUMBER() OVER(ORDER BY (SELECT 1 from Dual)) rid  FROM SET4)\n"
		"  select t_interval(Interval_Get_udf(TO_TIMESTAMP(startTime, 'dd/mm/yyyy') + rid/24 , 2))\n"
		"  into intervalList\n"
		"  from ControlSet;\n";


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

	fprintf(stdout,"Find Consts:");

	traverser = createSqlTraverser();

	nodeList = traverser->traverseSQL(traverser, &parser->pStatement[0]);

	visitor = createVisitor();
	iter = nodeList->getIterator(nodeList);

	while(nodeList->hasNext(nodeList,&iter))
	{
		gsp_node *node = (gsp_node *)nodeList->next(&iter);
		if(node->nodeType == t_gsp_expr)
			visitor->handle_node[node->nodeType](node, visitor);
	}

	if(constCount>0){
		char *str = gsp_node_newtext((gsp_node *)parser->pStatement[0].stmt);
		_printInfo(infoResult,"\n\nReplace Consts with '?':\n");
		_printInfo(infoResult," %s", str);
		gsp_free(str);
	}

	traverser->dispose(traverser);
	disposeVisitor(visitor);

	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(infoResult!=NULL)
		fclose(infoResult);

	gsp_parser_free(parser);
	return 0;
}
