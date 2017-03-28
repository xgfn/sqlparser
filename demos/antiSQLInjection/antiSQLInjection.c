/*!
*  \brief     Convert Oracle proprietary joins to ANSI SQL compliant joins.
*
*  \details   This demo is used to demonstrate how to convert Oracle proprietary 
*             joins to ANSI SQL compliant joins.
*  \author    cnfree2000@hotmail.com
*  \version   1a
*  \date      2013
*  \pre       need to compile with core parser and extension library.
*  \copyright Gudu Software
*/

/*!
**  \file antiSQLInjection.c
**
*/

#include "expr_traverse.h"
#include "gsp_base.h"
#include "gsp_node.h"
#include "gsp_list.h"
#include "gsp_sourcetoken.h"
#include "gsp_sqlparser.h"
#include "antiSQLInjection.h"
#include "cstring.h"
#include "modifysql.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

static BOOL isInjected_always_false_condition( TAntiSQLInjection* antiSQLInjection ); 
static BOOL isInjected_always_true_condition( TAntiSQLInjection* antiSQLInjection ); 
static BOOL isInjected_comment_at_the_end_statement( TAntiSQLInjection* antiSQLInjection ); 
static BOOL isInjected_stacking_queries( TAntiSQLInjection* antiSQLInjection ); 
static BOOL isInjected_allowed_statement( TAntiSQLInjection* antiSQLInjection ); 
static BOOL isInjected_union_set( TAntiSQLInjection* antiSQLInjection );
static gsp_walking_result __exprVisit(ExprTraverser *traverser, gsp_expr *expr, BOOL isLeafNode);

static TAntiSQLInjection* globalInjection;

static TSQLInjection* createSQLInjection(){
	TSQLInjection *injection = (TSQLInjection *)malloc(sizeof(TSQLInjection));
	injection->type = syntax_error;
	injection->description = NULL;
	return injection;
}


static GEvalValue* createGEvalValue(){
	GEvalValue* value = (GEvalValue*) malloc(sizeof(GEvalValue));
	value->type = type_unknown;
	value->value = NULL;
	globalInjection->gc->add(globalInjection->gc, value);
	return value;
}

static GEval* createGEval(){
	GEval* eval = (GEval*)malloc(sizeof(GEval));
	eval->exprs = createStack();
	eval->valueMap = createMap(Map_KeyType_DEFAULT);
	eval->value = NULL;
	return eval;
}

static void disposeGEval(GEval* eval){
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;
	exprs->dispose(exprs);
	valueMap->dispose(valueMap);
	free(eval);
}

TAntiSQLInjection* createAntiSQLInjection(){
	TAntiSQLInjection *injection = (TAntiSQLInjection *)malloc(sizeof(TAntiSQLInjection));
	injection->e_always_false_condition = TRUE;
	injection->e_always_true_condition = TRUE;
	injection->e_comment_at_the_end_of_statement = TRUE;
	injection->e_not_in_allowed_statement = TRUE;
	injection->e_stacking_queries = TRUE;
	injection->e_union_set = TRUE;
	injection->sqlParser = NULL;
	injection->sqlText = NULL;
	injection->sqlInjections = createList();
	injection->enabledStatements = createList();
	injection->enabledStatements->add(injection->enabledStatements, (void*)sstselect);
	injection->gc = createList();
	return injection;
}

static void disposeTSQLInjection(TSQLInjection* tSQLInjection){
	free(tSQLInjection);
}

void disposeAntiSQLInjection(TAntiSQLInjection* antiSQLInjection){
	int i;
	List* injections = antiSQLInjection->sqlInjections;
	List* gc = antiSQLInjection->gc;
	Iterator iter = gc->getIterator(gc);

	for(i=0;i<injections->size(injections);i++){
		TSQLInjection *injection = (TSQLInjection *)injections->get(injections, i);
		disposeTSQLInjection(injection);
	}
	injections->dispose(injections);

	while(gc->hasNext(gc, &iter)){
		void* point = gc->next(&iter);
		free(point);
	}
	gc->dispose(gc);

	antiSQLInjection->enabledStatements->dispose(antiSQLInjection->enabledStatements);
	free(antiSQLInjection);
}

void check_union_set(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_union_set = on;
}

void check_not_in_allowed_statement(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_not_in_allowed_statement = on;
}

void check_stacking_queries(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_stacking_queries = on;
}

void check_comment_at_the_end_of_statement(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_comment_at_the_end_of_statement = on;
}

void check_always_false_condition(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_always_false_condition = on;
}	

void check_always_true_condition(TAntiSQLInjection* antiSQLInjection, BOOL on){
	antiSQLInjection->e_always_true_condition = on;
}

void enableStatement(TAntiSQLInjection* antiSQLInjection, EStmtType sqltype){
	antiSQLInjection->enabledStatements->add(antiSQLInjection->enabledStatements, (void*)sqltype);
}

void disableStatement(TAntiSQLInjection* antiSQLInjection, EStmtType sqltype){
	antiSQLInjection->enabledStatements->remove(antiSQLInjection->enabledStatements, (void*)sqltype);
}

BOOL isInjected(TAntiSQLInjection* antiSQLInjection, char* sql){
	BOOL ret = FALSE;
	int i;
	globalInjection = antiSQLInjection;
	antiSQLInjection->sqlText = sql;
	antiSQLInjection->sqlInjections->clear(antiSQLInjection->sqlInjections);
	i = gsp_check_syntax(antiSQLInjection->sqlParser, antiSQLInjection->sqlText);
	if (i == 0){
		ret = ret | isInjected_always_false_condition(antiSQLInjection);
		ret = ret | isInjected_always_true_condition(antiSQLInjection);
		ret = ret | isInjected_comment_at_the_end_statement(antiSQLInjection);
		ret = ret | isInjected_stacking_queries(antiSQLInjection);
		ret = ret | isInjected_allowed_statement(antiSQLInjection);
		ret = ret | isInjected_union_set(antiSQLInjection);
	}else{
		TSQLInjection* s = createSQLInjection();
		s->type = syntax_error;
		s->description = gsp_errmsg(antiSQLInjection->sqlParser);
		antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, s);
		ret = TRUE;
	}
	return ret;
}

