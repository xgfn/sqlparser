#ifndef _JOIN_CONVERTER_H
#define _JOIN_CONVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "linked_list.h"
#include "tree_map.h"
#include "lifo_stack.h"

	typedef enum jointype {
		inner, left, right
	}jointype;

	typedef struct JoinCondition{
		char* lefttable;
		char* righttable;
		char* leftcolumn;
		char* rightcolumn;
		BOOL used;
		gsp_expr* lexpr;
		gsp_expr* rexpr;
		gsp_expr* expr;
		jointype jt;
	}JoinCondition;

	typedef struct FromClause{
		gsp_fromTable* joinTable;
		char* joinClause;
		char* condition;
	}FromClause;

	typedef struct ExprTableVisitor{
		List* tables;
		List* jrs;
	}ExprTableVisitor;

	typedef struct JoinConverter{
		int ErrorNo;
		char* query;
		FILE *sqlFile;
		CString* ErrorMessage;
		gsp_dbvendor dbvendor;
	}JoinConverter;

	JoinCondition* createJoinCondition();

	FromClause* createFromClause();

	ExprTableVisitor* createExprTableVisitor();

	JoinConverter* createJoinConverter();

	void disposeExprTableVisitor(ExprTableVisitor* visitor);

	void disposeJoinConverter(JoinConverter* converter);

	int convert(JoinConverter* converter);

#ifdef __cplusplus
}
#endif

#endif /* _JOIN_CONVERTER_H */