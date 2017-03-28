#ifndef GSP_SOURCETOKEN_H
#define GSP_SOURCETOKEN_H

/*!
**  \file  gsp_sourcetoken.h
**  \brief source token related functions.
*/

#include "gsp_base.h"
#include "gsp_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
* get string of token, caller must free this string.
*/
char *gsp_token_text(gsp_sourcetoken *st);

void gsp_print_token(gsp_sourcetoken *st);


void gsp_sourcetoken_init(gsp_sourcetoken *);


int gsp_token_is_solid(gsp_sourcetoken *,int bPcmtissolidtoken);
gsp_sourcetoken *gsp_token_search(gsp_sourcetoken *sourcetokenlist,int size,int targetTokenCode, char *targetTokenText, gsp_sourcetoken *startToken,  int range);
int gsp_token_serach_sqlplus_after(gsp_sourcetoken *sourcetokenlist,int size,int pStart);
int gsp_token_search_return_after(gsp_sourcetoken *sourcetokenlist,int size,int pStart,int ignorecomment);
int gsp_token_search_return_before(gsp_sourcetoken *sourcetokenlist,int size,int pStart,int  ignorecomment);



gsp_sourcetoken *gsp_token_search_next_solid_token(gsp_sourcetoken *sourcetokenlist,int size,int ptokenpos,int pstep,int pcmtissolidtoken);
int gsp_token_search_token_by_code(gsp_sourcetoken *sourcetokenlist,int size,int pStart,int pTokenCode,int pSteps,char *pIgnoreThisString);
gsp_sourcetoken *gsp_token_search_prev_solid_token(gsp_sourcetoken *lctokenlist,int size,int pStart);
gsp_sourcetoken *gsp_token_search_prev_solid_token_by_token(gsp_sourcetoken *lctokenlist,int size, gsp_sourcetoken *ptoken);


int gsp_token_icmp(gsp_sourcetoken *,char *);
int gsp_token_compare_token_code(gsp_sourcetoken *token, int code);

#ifdef EXACTSOLUTION
void gsp_token_set_char(gsp_sourcetoken *st, char c);
#endif

#ifdef __cplusplus
}
#endif

#endif   /* GSP_SOURCETOKEN_H */