static BOOL is_compare_condition( EExpressionType t )
{
	return ( ( t == eet_simple_comparison )
		|| ( t == eet_group_comparison ) || ( t == eet_in ) );
}


static GEvalValue* coerceInteger(GEvalValue* val)
{
	GEvalValue* result = createGEvalValue();
	if (val->type == type_null)
	{
		long* value = (long*)malloc(sizeof(long));
		globalInjection->gc->add(globalInjection->gc, value);
		value[0] = 0L;
		result->value = value;
		result->type = type_int;
	}
	else if (val->type == type_string)
	{
		if (strcmp((char*)val->value, "") == 0)
		{
			long* value = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = 0L;
			result->value = value;
			result->type = type_int;
		}
		else{
			long* value = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = atol((char*)val->value);
			result->value = value;
			result->type = type_int;
		}
	}
	else if(val->type == type_int || val->type == type_double || val->type == type_float || val->type == type_long){
		result->type = type_int;
		result->value = val->value;
	}
	return result;
}

static GEvalValue* coerceLong(GEvalValue* val)
{
	GEvalValue* result = createGEvalValue();
	if (val->type == type_null)
	{
		long* value = (long*)malloc(sizeof(long));
		globalInjection->gc->add(globalInjection->gc, value);
		value[0] = 0L;
		result->value = value;
		result->type = type_long;
	}
	else if (val->type == type_string)
	{
		if (strcmp((char*)val->value, "") == 0)
		{
			long* value = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = 0L;
			result->value = value;
			result->type = type_long;
		}
		else{
			long* value = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = atol((char*)val->value);
			result->value = value;
			result->type = type_long;
		}
	}
	else if(val->type == type_int || val->type == type_double || val->type == type_float || val->type == type_long){
		result->type = type_long;
		result->value = val->value;
	}
	return result;
}

static GEvalValue* coerceDouble(GEvalValue* val)
{
	GEvalValue* result = createGEvalValue();
	if (val->type == type_null)
	{
		double* value = (double*)malloc(sizeof(double));
		globalInjection->gc->add(globalInjection->gc, value);
		value[0] = 0L;
		result->value = value;
		result->type = type_double;
	}
	else if (val->type == type_string)
	{
		if (strcmp((char*)val->value, "") == 0)
		{
			double* value = (double*)malloc(sizeof(double));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = 0L;
			result->value = value;
			result->type = type_double;
		}
		else{
			double* value = (double*)malloc(sizeof(double));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = atof((char*)val->value);
			result->value = value;
			result->type = type_double;
		}
	}
	else if(val->type == type_int || val->type == type_double || val->type == type_float || val->type == type_long){
		result->type = type_double;
		result->value = val->value;
	}
	return result;
}

static GEvalValue* coerceFloat(GEvalValue* val)
{
	GEvalValue* result = createGEvalValue();
	if (val->type == type_null)
	{
		double* value = (double*)malloc(sizeof(double));
		globalInjection->gc->add(globalInjection->gc, value);
		value[0] = 0L;
		result->value = value;
		result->type = type_float;
	}
	else if (val->type == type_string)
	{
		if (strcmp((char*)val->value, "") == 0)
		{
			double* value = (double*)malloc(sizeof(double));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = 0L;
			result->value = value;
			result->type = type_float;
		}
		else{
			double* value = (double*)malloc(sizeof(double));
			globalInjection->gc->add(globalInjection->gc, value);
			value[0] = atof((char*)val->value);
			result->value = value;
			result->type = type_float;
		}
	}
	else if(val->type == type_int || val->type == type_double || val->type == type_float || val->type == type_long){
		result->type = type_float;
		result->value = val->value;
	}
	return result;
}

static GEvalValue* coerceBoolean(GEvalValue* val)
{
	GEvalValue* result = createGEvalValue();
	if (val->type == type_null)
	{
		result->type = type_boolean;
		result->value = (void*)FALSE;
	}
	else if (val->type == type_boolean)
	{
		result->type = type_boolean;
		result->value = val->value;
	}
	else if (val->type == type_string)
	{
		result->type = type_boolean;
		result->value = (void*)((compareToIgnoreCase((char*)val->value, "true")==0));
	}
	else{
		result->type = type_boolean;
		result->value = (void*)FALSE;
	}
	return result;
}

static GEvalValue* getValue(GEval* eval)
{
	if (eval->value == NULL)
	{
		Map *valueMap = eval->valueMap;
		Stack *exprs = eval->exprs;
		eval->value = valueMap->get(valueMap, exprs->pop(exprs));
	}
	return eval->value;
}


