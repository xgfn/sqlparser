#ifndef GSP_NODE_H
#define GSP_NODE_H

/*!
**  \file  gsp_node.h
**  \brief functions to manipulate parse tree nodes
*/

#include "gsp_base.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gsp_selectSqlNode;

/*!
* select statement
*/
typedef struct gsp_selectSqlNode gsp_selectStatement;

typedef struct gsp_dummy
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_node *node1;
	int int1;
}gsp_dummy;

typedef enum EConstantType{
	ect_unknown,ect_integer,ect_float,ect_string,ect_boolean,ect_null
}EConstantType;

//typedef enum EParameterMode{
//	epm_default,epm_in,epm_out
//}EParameterMode;


/*!
*  A constant (sometimes called a literal) specifies a value
*/
typedef struct gsp_constant
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_sourcetoken *signToken;
	gsp_sourcetoken *stringToken;
	EConstantType constantType;
}gsp_constant;

/**
** Oracle: [schema.]object[.part][@link]
** link : database[<.domain>[...n]][@connect_descriptor]
**
** object is the name of the object.
** schema is the schema containing the object.
** part is a part of the object. This identifier lets you refer to a part of a schema
** object, such as a column or a partition of a table. Not all types of objects have
** parts.
** dblink applies only when you are using the Oracle Database distributed
** functionality.
**
** postal_code is attribute
** SELECT c.cust_address.postal_code FROM customers c;
** 
*/
struct gsp_objectname
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	EDBObjectType objectType; 
	gsp_sourcetoken *serverToken;	/*!< sql server server token */
	gsp_sourcetoken *databaseToken; /*!< sql server database token */
	gsp_sourcetoken *schemaToken;
	gsp_sourcetoken *objectToken;
	gsp_sourcetoken *partToken;
	gsp_sourcetoken *propertyToken;	/*!< sql server property token */
	gsp_list *fields; /*!< list of fields which type is gsp_objectname */
	/*! 
		how many tokens consists of this objectname, 
		+ =1, partToken
		+ =2, objectToken.partToken
		+ =3, schemaToken.objectToken.partToken
		+ =4, schemaToken.objectToken.partToken.fields
				or databaseToken.schemaToken.objectToken.partToken
		+ =5, serverToken.databaseToken.schemaToken.objectToken.partToken
		+ =6, serverToken.databaseToken.schemaToken.objectToken.partToken.propertyToken
	*/
	int nTokens; 
	struct gsp_objectname *dblink;
	gsp_list *indices; /*!< PostgreSQL, list of gsp_indices */
};

typedef struct gsp_expr
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	EExpressionType expressionType;
	gsp_objectname *objectOperand;
	gsp_sourcetoken *operatorToken;
	struct gsp_expr *leftOperand;
	struct gsp_expr *rightOperand;
	struct gsp_selectSqlNode *subQueryNode; /*!< tramformed to gsp_selectStatement */
	//gsp_selectStatement *subquery; // use subQueryNode instead
	struct gsp_sql_statement *subQueryStmt; // in plsql
	gsp_constant *constantOperand;
	gsp_list *exprList;
	struct gsp_functionCall *functionCall;
	struct gsp_objectAccess *objectAccess;
	struct gsp_caseExpression *caseExpression;
	gsp_sourcetoken *quantifier;
	gsp_sourcetoken *notToken;
	struct gsp_expr *likeEscapeOperand;
	struct gsp_expr *betweenOperand;
	struct gsp_arrayAccess *arrayAccess;
	struct gsp_typename *dataTypeName;
	struct gsp_intervalExpression *intervalExpression;
	gsp_list *indices; /*!< PostgreSQL, list of gsp_indices */
	gsp_list *newVariantTypeArgumentList; /*!< Teradata, list of gsp_newVariantTypeArgument */
	//int isSubQueryInStmt;
}gsp_expr;

typedef struct gsp_objectAccess
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_expr *objectExpr;
	gsp_list *attributes;
	struct gsp_functionCall *method;
}gsp_objectAccess;

typedef struct gsp_aliasClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_objectname *aliasName;
	gsp_sourcetoken *AsToken;
	gsp_list *nameList; /*!< list of gsp_objectname */
}gsp_aliasClause;

typedef struct gsp_resultColumn
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_expr *expr;
	gsp_aliasClause *aliasClause;
}gsp_resultColumn;


typedef struct gsp_trimArgument
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_sourcetoken *both_trailing_leading;
	gsp_expr *stringExpression;
	gsp_expr *trimCharacter;
}gsp_trimArgument;


typedef struct gsp_orderByItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_expr *sortKey;
	gsp_sourcetoken *sortToken;
}gsp_orderByItem;

typedef struct gsp_orderBy
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_list *items;
}gsp_orderBy;


typedef struct gsp_keepDenseRankClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_orderBy *orderBy;
}gsp_keepDenseRankClause;

typedef struct gsp_analyticFunction
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_keepDenseRankClause *keepDenseRankClause;
	gsp_list *partitionBy_ExprList;
	gsp_orderBy *orderBy;
}gsp_analyticFunction;

typedef struct gsp_functionCall
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	EFunctionType functionType;
	gsp_objectname *functionName;
	EAggregateType aggregateType;
	gsp_trimArgument *trimArgument;
	gsp_analyticFunction *analyticFunction;
	gsp_list *Args; /*!< list of expr */
	gsp_sourcetoken *extract_time_token;
	gsp_expr *expr1,*expr2,*expr3;
	struct gsp_typename *dataTypename;
	struct gsp_windowDef *windowDef; /*!< postgresql */
	gsp_orderBy *sortClause; /*!< postgresql */
	gsp_list *sortList; /*!< Teradata, list of gsp_orderByItem */
}gsp_functionCall;


typedef struct gsp_whenClauseItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_expr *comparison_expr;
	gsp_list *stmts;
	gsp_expr *return_expr;
	gsp_list *countFractionDescriptionList; /*!< Teradata, list of gsp_expr */
}gsp_whenClauseItem;

typedef struct gsp_caseExpression
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_expr *input_expr;
	gsp_expr *else_expr;
	gsp_list *whenClauseItemList;
	gsp_list *else_statement_node_list;
}gsp_caseExpression;

typedef struct gsp_intervalExpression
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_intervalExpression;


typedef struct gsp_precisionScale
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	struct gsp_constant *precision;
	struct gsp_constant *scale;
}gsp_precisionScale;

typedef struct gsp_typename
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EDataType dataType;
	gsp_precisionScale	*precisionScale;
	struct gsp_constant *secondsPrecision;
	struct gsp_constant *length;
	gsp_objectname *genericName;
	gsp_list *indices; /*!< Postgresql, list of gsp_Indices*/
	gsp_list *datatypeAttributeList; /*!< Teradata, list of gsp_datatypeAttribute */
}gsp_typename;


typedef enum EKeyActionType {
    eka_delete,
    eka_update
}EKeyActionType;

typedef struct gsp_keyReference
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EKeyReferenceType referenceType;
}gsp_keyReference;

typedef struct gsp_keyAction
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EKeyActionType actionType;
	gsp_keyReference *keyReference;
}gsp_keyAction;

typedef struct gsp_constraint
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EConstraintType constraintType;
	gsp_objectname *constraintName;
	gsp_expr *checkCondition;
	gsp_list *columnList;
	gsp_list *automaticProperties;
	gsp_objectname *referencedObject;
	gsp_list *referencedColumnList;
	gsp_list *keyActions; /*<! list of gsp_keyAction */
	gsp_expr *defaultValue;
	gsp_expr *seedExpr;
	gsp_expr *incrementExpr;
}gsp_constraint;

