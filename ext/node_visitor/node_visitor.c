#include "linked_list.h"
#include "tree_map.h"
#include "node_visitor.h"
#include "expr_traverse.h"
#include "memento.h"
#include "comparer.h"
#include "gsp_base.h"
#include "gsp_node.h"
#include "cstring.h"

static void _process_expr(SqlTraverser *sqlTraverser, ExprContext *exprContext);
static void _traverseSelectSQL( SqlTraverser *sqlTraverser, gsp_selectStatement *select, NodeContext *container);
static List* _getInfoContainer(SqlTraverser *traverser, gsp_base_statement *stmt, sql_field_type field);
static Map* _getSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt);
static Map* _findSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt);
static NodeContext* _findContext(SqlTraverser *traverser, gsp_node *node);
static void _updateSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt, sql_field_type field, void* value);
static NodeContext* _insertIntoList(SqlTraverser *traverser, void* node, NodeContext *container);
static ExprContext* _insertExprIntoList(SqlTraverser *traverser, gsp_expr* node, NodeContext *container, ESqlClause clauseType);
static List* _getObjectNameReferences(SqlTraverser *traverser, gsp_node* table);
static BOOL _isTableDetermined(SqlTraverser *traverser, gsp_objectname *objectname);
static Map* _getTableInfo(SqlTraverser *traverser, gsp_node *table);
static void _traverseAliasClause(SqlTraverser *sqlTraverser, gsp_aliasClause *alias, NodeContext *parent);
static void _traverseStatement(SqlTraverser *traverser, gsp_sql_statement *stmt, NodeContext *container);
static void _traverseJoin(SqlTraverser *sqlTraverser, gsp_fromTable *join, NodeContext *parent);
static void _traverseDeleteSQL(SqlTraverser *sqlTraverser, gsp_deleteStatement *deleteStmt, NodeContext *container);
static void _traverseInsertSQL(SqlTraverser *sqlTraverser, gsp_insertStatement *insert, NodeContext *container);
static void _traverseUpdateSQL(SqlTraverser *sqlTraverser, gsp_updateStatement *update, NodeContext *container);
static void _traverseColumnDefinition( SqlTraverser * sqlTraverser, gsp_columnDefinition * columnDefinition, NodeContext * parent ) ;
static void _traverseOutputClause(SqlTraverser *sqlTraverser, gsp_mssql_outputClause *outputClause, NodeContext *parent);
static void _traverseIncludeColumns(SqlTraverser *sqlTraverser, gsp_includeColumns *includeColumns, NodeContext *parent);
static void _traverseIsolationClause(SqlTraverser *sqlTraverser, gsp_isolationClause *isolationClause, NodeContext *parent);
static void _traverseTopClause(SqlTraverser *sqlTraverser, gsp_topClause *topClause, NodeContext *parent);
static void _traverseLimitClause(SqlTraverser *sqlTraverser, gsp_limitClause *limitClause, NodeContext *parent);
static void _traverseBlockStmt(SqlTraverser *sqlTraverser, gsp_blockStatement *blockStmt, NodeContext *container);
static void _traverseWhenClauseItem(  SqlTraverser * sqlTraverser, gsp_whenClauseItem * item, NodeContext * parent);
static void _traverseTypeName(SqlTraverser *sqlTraverser, gsp_typename *typeName, NodeContext *parent);
static void _traverseOrderBy(SqlTraverser *sqlTraverser, gsp_orderBy *orderBy, NodeContext *parent);
static void _traverseOrderByItem(SqlTraverser *sqlTraverser, gsp_orderByItem *orderBy, NodeContext *parent);
static void _traverseWindowDef( SqlTraverser * sqlTraverser, gsp_windowDef * windowDef, NodeContext * parent );
static void _traverseTableElement( SqlTraverser * sqlTraverser, gsp_tableElement * tableElement, NodeContext * parent );
static void _traverseConstraint( SqlTraverser * sqlTraverser, gsp_constraint * constraint, NodeContext * parent );

static BOOL _isTableDetermined(SqlTraverser *traverser, gsp_objectname *objectname){
	return !traverser->__unTableDeterminedList->contains(traverser->__unTableDeterminedList, objectname);
}

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

static ETableSource _getTableSource(gsp_node* table){
	if(table->nodeType == t_gsp_table){
		gsp_table *normalTable = (gsp_table*)table;
		return normalTable->tableSource;
	}
	else if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return fromTable->fromtableType;
	}
	return ets_unknown;
}

static gsp_aliasClause* _getTableAlias(gsp_node* table){
	if(table->nodeType == t_gsp_table){
		gsp_table *normalTable = (gsp_table*)table;
		return normalTable->aliasClause;
	}
	else if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return fromTable->aliasClause;
	}
	return NULL;
}

static gsp_selectStatement* _getTableSubQuery(gsp_node* table){
	if(table->nodeType == t_gsp_fromTable){
		gsp_fromTable *fromTable = (gsp_fromTable*)table;
		return (gsp_selectStatement*)fromTable->subQueryNode;
	}
	return NULL;
}


static BOOL _searchColumnReference( SqlTraverser *traverser, gsp_node* table, gsp_objectname *cr )
{
	BOOL ret = FALSE;
	gsp_list *cteColumnRefers;
	Map *map = _getTableInfo(traverser, table);
	if(!map->containsKey(map, (void*)((int)tft_cteColumnRefs))){
		return ret;
	}

	cteColumnRefers=(gsp_list *)map->get(map,  (void*)((int)tft_cteColumnRefs));
	if(cteColumnRefers!=NULL){
		char* crString = gsp_node_text((gsp_node *)cr);
		gsp_listcell *element;
		foreach(element, cteColumnRefers){
			gsp_objectname *referName = (gsp_objectname*)gsp_list_celldata(element);
			char* refreString = gsp_node_text((gsp_node *)referName);
			if (crString!=NULL && refreString!=NULL && compareToIgnoreCase(crString, refreString)== 0 )
			{
				ret = TRUE;
				gsp_free(refreString);
				break;
			}
			else
			{
				gsp_free(refreString);
			}
		}
		gsp_free(crString);
	}
	return ret;
}

static gsp_list* _getStmtCTEList( gsp_base_statement *stmt )
{
	if(stmt->nodeType == t_gsp_selectStatement){
		return ((gsp_selectStatement *)stmt)->cteList;
	}
	if(stmt->nodeType == t_gsp_updateStatement){
		return ((gsp_updateStatement *)stmt)->cteList;
	}
	if(stmt->nodeType == t_gsp_deleteStatement){
		return ((gsp_deleteStatement *)stmt)->cteList;
	}
	if(stmt->nodeType == t_gsp_insertStatement){
		return ((gsp_insertStatement *)stmt)->cteList;
	}
	return NULL;
}

static BOOL _ableToIncludeCTE( ENodeType ent )
{
	return ( ( ent == t_gsp_selectStatement )
		|| ( ent == t_gsp_updateStatement )
		|| ( ent == t_gsp_insertStatement ) || ( ent == t_gsp_deleteStatement ) );
}

static gsp_list* _searchCTEList( SqlTraverser *traverser, gsp_base_statement *stmt )
{
	gsp_base_statement *lcParent;
	NodeContext *parentContext;
	gsp_list *ret = _getStmtCTEList(stmt);
	if ( ret != NULL )
		return ret;
	parentContext =  traverser->getContext(traverser, (gsp_node*)stmt)->parent;
	if(parentContext == NULL)
		lcParent = NULL;
	else
		lcParent = (gsp_base_statement *)parentContext->self;
	while ( lcParent != NULL )
	{
		if ( !( _ableToIncludeCTE( lcParent->nodeType ) ) )
			break;
		ret = _getStmtCTEList(lcParent);
		if ( ret != NULL )
			break;

		parentContext = parentContext->parent;
		if(parentContext == NULL)
			lcParent = NULL;
		else
			lcParent = (gsp_base_statement *)parentContext->self;
	}
	return ret;
}

static gsp_cte* _findCTEByName( SqlTraverser *traverser, gsp_base_statement *stmt, gsp_node *pfromTable )
{
	gsp_list *lcCteList = _searchCTEList(traverser,stmt);
	if ( lcCteList != NULL )
	{
		gsp_listcell *element;
		char* tableNameString = gsp_node_text((gsp_node *)_getTableName(pfromTable));
		foreach(element, lcCteList){
			gsp_cte *lcCte = (gsp_cte*)gsp_list_celldata(element);
			char* cteTableNameString = gsp_node_text((gsp_node *)lcCte->tableName);
			if (cteTableNameString!=NULL && tableNameString!=NULL && compareToIgnoreCase(tableNameString, cteTableNameString ) == 0 )
			{
				gsp_free(cteTableNameString);
				gsp_free(tableNameString);
				return lcCte;
			}
			else{
				gsp_free(cteTableNameString);
			}
		}
		gsp_free(tableNameString);
	}
	return NULL;
}


static void _analyzeFromTable(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_node *pfromTable){
	if(_getTableSource(pfromTable) == ets_objectname){
		gsp_list *lcCteList = _searchCTEList(traverser,stmt );
		if (lcCteList != NULL){
			gsp_listcell *element;
			char* tableNameString = gsp_node_text((gsp_node *)_getTableName(pfromTable));
			foreach(element, lcCteList){
				gsp_cte *lcCte = (gsp_cte*)gsp_list_celldata(element);
				char* cteTableNameString = gsp_node_text((gsp_node *)lcCte->tableName);
				if (cteTableNameString!=NULL && tableNameString!=NULL && compareToIgnoreCase(tableNameString, cteTableNameString ) == 0 )
				{
					Map *tableInfoMap = _getTableInfo(traverser, pfromTable);
					tableInfoMap->put(tableInfoMap, (void*)((int)tft_cteName), (void*)((int)TRUE));
					if( lcCte->columnList!=NULL)
						tableInfoMap->put(tableInfoMap, (void*)((int)tft_cteColumnRefs), lcCte->columnList);
					gsp_free(cteTableNameString);
					break;
				}
				else{
					gsp_free(cteTableNameString);
				}
			}
			gsp_free(tableNameString);
		}
	}
}

static int _checkColumnReferenceInTables(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_objectname *paramTObjectName)
{
	List* tableList;
	char* objTokenString;
	gsp_node* localTTable = NULL;
	int i = -1, j;
	if ( paramTObjectName->objectToken == NULL )
		return -2;

	tableList = _getInfoContainer(traverser, stmt, sft_table);

	objTokenString = (char*)malloc(sizeof(char)*(paramTObjectName->objectToken->nStrLen+1));
	memset(objTokenString, '\0', paramTObjectName->objectToken->nStrLen+1);
	strncpy(objTokenString, paramTObjectName->objectToken->pStr, paramTObjectName->objectToken->nStrLen);

	for ( j = 0; j < tableList->size( tableList ); ++j )
	{
		localTTable = (gsp_node*)tableList->get( tableList, j );
		if ( _getTableAlias(localTTable) != NULL )
		{
			char* tableAliasString = gsp_node_text((gsp_node*)_getTableAlias(localTTable));
			if (compareToIgnoreCase(tableAliasString, objTokenString ) == 0 )
			{
				paramTObjectName->objectToken->dbObjType = edb_table_alias;
				gsp_free(tableAliasString);
				gsp_free(objTokenString);
				return j;
			}
			else{
				gsp_free(tableAliasString);
			}
		}
		else if ( _getTableSource(localTTable) == ets_objectname )
		{
			char* tableNameString = gsp_node_text((gsp_node *)_getTableName(localTTable));
			if (compareToIgnoreCase(tableNameString, objTokenString ) == 0 )
			{
				paramTObjectName->objectToken->dbObjType = edb_table;
				gsp_free(tableNameString);
				gsp_free(objTokenString);
				return j;
			}
			else{
				gsp_free(tableNameString);
			}
		}
	}
	gsp_free(objTokenString);
	return i;
}

static gsp_base_statement* _getParentStmt(SqlTraverser *traverser, gsp_base_statement *stmt){
	NodeContext* entry = traverser->getContext(traverser, (gsp_node*)stmt);
	if(entry->parent == NULL)
		return NULL;
	return (gsp_base_statement*)(entry->parent)->self;
}

static gsp_base_statement* _getTopStatement(SqlTraverser *traverser, gsp_base_statement *stmt) {
	gsp_base_statement *ret;
	ret = _getParentStmt(traverser, stmt);
	if(ret == NULL)
		return stmt;
	while (ret != NULL) {
		gsp_base_statement *parent = _getParentStmt(traverser, ret);
		if(parent!=NULL)
			ret = parent;
		else break;
	}
	return ret;
}

static List* _getSymbolTable(SqlTraverser *traverser, gsp_base_statement *stmt) {
	Map *map = _getSQLInfo(traverser, stmt);
	if(!map->containsKey(map, (void *)((int)sft_symbolTable))){
		List *list = createList();
		map->put(map, (void *)((int)sft_symbolTable), list);
	}
	return (List*)map->get(map, (void *)((int)sft_symbolTable));
}

static void _insertToSymbolTable(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_node* node, EDBObjectType type) {
	SymbolTableItem *item;
	List* list = _getSymbolTable(traverser, _getTopStatement(traverser, stmt));
	item = (SymbolTableItem *)malloc(sizeof(SymbolTableItem));
	item->data = node;
	item->stmt = stmt;
	item->type = type;
	traverser->__entries->add(traverser->__entries, item);
	list->add(list, item);
}

static BOOL _isColumnNameInSelectList(SqlTraverser *traverser, char* pColumn, gsp_selectStatement *pSelect){
	BOOL ret = FALSE;
	if (pSelect->setOperator > eso_none){
		ret = _isColumnNameInSelectList(traverser, pColumn,pSelect->leftStmt);
		if (!ret){
			ret = _isColumnNameInSelectList(traverser, pColumn,pSelect->rightStmt);
		}
	}else{
		if (pSelect->resultColumnList != NULL){ //if it's a db2 value row, then pSelect.getResultColumnList() will be null
			gsp_listcell *cell;
			foreach(cell, pSelect->resultColumnList){
				char* columnExprString;
				gsp_resultColumn *lcColumn = (gsp_resultColumn *)gsp_list_celldata(cell);
				if (lcColumn->aliasClause != NULL){
					char* columnAliasString = gsp_node_text((gsp_node *)lcColumn->aliasClause);
					ret = compareToIgnoreCase(pColumn, columnAliasString) == 0;
					gsp_free(columnAliasString);
				}
				if (ret == TRUE)  break;
				columnExprString = gsp_node_text((gsp_node *)lcColumn->expr);
				ret = compareToIgnoreCase(pColumn, columnExprString) == 0;
				gsp_free(columnExprString);
				if (ret == TRUE)  break;
			}
		}
	}
	return ret;
}

static BOOL _checkNonQualifiedColumnReferenceInSubQueryOfUplevelStmt(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_objectname *crf,BOOL sameLevelOnly){
	BOOL ret = FALSE;
	gsp_base_statement *lcParent = stmt;

	char* crfString = gsp_node_text((gsp_node *)crf);

	while ( lcParent != NULL) {
		List* tableList = _getInfoContainer(traverser, lcParent, sft_table);
		int i;
		for (i=0;i<tableList->size(tableList);i++){
			gsp_node* lcTable = (gsp_node*)tableList->get(tableList, i);
			if (_getTableSource(lcTable) != ets_subquery) {continue;}
			ret = _isColumnNameInSelectList(traverser, crfString,_getTableSubQuery(lcTable));
			if (ret) {break;}
		}
		if (ret) {break;}
		else{
			if (sameLevelOnly){
				lcParent = NULL;
			}else{
				lcParent = _getParentStmt(traverser, lcParent);
			}
		}
	} // while

	gsp_free(crfString);
	return ret;
}

static BOOL _isCTEName( SqlTraverser *traverser, gsp_node *table){
	Map *map = _getTableInfo(traverser, table);
	return (long)map->get(map, (void*)tft_cteName);
}

static BOOL _isBaseTable( SqlTraverser *traverser, gsp_node *table)
{
	int retval = ( _getTableSource(table) == ets_objectname ) ? 1 : 0;

	if ( retval  &&  _isCTEName( traverser, table) )
	{
		retval = FALSE;
	}

	return retval;
}

static BOOL _isParentStmt(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_node *node){
	NodeContext *context = traverser->getContext(traverser, node);
	if(context == NULL)
		return FALSE;
	else{
		gsp_base_statement *temp = (gsp_base_statement *)context->self;
		while(temp!=NULL){
			if(temp == stmt)
				return TRUE;
			else{
				temp = _getParentStmt(traverser, temp);
			}
		}
		return FALSE;
	}
}

static int _locateVariableOrParameter(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_objectname *cr){
	int ret = -1;
	if (traverser->__parser->db != dbvoracle) {
		return ret;
	}
	else{
		SymbolTableItem *item;
		gsp_objectname *objName;
		gsp_objectname *qualifiedName;
		List* symbolTable = _getSymbolTable(traverser, _getTopStatement(traverser,stmt));
		char* crString = gsp_node_text((gsp_node *)cr);
		int i;

		for (i = symbolTable->size(symbolTable) - 1; i >= 0; i--){
			char* nameString;
			char* qualifiedNameString;
			char* fieldName;
			item = (SymbolTableItem*)symbolTable->get(symbolTable, i);
			if(!_isParentStmt(traverser, item->stmt, (gsp_node*)cr))
				continue;
			if (item->data->nodeType == t_gsp_parameterDeclaration){
				objName = ((gsp_parameterDeclaration*)item->data)->parameterName;
			}
			else if(item->data->nodeType == t_gsp_sql_statement && ((gsp_sql_statement*)item->data)->stmtType == sstplsql_vardecl){
				gsp_node *node = (gsp_node *)((gsp_sql_statement*)item->data)->stmt;
				if(node == NULL)
					node = (gsp_node *)((gsp_sql_statement*)item->data)->parseTree;
				if(node == NULL)
					objName = NULL;
				else objName = ((gsp_plsqlVarDeclStmt*)node)->elementName;
			}
			else{
				objName = NULL;
			}
			
			if(objName == NULL){
				continue;
			}

			nameString = gsp_node_text((gsp_node *)objName);
			if(compareToIgnoreCase(crString, nameString) == 0){
				gsp_free(nameString);
				ret = i;
				break;
			}

			if(strchr(crString, '.') == NULL){
				gsp_free(nameString);
				continue;
			}

			if(item->stmt->nodeType== t_gsp_createFunctionStatement){
				qualifiedName = ((gsp_createFunctionStatement*)item->stmt)->functionName;
			}
			else if(item->stmt->nodeType== t_gsp_createProcedureStatement){
				qualifiedName = ((gsp_createProcedureStatement*)item->stmt)->procedureName;
			}
			else if(item->stmt->nodeType== t_gsp_blockStatement){
				qualifiedName = ((gsp_blockStatement*)item->stmt)->label.labelName;
			}
			else{
				gsp_free(nameString);
				continue;
			}

			if(qualifiedName!=NULL){

				qualifiedNameString = gsp_node_text((gsp_node*)qualifiedName);

				fieldName = (char *)malloc((strlen(qualifiedNameString) + strlen(nameString) + 2)*sizeof(char));
				memset(fieldName,'\0', (strlen(qualifiedNameString) + strlen(nameString) + 2)*sizeof(char));
				strcat(fieldName,qualifiedNameString);
				strcat(fieldName,".");
				strcat(fieldName,nameString);


				if (compareToIgnoreCase(crString, fieldName) == 0) {
					ret = i;
				}

				gsp_free(qualifiedNameString);
				gsp_free(fieldName);
			}


			gsp_free(nameString);

			if(ret!=-1){
				break;
			}
		}
		//TODO: not implement : if(ret!=-1)objName.getReferencedObjects().addObjectName(cr);
		gsp_free(crString);
	}
	return ret;
}