static GEvalValue* eval_constant( gsp_expr * expr ) 
{
	gsp_constant* constant = expr->constantOperand;
	GEvalValue* result = createGEvalValue();
	if(constant->constantType == ect_integer){
		char* constantString = gsp_getSourceTokenText(constant->fragment.startToken);
		char* constantSign = NULL;
		GEvalValue* param = createGEvalValue();
		GEvalValue* value;
		param->type = type_string;
		param->value = constantString;
		value = coerceInteger(param);
		result->type = value->type;
		result->value = value->value;

		globalInjection->gc->add(globalInjection->gc, constantString);

		if(constant->signToken!=NULL){
			constantSign = gsp_getSourceTokenText(constant->signToken);
			globalInjection->gc->add(globalInjection->gc, constantSign);
		}
		if(constantSign!= NULL && strcmp(constantSign, "-")){
			result->value = (void*)(-((int)result->value));
		}
	}
	if(constant->constantType == ect_float){
		char* constantString = gsp_getSourceTokenText(constant->fragment.startToken);
		GEvalValue* param = createGEvalValue();
		GEvalValue* value;
		param->type = type_string;
		param->value = constantString;
		value = coerceDouble(param);
		result->type = value->type;
		result->value = value->value;
		globalInjection->gc->add(globalInjection->gc, constantString);
	}
	if(constant->constantType == ect_string){
		char* constantString = gsp_getNodeText((gsp_node*)constant);
		if(compareToIgnoreCase(constantString, "NULL") == 0){
			result->type = type_null;
			result->value = NULL;
		}
		else{
			char* value = (char*)malloc(sizeof(char*)*strlen(constantString));
			globalInjection->gc->add(globalInjection->gc, value);
			substring(value,constantString,1,strlen(constantString)-1 );
			result->value = (void*)value;
			result->type =type_string;
		}
		free(constantString);
	}
	return result;
}

static void toString( GEvalValue* value ) 
{
	if(value->type == type_double){
		CString* str = CStringNew();
		CStringAppendFormat(str, "%lf", ((double*)value->value)[0]);
		value->value = str->buffer;
		value->type = type_string;
		CStringDeleteWithoutBuffer(str);
	}
	else if(value->type == type_float){
		CString* str = CStringNew();
		CStringAppendFormat(str, "%f", ((double*)value->value)[0]);
		value->value = str->buffer;
		value->type = type_string;
		CStringDeleteWithoutBuffer(str);
	}
	else if(value->type == type_int){
		CString* str = CStringNew();
		CStringAppendFormat(str, "%d", ((long*)value->value)[0]);
		value->value = str->buffer;
		value->type = type_string;
		CStringDeleteWithoutBuffer(str);
	}
	else if(value->type == type_long){
		CString* str = CStringNew();
		CStringAppendFormat(str, "%ld", ((long*)value->value)[0]);
		value->value = str->buffer;
		value->type = type_string;
		CStringDeleteWithoutBuffer(str);
	}
	else if(value->type == type_boolean){
		if((BOOL)value->value==TRUE)
			value->value = "true";
		else
			value->value = "false";
		value->type = type_string;
	}
	else if(value->type == type_null){
		value->value = "";
		value->type = type_string;
	}
}

static void concatString( GEvalValue* l, GEvalValue* r ) 
{
	if(l->type == type_string && l->type == type_string){
		CString* str = CStringNew();
		CStringAppend(str,(char*)l->value);
		CStringAppend(str,(char*)r->value);
		CStringDeleteWithoutBuffer(str);
		l->value = str->buffer;
		CStringDeleteWithoutBuffer(str);
	}
	else{
		l->type = type_unknown;
		l->value = NULL;
	}
}

static GEvalValue* eval_add(GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
		|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))
		|| (left->type == type_string && ( indexOf((char*)left->value, ".")!=-1 || indexOf((char*)left->value, "e")!=-1 || indexOf((char*)left->value, "E")!=-1 )) 
		|| (right->type == type_string && ( indexOf((char*)right->value, ".")!=-1 || indexOf((char*)right->value, "e")!=-1 || indexOf((char*)right->value, "E")!=-1 ))){
			GEvalValue* l = coerceDouble(left);
			GEvalValue* r = coerceDouble(right);
			if(l->type == type_double && r->type == type_double){
				double* newValue = (double*)malloc(sizeof(double));
				globalInjection->gc->add(globalInjection->gc, newValue);
				newValue[0] =  ((double*)l->value)[0] + ((double*)r->value)[0];
				l->value =newValue;
				return l;
			}
			else{
				toString(l);
				toString(r);
				concatString(l, r);
				return l;
			}
	}
	else{
		GEvalValue* l = coerceLong(left);
		GEvalValue* r = coerceLong(right);
		if(l->type == type_long && r->type == type_long){
			long* newValue = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, newValue);
			newValue[0] =  ((long*)l->value)[0] + ((long*)r->value)[0];
			l->value =newValue;
			return l;
		}
		else{
			toString(l);
			toString(r);
			concatString(l, r);
			return l;
		}
	}
}

static GEvalValue* eval_subtract( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
		|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))
		|| (left->type == type_string && ( indexOf((char*)left->value, ".")!=-1 || indexOf((char*)left->value, "e")!=-1 || indexOf((char*)left->value, "E")!=-1 )) 
		|| (right->type == type_string && ( indexOf((char*)right->value, ".")!=-1 || indexOf((char*)right->value, "e")!=-1 || indexOf((char*)right->value, "E")!=-1 ))){
			GEvalValue* l = coerceDouble(left);
			GEvalValue* r = coerceDouble(right);
			if(l->type == type_double && r->type == type_double){
				double* newValue = (double*)malloc(sizeof(double));
				globalInjection->gc->add(globalInjection->gc, newValue);
				newValue[0] =  ((double*)l->value)[0] - ((double*)r->value)[0];
				l->value =newValue;
				return l;
			}
			else{
				l->type = type_unknown;
				l->value = NULL;
				return l;
			}
	}
	else{
		GEvalValue* l = coerceLong(left);
		GEvalValue* r = coerceLong(right);
		if(l->type == type_long && r->type == type_long){
			long* newValue = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, newValue);
			newValue[0] =  ((long*)l->value)[0] - ((long*)r->value)[0];
			l->value =newValue;
			return l;
		}
		else{
			l->type = type_unknown;
			l->value = NULL;
			return l;
		}
	}
}