typedef struct gsp_mergeInsertClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *columnList;//TObjectNameList
	gsp_list *valuelist; //TResultColumnList
	gsp_expr *deleteWhereClause;
}gsp_mergeInsertClause;

typedef struct gsp_mergeUpdateClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *updateColumnList;//TResultColumnList
	gsp_expr *updateWhereClause;
	gsp_expr *deleteWhereClause;
}gsp_mergeUpdateClause;

typedef struct gsp_mergeDeleteClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mergeDeleteClause;

typedef struct gsp_mergeWhenClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {emt_matched,emt_not_matched,
			emt_matched_with_condition,
			emt_not_matched_with_condition,
			emt_not_matched_by_target,
			emt_not_matched_by_target_with_condition,
			emt_not_matched_by_source,
			emt_not_matched_by_source_with_condition} matchType;
	gsp_expr *condition;
	gsp_mergeUpdateClause *updateClause;
	gsp_mergeInsertClause *insertClause;
	gsp_mergeDeleteClause *deleteClause;
	struct gsp_db2_signal *signalStmt;

}gsp_mergeWhenClause;

typedef struct gsp_dataChangeTable
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_node *stmtNode;
}gsp_dataChangeTable;

typedef struct gsp_fromTable
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_aliasClause *aliasClause;
	ETableSource fromtableType;
	struct gsp_selectSqlNode *subQueryNode;
	gsp_objectname *tableName;
	struct gsp_pxGranule *pxGranule;
	struct gsp_tableSample *tableSample;
	struct gsp_flashback *flashback;
	struct gsp_joinExpr *joinExpr;
	gsp_expr *tableExpr;
	gsp_sourcetoken *tableonly;
	gsp_dataChangeTable *dataChangeTable;
	gsp_list *tableHints; /*!< list of gsp_mssql_tableHint */
	gsp_functionCall *functionCall;
	struct gsp_mssql_openQuery *openQuery;
	struct gsp_mssql_openDatasource *openDatasource;
	struct gsp_mssql_containsTable *containsTable;
	struct gsp_mssql_freeTable *freeTable;
	struct gsp_mssql_openRowSet *openRowSet;
	struct gsp_mssql_openXML *openXML;
	struct gsp_pivotClause *pivotClause;
	struct gsp_unPivotClause *unPivotClause;
	gsp_list *rowList; /*!< list of gsp_multiTarget */
	struct gsp_informixOuterClause *outerClause;
}gsp_fromTable;

typedef struct gsp_table
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	ETableSource tableSource;
	gsp_aliasClause *aliasClause;
	gsp_objectname *tableName;
	//gsp_selectStatement *subquery;
	gsp_expr *tableExpr;
}gsp_table;

typedef struct gsp_setSchemaSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *schemaName; /*<! schema name */
	gsp_list *schemaNameList; /*<! list of schema name */
}gsp_setSchemaSqlNode;

typedef struct gsp_setSchemaSqlNode  gsp_setSchemaStatement;


typedef struct gsp_mergeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *cteList;
	gsp_list *whenClauses;/*!< list of TMergeWhenClause */ 
	gsp_fromTable *targetTableNode;
	gsp_fromTable *usingTableNode;
	gsp_expr *condition;
	struct gsp_mssql_outputClause *outputClause;
	gsp_list *columnList; /*!< list of columns type of gsp_objectname */


	gsp_table *targetTable;
	gsp_table *usingTable;
}gsp_mergeSqlNode;

typedef struct gsp_mergeSqlNode  gsp_mergeStatement;

typedef struct gsp_alterTableOption
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EAlterTableOptionType optionType;
	gsp_objectname *constraintName;
	gsp_objectname *newConstraintName;
	gsp_objectname *columnName;
	gsp_objectname *newColumnName;
	gsp_objectname *referencedObjectName;
	gsp_objectname *newTableName;
	gsp_objectname *indexName;
	gsp_list *columnDefinitionList; /*<! list of gsp_columnDefinition */
	gsp_list *constraintList;
	gsp_list *columnNameList;
	gsp_list *referencedColumnList;
	gsp_typename *datatype;
}gsp_alterTableOption;

typedef struct gsp_alterTableSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_list *tableElementList;/*<! list of gsp_tableElement */
	gsp_list *alterTableOptionList;/*<! list of gsp_alterTableOption */ 
}gsp_alterTableSqlNode;

typedef struct gsp_alterTableSqlNode  gsp_alterTableStatement;

typedef struct gsp_createSequenceSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *sequenceName;
	gsp_list *options;
}gsp_createSequenceSqlNode;

typedef struct gsp_createSequenceSqlNode gsp_createSequenceStatement;

typedef struct gsp_createSynonymSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *synonymName;
	gsp_objectname *forName;
	int isPublic;
	int isReplace;
}gsp_createSynonymSqlNode;

typedef struct gsp_createSynonymSqlNode gsp_createSynonymStatement;

typedef struct gsp_createDirectorySqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *directoryName;
	gsp_constant *path;
}gsp_createDirectorySqlNode;

typedef struct gsp_createDirectorySqlNode  gsp_createDirectoryStatement;

typedef struct gsp_dropViewSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *viewNameList;//list of gsp_objectname
}gsp_dropViewSqlNode;

typedef struct gsp_dropViewSqlNode gsp_dropViewStatement;

typedef struct gsp_dropDatabaseSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStmtType stmtType;
	gsp_list *databaseNameList;//list of gsp_objectname
}gsp_dropDatabaseSqlNode;

typedef struct gsp_dropDatabaseSqlNode gsp_dropDatabaseStatement;

typedef struct gsp_alterDatabaseSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStmtType stmtType;
	gsp_objectname *databaseName;// gsp_objectname
}gsp_alterDatabaseSqlNode;

typedef struct gsp_alterDatabaseSqlNode gsp_alterDatabaseStatement;

typedef struct gsp_createDatabaseSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStmtType stmtType;
	gsp_objectname *databaseName;// gsp_objectname
}gsp_createDatabaseSqlNode;

typedef struct gsp_createDatabaseSqlNode gsp_createDatabaseStatement;


typedef struct gsp_dropTableSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *tableNameList;
}gsp_dropTableSqlNode;

typedef struct gsp_dropTableSqlNode gsp_dropTableStatement;

typedef struct gsp_dropIndexItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *indexName, *tableName;
}gsp_dropIndexItem;


typedef struct gsp_dropIndexSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *indexName;
	gsp_list *itemList; /*<? list of gsp_dropIndexItem */
}gsp_dropIndexSqlNode;

typedef struct gsp_dropIndexSqlNode gsp_dropIndexStatement;

typedef struct gsp_truncateTableSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
}gsp_truncateTableSqlNode;



typedef struct gsp_viewAliasItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *alias;
	gsp_list *columnConstraintList;//list of TConstraint
	gsp_constraint *tableConstraint;
}gsp_viewAliasItem;

typedef struct gsp_viewAliasClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *viewAliasItemList;
}gsp_viewAliasClause;

typedef struct gsp_createViewSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *viewName;
	struct gsp_selectSqlNode *selectSqlNode;
	gsp_viewAliasClause *viewAliasClause;
	int isForce;
	int isReplace;
	gsp_objectname *rowTypeName;

	//gsp_selectStatement *subquery;
}gsp_createViewSqlNode;

typedef struct gsp_createViewSqlNode gsp_createViewStatement;

typedef struct gsp_createMaterializedViewSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *viewName;
	struct gsp_selectSqlNode *selectSqlNode;
	gsp_viewAliasClause *viewAliasClause;
}gsp_createMaterializedViewSqlNode;