static void _linkColumnReferenceToTable(SqlTraverser *traverser, gsp_base_statement *stmt, gsp_objectname *cr, ESqlClause pLocation){
	int ret;
	char* crString;
	gsp_base_statement *topStmt;
	if (cr == NULL) return;
	if (cr->objectType == edb_variable) return;
	if (cr->objectType == edb_column_alias) return;

	crString = gsp_node_text((gsp_node *)cr);

	if (compareToIgnoreCase(crString,"NULL") == 0){
		gsp_free(crString);
		return;
	}

	if (crString[0] == '@')
	{
		cr->objectType = edb_not_object;
		gsp_free(crString);
		return;
	}

	if ((compareToIgnoreCase(crString,"rowid") == 0)
		|| (compareToIgnoreCase(crString,"sysdate") == 0)
		|| (compareToIgnoreCase(crString,"nextval") == 0)
		|| (compareToIgnoreCase(crString,"rownum") == 0)
		|| (compareToIgnoreCase(crString,"level") == 0)
		){
			cr->objectType = edb_not_object;
			gsp_free(crString);
			return;
	}

	if( (startsWithIgnoreCase(crString, "INSERTED") || startsWithIgnoreCase(crString, "DELETED")) && pLocation == esc_output ){
		gsp_fromTable *targetTable = NULL;
		if(stmt->nodeType == t_gsp_deleteStatement){
			targetTable = ((gsp_deleteStatement*)stmt)->targetTableNode;
		}
		else if(stmt->nodeType == t_gsp_insertStatement){
			targetTable = ((gsp_insertStatement*)stmt)->targetTableNode;
		}
		if(targetTable!=NULL){
			List* references = _getObjectNameReferences(traverser, (gsp_node*)targetTable);
			references->add(references, cr);
			gsp_free(crString);
			return;
		}
	}

	topStmt = _getTopStatement(traverser, stmt);

	if ((startsWithIgnoreCase(crString, ":NEW") || startsWithIgnoreCase(crString, ":OLD"))
		&& topStmt->nodeType==t_gsp_createTriggerStatement
		&& traverser->__parser->db == dbvoracle)
	{
		List* tableList = _getInfoContainer(traverser, topStmt, sft_table);
		List* references = _getObjectNameReferences(traverser, (gsp_node*)tableList->get(tableList, 0));
		references->add(references, cr);
		gsp_free(crString);
		return;
	}

	if (_locateVariableOrParameter(traverser, stmt, cr) != -1) {
		gsp_free(crString);
		return;
	}
	ret = _checkColumnReferenceInTables(traverser, stmt, cr);

	if (ret >= 0) {
		List* tableList = _getInfoContainer(traverser, stmt, sft_table);
		gsp_node *lcTable = (gsp_node *)tableList->get(tableList, ret);
		if(_isBaseTable(traverser, lcTable)){
			List* references = _getObjectNameReferences(traverser, lcTable);
			references->add(references, cr);
		}
		else if (_isCTEName(traverser, lcTable)){
			gsp_cte *lccte = _findCTEByName(traverser, stmt, lcTable);
			if (lccte != NULL){
				gsp_objectname *objectName = (gsp_objectname *)malloc(sizeof(gsp_objectname));
				objectName->nodeType = t_gsp_objectname;
				objectName->partToken = cr->partToken;
				objectName->objectToken = NULL;
				objectName->fragment.startToken = cr->partToken;
				objectName->fragment.endToken = cr->partToken;
				traverser->__entries->add(traverser->__entries,objectName);
				_linkColumnReferenceToTable(traverser, (gsp_base_statement *)lccte->selectSqlNode, objectName, pLocation);
			}
		}
		else if (_getTableSource(lcTable) == ets_subquery){
			gsp_selectStatement *subquery = _getTableSubQuery(lcTable);
			if((!subquery->setOperator>eso_none)&&(subquery->resultColumnList->length == 1)){
				gsp_resultColumn *lcColumn = (gsp_resultColumn *)gsp_list_first(subquery->resultColumnList);
				char* columnString = gsp_node_text((gsp_node *)lcColumn);
				if (endsWith(trimString(columnString), "*")){
					BOOL isfound = FALSE;
					List* subqueryTableList = _getInfoContainer(traverser, (gsp_base_statement *)subquery, sft_table);
					int i;
					if (traverser->checkColumn != NULL){
						for(i=0;i<subqueryTableList->size(subqueryTableList);i++){
							char* schemaString;
							char* columnStr = NULL;
							gsp_node *current = (gsp_node *)subqueryTableList->get(subqueryTableList, i);
							char* nameString = gsp_node_text((gsp_node *)_getTableName(current));

							schemaString = (char*)malloc(sizeof(char)*(_getTableName(current)->schemaToken->nStrLen+1));
							memset(schemaString, '\0', _getTableName(current)->schemaToken->nStrLen+1);
							strncpy(schemaString,  _getTableName(current)->schemaToken->pStr, _getTableName(current)->schemaToken->nStrLen);

							if (cr->partToken != NULL){
								columnStr = (char*)malloc(sizeof(char)*(cr->partToken->nStrLen+1));
								memset(columnStr, '\0', cr->partToken->nStrLen+1);
								strncpy(columnStr,  cr->partToken->pStr, cr->partToken->nStrLen);
							}
							if (traverser->checkColumn(schemaString,nameString,columnStr)){
								List* subReferences = _getObjectNameReferences(traverser, current);
								if(!subReferences->contains(subReferences, cr))
									subReferences->add(subReferences, cr);
								isfound = TRUE;
								gsp_free(schemaString);
								gsp_free(nameString);
								if(columnStr!=NULL){
									gsp_free(columnStr);
								}
								break;
							}

							gsp_free(schemaString);
							gsp_free(nameString);
							if(columnStr!=NULL){
								gsp_free(columnStr);
							}
						}
					}

					if (!isfound)
					{
						if(subqueryTableList->size(subqueryTableList) > 1){
							traverser->__unTableDeterminedList->add(traverser->__unTableDeterminedList, cr);
						}
						for(i=0;i<subqueryTableList->size(subqueryTableList);i++){
							gsp_node *current = (gsp_node *)subqueryTableList->get(subqueryTableList, i);
							List* subReferences = _getObjectNameReferences(traverser, current);
							if(!subReferences->contains(subReferences, cr))
								subReferences->add(subReferences, cr);
						}
					}
				}
				gsp_free(columnString);
			}
		}
	}
	else if (ret == -2){
		BOOL isfound = FALSE;
		int i, k;
		List* tableList = _getInfoContainer(traverser, stmt, sft_table);
		for (i=0;i<tableList->size(tableList);i++){
			gsp_node *lcTable =(gsp_node *)tableList->get(tableList, i);
			if (_isCTEName(traverser, lcTable) && _searchColumnReference(traverser, lcTable, cr)){
				List* references = _getObjectNameReferences(traverser, lcTable);
				references->add(references, cr);
				isfound = TRUE;
				break;
			}
		}
		if (!isfound){
			gsp_base_statement *lcParent = _getParentStmt(traverser, stmt);
			while ( lcParent != NULL )
			{
				List* parentTableList;

				if (lcParent->nodeType != t_gsp_selectStatement) {
					break;
				}

				parentTableList = _getInfoContainer(traverser, lcParent, sft_table);
				/*TODO: here is different from the java version. 
						It computes the parent stmt table size after traverse the parent stmt. 
						The order againsts the java version. 
						So maybe it's a bug.
				*/
				for (i=0;i<parentTableList->size(parentTableList);i++){
					gsp_node *lcTable =(gsp_node *)parentTableList->get(parentTableList, i);
					List* references = _getObjectNameReferences(traverser, lcTable);
					if (_getTableSource(lcTable) == ets_objectname) {
						for(k = 0; k< references->size(references);k++){
							gsp_objectname *name = (gsp_objectname *)references->get(references, k);
							if (!traverser->__unTableDeterminedList->contains(traverser->__unTableDeterminedList, name)){
								char* nameString = gsp_node_text((gsp_node *)name);
								if (compareToIgnoreCase(crString, nameString)==0){
									isfound = TRUE;
									gsp_free(nameString);
									break;
								}
								gsp_free(nameString);
							}
						}
						if (isfound) break;
					}
				}

				if (isfound){
					break;
				}else{
					lcParent = _getParentStmt(traverser, lcParent);
				}
			}
		}

		if (!isfound){
			isfound = _checkNonQualifiedColumnReferenceInSubQueryOfUplevelStmt(traverser, stmt, cr, (pLocation == esc_resultColumn) );
		}

		if ((!isfound)&&(tableList->size(tableList) > 0)){
			int candidate = 0, firstCandidate = -1;
			// add this column reference to first non-cte( or cte with column list is null) and non-subquery table
			for (i=0;i<tableList->size(tableList);i++){
				gsp_node *lcTable =(gsp_node *)tableList->get(tableList, i);
				Map *tableInfo = _getTableInfo(traverser, lcTable);
				// no qualified column can't belong to a table with alias, that column must be qualified if it's belong to a table with alias
				//if (this.tables.getTable(i).aliasClause != null) continue;
				if ((!_isCTEName(traverser, lcTable)
					||(_isCTEName(traverser, lcTable) && !tableInfo->containsKey(tableInfo, (void*)((int)tft_cteColumnRefs))))
					&& _getTableSource(lcTable) != ets_subquery)
				{
					candidate++;
					if (firstCandidate == -1) firstCandidate = i;
					if (traverser->checkColumn != NULL){
						char* schemaString;
						char* nameString = gsp_node_text((gsp_node *)_getTableName(lcTable));

						schemaString = (char*)malloc(sizeof(char)*(_getTableName(lcTable)->schemaToken->nStrLen+1));
						memset(schemaString, '\0', _getTableName(lcTable)->schemaToken->nStrLen+1);
						strncpy(schemaString,  _getTableName(lcTable)->schemaToken->pStr, _getTableName(lcTable)->schemaToken->nStrLen);


						if (traverser->checkColumn(schemaString,nameString,crString)){
							List* references = _getObjectNameReferences(traverser, lcTable);
							references->add(references, cr);
							isfound = TRUE;
							gsp_free(schemaString);
							gsp_free(nameString);
							break;
						}
						gsp_free(schemaString);
						gsp_free(nameString);
					}else{
						List* references = _getObjectNameReferences(traverser, lcTable);
						references->add(references, cr);
						if (tableList->size(tableList) > 1){
							traverser->__unTableDeterminedList->add(traverser->__unTableDeterminedList, cr);
						}
						isfound = TRUE;
						break;
					}
				}
			}
			if ((!isfound) && (candidate == 1)){
				gsp_node *candidateTable = (gsp_node *)tableList->get(tableList, firstCandidate);
				List* references = _getObjectNameReferences(traverser, candidateTable);
				references->add(references, cr);
			}
		}
	}
	else if (ret == -1){
		gsp_base_statement *lcParent = _getParentStmt(traverser, stmt);
		while ( lcParent != NULL) {
			ret = _checkColumnReferenceInTables(traverser, lcParent, cr);
			if (ret >= 0){
				List* tableList = _getInfoContainer(traverser, lcParent, sft_table);
				List* references = _getObjectNameReferences(traverser, (gsp_node*)tableList->get(tableList, ret));
				references->add(references, cr);
				break;
			}else{
				lcParent = _getParentStmt(traverser, lcParent);
			}
		} // while
	}
	gsp_free(crString);
}

static Map* _getSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt){
	Map* sqlInfoMap = traverser->__sqlInfoMap;
	if(!sqlInfoMap->containsKey(sqlInfoMap, stmt)){
		Map *map = createObjectMap();
		sqlInfoMap->put(sqlInfoMap, stmt, map);
	}
	return (Map*)sqlInfoMap->get(sqlInfoMap, stmt);
}

static Map* _getTableInfo(SqlTraverser *traverser, gsp_node *table){
	Map* tableInfoMap = traverser->__tableInfoMap;
	if(!tableInfoMap->containsKey(tableInfoMap, table)){
		Map *map = createObjectMap();
		tableInfoMap->put(tableInfoMap, table, map);
	}
	return (Map*)tableInfoMap->get(tableInfoMap, table);
}

static Map* _findSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt){
	Map* sqlInfoMap = traverser->__sqlInfoMap;
	if(sqlInfoMap->containsKey(sqlInfoMap, stmt)){
		return (Map*)sqlInfoMap->get(sqlInfoMap, stmt);
	}
	return NULL;
}

static NodeContext* _findContext(SqlTraverser *traverser, gsp_node *node){
	Map* contextMap = traverser->__contextMap;
	if(contextMap->containsKey(contextMap, node)){
		return (NodeContext*)contextMap->get(contextMap, node);
	}
	return NULL;
}

static List* _getInfoContainer(SqlTraverser *traverser, gsp_base_statement *stmt, sql_field_type field){
	Map *map = _getSQLInfo(traverser, stmt);
	if(!map->containsKey(map, (void *)field)){
		List *list = createList();
		map->put(map, (void *)field, list);
	}
	return (List*)map->get(map, (void *)field);
}

static List* _getObjectNameReferences(SqlTraverser *traverser, gsp_node *table){
	Map *map = _getTableInfo(traverser, table);
	if(!map->containsKey(map, (void *)((int)tft_objectName))){
		List *list = createList();
		map->put(map,  (void *)((int)tft_objectName), list);
	}
	return (List*)map->get(map, (void *)((int)tft_objectName));
}

static void _updateSQLInfo(SqlTraverser *traverser, gsp_base_statement *stmt, sql_field_type field, void* value){
	List *list = _getInfoContainer(traverser, stmt, field);
	if(!list->contains(list, value)){
		list->add(list, value);
	}
}

static NodeContext* _createNodeContext(List* entries, gsp_node *node, NodeContext *container){
	NodeContext *entry = (NodeContext*)malloc(sizeof(NodeContext));
	entry->self = node;
	entry->parent = container;
	entries->add(entries, entry);
	return entry;
}

static ExprContext* _insertExprIntoList(SqlTraverser *traverser, gsp_expr *node, NodeContext *container, ESqlClause clauseType){
	List* nodeList =  traverser->__nodeList;
	List* entries = traverser->__entries;
	Map* contextMap = traverser->__contextMap;
	ExprContext *entry = NULL;
	gsp_base_statement *stmt = (gsp_base_statement *)((NodeContext *)container)->self;

	if(contextMap->containsKey(contextMap, node))
		return (ExprContext*)contextMap->get(contextMap, node);

	entry = (ExprContext*)malloc(sizeof(ExprContext));
	entry->stmt = stmt;
	entry->parent = (NodeContext*)container;
	entry->self = node;
	entry->clauseType = clauseType;
	entries->add(entries, entry);
	if(stmt->nodeType == t_gsp_selectStatement && node->expressionType == eet_simple_object_name){
		_updateSQLInfo(traverser, stmt, sft_field, node);
	}

	nodeList->add(nodeList, node);
	contextMap->put(contextMap, node, entry);
	return entry;
}

static NodeContext* _insertIntoList(SqlTraverser *traverser, void* node, NodeContext *container){
	List* nodeList =  traverser->__nodeList;
	Map* contextMap = traverser->__contextMap;
	if(node==NULL)
		return NULL;
	else if(nodeList->contains(nodeList, node)){
		return (NodeContext *)contextMap->get(contextMap, node);
	}
	else{
		List* entries = traverser->__entries;
		gsp_node *temp = (gsp_node *)node;
		NodeContext *entry = _createNodeContext(entries, temp, container);
		if(temp->nodeType == t_gsp_table){
			gsp_base_statement *stmt = (gsp_base_statement *)container->self;
			_updateSQLInfo(traverser, stmt, sft_table, node);
		}
		else if(temp->nodeType == t_gsp_resultColumn){
			gsp_base_statement *stmt = (gsp_base_statement *)container->self;
			_updateSQLInfo(traverser, stmt, sft_resultColumn, node);
		}
		else if(temp->nodeType == t_gsp_whereClause){
			gsp_base_statement *stmt = (gsp_base_statement *)container->self;
			_updateSQLInfo(traverser, stmt, sft_whereClause, node);
		}
		else if(temp->nodeType == t_gsp_groupBy){
			gsp_base_statement *stmt = (gsp_base_statement *)container->self;
			_updateSQLInfo(traverser, stmt, sft_groupByClause, node);
		}
		else if(temp->nodeType == t_gsp_orderBy){
			gsp_base_statement *stmt = (gsp_base_statement *)container->self;
			_updateSQLInfo(traverser, stmt, sft_orderByClause, node);
		}
		nodeList->add(nodeList, node);
		contextMap->put(contextMap, node, entry);
		return entry;
	}
}

static void _traverseTrimArgument(SqlTraverser *sqlTraverser, gsp_trimArgument *args, NodeContext *parent, ESqlClause clause){
	gsp_expr *expr;
	if(args == NULL)
		return;
	_insertIntoList( sqlTraverser, args, parent);
	expr = args->stringExpression;
	if(expr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
	}
	expr = args->trimCharacter;
	if(expr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
	}
}

static void _traverseFunctionCall(SqlTraverser *sqlTraverser, gsp_functionCall *func, NodeContext* parent, ESqlClause clause){
	gsp_listcell *element;
	gsp_expr *expr;
	if(func==NULL)
		return;
	_insertIntoList( sqlTraverser, func, parent);

	_insertIntoList( sqlTraverser, func->functionName, parent);

	if ( func->functionType == eft_trim ){
		gsp_trimArgument *args = func->trimArgument;
		_traverseTrimArgument(sqlTraverser, args, parent, clause);
	}

	expr = func->expr1;
	if(expr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
	}

	expr = func->expr2;
	if(expr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
	}

	expr = func->expr3;
	if(expr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
	}

	if(func->Args!=NULL){
		foreach(element, func->Args){
			expr = (gsp_expr *)gsp_list_celldata(element);
			if(expr!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
			}
		}
	}

	if(func->analyticFunction!=NULL){
		if(func->analyticFunction->partitionBy_ExprList!=NULL){
			foreach(element, func->analyticFunction->partitionBy_ExprList){
				expr = (gsp_expr *)gsp_list_celldata(element);
				if(expr!=NULL){
					_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, clause));
				}
			}
		}
		if(func->analyticFunction->orderBy!=NULL && func->analyticFunction->orderBy->items!=NULL){
			foreach(element, func->analyticFunction->orderBy->items){
				gsp_orderByItem *orderBy = (gsp_orderByItem *)gsp_list_celldata(element);
				_insertIntoList( sqlTraverser, orderBy, parent);
				if(orderBy->sortKey!=NULL){
					_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, orderBy->sortKey, parent, clause));
				}
			}
		}
		if(func->analyticFunction->keepDenseRankClause!=NULL
			&& func->analyticFunction->keepDenseRankClause->orderBy!=NULL
			&& func->analyticFunction->keepDenseRankClause->orderBy->items!=NULL){
				foreach(element, func->analyticFunction->keepDenseRankClause->orderBy->items){
					gsp_orderByItem *orderBy = (gsp_orderByItem *)gsp_list_celldata(element);
					_insertIntoList( sqlTraverser, orderBy, parent);
					if(orderBy->sortKey!=NULL){
						_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, orderBy->sortKey, parent, clause));
					}
				}
		}
	}

	if(func->dataTypename!=NULL){
		_traverseTypeName(sqlTraverser, func->dataTypename, parent);
	}

	if(func->sortClause!=NULL){
		_traverseOrderBy(sqlTraverser, func->sortClause, parent);
	}

	if(func->sortList!=NULL){
		foreach(element, func->sortList){
			gsp_orderByItem *orderBy = (gsp_orderByItem *)gsp_list_celldata(element);
			_traverseOrderByItem(sqlTraverser, orderBy, parent);
		}
	}

	if(func->windowDef!=NULL){
		_traverseWindowDef(sqlTraverser, func->windowDef, parent);
	}

	if(func->trimArgument!=NULL){
		_traverseTrimArgument(sqlTraverser, func->trimArgument, parent, clause);
	}
}

static  gsp_walking_result _exprVisit(ExprTraverser *traverser, gsp_expr *pExpr, BOOL isLeafNode){
	gsp_listcell *element;
	SqlTraverser *sqlTraverser = (SqlTraverser *)traverser->context;
	ExprContext* context = (ExprContext*)sqlTraverser->__context;

	_insertExprIntoList(sqlTraverser, pExpr, context->parent, context->clauseType);

	if(pExpr->objectOperand!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->objectOperand)){
			_insertIntoList(sqlTraverser, pExpr->objectOperand, context->parent);
		}
	}

	if(pExpr->expressionType == eet_function){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->functionCall)){
			_traverseFunctionCall(sqlTraverser, pExpr->functionCall,context->parent, context->clauseType);
		}
	}
	if(pExpr->subQueryNode != NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->subQueryNode)){
			_traverseSelectSQL( sqlTraverser, pExpr->subQueryNode, context->parent);
		}
	}

	if(pExpr->betweenOperand!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->betweenOperand)){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->betweenOperand, context->parent, context->clauseType));
		}
	}

	if(pExpr->leftOperand!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->leftOperand)){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->leftOperand, context->parent, context->clauseType));
		}
	}

	if(pExpr->rightOperand!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->rightOperand)){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->rightOperand, context->parent, context->clauseType));
		}
	}

	if(pExpr->exprList!=NULL){
		foreach(element, pExpr->exprList){
			gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
			if(!_findContext(sqlTraverser, (gsp_node*)expr)){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, context->parent, context->clauseType));
			}
		}
	}

	if(pExpr->likeEscapeOperand!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->likeEscapeOperand)){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->likeEscapeOperand, context->parent, context->clauseType));
		}
	}

	if(pExpr->intervalExpression!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->intervalExpression)){
			_insertIntoList( sqlTraverser, pExpr->intervalExpression, context->parent);
		}
	}

	if(pExpr->caseExpression!=NULL){
		if(!_findContext(sqlTraverser, (gsp_node*)pExpr->caseExpression)){
			_insertIntoList( sqlTraverser, pExpr->caseExpression, context->parent);

			if(pExpr->caseExpression->input_expr!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->caseExpression->input_expr, context->parent, context->clauseType));
			}
			if(pExpr->caseExpression->else_expr!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, pExpr->caseExpression->else_expr, context->parent, context->clauseType));
			}
			if(pExpr->caseExpression->whenClauseItemList!=NULL){
				foreach(element, pExpr->caseExpression->whenClauseItemList){
					gsp_whenClauseItem *when = (gsp_whenClauseItem *)gsp_list_celldata(element);
					_traverseWhenClauseItem(sqlTraverser, when, context->parent);
				}
			}
		}
	}
	return gsp_walking_continue;
}

