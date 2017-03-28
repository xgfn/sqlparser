#ifndef GSP_DBCMDS_H
#define GSP_DBCMDS_H

#include "gsp_base.h"
#include "gsp_sqlstatement.h"
#include "gsp_sourcetoken.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsp_sqlcmd {
    EStmtType sqlstatementtype;
	int token1;
    char *token2,*token3,*token4,*token5,*token6,*token7;
}gsp_sqlcmd;

gsp_sql_statement *gsp_dbcmds_issql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_isoraclesql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_ismssqlsql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_isnetezzasql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_ispostgresql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_isdb2sql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_ismysqlsql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_ismdxsql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_isteradatasql(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_isinformix(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );
gsp_sql_statement *gsp_dbcmds_issybase(gsp_sourcetoken *pcst, gsp_sqlparser *sqlparser, gsp_efindsqlstatetype pstate, gsp_sql_statement *psqlstatement );

EStmtType finddbcmd(gsp_sourcetoken *sourcetokenlist,int size,gsp_sourcetoken *ptoken,const gsp_sqlcmd *pdbcmds ,int num_cmds);
EStmtType findmssqlcmdusedbyyacc(gsp_sourcetoken *sourcetokenlist,int size,gsp_sourcetoken *ptoken);


#ifdef __cplusplus
}
#endif

#endif   /* GSP_DBCMDS_H */