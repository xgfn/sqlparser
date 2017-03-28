/*!
*  \brief     Convert Oracle or SQL Server proprietary joins to ANSI SQL compliant joins.
*
*  \details   This demo is used to demonstrate how to convert Oracle or SQL Server proprietary 
*             joins to ANSI SQL compliant joins.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser and extension library.
*  \copyright Gudu Software
*/

/*!
**  \file     JoinConverter.c
**
*/

#include "node_visitor.h"
#include "modifysql.h"
#include "expr_traverse.h"
#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "JoinConverter.h"

static void _analyzeExpr(ExprTableVisitor* visitor, gsp_expr* expr);

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

JoinCondition* createJoinCondition(){
	JoinCondition* jc = (JoinCondition*)malloc(sizeof(JoinCondition));
	jc->expr = NULL;
	jc->jt = inner;
	jc->leftcolumn = NULL;
	jc->lefttable = NULL;
	jc->lexpr = NULL;
	jc->rexpr = NULL;
	jc->rightcolumn = NULL;
	jc->righttable = NULL;
	jc->used = FALSE;
	return jc;
}

FromClause* createFromClause(){
	FromClause* fc = (FromClause*)malloc(sizeof(FromClause));
	fc->condition = NULL;
	fc->joinClause = NULL;
	fc->joinTable = NULL;
	return fc;
}

ExprTableVisitor* createExprTableVisitor(){
	ExprTableVisitor* visitor = (ExprTableVisitor*)malloc(sizeof(ExprTableVisitor));
	visitor->tables = createList();
	visitor->jrs = createList();
	return visitor;
}

JoinConverter* createJoinConverter(gsp_dbvendor vendor){
	JoinConverter* converter = (JoinConverter*)malloc(sizeof(JoinConverter));
	converter->ErrorMessage = CStringNew();
	converter->ErrorNo = 0;
	converter->query = NULL;
	converter->sqlFile = NULL;
	converter->dbvendor = vendor;
	return converter;
}

void disposeJoinConverter(JoinConverter* converter){
	CStringDelete(converter->ErrorMessage);
	free(converter);
}

void disposeExprTableVisitor(ExprTableVisitor* visitor){
	List* jrs = visitor->jrs;
	int i=0;
	for(i=0;i<jrs->size(jrs);i++){
		JoinCondition* jr = (JoinCondition*)jrs->get(jrs, i);
		free(jr);
	}
	visitor->tables->dispose(visitor->tables);
	visitor->jrs->dispose(visitor->jrs);
	free(visitor);
}

static BOOL is_compare_condition( EExpressionType t )
{
	return ( ( t == eet_simple_comparison )
		|| ( t == eet_group_comparison ) || ( t == eet_in ) );
}

static char* getExpressionTable( gsp_expr* expr )
{
	if ( expr->objectOperand != NULL )
		return gsp_getSourceTokenText(expr->objectOperand->objectToken);
	else if ( expr->leftOperand!=NULL
		&& expr->leftOperand->objectOperand != NULL )
		return gsp_getSourceTokenText(expr->leftOperand->objectOperand->objectToken);
	else if ( expr->rightOperand != NULL
		&& expr->rightOperand->objectOperand != NULL )
		return gsp_getSourceTokenText(expr->rightOperand->objectOperand->objectToken);
	else
		return NULL;
}

void analyzeMssqlJoinCondition(ExprTraverser *traverser, gsp_expr * expr )
{
	gsp_expr *slexpr, *srexpr;
	List* jrs;
	gsp_sourcetoken* startToken, *endToken;
	JoinCondition* jr = createJoinCondition();
	slexpr = expr->leftOperand;
	srexpr = expr->rightOperand;
	jr->used = FALSE;
	jr->lexpr = slexpr;
	jr->rexpr = srexpr;
	jr->expr = expr;

	startToken = expr->leftOperand->fragment.endToken->pNext;
	endToken = expr->rightOperand->fragment.startToken;

	if(startToken!=NULL && endToken!=NULL){
		while(startToken!=NULL){
			if(strcmp(gsp_getSourceTokenText(startToken), "*=") == 0 || strcmp(gsp_getSourceTokenText(startToken), "=*") == 0 ){
				gsp_setSourceTokenText(startToken, "=");
			}
			startToken = startToken->pNext;
			if(startToken == endToken){
				break;
			}
		}
	}

	
	if (expr->expressionType == eet_left_join)
	{
		jr->jt = left;
	}
	if (expr->expressionType == eet_right_join)
	{
		jr->jt = right;
	}
	if ( ( slexpr->expressionType == eet_simple_constant ) )
	{
		jr->lefttable = NULL;
		jr->righttable = getExpressionTable( srexpr );
	}
	else if ( srexpr->expressionType == eet_simple_constant )
	{
		jr->righttable = NULL;
		jr->lefttable = getExpressionTable( slexpr );
	}
	else
	{
		jr->lefttable = getExpressionTable( slexpr );
		jr->righttable = getExpressionTable( srexpr );
	}
	jrs = ((ExprTableVisitor*)traverser->context)->jrs;
	jrs->add(jrs, jr );
}

