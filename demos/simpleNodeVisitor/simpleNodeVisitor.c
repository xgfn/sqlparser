/*!
 *  \brief     	Visit the gsp node list.
 *
 *  \details   	This demo is used to demonstrate how to visit the gsp node list.
 *  \author    	cnfree2000@hotmail.com
 *  \version   	1a
 *  \date      	2013
 *  \pre       	need to compile with core parser and extension library.
 *  \copyright 	Gudu Software
*/

/*!
**  \file simpleNodeVisitor.c
**
*/

#include "node_visitor.h"

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
	_printInfo(infoResult,"Visit expression: %s \r\n", str);
	gsp_free(str);
}

static void _process_resultColumn(gsp_node *node, struct gsp_visitor *visitor){
	char *str = gsp_node_text(node);
	_printInfo(infoResult,"Visit result column: %s \r\n", str);
	gsp_free(str);
}

static void _process_joinItem(gsp_node *node, struct gsp_visitor *visitor){
	char *str = gsp_node_text(node);
	if(((gsp_fromTable*)node)->fromtableType == ets_join)
		_printInfo(infoResult,"Visit join item: %s \r\n", str);
	gsp_free(str);
}

static struct gsp_visitor* createVisitor(){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = 0;
	visitor->handle_node[t_gsp_resultColumn] = _process_resultColumn;
	visitor->handle_node[t_gsp_expr] = _process_expr;
	visitor->handle_node[t_gsp_fromTable] = _process_joinItem;
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
	gsp_dbvendor vendor = dbvoracle;
#if 1
	char *sqlText = "SELECT p.name AS product,\n"
		"       p.listprice AS 'List Price',\n"
		"       p.discount AS 'discount' \n"
		"FROM   \n"
		"  production1.product p \n"
		"  left /*fdf */ JOIN production2.productsubcategory s ON p.productsubcategoryid = s.productsubcategoryid \n"
		"WHERE  s.name LIKE product \n"
		"       AND p.listprice < maxprice;  \n";
#endif
#if 0
	char *sqlText = "INSERT INTO table_name (column1,column2,column3) VALUES (value1,value2,value3), (value1,value2,value3);";
#endif

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

	traverser = createSqlTraverser();

	nodeList = traverser->traverseSQL(traverser, &parser->pStatement[0]);

	visitor = createVisitor();
	iter = nodeList->getIterator(nodeList);
	while(nodeList->hasNext(nodeList,&iter))
	{
		gsp_node *node = (gsp_node*)nodeList->next(&iter);
		if(node->nodeType == t_gsp_resultColumn || node->nodeType == t_gsp_expr || node->nodeType == t_gsp_fromTable )
			visitor->handle_node[node->nodeType](node, visitor);
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
