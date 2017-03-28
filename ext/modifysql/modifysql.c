#include "modifysql.h"  

static int disposeMemoryList(gsp_sqlparser* parser);
static Map* memoryMap = NULL;

static List* getMemoryList(gsp_sqlparser* parser)
{
	List* memoryList; 
	if(parser == NULL)
		return NULL;
	if(memoryMap == NULL){
		memoryMap = createMap(Map_KeyType_DEFAULT);
	}

	if(!memoryMap->containsKey(memoryMap, parser)){
		memoryList = createList();
		memoryMap->put(memoryMap, parser, memoryList);
		parser->xDispose = disposeMemoryList;
	}
	else{
		memoryList = (List*)memoryMap->get(memoryMap, parser);
	}

	return memoryList;
}

static int disposeMemoryList(gsp_sqlparser* parser)
{ 
	if(parser == NULL || memoryMap == NULL)
		return 1;

	if(memoryMap->containsKey(memoryMap, parser)){
		List* memoryList = (List*)memoryMap->get(memoryMap, parser);
		Iterator iter = memoryList->getIterator(memoryList);
		while(memoryList->hasNext(memoryList, &iter)){
			void* node = memoryList->next(&iter);
			if(node!=NULL){
				free(node);
			}
		}
		memoryMap->remove(memoryMap, parser);
		memoryList->dispose(memoryList);
	}

	if(memoryMap->isEmpty(memoryMap)){
		memoryMap->dispose(memoryMap);
		memoryMap = NULL;
	}

	return 0;
}

static void collectMallocMemroy(gsp_sqlparser* parser, void* memroy)
{ 
	List* memoryList = getMemoryList(parser);
	if(memoryList!=NULL && memroy!=NULL){
		memoryList->add(memoryList, memroy);
	}
}


static BOOL gsp_isInnerNode( gsp_node* outer, gsp_node* inner) 
{
	if(inner == NULL || outer == NULL)
		return FALSE;
	else{
		gsp_sourcetoken* outerStartToken = NULL;
		gsp_sourcetoken* outerEndToken = NULL;

		gsp_sourcetoken* innerStartToken = NULL;
		gsp_sourcetoken* innerEndToken = NULL;

		if (outer->nodeType == t_gsp_list)
		{
			if(((gsp_list*)outer)->length>0){
				outerStartToken = (gsp_list_first((gsp_list *)outer))->fragment.startToken;
				outerEndToken = (gsp_list_last((gsp_list *)outer))->fragment.endToken;
			}

		}else{
			outerStartToken = outer->fragment.startToken;
			outerEndToken = outer->fragment.endToken;
		}

		if (inner->nodeType == t_gsp_list)
		{
			if(((gsp_list*)inner)->length>0){
				innerStartToken = (gsp_list_first((gsp_list *)inner))->fragment.startToken;
				innerEndToken = (gsp_list_last((gsp_list *)inner))->fragment.endToken;
			}

		}else{
			innerStartToken = inner->fragment.startToken;
			innerEndToken = inner->fragment.endToken;
		}

		while( outerStartToken!=NULL && outerStartToken!=outerEndToken && outerStartToken->tokenStatus == ets_deleted){
			outerStartToken = outerStartToken->pNext;
		}

		if(outerStartToken == NULL || outerStartToken->tokenStatus == ets_deleted){
			outerStartToken = NULL;
			outerEndToken = NULL;
		}
		else{
			while(outerEndToken!=NULL && outerEndToken!=outerStartToken && outerEndToken->tokenStatus == ets_deleted){
				outerEndToken = outerEndToken->pPrev;
			}
		}

		while(innerStartToken!=NULL && innerStartToken!=innerEndToken && innerStartToken->tokenStatus == ets_deleted){
			innerStartToken = innerStartToken->pNext;
		}

		if(innerStartToken==NULL || innerStartToken->tokenStatus == ets_deleted){
			innerStartToken = NULL;
			innerEndToken = NULL;
		}
		else{
			while(innerEndToken->tokenStatus == ets_deleted && innerEndToken!=NULL && innerEndToken!=innerStartToken){
				innerEndToken = innerEndToken->pPrev;
			}
		}


		if(innerStartToken == NULL || innerEndToken == NULL 
			|| outerStartToken == NULL || outerEndToken == NULL)
			return FALSE;

		if(innerStartToken->nPosInList>=outerStartToken->nPosInList 
			&& innerEndToken->nPosInList<=outerEndToken->nPosInList){
				return TRUE;
		}
		else return FALSE;
	}
}

static BOOL gsp_isInNodeRange( gsp_node* outer, int startPos, int endPos) 
{
	if(outer == NULL || startPos<0 || endPos<0)
		return FALSE;
	else{
		gsp_sourcetoken* outerStartToken = NULL;
		gsp_sourcetoken* outerEndToken = NULL;

		if (outer->nodeType == t_gsp_list)
		{
			if(((gsp_list*)outer)->length>0){
				outerStartToken = (gsp_list_first((gsp_list *)outer))->fragment.startToken;
				outerEndToken = (gsp_list_last((gsp_list *)outer))->fragment.endToken;
			}

		}else{
			outerStartToken = outer->fragment.startToken;
			outerEndToken = outer->fragment.endToken;
		}

		while( outerStartToken!=NULL && outerStartToken!=outerEndToken && outerStartToken->tokenStatus == ets_deleted){
			outerStartToken = outerStartToken->pNext;
		}

		if(outerStartToken == NULL || outerStartToken->tokenStatus == ets_deleted){
			outerStartToken = NULL;
			outerEndToken = NULL;
		}
		else{
			while(outerEndToken!=NULL && outerEndToken!=outerStartToken && outerEndToken->tokenStatus == ets_deleted){
				outerEndToken = outerEndToken->pPrev;
			}
		}


		if(outerStartToken == NULL || outerEndToken == NULL)
			return FALSE;

		if(startPos>=outerStartToken->nPosInList 
			&& endPos<=outerEndToken->nPosInList){
				return TRUE;
		}
		else return FALSE;
	}
}

static void gsp_removeNodeSelf(gsp_node* node){
	gsp_node* preNode = node->pPrev;
	gsp_node* nextNode = node->pNext;

	if(preNode!=NULL){
		preNode->pNext = nextNode;
	}
	if(nextNode!=NULL){
		nextNode->pPrev = preNode;
	}
}

static void gsp_removeInnerNodes(gsp_node* outerNode){
	gsp_node *node = (gsp_node*)outerNode->pNext;
	while(node!=NULL){
		gsp_node *temp = node->pNext;
		if(gsp_isInnerNode(outerNode, node)){
			gsp_removeNodeSelf(node);
		}
		node = temp;
	}

	node = (gsp_node*)outerNode->pPrev;
	while(node!=NULL){
		gsp_node *temp = node->pPrev;
		if(gsp_isInnerNode(outerNode, node)){
			gsp_removeNodeSelf(node);
		}
		node = temp;
	}
}