static GEvalValue* eval_mul( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
		|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))
		|| (left->type == type_string && ( indexOf((char*)left->value, ".")!=-1 || indexOf((char*)left->value, "e")!=-1 || indexOf((char*)left->value, "E")!=-1 )) 
		|| (right->type == type_string && ( indexOf((char*)right->value, ".")!=-1 || indexOf((char*)right->value, "e")!=-1 || indexOf((char*)right->value, "E")!=-1 ))){
			GEvalValue* l = coerceDouble(left);
			GEvalValue* r = coerceDouble(right);
			if(l->type == type_double && r->type == type_double){
				double* newValue = (double*)malloc(sizeof(double));
				globalInjection->gc->add(globalInjection->gc, newValue);
				newValue[0] =  ((double*)l->value)[0] * ((double*)r->value)[0];
				l->value =newValue;
				return l;
			}
			else{
				l->type = type_unknown;
				l->value = NULL;
				return l;
			}
	}
	else{
		GEvalValue* l = coerceLong(left);
		GEvalValue* r = coerceLong(right);
		if(l->type == type_long && r->type == type_long){
			long* newValue = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, newValue);
			newValue[0] =  ((long*)l->value)[0] * ((long*)r->value)[0];
			l->value =newValue;;
			return l;
		}
		else{;
		l->type = type_unknown;
		l->value = NULL;
		return l;
		}
	}
}

static GEvalValue* eval_divide( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
		|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))
		|| (left->type == type_string && ( indexOf((char*)left->value, ".")!=-1 || indexOf((char*)left->value, "e")!=-1 || indexOf((char*)left->value, "E")!=-1 )) 
		|| (right->type == type_string && ( indexOf((char*)right->value, ".")!=-1 || indexOf((char*)right->value, "e")!=-1 || indexOf((char*)right->value, "E")!=-1 ))){
			GEvalValue* l = coerceDouble(left);
			GEvalValue* r = coerceDouble(right);
			if(l->type == type_double && r->type == type_double && ((double*)r->value)[0]!=0){
				double* newValue = (double*)malloc(sizeof(double));
				globalInjection->gc->add(globalInjection->gc, newValue);
				newValue[0] =  ((double*)l->value)[0] / ((double*)r->value)[0];
				l->value =newValue;
				return l;
			}
			else{
				l->type = type_unknown;
				l->value = NULL;
				return l;
			}
	}
	else{
		GEvalValue* l = coerceLong(left);
		GEvalValue* r = coerceLong(right);
		if(l->type == type_long && r->type == type_long && ((long*)r->value)[0]!=0){
			long* newValue = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, newValue);
			newValue[0] =  ((long*)l->value)[0] / ((long*)r->value)[0];
			l->value =newValue;
			return l;
		}
		else{
			l->type = type_unknown;
			l->value = NULL;
			return l;
		}
	}
}

static GEvalValue* eval_concatenate( GEval* eval, gsp_expr * expr) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	toString(left);
	toString(right);
	concatString(left, right);
	return left;
}

static void checkSubquery( GEval* eval, gsp_selectStatement* select ) 
{
	if (select != NULL && select->whereCondition != NULL)
	{
		ExprTraverser visitor;
		GEvalValue* value;
		visitor.exprVisit = __exprVisit;
		visitor.context = createGEval();
		postOrderTraverse(&visitor, select->whereCondition->condition);
		value = getValue((GEval*)visitor.context);
		if (value->type == type_boolean)
		{
			eval->value = createGEvalValue();
			eval->value->type = value->type;
			eval->value->value = value->value;
		}
		disposeGEval((GEval*)visitor.context);
	}
}

static GEvalValue* eval_exists_condition(GEval* eval, gsp_expr * expr ) 
{
	if(expr->rightOperand!=NULL && expr->rightOperand->subQueryStmt!=NULL){
		checkSubquery(eval, (gsp_selectStatement*)expr->rightOperand->subQueryStmt);
		if (((gsp_selectStatement*)expr->rightOperand->subQueryStmt)->whereCondition != NULL)
		{
			ExprTraverser visitor;
			visitor.exprVisit = __exprVisit;
			visitor.context = createGEval();
			postOrderTraverse(&visitor, ((gsp_selectStatement*)expr->rightOperand->subQueryStmt)->whereCondition->condition);
			return ((GEval*)visitor.context)->value;
		}
		else{
			GEvalValue* boolValue = createGEvalValue();
			boolValue->type = type_boolean;
			boolValue->value = (void*)TRUE;
			return boolValue;
		}
	}
	return createGEvalValue();
}

static GEvalValue* eval_unknown_one_operand( GEval* eval, gsp_expr * expr) 
{
	Stack *exprs = eval->exprs;
	exprs->pop(exprs);
	return createGEvalValue();
}

static GEvalValue* eval_unknown_two_operand( GEval* eval, gsp_expr * expr) 
{
	Stack *exprs = eval->exprs;
	exprs->pop(exprs);
	exprs->pop(exprs);
	return createGEvalValue();
}

