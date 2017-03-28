#ifndef _NODE_VISITOR_H
#define _NODE_VISITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include "linked_list.h"
#include "tree_map.h"
#include "memento.h"
#include "comparer.h"
#include "expr_traverse.h"

PRIVATE typedef enum ESqlClause
{
	esc_unknown,
	esc_resultColumn,
	esc_where,
	esc_having,
	esc_groupby,
	esc_orderby,
	esc_joinCondition,
	esc_join,
	esc_hierarchical,
	esc_compute,
	esc_top,
	esc_set,
	esc_insertValues,
	esc_selectInto,
	esc_qualify,
	esc_sample,
	esc_teradataWith,
	esc_expandOn,
	esc_limit,
	esc_selectValue,
	esc_viewAlias,
	esc_cte,
	esc_forUpdate,
	esc_output,
	esc_returning,
	esc_setVariable,
	esc_lockingClause
} ESqlClause;

typedef struct NodeContext{
	gsp_node* self;
	struct NodeContext* parent;
}NodeContext;

typedef struct ExprContext{
	gsp_expr* self;
	NodeContext* parent;
	gsp_base_statement* stmt;
	ESqlClause clauseType;
}ExprContext;

typedef struct SymbolTableItem{
	EDBObjectType type;
	gsp_base_statement* stmt;
	gsp_node* data;
}SymbolTableItem;

//It's the key of sqlInfoMap map.
typedef enum sql_field_type{
	sft_field=1,
	sft_table=2,
	sft_resultColumn=3,
	sft_whereClause=4,
	sft_groupByClause=5,
	sft_orderByClause=6,
	sft_symbolTable=7,
}sql_field_type;

//It's the key of tableInfoMap map.
typedef enum table_field_type{
	tft_objectName=1,
	tft_cteName=2,
	tft_cteColumnRefs=3,
}table_field_type;

typedef struct SqlTraverser
{
	//Store traverser gsp_nodes.
	PRIVATE List* __nodeList;
	//Store gsp_node contexts.
	PRIVATE List* __entries;
	//Store stmt detail informations.
	PRIVATE Map* __sqlInfoMap;
	//A gsp_node - NodeContext map, parent context's node is stmt.
	PRIVATE Map* __contextMap;
	PRIVATE NodeContext* __context;
	//A gsp_table - gsp_objectname list map.
	PRIVATE Map* __tableInfoMap;
	//Store unTableDetermined List
	PRIVATE List* __unTableDeterminedList;

	PRIVATE gsp_sqlparser* __parser;

	//Return node list. Don't dispose it by self.
	List* (*traverseSQL)(struct SqlTraverser*, gsp_sql_statement *sql);
	//Return stmt relative information. Don't dispose it by self.
	Map* (*getSQLInfo)(struct SqlTraverser*, gsp_base_statement *stmt);
	//Return node context entry, an entry contains the container stmt and the parent context. Don't dispose it by self.
	NodeContext* (*getContext)(struct SqlTraverser*, gsp_node *node);
	//Return table object name references. Don't dispose it by self.
	List* (*getTableObjectNameReferences)(struct SqlTraverser*, gsp_node *table);
	//Dispose the traverser.
	void (*dispose)(struct SqlTraverser*);
	//User callback interface, it's used for checking the unTableDeterminedColumn.
	BOOL (*checkColumn)(char* schema, char* table, char* column);
	//Check if the specify column is a table determined column.
	BOOL (*isTableDetermined)(struct SqlTraverser*, gsp_objectname *objectname);
	//Chect if the specify table is a base table.
	BOOL (*isBaseTable)(struct SqlTraverser*, gsp_node *table);
}SqlTraverser;

SqlTraverser* createSqlTraverser();

#ifdef __cplusplus
}
#endif

#endif /* _NODE_VISITOR_H */