static gsp_walking_result __exprVisit(ExprTraverser *traverser, gsp_expr *expr, BOOL isLeafNode){
	gsp_expr *slexpr, *srexpr, *lc_expr;
	lc_expr = expr;

	if (expr->expressionType == eet_left_join || expr->expressionType == eet_right_join)
	{
		analyzeMssqlJoinCondition(traverser, expr);
	}	

	if ( is_compare_condition(lc_expr->expressionType)){
		slexpr = lc_expr->leftOperand;
		srexpr = lc_expr->rightOperand;

		if (slexpr == NULL || srexpr == NULL)
		{
			return gsp_walking_continue;
		}

		if(slexpr->expressionType == eet_outer_join || srexpr->expressionType == eet_outer_join){
			List* jrs;
			JoinCondition* jr = createJoinCondition();
			jr->used = FALSE;
			jr->lexpr = slexpr;
			jr->rexpr = srexpr;
			jr->expr = expr;
			if(slexpr->expressionType == eet_outer_join){
				// If the plus is on the left, the join type is right
				// out join.
				jr->jt = right;
				// remove (+)
				gsp_setSourceTokenText(slexpr->fragment.endToken, "");
			}
			if ( srexpr->expressionType == eet_outer_join )
			{
				// If the plus is on the right, the join type is left
				// out join.
				jr->jt = left;
				gsp_setSourceTokenText(srexpr->fragment.endToken, "");
			}
			if ( ( slexpr->expressionType == eet_simple_constant ) )
			{
				jr->lefttable = NULL;
				jr->righttable = getExpressionTable( srexpr );
			}
			else if ( srexpr->expressionType == eet_simple_constant )
			{
				jr->righttable = NULL;
				jr->lefttable = getExpressionTable( slexpr );
			}
			else
			{
				jr->lefttable = getExpressionTable( slexpr );
				jr->righttable = getExpressionTable( srexpr );
			}

			jrs = ((ExprTableVisitor*)traverser->context)->jrs;
			jrs->add(jrs, jr );
		}
		else if ( ( slexpr->expressionType == eet_simple_object_name )
			&& ( !startsWith(gsp_getNodeText((gsp_node*)slexpr), ":" ) )
			&& ( !startsWith(gsp_getNodeText((gsp_node*)slexpr), "?" ) )
			&& ( srexpr->expressionType == eet_simple_object_name )
			&& ( !startsWith(gsp_getNodeText((gsp_node*)srexpr), ":" ) )
			&& ( !startsWith(gsp_getNodeText((gsp_node*)srexpr), "?" ) ))
		{
			List* jrs;
			JoinCondition* jr = createJoinCondition();
			jr->used = false;
			jr->lexpr = slexpr;
			jr->rexpr = srexpr;
			jr->expr = expr;
			jr->jt = inner;
			jr->lefttable = getExpressionTable( slexpr );
			jr->righttable = getExpressionTable( srexpr );
			jrs = ((ExprTableVisitor*)traverser->context)->jrs;
			jrs->add( jrs,jr );
		}
		else
		{
			// not a join condition
		}

	}
	return gsp_walking_continue;
}

static char* getTableName( gsp_fromTable* table )
{
	if(table->subQueryNode!=NULL)
		return gsp_getNodeText((gsp_node*)table->subQueryNode);
	else if(table->tableName!=NULL)
		return gsp_getNodeText((gsp_node*)table->tableName);
	else 
		return gsp_getNodeText((gsp_node*)table);
}

static BOOL isNameOfTable( gsp_fromTable* table, char* name )
{
	return ( name == NULL ) ? FALSE : (compareToIgnoreCase(getTableName(table),
		name ) == 0);
}

