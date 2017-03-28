#include "expr_traverse.h"

static BOOL isLeafExpr(gsp_expr *e){
	BOOL ret = TRUE;
	if (e == NULL) return ret;
	switch (e->expressionType){
			case eet_case:
			case eet_simple_object_name:
			case eet_simple_constant:
			case eet_simple_source_token:
			case eet_group:
			case eet_list:
			case eet_cursor:
			case eet_function:
			case eet_type_constructor:
			case eet_subquery:
			case eet_multiset:
			case eet_object_access:
			case eet_place_holder:
			case eet_is_of_type:
			case eet_exists:
			case eet_arrayaccess:
			case eet_interval:
			case eet_new_structured_type:
			case eet_new_variant_type:
			case eet_member_of:
				return ret;
			default:;
	}

	if (e->leftOperand != NULL || e->rightOperand != NULL){
		ret = FALSE;
	}

	return ret;
}

gsp_walking_result preOrderTraverse (ExprTraverser* pTraverser, gsp_expr* pExpr){
	Stack *stack = createStack();
	gsp_walking_result visitResult = gsp_walking_continue;
	stack->push(stack, pExpr);

	while(stack->size(stack) > 0){
		gsp_expr* node = (gsp_expr*)stack->pop(stack);

		visitResult = pTraverser->exprVisit(pTraverser,node,isLeafExpr(node));
		if(visitResult == gsp_walking_abort){
			stack->dispose(stack);
			return visitResult;
		}
		else if(visitResult != gsp_walking_prune && !isLeafExpr(node)){
			if(node->rightOperand!=NULL){
				stack->push(stack, node->rightOperand);
			}
			if(node->leftOperand!=NULL){
				stack->push(stack, node->leftOperand);
			}
		}
	}
	stack->dispose(stack);
	return gsp_walking_continue;
}

gsp_walking_result inOrderTraverse (ExprTraverser *pTraverser, gsp_expr *pExpr){
	Stack *stack;
	gsp_walking_result visitResult = gsp_walking_continue;

	if (isLeafExpr(pExpr)){
		visitResult = pTraverser->exprVisit(pTraverser,pExpr,TRUE);
		return visitResult;
	}else{
		stack = createStack();
		stack->push(stack, pExpr);
	}

	while(stack->size(stack) > 0){
		gsp_expr* node = (gsp_expr*)stack->peek(stack);

		while(node != NULL){
			if (isLeafExpr(node)) {
				stack->push(stack, NULL);
			}else{
				stack->push(stack, node->leftOperand);
			}
			node = (gsp_expr*)stack->peek(stack);
		}

		// pop up the dummyOperator expression node
		stack->pop(stack);

		if (stack->size(stack) > 0){
			node = (gsp_expr*)stack->pop(stack);

			visitResult = pTraverser->exprVisit(pTraverser,node,isLeafExpr(node));
			if(visitResult == gsp_walking_abort){
				stack->dispose(stack);
				return visitResult;
			}

			if (isLeafExpr(node)) {
				stack->push(stack, NULL);
			}else{
				stack->push(stack, node->rightOperand);
			}
		}
	} //while
	stack->dispose(stack);
	return gsp_walking_continue;
}

gsp_walking_result postOrderTraverse(ExprTraverser *pTraverser, gsp_expr *pExpr){
	Memento* memento;
	Stack *stack;
	gsp_walking_result visitResult = gsp_walking_continue;
	int ctNone = 0;
	int ctL = 1;
	int ctR = 2;

	int *ctNone_p;
	int *ctL_p;
	int *ctR_p;

	ctNone_p = &ctNone;
	ctL_p = &ctL;
	ctR_p =&ctR;

	if (isLeafExpr(pExpr)){
		visitResult = pTraverser->exprVisit(pTraverser,pExpr,TRUE);
		return visitResult;
	}else{
		stack = createStack();
		memento = createMemento();
		stack->push(stack, memento->insert(memento, pExpr, ctNone_p));
	}

	while(stack->size(stack) > 0){
		Entry* entry = (Entry*)stack->peek(stack);
		gsp_expr* node = (gsp_expr*)entry->first;

		while(node != NULL){

			if (isLeafExpr(node)) {
				stack->push(stack, memento->insert(memento, NULL, ctNone_p));
			}else{
				stack->push(stack, memento->insert(memento, node->leftOperand, ctNone_p));
			}

			entry = (Entry*)stack->peek(stack);
			node = (gsp_expr*)entry->first;
			if (node != NULL){
				entry->second = ctL_p;
			}
		}

		// pop up the dummyOperator expression node
		stack->pop(stack);

		entry = (Entry*)stack->peek(stack);
		node = (gsp_expr*)entry->first;

		while((stack->size(stack) > 0) &&(entry->second == ctR_p)){
			entry = (Entry*)stack->pop(stack);
			node = (gsp_expr*)entry->first;
			entry->second = ctNone_p;

			visitResult = pTraverser->exprVisit(pTraverser,node,isLeafExpr(node));
			if(visitResult == gsp_walking_abort){
				stack->dispose(stack);
				memento->dispose(memento);
				return visitResult;
			}

			if (stack->size(stack) > 0){
				entry = (Entry*)stack->peek(stack);
				node = (gsp_expr*)entry->first;
			}else{
				break;
			}
		}

		if (stack->size(stack) > 0){
			entry = (Entry*)stack->peek(stack);
			node = (gsp_expr*)entry->first;
			entry->second = ctR_p;

			if (isLeafExpr(node)){
				stack->push(stack, memento->insert(memento, NULL, ctNone_p));
			}else{
				stack->push(stack, memento->insert(memento, node->rightOperand, ctNone_p));
			}
		}
	}//while

	stack->dispose(stack);
	memento->dispose(memento);
	return gsp_walking_continue;
}