typedef struct gsp_createMaterializedViewLogSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *mvName;
}gsp_createMaterializedViewLogSqlNode;


typedef struct gsp_createIndexSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EIndexType indexType;
	gsp_objectname *indexName;
	gsp_objectname *tableName;
	gsp_list *indexItemList;/*<! list of gsp_orderByItem list */
}gsp_createIndexSqlNode;

typedef struct gsp_commitSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *transName;
}gsp_commitSqlNode;

typedef struct gsp_rollbackSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *transName;
}gsp_rollbackSqlNode;

typedef struct gsp_saveTransSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *transName;
}gsp_saveTransSqlNode;



typedef gsp_createIndexSqlNode gsp_createIndexStatement;

typedef struct gsp_columnDefinition
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *columnName;
	gsp_typename *datatype;
	gsp_list *constraints;/*<! list of TConstraint */ 
	gsp_expr *defaultExpression;
	gsp_expr *computedColumnExpression;
	int isNull;
	int isRowGuidCol;
}gsp_columnDefinition;

typedef struct gsp_tableElement
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {ete_column_def, ete_table_constraint} kind;
	gsp_columnDefinition *columnDefinition;
	gsp_constraint *constraint;
}gsp_tableElement;


typedef struct gsp_createTableSqlNode
{
	ENodeType	nodeType;

	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_table *table,*oftable;
	gsp_list *tableElementList;
	gsp_list *columnList;
	gsp_objectname *columnName;

	//gsp_selectStatement *subquery;
	struct gsp_selectSqlNode *subQueryNode;
	gsp_objectname *superTableName;
	gsp_objectname *ofTypeName;
	gsp_objectname *likeTableName; /*!< MySQL */
}gsp_createTableSqlNode;

/*!
* create table statement
*/
typedef gsp_createTableSqlNode gsp_createTableStatement;

//typedef struct gsp_createTableStatement
//{
//	ENodeType	nodeType;
//
//	gsp_fragment fragment;
//	gsp_table *table;
//	gsp_list *tableElementList;
//	gsp_list *columnList;
//	gsp_objectname *columnName;
//
//	gsp_selectStatement *subquery;
//}gsp_createTableStatement;


typedef struct gsp_returningClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *columnValueList;
	gsp_list *variableList;
}gsp_returningClause;

typedef struct gsp_isolationClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_isolationClause;

typedef struct gsp_includeColumns
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *columnList;
}gsp_includeColumns;


typedef struct gsp_deleteSqlNode
{
	ENodeType	nodeType;

	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_sourcetoken *deleteToken;
	gsp_list *cteList;
	struct gsp_whereClause *whereCondition;
	gsp_returningClause *returningClause;
	gsp_isolationClause *isolationClause;
	gsp_includeColumns *includeColumns; //db2
	
	gsp_fromTable *targetTableNode;
	struct gsp_topClause *topClause;
	struct gsp_mssql_outputClause  *outputClause;
	gsp_list *sourceTableList; /*<! list of gsp_fromTable in from clause */
	gsp_list *targetTableList; /*<! MySQL, list of gsp_fromTable */
	struct gsp_limitClause *limitClause; /*!< MySQL limit clause */
	gsp_orderBy *sortClause; /*!< MySQL order by clause */

	
	//only in gsp_deleteStatement
	//gsp_table *targetTable;
}gsp_deleteSqlNode;

/*!
* delete statement
*/
typedef struct gsp_deleteSqlNode gsp_deleteStatement;

typedef struct gsp_updateSqlNode
{
	ENodeType	nodeType;

	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_sourcetoken *updateToken;
	gsp_list *cteList;
	gsp_list *resultColumnList;
	struct gsp_whereClause *whereCondition;
	gsp_returningClause *returningClause;
	gsp_isolationClause *isolationClause;
	gsp_includeColumns *includeColumns; //db2

	gsp_fromTable *targetTableNode;

	struct gsp_topClause *topClause;
	struct gsp_mssql_outputClause  *outputClause;
	gsp_list *sourceTableList; /*<! list of gsp_fromTable of from clause */
	gsp_list *targetTableList; /*<! MySQL, list of gsp_fromTable */
	struct gsp_limitClause *limitClause; /*!< MySQL limit clause */
	gsp_orderBy *sortClause; /*!< MySQL order by clause */

	// only in gsp_updateStatement
	//gsp_table *targetTable;
	//gsp_list *joins;

}gsp_updateSqlNode;

/*!
* update statement
*/
typedef struct gsp_updateSqlNode gsp_updateStatement;

typedef struct gsp_multiTarget
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	struct gsp_selectSqlNode *subQueryNode;
	//gsp_selectStatement *subquery;
	gsp_list *resultColumnList; //list of gsp_resultcolumn
}gsp_multiTarget;


typedef struct gsp_insertRest
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EInsertValue valueType;
	gsp_list *multiTargetList;
	struct gsp_selectSqlNode *subQueryNode;
	gsp_functionCall *functionCall;
	gsp_objectname *recordName;
	gsp_list *updateTargetList; /*!< MySQL, list of gsp_resultColumn */
}gsp_insertRest;

typedef struct gsp_insertValuesClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *multiTargetList; // list of gsp_multiTarget
}gsp_insertValuesClause;

typedef struct gsp_insertIntoValue
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_fromTable *fromTable;
	gsp_list *columnList;//objectname list
	gsp_insertValuesClause *valuesClause;
	gsp_table *table;
}gsp_insertIntoValue;

typedef struct gsp_insertCondition
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *insertIntoValues;
}gsp_insertCondition;


typedef struct gsp_insertSqlNode
{
	ENodeType	nodeType;

	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_sourcetoken *insertToken;
	EInsertValue valueType;
	gsp_list *cteList;
	gsp_list *columnList;/*<! list of gsp_objectname */
	gsp_list *insertIntoValues;//list of gsp_insertIntoValue
	gsp_list *insertConditions;//list of gsp_insertCondition
	gsp_returningClause *returningClause;
	gsp_isolationClause *isolationClause;
	gsp_includeColumns *includeColumns; //db2

	gsp_fromTable *targetTableNode;
	gsp_insertRest *insertRest;
	struct gsp_selectSqlNode *subQueryNode;

	struct gsp_topClause *topClause;
	struct gsp_mssql_outputClause *outputClause;
	gsp_list *onDuplicateUpdateList; /*!< MySQL, list of gsp_resultColumn */

	// only in gsp_insertStatement
	gsp_list *multiTargetList;
	gsp_objectname *recordName;
	gsp_functionCall *functionCall;
	//gsp_selectStatement *subquery;
	//gsp_table *targetTable;

}gsp_insertSqlNode;

/*!
* insert statement
*/
typedef struct gsp_insertSqlNode gsp_insertStatement;

typedef struct gsp_whereClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
}gsp_whereClause;

typedef struct gsp_joinExpr
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EJoinType	jointype;
	EJoinType	original_jontype;
	gsp_aliasClause *aliasClause;
	gsp_fromTable *leftOperand;
	gsp_fromTable *rightOperand;
	gsp_expr *onCondition;
	gsp_list *usingColumns;
}gsp_joinExpr;

typedef struct gsp_tableSamplePart
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_tableSamplePart;

typedef struct gsp_tableSample
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_tableSample;

typedef struct gsp_pxGranule
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_pxGranule;

typedef struct gsp_flashback
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_flashback;

typedef struct gsp_forUpdate
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *columnRefs;//list of objectname
}gsp_forUpdate;