static void gsp_removeNodeTokens( gsp_node* node ) 
{
	gsp_sourcetoken* startToken = NULL;
	gsp_sourcetoken* endToken = NULL;
	gsp_sourcetoken* preToken = NULL;
	gsp_sourcetoken* nextToken = NULL;
	gsp_sourcetoken* currentToken = NULL;
	int tokenCount;

	if (node->nodeType == t_gsp_list)
	{
		if(((gsp_list*)node)->length>0){
			startToken = (gsp_list_first((gsp_list *)node))->fragment.startToken;
			endToken = (gsp_list_last((gsp_list *)node))->fragment.endToken;
		}

	}else{
		startToken = node->fragment.startToken;
		endToken = node->fragment.endToken;
	}

	while(startToken!=NULL && startToken!=endToken && startToken->tokenStatus == ets_deleted ){
		startToken = startToken->pNext;
	}

	if(startToken==NULL || startToken->tokenStatus == ets_deleted){
		startToken = NULL;
		endToken = NULL;
		return;
	}
	else{
		while( endToken!=NULL && endToken!=startToken && endToken->tokenStatus == ets_deleted){
			endToken = endToken->pPrev;
		}
	}

	if(startToken == NULL || endToken == NULL)
		return;

	tokenCount = endToken->nPosInList - startToken->nPosInList + 1;

	preToken = startToken->pPrev;
	nextToken = endToken->pNext;
	if(preToken!=NULL && preToken->pNext == startToken){
		preToken->pNext = nextToken;
	}
	if(nextToken!=NULL && nextToken->pPrev == endToken){
		nextToken->pPrev = preToken;
		while(nextToken!=NULL){
			nextToken->nPosInList = nextToken->nPosInList-tokenCount;
			nextToken = nextToken->pNext;
		}
	}

	currentToken = startToken;
	while(currentToken!=NULL){
		nextToken = currentToken->pNext;
		if(currentToken == endToken){
			currentToken->tokenStatus = ets_deleted;
			break;
		}
		else{
			currentToken->tokenStatus = ets_deleted;
			currentToken = nextToken;
		}
	}
}

static void gsp_removeNode( gsp_node* node )
{
	if(node!=NULL){
		gsp_removeInnerNodes(node);
		gsp_removeNodeTokens(node);
		gsp_removeNodeSelf(node);
	}
}

static void adjustNodeTokensPosition(gsp_node* node, gsp_sourcetoken* endToken){
	gsp_sourcetoken *current;
	int increament;

	node->fragment.startToken->pPrev = endToken;
	node->fragment.endToken->pNext = endToken->pNext;

	increament = node->fragment.endToken->nPosInList - node->fragment.startToken->nPosInList + 1;

	current = node->fragment.startToken;
	while(current != node->fragment.endToken){
		current->nPosInList += endToken->nPosInList;
		current = current->pNext;
	}
	current->nPosInList += endToken->nPosInList;

	current = current->pNext;
	while(current!=NULL){
		current->nPosInList += increament;
		current = current->pNext;
	}

	endToken->pNext = node->fragment.startToken;
	if(node->fragment.endToken->pNext!=NULL){
		node->fragment.endToken->pNext->pPrev = node->fragment.endToken;
	}
} 

void gsp_addOrderBy( gsp_sqlparser *parser, gsp_selectStatement* select, char* text )
{
	gsp_orderBy *orderby = select->orderbyClause;

	if(orderby!=NULL && orderby->items!=NULL){
		gsp_list *items = orderby->items;
		gsp_orderByItem *lastItem = (gsp_orderByItem *)gsp_list_last(items);
		gsp_listcell *lastCell = items->tail;
		gsp_sourcetoken *endToken = lastItem->fragment.endToken;

		gsp_orderByItem *item = (gsp_orderByItem *)malloc(sizeof(gsp_orderByItem));
		gsp_listcell *cell = (gsp_listcell *)malloc(sizeof(gsp_listcell));

		CString* content = CStringNew();
		CStringAppend(content, "," );
		CStringAppend(content, text );

		collectMallocMemroy(parser, item);
		collectMallocMemroy(parser, cell);
		collectMallocMemroy(parser, content->buffer);

		cell->node = (gsp_node*)item;
		cell->pNext = lastCell->pNext;
		cell->pPrev = lastCell->pPrev;
		cell->nodeType = lastCell->nodeType;
		cell->fragment.startToken = NULL;
		cell->fragment.endToken = NULL;
		cell->fragment.text = NULL;

		cell->nextCell = NULL;
		lastCell->nextCell = cell;
		items->tail = cell;
		items->length+=1;
		item->pNext = NULL;
		item->pPrev = NULL;
		item->sortToken = NULL;
		item->nodeType = t_gsp_orderByItem;
		item->fragment.startToken = NULL;
		item->fragment.endToken = NULL;
		item->fragment.text = NULL;

		gsp_setNodeText(parser, (gsp_node*)item, content->buffer, FALSE);		
		adjustNodeTokensPosition((gsp_node*)item, endToken);

		item->pPrev = (gsp_node *)lastItem;
		item->pNext = lastItem->pNext;
		lastItem->pNext = (gsp_node*)item;
		if(item->pNext!=NULL)
			item->pNext->pPrev = (gsp_node*)item;

		{
			gsp_node *currentNode = (gsp_node*)orderby;

			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_orderByItem 
						|| node->nodeType == t_gsp_objectname
						||node->nodeType == t_gsp_expr){
						node = temp;
						continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_orderByItem 
						|| node->nodeType == t_gsp_objectname
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			currentNode->fragment.endToken = item->fragment.endToken;
		}

		CStringDeleteWithoutBuffer(content);
	}
	else{
		gsp_sourcetoken *endToken;
		gsp_node* currentNode;
		CString* content = CStringNew();
		BOOL isEndTerminate = FALSE;

		if(!startsWithIgnoreCase(trimString(text), "ORDER ")){
			CStringAppend(content, " ORDER BY ");
		}
		else
			CStringAppend(content, " ");
		CStringAppend(content, text);

		orderby = (gsp_orderBy *)malloc(sizeof(gsp_orderBy));
		orderby->nodeType = t_gsp_orderBy;
		orderby->fragment.startToken = NULL;
		orderby->fragment.endToken = NULL;
		orderby->fragment.text = NULL;
		orderby->items = NULL;
		orderby->pNext = NULL;
		orderby->pPrev = NULL;

		collectMallocMemroy(parser, orderby);
		collectMallocMemroy(parser, content->buffer);

		gsp_setNodeText(parser, (gsp_node*)orderby, content->buffer, FALSE);	

		if(select->groupByClause!=NULL){
			if(select->groupByClause->havingClause!=NULL){
				endToken = select->groupByClause->havingClause->fragment.endToken;
			}
			else{
				endToken = select->groupByClause->fragment.endToken;
			}
		}
		else if(select->whereCondition!=NULL){
			endToken = select->whereCondition->fragment.endToken;
		}
		else if(select->fromTableList!=NULL){
			endToken =  (gsp_list_last(select->fromTableList))->fragment.endToken;
		}
		else{
			endToken = select->fragment.endToken;
		}
		if(endToken->pNext == NULL)
			isEndTerminate = TRUE;

		adjustNodeTokensPosition((gsp_node*)orderby, endToken);

		currentNode = (gsp_node *)select;
		{	
			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = orderby->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = orderby->fragment.endToken;
				}
				node = temp;
			}

			if(isEndTerminate)
				currentNode->fragment.endToken = orderby->fragment.endToken;
		}

		select->orderbyClause = orderby;
		CStringDeleteWithoutBuffer(content);
	}
}