static void _process_expr( SqlTraverser *sqlTraverser, ExprContext* context){
	ExprTraverser traverser;
	traverser.exprVisit = _exprVisit;
	traverser.context = sqlTraverser;
	sqlTraverser->__context = (NodeContext*)context;
	preOrderTraverse(&traverser, context->self);
}

static void _traverseTable(SqlTraverser *sqlTraverser, gsp_table *table, NodeContext *parent){
	_insertIntoList( sqlTraverser, table, parent);
	if(table->tableExpr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, table->tableExpr, parent, esc_join));
	}
	if(table->tableName!=NULL){
		_insertIntoList( sqlTraverser, table->tableName, parent);
	}
	_traverseAliasClause(sqlTraverser, table->aliasClause, parent);
}

static void _traverseResultColumn(SqlTraverser *sqlTraverser, gsp_resultColumn *column, NodeContext *parent){
	if(column == NULL)
		return;
	_insertIntoList( sqlTraverser, column, parent);
	if(column->expr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, column->expr, parent, esc_resultColumn));
	}
	if(column->aliasClause!=NULL)
		_traverseAliasClause(sqlTraverser,column->aliasClause, parent);
}

static void _traverseMultiTarget(SqlTraverser *sqlTraverser, gsp_multiTarget *target, NodeContext *parent){
	gsp_listcell *element;
	_insertIntoList( sqlTraverser,  target, parent);
	if(target->resultColumnList!=NULL){
		foreach(element, target->resultColumnList){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
			_traverseResultColumn(sqlTraverser, column, parent);
		}
	}
	//TODO: How can I convert a gsp_selectSqlNode to a gsp_selectStatement
	if(target->subQueryNode!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)target->subQueryNode, parent);
	}
}

static void _traverseCTEItem(SqlTraverser *sqlTraverser, gsp_cte *cteItem, NodeContext *parent){
	gsp_listcell *cell;
	_insertIntoList( sqlTraverser,  cteItem, parent);
	if(cteItem->selectSqlNode!=NULL){
		_traverseSelectSQL( sqlTraverser, (gsp_selectStatement*)cteItem->selectSqlNode, parent);
	}
	if(cteItem->deleteSqlNode!=NULL){
		_traverseDeleteSQL( sqlTraverser, (gsp_deleteStatement*)cteItem->deleteSqlNode, parent);
	}
	if(cteItem->insertSqlNode!=NULL){
		_traverseInsertSQL( sqlTraverser, (gsp_insertStatement*)cteItem->insertSqlNode, parent);
	}
	if(cteItem->updateSqlNode!=NULL){
		_traverseUpdateSQL( sqlTraverser, (gsp_updateStatement*)cteItem->updateSqlNode, parent);
	}
	if(cteItem->tableName!=NULL){
		_insertIntoList( sqlTraverser, cteItem->tableName, parent);
	}
	if(cteItem->columnList){
		foreach(cell, cteItem->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			_insertIntoList( sqlTraverser,  column, parent);
		}
	}
}

static void _traverseAliasClause(SqlTraverser *sqlTraverser, gsp_aliasClause *alias, NodeContext *parent){
	if(alias == NULL)
		return;
	_insertIntoList( sqlTraverser,  alias, parent);
	if(alias->aliasName!=NULL){
		_insertIntoList( sqlTraverser,  alias->aliasName, parent);
	}
}

static void _traverseJoinExpr(SqlTraverser *sqlTraverser, gsp_joinExpr *joinExpr, NodeContext *parent){
	gsp_listcell *element;
	if(joinExpr == NULL)
		return;
	_insertIntoList( sqlTraverser, joinExpr, parent);

	if(joinExpr->onCondition!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, joinExpr->onCondition, parent, esc_joinCondition));
	}
	if(joinExpr->usingColumns!=NULL){
		foreach(element, joinExpr->usingColumns){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser,  column, parent);
		}
	}
	_traverseAliasClause(sqlTraverser, joinExpr->aliasClause, parent);

	if(joinExpr->leftOperand!=NULL){
		gsp_base_statement *stmt = (gsp_base_statement *)parent->self;
		_traverseJoin(sqlTraverser, joinExpr->leftOperand, parent);
		_updateSQLInfo(sqlTraverser, stmt, sft_table, joinExpr->leftOperand);
	}

	if(joinExpr->rightOperand!=NULL){
		gsp_base_statement *stmt = (gsp_base_statement *)parent->self;
		_traverseJoin(sqlTraverser, joinExpr->rightOperand, parent);
		_updateSQLInfo(sqlTraverser, stmt, sft_table, joinExpr->rightOperand);
	}

}

static void _traverseTableHint(SqlTraverser *sqlTraverser, gsp_mssql_tableHint *tableHint, NodeContext *parent){
	gsp_listcell *element;
	if(tableHint == NULL)
		return;
	_insertIntoList( sqlTraverser, tableHint, parent);
	_insertIntoList( sqlTraverser, tableHint->hintName, parent);

	if(tableHint->hintNameList!=NULL){
		foreach(element, tableHint->hintNameList){
			gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
		}
	}
}

static void _traverseContainsTable( SqlTraverser * sqlTraverser, struct gsp_mssql_containsTable * containsTable, NodeContext * parent ) 
{
	if(containsTable == NULL)
		return;
	_insertIntoList( sqlTraverser, containsTable, parent);
	_insertIntoList( sqlTraverser, containsTable->tableName, parent);

	if(containsTable->containExpr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, containsTable->containExpr, parent, esc_joinCondition));
	}

	if(containsTable->searchCondition!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, containsTable->searchCondition, parent, esc_joinCondition));
	}
}

static void _traverseDataChangeTable( SqlTraverser * sqlTraverser, gsp_dataChangeTable * dataChangeTable, NodeContext * parent ) 
{
	if(dataChangeTable == NULL)
		return;
	_insertIntoList( sqlTraverser, dataChangeTable, parent);
	
	if(dataChangeTable->stmtNode!=NULL){
		_insertIntoList( sqlTraverser, dataChangeTable->stmtNode, parent);

		if(dataChangeTable->stmtNode->nodeType == t_gsp_deleteSqlNode || dataChangeTable->stmtNode->nodeType == t_gsp_deleteStatement){
			_traverseDeleteSQL(sqlTraverser, (gsp_deleteStatement*)dataChangeTable->stmtNode, parent);
		}
		else if(dataChangeTable->stmtNode->nodeType == t_gsp_insertSqlNode || dataChangeTable->stmtNode->nodeType == t_gsp_insertStatement){
			_traverseInsertSQL(sqlTraverser, (gsp_insertStatement*)dataChangeTable->stmtNode, parent);
		}
		else if(dataChangeTable->stmtNode->nodeType == t_gsp_updateSqlNode || dataChangeTable->stmtNode->nodeType == t_gsp_updateStatement){
			_traverseUpdateSQL(sqlTraverser, (gsp_updateStatement*)dataChangeTable->stmtNode, parent);
		}
	}
}

static void _traverseFlashback( SqlTraverser * sqlTraverser, struct gsp_flashback * flashback, NodeContext * parent ) 
{
	if(flashback == NULL)
		return;
	_insertIntoList( sqlTraverser, flashback, parent);
}

static void _traverseFreeTable( SqlTraverser * sqlTraverser, struct gsp_mssql_freeTable * freeTable, NodeContext * parent ) 
{
	if(freeTable == NULL)
		return;
	_insertIntoList( sqlTraverser, freeTable, parent);
	_insertIntoList( sqlTraverser, freeTable->tableName, parent);

	if(freeTable->containExpr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, freeTable->containExpr, parent, esc_joinCondition));
	}

	if(freeTable->searchCondition!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, freeTable->searchCondition, parent, esc_joinCondition));
	}
}

static void _traverseOpenDatasource( SqlTraverser * sqlTraverser, struct gsp_mssql_openDatasource * openDatasource, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(openDatasource == NULL)
		return;
	_insertIntoList( sqlTraverser, openDatasource, parent);
	_insertIntoList( sqlTraverser, openDatasource->tableName, parent);
	_insertIntoList( sqlTraverser, openDatasource->providerName, parent);
	_insertIntoList( sqlTraverser, openDatasource->initString, parent);

	if(openDatasource->exprList!=NULL){
		foreach(element, openDatasource->exprList){
			gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_joinCondition));
		}
	}
}

static void _traverseOpenQuery( SqlTraverser * sqlTraverser, struct gsp_mssql_openQuery * openQuery, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(openQuery == NULL)
		return;
	_insertIntoList( sqlTraverser, openQuery, parent);

	if(openQuery->exprList!=NULL){
		foreach(element, openQuery->exprList){
			gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_joinCondition));
		}
	}
}

static void _traverseOpenRowSet( SqlTraverser * sqlTraverser, struct gsp_mssql_openRowSet * openRowSet, NodeContext * parent ) 
{
	if(openRowSet == NULL)
		return;
	_insertIntoList( sqlTraverser, openRowSet, parent);
}

static void _traverseOpenXML( SqlTraverser * sqlTraverser, struct gsp_mssql_openXML * openXML, NodeContext * parent ) 
{
	if(openXML == NULL)
		return;
	_insertIntoList( sqlTraverser, openXML, parent);
}

static void _traverseInformixOuterClause( SqlTraverser * sqlTraverser, struct gsp_informixOuterClause * outerClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(outerClause == NULL)
		return;
	_insertIntoList( sqlTraverser, outerClause, parent);

	if(outerClause->fromTable!=NULL){
		_traverseJoin(sqlTraverser,outerClause->fromTable, parent);
	}

	if(outerClause->fromTableList!=NULL){
		foreach(element, outerClause->fromTableList){
			gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_joinCondition));
		}
	}
}

static void _traversePivotClause( SqlTraverser * sqlTraverser, struct gsp_pivotClause * pivotClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(pivotClause == NULL)
		return;
	_insertIntoList( sqlTraverser, pivotClause, parent);

	if(pivotClause->aggregationFunction!=NULL){
		_traverseFunctionCall(sqlTraverser, pivotClause->aggregationFunction, parent, esc_joinCondition);
	}

	if(pivotClause->aliasClause!=NULL){
		_traverseAliasClause(sqlTraverser, pivotClause->aliasClause, parent);
	}

	if(pivotClause->inResultList!=NULL){
		foreach(element, pivotClause->inResultList){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
			_traverseResultColumn(sqlTraverser, column, parent);
		}
	}

	if(pivotClause->privotColumnList!=NULL){
		foreach(element, pivotClause->privotColumnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, column, parent);
		}
	}

	_insertIntoList( sqlTraverser, pivotClause->privotColumn, parent);
}

static void _traversePxGranule( SqlTraverser * sqlTraverser, struct gsp_pxGranule * pxGranule, NodeContext * parent ) 
{
	if(pxGranule == NULL)
		return;
	_insertIntoList( sqlTraverser, pxGranule, parent);
}

static void _traverseUnPivotClause( SqlTraverser * sqlTraverser, struct gsp_unPivotClause * unPivotClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(unPivotClause == NULL)
		return;
	_insertIntoList( sqlTraverser, unPivotClause, parent);

	if(unPivotClause->aggregationFunction!=NULL){
		_traverseFunctionCall(sqlTraverser, unPivotClause->aggregationFunction, parent, esc_joinCondition);
	}

	if(unPivotClause->aliasClause!=NULL){
		_traverseAliasClause(sqlTraverser, unPivotClause->aliasClause, parent);
	}

	if(unPivotClause->inResultList!=NULL){
		foreach(element, unPivotClause->inResultList){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
			_traverseResultColumn(sqlTraverser, column, parent);
		}
	}

	if(unPivotClause->privotColumnList!=NULL){
		foreach(element, unPivotClause->privotColumnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, column, parent);
		}
	}

	_insertIntoList( sqlTraverser, unPivotClause->privotColumn, parent);
	_insertIntoList( sqlTraverser, unPivotClause->valueColumn, parent);
}

static void _traverseTableSample( SqlTraverser * sqlTraverser, struct gsp_tableSample * tableSample, NodeContext * parent ) 
{
	if(tableSample == NULL)
		return;
	_insertIntoList( sqlTraverser, tableSample, parent);
}

static void _traverseJoin(SqlTraverser *sqlTraverser, gsp_fromTable *join, NodeContext *parent){
	gsp_listcell *element;
	if(join == NULL)
		return;
	_insertIntoList( sqlTraverser, join, parent);
	_insertIntoList( sqlTraverser, join->tableName, parent);
	if(join->tableExpr!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, join->tableExpr, parent, esc_joinCondition));
	}
	if(join->joinExpr!=NULL){
		_traverseJoinExpr(sqlTraverser, join->joinExpr, parent);
	}
	else{
		gsp_base_statement *stmt = (gsp_base_statement *)parent->self;
		_updateSQLInfo(sqlTraverser, stmt, sft_table, join);
	}
	_traverseAliasClause(sqlTraverser, join->aliasClause, parent);
	_traverseFunctionCall(sqlTraverser, join->functionCall, parent, esc_joinCondition);

	if(join->tableHints!=NULL){
		foreach(element, join->tableHints){
			gsp_mssql_tableHint *tableHint = (gsp_mssql_tableHint *)gsp_list_celldata(element);
			_traverseTableHint( sqlTraverser,  tableHint, parent);
		}
	}

	if(join->rowList!=NULL){
		foreach(element, join->rowList){
			gsp_multiTarget *target = (gsp_multiTarget *)gsp_list_celldata(element);
			_traverseMultiTarget( sqlTraverser,  target, parent);
		}
	}

	if(join->subQueryNode!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)join->subQueryNode, parent);
	}

	if(join->containsTable!=NULL){
		_traverseContainsTable(sqlTraverser, join->containsTable, parent);
	}

	if(join->dataChangeTable!=NULL){
		_traverseDataChangeTable(sqlTraverser, join->dataChangeTable, parent);
	}

	if(join->flashback!=NULL){
		_traverseFlashback(sqlTraverser, join->flashback, parent);
	}

	if(join->freeTable!=NULL){
		_traverseFreeTable(sqlTraverser, join->freeTable, parent);
	}

	if(join->openDatasource!=NULL){
		_traverseOpenDatasource(sqlTraverser, join->openDatasource, parent);
	}

	if(join->openQuery!=NULL){
		_traverseOpenQuery(sqlTraverser, join->openQuery, parent);
	}

	if(join->openRowSet!=NULL){
		_traverseOpenRowSet(sqlTraverser, join->openRowSet, parent);
	}

	if(join->openXML!=NULL){
		_traverseOpenXML(sqlTraverser, join->openXML, parent);
	}

	if(join->outerClause!=NULL){
		_traverseInformixOuterClause(sqlTraverser, join->outerClause, parent);
	}

	if(join->pivotClause!=NULL){
		_traversePivotClause(sqlTraverser, join->pivotClause, parent);
	}

	if(join->pxGranule!=NULL){
		_traversePxGranule(sqlTraverser, join->pxGranule, parent);
	}

	if(join->unPivotClause!=NULL){
		_traverseUnPivotClause(sqlTraverser, join->unPivotClause, parent);
	}

	if(join->tableSample!=NULL){
		_traverseTableSample(sqlTraverser, join->tableSample, parent);
	}
	
}

static void _traverseOrderByItem(SqlTraverser *sqlTraverser, gsp_orderByItem *orderByItem, NodeContext *parent){
	if(orderByItem!=NULL){
		_insertIntoList( sqlTraverser, orderByItem, parent);
		if(orderByItem->sortKey!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, orderByItem->sortKey, parent, esc_orderby));
		}
	}
}

static void _traverseOrderBy(SqlTraverser *sqlTraverser, gsp_orderBy *orderBy, NodeContext *parent){
	gsp_listcell *element;
	gsp_list *sortList;
	if(orderBy!=NULL){
		_insertIntoList( sqlTraverser,  orderBy, parent);
		sortList = orderBy->items;
		foreach(element, sortList){
			gsp_orderByItem *orderByItem = (gsp_orderByItem *)gsp_list_celldata(element);
			_traverseOrderByItem(sqlTraverser, orderByItem, parent);
		}
	}
}


static void _traverseValueRowItem( SqlTraverser * sqlTraverser, gsp_valueRowItem * valueRowItem, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(valueRowItem!=NULL){
		_insertIntoList( sqlTraverser,  valueRowItem, parent);
		if(valueRowItem->exprList!=NULL){
			foreach(element, valueRowItem->exprList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
			}
		}
		if(valueRowItem->expr!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, valueRowItem->expr, parent, esc_unknown));
		}
	}
}

static void _traverseValueClause( SqlTraverser * sqlTraverser, gsp_valueClause * valueClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(valueClause!=NULL){
		_insertIntoList( sqlTraverser,  valueClause, parent);
		if(valueClause->nameList!=NULL){
			foreach(element, valueClause->nameList){
				gsp_objectname *name = (gsp_objectname *)gsp_list_celldata(element);
				_insertIntoList( sqlTraverser, name, parent);	
			}
		}
		if(valueClause->valueList!=NULL){
			foreach(element, valueClause->valueList){
				gsp_valueRowItem *valueRowItem = (gsp_valueRowItem *)gsp_list_celldata(element);
				_traverseValueRowItem( sqlTraverser, valueRowItem, parent);	
			}
		}
	}
}

static void _traverseComputeExpr( SqlTraverser * sqlTraverser, gsp_mssql_computeExpr * computeExpr, NodeContext * parent ) 
{
	if(computeExpr!=NULL){
		_insertIntoList( sqlTraverser,  computeExpr, parent);
		if(computeExpr->expr!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, computeExpr->expr, parent, esc_unknown));
		}
	}
}

static void _traverseComputeClauseItem( SqlTraverser * sqlTraverser, gsp_mssql_computeClauseItem * item, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(item!=NULL){
		_insertIntoList( sqlTraverser,  item, parent);
		if(item->exprList!=NULL){
			foreach(element, item->exprList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
			}
		}
		if(item->computeExprList!=NULL){
			foreach(element, item->computeExprList){
				gsp_mssql_computeExpr *expr = (gsp_mssql_computeExpr *)gsp_list_celldata(element);
				_traverseComputeExpr( sqlTraverser, expr, parent);	
			}
		}
	}
}

static void _traverseMssqlComputeClause( SqlTraverser * sqlTraverser, struct gsp_mssql_computeClause * computeClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(computeClause!=NULL){
		_insertIntoList( sqlTraverser,  computeClause, parent);
		if(computeClause->items!=NULL){
			foreach(element, computeClause->items){
				gsp_mssql_computeClauseItem *item = (gsp_mssql_computeClauseItem *)gsp_list_celldata(element);
				_traverseComputeClauseItem( sqlTraverser, item, parent);	
			}
		}
	}
}

static void _traverseTeradataExpandOnClause( SqlTraverser * sqlTraverser, struct gsp_expandOnClause * expandOnClause, NodeContext * parent ) 
{
	if(expandOnClause!=NULL){
		_insertIntoList( sqlTraverser,  expandOnClause, parent);
		if(expandOnClause->columnAlias!=NULL){
			_traverseAliasClause(sqlTraverser, expandOnClause->columnAlias, parent);
		}
		if(expandOnClause->expr!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expandOnClause->expr, parent, esc_unknown));
		}
		if(expandOnClause->periodExpr!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expandOnClause->periodExpr, parent, esc_unknown));
		}
	}
}

static void _traverseFetchFirstClause( SqlTraverser * sqlTraverser, gsp_fetchFirstClause * fetchFirstClause, NodeContext * parent ) 
{
	if(fetchFirstClause!=NULL){
		_insertIntoList( sqlTraverser,  fetchFirstClause, parent);
	}
}

static void _traverseIntoTableClause( SqlTraverser * sqlTraverser, struct gsp_intoTableClause * intoTableClause, NodeContext * parent ) 
{
	if(intoTableClause!=NULL){
		_insertIntoList( sqlTraverser,  intoTableClause, parent);
		if(intoTableClause->tableName!=NULL){
			_insertIntoList( sqlTraverser,  intoTableClause->tableName, parent);
		}
	}
}