typedef struct gsp_groupingSetItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	struct gsp_rollupCube *rollupCubeClause;
	gsp_expr *expressionItem;
}gsp_groupingSetItem;

typedef struct gsp_groupingSet
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *items;
}gsp_groupingSet;

typedef struct gsp_rollupCube
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {erc_rollup,erc_cube} operation;
	gsp_list *items;
}gsp_rollupCube;

typedef struct gsp_gruopByItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
	gsp_rollupCube *rollupCube;
	gsp_groupingSet *groupingSet;
	gsp_aliasClause *aliasClause; /*!< MySQL */
}gsp_gruopByItem;

typedef struct gsp_groupBy
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_sourcetoken *stGroup,*stBy,*stHaving;
	gsp_expr *havingClause;
	gsp_list *items;// list of gsp_gruopByItem
}gsp_groupBy;



//typedef struct gsp_statementSqlNode
//{
//	ENodeType	nodeType;
//	gsp_fragment fragment;
//	gsp_node *sqlNode;
//	gsp_plsqlLabel label;
//	struct gsp_sql_statement *stmt;
//	int isParsed;
//}gsp_statementSqlNode;

typedef struct gsp_selectDistinct
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {edt_none,edt_Distinct,edt_DistinctOn,edt_All,edt_Unique,edt_DistinctRow} distinctType;
	gsp_list *exprList;
}gsp_selectDistinct;

typedef struct gsp_topClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	int bPercent;
	int bWithTies;
	gsp_expr *expr;
}gsp_topClause;


typedef struct gsp_hierarchical
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *connectByClause;
	gsp_expr *startWithClause;
}gsp_hierarchical;

typedef struct gsp_intoClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *exprList;
	gsp_sourcetoken *outfile;
}gsp_intoClause;


typedef struct gsp_valueClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *valueList;
	gsp_list *nameList;
}gsp_valueClause;

// db2
typedef struct gsp_fetchFirstClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_fetchFirstClause;

typedef struct gsp_optimizeForClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_optimizeForClause;


/**
* PostgreSQL value row item
*/

typedef struct gsp_valueRowItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
	gsp_list *exprList;
}gsp_valueRowItem;


typedef struct gsp_selectSqlNode
{
	ENodeType	nodeType;
	
	// fields below copied to gsp_selectStatement by memcpy, make keep same as gsp_selectStatement

	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	ESetOperator setOperator;
	gsp_list *cteList;//list of gsp_cte
	gsp_valueClause *valueClause;
	gsp_sourcetoken *selectToken;
	gsp_selectDistinct *selectDistinct;
	gsp_list *resultColumnList;
	gsp_intoClause *intoClause;
	gsp_whereClause *whereCondition;
	gsp_hierarchical *hierarchicalClause;
	struct gsp_groupBy *groupByClause;
	gsp_orderBy *orderbyClause;
	gsp_forUpdate *forupdateClause;
	gsp_fetchFirstClause *fetchFirstClause;
	gsp_optimizeForClause *optimizeForClause;
	gsp_isolationClause *isolationClause;
	struct gsp_mssql_computeClause *computeClause;
	gsp_topClause *topClause;
	struct gsp_intoTableClause *intoTableClause;
	struct gsp_limitClause *limitClause;
	struct gsp_lockingClause *lockingClause; /*!< postgresql lock clause */
	struct gsp_windowClause *windowClause; /*!< postgresql window clause */
	gsp_list *withClauses; /*!< teradata, list of gsp_teradataWithClause */
	struct gsp_qualifyClause *qualifyClause; /*!< teradata */
	struct gsp_sampleClause *sampleClause; /*!< teradata */
	struct gsp_expandOnClause *expandOnClause; /*!< teradata */

	//fields above copied to gsp_selectStatement by memcpy

	struct gsp_selectSqlNode *leftNode,*rightNode;
	gsp_list *fromTableList;

	//only in gsp_selectStatement
	//gsp_list *joins; //not used, use fromTableList instead.
	gsp_selectStatement *leftStmt,*rightStmt;
}gsp_selectSqlNode;



typedef struct gsp_cte
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_selectSqlNode *selectSqlNode;
	gsp_insertSqlNode *insertSqlNode;
	gsp_updateSqlNode *updateSqlNode;
	gsp_deleteSqlNode *deleteSqlNode;
	//gsp_selectStatement *subquery;
	//gsp_insertStatement *insertStmt;
	//gsp_updateStatement *updateStmt;
	//gsp_deleteStatement *deleteStmt;
	gsp_list *columnList;
}gsp_cte;

typedef struct gsp_commentSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EDBObjectType dbObjType;
	gsp_objectname *objectName;
	gsp_constant *message;
}gsp_commentSqlNode;

typedef struct gsp_callSpec
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_constant *declaration;
}gsp_callSpec;

typedef struct gsp_returnSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
	gsp_selectSqlNode *subQueryNode;
}gsp_returnSqlNode;

typedef struct gsp_continueSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_continueSqlNode;

typedef struct gsp_breakSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_breakSqlNode;

typedef struct gsp_grantSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *nameList;
}gsp_grantSqlNode;

/**
* PostgreSQL execute statement
*/
typedef struct gsp_executeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *moduleName;
	gsp_list *paramList; /*!< list of gsp_expr */
}gsp_executeSqlNode;

/**
* PostgreSQL drop role statement
*/
typedef struct gsp_dropRoleSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *roleNameList; /*!< list of gsp_objectname */
}gsp_dropRoleSqlNode;


/**
* PostgreSQL drop trigger statement
*/
typedef struct gsp_dropTriggerSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *triggerName;
	gsp_objectname *tableName;
}gsp_dropTriggerSqlNode;

/**
* PostgreSQL drop trigger statement
*/
typedef struct gsp_lockTableSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_lockTableSqlNode;



typedef struct gsp_revokeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_revokeSqlNode;

typedef struct gsp_fetchSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
	gsp_list *variableNames;
}gsp_fetchSqlNode;

typedef struct gsp_openSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
	gsp_list *nameList;
}gsp_openSqlNode;

typedef struct gsp_closeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
}gsp_closeSqlNode;

/**
* MySQL
*/
typedef struct gsp_iterateSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
}gsp_iterateSqlNode;

/**
* MySQL
*/
typedef struct gsp_leaveSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
}gsp_leaveSqlNode;


typedef struct gsp_createFunctionSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStoreProcedureMode kind;
	gsp_objectname *functionName;
	gsp_list *parameters;//TParameterDeclarationList
	gsp_typename *returnDataType;
	gsp_list *declareStmts;
	gsp_list *stmts;
	struct gsp_exceptionClause *exceptionClause;
	gsp_returnSqlNode *returnSqlNode;
	struct gsp_db2_compoundSqlNode *compoundSqlNode;
	gsp_callSpec *callSpec;
	struct gsp_blockSqlNode *blockSqlNode;
	struct gsp_sql_statement *stmtSqlNode;
}gsp_createFunctionSqlNode;

typedef struct gsp_createFunctionSqlNode  gsp_createFunctionStatement;


typedef struct gsp_parameterDeclaration
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *parameterName;
	gsp_typename *dataType;
	gsp_expr *defaultValue;
	int isVarying;
	gsp_constant *varyPrecision;
	int isNotNull;
	EParameterMode mode;
	EHowtoSetValue howtoSetValue;
}gsp_parameterDeclaration;

