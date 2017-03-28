#ifndef GSP_TRANSFORM_H
#define GSP_TRANSFORM_H

#include "gsp_base.h"
#include "gsp_node.h"

#ifdef __cplusplus
extern "C" {
#endif


#define gsp_makeStmt(yyparser, _type_,baseNode) gsp_makeNodeOnNode(yyparser, _type_,baseNode)

void gsp_transformNode(gsp_context *pContext,gsp_node *parseTreeNode);
void gsp_transformAllNodes(gsp_context *pContext,gsp_node *parseTree);
gsp_context *gsp_create_context(gsp_context *parentContext);
void gsp_free_context(gsp_context *pContext);
gsp_sql_statement * transformRawSqlStatement(gsp_context *pContext, 
											 gsp_sql_statement *stmtNode);



//gsp_base_statement *gsp_transformStmt(gsp_context *pContext, gsp_node *parseTree);
// gsp_arrayAccess * transformArrayAccess(gsp_context *pContext, 
//												  gsp_arrayAccess *arrayAccess);
//
// gsp_intervalExpression * transformIntervalExpression(gsp_context *pContext, 
//												  gsp_intervalExpression *intervalExpression);
//
// gsp_caseExpression * transformCaseExpression(gsp_context *pContext, 
//												  gsp_caseExpression *caseExpression);
//
// gsp_functionCall * transformFunctionCall(gsp_context *pContext, 
//												  gsp_functionCall *functionCall);
//
// gsp_resultColumn * transformResultColumn(gsp_context *pContext, 
//												  gsp_resultColumn *resultColumn);
//
// gsp_list * transformResultColumnList(gsp_context *pContext, 
//												  gsp_list *resultColumnList);
//
// gsp_selectStatement * transformSelectStmt(gsp_context *pContext, 
//												  gsp_selectSqlNode *selectNode);
//
// gsp_deleteStatement * transformDeleteStmt(gsp_context *pContext, 
//												  gsp_deleteSqlNode *deleteNode);
//
// gsp_updateStatement * transformUpdateStmt(gsp_context *pContext, 
//												  gsp_updateSqlNode *updateNode);
//
// gsp_insertStatement * transformInsertStmt(gsp_context *pContext, 
//												  gsp_insertSqlNode *insertNode);
//
// gsp_selectStatement * transformSimpleSelectStmt(gsp_context *pContext, 
//												  gsp_selectSqlNode *selectNode);
//
// gsp_createTableStatement * transformCreateTableStmt(gsp_context *pContext, 
//												  gsp_createTableSqlNode *createTableNode);
//
// gsp_createIndexStatement * transformCreateIndexStmt(gsp_context *pContext, 
//												  gsp_createIndexSqlNode *createIndexNode);
//
// gsp_createViewStatement * transformCreateViewStmt(gsp_context *pContext, 
//												  gsp_createViewSqlNode *createViewNode);
//
// gsp_dropTableStatement * transformDropTableStmt(gsp_context *pContext, 
//												  gsp_dropTableSqlNode *dropTableNode);
//
// gsp_dropIndexStatement * transformDropIndexStmt(gsp_context *pContext, 
//												  gsp_dropIndexSqlNode *dropIndexNode);
//
// gsp_dropViewStatement * transformDropViewStmt(gsp_context *pContext, 
//												  gsp_dropViewSqlNode *dropViewNode);
//
// gsp_createSequenceStatement * transformCreateSequenceStmt(gsp_context *pContext, 
//												  gsp_createSequenceSqlNode *createSequenceNode);
//
// gsp_createSynonymStatement * transformCreateSynonymStmt(gsp_context *pContext, 
//												  gsp_createSynonymSqlNode *createSynonymNode);
//
// gsp_createDirectoryStatement * transformCreateDirectoryStmt(gsp_context *pContext, 
//												  gsp_createDirectorySqlNode *createDirectoryNode);
//
// gsp_alterTableStatement * transformAlterTableStmt(gsp_context *pContext, 
//												  gsp_alterTableSqlNode *alterTableNode);
//
// gsp_mergeStatement * transformMergeStmt(gsp_context *pContext, 
//												  gsp_mergeSqlNode *mergeNode);
//
//
// gsp_createProcedureStatement * transformCreateProcedureStmt(gsp_context *pContext, 
//												  gsp_createProcedureSqlNode *createProcedureNode);
//
// gsp_createFunctionStatement * transformCreateFunctionStmt(gsp_context *pContext, 
//												  gsp_createFunctionSqlNode *createFunctionNode);
//
//
// gsp_createTriggerStatement * transformCreateTriggerStmt(gsp_context *pContext, 
//												  gsp_createTriggerSqlNode *createTriggerSqlNode);
//
//
// 
// gsp_parameterDeclaration * transformParameterDeclaration(gsp_context *pContext, 
//												  gsp_parameterDeclaration *parameterDeclarationNode);
//
// gsp_list * transformParameterDeclarationList(gsp_context *pContext, 
//												  gsp_list *parameterDeclarationList);
//
// gsp_exceptionClause * transformExceptionClause(gsp_context *pContext, 
//												  gsp_exceptionClause *exceptionClause);
//
// gsp_exceptionHandler * transformExceptionHandler(gsp_context *pContext, 
//												  gsp_exceptionHandler *exceptionHandler);
//
// gsp_mergeWhenClause * transformMergeWhenClause(gsp_context *pContext, 
//												  gsp_mergeWhenClause *mergeWhenNode);
//
// gsp_insertValuesClause * transformInsertValuesClause(gsp_context *pContext, 
//												  gsp_insertValuesClause *insertValuesClause);
//
// gsp_orderBy * transformOrderByClause(gsp_context *pContext, 
//												  gsp_orderBy *orderByClause);
//
// gsp_join *  gsp_analyzeJoin(gsp_context *pContext,gsp_joinExpr *pJoinExpr,
//						gsp_join *pJoin, int isSub);
//
//
// gsp_table *gsp_analyzeFromTable(gsp_context *pContext,gsp_fromTable *pFromTable);
//
// gsp_expr * transformExpr(gsp_context *pContext, gsp_expr *pExpr);
// gsp_list * transformExprList(gsp_context *pContext, gsp_list *pList);
// gsp_list * transformSqlStatementList(gsp_context *pContext, gsp_list *pList);
// gsp_sql_statement * transformSqlStatement(gsp_context *pContext, gsp_sql_statement *sqlStmt);
//
// gsp_analyticFunction * transformAnalyticFunction(gsp_context *pContext, gsp_analyticFunction *analyticFunction);
// gsp_trimArgument * transformTrimArgument(gsp_context *pContext, gsp_trimArgument *trimArgument);
// gsp_list * transformCTEList(gsp_context *pContext, gsp_list *pList);
// gsp_list * transformInsertIntoValues(gsp_context *pContext, gsp_list *insertIntoValues);
//
//
//// gsp_statementSqlNode * transformStatementSqlNode(gsp_context *pContext, gsp_statementSqlNode *ssNode);
//
// gsp_multiTarget * transformMultiTarget(gsp_context *pContext, gsp_multiTarget *multiTarget);
// gsp_list * transformMultiTargetList(gsp_context *pContext, gsp_list *multiTargetList);
//
// gsp_list * transformTableElementList(gsp_context *pContext, gsp_list *pList);
// gsp_tableElement * transformTableElement(gsp_context *pContext, gsp_tableElement *te);
//
// gsp_list * transformConstraintList(gsp_context *pContext, gsp_list *pList);
// gsp_constraint * transformConstraint(gsp_context *pContext, gsp_constraint *cn);
//
// gsp_columnDefinition * transformColumnDefinition(gsp_context *pContext, gsp_columnDefinition *cd);
//
// gsp_whenClauseItem * transformWhenClauseItem(gsp_context *pContext, gsp_whenClauseItem *wci);


#ifdef __cplusplus
}
#endif

#endif   /* GSP_TRANSFORM_H */