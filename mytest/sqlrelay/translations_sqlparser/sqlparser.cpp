#include <sqlrelay/sqlrserver.h>
#include <ctype.h>
#include <rudiments/stringbuffer.h>
#include <rudiments/character.h>
#include <rudiments/debugprint.h>

class sqlrtranslation_sqlparser : public sqlrtranslation {
	public:
			sqlrtranslation_sqlparser(
					sqlrservercontroller *cont,
					sqlrtranslations *sqlts,
					xmldomnode *parameters);
		bool	run(sqlrserverconnection *sqlrcon,
					sqlrservercursor *sqlrcur,
					const char *query,
					stringbuffer *translatedquery);
    private:
        bool    enabled;
        bool    encrypt;
		bool	debug;
};

sqlrtranslation_sqlparser::sqlrtranslation_sqlparser(
					sqlrservercontroller *cont,
					sqlrtranslations *sqlts,
					xmldomnode *parameters) :
				sqlrtranslation(cont,sqlts,parameters) {

	// "parameters" are the xml parameters from the config file
    debugFunction();
    
    debug=cont->getConfig()->getDebugTranslations();
    
    enabled=charstring::compareIgnoringCase(
        parameters->getAttributeValue("enabled"),"no");
    stdoutput.printf ("enabled:%d\n", enabled);
    encrypt=!charstring::compareIgnoringCase(
        parameters->getAttributeValue("encrypt"),"yes");
    stdoutput.printf ("encrypt:%d\n", encrypt);
}
                
#ifdef __cplusplus
                extern "C" {
#endif
                extern  int sqlparser(const char *sqlquery, int encrypt, char *sqlparsedout);
#ifdef __cplusplus
                }
#endif

bool sqlrtranslation_sqlparser::run(sqlrserverconnection *sqlrcon,
					sqlrservercursor *sqlrcur,
					const char *query,
					stringbuffer *translatedquery) {

	// "query" is the original query
	// "translatedquery" is the translated query

	// This example uses the C function xxx() to convert the query, but
	// the rudiments library provides several C++ classes for character and
	// string manipulation, including the character, charstring, and
	// regularexpression classes.
	//
	
	/* ensqlparser */
    stdoutput.printf("[sqlparser]original query:\n\"%s\"\n\n",query);
    char szOutput[2048] = {0};
    sqlparser (query, 1, szOutput);
    
	stdoutput.printf ("[sqlparser]sqlparser text\n");
    
    
    stdoutput.printf("[sqlparser]szOutput:\n\"%s\"\n\n",szOutput);
    
   translatedquery->clear ();
    translatedquery->write(szOutput);
    
    stdoutput.printf("[sqlparser]translatedquery:\n\"%s\"\n\n",translatedquery->getString());
  //  stdoutput.printf("translatedquery:%s\n", translatedquery->getString());
  //  stdoutput.printf("query:%s\n", query);
  //  translatedquery->clear ();
   // translatedquery->write("insert into runoob_tbl (runoob_title,runoob_author,submission_date) values (\"Learn Linux\",\"CSQ\",now())");

	//for (const char *c=query; *c; c++) {
		//translatedquery->append(query);
	//}
     //   stdoutput.printf("sec translatedquery:%s\n", translatedquery->getString());

	return true;
}

extern "C" {
	SQLRSERVER_DLLSPEC sqlrtranslation *new_sqlrtranslation_sqlparser(
						sqlrservercontroller *cont,
						sqlrtranslations *sqlts,
						xmldomnode *parameters) {
		return new sqlrtranslation_sqlparser(cont,sqlts,parameters);
	}
}