typedef struct gsp_createProcedureSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStoreProcedureMode kind;
	gsp_objectname *procedureName;
	gsp_list *parameters; /*!< list of gsp_parameterDeclaration */
	gsp_list *declareStmts; /*!< list of gsp_sql_statement */
	gsp_list *innerStmts; /*!< list of gsp_sql_statement */
	gsp_list *stmts;	/*!< list of gsp_sql_statement */
	struct gsp_exceptionClause *exceptionClause;
	gsp_callSpec *callSpec;
	struct gsp_blockSqlNode *blockSqlNode; /*!< MySQL */
	struct gsp_sql_statement *stmtSqlNode; /*!< MySQL */
}gsp_createProcedureSqlNode;

typedef struct gsp_createProcedureSqlNode gsp_createProcedureStatement;

typedef struct gsp_exceptionClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *Handlers;
}gsp_exceptionClause;


typedef struct gsp_blockSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_plsqlLabel label;
	gsp_list *stmts;
	gsp_exceptionClause *exceptionClause;
	gsp_list *declareStmts; /*!< list of type gsp_sql_statement */
}gsp_blockSqlNode;

typedef struct gsp_blockSqlNode gsp_blockStatement;

typedef struct gsp_arrayAccess
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *arrayName;
	gsp_expr *index1,*index2,*index3;
	gsp_objectname *propertyName;
}gsp_arrayAccess;

typedef enum gsp_EDeclareType{
	edc_variable,edc_conditions,edc_returnCode,
	edc_statement,edc_cursor,edc_handlers
}gsp_EDeclareType;

typedef enum gsp_EVariableType{
	evt_normal,evt_cursor,evt_table
}gsp_EVariableType;

typedef struct gsp_declareVariable
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *varName;
	gsp_typename *varDatatype;
	gsp_expr *defaultValue;
	gsp_list *tableTypeDefinitions; /*<! list of gsp_tableElement */
	gsp_EVariableType variableType;
}gsp_declareVariable;


typedef struct gsp_declareSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_EDeclareType declareType;
	gsp_list *vars; /*<! list of gsp_declareVariable */
	gsp_typename *varType;
	int howtoSetValue;
	gsp_expr *defaultValue;
	gsp_objectname *conditionName;
	gsp_objectname *cursorName;
	gsp_selectSqlNode *subQueryNode;
	gsp_sql_statement *stmt;
}gsp_declareSqlNode;

typedef struct gsp_createTriggerUpdateColumn
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *columnName;
}gsp_createTriggerUpdateColumn;


typedef struct gsp_ifSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *stmts; /*<! list of gsp_sql_statement */
	gsp_list *elseStmts; /*<! list of gsp_sql_statement */
	gsp_list *elseifList; /*<! list of gsp_elseIfSqlNode */
	gsp_sql_statement *stmt; /*<! sql server single statement */
	gsp_sql_statement *elseStmt; /*<! sql server single else statement */
	gsp_list  *updateColumnList; /*<! list of gsp_createTriggerUpdateColumn */
}gsp_ifSqlNode;

typedef struct gsp_elseIfSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *stmts; /*<! list of gsp_sql_statement */
}gsp_elseIfSqlNode;

typedef struct gsp_whileSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_sql_statement *stmt; /*<! sql server single statement */
	gsp_list *stmts; /*<! list of gsp_sql_statement */
}gsp_whileSqlNode;

typedef struct gsp_repeatSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *stmts; /*<! list of gsp_sql_statement */
}gsp_repeatSqlNode;


typedef struct gsp_loopSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *stmts; /*<! list of gsp_sql_statement */
}gsp_loopSqlNode;

/*!
** base statement of all transformed statements
*/
struct gsp_base_statement
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
};


typedef struct gsp_unknownStatement
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_unknownStatement;

//typedef struct gsp_join
//{
//	ENodeType	nodeType;
//	gsp_node *pNext,*pPrev;gsp_fragment fragment;
//	EJoinSource kind;
//	gsp_table *table;
//	struct gsp_join *join;
//	gsp_aliasClause *aliasClause;
//	gsp_list *joinItems;
//}gsp_join;
//
//typedef struct gsp_joinItem
//{
//	ENodeType	nodeType;
//	gsp_node *pNext,*pPrev;gsp_fragment fragment;
//	EJoinSource kind;
//	EJoinType	jointype;
//	gsp_table *table;
//	gsp_join *join;
//	gsp_expr *onCondition;
//	gsp_list *usingColumns;
//}gsp_joinItem;

typedef struct gsp_createTriggerSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	ETriggerMode fireMode;
	gsp_objectname *triggerName;
	gsp_objectname *tableName;
	gsp_expr *whenCondition;
	struct gsp_trigger_event *triggerEvent;
	struct gsp_db2_triggerAction *triggerAction;
	struct gsp_sql_statement *stmt;
	gsp_list *stmts; /*!< list of gsp_sql_statement */
	struct gsp_blockSqlNode *blockSqlNode; /*!< MySQL */
	struct gsp_sql_statement *stmtSqlNode; /*!< MySQL */
}gsp_createTriggerSqlNode;

typedef struct gsp_createTriggerSqlNode  gsp_createTriggerStatement;

typedef struct gsp_exceptionHandler
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *exceptionNames;
	gsp_list *stmts;
}gsp_exceptionHandler;

typedef struct gsp_pivotClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_functionCall *aggregationFunction;
	gsp_objectname *privotColumn;
	gsp_list *privotColumnList;/*!< list of gsp_objectname */
	gsp_list *inResultList; /*!< list of gsp_resultColumn  */
	gsp_aliasClause *aliasClause;
}gsp_pivotClause;

typedef struct gsp_unPivotClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_functionCall *aggregationFunction;
	gsp_objectname *valueColumn;
	gsp_objectname *privotColumn;
	gsp_list *privotColumnList;/*!< list of gsp_objectname */
	gsp_list *inResultList; /*!< list of gsp_resultColumn  */
	gsp_aliasClause *aliasClause;
}gsp_unPivotClause;

/**
* informix rename column statement
*/
typedef struct gsp_renameColumnSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *columnName;
	gsp_objectname *newColumnName;
}gsp_renameColumnSqlNode;

typedef struct gsp_renameColumnSqlNode gsp_renameColumnStmt;

/**
* informix rename sequence statement
*/
typedef struct gsp_renameSequenceSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *sequenceName;
	gsp_objectname *newSequenceName;
}gsp_renameSequenceSqlNode;

typedef struct gsp_renameSequenceSqlNode gsp_renameSequenceStmt;


/**
* informix rename table statement
*/
typedef struct gsp_renameTableSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_objectname *newTableName;
}gsp_renameTableSqlNode;

typedef struct gsp_renameTableSqlNode gsp_renameTableStmt;

/**
* informix rename index statement
*/
typedef struct gsp_renameIndexSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *indexName;
	gsp_objectname *newIndexName;
}gsp_renameIndexSqlNode;

typedef struct gsp_renameIndexSqlNode gsp_renameIndexStmt;


/**
* informix drop sequence statement
*/
typedef struct gsp_dropSequenceSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *sequenceName;
}gsp_dropSequenceSqlNode;

typedef struct gsp_dropSequenceSqlNode gsp_dropSequenceStmt;

/**
* informix drop synonym statement
*/
typedef struct gsp_dropSynonymSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *synonymName;
}gsp_dropSynonymSqlNode;

typedef struct gsp_dropSynonymSqlNode gsp_dropSynonymStmt;

/**
* informix drop synonym statement
*/
typedef struct gsp_dropRowTypeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *rowTypeName;
}gsp_dropRowTypeSqlNode;

typedef struct gsp_dropRowTypeSqlNode gsp_dropRowTypeStmt;

