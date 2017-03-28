#ifndef GSP_SQLSTATEMENT_H
#define GSP_SQLSTATEMENT_H

/*!
**  \file  gsp_sqlstatement.h
**  \brief sql statements
*/

#include "gsp_base.h"

#ifdef __cplusplus
extern "C" {
#endif

int gsp_stmt_is_plsql(gsp_sql_statement *);
gsp_sql_statement *gsp_create_stmt(gsp_sqlparser *, EStmtType);


#ifdef __cplusplus
}
#endif

#endif   /* GSP_SQLSTATEMENT_H */