static void _traverseLockingClause( SqlTraverser * sqlTraverser, struct gsp_lockingClause * lockingClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(lockingClause!=NULL){
		_insertIntoList( sqlTraverser,  lockingClause, parent);
		if(lockingClause->lockedObjects!=NULL){
			foreach(element, lockingClause->lockedObjects){
				gsp_objectname *objectName = (gsp_objectname *)gsp_list_celldata(element);
				_insertIntoList( sqlTraverser,  objectName, parent);
			}
		}
	}
}

static void _traverseOptimizeForClause( SqlTraverser * sqlTraverser, gsp_optimizeForClause * optimizeForClause, NodeContext * parent ) 
{
	if(optimizeForClause!=NULL){
		_insertIntoList( sqlTraverser,  optimizeForClause, parent);
	}
}

static void _traverseQualifyClause( SqlTraverser * sqlTraverser, struct gsp_qualifyClause * qualifyClause, NodeContext * parent ) 
{
	if(qualifyClause!=NULL){
		_insertIntoList( sqlTraverser,  qualifyClause, parent);
		if(qualifyClause->expr!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, qualifyClause->expr, parent, esc_unknown));
		}
	}
}

static void _traverseSampleClause( SqlTraverser * sqlTraverser, struct gsp_sampleClause * sampleClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(sampleClause!=NULL){
		_insertIntoList( sqlTraverser,  sampleClause, parent);
		if(sampleClause->countFractionDescriptionList!=NULL){
			foreach(element, sampleClause->countFractionDescriptionList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
			}
		}
		if(sampleClause->whenThenList!=NULL){
			foreach(element, sampleClause->whenThenList){
				gsp_whenClauseItem *item = (gsp_whenClauseItem *)gsp_list_celldata(element);
				_traverseWhenClauseItem(sqlTraverser, item, parent);
			}
		}
	}
}

static void _traversePartitionClause( SqlTraverser * sqlTraverser, gsp_partitionClause * partitionClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(partitionClause!=NULL){
		_insertIntoList( sqlTraverser,  partitionClause, parent);
		if(partitionClause->exprList!=NULL){
			foreach(element, partitionClause->exprList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
			}
		}
	}
}

static void _traverseWindowDef( SqlTraverser * sqlTraverser, gsp_windowDef * windowDef, NodeContext * parent ) 
{
	if(windowDef!=NULL){
		_insertIntoList( sqlTraverser,  windowDef, parent);
		if(windowDef->endOffset!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, windowDef->endOffset, parent, esc_unknown));
		}
		if(windowDef->frameClause!=NULL){
			_traverseWindowDef(sqlTraverser, windowDef->frameClause, parent);
		}
		if(windowDef->partitionClause!=NULL){
			_traversePartitionClause(sqlTraverser, windowDef->partitionClause, parent);
		}
		if(windowDef->referenceName!=NULL){
			_insertIntoList( sqlTraverser,  windowDef->referenceName, parent);
		}
		if(windowDef->sortClause!=NULL){
			_traverseOrderBy( sqlTraverser,  windowDef->sortClause, parent);
		}
		if(windowDef->startOffset!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, windowDef->startOffset, parent, esc_unknown));
		}
		if(windowDef->windowName!=NULL){
			_insertIntoList( sqlTraverser,  windowDef->windowName, parent);
		}
	}
}

static void _traverseWindowClause( SqlTraverser * sqlTraverser, struct gsp_windowClause * windowClause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(windowClause!=NULL){
		_insertIntoList( sqlTraverser,  windowClause, parent);
		if(windowClause->windownsDefs!=NULL){
			foreach(element, windowClause->windownsDefs){
				gsp_windowDef *windowDef = (gsp_windowDef *)gsp_list_celldata(element);
				_traverseWindowDef(sqlTraverser, windowDef, parent);
			}
		}
	}
}

static void _traverseTeradataWithClause( SqlTraverser * sqlTraverser, gsp_teradataWithClause * clause, NodeContext * parent ) 
{
	gsp_listcell *element;
	if(clause!=NULL){
		_insertIntoList( sqlTraverser,  clause, parent);
		if(clause->exprList!=NULL){
			foreach(element, clause->exprList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
			}
		}
		if(clause->byList!=NULL){
			foreach(element, clause->byList){
				gsp_orderByItem *item = (gsp_orderByItem *)gsp_list_celldata(element);
				_traverseOrderByItem(sqlTraverser, item, parent);
			}
		}
	}
}

static void _traverseSelectSQL( SqlTraverser *sqlTraverser, gsp_selectStatement *select, NodeContext *container){
	gsp_listcell *cell, *element;
	gsp_list *fields, *joins, *groupByList, *sortList;
	NodeContext *parent;
	parent = _insertIntoList( sqlTraverser,  select, container);

	fields = select->resultColumnList;
	if(fields!=NULL){
		foreach(cell, fields){
			gsp_resultColumn *field = (gsp_resultColumn *)gsp_list_celldata(cell);
			_traverseResultColumn(sqlTraverser, field, parent);
		}
	}

	joins = select->fromTableList;
	if(joins!=NULL){
		foreach(cell, joins){
			gsp_fromTable *join = (gsp_fromTable *)gsp_list_celldata(cell);
			_traverseJoin(sqlTraverser, join, parent);
		}
	}

	if(select->groupByClause!=NULL){
		_insertIntoList( sqlTraverser,  select->groupByClause, parent);
		groupByList = select->groupByClause->items;
		foreach(element, groupByList){
			gsp_gruopByItem *groupBy = (gsp_gruopByItem *)gsp_list_celldata(element);
			if(groupBy!=NULL){
				_insertIntoList( sqlTraverser,  groupBy, parent);
				if(groupBy->expr!=NULL){
					_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, groupBy->expr, parent, esc_groupby));
				}

				if(groupBy->aliasClause!=NULL){
					_traverseAliasClause(sqlTraverser, groupBy->aliasClause, parent);
				}
			}
		}
		if(select->groupByClause->havingClause!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, select->groupByClause->havingClause, parent, esc_having));
		}
	}

	if(select->cteList!=NULL){
		foreach(element, select->cteList){
			gsp_cte *cteItem = (gsp_cte*)gsp_list_celldata(element);
			if(cteItem!=NULL){
				_traverseCTEItem(sqlTraverser, cteItem, parent);
			}
		}
	}

	if(select->intoClause!=NULL && select->intoClause->exprList!=NULL){
		_insertIntoList( sqlTraverser,  select->intoClause, parent);
		foreach(element, select->intoClause->exprList){
			gsp_expr *exprItem = (gsp_expr*)gsp_list_celldata(element);
			if(exprItem!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, exprItem, parent, esc_selectInto));
			}
		}
	}

	if(select->orderbyClause!=NULL){
		_insertIntoList( sqlTraverser,  select->orderbyClause, parent);
		sortList = select->orderbyClause->items;
		foreach(element, sortList){
			gsp_orderByItem *orderBy = (gsp_orderByItem *)gsp_list_celldata(element);
			if(orderBy!=NULL){
				_insertIntoList( sqlTraverser,  orderBy, parent);
				if(orderBy->sortKey!=NULL){
					_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, orderBy->sortKey, parent, esc_orderby));
				}
			}
		}
	}

	if(select->hierarchicalClause!=NULL){
		_insertIntoList( sqlTraverser,  select->hierarchicalClause, parent);
		if(select->hierarchicalClause->startWithClause!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, select->hierarchicalClause->startWithClause, parent, esc_hierarchical));
		}
		if(select->hierarchicalClause->connectByClause!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, select->hierarchicalClause->connectByClause, parent, esc_hierarchical));
		}
	}

	if(select->whereCondition!=NULL && select->whereCondition->condition!=NULL){
		_insertIntoList( sqlTraverser,  select->whereCondition, parent);
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, select->whereCondition->condition, parent, esc_where));
	}

	if(select->forupdateClause!=NULL && select->forupdateClause->columnRefs!=NULL){
		foreach(element, select->forupdateClause->columnRefs){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, column, parent);
		}
	}

	if(select->valueClause!=NULL){
		_traverseValueClause( sqlTraverser, select->valueClause, parent);
	}

	if(select->leftStmt!= NULL){
		_traverseSelectSQL( sqlTraverser, select->leftStmt, parent);
	}

	if(select->rightStmt != NULL){
		_traverseSelectSQL( sqlTraverser, select->rightStmt, parent);
	}

	if(select->computeClause != NULL){
		_traverseMssqlComputeClause( sqlTraverser, select->computeClause, parent);
	}

	if(select->expandOnClause != NULL){
		_traverseTeradataExpandOnClause( sqlTraverser, select->expandOnClause, parent);
	}

	if(select->fetchFirstClause != NULL){
		_traverseFetchFirstClause( sqlTraverser, select->fetchFirstClause, parent);
	}

	if(select->intoTableClause != NULL){
		_traverseIntoTableClause( sqlTraverser, select->intoTableClause, parent);
	}

	if(select->isolationClause != NULL){
		_traverseIsolationClause( sqlTraverser, select->isolationClause, parent);
	}

	if(select->limitClause != NULL){
		_traverseLimitClause( sqlTraverser, select->limitClause, parent);
	}

	if(select->lockingClause != NULL){
		_traverseLockingClause( sqlTraverser, select->lockingClause, parent);
	}

	if(select->optimizeForClause != NULL){
		_traverseOptimizeForClause( sqlTraverser, select->optimizeForClause, parent);
	}

	if(select->qualifyClause != NULL){
		_traverseQualifyClause( sqlTraverser, select->qualifyClause, parent);
	}

	if(select->sampleClause != NULL){
		_traverseSampleClause( sqlTraverser, select->sampleClause, parent);
	}

	if(select->topClause != NULL){
		_traverseTopClause( sqlTraverser, select->topClause, parent);
	}

	if(select->windowClause != NULL){
		_traverseWindowClause( sqlTraverser, select->windowClause, parent);
	}

	if(select->withClauses != NULL){
		foreach(element, select->withClauses){
			gsp_teradataWithClause *clause = (gsp_teradataWithClause *)gsp_list_celldata(element);
			_traverseTeradataWithClause( sqlTraverser, clause, parent);
		}
	}
}

static  void _traverseUpdateSQL(SqlTraverser *sqlTraverser, gsp_updateStatement *update, NodeContext *container){
	gsp_listcell *cell, *element;
	gsp_list *fields;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  update, container);

	fields = update->resultColumnList;
	if(fields!=NULL){
		foreach(cell, fields){
			gsp_resultColumn *field = (gsp_resultColumn *)gsp_list_celldata(cell);
			_traverseResultColumn(sqlTraverser, field, parent);
		}
	}

	if(update->targetTableNode!=NULL){
		gsp_fromTable *table = update->targetTableNode;
		_traverseJoin(sqlTraverser, table, parent);
	}

	if(update->whereCondition!=NULL && update->whereCondition->condition!=NULL){
		_insertIntoList( sqlTraverser,  update->whereCondition, parent);
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, update->whereCondition->condition, parent, esc_where));
	}

	if(update->returningClause!=NULL){
		_insertIntoList( sqlTraverser,  update->returningClause, parent);
		if(update->returningClause->columnValueList!=NULL){
			foreach(element, update->returningClause->columnValueList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
		if(update->returningClause->variableList!=NULL){
			foreach(element, update->returningClause->variableList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
	}

	if(update->includeColumns!=NULL){
		_traverseIncludeColumns(sqlTraverser, update->includeColumns, parent);
	}

	if(update->isolationClause!=NULL){
		_traverseIsolationClause(sqlTraverser, update->isolationClause, parent);
	}

	if(update->limitClause!=NULL){
		_traverseLimitClause(sqlTraverser, update->limitClause, parent);
	}

	if(update->outputClause!=NULL){
		_traverseOutputClause(sqlTraverser, update->outputClause, parent);
	}

	if(update->sortClause!=NULL){
		_traverseOrderBy(sqlTraverser, update->sortClause, parent);
	}

	if(update->topClause!=NULL){
		_traverseTopClause(sqlTraverser, update->topClause, parent);
	}

	if(update->sourceTableList!=NULL){
		foreach(element, update->sourceTableList){
			gsp_fromTable *table = (gsp_fromTable *)gsp_list_celldata(element);
			_traverseJoin(sqlTraverser, table, parent);
		}
	}

	if(update->targetTableList!=NULL){
		foreach(element, update->targetTableList){
			gsp_fromTable *table = (gsp_fromTable *)gsp_list_celldata(element);
			_traverseJoin(sqlTraverser, table, parent);
		}
	}


	if(update->cteList!=NULL){
		foreach(element, update->cteList){
			gsp_cte *cteItem = (gsp_cte*)gsp_list_celldata(element);
			if(cteItem!=NULL){
				_traverseCTEItem(sqlTraverser, cteItem, parent);
			}
		}
	}
}

static void _traverseCreateTableSQL(SqlTraverser *sqlTraverser, gsp_createTableStatement *create, NodeContext *container){
	gsp_listcell *cell;
	NodeContext* parent;

	parent = _insertIntoList( sqlTraverser,  create, container);

	if(create->tableElementList!=NULL){
		foreach(cell, create->tableElementList){
			gsp_tableElement *tableElement = (gsp_tableElement *)gsp_list_celldata(cell);
			_traverseTableElement(sqlTraverser, tableElement, parent);
		}
	}

	if(create->subQueryNode!=NULL){
		_traverseSelectSQL( sqlTraverser, (gsp_selectStatement*)create->subQueryNode, parent);
	}

	if(create->table!=NULL){
		_traverseTable(sqlTraverser,  create->table, parent);
	}

	if(create->oftable!=NULL){
		_traverseTable(sqlTraverser,  create->oftable, parent);
	}

	if(create->likeTableName!=NULL){
		_insertIntoList( sqlTraverser,  create->likeTableName, parent);
	}

	if(create->columnName!=NULL){
		_insertIntoList( sqlTraverser,  create->columnName, parent);
	}

	if(create->superTableName!=NULL){
		_insertIntoList( sqlTraverser,  create->superTableName, parent);
	}

	if(create->ofTypeName!=NULL){
		_insertIntoList( sqlTraverser,  create->ofTypeName, parent);
	}

	if(create->columnList!=NULL){
		foreach(cell, create->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)create->table);
			if(!subReferences->contains(subReferences, column))
				subReferences->add(subReferences, column);
			_insertIntoList( sqlTraverser,  column, parent);
		}
	}
}

static void _traverseAlterTableOption(SqlTraverser * sqlTraverser, gsp_table * table, gsp_alterTableOption * alterTableOption, NodeContext* container)
{
	gsp_listcell *cell;
	NodeContext* parent;

	parent = _insertIntoList( sqlTraverser,  alterTableOption, container);

	if(alterTableOption->columnDefinitionList!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->columnDefinitionList, parent);
		foreach(cell, alterTableOption->columnDefinitionList){
			gsp_columnDefinition *columnDefinition = (gsp_columnDefinition *)gsp_list_celldata(cell);
			_traverseColumnDefinition(sqlTraverser, columnDefinition, parent );
		}
	}

	if(alterTableOption->columnName!=NULL){
		List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)table);
		if(!subReferences->contains(subReferences, alterTableOption->columnName))
			subReferences->add(subReferences, alterTableOption->columnName);
		_insertIntoList( sqlTraverser, alterTableOption->columnName, parent);
	}

	if(alterTableOption->columnNameList!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->columnNameList, parent);
		foreach(cell, alterTableOption->columnNameList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)table);
			if(!subReferences->contains(subReferences, alterTableOption->columnName))
				subReferences->add(subReferences, column);
			_insertIntoList( sqlTraverser, column, parent);
		}
	}

	if(alterTableOption->constraintList!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->constraintList, parent);
		foreach(cell, alterTableOption->constraintList){
			gsp_constraint *constraint = (gsp_constraint *)gsp_list_celldata(cell);
			_traverseConstraint(sqlTraverser, constraint, parent);
		}
	}

	if(alterTableOption->constraintName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->constraintName, parent);
	}

	if(alterTableOption->datatype!=NULL){
		_traverseTypeName( sqlTraverser, alterTableOption->datatype, parent);
	}

	if(alterTableOption->indexName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->indexName, parent);
	}

	if(alterTableOption->newColumnName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->newColumnName, parent);
	}

	if(alterTableOption->newConstraintName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->newConstraintName, parent);
	}

	if(alterTableOption->newTableName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->newTableName, parent);
	}

	if(alterTableOption->referencedColumnList!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->referencedColumnList, parent);
		foreach(cell, alterTableOption->columnNameList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			_insertIntoList( sqlTraverser,  column, parent);
		}
	}

	if(alterTableOption->referencedObjectName!=NULL){
		_insertIntoList( sqlTraverser, alterTableOption->referencedObjectName, parent);
	}
}

static void _traverseAlterTableSQL(SqlTraverser *sqlTraverser, gsp_alterTableStatement *alterTable, NodeContext *container){
	gsp_listcell *cell;
	NodeContext* parent;

	parent = _insertIntoList( sqlTraverser,  alterTable, container);

	if(alterTable->tableElementList!=NULL){
		foreach(cell, alterTable->tableElementList){
			gsp_tableElement *tableElement = (gsp_tableElement *)gsp_list_celldata(cell);
			_traverseTableElement(sqlTraverser, tableElement, parent);
		}
	}

	if(alterTable->tableName!=NULL){
		gsp_table *table;
		table = (gsp_table *)malloc(sizeof(gsp_table));
		table->tableName = alterTable->tableName;
		table->tableSource = ets_objectname;
		table->aliasClause = NULL;
		table->nodeType = t_gsp_table;
		table->tableExpr = NULL;
		sqlTraverser->__entries->add(sqlTraverser->__entries,table);
		_insertIntoList(sqlTraverser, table, parent);
		_insertIntoList( sqlTraverser, alterTable->tableName, parent);

		if(alterTable->alterTableOptionList!=NULL){
			foreach(cell, alterTable->alterTableOptionList){
				gsp_alterTableOption *alterTableOption = (gsp_alterTableOption *)gsp_list_celldata(cell);
				_traverseAlterTableOption(sqlTraverser, table, alterTableOption, parent);
			}
		}
	}
}

static void _traverseInsertRest(SqlTraverser *sqlTraverser, gsp_insertRest *insertRest, NodeContext *parent){
	gsp_listcell *element;

	if(insertRest!=NULL){
		_insertIntoList( sqlTraverser,  insertRest, parent);
		if(insertRest->functionCall!=NULL){
			_traverseFunctionCall(sqlTraverser, insertRest->functionCall, parent, esc_insertValues);
		}

		if(insertRest->updateTargetList!=NULL){
			foreach(element, insertRest->updateTargetList){
				gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
				_traverseResultColumn(sqlTraverser, column, parent );
			}
		}

		if(insertRest->multiTargetList!=NULL){
			foreach(element, insertRest->multiTargetList){
				gsp_multiTarget *target = (gsp_multiTarget *)gsp_list_celldata(element);
				_traverseMultiTarget(sqlTraverser, target, parent);
			}
		}

		if(insertRest->recordName!=NULL){
			_insertIntoList( sqlTraverser, insertRest->recordName, parent);
		}

		if(insertRest->subQueryNode!=NULL){
			_traverseSelectSQL(sqlTraverser, insertRest->subQueryNode, parent);
		}
	}
}