void gsp_addGroupBy( gsp_sqlparser *parser, gsp_selectStatement* select, char* text )
{
	gsp_groupBy *groupby = select->groupByClause;

	if(groupby!=NULL && groupby->items!=NULL){
		gsp_list *items = groupby->items;
		gsp_gruopByItem *lastItem = (gsp_gruopByItem *)gsp_list_last(items);
		gsp_listcell *lastCell = items->tail;
		gsp_sourcetoken *endToken = lastItem->fragment.endToken;

		gsp_gruopByItem *item = (gsp_gruopByItem *)malloc(sizeof(gsp_gruopByItem));
		gsp_listcell *cell = (gsp_listcell *)malloc(sizeof(gsp_listcell));

		CString* content = CStringNew();
		CStringAppend(content, "," );
		CStringAppend(content, text );

		collectMallocMemroy(parser, item);
		collectMallocMemroy(parser, cell);
		collectMallocMemroy(parser, content->buffer);

		cell->node = (gsp_node*)item;
		cell->pNext = lastCell->pNext;
		cell->pPrev = lastCell->pPrev;
		cell->nodeType = lastCell->nodeType;
		cell->fragment.startToken = NULL;
		cell->fragment.endToken = NULL;
		cell->fragment.text = NULL;

		cell->nextCell = NULL;
		lastCell->nextCell = cell;
		items->tail = cell;
		item->pNext = NULL;
		item->pPrev = NULL;
		item->aliasClause = NULL;
		item->groupingSet = NULL;
		item->rollupCube = NULL;
		item->expr = NULL;
		item->nodeType = t_gsp_gruopByItem;
		item->fragment.startToken = NULL;
		item->fragment.endToken = NULL;
		item->fragment.text = NULL;

		gsp_setNodeText(parser, (gsp_node*)item, content->buffer, FALSE);		
		adjustNodeTokensPosition((gsp_node*)item, endToken);

		item->pPrev = (gsp_node *)lastItem;
		item->pNext = lastItem->pNext;
		lastItem->pNext = (gsp_node*)item;
		if(item->pNext!=NULL)
			item->pNext->pPrev = (gsp_node*)item;

		if(groupby->havingClause == NULL)
		{
			gsp_node *currentNode = (gsp_node*)groupby;

			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_gruopByItem 
						|| node->nodeType == t_gsp_objectname
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_gruopByItem 
						|| node->nodeType == t_gsp_objectname
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			currentNode->fragment.endToken = item->fragment.endToken;
		}

		CStringDeleteWithoutBuffer(content);
	}
	else{
		gsp_sourcetoken *endToken;
		gsp_node* currentNode;
		CString* content = CStringNew();
		gsp_sourcetoken *havingToken = NULL;
		gsp_sourcetoken *currentToken;
		BOOL isEndTerminate = FALSE;

		if(!startsWithIgnoreCase(trimString(text), "GROUP ")){
			CStringAppend(content, " GROUP BY ");
		}
		else
			CStringAppend(content, " ");
		CStringAppend(content, text);

		groupby = (gsp_groupBy *)malloc(sizeof(gsp_groupBy));
		groupby->nodeType = t_gsp_groupBy;
		groupby->fragment.startToken = NULL;
		groupby->fragment.endToken = NULL;
		groupby->fragment.text = NULL;
		groupby->items = NULL;
		groupby->pNext = NULL;
		groupby->pPrev = NULL;
		groupby->havingClause = NULL;
		groupby->stBy = NULL;
		groupby->stGroup = NULL;
		groupby->stHaving = NULL;

		collectMallocMemroy(parser, groupby);
		collectMallocMemroy(parser, content->buffer);

		gsp_setNodeText(parser, (gsp_node*)groupby, content->buffer, FALSE);	

		currentToken = groupby->fragment.startToken;
		while(currentToken!=groupby->fragment.endToken){
			if(startsWithIgnoreCase(currentToken->pStr, "HAVING ")){
				havingToken = currentToken;
				break;
			}
			currentToken = currentToken->pNext;
		}
		groupby->stHaving = havingToken;

		if(select->orderbyClause!=NULL){
			endToken = select->orderbyClause->fragment.startToken->pPrev;
		}
		else if(select->whereCondition!=NULL){
			endToken = select->whereCondition->fragment.endToken;
		}
		else if(select->fromTableList!=NULL){
			endToken =  (gsp_list_last(select->fromTableList))->fragment.endToken;
		}
		else{
			endToken = select->fragment.endToken;
		}
		if(endToken->pNext == NULL)
			isEndTerminate = TRUE;

		adjustNodeTokensPosition((gsp_node*)groupby, endToken);

		currentNode = (gsp_node *)select;
		{	
			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = groupby->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = groupby->fragment.endToken;
				}
				node = temp;
			}

			if(isEndTerminate)
				currentNode->fragment.endToken = groupby->fragment.endToken;

		}

		if(havingToken!=NULL){
			gsp_expr *havingClause = (gsp_expr *)malloc(sizeof(gsp_expr));
			havingClause->nodeType = t_gsp_expr;
			currentToken = havingToken->pNext;
			while(currentToken!=NULL && strlen(trimString(currentToken->pStr))==0){
				currentToken = currentToken->pNext;
			}
			havingClause->fragment.startToken = currentToken;
			havingClause->fragment.endToken = groupby->fragment.endToken;
			havingClause->fragment.text = NULL;
			havingClause->pNext = (gsp_node*)groupby;
			havingClause->pPrev = (gsp_node*)select;

			collectMallocMemroy(parser, havingClause);
		}
		select->groupByClause = groupby;
		CStringDeleteWithoutBuffer(content);
	}
}