/**
* informix alter index statement
*/
typedef struct gsp_alterIndexSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EStmtType stmtType;
	gsp_objectname *indexName;
}gsp_alterIndexSqlNode;

/**
* PostGreSQL alter index statement
*/
typedef struct gsp_alterSequenceSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *sequenceName;
}gsp_alterSequenceSqlNode;

/**
* PostGreSQL alter view statement
*/
typedef struct gsp_alterViewSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *viewName;
}gsp_alterViewSqlNode;



typedef struct gsp_alterIndexSqlNode gsp_alterIndexStmt;

/**
* informix into table clause
*/
typedef struct gsp_intoTableClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
}gsp_intoTableClause;


typedef struct gsp_informixOuterClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_fromTable *fromTable;
	gsp_list *fromTableList;
}gsp_informixOuterClause;

typedef struct gsp_createRowTypeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *rowTypeName;
	gsp_objectname *superTableName;
	gsp_list *columnDefList; /*!< list of gsp_columnDefinition */
}gsp_createRowTypeSqlNode;

typedef struct gsp_createRowTypeSqlNode gsp_createRowTypeStmt;

/**
* informix subscript
*/
typedef struct gsp_subscripts
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_constant *first,*last;
}gsp_subscripts;

/**
* MySQL, PostgreSQL limit clause
*/
typedef struct gsp_limitClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *offset;
	gsp_expr *rowCount;
	gsp_expr *limitValue; /*!< postgresql */
}gsp_limitClause;


/**
* MySQL call statement
*/
typedef struct gsp_callSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *functionName;
	gsp_list *args;/*!< list of gsp_expr */
}gsp_callSqlNode;

typedef struct gsp_callSqlNode gsp_callStmt;

/**
* MySQL and SQL Server use database statement
*/
typedef struct gsp_useDatabaseSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *databaseName;
}gsp_useDatabaseSqlNode;

typedef struct gsp_useDatabaseSqlNode gsp_useDatabaseStmt;


/**
* PostgreSQL locking clause
*/

typedef struct gsp_lockingClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *lockedObjects; /*!< list of gsp_objectname */
}gsp_lockingClause;

/**
* PostgreSQL window clause
*/
typedef struct gsp_windowClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *windownsDefs; /*!< list of gsp_windowDef */
}gsp_windowClause;

typedef struct gsp_partitionClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *exprList;
}gsp_partitionClause;


/**
* PostgreSQL window def
*/
typedef struct gsp_windowDef
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *referenceName;
	gsp_partitionClause *partitionClause;
	gsp_orderBy *sortClause;
	struct gsp_windowDef *frameClause;
	gsp_expr *startOffset;
	gsp_expr *endOffset;
	gsp_objectname *windowName;
}gsp_windowDef;

/**
* PostgreSQL indices
*/
typedef struct gsp_indices
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *attributeName;
	gsp_expr *lowerSubscript;
	gsp_expr *upperSubscript;
}gsp_indices;


/**
* Teradata
*/
typedef struct gsp_collectStatisticsSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_objectname *indexName;
	gsp_objectname *columnName;
	gsp_list *columnList;/*!< list of gsp_objectname */
}gsp_collectStatisticsSqlNode;

typedef struct gsp_teradataWithClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *exprList;
	gsp_list *byList; /* list of gsp_orderByItem */
}gsp_teradataWithClause;

/**
* Teradata qualify clause
*/
typedef struct gsp_qualifyClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
}gsp_qualifyClause;

void gsp_setTableOnObjectNameList(gsp_list *dbobjects);

/**
* Teradata sample clause
*/
typedef struct gsp_sampleClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *countFractionDescriptionList; /*!< list gsp_expr */
	gsp_list *whenThenList; /*!< list of gsp_whenClauseItem */
}gsp_sampleClause;

/**
* Teradata expand on clause
*/
typedef struct gsp_expandOnClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
	gsp_aliasClause *columnAlias;
	gsp_expr *periodExpr;
}gsp_expandOnClause;

/**
* Teradata datatype attribute
*/
typedef struct gsp_datatypeAttribute
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_datatypeAttribute;

typedef struct gsp_newVariantTypeArgument
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
	gsp_objectname *aliasName;
}gsp_newVariantTypeArgument;

typedef struct gsp_outputFormatPhrase
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_typename *dataTypeName;
	gsp_datatypeAttribute *datatypeAttribute;

}gsp_outputFormatPhrase;

// oracle nodes
typedef struct gsp_plsqlCreateTypeBody
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *typeName;
	gsp_list *stmts;
}gsp_plsqlCreateTypeBody;



typedef struct gsp_typeAttribute
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *attributeName;
	gsp_typename *datatype;
}gsp_typeAttribute;

typedef struct gsp_plsqlCreateType
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *typeName;
	enum {ept_create,ept_declare,ept_define,ept_create_body,
		ept_create_incomplete,ept_create_varray,ept_create_nested_table,
		ept_create_type_placeholder} kind;
	gsp_list *attributes; // list of gsp_typeAttribute
}gsp_plsqlCreateType;

typedef struct gsp_dmlEventClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_list *dml_event_items;
}gsp_dmlEventClause;

typedef struct gsp_nonDmlTriggerClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *schemaName;
	EFireMode fireMode;
	int isSchema;
	int isDatabase;
	gsp_list *ddl_event_list;
	gsp_list *database_event_list;
}gsp_nonDmlTriggerClause;

typedef struct gsp_compoundDmlTriggerClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_dmlEventClause *dmlEventClause;
}gsp_compoundDmlTriggerClause;

typedef struct gsp_simpleDmlTriggerClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EFireMode fireMode;
	gsp_dmlEventClause *dmlEventClause;
}gsp_simpleDmlTriggerClause;


typedef struct gsp_createPackageSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *packageName;
	EStoreProcedureMode kind;
	gsp_list *definitions_or_declarations;
	gsp_list *stmts;//list of TStatementListSqlNode
	struct gsp_exceptionClause *exceptionClause;

	gsp_list *declareStatements;
	gsp_list *bodyStatements;
}gsp_createPackageSqlNode;

typedef struct gsp_createPackageSqlNode gsp_createPackageStatement;

typedef struct gsp_plsqlVarDeclStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EWhatDeclared whatDeclared;
	gsp_objectname *exception_name;
	gsp_expr *error_number;
	gsp_objectname *elementName;
	gsp_typename *dataType;
	gsp_expr *value;
	int isNotNull;
	EHowtoSetValue howtoSetValue;
}gsp_plsqlVarDeclStmt;



typedef struct  gsp_returnSqlNode gsp_plsqlReturnStmt;

typedef struct gsp_plsqlRaiseStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *exceptionName;
}gsp_plsqlRaiseStmt;

typedef struct gsp_plsqlLoopStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {elt_loop,elt_while,elt_for,elt_cursor_for} kind;
	gsp_list *stmts;
	gsp_expr *condition;
	gsp_objectname *indexName;
	gsp_objectname *cursorName;
	gsp_expr *lower_bound,*upper_bound;
	//gsp_sql_statement *subquery;
	gsp_list *cursorParameterNames;
}gsp_plsqlLoopStmt;



typedef struct gsp_plsqlCaseStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_plsqlLabel label;
	gsp_caseExpression *caseExpr;
}gsp_plsqlCaseStmt;

typedef struct gsp_plsqlForallStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *indexName;
	struct gsp_sql_statement *stmt;
}gsp_plsqlForallStmt;

typedef struct gsp_plsqlElsifStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *thenStmts;
}gsp_plsqlElsifStmt;