static void _traverseInsertSQL(SqlTraverser *sqlTraverser, gsp_insertStatement *insert, NodeContext *container){
	gsp_listcell *cell, *element, *subElement;
	NodeContext* parent;

	parent = _insertIntoList( sqlTraverser,  insert, container);

	if(insert->targetTableNode!=NULL){
		_traverseJoin(sqlTraverser,  insert->targetTableNode, parent);
	}

	if(insert->recordName!=NULL){
		_insertIntoList( sqlTraverser, insert->recordName, parent);
	}

	if(insert->columnList!=NULL){
		foreach(cell, insert->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
			List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)insert->targetTableNode);
			if(!subReferences->contains(subReferences, column))
				subReferences->add(subReferences, column);
			_insertIntoList( sqlTraverser,  column, parent);
		}
	}

	if(insert->multiTargetList!=NULL){
		foreach(cell, insert->multiTargetList){
			gsp_multiTarget *target = (gsp_multiTarget *)gsp_list_celldata(cell);
			_traverseMultiTarget(sqlTraverser, target, parent);
		}
	}

	if(insert->functionCall!=NULL){
		_traverseFunctionCall(sqlTraverser, insert->functionCall, parent, esc_insertValues);
	}

	if(insert->subQueryNode!=NULL){
		_traverseSelectSQL( sqlTraverser, insert->subQueryNode, parent);
	}

	if(insert->insertIntoValues!=NULL){
		foreach(cell, insert->insertIntoValues){
			gsp_insertIntoValue *value = (gsp_insertIntoValue *)gsp_list_celldata(cell);
			if(value->columnList!=NULL){
				gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(cell);
				List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)insert->targetTableNode);
				if(!subReferences->contains(subReferences, column))
					subReferences->add(subReferences, column);
				_insertIntoList( sqlTraverser,  column, parent);
			}
			if(value->table!=NULL){
				_traverseTable(sqlTraverser, value->table, parent);
			}
			if(value->valuesClause!=NULL){
				_insertIntoList( sqlTraverser, value->valuesClause, parent);
				if(value->valuesClause->multiTargetList!=NULL){
					foreach(subElement, value->valuesClause->multiTargetList){
						gsp_multiTarget *target = (gsp_multiTarget *)gsp_list_celldata(subElement);
						_traverseMultiTarget(sqlTraverser, target, parent);
					}
				}
			}
		}
	}

	if(insert->insertConditions!=NULL){
		foreach(cell, insert->insertConditions){
			gsp_insertCondition *condition = (gsp_insertCondition *)gsp_list_celldata(cell);
			_insertIntoList( sqlTraverser,  condition, parent);
			if(condition->condition!=NULL){
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, condition->condition, parent, esc_insertValues));
			}
			if(condition->insertIntoValues!=NULL){
				foreach(element, condition->insertIntoValues){
					gsp_insertIntoValue *value = (gsp_insertIntoValue *)gsp_list_celldata(element);
					_insertIntoList( sqlTraverser,  value, parent);
					if(value->columnList!=NULL){
						foreach(subElement, value->columnList){
							gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(subElement);
							_insertIntoList( sqlTraverser,  column, parent);
						}
					}
					if(value->table!=NULL){
						_traverseTable(sqlTraverser, value->table, parent);
					}
					if(value->valuesClause!=NULL && value->valuesClause->multiTargetList!=NULL ){
						foreach(subElement, value->valuesClause->multiTargetList){
							gsp_multiTarget *target = (gsp_multiTarget *)gsp_list_celldata(subElement);
							_traverseMultiTarget(sqlTraverser, target, parent);
						}
					}
				}
			}
		}
	}

	if(insert->returningClause!=NULL){
		_insertIntoList( sqlTraverser,  insert->returningClause, parent);
		if(insert->returningClause->columnValueList!=NULL){
			foreach(element, insert->returningClause->columnValueList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
		if(insert->returningClause->variableList!=NULL){
			foreach(element, insert->returningClause->variableList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
	}

	if(insert->includeColumns!=NULL){
		_traverseIncludeColumns(sqlTraverser, insert->includeColumns, parent);
	}

	if(insert->isolationClause!=NULL){
		_traverseIsolationClause(sqlTraverser, insert->isolationClause, parent);
	}

	if(insert->topClause!=NULL){
		_traverseTopClause(sqlTraverser, insert->topClause, parent);
	}

	if(insert->onDuplicateUpdateList!=NULL){
		foreach(cell, insert->onDuplicateUpdateList){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(cell);
			_traverseResultColumn(sqlTraverser, column, parent);
		}
	}

	if(insert->insertRest!=NULL){
		_traverseInsertRest(sqlTraverser, insert->insertRest, parent);
	}

	if(insert->cteList!=NULL){
		foreach(element, insert->cteList){
			gsp_cte *cteItem = (gsp_cte*)gsp_list_celldata(element);
			if(cteItem!=NULL){
				_traverseCTEItem(sqlTraverser, cteItem, parent);
			}
		}
	}
}

static void _traverseOutputClause(SqlTraverser *sqlTraverser, gsp_mssql_outputClause *outputClause, NodeContext *parent){
	gsp_listcell *element;

	if(outputClause!=NULL){
		_insertIntoList( sqlTraverser,  outputClause, parent);
		if(outputClause->selectItemList!=NULL){
			foreach(element, outputClause->selectItemList){
				gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
				_traverseResultColumn(sqlTraverser, column, parent );
			}
		}

		if(outputClause->selectItemList2!=NULL){
			foreach(element, outputClause->selectItemList2){
				gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
				_traverseResultColumn(sqlTraverser, column, parent );
			}
		}

		if(outputClause->intoColumnList!=NULL){
			foreach(element, outputClause->intoColumnList){
				gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
				_insertIntoList( sqlTraverser, column, parent);
			}
		}

		if(outputClause->tableName!=NULL){
			_insertIntoList( sqlTraverser, outputClause->tableName, parent);
		}
	}
}

static void _traverseIncludeColumns(SqlTraverser *sqlTraverser, gsp_includeColumns *includeColumns, NodeContext *parent){
	gsp_listcell *element;

	if(includeColumns!=NULL){
		_insertIntoList( sqlTraverser,  includeColumns, parent);
		foreach(element, includeColumns->columnList){
			gsp_columnDefinition *column = (gsp_columnDefinition *)gsp_list_celldata(element);
			_traverseColumnDefinition(sqlTraverser, column, parent );
		}
	}
}

static void _traverseIsolationClause(SqlTraverser *sqlTraverser, gsp_isolationClause *isolationClause, NodeContext *parent){

	if(isolationClause!=NULL){
		_insertIntoList( sqlTraverser, isolationClause, parent);
	}
}

static void _traverseTopClause(SqlTraverser *sqlTraverser, gsp_topClause *topClause, NodeContext *parent){

	if(topClause!=NULL){
		_insertIntoList( sqlTraverser, topClause, parent);
		if(topClause->expr!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, topClause->expr, parent,  esc_top));
		}
	}
}

static void _traverseLimitClause(SqlTraverser *sqlTraverser, gsp_limitClause *limitClause, NodeContext *parent){
	
	if(limitClause!=NULL){
		_insertIntoList( sqlTraverser,  limitClause, parent);
		if(limitClause->limitValue!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, limitClause->limitValue, parent,  esc_limit));
		}
		if(limitClause->offset!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, limitClause->offset, parent,  esc_limit));
		}
		if(limitClause->rowCount!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, limitClause->rowCount, parent,  esc_limit));
		}
	}
}

static void _traverseDeleteSQL(SqlTraverser *sqlTraverser, gsp_deleteStatement *deleteStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  deleteStmt, container);

	if(deleteStmt->targetTableNode!=NULL){
		_traverseJoin(sqlTraverser,  deleteStmt->targetTableNode, parent);
	}

	if(deleteStmt->whereCondition!=NULL && deleteStmt->whereCondition->condition!=NULL){
		_insertIntoList( sqlTraverser,  deleteStmt->whereCondition, parent);
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, deleteStmt->whereCondition->condition, parent, esc_where));
	}

	if(deleteStmt->returningClause!=NULL){
		_insertIntoList( sqlTraverser,  deleteStmt->returningClause, parent);
		if(deleteStmt->returningClause->columnValueList!=NULL){
			foreach(element, deleteStmt->returningClause->columnValueList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
		if(deleteStmt->returningClause->variableList!=NULL){
			foreach(element, deleteStmt->returningClause->variableList){
				gsp_expr *expr = (gsp_expr *)gsp_list_celldata(element);
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_returning));
			}
		}
	}

	if(deleteStmt->outputClause!=NULL){
		_traverseOutputClause(sqlTraverser, deleteStmt->outputClause, parent);
	}

	if(deleteStmt->includeColumns!=NULL){
		_traverseIncludeColumns(sqlTraverser, deleteStmt->includeColumns, parent );
	}

	if(deleteStmt->isolationClause!=NULL){
		_traverseIsolationClause(sqlTraverser, deleteStmt->isolationClause, parent );
	}

	if(deleteStmt->limitClause!=NULL){
		_traverseLimitClause(sqlTraverser, deleteStmt->limitClause, parent );
	}

	if(deleteStmt->sortClause!=NULL){
		_traverseOrderBy(sqlTraverser, deleteStmt->sortClause, parent );
	}

	if(deleteStmt->topClause!=NULL){
		_traverseTopClause(sqlTraverser, deleteStmt->topClause, parent );
	}

	if(deleteStmt->sourceTableList!=NULL){
		foreach(element, deleteStmt->sourceTableList){
			gsp_fromTable *table = (gsp_fromTable *)gsp_list_celldata(element);
			_traverseJoin(sqlTraverser, table, parent);
		}
	}

	if(deleteStmt->targetTableList!=NULL){
		foreach(element, deleteStmt->targetTableList){
			gsp_fromTable *table = (gsp_fromTable *)gsp_list_celldata(element);
			_traverseJoin(sqlTraverser, table, parent);
		}
	}

	if(deleteStmt->cteList!=NULL){
		foreach(element, deleteStmt->cteList){
			gsp_cte *cteItem = (gsp_cte*)gsp_list_celldata(element);
			if(cteItem!=NULL){
				_traverseCTEItem(sqlTraverser, cteItem, parent);
			}
		}
	}
}

static void _traverseMergeInsertClause(SqlTraverser *sqlTraverser, gsp_mergeStatement *merge, gsp_mergeInsertClause *insertClause, NodeContext *parent){
	gsp_listcell *element;
	if(insertClause->deleteWhereClause!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, insertClause->deleteWhereClause, parent, esc_unknown));
	}
	if(insertClause->valuelist!=NULL){
		foreach(element, insertClause->valuelist){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, column, parent);
			if(column->expr!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, column->expr, parent, esc_unknown));
			}
			if(column->aliasClause!=NULL)
				_traverseAliasClause(sqlTraverser,column->aliasClause, parent);
		}
	}
	if(insertClause->columnList!=NULL){
		foreach(element, insertClause->columnList){
			gsp_objectname *column = (gsp_objectname *)gsp_list_celldata(element);
			List* subReferences = _getObjectNameReferences(sqlTraverser, (gsp_node*)merge->targetTable);
			if(!subReferences->contains(subReferences, column))
				subReferences->add(subReferences, column);
			_insertIntoList( sqlTraverser, column, parent);
		}
	}
	_insertIntoList( sqlTraverser, insertClause, parent);
}

static void _traverseMergeUpdateClause(SqlTraverser *sqlTraverser, gsp_mergeUpdateClause *updateClause, NodeContext *parent){
	gsp_listcell *element;
	if(updateClause->deleteWhereClause!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, updateClause->deleteWhereClause, parent, esc_unknown));
	}
	if(updateClause->updateWhereClause!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, updateClause->updateWhereClause, parent, esc_unknown));
	}
	if(updateClause->updateColumnList!=NULL){
		foreach(element, updateClause->updateColumnList){
			gsp_resultColumn *column = (gsp_resultColumn *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, column, parent);
			if(column->expr!=NULL){
				_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, column->expr, parent, esc_set));
			}
			if(column->aliasClause!=NULL)
				_traverseAliasClause(sqlTraverser,column->aliasClause, parent);
		}
	}
	_insertIntoList( sqlTraverser, updateClause, parent);
}

static void _traverseMergeSQL(SqlTraverser *sqlTraverser, gsp_mergeStatement *merge, NodeContext *container){
	gsp_listcell *element;
	NodeContext* parent;

	parent = _insertIntoList( sqlTraverser,  merge, container);

	if(merge->targetTable!=NULL){
		_traverseTable(sqlTraverser,  merge->targetTable, parent);
	}

	if(merge->usingTable!=NULL){
		_traverseTable(sqlTraverser,  merge->usingTable, parent);
	}

	if(merge->whenClauses!=NULL){
		foreach(element, merge->whenClauses){
			gsp_mergeWhenClause *item = (gsp_mergeWhenClause *)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser,  item, parent);
			if(item->condition!=NULL){
				_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, item->condition, parent, esc_unknown));
			}
			if(item->insertClause!=NULL){
				gsp_mergeInsertClause *insertClause = (gsp_mergeInsertClause *)item->insertClause;
				_traverseMergeInsertClause(sqlTraverser, merge, insertClause, parent);
			}
			else if(item->updateClause!=NULL){
				gsp_mergeUpdateClause *updateClause = (gsp_mergeUpdateClause *)item->updateClause;
				_traverseMergeUpdateClause(sqlTraverser,  updateClause, parent);
			}
			else if(item->deleteClause!=NULL){
				gsp_mergeDeleteClause *deleteClause = (gsp_mergeDeleteClause *)item->deleteClause;
				_insertIntoList( sqlTraverser, deleteClause, parent);
			}
		}
	}

	if(merge->condition!=NULL){
		_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, merge->condition, parent, esc_joinCondition));
	}

	if(merge->cteList!=NULL){
		foreach(element, merge->cteList){
			gsp_cte *cteItem = (gsp_cte*)gsp_list_celldata(element);
			if(cteItem!=NULL){
				_traverseCTEItem(sqlTraverser, cteItem, parent);
			}
		}
	}
}

static void _traverseIndices( SqlTraverser * sqlTraverser, gsp_indices * indices, NodeContext * parent ) 
{
	if(indices!=NULL){
		_insertIntoList( sqlTraverser, indices, parent);
		_insertIntoList( sqlTraverser, indices->attributeName, parent);
		if(indices->lowerSubscript!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, indices->lowerSubscript, parent, esc_unknown));
		}
		if(indices->upperSubscript!=NULL){
			_process_expr(sqlTraverser, _insertExprIntoList(sqlTraverser, indices->upperSubscript, parent, esc_unknown));
		}
	}
}

static void _traverseDatatypeAttribute( SqlTraverser * sqlTraverser, gsp_datatypeAttribute * attr, NodeContext * parent ) 
{
	if(attr!=NULL){
		_insertIntoList( sqlTraverser, attr, parent);
	}
}

static void _traverseTypeName(SqlTraverser *sqlTraverser, gsp_typename *typeName, NodeContext *parent){
	gsp_listcell *cell;
	if(typeName!=NULL){
		_insertIntoList( sqlTraverser, typeName, parent);
		_insertIntoList( sqlTraverser, typeName->length, parent);
		_insertIntoList( sqlTraverser, typeName->secondsPrecision, parent);
		if(typeName->precisionScale!=NULL){
			_insertIntoList( sqlTraverser, typeName->precisionScale->precision, parent);
			_insertIntoList( sqlTraverser, typeName->precisionScale->scale, parent);
		}
		if(typeName->indices!=NULL){
			foreach(cell, typeName->indices){
				gsp_indices *indices = (gsp_indices*)gsp_list_celldata(cell);
				_traverseIndices(sqlTraverser, indices, parent);
			}
		}
		if(typeName->datatypeAttributeList!=NULL){
			foreach(cell, typeName->datatypeAttributeList){
				gsp_datatypeAttribute *attr = (gsp_datatypeAttribute*)gsp_list_celldata(cell);
				_traverseDatatypeAttribute(sqlTraverser, attr, parent);
			}
		}
	}
}

static void _traverseParameterDeclaration(SqlTraverser *sqlTraverser, gsp_parameterDeclaration *parameterDeclaration, NodeContext *parent){
	_insertIntoList( sqlTraverser, parameterDeclaration, parent);

	if(parameterDeclaration->parameterName!=NULL){
		_insertIntoList( sqlTraverser, parameterDeclaration->parameterName, parent);
	}

	if(parameterDeclaration->dataType!=NULL){
		_traverseTypeName( sqlTraverser, parameterDeclaration->dataType, parent);
	}

	if(parameterDeclaration->varyPrecision!=NULL){
		_insertIntoList( sqlTraverser, parameterDeclaration->varyPrecision, parent);
	}

	if(parameterDeclaration->defaultValue!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, parameterDeclaration->defaultValue, parent, esc_unknown));
	}
}

static void _traverseExceptionClause(SqlTraverser *sqlTraverser, gsp_exceptionClause *exceptionClause, NodeContext *parent){
	gsp_listcell *element, *cell;
	if(exceptionClause==NULL)
		return;
	_insertIntoList( sqlTraverser, exceptionClause, parent);
	if(exceptionClause->Handlers!=NULL){
		foreach(element, exceptionClause->Handlers){
			gsp_exceptionHandler *handler = (gsp_exceptionHandler*)gsp_list_celldata(element);
			if(handler!=NULL){
				if(handler->exceptionNames!=NULL){
					foreach(cell, handler->exceptionNames){
						gsp_objectname *name = (gsp_objectname*)gsp_list_celldata(cell);
						if(name!=NULL){
							_insertIntoList( sqlTraverser, name, parent);
						}
					}
				}
				if(handler->stmts!=NULL){
					foreach(cell, handler->stmts){
						gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(cell);
						if(stmt!=NULL){
							_traverseStatement(sqlTraverser, stmt, parent);
						}
					}
				}
			}
		}
	}
}


static void _traverseTriggerEvent(SqlTraverser *sqlTraverser, gsp_trigger_event *triggerEvent, NodeContext *parent){
	if(triggerEvent == NULL)
		return;
	else{
		gsp_listcell *element;
		_insertIntoList( sqlTraverser, triggerEvent, parent);
		if(triggerEvent->dmlType == 1){
			if(triggerEvent->dml.compound!=NULL && triggerEvent->dml.compound->dmlEventClause!=NULL){
				gsp_table *table;
				_insertIntoList(sqlTraverser, triggerEvent->dml.compound->dmlEventClause, parent);
				table = (gsp_table *)malloc(sizeof(gsp_table));
				table->tableName = triggerEvent->dml.compound->dmlEventClause->tableName;
				table->tableSource = ets_objectname;
				table->aliasClause = NULL;
				table->nodeType = t_gsp_table;
				table->tableExpr = NULL;
				sqlTraverser->__entries->add(sqlTraverser->__entries,table);
				_insertIntoList(sqlTraverser, table, parent);

				if(triggerEvent->dml.compound->dmlEventClause->dml_event_items!=NULL){
					gsp_list *items = triggerEvent->dml.compound->dmlEventClause->dml_event_items;
					foreach(element, items){
						//TODO
					}
				}
			}
		}
		else if(triggerEvent->dmlType == 2){
			if(triggerEvent->dml.non!=NULL){
				_insertIntoList(sqlTraverser, triggerEvent->dml.non->schemaName, parent);
				if(triggerEvent->dml.non->database_event_list!=NULL){
					gsp_list *items = triggerEvent->dml.non->database_event_list;
					foreach(element, items){
						//TODO
					}
				}
				if(triggerEvent->dml.non->ddl_event_list!=NULL){
					gsp_list *items = triggerEvent->dml.non->ddl_event_list;
					foreach(element, items){
						//TODO
					}
				}
			}
		}
		else if(triggerEvent->dmlType == 0){
			if(triggerEvent->dml.simple!=NULL && triggerEvent->dml.simple->dmlEventClause!=NULL){
				gsp_table *table;
				_insertIntoList(sqlTraverser, triggerEvent->dml.simple->dmlEventClause, parent);
				table = (gsp_table *)malloc(sizeof(gsp_table));
				table->tableName = triggerEvent->dml.simple->dmlEventClause->tableName;
				table->tableSource = ets_objectname;
				table->aliasClause = NULL;
				table->nodeType = t_gsp_table;
				table->tableExpr = NULL;
				sqlTraverser->__entries->add(sqlTraverser->__entries,table);
				_insertIntoList(sqlTraverser, table, parent);

				if(triggerEvent->dml.simple->dmlEventClause->dml_event_items!=NULL){
					gsp_list *items = triggerEvent->dml.simple->dmlEventClause->dml_event_items;
					foreach(element, items){
						//TODO
					}
				}
			}
		}
	}
}

static void _traversePlsqlCreateType(SqlTraverser *sqlTraverser, gsp_plsqlCreateType *plsqlCreateType, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  plsqlCreateType, container);

	_insertIntoList( sqlTraverser,  plsqlCreateType->typeName, parent);

	if(plsqlCreateType->attributes!=NULL){
		foreach(element, plsqlCreateType->attributes){
			gsp_typeAttribute *attr = (gsp_typeAttribute*)gsp_list_celldata(element);
			if(attr!=NULL){
				_insertIntoList( sqlTraverser,  attr, parent);
				_insertIntoList( sqlTraverser,  attr->attributeName, parent);
				_traverseTypeName(sqlTraverser,  attr->datatype, parent);
			}
		}
	}
}

static void _traversePlsqlCreateTypeBody(SqlTraverser *sqlTraverser, gsp_plsqlCreateTypeBody *plsqlCreateTypeBody, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  plsqlCreateTypeBody, container);

	_insertIntoList( sqlTraverser,  plsqlCreateTypeBody->typeName, parent);

	if(plsqlCreateTypeBody->stmts!=NULL){
		foreach(element, plsqlCreateTypeBody->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}
}