void gsp_addHavingClause( gsp_sqlparser *parser, gsp_selectStatement* select, char* text ){
	if(select->groupByClause == NULL)
		return;
	if(select->groupByClause->havingClause !=NULL)
		gsp_removeHavingClause(select);
	{
		gsp_sourcetoken *endToken;
		gsp_node* currentNode;
		CString* content = CStringNew();
		gsp_sourcetoken *havingToken = NULL;
		gsp_sourcetoken *currentToken;
		gsp_expr *havingClause;
		gsp_groupBy* groupby = select->groupByClause;

		if(!startsWithIgnoreCase(trimString(text), "HAVING ")){
			CStringAppend(content, " HAVING ");
		}
		else
			CStringAppend(content, " ");
		CStringAppend(content, text);

		havingClause = (gsp_expr *)malloc(sizeof(gsp_expr));
		havingClause->nodeType = t_gsp_expr;
		havingClause->fragment.startToken = NULL;
		havingClause->fragment.endToken = NULL;
		havingClause->fragment.text = NULL;
		havingClause->pNext = NULL;
		havingClause->pPrev = NULL;

		collectMallocMemroy(parser, havingClause);
		collectMallocMemroy(parser, content->buffer);

		gsp_setNodeText(parser, (gsp_node*)havingClause, content->buffer, FALSE);	

		currentToken = havingClause->fragment.startToken;
		while(currentToken!=havingClause->fragment.endToken){
			if(startsWithIgnoreCase(currentToken->pStr, "HAVING ")){
				havingToken = currentToken;
				break;
			}
			currentToken = currentToken->pNext;
		}
		groupby->stHaving = havingToken;
		groupby->havingClause = havingClause;
		endToken = groupby->fragment.endToken;

		adjustNodeTokensPosition((gsp_node*)havingClause, endToken);

		currentNode = (gsp_node *)groupby;
		{	
			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = havingClause->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = havingClause->fragment.endToken;
				}
				node = temp;
			}

			currentNode->fragment.endToken = havingClause->fragment.endToken;
		}

		if(havingToken!=NULL){
			havingClause->pNext = (gsp_node*)groupby;
			havingClause->pPrev = groupby->pPrev;
		}
		CStringDeleteWithoutBuffer(content);
	}
}

static gsp_sourcetoken* gsp_getNodeStartToken( gsp_node* node ) 
{
	gsp_sourcetoken* startToken = NULL;
	gsp_sourcetoken* endToken = NULL;

	if (node->nodeType == t_gsp_list)
	{
		if(((gsp_list*)node)->length>0){
			startToken = (gsp_list_first((gsp_list *)node))->fragment.startToken;
			endToken = (gsp_list_last((gsp_list *)node))->fragment.endToken;
		}

	}else{
		startToken = node->fragment.startToken;
		endToken = node->fragment.endToken;
	}

	while( startToken!=NULL && startToken!=endToken && startToken->tokenStatus == ets_deleted){
		startToken = startToken->pNext;
	}

	return startToken;
}

static gsp_sourcetoken* gsp_getNodeEndToken( gsp_node* node ) 
{
	gsp_sourcetoken* startToken = NULL;
	gsp_sourcetoken* endToken = NULL;

	if (node->nodeType == t_gsp_list)
	{
		if(((gsp_list*)node)->length>0){
			startToken = (gsp_list_first((gsp_list *)node))->fragment.startToken;
			endToken = (gsp_list_last((gsp_list *)node))->fragment.endToken;
		}

	}else{
		startToken = node->fragment.startToken;
		endToken = node->fragment.endToken;
	}

	while(endToken!=NULL && endToken!=startToken && endToken->tokenStatus == ets_deleted){
		endToken = endToken->pPrev;
	}

	return endToken;
}

void gsp_addWhereClause( gsp_sqlparser *parser, gsp_base_statement* stmt, char* text )
{
	gsp_selectStatement* select = NULL;
	gsp_updateStatement* update = NULL;
	gsp_deleteStatement* deleteStmt = NULL;
	gsp_whereClause *whereClause = NULL;

	if(stmt->nodeType == t_gsp_selectStatement){
		select = (gsp_selectStatement*)stmt;
		whereClause = select->whereCondition;
	}
	else if(stmt->nodeType == t_gsp_updateStatement){
		update = (gsp_updateStatement*)stmt;
		whereClause = update->whereCondition;
	}
	else if(stmt->nodeType == t_gsp_deleteStatement){
		deleteStmt = (gsp_deleteStatement*)stmt;
		whereClause = deleteStmt->whereCondition;
	}

	if(whereClause!=NULL && whereClause->condition!=NULL){
		gsp_expr *expr = whereClause->condition;
		int startPos = gsp_getNodeStartToken((gsp_node*)expr)->nPosInList;
		int endPos = gsp_getNodeEndToken((gsp_node*)expr)->nPosInList;
		gsp_sourcetoken* endToken = expr->fragment.endToken;
		CString* content = CStringNew();
		char* exprText = gsp_getNodeText((gsp_node*)whereClause->condition);
		CStringAppend(content, exprText);
		if(endsWithIgnoreCase(exprText, " ") || endsWithIgnoreCase(exprText, "\n") || endsWithIgnoreCase(exprText, "\t")){
			CStringAppend(content, "AND " );
		}
		else{
			CStringAppend(content, " AND " );
		}
		CStringAppend(content, text );

		gsp_setNodeText(parser, (gsp_node*)whereClause->condition, content->buffer, FALSE);

		{
			gsp_node *currentNode = (gsp_node*)expr;

			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInNodeRange(node, startPos, endPos)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = expr->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInNodeRange(node, startPos, endPos)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = expr->fragment.endToken;
				}
				node = temp;
			}
		}
		CStringDeleteWithoutBuffer(content);
	}
	else{
		gsp_sourcetoken *endToken = NULL;
		gsp_node* currentNode = NULL;
		CString* content = CStringNew();
		BOOL isEndTerminate = FALSE;

		if(!startsWithIgnoreCase(trimString(text), "WHERE ")){
			CStringAppend(content, " WHERE ");
		}
		else
			CStringAppend(content, " ");
		CStringAppend(content, text);

		whereClause = (gsp_whereClause *)malloc(sizeof(gsp_whereClause));
		whereClause->nodeType = t_gsp_whereClause;
		whereClause->fragment.startToken = NULL;
		whereClause->fragment.endToken = NULL;
		whereClause->fragment.text = NULL;
		whereClause->condition = NULL;
		whereClause->pNext = NULL;
		whereClause->pPrev = NULL;

		collectMallocMemroy(parser, whereClause);
		collectMallocMemroy(parser, content->buffer);

		gsp_setNodeText(parser, (gsp_node*)whereClause, content->buffer, FALSE);	

		if(select!=NULL){
			if(select->fromTableList!=NULL && select->fromTableList->length>0){
				endToken =  (gsp_list_last(select->fromTableList))->fragment.endToken;
			}
			else if(select->groupByClause!=NULL){
				endToken =  select->groupByClause->fragment.startToken->pPrev;
			}
			else if(select->orderbyClause!=NULL){
				endToken =  select->orderbyClause->fragment.startToken->pPrev;
			}
			else if(select->resultColumnList!=NULL && select->resultColumnList->length>0){
				endToken =  (gsp_list_last(select->resultColumnList))->fragment.endToken;
			}
			else{
				endToken = select->fragment.endToken;
			}
		}
		else if(update!=NULL){
			if(update->resultColumnList!=NULL && update->resultColumnList->length>0){
				endToken =  (gsp_list_last(update->resultColumnList))->fragment.endToken;
			}
			else{
				endToken = select->fragment.endToken;
			}
		}
		else if(deleteStmt!=NULL){
			endToken = deleteStmt->fragment.endToken;
		}

		if(endToken->pNext == NULL)
			isEndTerminate = TRUE;

		adjustNodeTokensPosition((gsp_node*)whereClause, endToken);

		currentNode = (gsp_node *)select;
		{	
			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = whereClause->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = whereClause->fragment.endToken;
				}
				node = temp;
			}

			if(isEndTerminate)
				currentNode->fragment.endToken = whereClause->fragment.endToken;
		}

		select->whereCondition = whereClause;
		CStringDeleteWithoutBuffer(content);	
	}
}