static GEvalValue* eval_mod( GEval* eval, gsp_expr * expr) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(left->type == type_unknown || right->type == type_unknown){
		return createGEvalValue();
	}

	if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
		|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))
		|| (left->type == type_string && ( indexOf((char*)left->value, ".")!=-1 || indexOf((char*)left->value, "e")!=-1 || indexOf((char*)left->value, "E")!=-1 )) 
		|| (right->type == type_string && ( indexOf((char*)right->value, ".")!=-1 || indexOf((char*)right->value, "e")!=-1 || indexOf((char*)right->value, "E")!=-1 ))){
			GEvalValue* l = coerceDouble(left);
			GEvalValue* r = coerceDouble(right);
			if(l->type == type_double && r->type == type_double && ((double*)r->value)[0]!=0){
				double* newValue = (double*)malloc(sizeof(double));
				globalInjection->gc->add(globalInjection->gc, newValue);
				newValue[0] =  ((double*)l->value)[0] * ((double*)r->value)[0];
				l->value =newValue;
				return l;
			}
			else{
				l->type = type_unknown;
				l->value = NULL;
				return l;
			}
	}
	else{
		GEvalValue* l = coerceLong(left);
		GEvalValue* r = coerceLong(right);
		if(l->type == type_long && r->type == type_long && ((long*)r->value)[0]!=0){
			long* newValue = (long*)malloc(sizeof(long));
			globalInjection->gc->add(globalInjection->gc, newValue);
			newValue[0] =  ((long*)l->value)[0] % ((long*)r->value)[0];
			l->value =newValue;
			return l;
		}
		else{
			l->type = type_unknown;
			l->value = NULL;
			return l;
		}
	}
}

static GEvalValue* eval_equal( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null &&  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)TRUE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] == ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] == ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_boolean || right->type == type_boolean){
			GEvalValue *l  = coerceBoolean(left);
			GEvalValue *r  = coerceBoolean(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_boolean && r->type == type_boolean){
				value->type = type_boolean;
				if(((BOOL)l->value == TRUE && (BOOL)r->value == TRUE)
					|| ((BOOL)l->value == FALSE && (BOOL)r->value == FALSE)){
						value->value = (void*)TRUE;
				}
				else{
					value->value = (void*)FALSE;
				}
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();		
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) == 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_notequal( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null &&  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)TRUE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] != ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] != ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_boolean || right->type == type_boolean){
			GEvalValue *l  = coerceBoolean(left);
			GEvalValue *r  = coerceBoolean(right);
			GEvalValue *value = createGEvalValue();
			if(left->type == type_boolean && right->type == type_boolean){
				value->type = type_boolean;
				if(((BOOL)l->value == TRUE && (BOOL)r->value == TRUE)
					|| ((BOOL)l->value == FALSE && (BOOL)r->value == FALSE)){
						value->value = (void*)FALSE;
				}
				else{
					value->value = (void*)TRUE;
				}
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) != 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_gt( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] > ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] > ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) > 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_lt( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] < ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] < ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) < 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_le( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] <= ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] <= ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) <= 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_ge( GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if ((left->type == type_unknown) || (right->type == type_unknown))
	{
		return createGEvalValue();
	}

	if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else if(left->type == type_null ||  right->type == type_null){
		GEvalValue *value = createGEvalValue();
		value->type = type_boolean;
		value->value = (void*)FALSE;
		return value;
	}
	else{
		if(((left->type == type_float || left->type == type_double) && (right->type == type_float || right->type == type_double || right->type == type_int|| right->type == type_long))
			|| ((left->type == type_float || left->type == type_double || left->type == type_int|| left->type == type_long) && (right->type == type_float || right->type == type_double))){
				GEvalValue *l  = coerceDouble(left);
				GEvalValue *r  = coerceDouble(right);
				GEvalValue *value = createGEvalValue();
				if(l->type == type_double && r->type == type_double){
					value->type = type_boolean;
					value->value = (void*)(((double*)l->value)[0] >= ((double*)r->value)[0]);

				}
				return value;
		}
		else if((left->type == type_int || left->type == type_long) && (right->type == type_int || right->type == type_long)){
			GEvalValue *l  = coerceLong(left);
			GEvalValue *r  = coerceLong(right);
			GEvalValue *value = createGEvalValue();
			if(l->type == type_long && r->type == type_long){
				value->type = type_boolean;
				value->value = (void*)(((long*)l->value)[0] >= ((long*)r->value)[0]);
			}
			return value;
		}
		else if(left->type == type_string || right->type == type_string){
			GEvalValue *value = createGEvalValue();
			toString(left);
			toString(right);
			if(left->type == type_string && right->type == type_string){
				value->type = type_boolean;
				value->value = (void*)(strcmp((char*)left->value, (char*)right->value) >= 0);
			}
			return value;
		}
		else return  createGEvalValue();
	}
}

static GEvalValue* eval_simple_comparison_conditions(GEval* eval, gsp_expr * expr ) 
{
	GEvalValue *left, *right;
	char* tokenText = NULL;
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	void* rightExpr = exprs->pop(exprs);
	void* leftExpr = exprs->pop(exprs);
	exprs->push(exprs, leftExpr);
	exprs->push(exprs, rightExpr);

	right = valueMap->get(valueMap, rightExpr);
	left = valueMap->get(valueMap, leftExpr);

	if(left->type == type_unknown || right->type == type_unknown){
		exprs->pop(exprs);
		exprs->pop(exprs);
		return createGEvalValue();
	}

	tokenText = trimString(gsp_getSourceTokenText(expr->operatorToken));
	globalInjection->gc->add(globalInjection->gc, tokenText);
	if(strcmp(tokenText,"=") == 0){
		return eval_equal(eval, expr);
	}
	else if(strcmp(tokenText,">") == 0){
		return eval_gt(eval, expr);
	}
	else if(strcmp(tokenText,"<") == 0){
		return eval_lt(eval, expr);
	}
	else if(strcmp(tokenText,"<=") == 0){
		return eval_le(eval, expr);
	}
	else if(strcmp(tokenText,">=") == 0){
		return eval_ge(eval, expr);
	}
	else if(strcmp(tokenText,"<>") == 0
		|| strcmp(tokenText,"!=") == 0){
			return eval_notequal(eval, expr);
	}

	exprs->pop(exprs);
	exprs->pop(exprs);
	return createGEvalValue();
}