static BOOL isAliasOfTable( gsp_fromTable* table, char* alias )
{
	if ( table->aliasClause == NULL )
	{
		return FALSE;
	}
	else
		return ( alias == NULL ) ? FALSE : (compareToIgnoreCase(gsp_getNodeText((gsp_node*)table->aliasClause),
		alias ) == 0);
}

static BOOL isNameOrAliasOfTable( gsp_fromTable* table, char* str )
{
	return isAliasOfTable( table, str ) || isNameOfTable( table, str );
}

static List* getJoinCondition( gsp_fromTable* lefttable,
							  gsp_fromTable* righttable, List* jrs )
{
	List* lcjrs = createList();
	int i;
	for ( i = 0; i < jrs->size(jrs); i++ )
	{
		JoinCondition* jc = (JoinCondition*)jrs->get(jrs, i );
		if ( jc->used )
		{
			continue;
		}

		if ( isNameOrAliasOfTable( lefttable, jc->lefttable )
			&& isNameOrAliasOfTable( righttable, jc->righttable ) )
		{
			lcjrs->add(lcjrs, jc );
			jc->used = TRUE;
		}
		else if ( isNameOrAliasOfTable( lefttable, jc->righttable )
			&& isNameOrAliasOfTable( righttable, jc->lefttable ) )
		{
			if ( jc->jt == left )
				jc->jt = right;
			else if ( jc->jt == right )
				jc->jt = left;

			lcjrs->add( lcjrs, jc );
			jc->used = TRUE;
		}
		else if ( ( jc->lefttable == NULL )
			&& ( isNameOrAliasOfTable( lefttable, jc->righttable ) || isNameOrAliasOfTable( righttable,
			jc->righttable ) ) )
		{
			// 'Y' = righttable.c1(+)
			lcjrs->add( lcjrs, jc );
			jc->used = TRUE;
		}
		else if ( ( jc->righttable == NULL )
			&& ( isNameOrAliasOfTable( lefttable, jc->lefttable ) || isNameOrAliasOfTable( righttable,
			jc->lefttable ) ) )
		{
			// lefttable.c1(+) = 'Y'
			if ( jc->jt == left )
				jc->jt = right;
			else if ( jc->jt == right )
				jc->jt = left;
			lcjrs->add( lcjrs,jc );
			jc->used = TRUE;
		}
	}
	return lcjrs;
}

static BOOL areTableJoined( gsp_fromTable* lefttable, gsp_fromTable* righttable,
						   List* jrs )
{

	BOOL ret = FALSE;
	int i;
	for ( i = 0; i < jrs->size( jrs); i++ )
	{
		JoinCondition* jc = jrs->get( jrs, i );
		if ( jc->used )
		{
			continue;
		}
		ret = isNameOrAliasOfTable( lefttable, jc->lefttable )
			&& isNameOrAliasOfTable( righttable, jc->righttable );
		if ( ret )
			break;
		ret = isNameOrAliasOfTable( lefttable, jc->righttable )
			&& isNameOrAliasOfTable( righttable, jc->lefttable );
		if ( ret )
			break;
	}

	return ret;
}

static char* getJoinType( List* jrs )
{
	int i;
	for ( i = 0; i < jrs->size( jrs); i++ )
	{
		if ( ((JoinCondition*)jrs->get( jrs, i ))->jt == left )
		{
			return "left outer join";
		}
		else if ( ((JoinCondition*)jrs->get( jrs, i ))->jt == right )
		{
			return "right outer join";
		}
	}
	return "inner join";
}



static int getFromTableIndex( List* fromClauses, gsp_selectStatement * select, int i )
{

	gsp_listcell *cell;
	gsp_fromTable *table = NULL;
	int index = 0;
	foreach(cell, select->fromTableList){
		if(index == i){
			table = (gsp_fromTable *)gsp_list_celldata(cell);
			break;
		}
		index++;
	}

	if(table!=NULL){
		int i=0;
		for (i=0;i<fromClauses->size(fromClauses);i++)
		{
			FromClause* fromClause = (FromClause*)fromClauses->get(fromClauses, i);
			char* fromTable = gsp_getNodeText((gsp_node*) fromClause->joinTable);
			char* toTable = gsp_getNodeText((gsp_node*) table);
			if(fromClause->joinTable == table)
				return i;
		}
	}

	return -1;
}