gsp_sourcetoken * gsp_getSelectFromToken( gsp_selectStatement* select ) 
{
	gsp_sourcetoken *endToken = select->selectToken;
	int index = 0;
	if(select->resultColumnList!=NULL && select->resultColumnList->length>0){
		endToken = (gsp_list_last(select->resultColumnList))->fragment.endToken;
	}

	if(endToken!=NULL)
		endToken = endToken->pNext;

	while(index == 0 && endToken!=NULL){
		if(startsWith(endToken->pStr, " ") || startsWith(endToken->pStr, "\n") || startsWith(endToken->pStr, "\t")){
			endToken = endToken->pNext;
		}
		else if(endToken->tokenStatus == ets_deleted){
			endToken = endToken->pNext;
		}
		else{
			index++;
			if(startsWithIgnoreCase(endToken->pStr, "FROM ")){
				return endToken;
			} 
		}
	}

	return NULL;
}

void gsp_addJoinItem(gsp_sqlparser *parser, gsp_selectStatement* select, char* text){
	if(select!=NULL && select->fromTableList!=NULL){
		gsp_list *items = select->fromTableList;

		gsp_fromTable *item = (gsp_fromTable *)malloc(sizeof(gsp_fromTable));
		gsp_listcell *cell = (gsp_listcell *)malloc(sizeof(gsp_listcell));
		gsp_fromTable *lastItem = NULL;
		gsp_listcell *lastCell = NULL;
		gsp_sourcetoken *endToken = NULL;
		gsp_sourcetoken *fromToken = NULL;

		CString* content = CStringNew();

		collectMallocMemroy(parser,item);
		collectMallocMemroy(parser, cell);

		fromToken = gsp_getSelectFromToken(select);

		if(select->fromTableList->length>0){
			lastItem = (gsp_fromTable *)gsp_list_last(items);
			lastCell = items->tail;
			endToken = lastItem->fragment.endToken;
		}
		else{
			if(fromToken!=NULL)
				endToken = fromToken;
			else if(select->resultColumnList!=NULL && select->resultColumnList->length>0){
				endToken = (gsp_list_last(select->resultColumnList))->fragment.endToken;
			}
			else{
				endToken = select->selectToken;
			}
		} 

		if(select->fromTableList->length>0)
			CStringAppend(content, "," );
		else if(fromToken == NULL){
			if(startsWithIgnoreCase(trimString(text), "FROM")){
				CStringAppend(content, " " );
			}
			else CStringAppend(content, " FROM " );
		}
		else 
			CStringAppend(content, " " );
		CStringAppend(content, text );

		collectMallocMemroy(parser, content->buffer);

		cell->node = (gsp_node*)item;
		if(lastCell!=NULL){
			cell->pNext = lastCell->pNext;
			cell->pPrev = lastCell->pPrev;
			cell->nodeType = lastCell->nodeType;
		}
		else{
			cell->pPrev = (gsp_node*)select->fromTableList;
			cell->pNext = NULL;
			cell->nodeType = t_gsp_listcell;
		}
		cell->fragment.startToken = NULL;
		cell->fragment.endToken = NULL;
		cell->fragment.text = NULL;

		cell->nextCell = NULL;
		if(lastCell!=NULL){
			lastCell->nextCell = cell;
		}
		else{
			items->head = cell;
		}
		items->tail = cell;
		items->length+=1;
		item->pNext = NULL;
		item->pPrev = NULL;
		item->aliasClause = NULL;
		item->tableExpr = NULL;
		item->functionCall = NULL;
		item->joinExpr = NULL;
		item->aliasClause = NULL;
		item->fromtableType = ets_objectname;
		item->nodeType = t_gsp_join;
		item->fragment.startToken = NULL;
		item->fragment.endToken = NULL;
		item->fragment.text = NULL;

		gsp_setNodeText(parser, (gsp_node*)item, content->buffer, FALSE);		
		adjustNodeTokensPosition((gsp_node*)item, endToken);

		if(lastItem!=NULL){
			item->pPrev = (gsp_node *)lastItem;
			item->pNext = lastItem->pNext;
			lastItem->pNext = (gsp_node*)item;
			if(item->pNext!=NULL)
				item->pNext->pPrev = (gsp_node*)item;
		}
		{
			gsp_node *currentNode = (gsp_node*)select->fromTableList;

			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_join 
						|| node->nodeType == t_gsp_objectname
						|| node->nodeType == t_gsp_functionCall
						|| node->nodeType == t_gsp_fromTable
						|| node->nodeType == t_gsp_table
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_join 
						|| node->nodeType == t_gsp_objectname
						|| node->nodeType == t_gsp_functionCall
						|| node->nodeType == t_gsp_fromTable
						|| node->nodeType == t_gsp_table
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}
		}

		CStringDeleteWithoutBuffer(content);
	}
}