static GEvalValue* eval_group_comparison_conditions(GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	exprs->pop(exprs);
	exprs->pop(exprs);

	return createGEvalValue();
}


static GEvalValue* eval_in_conditions(GEval* eval, gsp_expr * expr) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;
	while(TRUE){
		gsp_expr* peekExpr = exprs->peek(exprs);
		//FIXME: here has a bug.
		if(peekExpr->expressionType == eet_simple_constant){
			exprs->pop(exprs);
		}
		else{
			exprs->pop(exprs);
			break;
		}
	}
	return createGEvalValue();
}

static GEvalValue* eval_logical_conditions_and(GEval* eval, gsp_expr * expr) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(right->type == type_unknown){
		if(left->type == type_unknown){
			return createGEvalValue();
		}
		else{
			GEvalValue* boolValue = coerceBoolean(left);
			if(boolValue->type == type_boolean && (BOOL)boolValue->value == FALSE){
				return boolValue;
			}
			else{
				return createGEvalValue();
			}
		}
	}
	else if(left->type == type_unknown){
		GEvalValue* boolValue = coerceBoolean(right);
		if(boolValue->type == type_boolean && (BOOL)boolValue->value == FALSE){
			return boolValue;
		}
		else{
			return createGEvalValue();
		}
	}
	else{
		GEvalValue* rightValue = coerceBoolean(right);
		GEvalValue* leftValue = coerceBoolean(left);

		if(leftValue->type == type_boolean && rightValue->type == type_boolean && 
			((BOOL)leftValue->value && (BOOL)rightValue->value) ){
				leftValue->value = (void*)TRUE;
				return leftValue;
		}
		else{
			leftValue->type = type_boolean;
			leftValue->value = (void*)FALSE;
			return leftValue;
		}
	}
}

static GEvalValue* eval_logical_conditions_or(GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));

	if(right->type == type_unknown){
		if(left->type == type_unknown){
			return createGEvalValue();
		}
		else{
			GEvalValue* boolValue = coerceBoolean(left);
			if(boolValue->type == type_boolean && (BOOL)boolValue->value == TRUE){
				return boolValue;
			}
			else{
				return createGEvalValue();
			}
		}
	}
	else if(left->type == type_unknown){
		GEvalValue* boolValue = coerceBoolean(right);
		if(boolValue->type == type_boolean && (BOOL)boolValue->value == TRUE){
			return boolValue;
		}
		else{
			return createGEvalValue();
		}
	}
	else{
		GEvalValue* rightValue = coerceBoolean(right);
		GEvalValue* leftValue = coerceBoolean(left);

		if(leftValue->type == type_boolean && rightValue->type == type_boolean && 
			((BOOL)leftValue->value || (BOOL)rightValue->value) ){
				leftValue->value = (void*)TRUE;
				return leftValue;
		}
		else{
			leftValue->type = type_boolean;
			leftValue->value = (void*)FALSE;
			return leftValue;
		}
	}
}

static GEvalValue* eval_logical_conditions_not(GEval* eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));
	if(left->type == type_unknown){
		return createGEvalValue();
	}
	else{
		GEvalValue* boolValue = coerceBoolean(left);
		if(boolValue->type == type_boolean){
			boolValue->value = (BOOL)boolValue->value == TRUE? (void*)FALSE:(void*)TRUE;
		}
		return boolValue;
	}
}

static GEvalValue* eval_isnull( GEval* eval, gsp_expr * expr) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* left = valueMap->get(valueMap, exprs->pop(exprs));
	if(left->type == type_unknown){
		return createGEvalValue();
	}
	else{
		if(left->type == type_null){
			left->type = type_boolean;
			left->value = (void*)TRUE;
			return left;
		}
		else if(left->type == type_boolean && (BOOL)left->value == FALSE){
			left->type = type_boolean;
			left->value = (void*)FALSE;
			return left;
		}
		else{
			return createGEvalValue();
		}
	}
}

static void eval_assignment( GEval * eval, gsp_expr * expr ) 
{
	Map *valueMap = eval->valueMap;
	Stack *exprs = eval->exprs;

	GEvalValue* right = valueMap->get(valueMap, exprs->pop(exprs));
	valueMap->put(valueMap, exprs->pop(exprs), right);
}