typedef struct gsp_plsqlIfStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *thenStmts;
	gsp_list *elseStmts;
	gsp_list *elsifStmts;
}gsp_plsqlIfStmt;

typedef struct gsp_plsqlGotoStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *gotolabelName;
}gsp_plsqlGotoStmt;

typedef struct gsp_plsqlExitStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *whenCondition;
	gsp_objectname *exitlabelName;
}gsp_plsqlExitStmt;

typedef struct gsp_plsqlAssignStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *left;
	gsp_expr *right;
}gsp_plsqlAssignStmt;

typedef struct gsp_plsqlCursorDeclStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	struct gsp_sql_statement *subqueryNode;
	//gsp_selectStatement *subquery;
	gsp_objectname *cursorName;
	gsp_list *cursorParameterDeclarations;
	gsp_typename *rowtype;
	enum {ecd_declaration,ecd_specification,ecd_body,
		ecd_ref_type_definition} kind;
	gsp_objectname *cursorTypeName;
}gsp_plsqlCursorDeclStmt;


typedef struct gsp_plsqlRecordTypeDefStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *typeName;
	gsp_list *fieldDeclarations;
}gsp_plsqlRecordTypeDefStmt;

typedef struct gsp_plsqlVarrayTypeDefStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *typeName;
	gsp_typename *elementDataType;
	gsp_constant *sizeLimit;
	int isNotNull;
}gsp_plsqlVarrayTypeDefStmt;

typedef struct gsp_plsqlTableTypeDefStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *typeName;
	gsp_typename *elementDataType;
	int isNotNull;
	gsp_typename *indexByDataType;
}gsp_plsqlTableTypeDefStmt;

typedef struct gsp_plsqlNullStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_plsqlNullStmt;


typedef struct gsp_fetchSqlNode gsp_plsqlFetchStmt;

typedef struct gsp_plsqlPipeRowStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_plsqlPipeRowStmt;

typedef struct gsp_bindArgument
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EParameterMode mode;
	gsp_expr *bindArgumentExpr;
}gsp_bindArgument;

typedef struct gsp_execImmeNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *dynamicStringExpr;
	gsp_list *intoVariables;
	gsp_list *bindArguments;
	gsp_list *returnNames;
}gsp_execImmeNode;

typedef struct gsp_plsqlOpenforStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorVariableName;
	//gsp_selectStatement *subquery;
	struct gsp_sql_statement *subqueryNode;
	gsp_expr *dynamic_string;
}gsp_plsqlOpenforStmt;

typedef struct gsp_plsqlOpenStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
	gsp_list *cursorParameterNames;
}gsp_plsqlOpenStmt;


typedef struct gsp_closeSqlNode gsp_plsqlCloseStmt;

typedef struct gsp_plsqlBasicStmt
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
}gsp_plsqlBasicStmt;

typedef struct gsp_trigger_event
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	enum {ete_simple,ete_compound,ete_non} dmlType;
	union
	{
		gsp_simpleDmlTriggerClause *simple;
		gsp_compoundDmlTriggerClause *compound;
		gsp_nonDmlTriggerClause *non;
	}dml;
}gsp_trigger_event;


//gsp_createPackageStatement * transformCreatePackageStmt(gsp_context *pContext,
//													gsp_createPackageSqlNode *createPackageNode);
//
// gsp_plsqlVarDeclStmt * transformPlsqlVarDeclStmt(gsp_context *pContext, 
//												  gsp_plsqlVarDeclStmt *varDeclNode);
//
// gsp_plsqlTableTypeDefStmt * transformPlsqlTableTypeDefStmt(gsp_context *pContext, 
//												  gsp_plsqlTableTypeDefStmt *tableTypeDefNode);
//
// gsp_plsqlVarrayTypeDefStmt * transformPlsqlVarrayTypeDefStmt(gsp_context *pContext, 
//												  gsp_plsqlVarrayTypeDefStmt *varrayTypeDefNode);
//
// gsp_plsqlRecordTypeDefStmt * transformPlsqlRecordTypeDefStmt(gsp_context *pContext, 
//												  gsp_plsqlRecordTypeDefStmt *recordTypeDefNode);
//
// gsp_plsqlCursorDeclStmt * transformPlsqlCursorDeclStmt(gsp_context *pContext, 
//												  gsp_plsqlCursorDeclStmt *plsqlCursorDeclNode);
//
// gsp_plsqlOpenforStmt * transformPlsqlOpenforStmt(gsp_context *pContext, 
//												  gsp_plsqlOpenforStmt *plsqlOpenforNode);
//
// gsp_blockStatement * transformPlsqlBlockStmt(gsp_context *pContext, 
//												  gsp_blockSqlNode *blockSqlNode);
// gsp_plsqlBasicStmt * transformPlsqlBasicStmt(gsp_context *pContext, gsp_plsqlBasicStmt *bs);
//
// gsp_plsqlCloseStmt * transformPlsqlCloseStmt(gsp_context *pContext, gsp_plsqlCloseStmt *cs);
// gsp_plsqlOpenStmt * transformPlsqlOpenStmt(gsp_context *pContext, gsp_plsqlOpenStmt *os);
//
// gsp_execImmeNode * transformPlsqlExecImmeStmt(gsp_context *pContext, gsp_execImmeNode *ein);
// gsp_plsqlFetchStmt * transformPlsqlFetchStmt(gsp_context *pContext, gsp_plsqlFetchStmt *ein);
//
// gsp_plsqlAssignStmt * transformPlsqlAssignStmt(gsp_context *pContext, gsp_plsqlAssignStmt *as);
//
// gsp_plsqlExitStmt * transformPlsqlExitStmt(gsp_context *pContext, gsp_plsqlExitStmt *es);
//
// gsp_plsqlIfStmt * transformPlsqlIfStmt(gsp_context *pContext, gsp_plsqlIfStmt *is);
//
// gsp_plsqlElsifStmt * transformPlsqlElsifStmt(gsp_context *pContext, gsp_plsqlElsifStmt *eis);
//
// gsp_plsqlForallStmt * transformPlsqlForallStmt(gsp_context *pContext, gsp_plsqlForallStmt *fas);
//
// gsp_plsqlCaseStmt * transformPlsqlCaseStmt(gsp_context *pContext, gsp_plsqlCaseStmt *cs);
//
// gsp_plsqlLoopStmt * transformPlsqlLoopStmt(gsp_context *pContext, gsp_plsqlLoopStmt *ls);
//
// gsp_plsqlReturnStmt * transformPlsqlReturnStmt(gsp_context *pContext, gsp_plsqlReturnStmt *rs);
//
// gsp_plsqlCreateTypeBody * transformPlsqlCreateTypeBodyStmt(gsp_context *pContext, gsp_plsqlCreateTypeBody *ctb);

// oracle nodes end


// sql server nodes
typedef struct gsp_mssql_insertBulkSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_list *columnDefinitionList; /*<! list of gsp_columnDefinition */
}gsp_mssql_insertBulkSqlNode;

typedef struct gsp_mssql_ThrowSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_ThrowSqlNode;

typedef struct gsp_mssql_ReconfigureSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_ReconfigureSqlNode;

typedef struct gsp_mssql_executeAsSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_executeAsSqlNode;

typedef enum EExecType{
	eet_exec_sp,eet_exec_stringCmd,eet_exec_stringCmdLinkServer,
	eet_exec_noExecKeyword 
}EExecType;