static void analyzeSelect(JoinConverter* converter, gsp_selectStatement * select, gsp_sqlparser *parser) 
{
	if ( !select->setOperator!=eso_none )
	{
		if ( select->fromTableList->length <= 1 )
			return;

		if ( select->whereCondition == NULL || select->whereCondition->condition == NULL)
		{
			CString *joinBuffer = CStringNew();
			gsp_fromTable *fromTable = (gsp_fromTable *)gsp_list_first(select->fromTableList);
			gsp_listcell *cell = select->fromTableList->head;
			int i;

			CStringAppend(joinBuffer, getTableName(fromTable));
			if(fromTable->aliasClause!=NULL){
				CStringAppend(joinBuffer, " ");
				CStringAppend(joinBuffer, gsp_getNodeText((gsp_node*)fromTable->aliasClause));
			}

			for ( i = 1; i < select->fromTableList->length; i++ )
			{
				gsp_fromTable *table;
				cell = cell->nextCell;
				table = (gsp_fromTable *)gsp_list_celldata(cell);
				CStringAppend(joinBuffer, "\ncross join ");
				CStringAppend(joinBuffer, getTableName(table));
				if(table->aliasClause!=NULL){
					CStringAppend(joinBuffer, " ");
					CStringAppend(joinBuffer, gsp_getNodeText((gsp_node*)table->aliasClause));
				}
			}

			for ( i = select->fromTableList->length - 1; i > 0; i-- )
			{
				gsp_removeJoinItem(select, i);
			}
			gsp_setNodeText(parser, (gsp_node *)fromTable, joinBuffer->buffer, TRUE);
			CStringDeleteWithoutBuffer(joinBuffer);
		}
		else{
			ExprTraverser visitor;
			List* jrs;
			BOOL* tableUsed;
			int i,j,k;
			BOOL foundTableJoined;
			List* fromClauses;
			CString *fromclause = CStringNew();
			List* tables = createList();
			gsp_fromTable *fromTable;
			gsp_listcell *cell;
			List* removeExprs;
			if(select->fromTableList!=NULL){
				foreach(cell, select->fromTableList){
					gsp_fromTable *join = (gsp_fromTable *)gsp_list_celldata(cell);
					tables->add(tables, join);
				}
			}

			fromTable = (gsp_fromTable *)gsp_list_first(select->fromTableList);

			visitor.exprVisit = __exprVisit;
			visitor.context = createExprTableVisitor();
			preOrderTraverse(&visitor, select->whereCondition->condition);

			jrs = ((ExprTableVisitor*)visitor.context)->jrs;
			tableUsed = (BOOL*)malloc(sizeof(BOOL)*select->fromTableList->length);
			for ( i = 0; i < select->fromTableList->length; i++ )
			{
				tableUsed[i] = FALSE;
			}

			CStringAppend(fromclause, getTableName(fromTable));
			if(fromTable->aliasClause!=NULL){
				CStringAppend(fromclause, " ");
				CStringAppend(fromclause, gsp_getNodeText((gsp_node*)fromTable->aliasClause));
			}

			tableUsed[0] = TRUE;

			fromClauses = createList();
			removeExprs = createList();
			while(TRUE){
				foundTableJoined = FALSE;
				for ( i = 0; i < select->fromTableList->length; i++ )
				{
					gsp_fromTable *lcTable1;
					gsp_fromTable *leftTable = NULL;
					gsp_fromTable *rightTable = NULL;
					lcTable1 = (gsp_fromTable *)tables->get(tables,i);
					for(j = i + 1; j < select->fromTableList->length; j++ ){
						gsp_fromTable* lcTable2 =  (gsp_fromTable *)tables->get(tables,j);
						CString *condition = CStringNew();
						if ( areTableJoined( lcTable1, lcTable2, jrs ) ){
							if ( tableUsed[i] && ( !tableUsed[j] ) )
							{
								leftTable = lcTable1;
								rightTable = lcTable2;
							}
							else if ( ( !tableUsed[i] ) && tableUsed[j] )
							{
								leftTable = lcTable2;
								rightTable = lcTable1;
							}
							if ( ( leftTable != NULL )
								&& ( rightTable != NULL ) )
							{
								List* lcjrs = getJoinCondition( leftTable,
									rightTable,
									jrs );
								FromClause* fc = createFromClause();
								
								if(lcjrs->isEmpty(lcjrs))
									continue;
									
								fc->joinTable = rightTable;
								fc->joinClause = getJoinType( lcjrs );

								for ( k = 0; k < lcjrs->size( lcjrs); k++ )
								{
									gsp_expr* lc_expr = ((JoinCondition*)lcjrs->get(lcjrs, k))->expr;
									CStringAppend(condition, gsp_getNodeText((gsp_node*)lc_expr));
									if ( k != lcjrs->size( lcjrs ) - 1 )
									{
										CStringAppend(condition, " and ");
									}
									removeExprs->add(removeExprs, lc_expr);
								}
								fc->condition = condition->buffer;
								CStringDeleteWithoutBuffer(condition);

								fromClauses->add(fromClauses, fc );
								tableUsed[i] = TRUE;
								tableUsed[j] = TRUE;

								foundTableJoined = TRUE;
							}
						}
					}
					
														
				}
				if ( !foundTableJoined )
				{
					break;
				}

				
			}

			for(i=0;i<removeExprs->size(removeExprs);i++){
				gsp_removeExpression((gsp_expr*)removeExprs->get(removeExprs,i));
			}

			removeExprs->dispose(removeExprs);


			// are all join conditions used?
			for ( i = 0; i < jrs->size(jrs ); i++ )
			{
				JoinCondition* jc = (JoinCondition*)jrs->get(jrs, i );
				if ( !jc->used )
				{
					for ( j = fromClauses->size(fromClauses ) - 1; j >= 0; j-- )
					{
						FromClause* clause =  ((FromClause*)fromClauses->get( fromClauses, j ));
						if ( isNameOrAliasOfTable(clause->joinTable,
							jc->lefttable )
							|| isNameOrAliasOfTable( clause->joinTable,
							jc->righttable ) )
						{
							CString *condition = CStringNew();
							CStringAppend(condition, clause->condition);
							CStringAppend(condition, " and ");
							CStringAppend(condition, gsp_getNodeText((gsp_node*)jc->expr));

							clause->condition = condition->buffer;
							CStringDeleteWithoutBuffer(condition);

							jc->used = true;
							gsp_removeExpression(jc->expr);
							break;
						}
					}
				}
			}

			for ( i = 0; i < tables->size(tables); i++ )
			{
				if ( !tableUsed[i] )
				{
					converter->ErrorNo++;
					CStringAppendFormat(converter->ErrorMessage, "\nError %d, Message: %s", converter->ErrorNo, "This table has no join condition: ");
					CStringAppend(converter->ErrorMessage, getTableName((gsp_fromTable*)tables->get(tables,i)));
				}
			}

			// link all join clause
			for ( i = 1; i < select->fromTableList->length; i++ )
			{
				int index = getFromTableIndex(fromClauses, select, i);

				FromClause* fc = (FromClause*)fromClauses->get( fromClauses, index);

				CString *buffer = CStringNew();
				CString* buffer1 = CStringNew();
				CStringAppend(buffer, fromclause->buffer);
				CStringAppend(buffer, "\n");

				
				CStringAppend(buffer1, trimString(fc->joinClause));
				CStringAppend(buffer1, " ");

				CStringAppend(buffer1, getTableName(fc->joinTable));
				if(fc->joinTable->aliasClause!=NULL){
					CStringAppend(buffer1, " ");
					CStringAppend(buffer1, gsp_getNodeText((gsp_node*)fc->joinTable->aliasClause));
				}

				CStringAppend(buffer1, " on ");
				CStringAppend(buffer1, fc->condition);

				CStringDelete(fromclause);

				fromclause = CStringNew();
				CStringAppend(fromclause, buffer->buffer);
				CStringAppend(fromclause, trimString(buffer1->buffer));

				CStringDelete(buffer);
				CStringDelete(buffer1);
				
				fromClauses->remove(fromClauses, fc);
			}

			fromClauses->dispose(fromClauses);

			for ( k = select->fromTableList->length; k > 0; k-- )
			{
				gsp_removeJoinItem(select, k);
			}
		
			gsp_setNodeText(parser, (gsp_node*)fromTable, fromclause->buffer, TRUE);

			if ( select->whereCondition!=NULL 
				&& compareToIgnoreCase(trimString(gsp_getNodeText( (gsp_node*)select->whereCondition)), "where") == 0 ) 
			{
				// no where condition, remove WHERE keyword
				gsp_removeWhereClause((gsp_base_statement*)select);
			}

			tables->dispose(tables);
			disposeExprTableVisitor((ExprTableVisitor*)visitor.context);
		}
	}
}

