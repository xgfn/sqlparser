#ifndef _ANTI_SQL_INJECTION_H
#define _ANTI_SQL_INJECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "linked_list.h"
#include "tree_map.h"
#include "lifo_stack.h"

	typedef enum ESQLInjectionType {
		syntax_error, always_true_condition, always_false_condition, comment_at_the_end_of_statement,
		stacking_queries, not_in_allowed_statement, union_set
	}ESQLInjectionType;

	typedef struct TSQLInjection{
		ESQLInjectionType type;
		char* description;
	}TSQLInjection;

	typedef enum GEvalValueType {
		type_int, type_float, type_double, type_long, type_boolean, type_unknown, type_null, type_string
	}GEvalValueType;

	typedef struct GEvalValue{
		void* value;
		GEvalValueType type;
	}GEvalValue;

	typedef struct GEval{
		Map* valueMap;
		Stack* exprs;
		GEvalValue* value;
	}GEval;

	typedef struct TAntiSQLInjection{
		BOOL e_always_true_condition;
		BOOL e_always_false_condition;
		BOOL e_comment_at_the_end_of_statement;
		BOOL e_stacking_queries;
		BOOL e_not_in_allowed_statement;
		BOOL e_union_set;
		List* sqlInjections;
		List* enabledStatements;
		char* sqlText;
		gsp_sqlparser *sqlParser;
		List* gc;
	}TAntiSQLInjection;


	TAntiSQLInjection* createAntiSQLInjection();

	void disposeAntiSQLInjection(TAntiSQLInjection* antiSQLInjection);

	void check_union_set(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void check_not_in_allowed_statement(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void check_stacking_queries(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void check_comment_at_the_end_of_statement(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void check_always_false_condition(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void check_always_true_condition(TAntiSQLInjection* antiSQLInjection, BOOL on);

	void enableStatement(TAntiSQLInjection* antiSQLInjection, EStmtType sqltype);

	void disableStatement(TAntiSQLInjection* antiSQLInjection, EStmtType sqltype);

	BOOL isInjected(TAntiSQLInjection* antiSQLInjection, char* sql);

#ifdef __cplusplus
}
#endif

#endif /* _ANTI_SQL_INJECTION_H */