static gsp_walking_result __exprVisit(ExprTraverser *traverser, gsp_expr *expr, BOOL isLeafNode)
{
	GEval *eval = (GEval*)traverser->context;
	Map *valueMap = eval->valueMap;
	Stack *stack = eval->exprs;
	switch(expr->expressionType){
	case eet_simple_source_token:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_simple_object_name:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_simple_constant:
		valueMap->put(valueMap, expr, eval_constant( expr ));
		stack->push(stack, expr);
		break;
	case eet_arithmetic_plus:
		valueMap->put(valueMap, expr, eval_add( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_arithmetic_minus:
		valueMap->put(valueMap, expr, eval_subtract( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_arithmetic_times:
		valueMap->put(valueMap, expr, eval_mul( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_arithmetic_divide:
		valueMap->put(valueMap, expr, eval_divide( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_parenthesis:
		valueMap->put(valueMap, expr, valueMap->get(valueMap, stack->pop(stack)));
		stack->push(stack, expr);
		break;
	case eet_concatenate:
		valueMap->put(valueMap, expr, eval_concatenate( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_unary_plus:
		valueMap->put(valueMap, expr, valueMap->get(valueMap, stack->pop(stack)));
		stack->push(stack, expr);
		break;
	case eet_unary_minus:
		{
			GEvalValue* l = coerceLong(valueMap->get(valueMap, stack->pop(stack)));
			if(l->type ==  type_long){
				((long*)l->value)[0] = -((long*)l->value)[0];
			}
			valueMap->put(valueMap, expr, l);
			stack->push(stack, expr);
		}
		break;
	case eet_assignment:
		eval_assignment(eval, expr);
		stack->push(stack, expr);
		break;
	case eet_group:
		if(expr->rightOperand->subQueryStmt!=NULL){
			checkSubquery(eval, (gsp_selectStatement*)expr->rightOperand->subQueryStmt);
		}
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_list:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_function:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_new_structured_type:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_cursor:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_subquery:
		if(expr->subQueryStmt!=NULL)
		{
			checkSubquery(eval, (gsp_selectStatement*)expr->subQueryStmt);
		}
		else{
			valueMap->put(valueMap, expr, createGEvalValue());
		}
		stack->push(stack, expr);
		break;
	case eet_case:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_pattern_matching:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_exists:
		valueMap->put(valueMap, expr, eval_exists_condition( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_new_variant_type:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break;
	case eet_unary_prior:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_unary_bitwise_not:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_sqlserver_proprietary_column_alias:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_arithmetic_modulo:
		valueMap->put(valueMap, expr, eval_mod( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_bitwise_exclusive_or:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_bitwise_or:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_bitwise_and:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_bitwise_xor:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_exponentiate:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_scope_resolution:
		stack->push(stack, expr);
		break; 
	case eet_at_time_zone:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_at_local:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_day_to_second:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_year_to_month:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_simple_comparison:
		valueMap->put(valueMap, expr, eval_simple_comparison_conditions( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_group_comparison:
		valueMap->put(valueMap, expr, eval_group_comparison_conditions( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_in:
		valueMap->put(valueMap, expr, eval_in_conditions( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_floating_point:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_logical_and:
		valueMap->put(valueMap, expr, eval_logical_conditions_and( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_logical_or:
		valueMap->put(valueMap, expr, eval_logical_conditions_or( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_logical_not:
		valueMap->put(valueMap, expr, eval_logical_conditions_not( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_logical_xor:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_null:
		valueMap->put(valueMap, expr, eval_isnull( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_between:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_is_of_type:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_collate:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_left_join:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_right_join:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_outer_join:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_ref_arrow:
		valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		stack->push(stack, expr);
		break;
	case eet_typecast:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_arrayaccess:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break; 
	case eet_unary_connect_by_root:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_interval:
		valueMap->put(valueMap, expr, createGEvalValue());
		stack->push(stack, expr);
		break; 
	case eet_unary_binary_operator:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_left_shift:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	case eet_right_shift:
		valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		stack->push(stack, expr);
		break; 
	default:
		if(expr->leftOperand!=NULL && expr->rightOperand!=NULL){
			valueMap->put(valueMap, expr, eval_unknown_two_operand( eval, expr ));
		}
		else if(expr->leftOperand!=NULL){
			valueMap->put(valueMap, expr, eval_unknown_one_operand( eval, expr ));
		}
		stack->push(stack, expr);
		break;
	}
	return gsp_walking_continue;
}

static BOOL isInjected_always_false_condition( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	if (!antiSQLInjection->e_always_true_condition) { return FALSE; }
	if (antiSQLInjection->sqlParser->nStatement == 0) { return ret; }
	else{
		gsp_sql_statement *stmt = &antiSQLInjection->sqlParser->pStatement[0];
		if (stmt->stmtType ==  sstselect){
			gsp_selectStatement *select = (gsp_selectStatement *)stmt->stmt;
			if(select->whereCondition!=NULL && select->whereCondition->condition!=NULL){
				GEval *e = createGEval();
				GEvalValue* value;
				ExprTraverser visitor;
				visitor.context = e;
				visitor.exprVisit = __exprVisit;
				postOrderTraverse(&visitor, select->whereCondition->condition);
				value = getValue(e); 
				if (value->type == type_boolean)
				{
					if (((BOOL)e->value->value) == FALSE)
					{
						TSQLInjection* injection = createSQLInjection();
						injection->type = always_false_condition;
						injection->description="always_false_condition";
						antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
						ret = true;
					}
				}
				disposeGEval(e);
			}
		}
	}
	return ret;
}

static BOOL isInjected_always_true_condition( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	if (!antiSQLInjection->e_always_true_condition) { return FALSE; }
	if (antiSQLInjection->sqlParser->nStatement == 0) { return ret; }
	else{
		gsp_sql_statement *stmt = &antiSQLInjection->sqlParser->pStatement[0];
		if (stmt->stmtType ==  sstselect){
			gsp_selectStatement *select = (gsp_selectStatement *)stmt->stmt;
			if(select->whereCondition!=NULL && select->whereCondition->condition!=NULL ){
				GEval *e = createGEval();
				GEvalValue* value;
				ExprTraverser visitor;
				visitor.context = e;
				visitor.exprVisit = __exprVisit;
				postOrderTraverse(&visitor, select->whereCondition->condition);
				value = getValue(e); 
				if (value->type == type_boolean)
				{
					if (((BOOL)value->value) == TRUE)
					{
						TSQLInjection* injection = createSQLInjection();
						injection->type = always_true_condition;
						injection->description="always_true_condition";
						antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
						ret = true;
					}
				}
				disposeGEval(e);
			}
		}
	}
	return ret;
}

static BOOL isInjected_comment_at_the_end_statement( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	if (!antiSQLInjection->e_comment_at_the_end_of_statement) { return FALSE; }
	else{
		gsp_sourcetoken st;
		gsp_sourcetoken *tokenList = antiSQLInjection->sqlParser->sourcetokenlist;
		int count = antiSQLInjection->sqlParser->number_of_token;
		int j;
		for (j = count - 1; j >= 0; j--)
		{
			st = tokenList[j];
			if ((st.nCode == gsp_tc_lexspace )
				|| (st.nCode == gsp_tc_return) 
				|| (st.nCode == gsp_tc_lexnewline)
				)
			{ continue; }
			else
			{
				break;
			}
		}
		if ((st.nCode == gsp_tc_cmtdoublehyphen) || (st.nCode == gsp_tc_cmtslashstar) || (st.nCode == gsp_tc_comment))
		{
			TSQLInjection* injection = createSQLInjection();
			injection->type = comment_at_the_end_of_statement;
			injection->description="comment_at_the_end_of_statement";
			antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
			ret = TRUE;
		}
		return ret;
	}
}

static BOOL isInjected_stacking_queries( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	if (!antiSQLInjection->e_stacking_queries) { return FALSE; }
	if (antiSQLInjection->sqlParser->nStatement > 1)
	{
		TSQLInjection* injection = createSQLInjection();
		injection->type = stacking_queries;
		injection->description="stacking_queries";
		antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
		ret = TRUE;
	}
	return ret;
}

static BOOL isAllowedStatement( TAntiSQLInjection* antiSQLInjection, EStmtType stmtType ) 
{
	BOOL ret = FALSE;
	int i;
	for (i = 0; i < antiSQLInjection->enabledStatements->size(antiSQLInjection->enabledStatements); i++)
	{
		if ( ((EStmtType)antiSQLInjection->enabledStatements->get(antiSQLInjection->enabledStatements,i)) == stmtType)
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}

static BOOL isInjected_allowed_statement( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	int j;
	if (!antiSQLInjection->e_not_in_allowed_statement) { return FALSE; }
	for (j = 0; j < antiSQLInjection->sqlParser->nStatement; j++)
	{
		gsp_sql_statement *stmt = &antiSQLInjection->sqlParser->pStatement[j];
		if (!isAllowedStatement(antiSQLInjection, stmt->stmtType))
		{
			TSQLInjection* injection = createSQLInjection();
			injection->type = not_in_allowed_statement;
			switch (stmt->stmtType)
			{
			case sstselect:
				injection->description="sstselect";
				break;
			case sstinsert:
				injection->description="sstinsert";
				break;
			case sstupdate:
				injection->description="sstupdate";
				break;
			case sstdelete:
				injection->description="sstdelete";
				break;
			case sstcreatetable:
				injection->description="sstcreatetable";
				break;
			case sstcreateview:
				injection->description="sstcreateview";
				break;
			case sstcreatedatabase:
				injection->description="sstcreatedatabase";
				break;
			case sstmerge:
				injection->description="sstmerge";
				break;
			case sstdroptable:
				injection->description="sstdroptable";
				break;
			case sstdropview:
				injection->description="sstdropview";
				break;
			case sstaltertable:
				injection->description="sstaltertable";
				break;
			default:
				injection->description="other type";
				break;
			}
			antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
			ret = ret | TRUE;
		};

	}
	return ret;
}

static BOOL isInjected_union_set( TAntiSQLInjection* antiSQLInjection ) 
{
	BOOL ret = FALSE;
	if (!antiSQLInjection->e_union_set) { return false; }
	if (antiSQLInjection->sqlParser->nStatement == 0) { return ret; }
	else{
		gsp_sql_statement *stmt = &antiSQLInjection->sqlParser->pStatement[0];
		if (stmt->stmtType ==  sstselect){
			gsp_selectStatement *select = (gsp_selectStatement *)stmt->stmt;
			if(select->setOperator !=  eso_none){
				TSQLInjection* injection = createSQLInjection();
				injection->type = union_set;
				injection->description="union_set";
				antiSQLInjection->sqlInjections->add(antiSQLInjection->sqlInjections, injection);
				ret = TRUE;
			}
		}
		return ret;
	}
}

int main()
{
	int  i;
	gsp_sqlparser *parser;
	char *sqlText = "select col1 from table1 where 1 in (1,2,4) and 1!=1;";
	TAntiSQLInjection* anti;

	gsp_parser_create(dbvoracle,&parser);

	anti = createAntiSQLInjection();
	anti->sqlParser = parser;

	if(isInjected(anti, sqlText)){
		printf("SQL injected found for this sql:\n%s\n\n",sqlText);
		for(i=0;i<anti->sqlInjections->size(anti->sqlInjections);i++){
			TSQLInjection* injection = (TSQLInjection*)anti->sqlInjections->get(anti->sqlInjections, i);
			printf("type: ");
			switch(injection->type){
			case syntax_error:
				printf("syntax_error, ");
				break;
			case always_true_condition:
				printf("always_true_condition, ");
				break;
			case always_false_condition:
				printf("always_false_condition, ");
				break;
			case comment_at_the_end_of_statement:
				printf("comment_at_the_end_of_statement, ");
				break;
			case stacking_queries:
				printf("stacking_queries, ");
				break;
			case not_in_allowed_statement:
				printf("not_in_allowed_statement, ");
				break;
			case union_set:
				printf("union_set, ");
				break;
			}
			printf("description: ");
			printf("%s\n", injection->description);
		}
	}
	else{
		printf("Not injected");
	}

	disposeAntiSQLInjection(anti);
	gsp_parser_free(parser);
	return 0;
}
