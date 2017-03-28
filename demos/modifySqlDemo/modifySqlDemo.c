/*!
 *  \brief     Modify sql.
 *
 *  \details   The demo is used to demonstrate how to use the modifysql api.
 *  \author    cnfree2000@hotmail.com
 *  \version   1a
 *  \date      2013
 *  \pre       need to compile with core parser and extension library.
 *  \copyright Gudu Software
*/

/*!
**  \file modifysqlTester.c
**
*/
#include "modifysql.h"

int main()
{
	int rc;
	gsp_sqlparser *parser;

	char *sqltext = "SELECT ProductID, ListPrice \nFROM Product, Product1 \nleft join Product1 t2 on t1.f1 = t2.f2 \nWHERE ListPrice!=1 AND ListPrice!=3 \nGROUP BY ListPrice \nHAVING SUM(ListPrice)>1000 \nORDER BY ListPrice ASC";

	rc = gsp_parser_create(dbvoracle,&parser);
	if (rc){
		fprintf(stderr,"create parser error");
		return 1;
	}

	rc= gsp_check_syntax(parser, sqltext);
	if (rc != 0){
		fprintf(stderr,"parser error:%s\n",gsp_errmsg(parser));
		return 1;
	}

	fprintf(stdout,"Original SQL:\n%s\n\n", sqltext);

	{
		gsp_selectStatement *stmt = (gsp_selectStatement *)(&parser->pStatement[0])->stmt;
		gsp_node *node = (gsp_node*)stmt;
		int count = 0;
		gsp_expr* rightExpr;
		gsp_expr* leftExpr;
		char* nodeText; 

		fprintf(stdout,"Remove Join Item: Product1\n");
		gsp_removeJoinItem(stmt, 1);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add OrderBy Item: ProductID DESC\n");
		gsp_addOrderBy(parser, stmt, "ProductID DESC");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add GroupBy Item: ProductID\n");
		gsp_addGroupBy(parser, stmt, "ProductID");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Remove GroupBy and OrderBy\n");
		gsp_removeGroupBy(stmt);
		gsp_removeOrderBy(stmt);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add OrderBy Item: ListPrice ASC\n");
		gsp_addOrderBy(parser, stmt, "ListPrice ASC");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add GroupBy Item: ProductID\n");
		gsp_addGroupBy(parser, stmt, "ProductID");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add Having Clause: SUM(ProductID)<10\n");
		gsp_addHavingClause(parser, stmt, "SUM(ProductID)<10");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		rightExpr = stmt->whereCondition->condition->rightOperand;
		leftExpr = stmt->whereCondition->condition->leftOperand;

		nodeText = gsp_getNodeText((gsp_node*)rightExpr);
		fprintf(stdout,"Remove Condition: %s\n", trimString(nodeText));
		free(nodeText);

		gsp_removeExpression(rightExpr);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add Condition: ListPrice!=2\n");
		gsp_addWhereClause(parser,(gsp_base_statement*)stmt, "ListPrice!=2");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		
		nodeText = gsp_getNodeText((gsp_node*)stmt->whereCondition);
		fprintf(stdout,"Remove Where Clause: %s\n", trimString(nodeText));
		free(nodeText);

		gsp_removeWhereClause((gsp_base_statement*)stmt);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		nodeText = gsp_getNodeText((gsp_node*)stmt->groupByClause->havingClause);
		fprintf(stdout,"Remove Having Clause: %s\n", trimString(nodeText));
		free(nodeText);

		gsp_removeHavingClause(stmt);
		
		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		nodeText = gsp_getNodeText((gsp_node*)stmt->groupByClause);
		fprintf(stdout,"Remove GroupBy Clause: %s\n", trimString(nodeText));
		free(nodeText);

		gsp_removeGroupBy(stmt);
		
		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		nodeText = gsp_getNodeText((gsp_node*)stmt->orderbyClause);
		fprintf(stdout,"Remove OrderBy Clause: %s\n", trimString(nodeText));
		free(nodeText);

		gsp_removeOrderBy(stmt);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Remove Column: ListPrice\n");

		gsp_removeResultColumn((gsp_base_statement*)stmt, 1);

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

		fprintf(stdout,"Add Column: ProductName\n");

		gsp_addResultColumn(parser, (gsp_base_statement*)stmt, "ProductName");

		nodeText = gsp_getNodeText((gsp_node*)stmt);
		fprintf(stdout,"Modified SQL:\n%s\n\n", trimString(nodeText));
		free(nodeText);

	}
	gsp_parser_free(parser);
	return 0;
}
