#ifndef _MODIFY_SQL_H
#define _MODIFY_SQL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsp_base.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_node.h"
#include "gsp_sqlparser.h"
#include "cstring.h"  
#include "collection.h"  
#include "tree_map.h"  
#include "linked_list.h"  

	void gsp_setSourceTokenText(gsp_sourcetoken* token, char* text);
	void gsp_removeSourceToken( gsp_sourcetoken* token );

	char* gsp_getNodeText(gsp_node* node);
	char* gsp_getSourceTokenText(gsp_sourcetoken* token);

	int gsp_setNodeText(gsp_sqlparser *parser, gsp_node* node, char* text, BOOL forceUpdateOuterNode);

	void gsp_addWhereClause(gsp_sqlparser *parser, gsp_base_statement* select, char* text);
	void gsp_addOrderBy(gsp_sqlparser *parser, gsp_selectStatement* select, char* text);
	void gsp_addGroupBy(gsp_sqlparser *parser, gsp_selectStatement* select, char* text);
	void gsp_addHavingClause(gsp_sqlparser *parser, gsp_selectStatement* select, char* text);
	void gsp_addJoinItem(gsp_sqlparser *parser, gsp_selectStatement* select, char* text);
	void gsp_addResultColumn(gsp_sqlparser *parser, gsp_base_statement* stmt, char* text);

	void gsp_removeWhereClause(gsp_base_statement* select);
	void gsp_removeOrderBy(gsp_selectStatement* select);
	void gsp_removeGroupBy(gsp_selectStatement* select);
	void gsp_removeHavingClause(gsp_selectStatement* select);
	void gsp_removeExpression(gsp_expr* expr);
	void gsp_removeJoinItem(gsp_selectStatement* select, int joinIndex);
	void gsp_removeResultColumn(gsp_base_statement* stmt, int columnIndex);
	void gsp_removeOrderByItem(gsp_selectStatement* select, int orderByIndex);
	void gsp_removeGroupByItem(gsp_selectStatement* select, int groupByIndex);

#ifdef __cplusplus
}
#endif

#endif /* _MODIFY_SQL_H */