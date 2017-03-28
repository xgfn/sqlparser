#include <stdio.h>
#include "gsp_base.h" 
#include "gsp_sqlparser.h"

int main()
{
        gsp_sqlparser *parser;
        int rc;
        char *sql = 
                                "SELECT last_name, \n"
                                "       job_id, \n"
                                "       salary \n"
                                "FROM   employees \n"
                                "WHERE  job_id = (SELECT job_id \n"
                                "                 FROM   employees \n"
                                "                 WHERE  employee_id = 141) \n"
                                "       AND salary > (SELECT salary \n"
                                "                     FROM   employees \n"
                                "                     WHERE  employeesname = 'gjc');\n"
                                ;

        rc = gsp_parser_create(dbvoracle,&parser);
        if (rc){
                fprintf(stderr,"create parser error");
                return 1;
        }

        rc= gsp_check_syntax(parser,sql);
        if (rc != 0){
                fprintf(stderr,"parser error:%s\n",gsp_errmsg(parser));
        }else{
                fprintf(stdout,"parser ok\n");
        }

        gsp_parser_free(parser);

        return 0;
}