typedef struct gsp_mssql_executeSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EExecType execType;
	gsp_objectname *moduleName;
	gsp_objectname *returnStatus;
	gsp_list *parameterList; /*<! list of gsp_mssql_execParameter */
	gsp_list *stringValues; /*<! list of gsp_expr */
}gsp_mssql_executeSqlNode;


typedef struct gsp_mssql_execParameter
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EParameterMode parameterMode;
	gsp_objectname *parameterName;
	gsp_expr *parameterValue;
}gsp_mssql_execParameter;


typedef struct gsp_mssql_dropDbObjectSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	EDBObjectType dbObjectType;
	gsp_list *objectNames; /*<! list of gsp_objectname */
}gsp_mssql_dropDbObjectSqlNode;

typedef enum ESetType{
	est_mstUnknown,est_mstLocalVar,est_mstLocalVarCursor,
	est_mstSetCmd,est_mstXmlMethod,est_mstSybaseLocalVar
}ESetType;

typedef struct gsp_mssql_setSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	ESetType setType;
	gsp_objectname *varName;
	gsp_expr *varExpr;
	gsp_selectSqlNode *selectSqlNode;
	gsp_list *exprList;/*!< list of assignment expre */
}gsp_mssql_setSqlNode;

typedef struct gsp_mssql_beginTranSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *transactionName;
	int distributed;
	int withMark;
	gsp_constant *withMarkDescription;
}gsp_mssql_beginTranSqlNode;

typedef struct gsp_mssql_raiserrorSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *msgs; /*<! list of gsp_expr */
}gsp_mssql_raiserrorSqlNode;

typedef struct gsp_mssql_gotoSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *labelName;
}gsp_mssql_gotoSqlNode;

typedef struct gsp_mssql_labelSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_labelSqlNode;

typedef struct gsp_mssql_killSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_killSqlNode;

typedef struct gsp_mssql_deallocateSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *cursorName;
	int bGlobal;
}gsp_mssql_deallocateSqlNode;

typedef struct gsp_mssql_beginDialogSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *dialogHandle;
	gsp_objectname *initiatorServiceName;
	gsp_objectname *targetServiceName;
}gsp_mssql_beginDialogSqlNode;

typedef struct gsp_mssql_sendOnConversationSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *conversationHandle;
}gsp_mssql_sendOnConversationSqlNode;

typedef struct gsp_mssql_endConversationSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *conversationHandle;
}gsp_mssql_endConversationSqlNode;

typedef struct gsp_mssql_revertSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_revertSqlNode;

typedef struct gsp_mssql_goSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_goSqlNode;

typedef struct gsp_mssql_useSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *databaseName;
}gsp_mssql_useSqlNode;

typedef struct gsp_mssql_printSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *vars; /*<! list of gsp_expr */
}gsp_mssql_printSqlNode;

typedef struct gsp_mssql_computeClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *items; /*<! list of gsp_mssql_computeClauseItem */
}gsp_mssql_computeClause;

typedef struct gsp_mssql_computeClauseItem
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *computeExprList; /*<! list of gsp_mssql_computeExpr */
	gsp_list *exprList; /*<! list of gsp_expr */
}gsp_mssql_computeClauseItem;

typedef struct gsp_mssql_computeExpr
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *expr;
}gsp_mssql_computeExpr;


typedef struct gsp_mssql_containsTable
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_expr *containExpr;
	gsp_expr *searchCondition;
}gsp_mssql_containsTable;

typedef struct gsp_mssql_freeTable
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_expr *containExpr;
	gsp_expr *searchCondition;
}gsp_mssql_freeTable;

typedef struct gsp_mssql_openXML
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_openXML;

typedef struct gsp_mssql_openRowSet
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_mssql_openRowSet;

typedef struct gsp_mssql_openQuery
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *exprList;
}gsp_mssql_openQuery;

typedef struct gsp_mssql_openDatasource
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_constant *providerName;
	gsp_constant *initString;
	gsp_objectname *tableName;
	gsp_list *exprList;
}gsp_mssql_openDatasource;


typedef struct gsp_mssql_tableHint
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *hintName;
	gsp_list *hintNameList; /* list of gsp_expr */
}gsp_mssql_tableHint;

typedef struct gsp_mssql_bulkInsertSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *tableName;
	gsp_constant *datafile;
}gsp_mssql_bulkInsertSqlNode;

typedef struct gsp_mssql_outputClause
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *selectItemList;  /*<! list of gsp_resultColumn */
	gsp_list *selectItemList2; /*<! list of gsp_resultColumn */
	gsp_list *intoColumnList;  /*<! list of gsp_objectname */
	gsp_objectname *tableName;
}gsp_mssql_outputClause;


typedef struct gsp_mssql_updateTextSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *destColumnName;
	gsp_objectname *destTextPtr;
}gsp_mssql_updateTextSqlNode;

// sql server nodes end

// db2 nodes

typedef struct gsp_db2_signal
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_db2_signal;

typedef struct gsp_db2_compoundSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *declareStmts; /*<! list of gsp_sql_statement */
	gsp_list *bodyStmts; /*<! list of gsp_sql_statement */
}gsp_db2_compoundSqlNode;


typedef struct gsp_db2_triggerAction
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *whenCondition;
	gsp_db2_compoundSqlNode *compoundSqlNode;
	gsp_sql_statement *stmt;
}gsp_db2_triggerAction;

typedef struct gsp_db2_callStmtSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *procedureName;
	gsp_list *args;
}gsp_db2_callStmtSqlNode;

typedef struct gsp_db2_forSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *loopName,*cursorName;
	gsp_selectSqlNode *subQueryNode;
	gsp_list *stmts;
}gsp_db2_forSqlNode;


typedef struct gsp_db2_iterateStmtSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *labelName;
}gsp_db2_iterateStmtSqlNode;

typedef struct gsp_db2_leaveStmtSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *labelName;
}gsp_db2_leaveStmtSqlNode;

typedef struct gsp_db2_setSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
}gsp_db2_setSqlNode;

typedef struct gsp_db2_whileSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *stmts;
}gsp_db2_whileSqlNode;

typedef struct gsp_db2_repeatSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_expr *condition;
	gsp_list *stmts;
}gsp_db2_repeatSqlNode;


typedef struct gsp_db2_gotoSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_objectname *labelName;
}gsp_db2_gotoSqlNode;

typedef struct gsp_db2_loopSqlNode
{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;gsp_fragment fragment;
	gsp_list *stmts;
}gsp_db2_loopSqlNode;

// db2 nodes end



/*!
* get original text of node.
* caller must free this text
*/
char * gsp_node_text(gsp_node *node);
void gsp_node_print(gsp_node *node);

gsp_sourcetoken *gsp_node_get_start_token(gsp_node *node);
gsp_sourcetoken *gsp_node_get_end_token(gsp_node *node);

int isSubNode(gsp_node *subnode, gsp_node *node);


/*
 * foreachnode -
 *	  a convenience macro which loops through the parentNode  
 *    which is included as well.
 */
#define foreachnode(node, parentNode)	\
	for ((node) = (parentNode); (((node) != 0)&&(isSubNode((node),(parentNode)))); (node) = (node)->pPrev)


/*!
* set new text of this node, text set by this node will over writed
* if parent node set a new text later, or if parent node already set
* a text, then this set will be ignored
*
* caller must free this text after gsp_node_newtext() was called.
* input parameter *text was \0 terminated UTF-8 encoded string.
*/
void gsp_node_set_text(gsp_node *node, char *text);

/*!
* get node text after modify node or sub node of it.
* caller must free this text
*/
char *gsp_node_newtext(gsp_node *node);

#ifdef __cplusplus
}
#endif

#endif   /* GSP_NODE_H */