static void _traverseCreateTrigger(SqlTraverser *sqlTraverser, gsp_createTriggerStatement *createTriggerStatement, NodeContext *container){
	NodeContext * parent = _insertIntoList( sqlTraverser,  createTriggerStatement, container);

	_insertIntoList( sqlTraverser,  createTriggerStatement->tableName, parent);

	if(createTriggerStatement->stmt!=NULL){
		_traverseStatement(sqlTraverser, createTriggerStatement->stmt, parent);
	}

	_insertIntoList( sqlTraverser,  createTriggerStatement->triggerName, parent);

	if(createTriggerStatement->triggerEvent!=NULL){
		_traverseTriggerEvent(sqlTraverser,  createTriggerStatement->triggerEvent, parent);
	}

	if(createTriggerStatement->whenCondition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, createTriggerStatement->whenCondition, parent, esc_unknown));
	}

	if(createTriggerStatement->blockSqlNode!=NULL){
		_traverseBlockStmt(sqlTraverser, createTriggerStatement->blockSqlNode, parent);
	}

	if(createTriggerStatement->stmtSqlNode!=NULL){
		_traverseStatement(sqlTraverser, createTriggerStatement->stmtSqlNode, parent);
	}
}

static void _traverseCreateFunction(SqlTraverser *sqlTraverser, gsp_createFunctionStatement *createFunctionStatement, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  createFunctionStatement, container);

	if(createFunctionStatement->callSpec!=NULL){
		_insertIntoList( sqlTraverser,  createFunctionStatement->callSpec, parent);
		if(createFunctionStatement->callSpec->declaration!=NULL){
			_insertIntoList( sqlTraverser,  createFunctionStatement->callSpec->declaration, parent);
		}
	}

	if(createFunctionStatement->declareStmts!=NULL){
		foreach(element, createFunctionStatement->declareStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
				_insertToSymbolTable(sqlTraverser, (gsp_base_statement*)createFunctionStatement, (gsp_node*)stmt, edb_variable);
			}
		}
	}

	_traverseExceptionClause(sqlTraverser, createFunctionStatement->exceptionClause, parent);

	if(createFunctionStatement->functionName!=NULL){
		_insertIntoList( sqlTraverser, createFunctionStatement->functionName, parent);
	}

	if(createFunctionStatement->parameters!=NULL){
		foreach(element, createFunctionStatement->parameters){
			gsp_parameterDeclaration *parameter = (gsp_parameterDeclaration*)gsp_list_celldata(element);
			if(parameter!=NULL){
				_traverseParameterDeclaration(sqlTraverser, parameter, parent);
				_insertToSymbolTable(sqlTraverser, (gsp_base_statement*)createFunctionStatement, (gsp_node*)parameter, edb_parameter);
			}
		}
	}

	if(createFunctionStatement->returnDataType!=NULL){
		_traverseTypeName( sqlTraverser, createFunctionStatement->returnDataType, parent);
	}

	if(createFunctionStatement->stmts!=NULL){
		foreach(element, createFunctionStatement->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(createFunctionStatement->blockSqlNode!=NULL){
		_traverseBlockStmt(sqlTraverser, createFunctionStatement->blockSqlNode, parent);
	}

	if(createFunctionStatement->stmtSqlNode!=NULL){
		_traverseStatement(sqlTraverser, createFunctionStatement->stmtSqlNode, parent);
	}
}

static void _traverseCreatePackage(SqlTraverser *sqlTraverser, gsp_createPackageStatement *createPackageStatement, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  createPackageStatement, container);

	if(createPackageStatement->bodyStatements!=NULL){
		foreach(element, createPackageStatement->bodyStatements){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(createPackageStatement->declareStatements!=NULL){
		foreach(element, createPackageStatement->declareStatements){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(createPackageStatement->definitions_or_declarations!=NULL){
		foreach(element, createPackageStatement->definitions_or_declarations){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	_traverseExceptionClause(sqlTraverser, createPackageStatement->exceptionClause, parent);

	_insertIntoList( sqlTraverser, createPackageStatement->packageName, parent);


	if(createPackageStatement->stmts!=NULL){
		foreach(element, createPackageStatement->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}
}

static void _traverseCreateProcedure(SqlTraverser *sqlTraverser, gsp_createProcedureStatement *createProcedureStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  createProcedureStmt, container);

	_insertIntoList( sqlTraverser, createProcedureStmt->procedureName, parent);


	_traverseExceptionClause(sqlTraverser, createProcedureStmt->exceptionClause, parent);

	if(createProcedureStmt->declareStmts!=NULL){
		foreach(element, createProcedureStmt->declareStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
				_insertToSymbolTable(sqlTraverser, (gsp_base_statement*)createProcedureStmt, (gsp_node*)stmt, edb_variable);
			}
		}
	}

	if(createProcedureStmt->innerStmts!=NULL){
		foreach(element, createProcedureStmt->innerStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(createProcedureStmt->parameters!=NULL){
		foreach(element, createProcedureStmt->parameters){
			gsp_parameterDeclaration *parameter = (gsp_parameterDeclaration*)gsp_list_celldata(element);
			if(parameter!=NULL){
				_traverseParameterDeclaration(sqlTraverser, parameter, parent);
				_insertToSymbolTable(sqlTraverser, (gsp_base_statement*)createProcedureStmt, (gsp_node*)parameter, edb_parameter);
			}
		}
	}

	if(createProcedureStmt->stmts!=NULL){
		foreach(element, createProcedureStmt->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(createProcedureStmt->callSpec!=NULL){
		_insertIntoList( sqlTraverser,  createProcedureStmt->callSpec, parent);
		_insertIntoList( sqlTraverser,  createProcedureStmt->callSpec->declaration, parent);
	}

	if(createProcedureStmt->blockSqlNode!=NULL){
		_traverseBlockStmt(sqlTraverser, createProcedureStmt->blockSqlNode, parent);
	}

	if(createProcedureStmt->stmtSqlNode!=NULL){
		_traverseStatement(sqlTraverser, createProcedureStmt->stmtSqlNode, parent);
	}
}

static void _traversePlsqlBasicStmt(SqlTraverser *sqlTraverser, gsp_plsqlBasicStmt *plsqlBasicStmt, NodeContext *container){
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser, plsqlBasicStmt, container);

	if(plsqlBasicStmt->expr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlBasicStmt->expr, parent, esc_unknown));
	}
}

static void _traversePlsqlIfStmt(SqlTraverser *sqlTraverser, gsp_plsqlIfStmt *plsqlIfStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser, plsqlIfStmt, container);

	if(plsqlIfStmt->condition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlIfStmt->condition, parent, esc_unknown));
	}

	if(plsqlIfStmt->elsifStmts!=NULL){
		foreach(element, plsqlIfStmt->elsifStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(plsqlIfStmt->elseStmts!=NULL){
		foreach(element, plsqlIfStmt->elseStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	if(plsqlIfStmt->thenStmts!=NULL){
		foreach(element, plsqlIfStmt->thenStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}
}

static void _traversePlsqlLoopStmt(SqlTraverser *sqlTraverser, gsp_plsqlLoopStmt *plsqlLoopStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser, plsqlLoopStmt, container);

	_insertIntoList( sqlTraverser, plsqlLoopStmt->cursorName, parent);


	if(plsqlLoopStmt->condition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlLoopStmt->condition, parent, esc_unknown));
	}


	_insertIntoList( sqlTraverser, plsqlLoopStmt->indexName, parent);


	if(plsqlLoopStmt->lower_bound!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlLoopStmt->lower_bound, parent, esc_unknown));
	}

	if(plsqlLoopStmt->upper_bound!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlLoopStmt->upper_bound, parent, esc_unknown));
	}

	if(plsqlLoopStmt->stmts!=NULL){
		foreach(element, plsqlLoopStmt->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}
}

static void _traversePlsqlVarDeclareStmt(SqlTraverser *sqlTraverser, gsp_plsqlVarDeclStmt *plsqlVarDeclStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlVarDeclStmt, container);

	_traverseTypeName( sqlTraverser, plsqlVarDeclStmt->dataType, parent);


	_insertIntoList( sqlTraverser, plsqlVarDeclStmt->elementName, parent);


	if(plsqlVarDeclStmt->error_number!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlVarDeclStmt->error_number, parent, esc_unknown));
	}

	_insertIntoList( sqlTraverser, plsqlVarDeclStmt->exception_name, parent);


	if(plsqlVarDeclStmt->value!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlVarDeclStmt->value, parent, esc_unknown));
	}
}

static void _traversePlsqlAssignStmt(SqlTraverser *sqlTraverser, gsp_plsqlAssignStmt *plsqlAssignStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlAssignStmt, container);

	if(plsqlAssignStmt->left!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlAssignStmt->left, parent, esc_unknown));
	}

	if(plsqlAssignStmt->right!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlAssignStmt->right, parent, esc_unknown));
	}
}

static void _traverseWhenClauseItem(  SqlTraverser * sqlTraverser, gsp_whenClauseItem * item, NodeContext * parent) 
{
	gsp_listcell *element;

	if(item!=NULL){
		_insertIntoList( sqlTraverser, item, parent);

		if(item->comparison_expr!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, item->comparison_expr, parent, esc_unknown));
		}
		if(item->return_expr!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, item->return_expr, parent, esc_unknown));
		}
		if(item->countFractionDescriptionList!=NULL){
			foreach(element, item->countFractionDescriptionList){
				_insertIntoList( sqlTraverser,  (gsp_constant*)gsp_list_celldata(element), parent);
			}
		}
		if(item->stmts!=NULL){
			foreach(element, item->stmts){
				gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
				if(stmt!=NULL){
					_traverseStatement(sqlTraverser, stmt, parent);
				}
			}
		}
	}
}

static void _traversePlsqlCaseStmt(SqlTraverser *sqlTraverser, gsp_plsqlCaseStmt *plsqlCaseStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser, plsqlCaseStmt, container);

	if(plsqlCaseStmt->caseExpr!=NULL){
		_insertIntoList( sqlTraverser, plsqlCaseStmt->caseExpr, parent);
		if( plsqlCaseStmt->caseExpr->input_expr!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser,plsqlCaseStmt->caseExpr->input_expr, parent, esc_unknown));
		}
		if( plsqlCaseStmt->caseExpr->else_expr!=NULL){
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser,plsqlCaseStmt->caseExpr->else_expr, parent, esc_unknown));
		}
		if( plsqlCaseStmt->caseExpr->whenClauseItemList!=NULL){
			foreach(element, plsqlCaseStmt->caseExpr->whenClauseItemList){
				gsp_whenClauseItem *item = (gsp_whenClauseItem*)gsp_list_celldata(element);
				_traverseWhenClauseItem(sqlTraverser, item, parent);
			}
		}
	}
}

static void _traverseBlockStmt(SqlTraverser *sqlTraverser, gsp_blockStatement *blockStmt, NodeContext *container){
	gsp_listcell *element;
	NodeContext *parent;

	parent = _insertIntoList( sqlTraverser,  blockStmt, container);

	if(blockStmt->declareStmts!=NULL){
		foreach(element, blockStmt->declareStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
				_insertToSymbolTable(sqlTraverser, (gsp_base_statement*)blockStmt, (gsp_node*)stmt, edb_variable);
			}
		}
	}

	if(blockStmt->stmts!=NULL){
		foreach(element, blockStmt->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}

	_traverseExceptionClause(sqlTraverser, blockStmt->exceptionClause, parent);
}

static void _traversePlsqlGotoStmt(SqlTraverser *sqlTraverser, gsp_plsqlGotoStmt *plsqlGotoStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlGotoStmt, container);

	_insertIntoList( sqlTraverser, plsqlGotoStmt->gotolabelName, parent);
}

static void _traversePlsqlReturnStmt(SqlTraverser *sqlTraverser, gsp_plsqlReturnStmt *plsqlReturnStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlReturnStmt, container);
	if(plsqlReturnStmt->expr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlReturnStmt->expr, parent, esc_unknown));
	}
}


static void _traversePlsqlCloseStmt(SqlTraverser *sqlTraverser, gsp_plsqlCloseStmt *plsqlCloseStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlCloseStmt, container);

	_insertIntoList( sqlTraverser, plsqlCloseStmt->cursorName, parent);
}

static void _traversePlsqlOpenStmt(SqlTraverser *sqlTraverser, gsp_plsqlOpenStmt *plsqlOpenStmt, NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlOpenStmt, container);

	_insertIntoList( sqlTraverser, plsqlOpenStmt->cursorName, parent);

	if(plsqlOpenStmt->cursorParameterNames!=NULL){
		foreach(element, plsqlOpenStmt->cursorParameterNames){
			_insertIntoList( sqlTraverser, (gsp_objectname*)gsp_list_celldata(element), parent);
		}
	}
}

static void _traversePlsqlVarrayTypeDefStmt(SqlTraverser *sqlTraverser, gsp_plsqlVarrayTypeDefStmt *plsqlVarrayTypeDefStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlVarrayTypeDefStmt, container);

	_traverseTypeName( sqlTraverser, plsqlVarrayTypeDefStmt->elementDataType, parent);
	_insertIntoList( sqlTraverser, plsqlVarrayTypeDefStmt->sizeLimit, parent);
	_insertIntoList( sqlTraverser, plsqlVarrayTypeDefStmt->typeName, parent);
}

static void _traversePlsqlTableTypeDefStmt(SqlTraverser *sqlTraverser, gsp_plsqlTableTypeDefStmt *plsqlTableTypeDefStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlTableTypeDefStmt, container);

	_traverseTypeName( sqlTraverser, plsqlTableTypeDefStmt->indexByDataType, parent);
	_traverseTypeName( sqlTraverser, plsqlTableTypeDefStmt->elementDataType, parent);
	_insertIntoList( sqlTraverser, plsqlTableTypeDefStmt->typeName, parent);
}

static void _traversePlsqlRaiseStmt(SqlTraverser *sqlTraverser, gsp_plsqlRaiseStmt *plsqlRaiseStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlRaiseStmt, container);
	_insertIntoList( sqlTraverser, plsqlRaiseStmt->exceptionName, parent);
}

static void _traversePlsqlElsifStmt(SqlTraverser *sqlTraverser, gsp_plsqlElsifStmt *plsqlElsifStmt, NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlElsifStmt, container);
	if(plsqlElsifStmt->condition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlElsifStmt->condition, parent, esc_unknown));
	}

	if(plsqlElsifStmt->thenStmts!=NULL){
		foreach(element, plsqlElsifStmt->thenStmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			if(stmt!=NULL){
				_traverseStatement(sqlTraverser, stmt, parent);
			}
		}
	}
}

static void _traversePlsqlCursorDeclStmt(SqlTraverser *sqlTraverser, gsp_plsqlCursorDeclStmt *plsqlCursorDeclStmt, NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlCursorDeclStmt, container);

	if(plsqlCursorDeclStmt->cursorName!=NULL){
		_insertIntoList( sqlTraverser, plsqlCursorDeclStmt->cursorName, parent);
	}

	if(plsqlCursorDeclStmt->cursorParameterDeclarations!=NULL){
		foreach(element, plsqlCursorDeclStmt->cursorParameterDeclarations){
			gsp_parameterDeclaration *param = (gsp_parameterDeclaration*)gsp_list_celldata(element);
			if(param!=NULL){
				_traverseParameterDeclaration(sqlTraverser, param, parent);
			}
		}
	}

	if(plsqlCursorDeclStmt->cursorTypeName!=NULL){
		_insertIntoList( sqlTraverser, plsqlCursorDeclStmt->cursorTypeName, parent);
	}

	if(plsqlCursorDeclStmt->rowtype!=NULL){
		_traverseTypeName(sqlTraverser, plsqlCursorDeclStmt->rowtype, parent);
	}

	if(plsqlCursorDeclStmt->subqueryNode!=NULL && plsqlCursorDeclStmt->subqueryNode->parseTree!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)plsqlCursorDeclStmt->subqueryNode->parseTree, parent);
	}
}

static void _traversePlsqlExitStmt(SqlTraverser *sqlTraverser, gsp_plsqlExitStmt *plsqlExitStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlExitStmt, container);

	if(plsqlExitStmt->exitlabelName!=NULL){
		_insertIntoList( sqlTraverser, plsqlExitStmt->exitlabelName, parent);

	}
	if(plsqlExitStmt->whenCondition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlExitStmt->whenCondition, parent, esc_unknown));
	}
}

static void _traversePlsqlFetchStmt(SqlTraverser *sqlTraverser, gsp_plsqlFetchStmt *plsqlFetchStmt, NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlFetchStmt, container);

	_insertIntoList( sqlTraverser, plsqlFetchStmt->cursorName, parent);

	if(plsqlFetchStmt->variableNames!=NULL){
		foreach(element, plsqlFetchStmt->variableNames){
			gsp_expr *expr = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
		}
	}
}

static void _traversePlsqlForallStmt(SqlTraverser *sqlTraverser, gsp_plsqlForallStmt *plsqlForallStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlForallStmt, container);

	_insertIntoList( sqlTraverser, plsqlForallStmt->indexName, parent);

	if(plsqlForallStmt->stmt!=NULL){
		_traverseStatement(sqlTraverser, plsqlForallStmt->stmt, parent);
	}
}


static void _traversePlsqlOpenforStmt(SqlTraverser *sqlTraverser, gsp_plsqlOpenforStmt *plsqlOpenforStmt, NodeContext *container){
	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlOpenforStmt, container);

	if(plsqlOpenforStmt->cursorVariableName!=NULL){
		_insertIntoList( sqlTraverser, plsqlOpenforStmt->cursorVariableName, parent);
	}

	if(plsqlOpenforStmt->dynamic_string!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, plsqlOpenforStmt->dynamic_string, parent, esc_unknown));
	}

	if(plsqlOpenforStmt->subqueryNode!=NULL && plsqlOpenforStmt->subqueryNode->parseTree!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)plsqlOpenforStmt->subqueryNode->parseTree, parent);
	}
}

static void _traversePlsqlPiperowStmt(SqlTraverser *sqlTraverser, gsp_plsqlPipeRowStmt *plsqlPipeRowStmt, NodeContext *container){
	_insertIntoList( sqlTraverser, plsqlPipeRowStmt, container);
}

static void _traversePlsqlRecordTypedef(SqlTraverser *sqlTraverser, gsp_plsqlRecordTypeDefStmt *plsqlRecordTypeDefStmt, NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, plsqlRecordTypeDefStmt, container);

	_insertIntoList( sqlTraverser, plsqlRecordTypeDefStmt->typeName, parent);

	if(plsqlRecordTypeDefStmt->fieldDeclarations!=NULL){
		foreach(element, plsqlRecordTypeDefStmt->fieldDeclarations){
			gsp_parameterDeclaration *param = (gsp_parameterDeclaration*)gsp_list_celldata(element);
			_traverseParameterDeclaration(sqlTraverser, param, parent);
		}
	}
}

static void _traversePlsqlNullStmt( SqlTraverser * sqlTraverser, gsp_plsqlNullStmt * plsqlNullStmt, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, plsqlNullStmt, container);
}

static void _traverseConstraint( SqlTraverser * sqlTraverser, gsp_constraint * constraint, NodeContext * parent ) 
{
	_insertIntoList( sqlTraverser, constraint, parent);

	if(constraint->checkCondition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, constraint->checkCondition, parent, esc_unknown));
	}

	if(constraint->defaultValue!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, constraint->defaultValue, parent, esc_unknown));
	}

	if(constraint->seedExpr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, constraint->seedExpr, parent, esc_unknown));
	}

	if(constraint->incrementExpr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, constraint->incrementExpr, parent, esc_unknown));
	}

	if(constraint->automaticProperties!=NULL){
		//TODO
	}

	if(constraint->columnList!=NULL){
		//TODO
	}

	if(constraint->keyActions!=NULL){
		//TODO
	}

	if(constraint->referencedColumnList!=NULL){
		//TODO
	}

	if(constraint->incrementExpr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, constraint->incrementExpr, parent, esc_unknown));
	}

	_insertIntoList( sqlTraverser, constraint->constraintName, parent);

	_insertIntoList( sqlTraverser, constraint->referencedObject, parent);
}

static void _traverseColumnDefinition( SqlTraverser * sqlTraverser, gsp_columnDefinition * columnDefinition, NodeContext * parent ) 
{
	_insertIntoList( sqlTraverser, columnDefinition, parent);
	_insertIntoList( sqlTraverser, columnDefinition->columnName, parent);
	
	if(columnDefinition->computedColumnExpression!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, columnDefinition->computedColumnExpression, parent, esc_unknown));
	}

	if(columnDefinition->defaultExpression!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, columnDefinition->defaultExpression, parent, esc_unknown));
	}

	if(columnDefinition->datatype!=NULL){
		_traverseTypeName(sqlTraverser, columnDefinition->datatype, parent);
	}

	if(columnDefinition->computedColumnExpression!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, columnDefinition->computedColumnExpression, parent, esc_unknown));
	}
}

static void _traverseTableElement( SqlTraverser * sqlTraverser, gsp_tableElement * tableElement, NodeContext * parent ) 
{
	_insertIntoList( sqlTraverser, tableElement, parent);

	if(tableElement->columnDefinition!=NULL){
		_traverseColumnDefinition(sqlTraverser, tableElement->columnDefinition, parent);
	}

	if(tableElement->constraint!=NULL){
		_traverseConstraint(sqlTraverser, tableElement->constraint, parent);
	}
}

