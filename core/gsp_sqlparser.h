#ifndef GSP_SQLPARSER_H
#define GSP_SQLPARSER_H

/*!
**  \file  gsp_sqlparser.h
**  \brief functions used to create/free an instance of sql parser.
** 
**  parse sql script using instance of sql parser.
*/

#include "gsp_base.h"

#ifdef __cplusplus
extern "C" {
#endif


/*!
* create an instance of sql parser
*/
int gsp_parser_create(gsp_dbvendor dbVendor,gsp_sqlparser **parser);

/*!
* free an instance of sql parser created by gsp_parser_create
*/
void gsp_parser_free(gsp_sqlparser *sqlparser);

int gsp_tokenize(gsp_sqlparser *sqlparser, char *sql);
int gsp_tokenize_file(gsp_sqlparser *sqlparser,FILE *sqlfile);

int gsp_separate_statements(gsp_sqlparser *sqlparser, char *sql);
int gsp_separate_sqlstatements_file(gsp_sqlparser *sqlparser,FILE *sqlfile);

/*!
* check syntax of input sql which is UTF-8 encoded
*/
int gsp_check_syntax(gsp_sqlparser *sqlparser, char *sql);
int gsp_check_syntax_file(gsp_sqlparser *sqlparser,FILE *sqlfile);

/*!
* get error message after parsing a sql script.
*/
char *gsp_errmsg(gsp_sqlparser *sqlparser);

gsp_sourcetoken *gsp_token_list(gsp_sqlparser *sqlparser);

int gsp_sqlparser_read_sql_from_file(gsp_sqlparser *sqlparser,FILE *in);

//gsp_node *gsp_get_first_node(gsp_sqlparser *sqlparser);
//gsp_lib.h end


#ifdef __cplusplus
}
#endif

#endif