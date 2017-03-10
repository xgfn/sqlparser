// Copyright (c) 2014  David Muse
// See the file COPYING for more information

#include <sqlrelay/sqlrserver.h>
//#define DEBUG_MESSAGES 1
#include <rudiments/debugprint.h>

class SQLRSERVER_DLLSPEC sqlrresultsettranslation_lktdecrypt :
					public sqlrresultsettranslation {
	public:
			sqlrresultsettranslation_lktdecrypt(
					sqlrservercontroller *cont,
					sqlrresultsettranslations *rs,
					xmldomnode *parameters);
			~sqlrresultsettranslation_lktdecrypt();
		bool	run(sqlrserverconnection *sqlrcon,
					sqlrservercursor *sqlrcur,
					const char *fieldname,
					uint32_t fieldindex,
					const char **field,
					uint64_t *fieldlength);
	private:

		bool	enabled;

		bool	debug;
};

sqlrresultsettranslation_lktdecrypt::
	sqlrresultsettranslation_lktdecrypt(
				sqlrservercontroller *cont,
				sqlrresultsettranslations *rs,
				xmldomnode *parameters) :
				sqlrresultsettranslation(cont,rs,parameters) {


}

sqlrresultsettranslation_lktdecrypt::
	~sqlrresultsettranslation_lktdecrypt() {
}
#ifdef __cplusplus
                    extern "C" {
#endif
                    extern int sql_decrypt (const char *input, char *output, int *outlen);
#ifdef __cplusplus
                    }
#endif

bool sqlrresultsettranslation_lktdecrypt::run(
					sqlrserverconnection *sqlrcon,
					sqlrservercursor *sqlrcur,
					const char *fieldname,
					uint32_t fieldindex,
					const char **field,
					uint64_t *fieldlength) {

    stdoutput.printf ("field:%s\n", *field);
    stdoutput.printf ("fieldlength:%d\n", *fieldlength);

    if ((field[0][0] == '$') && (field[0][1] == 'l') && (field[0][2] == 'k') && (field[0][3] == 't'))
    {
        stdoutput.printf ("xxx\n");
        char *szoutput =  new char[2048] ;
        int outlen = 0;
        sql_decrypt (((*field) + 4), szoutput, &outlen);
        *field = szoutput;
        *fieldlength = outlen;

        stdoutput.printf ("after decode:\n");
        stdoutput.printf ("field:%s\n", *field);
        stdoutput.printf ("*fieldlength:%d\n", *fieldlength);
    }
	return true;
}

extern "C" {
	SQLRSERVER_DLLSPEC sqlrresultsettranslation
			*new_sqlrresultsettranslation_lktdecrypt(
					sqlrservercontroller *cont,
					sqlrresultsettranslations *rs,
					xmldomnode *parameters) {
		return new sqlrresultsettranslation_lktdecrypt(
							cont,rs,parameters);
	}
}