static struct gsp_visitor* _createVisitor(SqlTraverser* traverser){
	struct gsp_visitor *visitor = (struct gsp_visitor *)malloc(sizeof(struct gsp_visitor)+t_gsp_exceptionClause*sizeof(&visitor->handle_node));
	visitor->context = traverser;
	return visitor;
}

static void _disposeVisitor(struct gsp_visitor* visitor){
	free(visitor);
}

int convert( JoinConverter* converter )
{

	gsp_sqlparser *parser;
	gsp_sql_statement *stmt;
	List *nodeList;
	Iterator iter;
	struct gsp_visitor *visitor;
	SqlTraverser* traverser;
	Stack* selectStack;

	int rc = gsp_parser_create(converter->dbvendor,&parser);

	if (rc){
		return rc;
	}

	if(converter->sqlFile!=NULL){
		rc= gsp_check_syntax_file(parser, converter->sqlFile);
	}
	else {
		rc= gsp_check_syntax(parser, converter->query);
	}
	
	if (rc != 0){
		CStringAppend(converter->ErrorMessage, gsp_errmsg(parser));
		gsp_parser_free(parser);
		return rc;
	}

	stmt = &parser->pStatement[0];

	traverser = createSqlTraverser();
	nodeList = traverser->traverseSQL(traverser, &parser->pStatement[0]);
	visitor = _createVisitor(traverser);
	selectStack = createStack();

	iter = nodeList->getIterator(nodeList);
	while(nodeList->hasNext(nodeList,&iter))
	{
		gsp_node *node = (gsp_node*)nodeList->next(&iter);
		if(node->nodeType == t_gsp_selectStatement){
			selectStack->push(selectStack, (gsp_selectStatement*)node);
		}
	}
	traverser->dispose(traverser);
	_disposeVisitor(visitor);

	while(!selectStack->isEmpty(selectStack)){
		gsp_selectStatement* select =  (gsp_selectStatement*)selectStack->pop(selectStack);
		analyzeSelect( converter,select,parser);
	}
	selectStack->dispose(selectStack);

	converter->query = gsp_getNodeText((gsp_node*)stmt->stmt);

	gsp_parser_free(parser);

	return converter->ErrorNo;
}