static void _traverseDeclareVariable( SqlTraverser *sqlTraverser, gsp_declareVariable * variable, NodeContext *parent ) 
{
	gsp_listcell *element;

	_insertIntoList( sqlTraverser, variable, parent);
	
	if(variable->defaultValue!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, variable->defaultValue, parent, esc_unknown));
	}

	_insertIntoList( sqlTraverser, variable->varName, parent);
	
	if(variable->varDatatype!=NULL){
		_traverseTypeName(sqlTraverser, variable->varDatatype, parent);
	}

	if(variable->tableTypeDefinitions!=NULL){
		foreach(element, variable->tableTypeDefinitions){
			gsp_tableElement *tableElement = (gsp_tableElement*)gsp_list_celldata(element);
			_traverseTableElement(sqlTraverser, tableElement, parent);
		}
	}
}

static void _traverseDeclareSqlNode(SqlTraverser *sqlTraverser, gsp_declareSqlNode *declareSqlNode,  NodeContext *container){
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, declareSqlNode, container);
	
	_insertIntoList( sqlTraverser, declareSqlNode->conditionName, parent);
	_insertIntoList( sqlTraverser, declareSqlNode->cursorName, parent);
	
	if(declareSqlNode->defaultValue!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, declareSqlNode->defaultValue, parent, esc_unknown));
	}

	if(declareSqlNode->stmt!=NULL){
		_traverseStatement(sqlTraverser, declareSqlNode->stmt, parent);
	}

	if(declareSqlNode->subQueryNode!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)declareSqlNode->subQueryNode, parent);
	}

	if(declareSqlNode->vars!=NULL){
		foreach(element, declareSqlNode->vars){
			gsp_declareVariable *variable = (gsp_declareVariable*)gsp_list_celldata(element);
			_traverseDeclareVariable(sqlTraverser, variable, parent);
		}
	}

	if(declareSqlNode->varType!=NULL){
		_traverseTypeName(sqlTraverser, declareSqlNode->varType, parent);
	}
}

static void _traverseDB2CallStmtSqlNode( SqlTraverser * sqlTraverser, gsp_db2_callStmtSqlNode * callStmtSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, callStmtSqlNode, container);

	_insertIntoList( sqlTraverser, callStmtSqlNode->procedureName, parent);

	if(callStmtSqlNode->args!=NULL){
		foreach(element, callStmtSqlNode->args){
			gsp_expr *param = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, param, parent, esc_unknown));
		}
	}
}

static void _traverseDB2ForSqlNode( SqlTraverser * sqlTraverser, gsp_db2_forSqlNode * forSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, forSqlNode, container);

	_insertIntoList( sqlTraverser, forSqlNode->cursorName, parent);
	_insertIntoList( sqlTraverser, forSqlNode->loopName, parent);

	if(forSqlNode->stmts!=NULL){
		foreach(element, forSqlNode->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			_traverseStatement(sqlTraverser, stmt, parent);
		}
	}

	if(forSqlNode->subQueryNode!=NULL){
		_traverseSelectSQL(sqlTraverser, (gsp_selectStatement*)forSqlNode->subQueryNode, parent);
	}

}

static void _traverseDB2GotoSqlNode( SqlTraverser * sqlTraverser, gsp_db2_gotoSqlNode * gotoSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, gotoSqlNode, container);

	_insertIntoList( sqlTraverser, gotoSqlNode->labelName, parent);
}

static void _traverseDB2IterateStmtSqlNode( SqlTraverser * sqlTraverser, gsp_db2_iterateStmtSqlNode * iterateStmtSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, iterateStmtSqlNode, container);

	_insertIntoList( sqlTraverser, iterateStmtSqlNode->labelName, parent);
}

static void _traverseDB2LeaveStmtSqlNode( SqlTraverser * sqlTraverser, gsp_db2_leaveStmtSqlNode * leaveStmtSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, leaveStmtSqlNode, container);

	_insertIntoList( sqlTraverser, leaveStmtSqlNode->labelName, parent);
}

static void _traverseDB2LoopSqlNode( SqlTraverser * sqlTraverser, gsp_db2_loopSqlNode * loopSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, loopSqlNode, container);

	if(loopSqlNode->stmts!=NULL){
		foreach(element, loopSqlNode->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			_traverseStatement(sqlTraverser, stmt, parent);
		}
	}
}

static void _traverseDB2RepeatSqlNode( SqlTraverser * sqlTraverser, gsp_db2_repeatSqlNode * repeatSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, repeatSqlNode, container);

	if(repeatSqlNode->condition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, repeatSqlNode->condition, parent, esc_unknown));
	}

	if(repeatSqlNode->stmts!=NULL){
		foreach(element, repeatSqlNode->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			_traverseStatement(sqlTraverser, stmt, parent);
		}
	}
}

static void _traverseDB2WhileSqlNode( SqlTraverser * sqlTraverser, gsp_db2_whileSqlNode * whileSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, whileSqlNode, container);

	if(whileSqlNode->condition!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, whileSqlNode->condition, parent, esc_unknown));
	}

	if(whileSqlNode->stmts!=NULL){
		foreach(element, whileSqlNode->stmts){
			gsp_sql_statement *stmt = (gsp_sql_statement*)gsp_list_celldata(element);
			_traverseStatement(sqlTraverser, stmt, parent);
		}
	}
}

static void _traverseDB2SetSqlNode( SqlTraverser * sqlTraverser, gsp_db2_setSqlNode * setSqlNode, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, setSqlNode, container);
}

static void _traverseMssqlGotoSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_gotoSqlNode * gotoSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, gotoSqlNode, container);

	_insertIntoList( sqlTraverser, gotoSqlNode->labelName, parent);
}

static void _traverseMssqlBeginDialogSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_beginDialogSqlNode * beginDialogSqlNode , NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, beginDialogSqlNode, container);

	_insertIntoList( sqlTraverser, beginDialogSqlNode->dialogHandle, parent);
	_insertIntoList( sqlTraverser, beginDialogSqlNode->initiatorServiceName, parent);
	_insertIntoList( sqlTraverser, beginDialogSqlNode->targetServiceName, parent);
}

static void _traverseMssqlBeginTranSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_beginTranSqlNode * beginTranSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, beginTranSqlNode, container);

	_insertIntoList( sqlTraverser, beginTranSqlNode->transactionName, parent);
	_insertIntoList( sqlTraverser, beginTranSqlNode->withMarkDescription, parent);
}

static void _traverseMssqlBulkInsertSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_bulkInsertSqlNode * bulkInsertSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, bulkInsertSqlNode, container);

	_insertIntoList( sqlTraverser, bulkInsertSqlNode->datafile, parent);
	_insertIntoList( sqlTraverser, bulkInsertSqlNode->tableName, parent);
}

static void _traverseMssqlDeallocateSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_deallocateSqlNode * deallocateSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, deallocateSqlNode, container);

	_insertIntoList( sqlTraverser, deallocateSqlNode->cursorName, parent);
}

static void _traverseMssqlDropDbObjectSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_dropDbObjectSqlNode * dropDbObjectSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, dropDbObjectSqlNode, container);

	if(dropDbObjectSqlNode->objectNames!=NULL){
		foreach(element, dropDbObjectSqlNode->objectNames){
			gsp_objectname *name = (gsp_objectname*)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, name, parent);
		}
	}
}

static void _traverseMssqlEndConversationSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_endConversationSqlNode * endConversationSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, endConversationSqlNode, container);

	_insertIntoList( sqlTraverser, endConversationSqlNode->conversationHandle, parent);
}

static void _traverseMssqlExecuteAsSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_executeAsSqlNode * executeAsSqlNode, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, executeAsSqlNode, container);
}

static void _traverseMssqlExceParameter( SqlTraverser * sqlTraverser, gsp_mssql_execParameter * execParameter, NodeContext * parent ) 
{
	_insertIntoList( sqlTraverser, execParameter, parent);
	_insertIntoList( sqlTraverser, execParameter->parameterName, parent);

	if(execParameter->parameterValue!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, execParameter->parameterValue, parent, esc_unknown));
	}
}

static void _traverseMssqlExecuteSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_executeSqlNode * executeSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, executeSqlNode, container);

	_insertIntoList( sqlTraverser, executeSqlNode->moduleName, parent);

	if(executeSqlNode->parameterList!=NULL){
		foreach(element, executeSqlNode->parameterList){
			gsp_mssql_execParameter *param = (gsp_mssql_execParameter*)gsp_list_celldata(element);
			_traverseMssqlExceParameter( sqlTraverser, param, parent);
		}
	}

	_insertIntoList( sqlTraverser, executeSqlNode->returnStatus, parent);

	if(executeSqlNode->stringValues!=NULL){
		foreach(element, executeSqlNode->stringValues){
			gsp_expr *stringValue = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, stringValue, parent, esc_unknown));
		}
	}
}

static void _traverseMssqlGoSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_goSqlNode * goSqlNode, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, goSqlNode, container);
}

static void _traverseMssqlLabelSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_labelSqlNode * labelSqlNode, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, labelSqlNode, container);
}

static void _traverseMssqlPrintSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_printSqlNode * printSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, printSqlNode, container);

	if(printSqlNode->vars!=NULL){
		foreach(element, printSqlNode->vars){
			gsp_expr *var = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, var, parent, esc_unknown));
		}
	}
}

static void _traverseMssqlRaiserrorSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_raiserrorSqlNode * raiserrorSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, raiserrorSqlNode, container);

	if(raiserrorSqlNode->msgs!=NULL){
		foreach(element, raiserrorSqlNode->msgs){
			gsp_expr *msg = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, msg, parent, esc_unknown));
		}
	}
}

static void _traverseMssqlRevertSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_revertSqlNode * revertSqlNode, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, revertSqlNode, container);
}

static void _traverseMssqlSendOnConversationSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_sendOnConversationSqlNode * sendOnConversationSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, sendOnConversationSqlNode, container);

	_insertIntoList( sqlTraverser, sendOnConversationSqlNode->conversationHandle, parent);
}

static void _traverseMssqlSetSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_setSqlNode * setSqlNode, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, setSqlNode, container);
	
	if(setSqlNode->exprList!=NULL){
		foreach(element, setSqlNode->exprList){
			gsp_expr *expr = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
		}
	}

	if(setSqlNode->selectSqlNode!=NULL){
		_traverseSelectSQL(sqlTraverser,  (gsp_selectStatement*)setSqlNode->selectSqlNode, parent);
	}

	_insertIntoList( sqlTraverser, setSqlNode->varName, parent);

	if(setSqlNode->varExpr!=NULL){
		_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, setSqlNode->varExpr, parent, esc_unknown));
	}
}

static void _traverseMssqlUpdateTextSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_updateTextSqlNode * updateTextSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, updateTextSqlNode, container);

	_insertIntoList( sqlTraverser, updateTextSqlNode->destColumnName, parent);
	_insertIntoList( sqlTraverser, updateTextSqlNode->destTextPtr, parent);
}

static void _traverseMssqlUseSqlNode( SqlTraverser * sqlTraverser, gsp_mssql_useSqlNode * useSqlNode, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, useSqlNode, container);

	_insertIntoList( sqlTraverser, useSqlNode->databaseName, parent);
}


static void _traverseMysqlIterateSqlNode( SqlTraverser * sqlTraverser, gsp_iterateSqlNode * iteratorStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, iteratorStmt, container);

	if(iteratorStmt->cursorName!=NULL){
		_insertIntoList( sqlTraverser, iteratorStmt->cursorName, parent);
	}
}

static void _traverseMysqlLeaveSqlNode( SqlTraverser * sqlTraverser, gsp_leaveSqlNode * leaveStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, leaveStmt, container);

	if(leaveStmt->cursorName!=NULL){
		_insertIntoList( sqlTraverser, leaveStmt->cursorName, parent);
	}
}

static void _traverseMysqlCallSqlNode( SqlTraverser * sqlTraverser, gsp_callSqlNode * callStmt, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, callStmt, container);

	if(callStmt->args!=NULL){
		foreach(element, callStmt->args){
			gsp_expr *expr = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
		}
	}

	if(callStmt->functionName!=NULL){
		_insertIntoList( sqlTraverser, callStmt->functionName, parent);
	}
}

static void _traverseInformixRenameColumnSqlNode( SqlTraverser * sqlTraverser, gsp_renameColumnSqlNode * renameColumnStmt, NodeContext * container ) 
{

	NodeContext *parent = _insertIntoList( sqlTraverser, renameColumnStmt, container);

	if(renameColumnStmt->columnName!=NULL){
		_insertIntoList( sqlTraverser, renameColumnStmt->columnName, parent);
	}

	if(renameColumnStmt->newColumnName!=NULL){
		_insertIntoList( sqlTraverser, renameColumnStmt->newColumnName, parent);
	}
}

static void _traverseInformixRenameSequenceSqlNode( SqlTraverser * sqlTraverser, gsp_renameSequenceSqlNode * renameSequenceStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, renameSequenceStmt, container);

	if(renameSequenceStmt->newSequenceName!=NULL){
		_insertIntoList( sqlTraverser, renameSequenceStmt->newSequenceName, parent);
	}

	if(renameSequenceStmt->sequenceName!=NULL){
		_insertIntoList( sqlTraverser, renameSequenceStmt->sequenceName, parent);
	}
}

static void _traverseInformixRenameTableSqlNode( SqlTraverser * sqlTraverser, gsp_renameTableSqlNode * renameTableStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, renameTableStmt, container);

	if(renameTableStmt->newTableName!=NULL){
		_insertIntoList( sqlTraverser, renameTableStmt->newTableName, parent);
	}

	if(renameTableStmt->tableName!=NULL){
		_insertIntoList( sqlTraverser, renameTableStmt->tableName, parent);
	}
}

static void _traverseInformixRenameIndexSqlNode( SqlTraverser * sqlTraverser, gsp_renameIndexSqlNode * renameIndexStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, renameIndexStmt, container);

	if(renameIndexStmt->newIndexName!=NULL){
		_insertIntoList( sqlTraverser, renameIndexStmt->newIndexName, parent);
	}

	if(renameIndexStmt->indexName!=NULL){
		_insertIntoList( sqlTraverser, renameIndexStmt->indexName, parent);
	}
}

static void _traverseInformixDropSequenceSqlNode( SqlTraverser * sqlTraverser, gsp_dropSequenceSqlNode * dropSequenceStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, dropSequenceStmt, container);

	if(dropSequenceStmt->sequenceName!=NULL){
		_insertIntoList( sqlTraverser, dropSequenceStmt->sequenceName, parent);
	}
}

static void _traverseInformixDropSynonymSqlNode( SqlTraverser * sqlTraverser, gsp_dropSynonymSqlNode * dropSynonymStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, dropSynonymStmt, container);

	if(dropSynonymStmt->synonymName!=NULL){
		_insertIntoList( sqlTraverser, dropSynonymStmt->synonymName, parent);
	}
}

static void _traverseInformixDropRowTypeSqlNode( SqlTraverser * sqlTraverser, gsp_dropRowTypeSqlNode * dropRowTypeStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, dropRowTypeStmt, container);

	if(dropRowTypeStmt->rowTypeName!=NULL){
		_insertIntoList( sqlTraverser, dropRowTypeStmt->rowTypeName, parent);
	}
}

static void _traverseInformixAlterIndexSqlNode( SqlTraverser * sqlTraverser, gsp_alterIndexSqlNode * alterIndexStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, alterIndexStmt, container);

	if(alterIndexStmt->indexName!=NULL){
		_insertIntoList( sqlTraverser, alterIndexStmt->indexName, parent);
	}
}

static void _traversePostGreSQLAlterSequenceSqlNode( SqlTraverser * sqlTraverser, gsp_alterSequenceSqlNode * alterSequenceStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, alterSequenceStmt, container);

	if(alterSequenceStmt->sequenceName!=NULL){
		_insertIntoList( sqlTraverser, alterSequenceStmt->sequenceName, parent);
	}
}

static void _traversePostGreSQLAlterViewSqlNode( SqlTraverser * sqlTraverser, gsp_alterViewSqlNode * alterViewStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, alterViewStmt, container);

	if(alterViewStmt->viewName!=NULL){
		_insertIntoList( sqlTraverser, alterViewStmt->viewName, parent);
	}
}

static void _traversePostgreExecuteSqlNode( SqlTraverser * sqlTraverser, gsp_executeSqlNode * executeStmt, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, executeStmt, container);

	if(executeStmt->moduleName!=NULL){
		_insertIntoList( sqlTraverser, executeStmt->moduleName, parent);
	}

	if(executeStmt->paramList!=NULL){
		foreach(element, executeStmt->paramList){
			gsp_expr *expr = (gsp_expr*)gsp_list_celldata(element);
			_process_expr(sqlTraverser,  _insertExprIntoList(sqlTraverser, expr, parent, esc_unknown));
		}
	}
}

static void _traversePostgreDropRoleSqlNode( SqlTraverser * sqlTraverser, gsp_dropRoleSqlNode * dropRoleStmt, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, dropRoleStmt, container);

	if(dropRoleStmt->roleNameList!=NULL){
		foreach(element, dropRoleStmt->roleNameList){
			gsp_objectname *objectName = (gsp_objectname*)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, objectName, parent);
		}
	}
}

static void _traversePostgreDropTriggerSqlNode( SqlTraverser * sqlTraverser, gsp_dropTriggerSqlNode * dropTriggerStmt, NodeContext * container ) 
{
	NodeContext *parent = _insertIntoList( sqlTraverser, dropTriggerStmt, container);

	_insertIntoList( sqlTraverser, dropTriggerStmt->tableName, parent);
	_insertIntoList( sqlTraverser, dropTriggerStmt->triggerName, parent);
}

static void _traversePostgreLockTableSqlNode( SqlTraverser * sqlTraverser, gsp_lockTableSqlNode * lockTableStmt, NodeContext * container ) 
{
	_insertIntoList( sqlTraverser, lockTableStmt, container);
}

static void _traverseTeradataCollectStatisticsSqlNode( SqlTraverser * sqlTraverser, gsp_collectStatisticsSqlNode * collectStatisticsStmt, NodeContext * container ) 
{
	gsp_listcell *element;

	NodeContext *parent = _insertIntoList( sqlTraverser, collectStatisticsStmt, container);

	if(collectStatisticsStmt->columnList!=NULL){
		foreach(element, collectStatisticsStmt->columnList){
			gsp_objectname *objectName = (gsp_objectname*)gsp_list_celldata(element);
			_insertIntoList( sqlTraverser, objectName, parent);
		}
	}

	_insertIntoList( sqlTraverser, collectStatisticsStmt->columnName, parent);
	_insertIntoList( sqlTraverser, collectStatisticsStmt->indexName, parent);
	_insertIntoList( sqlTraverser, collectStatisticsStmt->tableName, parent);
}