void gsp_addResultColumn(gsp_sqlparser *parser, gsp_base_statement* stmt, char* text){

	gsp_selectStatement* select = NULL;
	gsp_updateStatement* update = NULL;
	gsp_list *resultColumnList = NULL;

	if(stmt->nodeType == t_gsp_selectStatement){
		select = (gsp_selectStatement*)stmt;
		resultColumnList = select->resultColumnList;
	}
	else if(stmt->nodeType == t_gsp_updateStatement){
		update = (gsp_updateStatement*)stmt;
		resultColumnList = update->resultColumnList;
	}

	if(resultColumnList!=NULL){
		gsp_list *items = resultColumnList;

		gsp_resultColumn *item = (gsp_resultColumn *)malloc(sizeof(gsp_resultColumn));
		gsp_listcell *cell = (gsp_listcell *)malloc(sizeof(gsp_listcell));
		gsp_resultColumn *lastItem = NULL;
		gsp_listcell *lastCell = NULL;
		gsp_sourcetoken *endToken = NULL;

		CString* content = CStringNew();
		if(items->length>0)
			CStringAppend(content, "," );
		else 
			CStringAppend(content, " " );
		CStringAppend(content, text );

		collectMallocMemroy(parser, item);
		collectMallocMemroy(parser, cell);
		collectMallocMemroy(parser, content->buffer);

		if(items->length>0){
			lastItem = (gsp_resultColumn *)gsp_list_last(items);
			lastCell = items->tail;
			endToken = lastItem->fragment.endToken;
		}
		else{
			if(select!=NULL)
				endToken = select->selectToken;
			else if(update!=NULL)
				endToken = update->updateToken;
		} 
		cell->node = (gsp_node*)item;
		if(lastCell!=NULL){
			cell->pNext = lastCell->pNext;
			cell->pPrev = lastCell->pPrev;
			cell->nodeType = lastCell->nodeType;
		}
		else{
			cell->pPrev = (gsp_node*)items;
			cell->pNext = NULL;
			cell->nodeType = t_gsp_listcell;
		}
		cell->fragment.startToken = NULL;
		cell->fragment.endToken = NULL;
		cell->fragment.text = NULL;

		cell->nextCell = NULL;
		if(lastCell!=NULL){
			lastCell->nextCell = cell;
		}
		else{
			items->head = cell;
		}
		items->tail = cell;
		items->length+=1;
		item->pNext = NULL;
		item->pPrev = NULL;
		item->aliasClause = NULL;
		item->expr = NULL;
		item->nodeType = t_gsp_resultColumn;
		item->fragment.startToken = NULL;
		item->fragment.endToken = NULL;
		item->fragment.text = NULL;

		gsp_setNodeText(parser, (gsp_node*)item, content->buffer, FALSE);		
		adjustNodeTokensPosition((gsp_node*)item, endToken);

		if(lastItem!=NULL){
			item->pPrev = (gsp_node *)lastItem;
			item->pNext = lastItem->pNext;
			lastItem->pNext = (gsp_node*)item;
			if(item->pNext!=NULL)
				item->pNext->pPrev = (gsp_node*)item;
		}
		{
			gsp_node *currentNode = (gsp_node*)items;

			gsp_node *node = currentNode->pNext;
			while(node!=NULL){
				gsp_node *temp = node->pNext;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_resultColumn 
						|| node->nodeType == t_gsp_objectname
						|| node->nodeType == t_gsp_functionCall
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}

			node = (gsp_node*)currentNode->pPrev;
			while(node!=NULL){
				gsp_node *temp = node->pPrev;
				if(gsp_isInnerNode(node, currentNode)){
					if(node->nodeType == t_gsp_resultColumn 
						|| node->nodeType == t_gsp_objectname
						|| node->nodeType == t_gsp_functionCall
						||node->nodeType == t_gsp_expr){
							node = temp;
							continue;
					}
					if(node->fragment.endToken == endToken)
						node->fragment.endToken = item->fragment.endToken;
				}
				node = temp;
			}
		}

		CStringDeleteWithoutBuffer(content);
	}
}

static gsp_expr* gsp_getParentExpr( gsp_expr* expr ) 
{
	gsp_node *node = (gsp_node*)expr->pNext;
	while(node!=NULL){
		if(gsp_isInnerNode(node, (gsp_node*)expr)){
			if(node->nodeType == t_gsp_expr || node->nodeType == t_gsp_functionCall){
				gsp_expr *parent = (gsp_expr*)node;
				if(parent->leftOperand == expr || parent->rightOperand == expr)
					return parent;
			}
		}
		node = node->pNext;
	}

	node = (gsp_node*)expr->pPrev;
	while(node!=NULL){
		if(gsp_isInnerNode(node, (gsp_node*)expr)){
			if(node->nodeType == t_gsp_expr || node->nodeType == t_gsp_functionCall){
				gsp_expr *parent = (gsp_expr*)node;
				if(parent->leftOperand == expr || parent->rightOperand == expr)
					return parent;
			}
		}
		node = node->pPrev;
	}

	return NULL;
}

void gsp_removeExpression(gsp_expr* expr){
	gsp_expr* parent = gsp_getParentExpr(expr);
	if(parent == NULL){
		gsp_removeNode((gsp_node*)expr);
	}
	else if(parent->expressionType == eet_logical_and || parent->expressionType == eet_logical_or){
		gsp_removeNode((gsp_node*)expr);
		gsp_removeSourceToken(parent->operatorToken);
		if((parent->leftOperand->fragment.startToken!=NULL && parent->leftOperand->fragment.startToken->tokenStatus!=ets_deleted )
			|| (parent->rightOperand->fragment.startToken!=NULL && parent->rightOperand->fragment.startToken->tokenStatus!=ets_deleted)){
				return;
		}
		if(parent->fragment.endToken->pNext!=NULL && parent->fragment.endToken->pNext->pPrev!=NULL)
			parent->fragment.endToken = parent->fragment.endToken->pNext->pPrev; 
		gsp_removeExpression(parent);
	}
	else if(parent->expressionType == eet_parenthesis){
		gsp_removeExpression(parent);
	}
	else{
		gsp_removeNode((gsp_node*)expr);
	}
}

void gsp_removeOrderBy( gsp_selectStatement* select )
{
	if(select!=NULL){
		gsp_sourcetoken* newEndToken = select->orderbyClause->fragment.startToken->pNext;
		gsp_sourcetoken* endToken = select->orderbyClause->fragment.endToken;
		gsp_removeNode((gsp_node*)select->orderbyClause);
		select->orderbyClause = NULL;
		if(select->fragment.endToken == endToken){
			select->fragment.endToken = newEndToken;
		}
	}
}

void gsp_removeGroupBy( gsp_selectStatement* select )
{
	if(select!=NULL){
		gsp_sourcetoken* newEndToken = select->groupByClause->fragment.startToken->pNext;
		gsp_sourcetoken* endToken = select->groupByClause->fragment.endToken;
		gsp_removeNode((gsp_node*)select->groupByClause);
		select->groupByClause = NULL;
		if(select->fragment.endToken == endToken){
			select->fragment.endToken = newEndToken;
		}
	}
}

void gsp_removeHavingClause( gsp_selectStatement* select )
{
	if(select!=NULL){
		gsp_groupBy* groupBy = (gsp_groupBy*)select->groupByClause;
		if(groupBy!=NULL && groupBy->havingClause!=NULL){
			gsp_sourcetoken* endToken = groupBy->stHaving->pPrev;
			gsp_removeNode((gsp_node*)groupBy->havingClause);
			groupBy->fragment.endToken = endToken;
			gsp_removeSourceToken(groupBy->stHaving);
			groupBy->havingClause = NULL;
			groupBy->stHaving = NULL;
		}
	}
}