int main(int argc,char *argv[])
{
	int argIndex, index;
	List *argList;
	FILE *sqlFile, *convertResult;
	char *sqlFileName;
	char *sqlText = "SELECT e.employee_id,\n       e.last_name,\n       e.department_id\nFROM   employees e,\n       departments d\nWHERE  e.department_id(+) = d.department_id";
	JoinConverter* converter;
	gsp_dbvendor vendor = dbvoracle;

	argList = createStringList(FALSE);
	for(index=0;index<argc;index++){
		argList->add(argList, argv[index]);
	}

	argIndex = argList->indexOf(argList, "-o");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		convertResult=fopen(argv[argIndex+1],"w");
	}
	else convertResult = NULL;

	argIndex = argList->indexOf(argList, "-f");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		sqlFileName = argv[argIndex+1];
		sqlFile=fopen(argv[argIndex+1],"r");
		if(sqlFile == NULL){
			fprintf(stderr,"script file %s doesn't exist!\n", argv[argIndex+1] );
			if(convertResult!=NULL)
				fclose(convertResult);
			argList->dispose(argList);
			return 1;
		}
	}
	else sqlFile = NULL;

	argIndex = argList->indexOf(argList, "-d");
	if(argIndex!=-1 && argList->size(argList) >argIndex+1){
		if(strcmp("mssql",argv[argIndex+1])==0){
			vendor = dbvmssql;
		}
	}

	argList->dispose(argList);

	if(sqlFile == NULL){
		if(vendor == dbvmssql)
			_printInfo( convertResult, "SQL with SQL Server propriety joins\n" );
		else
			_printInfo( convertResult, "SQL with Oracle propriety joins\n" );
		_printInfo( convertResult, sqlText );
	}
	else{
		if(vendor == dbvmssql)
			_printInfo( convertResult, "SQL script with SQL Server propriety joins\n" );
		else
			_printInfo( convertResult, "SQL script with Oracle propriety joins\n" );
		_printInfo( convertResult, sqlFileName );
	}
	converter = createJoinConverter( vendor );

	if(sqlFile == NULL){
		converter->query = sqlText;
	}
	else{
		converter->sqlFile = sqlFile;
	}

	if ( convert(converter) != 0 )
	{
		_printInfo( convertResult, converter->ErrorMessage->buffer );
	}
	else
	{
		_printInfo( convertResult, "\n\nSQL in ANSI joins\n" );
		_printInfo( convertResult, converter->query );
	}

	disposeJoinConverter(converter);

	if(sqlFile!=NULL)
		fclose(sqlFile);
	if(convertResult!=NULL)
		fclose(convertResult);

	return 0;
}