static void _traverseStatement(SqlTraverser *traverser, gsp_sql_statement *stmt, NodeContext *container){

	if(stmt == NULL || (stmt->stmt == NULL && stmt->parseTree == NULL))
		return;
	else{
		gsp_node* node = (gsp_node*)stmt->stmt;
		if(node == NULL){
			node = stmt->parseTree;
		}

		switch(stmt->stmtType){
		case sstselect:
			_traverseSelectSQL(traverser, (gsp_selectStatement *)node, container);
			break;
		case sstupdate:
			_traverseUpdateSQL(traverser, (gsp_updateStatement *)node, container);
			break;
		case sstdelete:
			_traverseDeleteSQL(traverser, (gsp_deleteStatement *)node, container);
			break;
		case sstinsert:
			_traverseInsertSQL(traverser, (gsp_insertStatement *)node, container);
			break;
		case sstmerge:
			_traverseMergeSQL(traverser, (gsp_mergeStatement *)node, container);
			break;
		case sstcreatetable:
			_traverseCreateTableSQL(traverser, (gsp_createTableStatement *)node, container);
			break;
		case sstaltertable:
			_traverseAlterTableSQL(traverser, (gsp_alterTableStatement *)node, container);
			break;
		case sstplsql_assignstmt:
			_traversePlsqlAssignStmt(traverser, (gsp_plsqlAssignStmt *)node, container);
			break;
		case sstplsql_block:
			_traverseBlockStmt(traverser, (gsp_blockStatement *)node, container);
			break;
		case sstplsql_casestmt:
			_traversePlsqlCaseStmt(traverser, (gsp_plsqlCaseStmt *)node, container);
			break;
		case sstplsql_closestmt:
			_traversePlsqlCloseStmt(traverser, (gsp_plsqlCloseStmt *)node, container);
			break;
		case sstplsql_createpackage:
			_traverseCreatePackage(traverser, (gsp_createPackageStatement *)node, container);
			break;
		case sstplsql_createprocedure:
			_traverseCreateProcedure(traverser, (gsp_createProcedureStatement *)node, container);
			break;
		case sstplsql_createfunction:
			_traverseCreateFunction(traverser, (gsp_createFunctionStatement *)node, container);
			break;
		case sstplsql_createtype:
			_traversePlsqlCreateType(traverser, (gsp_plsqlCreateType *)node, container);
			break;
		case sstplsql_createtypebody:
			_traversePlsqlCreateTypeBody(traverser, (gsp_plsqlCreateTypeBody *)node, container);
			break;
		case sstplsql_createtrigger:
			_traverseCreateTrigger(traverser, (gsp_createTriggerStatement *)node, container);
			break;
		case sstplsql_cursordecl:
			_traversePlsqlCursorDeclStmt(traverser, (gsp_plsqlCursorDeclStmt *)node, container);
			break;
		case sstplsql_elsifstmt:
			_traversePlsqlElsifStmt(traverser, (gsp_plsqlElsifStmt *)node, container);
			break;
		case sstplsql_exitstmt:
			_traversePlsqlExitStmt(traverser, (gsp_plsqlExitStmt *)node, container);
			break;
		case sstplsql_fetchstmt:
			_traversePlsqlFetchStmt(traverser, (gsp_plsqlFetchStmt *)node, container);
			break;
		case sstplsql_forallstmt:
			_traversePlsqlForallStmt(traverser, (gsp_plsqlForallStmt *)node, container);
			break;
		case sstplsql_gotostmt:
			_traversePlsqlGotoStmt(traverser, (gsp_plsqlGotoStmt *)node, container);
			break;
		case sstplsql_ifstmt:
			_traversePlsqlIfStmt(traverser, (gsp_plsqlIfStmt *)node, container);
			break;
		case sstplsql_loopstmt:
			_traversePlsqlLoopStmt(traverser, (gsp_plsqlLoopStmt *)node, container);
			break;
		case sstplsql_nullstmt:
			_traversePlsqlNullStmt(traverser, (gsp_plsqlNullStmt *)node, container);
			break;
		case sstplsql_openforstmt:
			_traversePlsqlOpenforStmt(traverser, (gsp_plsqlOpenforStmt *)node, container);
			break;
		case sstplsql_openstmt:
			_traversePlsqlOpenStmt(traverser, (gsp_plsqlOpenStmt *)node, container);
			break;
		case sstplsql_piperowstmt:
			_traversePlsqlPiperowStmt(traverser, (gsp_plsqlPipeRowStmt *)node, container);
			break;
		case sstplsql_procbasicstmt:
			_traversePlsqlBasicStmt(traverser, (gsp_plsqlBasicStmt *)node, container);
			break;
		case sstplsql_raisestmt:
			_traversePlsqlRaiseStmt(traverser, (gsp_plsqlRaiseStmt *)node, container);
			break;
		case sstplsql_recordtypedef:
			_traversePlsqlRecordTypedef(traverser, (gsp_plsqlRecordTypeDefStmt *)node, container);
			break;
		case sstplsql_returnstmt:
			_traversePlsqlReturnStmt(traverser, (gsp_plsqlReturnStmt *)node, container);
			break;
		case sstplsql_tabletypedef:
			_traversePlsqlTableTypeDefStmt(traverser, (gsp_plsqlTableTypeDefStmt *)node, container);
			break;
		case sstplsql_vardecl:
			_traversePlsqlVarDeclareStmt(traverser, (gsp_plsqlVarDeclStmt *)node, container);
			break;
		case sstplsql_varraytypedef:
			_traversePlsqlVarrayTypeDefStmt(traverser, (gsp_plsqlVarrayTypeDefStmt *)node, container);
			break;
		case sstdb2declarecursorstatement:
			_traverseDeclareSqlNode(traverser, (gsp_declareSqlNode *)node, container);
			break;
		case sstdb2callstmt:
			_traverseDB2CallStmtSqlNode(traverser, (gsp_db2_callStmtSqlNode *)node, container);
			break;
		case sstdb2forstmt:
			_traverseDB2ForSqlNode(traverser, (gsp_db2_forSqlNode *)node, container);
			break;
		case sstdb2gotostmt:
			_traverseDB2GotoSqlNode(traverser, (gsp_db2_gotoSqlNode *)node, container);
			break;
		case sstdb2iteratestmt:
			_traverseDB2IterateStmtSqlNode(traverser, (gsp_db2_iterateStmtSqlNode *)node, container);
			break;
		case sstdb2leavestmt:
			_traverseDB2LeaveStmtSqlNode(traverser, (gsp_db2_leaveStmtSqlNode *)node, container);
			break;
		case sstdb2loopstmt:
			_traverseDB2LoopSqlNode(traverser, (gsp_db2_loopSqlNode *)node, container);
			break;
		case sstdb2repeatstmt:
			_traverseDB2RepeatSqlNode(traverser, (gsp_db2_repeatSqlNode *)node, container);
			break;
		case sstdb2set:
			_traverseDB2SetSqlNode(traverser, (gsp_db2_setSqlNode *)node, container);
			break;
		case sstdb2whilestmt:
			_traverseDB2WhileSqlNode(traverser, (gsp_db2_whileSqlNode *)node, container);
			break;
		case sstmssqlbegindialog:
			_traverseMssqlBeginDialogSqlNode(traverser, (gsp_mssql_beginDialogSqlNode *)node, container);
			break;
		case sstmssqlbegintran:
			_traverseMssqlBeginTranSqlNode(traverser, (gsp_mssql_beginTranSqlNode *)node, container);
			break;
		case sstmssqlbulkinsert:
			_traverseMssqlBulkInsertSqlNode(traverser, (gsp_mssql_bulkInsertSqlNode *)node, container);
			break;
		case sstmssqlcreatefunction:
			_traverseCreateFunction(traverser, (gsp_createFunctionStatement *)node, container);
			break;
		case sstmssqldeallocate:
			_traverseMssqlDeallocateSqlNode(traverser, (gsp_mssql_deallocateSqlNode *)node, container);
			break;
		case sstmssqldeclare:
			_traverseDeclareSqlNode(traverser, (gsp_declareSqlNode *)node, container);
			break;
		case sstmssqldropdbobject:
			_traverseMssqlDropDbObjectSqlNode(traverser, (gsp_mssql_dropDbObjectSqlNode *)node, container);
			break;
		case sstmssqlendconversation:
			_traverseMssqlEndConversationSqlNode(traverser, (gsp_mssql_endConversationSqlNode *)node, container);
			break;
		case sstmssqlexecuteas:
			_traverseMssqlExecuteAsSqlNode(traverser, (gsp_mssql_executeAsSqlNode *)node, container);
			break;
		case sstmssqlexec:
			_traverseMssqlExecuteSqlNode(traverser, (gsp_mssql_executeSqlNode *)node, container);
			break;
		case sstmssqlgo:
			_traverseMssqlGoSqlNode(traverser, (gsp_mssql_goSqlNode *)node, container);
			break;
		case sstmssqlgoto:
			_traverseMssqlGotoSqlNode(traverser, (gsp_mssql_gotoSqlNode *)node, container);
			break;
		case sstmssqllabel:
			_traverseMssqlLabelSqlNode (traverser, (gsp_mssql_labelSqlNode *)node, container);
			break;
		case sstmssqlprint:
			_traverseMssqlPrintSqlNode(traverser, (gsp_mssql_printSqlNode *)node, container);
			break;
		case sstmssqlraiserror:
			_traverseMssqlRaiserrorSqlNode(traverser, (gsp_mssql_raiserrorSqlNode *)node, container);
			break;
		case sstmssqlrevert:
			_traverseMssqlRevertSqlNode(traverser, (gsp_mssql_revertSqlNode *)node, container);
			break;
		case sstmssqlsendonconversation:
			_traverseMssqlSendOnConversationSqlNode(traverser, (gsp_mssql_sendOnConversationSqlNode *)node, container);
			break;
		case sstmssqlset:
			_traverseMssqlSetSqlNode(traverser, (gsp_mssql_setSqlNode *)node, container);
			break;
		case sstmssqlupdatetext:
			_traverseMssqlUpdateTextSqlNode(traverser, (gsp_mssql_updateTextSqlNode *)node, container);
			break;
		case sstmssqluse:
			_traverseMssqlUseSqlNode(traverser, (gsp_mssql_useSqlNode *)node, container);
			break;
		case sstmysqliterate:
			_traverseMysqlIterateSqlNode(traverser, (gsp_iterateSqlNode *)node, container);
			break;
		case sstmysqlleave:
			_traverseMysqlLeaveSqlNode(traverser, (gsp_leaveSqlNode *)node, container);
			break;
		case sstmysqlcall:
			_traverseMysqlCallSqlNode(traverser, (gsp_callSqlNode *)node, container);
			break;
		case sstinformixRenameColumn:
			_traverseInformixRenameColumnSqlNode(traverser, (gsp_renameColumnSqlNode *)node, container);
			break;
		case sstinformixRenameSequence:
			_traverseInformixRenameSequenceSqlNode(traverser, (gsp_renameSequenceSqlNode *)node, container);
			break;
		case sstinformixRenameTable:
			_traverseInformixRenameTableSqlNode(traverser, (gsp_renameTableSqlNode *)node, container);
			break;
		case sstinformixRenameIndex:
			_traverseInformixRenameIndexSqlNode(traverser, (gsp_renameIndexSqlNode *)node, container);
			break;
		case sstinformixDropSequence:
			_traverseInformixDropSequenceSqlNode(traverser, (gsp_dropSequenceSqlNode *)node, container);
			break;
		case sstinformixDropSynonym:
			_traverseInformixDropSynonymSqlNode(traverser, (gsp_dropSynonymSqlNode *)node, container);
			break;
		case sstinformixDropRowType:
			_traverseInformixDropRowTypeSqlNode(traverser, (gsp_dropRowTypeSqlNode *)node, container);
			break;
		case sstinformixAlterIndex:
			_traverseInformixAlterIndexSqlNode(traverser, (gsp_alterIndexSqlNode *)node, container);
			break;
		case sstpostgresqlAlterSequence:
			_traversePostGreSQLAlterSequenceSqlNode(traverser, (gsp_alterSequenceSqlNode *)node, container);
			break;
		case sstpostgresqlAlterView:
			_traversePostGreSQLAlterViewSqlNode(traverser, (gsp_alterViewSqlNode *)node, container);
			break;
		case sstpostgresqlExecute:
			_traversePostgreExecuteSqlNode(traverser, (gsp_executeSqlNode *)node, container);
			break;
		case sstpostgresqlDropRole:
			_traversePostgreDropRoleSqlNode(traverser, (gsp_dropRoleSqlNode *)node, container);
			break;
		case sstpostgresqlDropTrigger:
			_traversePostgreDropTriggerSqlNode(traverser, (gsp_dropTriggerSqlNode *)node, container);
			break;
		case sstlocktable:
			_traversePostgreLockTableSqlNode(traverser, (gsp_lockTableSqlNode *)node, container);
			break;
		case sstteradatacollectstatistics:
			_traverseTeradataCollectStatisticsSqlNode(traverser, (gsp_collectStatisticsSqlNode *)node, container);
			break;
		default:
			//printf("%s", gsp_node_text((gsp_node*)node));
			break;
		}
	}
}

static void _disposeSQLInfoMap(Map *map){
	Iterator iter = map->getIterator(map);
	while(map->hasNext(map, &iter)){
		List* list = (List* )map->get(map, map->next(&iter));
		list->dispose(list);
	}
	map->dispose(map);
}

static void _disposeTableInfoMap(Map *map){
	if(map->containsKey(map,(void *)((int)tft_objectName))){
		List* list = (List* )map->get(map, (void *)((int)tft_objectName));
		list->dispose(list);
	}
	map->dispose(map);
}

static List* _traverseSQL(SqlTraverser *traverser, gsp_sql_statement *stmt){
	Map* sqlInfoMap = traverser->__sqlInfoMap;
	List* nodeList =  traverser->__nodeList;
	List* entries = traverser->__entries;
	Map* contextMap = traverser->__contextMap;
	Map* tableInfoMap = traverser->__tableInfoMap;
	List* unTableDeterminedList = traverser->__unTableDeterminedList;

	traverser->__parser = stmt->sqlparser;

	if(nodeList!=NULL){
		if(!sqlInfoMap->isEmpty(sqlInfoMap)){
			Iterator iter = sqlInfoMap->getIterator(sqlInfoMap);
			while(sqlInfoMap->hasNext(sqlInfoMap, &iter)){
				Map* map = (Map* )sqlInfoMap->get(sqlInfoMap, sqlInfoMap->next(&iter));
				_disposeSQLInfoMap(map);
			}
		}

		if(!entries->isEmpty(entries))
		{
			Iterator entryIter = entries->getIterator(entries);
			while(entries->hasNext(entries, &entryIter)){
				NodeContext *context = (NodeContext *)entries->next(&entryIter);
				free(context);
			}
		}

		if(!tableInfoMap->isEmpty(tableInfoMap)){
			Iterator iter = tableInfoMap->getIterator(tableInfoMap);
			while(tableInfoMap->hasNext(tableInfoMap, &iter)){
				Map* map = (Map* )tableInfoMap->get(tableInfoMap, tableInfoMap->next(&iter));
				_disposeTableInfoMap(map);
			}
		}

		nodeList->clear(nodeList);
		entries->clear(entries);
		sqlInfoMap->clear(sqlInfoMap);
		contextMap->clear(contextMap);
		tableInfoMap->clear(tableInfoMap);
		unTableDeterminedList->clear(unTableDeterminedList);
	}

	_traverseStatement(traverser, stmt, NULL);

	if(!sqlInfoMap->isEmpty(sqlInfoMap)){
		Iterator iter = sqlInfoMap->getIterator(sqlInfoMap);
		while(sqlInfoMap->hasNext(sqlInfoMap, &iter)){
			gsp_base_statement *key = (gsp_base_statement *)sqlInfoMap->next(&iter);
			Map *map = (Map *)sqlInfoMap->get(sqlInfoMap, key);
			if(map->containsKey(map, (void*)((int)sft_table))){
				List *tables = (List*)map->get(map, (void*)((int)sft_table));
				Iterator tableIter = tables->getIterator(tables);
				while(tables->hasNext(tables, &tableIter)){
					gsp_node *table = (gsp_node*)tables->next(&tableIter);
					_analyzeFromTable(traverser,key, table );
				}
			}
		}
	}

	if(!nodeList->isEmpty(nodeList)){
		Iterator iter = nodeList->getIterator(nodeList);
		while(nodeList->hasNext(nodeList, &iter)){
			gsp_node *node = (gsp_node *)nodeList->next(&iter);
			if(node->nodeType == t_gsp_expr){
				ExprContext *context = (ExprContext *)traverser->getContext(traverser, node);
				gsp_base_statement *baseStmt = (gsp_base_statement *)((NodeContext *)traverser->getContext(traverser, node)->parent)->self;
				_linkColumnReferenceToTable(traverser, baseStmt, context->self->objectOperand, context->clauseType);
			}
		}
		iter = nodeList->getIterator(nodeList);
		while(nodeList->hasNext(nodeList, &iter)){
			gsp_node *node = (gsp_node *)nodeList->next(&iter);
			if(node->nodeType == t_gsp_joinExpr){
				gsp_base_statement *baseStmt = (gsp_base_statement *)((NodeContext *)traverser->getContext(traverser, node)->parent)->self;
				gsp_joinExpr *joinItem = (gsp_joinExpr*)node;
				if(joinItem->usingColumns!=NULL){
					Map *map = (Map *)sqlInfoMap->get(sqlInfoMap, baseStmt);
					if(map->containsKey(map, (void*)((int)sft_table))){
						List *tables = (List*)map->get(map, (void*)((int)sft_table));
						int size = tables->size(tables);
						if(size>1){
							gsp_listcell *columnElement;
							foreach(columnElement, joinItem->usingColumns){
								gsp_objectname *column = ((gsp_objectname *)gsp_list_celldata(columnElement));
								if(column!=NULL){
									//Maybe here has a bug.
									List* left = _getObjectNameReferences(traverser, (gsp_node*)tables->get(tables, size-2));
									List* right = _getObjectNameReferences(traverser, (gsp_node*)tables->get(tables, size-1));
									left->add(left, column);
									right->add(right, column);
								}
							}
						}
					}
				}
			}
			else if(node->nodeType == t_gsp_updateStatement || node->nodeType == t_gsp_deleteStatement){
				gsp_base_statement *baseStmt = (gsp_base_statement *)node;
				Map *map = (Map *)sqlInfoMap->get(sqlInfoMap, baseStmt);
				if(map->containsKey(map, (void*)((int)sft_table))){
					int i, j, k;
					int *deletedTables;
					List *tables = (List*)map->get(map, (void*)((int)sft_table));
					deletedTables = (int *)malloc(sizeof(int)*tables->size(tables));
					for (i = 0; i < tables->size(tables); i++) {
						gsp_node *lcTable = (gsp_node *)tables->get(tables, i);
						if (_getTableAlias(lcTable) == NULL && _isBaseTable(traverser, lcTable) == TRUE) {
							char *tableName = gsp_node_text((gsp_node*)lcTable);
							for (j = 0; j < tables->size(tables); j++) {
								if (i != j) {
									gsp_node *lcTable2 = (gsp_node *)tables->get(tables, j);
									if (_getTableAlias(lcTable2) != NULL){
										char *alias = gsp_node_text((gsp_node*)_getTableAlias(lcTable2));
										if(compareToIgnoreCase(tableName, alias)==0){
											List* references = _getObjectNameReferences(traverser, lcTable);
											List* references2 = _getObjectNameReferences(traverser, lcTable2);
											deletedTables[i] = 1;
											for (k = 0; k < references->size(references); k++) {
												references2->add(references2, references->get(references, k));
											}
										}
										gsp_free(alias);
									}
								}
							}
							gsp_free(tableName);
						}
					}

					for (i = tables->size(tables) - 1; i >= 0; i--) {
						if (deletedTables[i] == 1) {
							tables->remove(tables, tables->get(tables, i));
						}
					}

					free(deletedTables);
				}
			}
		}
	}

	return nodeList;
}

static void _dispose(SqlTraverser *traverser){
	Map* sqlInfoMap = traverser->__sqlInfoMap;
	List* nodeList =  traverser->__nodeList;
	List* entries = traverser->__entries;
	Map* contextMap = traverser->__contextMap;
	Map* tableInfoMap = traverser->__tableInfoMap;
	List* unTableDeterminedList = traverser->__unTableDeterminedList;

	if(nodeList!=NULL){
		if(!sqlInfoMap->isEmpty(sqlInfoMap)){
			Iterator iter = sqlInfoMap->getIterator(sqlInfoMap);
			while(sqlInfoMap->hasNext(sqlInfoMap, &iter)){
				Map* map = (Map* )sqlInfoMap->get(sqlInfoMap, sqlInfoMap->next(&iter));
				_disposeSQLInfoMap(map);
			}
		}

		if(!entries->isEmpty(entries))
		{
			Iterator entryIter = entries->getIterator(entries);
			while(entries->hasNext(entries, &entryIter)){
				NodeContext *context = (NodeContext *)entries->next(&entryIter);
				free(context);
			}
		}

		if(!tableInfoMap->isEmpty(tableInfoMap)){
			Iterator iter = tableInfoMap->getIterator(tableInfoMap);
			while(tableInfoMap->hasNext(tableInfoMap, &iter)){
				Map* map = (Map* )tableInfoMap->get(tableInfoMap, tableInfoMap->next(&iter));
				_disposeTableInfoMap(map);
			}
		}

		nodeList->dispose(nodeList);
		entries->dispose(entries);
		sqlInfoMap->dispose(sqlInfoMap);
		contextMap->dispose(contextMap);
		tableInfoMap->dispose(tableInfoMap);
		unTableDeterminedList->dispose(unTableDeterminedList);
	}
	free(traverser);
}

SqlTraverser* createSqlTraverser(){
	SqlTraverser *traverser = (SqlTraverser *)malloc(sizeof(SqlTraverser));
	traverser->traverseSQL = _traverseSQL;
	traverser->getSQLInfo = _findSQLInfo;
	traverser->getContext = _findContext;
	traverser->dispose = _dispose;
	traverser->checkColumn = NULL;
	traverser->isTableDetermined = _isTableDetermined;
	traverser->getTableObjectNameReferences = _getObjectNameReferences;
	traverser->isBaseTable = _isBaseTable;

	traverser->__nodeList = createList();
	traverser->__entries = createList();
	traverser->__sqlInfoMap = createObjectMap();
	traverser->__contextMap = createObjectMap();
	traverser->__tableInfoMap = createObjectMap();
	traverser->__unTableDeterminedList = createList();
	return traverser;
}