void gsp_removeWhereClause( gsp_base_statement* stmt )
{
	gsp_selectStatement* select = NULL;
	gsp_updateStatement* update = NULL;
	gsp_deleteStatement* deleteStmt = NULL;
	gsp_whereClause *whereClause = NULL;

	if(stmt->nodeType == t_gsp_selectStatement){
		select = (gsp_selectStatement*)stmt;
		whereClause = select->whereCondition;
	}
	else if(stmt->nodeType == t_gsp_updateStatement){
		update = (gsp_updateStatement*)stmt;
		whereClause = update->whereCondition;
	}
	else if(stmt->nodeType == t_gsp_deleteStatement){
		deleteStmt = (gsp_deleteStatement*)stmt;
		whereClause = deleteStmt->whereCondition;
	}

	if(whereClause){
		gsp_sourcetoken* newEndToken = whereClause->fragment.startToken->pNext;
		gsp_sourcetoken* endToken = whereClause->fragment.endToken;
		gsp_removeNode( (gsp_node*)whereClause);
		if(select!=NULL){
			select->whereCondition = NULL;
			if(select->fragment.endToken == endToken){
				select->fragment.endToken = newEndToken;
			}
		}
		else if(update!=NULL){
			update->whereCondition = NULL;
			if(update->fragment.endToken == endToken){
				update->fragment.endToken = newEndToken;
			}
		}
		else if(deleteStmt!=NULL){
			deleteStmt->whereCondition = NULL;
			if(deleteStmt->fragment.endToken == endToken){
				deleteStmt->fragment.endToken = newEndToken;
			}
		}
	}
}

void gsp_removeBeforeSpaceTokens( gsp_node * node )
{
	gsp_sourcetoken* startToken = node->fragment.startToken->pPrev;
	while(startToken!=NULL){
		if(startsWith(startToken->pStr, " ") || startsWith(startToken->pStr, "\n") || startsWith(startToken->pStr, "\t")){
			gsp_sourcetoken *currentToken = startToken;
			startToken = startToken->pPrev;
			gsp_removeSourceToken(currentToken);
		}
		else if(startToken->tokenStatus == ets_deleted){
			startToken = startToken->pPrev;
		}
		else
			break;
	}
	node->fragment.startToken->pPrev = startToken;
}

static void gsp_removeListItem(gsp_base_statement* stmt, gsp_list* list , int itemIndex){
	if(stmt!=NULL && list!=NULL && itemIndex>=0 && itemIndex<list->length){
		int size = list->length;
		gsp_sourcetoken *commaToken = NULL;

		gsp_listcell *cell;
		gsp_listcell *firstCell = NULL;
		gsp_listcell *lastCell = NULL;
		gsp_listcell *nodeCell = NULL;
		gsp_listcell *preCell = NULL;
		gsp_listcell *nextCell = NULL;

		gsp_node *node = NULL;
		int index = 0;
		foreach(cell, list){
			if(index == 0){
				firstCell = cell;
			}
			if(index == list->length-1){
				lastCell = cell;
			}

			if(index == itemIndex){
				node = (gsp_node *)gsp_list_celldata(cell);
				nodeCell = cell;
			}
			else if(index == itemIndex-1){
				preCell = cell;
			}
			else if(index == itemIndex+1){
				nextCell = cell;
			}
			index++;
		}

		if(size>1){
			if(itemIndex!=size-1){
				gsp_sourcetoken* endToken = node->fragment.endToken->pNext;
				index = 0;
				while(index == 0 && endToken!=NULL){
					if(startsWith(endToken->pStr, " ") || startsWith(endToken->pStr, "\n") || startsWith(endToken->pStr, "\t")){
						endToken = endToken->pNext;
					}
					else if(endToken->tokenStatus == ets_deleted){
						endToken = endToken->pNext;
					}
					else{
						index++;
						if(startsWith(endToken->pStr, ",")){
							commaToken = endToken;
						} 
					}
				}
			}
			else{
				gsp_sourcetoken* startToken = node->fragment.startToken->pPrev;
				index = 0;
				while(index == 0 && startToken!=NULL){
					if(startsWith(startToken->pStr, " ") || startsWith(startToken->pStr, "\n") || startsWith(startToken->pStr, "\t")){
						startToken = startToken->pPrev;
					}
					else if(startToken->tokenStatus == ets_deleted){
						startToken = startToken->pPrev;
					}
					else{
						index++;
						if(startsWith(startToken->pStr, ",")){
							commaToken = startToken;
						} 
					}
				}
			}
		}

		if(commaToken!=NULL){
			gsp_removeSourceToken(commaToken);
		}

		if(node!=NULL){
			gsp_sourcetoken* endToken = node->fragment.endToken;
			if(commaToken!=NULL){
				gsp_removeBeforeSpaceTokens(node);
			}
			gsp_removeNode( node );
			if(stmt->fragment.endToken == endToken){
				if(commaToken!=NULL){
					stmt->fragment.endToken = commaToken->pPrev;
				}
				else{
					stmt->fragment.endToken = node->fragment.startToken->pPrev;
				}
			}

			list->length--;

			if(firstCell == nodeCell){
				list->head = nextCell;
				if(list->length == 0)
					list->tail = NULL;
			}
			if(lastCell == nodeCell){
				list->tail = preCell;
				if(preCell!=NULL)
					preCell->nextCell = NULL;
			}
			if(preCell!=NULL)
				preCell->nextCell = nextCell;
		}
	}
}

void gsp_removeJoinItem(gsp_selectStatement* select, int joinIndex){
	gsp_removeListItem((gsp_base_statement*)select, select->fromTableList, joinIndex);
}

void gsp_removeResultColumn(gsp_base_statement* stmt, int columnIndex){
	if(stmt->nodeType == t_gsp_selectStatement)
		gsp_removeListItem(stmt, ((gsp_selectStatement*)stmt)->resultColumnList, columnIndex);
	else if(stmt->nodeType == t_gsp_updateStatement)
		gsp_removeListItem(stmt, ((gsp_updateStatement*)stmt)->resultColumnList, columnIndex);
}

void gsp_removeOrderByItem(gsp_selectStatement* select, int orderByIndex){
	if(select->orderbyClause->items->length>1)
		gsp_removeListItem((gsp_base_statement*)select, select->orderbyClause->items, orderByIndex);
	else
		gsp_removeOrderBy(select);
}

void gsp_removeGroupByItem(gsp_selectStatement* select, int groupByIndex){
	if(select->groupByClause->items->length>1)
		gsp_removeListItem((gsp_base_statement*)select, select->orderbyClause->items, groupByIndex);
	else
		gsp_removeGroupBy(select);
}

int gsp_setNodeText( gsp_sqlparser *parser, gsp_node* node, char* text, BOOL forceUpdateOuterNode )
{
	gsp_sqlparser *subParser;
	int rc = gsp_parser_create(parser->db,&subParser);
	if (rc){
		return rc;
	}
	rc = gsp_tokenize(subParser, text);
	if(rc!=0){
		gsp_free(subParser);
		return rc;
	}
	else {
		gsp_sourcetoken *tokenList = subParser->sourcetokenlist;
		int count = subParser->number_of_token;

		gsp_sourcetoken* startToken = NULL;
		gsp_sourcetoken* endToken = NULL;
		gsp_sourcetoken* lastToken = NULL;
		gsp_sourcetoken* newEndToken = NULL;
		int startIndex;
		int endIndex;
		int tokenCount;
		int increament;
		int index;

		if (node->nodeType == t_gsp_list && ((gsp_list*)node)->length>0)
		{
			startToken = (gsp_list_first((gsp_list *)node))->fragment.startToken;
			endToken = (gsp_list_last((gsp_list *)node))->fragment.endToken;

		}else{
			startToken = node->fragment.startToken;
			endToken = node->fragment.endToken;
		}

		if(startToken==NULL)
			startIndex = 1;
		else
			startIndex = startToken->nPosInList;
		if(endToken == NULL)
			endIndex = 1;
		else
			endIndex = endToken->nPosInList;

		tokenCount = endIndex - startIndex + 1;

		if(startToken == NULL)
			lastToken = NULL;
		else 
			lastToken = startToken->pPrev;

		increament = count-tokenCount;

		for(index=0;index<count;index++){
			gsp_sourcetoken* token = (gsp_sourcetoken*)malloc(sizeof(gsp_sourcetoken));
			collectMallocMemroy(parser, token);

			if(index == 0){
				node->fragment.startToken = token;
			}
			token->nPosInList = startIndex+index;
			token->pStr = tokenList[index].pStr;
			token->nStrLen = tokenList[index].nStrLen;
			token->nCode = tokenList[index].nCode;
			token->tokenStatus = tokenList[index].tokenStatus;
			token->pPrev = lastToken;
			if(lastToken!=NULL){
				lastToken->pNext = token;
			}
			lastToken = token;
		}

		newEndToken = lastToken;

		if(endToken != NULL)
			lastToken->pNext = endToken->pNext;
		else 
			lastToken->pNext = NULL;

		if(lastToken->pNext!=NULL){
			lastToken->pNext->pPrev = lastToken;
		}
		node->fragment.endToken = lastToken;

		if(increament!=0){
			while(lastToken->pNext!=NULL){
				lastToken = lastToken->pNext;
				lastToken->nPosInList = lastToken->nPosInList+increament;
			}
		}

		if(forceUpdateOuterNode){
			gsp_node *currentNode = node->pNext;
			while(currentNode!=NULL){
				gsp_node *temp = currentNode->pNext;
				if(gsp_isInnerNode(currentNode, node)){
					if(currentNode->fragment.endToken == endToken)
						currentNode->fragment.endToken = newEndToken;
				}
				currentNode = temp;
			}

			currentNode = (gsp_node*)node->pPrev;
			while(currentNode!=NULL){
				gsp_node *temp = currentNode->pPrev;
				if(gsp_isInnerNode(currentNode, node)){
					if(currentNode->fragment.endToken == endToken)
						currentNode->fragment.endToken = newEndToken;
				}
				currentNode = temp;
			}
		}
	}
	gsp_free(subParser);
	return rc;
}

void gsp_setSourceTokenText( gsp_sourcetoken* token, char* text )
{
	if(text!=NULL){
		token->pStr = text;
		token->nStrLen = strlen(token->pStr);
	}
	else{
		token->pStr = NULL;
		token->nStrLen = 0;
		gsp_removeSourceToken(token);
	}
}

void gsp_removeSourceToken( gsp_sourcetoken* token )
{
	if(token == NULL){
		return;
	}
	else{
		gsp_sourcetoken* preToken = token->pPrev;
		gsp_sourcetoken* nextToken = token->pNext;
		if(token->tokenStatus == ets_deleted)
			return;
		if(preToken!=NULL && preToken->pNext == token){
			preToken->pNext = nextToken;
		}
		if(nextToken!=NULL && nextToken->pPrev == token ){
			nextToken->pPrev = preToken;
			while(nextToken!=NULL){
				nextToken->nPosInList = nextToken->nPosInList-1;
				nextToken = nextToken->pNext;
			}
		}
		token->tokenStatus = ets_deleted;
	}
}

char* gsp_getSourceTokenText(gsp_sourcetoken* token){
	CString* content = CStringNew();
	char* rc; 
	CStringNAppend(content, token->pStr, token->nStrLen);
	rc = content->buffer;
	CStringDeleteWithoutBuffer(content);
	return rc;
}

char* gsp_getNodeText( gsp_node* node )
{
	gsp_sourcetoken* startToken = NULL;
	gsp_sourcetoken* endToken = NULL;
	gsp_sourcetoken* currentToken = NULL;
	CString* content;
	char* rc; 

	if(node == NULL)
		return NULL;
	if (node->nodeType == t_gsp_list)
	{
		if(((gsp_list*)node)->length>0){
			startToken = (gsp_list_first((gsp_list *)node))->fragment.startToken;
			endToken = (gsp_list_last((gsp_list *)node))->fragment.endToken;
		}

	}else{
		startToken = node->fragment.startToken;
		endToken = node->fragment.endToken;
	}

	currentToken = startToken;
	if(currentToken == NULL)
		return NULL;


	while( startToken!=NULL && startToken!=endToken && startToken->tokenStatus == ets_deleted){
		startToken = startToken->pNext;
	}

	if(startToken == NULL || startToken->tokenStatus == ets_deleted){
		startToken = NULL;
		endToken = NULL;
		return NULL;
	}
	else{
		while(endToken!=NULL && endToken!=startToken && endToken->tokenStatus == ets_deleted){
			endToken = endToken->pPrev;
		}

		if(endToken == NULL || endToken->tokenStatus == ets_deleted){
			startToken = NULL;
			endToken = NULL;
			return NULL;
		}

		content = CStringNew();

		if(currentToken->tokenStatus!=ets_deleted)
			CStringNAppend(content, currentToken->pStr, currentToken->nStrLen);

		while(currentToken != endToken && currentToken->pNext!=NULL){
			currentToken = currentToken->pNext;
			if(currentToken->tokenStatus!=ets_deleted)
				CStringNAppend(content, currentToken->pStr, currentToken->nStrLen);
			if(currentToken == endToken)
				break;
		}

		rc = content->buffer;
		CStringDeleteWithoutBuffer(content);
		return rc;
	}
}
