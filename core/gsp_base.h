/*! 
\mainpage 
General SQL Parser C/C++ version is written in ANSI-C, 
so this SQL library can be used by the most C/C++ compilers including but not limited to GCC, Objective-C, Mircosoft VC, Borland C++ builder. This SQL library can be used under various platforms such as Liunx, HP-UX, IBM AIX, SUN solaris, MAC-OS and windows.

General SQL Parser C/C++ version is valuable because it provides an in-depth and detailed analysis of SQL scripts for various databases, including Oracle, SQL Server, DB2, MySQL,PostgreSQL, Teradata and Access. Without a complete query parser like this, such a task will be impossible. You now have the chance to fully incorporate this Ansi C/C++ SQL parser into your products, instantly adding a powerful SQL processing capability to your C and C++ programs.

You can always get the latest version from our official site: http://www.sqlparser.com

\subpage getting_started "Getting Started"

\subpage history "Version history"

\subpage demos "demos"

*/

/*! \page getting_started Getting Started
<h4>Directories and files:</h4>
+ core: header files for core parser.
+ lib: library for core parser and collection 
  - gspcore.lib, core parser library for windows
  - libgspcore.a, core parser library for Linux
  - gspcollection.lib, extension library for windows
	+ you can build this library from the source code under ext/collection
  - libgspcollection.a, core parser library for Linux
    + you can build this library from the source code under ext/collection

+ ext: source code for extensions of core parser including visitors and iterators.
+ [demos](demos.html): demos demonstrate how to use this SQL Parser.

<h4>Write Programs That Use General SQL Parser(GSP)</h4>

Below is a simple C program that demonstrates how to use the C/C++ interface to GSP.

\code
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
				"                     WHERE  employee_id = 141);\n"
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
\endcode

<h4>compile this simple C program under windows using Visual C</h4>

Set environment for using Microsoft Visual Studio 2008 x86 tools by running:
    
    C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat

then, generate first.exe by running:

    cl first.c  /I ../core /link ../lib/gspcore.lib


<h4>compile this simple C program under Linux using gcc</h4>

    gcc -O2 first.c -I../core -L../lib -lgspcore -o first


*/

/*! \page history Version history

+ version C 1.0.3 (2016-04-08)
 - [PostgreSQL] fix an access violation bug when process for update clause in gettablecolumn demo

+ version C 1.0.2 (2015-12-18)
 - [MySQL] able to detect syntax error while * is not the first column in select list.

+ version C 1.0.2 (2015-12-07)
 - [MySQL] support join table reference in delete statement(multi-table syntax)

+ version C 1.0.1 (2015-11-11)
 - [MySQL] able to detect syntax error in group by clause when desc/asc is mistyped.
 - [MySQL] fix a bug can't parse join without on clause.
 - [MySQL] support create schema statement.
 - [MySQL] fix a bug can't recoginze show session variables.

+ version C 0.4.1 (2015-10-16)
 - [PostgreSQL] support set schema statement.

+ version C 0.4.0 (2015-08-17)
 - [MySQL] Add support for use database statement: gsp_useDatabaseStmt
 - [PostgreSQL] fix a AV bug when parsing name list in table alias.


+ version C 0.3.8 (2015-03-28)
 - [MySQL] fix a bug can't parse column start with number in a qualified name:
 - [sql injection demo] improvement to handle always_false_condition

+ version C 0.3.7 (2015-03-19)
 - [sql server] fix a bug can't parse operator <> if there is no space before @ like  f<>@currentval
 - [sql server] fix a bug can't parse set clause if a variable is left operarand.
 - [general] fix a bug can't parse script if it includes more than 50 queries	
 - [PostgreSQL] fix a bug when parse query including limit clause.
 - [demos] improvement of sqlinjection

+ version C 0.3.7 (2015-02-21)
 - [SQL Server] fix a bug can't parse update statement inside if

+ version C 0.3.6 (2014-12-18)
 - [General] out of memory in release version after processing more than 4 millions sql queries.

+ version C 0.3.6 (2014-12-17)
 - [General] Improve lexer to handle illegal string correctly.

+ version C 0.3.5 (2014-11-27)
 - [SQL Server] Fix a bug when parse SELECT causes an infinite loop
 - [Oracle] support xml attribute clause in xmlelement function


+ version C 0.3.4 (2014-11-16)
 - [Oracle] segfault error while tokenize sql only including  *\/ character

+ version C 0.3.3 (2014-08-01)
 - [general] fix a bug can't read sql file(no content read) under 64 bit windows, 


+ version C 0.3.3 (2014-06-12)
 - [SQL Server] support sp_executesql statement
 - [SQL Server] support alter index statement
 - [SQL Server] support create database statement

+ version C 0.3.3 (2014-06-11)
 - [SQL Server] support alter database statement
 - [SQL Server] fully support drop database statement

+ version C 0.3.3 (2014-05-30)
 - [SQL Server] fix a bug can't parse "bad" query when start part of comment was included in literal.


+ version C 0.3.2 (2014-03-24)
 - [MySQL] replace into statement share the same structure with Insert statement
 - [MySQL] fix a bug can't parse > operator if followd by ? without a space
 - [MySQL] support full outer join


+ version C 0.3.2 (2014-03-20)
 - [general] fix a bug can't calculate token position correctly when ( at the end of line in Oracle database.
 - [PostgreSQL] fully support drop table statement

+ version C 0.3.1 (2014-02-19)
 - [General] shared library for Linux 64 bit
 - [SQL Server] fix a big can't get detailed table information from drop table statment
 - [PostgreSQL] fully support drop table statement

+ version C 0.3.0 (2014-01-03)
 - [MySQL] fix a bug can't parse binary operator b¡¯<binary number>¡¯

+ version C 0.3.0 (2013-12-27)
 - [MySQL] add support for into outfile clause in select statement
 - [MySQL] fix a bug can't parse ON DUPLICATE KEY of insert statement

+ version C 0.2.6 (2013-10-31)
 - [DB2] support SYS naming option where tables is identified as library/file (library=schema and file=table)


+ version C 0.2.4 (2013-07-13)
 - add an extension module to support modify sql feature.
 - add new demo: antiSQLInjection, illustrates how to detect sql injection  by using general sql parser library.
 - add new demo: oracleJoinConverter, illustrate how to Rewrite Oracle  proprietary joins to ANSI SQL compliant joins.
 - add new demo: gettablecolumns, this demo illustrates how to get all table 
	and column in a sql query.


+ Jan 2013
	- fully support Oracle including plsql with all parse tree nodes available.
	- other databases still under development(all will be available before May 2013).
	- library shipped in 32-bit for windows and linux, 64-bit library is available on request(info@sqlparser.com) 
	- demos demonstrate how to 
		+ get table/columns [collectSqlInfo.c]
		+ find and replace constants [collectConst.c]
		+ Visit SQL statement recursively [iterateStatement.c]
		+ iterate parse tree nodes [simpleNodeVisitor.c]
		+ SQL expression parse tree traversal in preorder/inorder/postorder [expressionTraverser.c]

+ Mar 2012
	- The first version.
	- Main feature is offline check syntax, support Oracle, SQL Server/Sybase, DB2, MySQL, Teradata, PostgreSQL, Netezza and MDX.
	- Provides static library that can be used in VC++ and GCC under windows and Linux platform.
*/

/*! \page demos demos
	+ get table/columns [collectSqlInfo.c]
	+ find and replace constants [collectConst.c]
	+ Visit SQL statement recursively [iterateStatement.c]
	+ iterate parse tree nodes [simpleNodeVisitor.c]
	+ SQL expression parse tree traversal in preorder/inorder/postorder [expressionTraverser.c]
*/

/*!
**  \file  gsp_base.h
** \brief Includes all basic data structures used by SQL Parser.
*/

#ifndef GSP_H
#define GSP_H

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif



#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#include "gsp_base_keywords.h"

#define GSP_VERSION_ID	"1.0.3"

#define INTIAL_BUFSIZE  16384
#define MAX_MATCHES		1024*20*10*2
#define MAX_RULES		256*2*10
#define yymaxdepth		1048
#define MAX_ERROR_SLOTS	50

//#define DB_KEYWORD(a,b,c) {a,b,c},
#define GSP_DEFAULT_ENCODING			0
#define GSP_UTF8						1
#define GSP_UTF16LE						2
#define GSP_UTF16BE						3
#define GSP_UTF16						4    /* Use native byte order */
#define SEMICOLON_CODE	59
// slash: /
#define SLASH_CODE		47
#define	COMMA_CODE		44
#define	LEFT_PARENTHESIS_CODE	40
#define	RIGHT_PARENTHESIS_CODE	41
#define PERIOD_CODE		46

#define yyconst const

#define lengthof(array) (sizeof (array) / sizeof ((array)[0]))
#define	elog(a_int, b_string,c_size) fprintf(stderr,b_string,c_size)


/*
** Result Codes
*/
#define GSP_OK		0	/* Successful result */
#define GSP_ERROR	1
#define GSP_NOMEM   2   /* A malloc() failed */

/*
** Forward references to structures
*/


typedef struct	gsp_objectname		gsp_objectname;
typedef struct	gsp_symbol			gsp_symbol;
typedef struct	gsp_base_statement	gsp_base_statement;
//typedef struct	gsp_valueClause		gsp_valueClause;
//typedef struct	gsp_join			gsp_join;
//typedef struct	gsp_joinitem		gsp_joinitem;
//typedef enum	EDBObjectType		EDBObjectType;



typedef enum gsp_efindsqlstatetype {
    stnormal,stsqlplus,stsql,ststoredprocedure,sterror,stblock,sttrycatch,ststoredprocedurebody,stExec
}gsp_efindsqlstatetype;


typedef enum EErrorType {
    sperror,spfatalerror,spfatalabort,
    spwarning,sphint,sppperror,spppexception,
    spppdifferencetext,spmssqlrecover,
	spmem,
    spwarningdbobject
}EErrorType;


/*!
** unique code represents source token generated by scanner.
*/
typedef enum gsp_token_code 
{gsp_tc_cmtslashstar = 257
,gsp_tc_cmtdoublehyphen =  258
,gsp_tc_lexspace  = 259
,gsp_tc_lexnewline =  260
,gsp_tc_fconst  = 261
,gsp_tc_sconst  = 262
,gsp_tc_iconst  = 263
,gsp_tc_ident  = 264
,gsp_tc_op  = 265
,gsp_tc_cmpop =  266
,gsp_tc_bind_v  = 267
,gsp_tc_assign_sign =  268
,gsp_tc_double_dot =  269
,gsp_tc_label_begin =  270
,gsp_tc_label_end =  271
,gsp_tc_substitution_v =  272
,gsp_tc_sqlpluscmd  = 273
,gsp_tc_lex_error  = 274
,gsp_tc_variable  = 275
,gsp_tc_mslabel  = 276
,gsp_tc_leftjoin_op =  277
,gsp_tc_rightjoin_op =  278
,gsp_tc_db2label  = 279
,gsp_tc_ref_arrow =  280
,gsp_tc_scriptoptions =  281
,gsp_tc_mysqllabel =  282
,gsp_tc_concatenationop =  283
,gsp_tc_not_deferrable =  284
,gsp_tc_for1 =  285
,gsp_tc_stmt_delimiter =  286
,gsp_tc_m_clause =  287
,gsp_tc_k_clause =  288
,gsp_tc_outer_join =  289
,gsp_tc_not_equal =  290
,gsp_tc_not_great =  291
,gsp_tc_not_less =  292
,gsp_tc_great_equal =  293
,gsp_tc_less_equal =  294
,gsp_tc_exponentiate =  295
,gsp_tc_locktable =  296
,gsp_tc_foreign2 =  297
,gsp_tc_constraint2 =  298
,gsp_tc_primary2  = 299
,gsp_tc_unique2 =  300
,gsp_tc_select =  301
,gsp_tc_insert =  302
,gsp_tc_delete =  303
,gsp_tc_update =  304
,gsp_tc_if  = 305
,gsp_tc_for =  306
,gsp_tc_create =  307
,gsp_tc_table =  308
,gsp_tc_index =  309
,gsp_tc_view =  310
,gsp_tc_with =  311
,gsp_tc_start =  312
,gsp_tc_end =  313
,gsp_tc_drop =  314
,gsp_tc_declare =  315
,gsp_tc_case  = 316
,gsp_tc_where =  317
,gsp_tc_having =  318
,gsp_tc_and  = 319
,gsp_tc_or  = 320
,gsp_tc_not =  321
,gsp_tc_when =  322
,gsp_tc_on  = 323
,gsp_tc_join  = 324
,gsp_tc_set =  325
,gsp_tc_values =  326
,gsp_tc_object =  327
,gsp_tc_record =  328
,gsp_tc_from =  329
,gsp_tc_group =  330
,gsp_tc_sort =  331
,gsp_tc_into =  332
,gsp_tc_order =  333
,gsp_tc_fetch =  334
,gsp_tc_decode =  335
,gsp_tc_over =  336
,gsp_tc_extract =  337
,gsp_tc_distinct =  338
,gsp_tc_all =  339
,gsp_tc_by =  340
,gsp_tc_as =  341
,gsp_tc_union =  342
,gsp_tc_left =  343
,gsp_tc_right =  344
,gsp_tc_inner =  345
,gsp_tc_full =  346
,gsp_tc_outer =  347
,gsp_tc_then =  348
,gsp_tc_else =  349
,gsp_tc_between =  350
,gsp_tc_begin =  351
,gsp_tc_except =  352
,gsp_tc_minus =  353
,gsp_tc_intersect =  354
,gsp_tc_bit  = 355
,gsp_tc_tinyint =  356
,gsp_tc_smallint =  357
,gsp_tc_mediumint =  358
,gsp_tc_middleint =  359
,gsp_tc_int =  360
,gsp_tc_integer =  361
,gsp_tc_int1 =  362
,gsp_tc_int2 =  363
,gsp_tc_int3 =  364
,gsp_tc_int4 =  365
,gsp_tc_int8 =  366
,gsp_tc_bigint =  367
,gsp_tc_float =  368
,gsp_tc_float4 =  369
,gsp_tc_float8 =  370
,gsp_tc_num =  371
,gsp_tc_numeric =  372
,gsp_tc_number =  373
,gsp_tc_real =  374
,gsp_tc_decimal =  375
,gsp_tc_dec =  376
,gsp_tc_double =  377
,gsp_tc_precision =  378
,gsp_tc_char =  379
,gsp_tc_nchar =  380
,gsp_tc_character =  381
,gsp_tc_varchar =  382
,gsp_tc_varchar2 =  383
,gsp_tc_national =  384
,gsp_tc_nvarchar =  385
,gsp_tc_nvarchar2 =  386
,gsp_tc_varbinary =  387
,gsp_tc_varying =  388
,gsp_tc_tinytext =  389
,gsp_tc_text =  390
,gsp_tc_mediumtext =  391
,gsp_tc_longtext =  392
,gsp_tc_time =  393
,gsp_tc_date =  394
,gsp_tc_timestamp =  395
,gsp_tc_year =  396
,gsp_tc_local =  397
,gsp_tc_zone =  398
,gsp_tc_long =  399
,gsp_tc_raw =  400
,gsp_tc_blob =  401
,gsp_tc_tinyblob =  402
,gsp_tc_mediumblob =  403
,gsp_tc_longblob =  404
,gsp_tc_clob =  405
,gsp_tc_nclob =  406
,gsp_tc_dbclob  = 407
,gsp_tc_bfile  = 408
,gsp_tc_large  = 409
,gsp_tc_data  = 410
,gsp_tc_binary  = 411
,gsp_tc_graphic =  412
,gsp_tc_vargraphic =  413
,gsp_tc_datalink  = 414
,gsp_tc_enum  = 415
,gsp_tc_interval =  416
,gsp_tc_urowid =  417
,gsp_tc_merge =  418
,gsp_tc_commit  = 419
,gsp_tc_rollback  = 420
,gsp_tc_savepoint  = 421
,gsp_tc_revoke  = 422
,gsp_tc_grant  = 423
,gsp_tc_truncate =  424
,gsp_tc_database =  425
,gsp_tc_alter =  426
,gsp_tc_return =  427
,gsp_tc_add =  428
,gsp_tc_close =  429
,gsp_tc_continue =  430
,gsp_tc_backup =  431
,gsp_tc_break =  432
,gsp_tc_bulk =  433
,gsp_tc_dbcc =  434
,gsp_tc_deallocate =  435
,gsp_tc_deny =  436
,gsp_tc_disable =  437
,gsp_tc_enable =  438
,gsp_tc_exec =  439
,gsp_tc_execute =  440
,gsp_tc_goto =  441
,gsp_tc_kill =  442
,gsp_tc_open =  443
,gsp_tc_save =  444
,gsp_tc_move =  445
,gsp_tc_print =  446
,gsp_tc_raiserror =  447
,gsp_tc_readtext =  448
,gsp_tc_receive =  449
,gsp_tc_reconfigure =  450
,gsp_tc_restore =  451
,gsp_tc_send =  452
,gsp_tc_setuser =  453
,gsp_tc_shutdown =  454
,gsp_tc_sign =  455
,gsp_tc_try =  456
,gsp_tc_updatetext =  457
,gsp_tc_use  = 458
,gsp_tc_waitfor =  459
,gsp_tc_while =  460
,gsp_tc_writetext =  461
,gsp_tc_catch  = 462
,gsp_tc_go  = 463
,gsp_tc_openrowset =  464
,gsp_tc_analyze =  465
,gsp_tc_associate =  466
,gsp_tc_audit  = 467
,gsp_tc_call  = 468
,gsp_tc_comment =  469
,gsp_tc_disassociate =  470
,gsp_tc_explain =  471
,gsp_tc_flashback =  472
,gsp_tc_lock  = 473
,gsp_tc_noaudit =  474
,gsp_tc_purge  = 475
,gsp_tc_rename  = 476
,gsp_tc_procedure =  477
,gsp_tc_function =  478
,gsp_tc_package =  479
,gsp_tc_allocate=   480
,gsp_tc_connect =  481
,gsp_tc_describe =  482
,gsp_tc_disconnect =  483
,gsp_tc_flush =  484
,gsp_tc_free =  485
,gsp_tc_get =  486
,gsp_tc_include =  487
,gsp_tc_iterate =  488
,gsp_tc_leave=   489
,gsp_tc_loop =  490
,gsp_tc_prepare =  491
,gsp_tc_refresh =  492
,gsp_tc_release =  493
,gsp_tc_repeat =  494
,gsp_tc_resignal =  495
,gsp_tc_signal =  496
,gsp_tc_cache =  497
,gsp_tc_change =  498
,gsp_tc_check =  499
,gsp_tc_checksum  = 500
,gsp_tc_do  = 501
,gsp_tc_handler  = 502
,gsp_tc_load  = 503
,gsp_tc_optimize =  504
,gsp_tc_replace =  505
,gsp_tc_repair =  506
,gsp_tc_reset =  507
,gsp_tc_show =  508
,gsp_tc_stop =  509
,gsp_tc_unlock =  510
,gsp_tc_terminate =  511
,gsp_tc_to =  512
,gsp_tc_primary =  513
,gsp_tc_unique =  514
,gsp_tc_constraint =  515
,gsp_tc_foreign =  516
,gsp_tc_revert =  517
,gsp_tc_checkpoint =  518
,gsp_tc_calculate =  519
,gsp_tc_clear =  520
,gsp_tc_drillthrough =  521
,gsp_tc_freeze =  522
,gsp_tc_existing =  523
,gsp_tc_scope =  524
,gsp_tc_is =  525
,gsp_tc_body =  526
,gsp_tc_abort =  527
,gsp_tc_using =  528
} gsp_token_code;

//mdx quoted ident
#define gsp_tc_quoted_ident 282
#define gsp_tc_amp_quoted_id 285
#define gsp_tc_amp_unquoted_id 286

#define	gsp_tc_semi_colon_after_begin	(gsp_tc_abort + 4)
#define	gsp_tc_semi_colon_in_openrowset	(gsp_tc_abort + 5)

//mssql
#define	gsp_tc_mssql_output			(gsp_tc_abort + 6)
#define	gsp_tc_mssql_out			(gsp_tc_abort + 7)
#define	gsp_tc_mssql_readonly		(gsp_tc_abort + 8)
#define	gsp_tc_mssql_sp_executesql		(gsp_tc_abort + 36)


#define	gsp_tc_custom	(gsp_tc_abort + 2)
#define	gsp_tc_logical_and	(gsp_tc_abort + 2)
#define	gsp_tc_logical_or	(gsp_tc_abort + 3)
#define gsp_tc_compoundassignmentoperator	(gsp_tc_abort + 3)
#define gsp_tc_left_shift	(gsp_tc_abort + 4)
#define gsp_tc_right_shift	(gsp_tc_abort + 5)

#define gsp_tc_keep							(gsp_tc_abort + 5)
#define gsp_tc_keep_before_dense_rank		(gsp_tc_abort + 6)
#define gsp_tc_dense_rank					(gsp_tc_abort + 7)

//db2
#define gsp_tc_rr							(gsp_tc_abort + 3)
#define gsp_tc_rs							(gsp_tc_abort + 4)
#define gsp_tc_cs							(gsp_tc_abort + 5)
#define gsp_tc_ur							(gsp_tc_abort + 6)
#define gsp_tc_with_isolation				(gsp_tc_abort + 7)
#define gsp_tc_echo							(gsp_tc_abort + 9)

//teradata
#define	gsp_tc_bt			(gsp_tc_abort + 2)
#define	gsp_tc_collect		(gsp_tc_abort + 3)
#define	gsp_tc_cm			(gsp_tc_abort + 4)
#define	gsp_tc_ct			(gsp_tc_abort + 5)
#define	gsp_tc_del			(gsp_tc_abort + 6)
#define	gsp_tc_diagnostic	(gsp_tc_abort + 7)
#define	gsp_tc_dump			(gsp_tc_abort + 8)
#define	gsp_tc_echo			(gsp_tc_abort + 9)
#define	gsp_tc_et			(gsp_tc_abort + 10)
#define	gsp_tc_give			(gsp_tc_abort + 11)
#define	gsp_tc_help			(gsp_tc_abort + 12)
#define	gsp_tc_ins			(gsp_tc_abort + 13)
#define	gsp_tc_logoff		(gsp_tc_abort + 14)
#define	gsp_tc_logon		(gsp_tc_abort + 15)
#define	gsp_tc_modify		(gsp_tc_abort + 16)
#define	gsp_tc_position		(gsp_tc_abort + 17)
#define	gsp_tc_restart		(gsp_tc_abort + 18)
#define	gsp_tc_rewind		(gsp_tc_abort + 19)
#define	gsp_tc_sel			(gsp_tc_abort + 20)
#define	gsp_tc_ss			(gsp_tc_abort + 21)
#define	gsp_tc_upd			(gsp_tc_abort + 22)
#define	gsp_tc_wait			(gsp_tc_abort + 23)
    

//postgresql
#define	gsp_tc_cluster			(gsp_tc_abort + 2)
#define	gsp_tc_copy			(gsp_tc_abort + 3)
#define	gsp_tc_discard			(gsp_tc_abort + 4)
#define	gsp_tc_listen			(gsp_tc_abort + 5)
#define	gsp_tc_notify			(gsp_tc_abort + 6)
#define	gsp_tc_reassign			(gsp_tc_abort + 7)
#define	gsp_tc_reindex			(gsp_tc_abort + 8)
#define	gsp_tc_security			(gsp_tc_abort + 9)
#define	gsp_tc_unlisten			(gsp_tc_abort + 10)
#define	gsp_tc_param			(gsp_tc_abort + 11)

//oracle
#define	gsp_tc_before			(gsp_tc_abort + 12)
#define	gsp_tc_after			(gsp_tc_abort + 13)

//informix
#define gsp_tc_connect_to  276
#define gsp_tc_unload			(gsp_tc_abort + 5)
#define gsp_tc_whenever			(gsp_tc_abort + 6)
#define gsp_tc_put				(gsp_tc_abort + 7)
#define gsp_tc_output			(gsp_tc_abort + 8)
#define gsp_tc_info				(gsp_tc_abort + 9)

/*!
** type of various sql statement.
*/
typedef enum EStmtType {
		sstunknown,sstinvalid,sstselect,sstdelete,sstupdate,
		sstinsert,sstcreatetable,sstcreateview,sstsqlpluscmd, sstcreatesequence,
		
		sstdropsequencestmt,sstdroptypestmt,sstplsql_packages,sstplsql_objecttype,sstcreate_plsql_procedure,
		sstcreate_plsql_function,sstcreate_varray_type,sstcreate_nested_table_type,sstcreateobjecttablestmt,sstplsql_block,
		sstplsql_createprocedure,sstplsql_createfunction,sstplsql_createpackage,sstplsql_createtrigger,sstplsql_createtype,
		sstplsql_createtypebody,sstplsql_tabletypedef,sstplsql_varraytypedef,sstplsql_createtype_placeholder,
		sstaltersession,sstcreateindex,sstdropindex,sstdropview,sstmerge,
		sstdroptable,sstaltertable,sstcommit,sstrollback,sstsavepoint,
		sstsettransaction,sstlocktable,sstmssqldummystmt,sstcreatedatabase,sstrevoke,
        sstTruncate,   sstcreatematerializedview,  sstcreatesynonym,
		sstmssqlcreateprocedure,sstmssqlcreatetrigger,sstmssqlcreatefunction,sstmssqlalterprocedure,sstmssqlaltertrigger,
		sstmssqlalterfunction,sstmssqlif,sstmssqlblock,sstmssqlgo,sstmssqldbcc,
		sstmssqlrestore,sstmssqlbackup,sstmssqlrevoke,sstmssqlreadtext,sstmssqlgrant,
		sstmssqltruncatetable,sstmssqladdsignature,sstmssqlalterapplicationrole,sstmssqlalterassembly,sstmssqlalterasymmetrickey,
		sstmssqlalterauthorization,sstmssqlaltercertificate,sstmssqlaltercredential,
		sstmssqlalterdatabase,sstmssqlcreatedatabase,
		sstmssqlalterendpoint,
		sstmssqlalterfulltextcatalog,sstmssqlalterfulltextindex,sstmssqlalterindex,sstmssqlalterlogin,sstmssqlaltermasterkey,
		sstmssqlaltermessagetype,sstmssqlalterpartitionfunction,sstmssqlalterpartitionscheme,sstmssqlalterqueue,sstmssqlalterremoteservicebinding,
		sstmssqlalterrole,sstmssqlalterroute,sstmssqlalterschema,sstmssqlalterservice,sstmssqlalterservicemasterkey,
		sstmssqlaltersymmetrickey,sstmssqlalteruser,sstmssqlalterview,sstmssqlalterxmlschemacollection,sstmssqlbackupdatabase,
		sstmssqlbackupcertificate,sstmssqlbackuplog,sstmssqlbackupmasterkey,sstmssqlbackupservicemasterkey,sstmssqlbeginconversationtimer,
		sstmssqlbegindialog,sstmssqlbegindistributed,sstmssqlbegintran,sstmssqlbreak,sstmssqlbulkinsert,
		sstmssqlclose,sstmssqlclosemasterkey,sstmssqlclosesymmetrickey,sstmssqlcontinue,sstmssqlcreateaggregate,
		sstmssqlcreateapplicationrole,sstmssqlcreateassembly,sstmssqlcreateasymmetrickey,sstmssqlcreatecertificate,sstmssqlcreatecontract,
		sstmssqlcreatecredential,sstmssqlcreatedefault,sstmssqlcreateendpoint,sstmssqlcreateeventnotification,sstmssqlcreatefulltextcatalog,
		sstmssqlcreatefulltextindex,sstmssqlcreatelogin,sstmssqlcreatemasterkey,sstmssqlcreatemessagetype,sstmssqlcreatepartitionfunction,
		sstmssqlcreatepartitionscheme,sstmssqlcreatequeue,sstmssqlcreateremoteservicebinding,sstmssqlcreaterole,sstmssqlcreateroute,
		sstmssqlcreaterule,sstmssqlcreateschema,sstmssqlcreateservice,sstmssqlcreatestatistics,sstmssqlcreatesymmetrickey,
		sstmssqlcreatesynonym,sstmssqlcreatetype,sstmssqlcreateuser,sstmssqlcreatexmlschemacollection,sstmssqldeallocate,
		sstmssqldeclare,sstmssqldeny,sstmssqldisabletrigger,sstmssqldropaggregate,sstmssqldropapplicationrole,
		sstmssqldropassembly,sstmssqldropasymmetrickey,sstmssqldropcertificate,sstmssqldropcontract,sstmssqldropcredential,
		sstmssqldropdatabase,sstmssqldropdefault,sstmssqldropendpoint,sstmssqldropeventnotification,sstmssqldropfulltextcatalog,
		sstmssqldropfulltextindex,sstmssqldropfunction,sstmssqldropdbobject,sstmssqldropindex,sstmssqldroplogin,
		sstmssqldropmasterkey,sstmssqldropmessagetype,sstmssqldroppartitionfunction,sstmssqldroppartitionscheme,sstmssqldropprocedure,
		sstmssqldropqueue,sstmssqldropremoteservicebinding,sstmssqldroprole,sstmssqldroproute,sstmssqldroprule,
		sstmssqldropschema,sstmssqldropservice,sstmssqldropsignature,sstmssqldropstatistics,sstmssqldropsymmetrickey,
		sstmssqldropsynonym,sstmssqldroptable,sstmssqldroptrigger,sstmssqldroptype,sstmssqldropuser,
		sstmssqldropview,sstmssqldropxmlschemacollection,sstmssqlenabletrigger,sstmssqlendconversation,sstmssqlexecuteas,
		sstmssqlfetch,sstmssqlgoto,sstmssqlkill,sstmssqlkillquerynotificationsubscription,sstmssqlkillstats,
		sstmssqlmoveconversation,sstmssqlopen,sstmssqlopenmasterkey,sstmssqlopensymmetrickey,sstmssqlprint,
		sstmssqlraiserror,sstmssqlreceive,sstmssqlreconfigure,sstmssqlrestoredatabase,sstmssqlrestorefilelistonly,
		sstmssqlrestoreheaderonly,sstmssqlrestorelabelonly,sstmssqlrestorelog,sstmssqlrestoremasterkey,sstmssqlrestorerewindonly,
		sstmssqlrestoreservicemasterkey,sstmssqlrestoreverifyonly,sstmssqlrevert,sstmssqlreturn,sstmssqlsavetran,
		sstmssqlselect,sstmssqlsendonconversation,sstmssqlset,sstmssqlsetuser,sstmssqlshutdown,
		sstmssqlsign,sstmssqlbegintry,sstmssqlbegincatch,sstmssqlupdatestatistics,sstmssqlupdatetext,
		sstmssqluse,sstmssqlwaitfor,sstmssqlwhile,sstmssqlcte,sstmssqlwithxmlnamespaces,
		sstmssqlwritetext,sstmssqlexec,sstexecutestmt,sstsetstmt,sstmssqlcommit,
		sstmssqlrollback,sstraiserror,sstmssqlwithas,sstmssqllabel,ssterrorstmt,
		sstmssqldrop,sstmssqlstmtstub,sstmssqlcheckpoint,
		sstoraclealtercluster,sstoraclealterdatabase,
		sstmssqlexecfake,
		sstoraclealterdimension,sstoraclealterdiskgroup,sstoraclealterfunction,sstoraclealterindex,sstoraclealterindextype,
		sstoraclealterjava,sstoraclealtermaterializedview,sstoraclealtermaterializedviewlog,sstoraclealteroperator,sstoraclealteroutline,
		sstoraclealterpackage,sstoraclealterprocedure,sstoraclealterprofile,sstoraclealterresourcecost,sstoraclealterrole,
		sstoraclealterrollbacksegment,sstoraclealtersequence,sstoraclealtersession,sstoraclealtersystem,sstoraclealtertablespace,
		sstoraclealtertrigger,sstoraclealtertype,sstoraclealteruser,sstoraclealterview,sstoracleanalyze,
		sstoracleassociatestatistics,sstoracleaudit,sstoraclecall,sstoraclecomment,sstoraclecommit,
		sstoraclecreatecluster,sstoraclecreatecontext,sstoraclecreatecontrolfile,sstoraclecreatedatabase,sstoraclecreatedatabaselink,
		sstoraclecreatedimension,sstoraclecreatedirectory,sstoraclecreatediskgroup,sstoraclecreatefunction,sstoraclecreateindex,
		sstoraclecreateindextype,sstoraclecreatejava,sstoraclecreatelibrary,sstoraclecreatematerializedview,sstoraclecreatematerializedviewlog,
		sstoraclecreateoperator,sstoraclecreateoutline,sstoraclecreatepackagebody,sstoraclecreatepfile,
		sstoraclecreateprocedure,sstoraclecreateprofile,sstoraclecreaterestorepoint,sstoraclecreaterole,sstoraclecreaterollbacksegment,
		sstoraclecreateschema,sstoraclecreatesequence,sstoraclecreatespfile,sstoraclecreatesynonym,sstoraclecreatetablespace,
		sstoraclecreatetrigger,sstoraclecreatetype, sstoraclecreateuser,sstoraclecreateview,
		sstoracledisassociatestatistics,sstoracledropcluster,sstoracledropcontext,sstoracledropdatabase,sstoracledropdatabaselink,
		sstoracledropdimension,sstoracledropdirectory,sstoracledropdiskgroup,sstoracledropfunction,sstoracledropindex,
		sstoracledropindextype,sstoracledropjava,sstoracledroplibrary,sstoracledropmaterializedview,sstoracledropmaterializedviewlog,
		sstoracledropoperator,sstoracledropoutline,sstoracledroppackage,sstoracledropprocedure,sstoracledropprofile,
		sstoracledroprestorepoint,sstoracledroprole,sstoracledroprollbacksegment,sstoracledropsequence,sstoracledropsynonym,
		sstoracledroptable,sstoracledroptablespace,sstoracledroptrigger,sstoracledroptype,sstoracledroptypebody,
		sstoracledropuser,sstoracledropview,sstoracleexplainplan,sstoracleflashbackdatabase,
		sstoracleflashbacktable,sstoraclegrant,sstoraclelocktable,sstoraclenoaudit,sstoraclepurge,
		sstoraclerename,sstoraclerevoke,sstoraclerollback,sstoraclesavepoint,sstoraclesetconstraint,
		sstoraclesetrole,sstoraclesettransaction,sstoracletruncate,
        sstmysqlalterdatabase,sstmysqlalterfunction,
		sstmysqlalterprocedure,sstmysqlalterview,sstmysqlanalyzetable,sstmysqlbackuptable,sstmysqlcacheindex,
		sstmysqlcall,sstmysqlcase,sstmysqlchangemasterto,sstmysqlchecktable,sstmysqlchecksumtable,
		sstmysqlclose,sstmysqlcommit,sstmysqlcreatedatabase,sstmysqlcreateindex,sstmysqlcreatefunction,
		sstmysqlcreateprocedure,sstmysqlcreatetrigger,sstmysqlcreateuser,sstmysqlcreateview,sstmysqldeclare,
		sstmysqldescribe,sstmysqldo,sstmysqldropdatabase,sstmysqldropfunction,sstmysqldropindex,
		sstmysqldropprocedure,sstmysqldroptable,sstmysqldroptrigger,sstmysqldropuser,sstmysqldropview,
		sstmysqlexecute,sstmysqlfetch,sstmysqlflush,sstmysqlgrant,sstmysqlhandler,
		sstmysqlif,sstmysqliterate,sstmysqlkill,sstmysqlleave,sstmysqlloaddatainfile,
		sstmysqlloaddatafrommaster,sstmysqlloadindexintocache,sstmysqlloadtable,sstmysqllocktable,sstmysqlloop,
		sstmysqlopen,sstmysqloptimizetable,sstmysqldeallocateprepare,sstmysqldropprepare,sstmysqlprepare,
		sstmysqlpurgelogs,sstmysqlrepeat,sstmysqlreplace,sstmysqlrenametable,sstmysqlrepairtable,
		sstmysqlreleasesavepoint,sstmysqlrenameuser,sstmysqlrest,sstmysqlresetmaster,sstmysqlresetslave,
		sstmysqlrestoretable,sstmysqlrevoke,sstmysqlrollback,sstmysqlsavepoint,sstmysqlreset,
		sstmysqlset,sstmysqlsetautocommit,sstmysqlsettransaction,sstmysqlsetpassword,sstmysqlshowcreatedatabase,
		sstmysqlshowcharacterset,sstmysqlshowcollation,sstmysqlshowcolumns,sstmysqlshowcreatetable,sstmysqlshowcreateview,
		sstmysqlshowcreatefunction,sstmysqlshowcreateprocedure,sstmysqlshowdatabases,sstmysqlshowengines,sstmysqlshowerrors,
		sstmysqlshowgrants,sstmysqlshowfunctionstatus,sstmysqlshowindex,sstmysqlshowinnodbstatus,sstmysqlshowlogs,
		sstmysqlshowprivileges,sstmysqlshowprocesslist,sstmysqlshowstatus,sstmysqlshowtablestatus,sstmysqlshowtables,
		sstmysqlshowtriggers,sstmysqlshowvariables,sstmysqlshowwarnings,sstmysqlshowbinlogevents,sstmysqlshowmasterlogs,
		sstmysqlshowmasterstatus,sstmysqlshowslavehosts,sstmysqlshowslavestatus,sstmysqlstartslave,sstmysqlstarttransaction,
		sstmysqlsetglobalsql_slave_skip_counter,sstmysqlsetsql_log_bin,sstmysqlstopslave,sstmysqltruncate,sstmysqlunlocktable,
		sstmysqluse,sstmysqlwhile,sstmysqlshow,sstmysqlreturn,sstmysqlrepeatstmt,
		sstmysqlwhilestmt,sstmysqlopencursor,sstmysqlfetchcursor,sstmysqlcasestmt,sstmysqlifstmt,
		sstmysqlloopstmt,sstmysqlstmtstub, sstmysqlblock,
        sstdb2allocatecursor,sstdb2alterbufferpool,sstdb2alterdatabasepartitiongroup,sstdb2alterfunction,
		sstdb2altermethod,sstdb2alternickname,sstdb2alterprocedure,sstdb2altersequence,sstdb2alterserver,
		sstdb2altertable,sstdb2altertablespace,sstdb2altertype,sstdb2alterusermapping,sstdb2alterview,
		sstdb2alterwrapper,sstdb2associatelocators,sstdb2begindeclaresection,sstdb2call,sstdb2case,
		sstdb2close,sstdb2comment,sstdb2commit,sstdb2connect,sstdb2createalias,
		sstdb2createbufferpool,sstdb2createdatabasepartitiongroup,sstdb2createdistincttype,sstdb2createeventmonitor,sstdb2createfunction,
		sstdb2createfunctionmapping,sstdb2createindex,sstdb2createindexextension,sstdb2createmethod,sstdb2createnickname,
		sstdb2createprocedure,sstdb2createschema,sstdb2createsequence,sstdb2createserver,sstdb2createtablespace,
		sstdb2createtransform,sstdb2createtrigger,sstdb2createtype,sstdb2createtypemapping,sstdb2createusermapping,
		sstdb2createwrapper,sstdb2declarecursor,sstdb2declareglobaltemporarytable,sstdb2describe,sstdb2disconnect,
		sstdb2drop,sstdb2echo,
		sstdb2enddeclaresection,sstdb2execute,sstdb2executeimmediate,sstdb2explain,
		sstdb2fetch,sstdb2flusheventmonitor,sstdb2flushpackagecache,sstdb2for,sstdb2freelocator,
		sstdb2getdiagnostics,sstdb2goto,sstdb2grant,sstdb2if,sstdb2include,
		sstdb2iterate,sstdb2leave,sstdb2locktable,sstdb2loop,sstdb2open,
		sstdb2prepare,sstdb2refreshtable,sstdb2release,sstdb2releasesavepoint,sstdb2rename,
		sstdb2renametablespace,sstdb2repeat,sstdb2resignal,sstdb2return,sstdb2revoke,
		sstdb2rollback,sstdb2savepoint,sstdb2setconnection,sstdb2setcurrentdefaulttransformgroup,sstdb2setcurrentdegree,
		sstdb2setcurrentexplainmode,sstdb2setcurrentexplainsnapshot,sstdb2setcurrentisolation,sstdb2setcurrentlocktimeout,sstdb2setcurrentmaintainedtabletypesforoptimization,
		sstdb2setcurrentpackagepath,sstdb2setcurrentpackageset,sstdb2setcurrentqueryoptimization,sstdb2setcurrentrefreshage,sstdb2setencryptionpassword,
		sstdb2seteventmonitorstate,sstdb2setintegrity,sstdb2setpassthru,sstdb2setpath,sstdb2setschema,
		sstdb2setserveroption,sstdb2setsessionauthorization,sstdb2set,sstdb2terminate,sstdb2signal,
		sstdb2values,sstdb2whenever,sstdb2while,sstdb2sqlvariabledeclaration,sstdb2conditiondeclaration,
		sstdb2returncodesdeclaration,sstdb2statementdeclaration,sstdb2declarecursorstatement,sstdb2handlerdeclaration,sstdb2sqlprocedurestatement,
		sstdb2callstmt,sstdb2forstmt,sstdb2ifstmt,sstdb2iteratestmt,sstdb2leavestmt,
		sstdb2signalstatement,sstdb2whilestmt,sstdb2repeatstmt,sstdb2closecursorstmt,sstdb2opencursorstmt,
		sstdb2fetchcursorstmt,sstdb2gotostmt,sstdb2loopstmt,sstdb2casestmt,sstdb2procedurecompoundstatement,
		sstdb2dynamiccompoundstatement,sstdb2returnstmt,sstdb2dummystmt,sstdb2valuesinto,
        sstdb2stmtstub,sstdb2declare,
        sstplsql_assignstmt,
		sstplsql_casestmt,sstplsql_closestmt,sstplsql_cursordecl,sstplsql_dummystmt,sstplsql_elsifstmt,
		sstplsql_execimmestmt,sstplsql_exitstmt,sstplsql_fetchstmt,sstplsql_forallstmt,sstplsql_gotostmt,
		sstplsql_ifstmt,sstplsql_loopstmt,sstplsql_nullstmt,sstplsql_openforstmt,sstplsql_openstmt,
		sstplsql_pragmadecl,sstplsql_procbasicstmt,sstplsql_procedurespec,sstplsql_raisestmt,sstplsql_recordtypedef,
		sstplsql_returnstmt,sstplsql_sqlstmt,sstplsql_proceduredecl,sstplsql_vardecl,
		sstplsql_piperowstmt, 
        sstsybaselocktable,
        sstmdxunknown,sstmdxselect,sstmdxupdate,sstmdxalterdimension,
		sstmdxcall,sstmdxclearcalculations,sstmdxdrillthrough,sstmdxaltercube,sstmdxcreateaction,
		sstmdxcreatecellcalculation,sstmdxcreatedimensionmember,sstmdxcreateglobalcube,sstmdxcreatemember,sstmdxcreatesessioncube,
		sstmdxcreateset,sstmdxcreatesubcube,sstmdxdropaction,sstmdxdropcellcalculation,sstmdxdropdimensionmember,
		sstmdxdropmember,sstmdxdropset,sstmdxdropsubcube,sstmdxrefreshcube,sstmdxcalculate,
		sstmdxcase,sstmdxexisting,sstmdxfreeze,sstmdxif, sstmdxscope,sstmdxexpression,
        sstteradataabort,sstteradataalterfunction,sstteradataaltermethod,sstteradataalterprocedure,
        sstteradataalterreplicationgroup,sstteradataaltertable,sstteradataaltertrigger,
		sstteradataaltertype,sstteradatabegindeclaresection,
        sstteradatabeginlogging,sstteradatabeginquerylogging,sstteradatabegintransaction,
        sstteradatacall,
        sstteradatacheckpoint,sstteradataclose,
		sstteradatacollectdemographics,sstteradatacollectstatistics,
		sstteradatacomment,sstteradatacommit,sstteradataconnect,
		sstteradatacreatecast,
		sstteradatacreateauthorization,
		sstteradatacreatedatabase,sstteradatacreatefunction,sstteradatacreateindex,
		sstteradatacreatemacro, sstteradatacreatemethod,sstteradatacreateordering,
		sstteradatacreateprocedure,sstteradatacreateprofile,
		sstteradatacreatereplicationgroup,sstteradatacreaterole,sstteradatacreatetransform,
		sstteradatacreatetrigger,sstteradatacreatetype,
		sstteradatacreateuser,sstteradatadatabase,sstteradatadeclarecursor,sstteradatadeclarestatement,
		sstteradatadeclaretable,sstteradatadeletedatabase,sstteradatadeleteuser,sstteradatadescribe,
		sstteradatadiagnostic,sstteradatadropauthorization,sstteradatadropdatabase,
		sstteradatadropdbobject,
		sstteradatadropfunction,
		sstteradatadropuser,
		sstteradatadropcast,sstteradatadropmacro,sstteradatadropordering,sstteradatadropprocedure,
		sstteradatadropprofile,sstteradatadropreplicationgroup,sstteradatadroprole,
		sstteradatadropstatistics,sstteradatadroptransform,sstteradatadroptrigger,sstteradatadroptype,
		sstteradatadumpexplain,sstteradataecho,sstteradataenddeclaresection,sstteradataendlogging,
		sstteradataendquerylogging,sstteradataendtransaction,sstteradataexecute,sstteradataexecuteimmediate,
		sstteradatafetch, 
		sstteradatagetcrash,sstteradatagive,sstteradatagrant,sstteradatagrantlogon,sstteradatagrantmonitor,
		sstteradatahelp,sstteradatainclude,sstteradataincludesqlca,sstteradatasqlda,sstteradatainitiateindexanalysis,
		sstteradatainsertexplain,sstteradatalogoff,sstteradatalogon,sstteradatamodifydatabase,
		sstteradatamodifyprofile,sstteradatamodifyuser,sstteradataopen,sstteradataposition,
		sstteradataprepare,
		sstteradatarenamefunction,sstteradatarenamemacro,
		sstteradatarenameprocedure,sstteradatarenametable,sstteradatarenametrigger,sstteradatarenameview,
		sstteradatareplacecast,
		sstteradatareplacefunction,sstteradatareplacemacro,sstteradatareplacemethod,
		sstteradatareplaceordering,
		sstteradatareplaceprocedure,sstteradatareplacetransform,sstteradatareplacetrigger,sstteradatareplaceview,
		sstteradatarestartindexanalysis,sstteradatarevoke,sstteradatarevokelogon,sstteradatarevokemonitor,
		sstteradatarevokerole,sstteradatarewind,sstteradatarollback,sstteradatasetbuffersize,
		sstteradatasetcharset,sstteradatasetconnection,sstteradatasetcrash,sstteradatasetrole,
		sstteradatasetsessionaccount,
		sstteradatasetsession,sstteradatasettimezone,sstteradatashow,sstteradatashowfunction,
		sstteradatashowindex,sstteradatashowmacro,sstteradatashowmethod,sstteradatashowprocedure,
		sstteradatashowreplicationgroup,sstteradatashowtable,sstteradatashowtrigger,
		sstteradatashowtype,sstteradatashowview,
		sstteradatatest,sstteradatawait,sstteradatawhenever,sstteradataasync,sstteradataexplain,
		sstteradatausing,sstteradatanotimplement,
		sstblockstmt , sststoredprocedurestmt,sstoraclestoredprocedurestmt,
		sstpostgresqlabort,sstpostgresqlAlterAggregate,sstpostgresqlAlterCollation,
		sstpostgresqlAlterConversion,sstpostgresqlalterdatabase,
		sstpostgresqlAlterDefaultPrivileges,sstpostgresqlAlterDomain , sstpostgresqlAlterExtension,
		sstpostgresqlAlterForeignDataWrapper,sstpostgresqlAlterForeignTable,
		sstpostgresqlAlterfunction, sstpostgresqlAlterGroup,sstpostgresqlAlterIndex,
		sstpostgresqlAlterLanguage, sstpostgresqlAlterLargeObject,sstpostgresqlAlterOperator,
		sstpostgresqlAlterOperatorClass,sstpostgresqlAlterOperatorFamily,sstpostgresqlAlterRole,
		sstpostgresqlAlterSchema,sstpostgresqlAlterSequence,sstpostgresqlAlterServer,
		sstpostgresqlAlterTablespace, sstpostgresqlAlterTextSearchConfiguration,
		sstpostgresqlAlterTextSearchDictionary,sstpostgresqlAlterTextSearchParser,
		sstpostgresqlAlterTextSearchTemplate,sstpostgresqlAlterTrigger,
		sstpostgresqlAlterType,sstpostgresqlAlterUser,sstpostgresqlAlterUserMapping,
		sstpostgresqlAlterView,sstpostgresqlAnalyze,sstpostgresqlBegin,sstpostgresqlCheckpoint,
		sstpostgresqlClose,sstpostgresqlCluster,sstpostgresqlComment,sstpostgresqlCommit,
		sstpostgresqlCommitPrepared,sstpostgresqlCopy,
		sstpostgresqlCreateAggregate,sstpostgresqlCreateCast,sstpostgresqlCreateCollation,
		sstpostgresqlConversion,sstpostgresqlCreateDatabase,sstpostgresqlCreateDomain,
		sstpostgresqlCreateExtension,sstpostgresqlCreateForeignDataWrapper,sstpostgresqlCreateForeignTable,
		sstpostgresqlCeateFunction,sstpostgresqlCeateGroup,sstpostgresqlCreateIndex,sstpostgresqlCreateLanguage,
		sstpostgresqlCreateOperator,sstpostgresqlCreateOperatorFimaly,sstpostgresqlCreateOperatorClass,
		sstpostgresqlCreateRole,sstpostgresqlCreateRule,sstpostgresqlCreateSchema,
		sstpostgresqlCreateSequence,sstpostgresqlCreateServer,sstpostgresqlCreateTablespace,
		sstpostgresqlCreateTextSearchConfiguration,sstpostgresqlCreateTextSearchDictionary,
		sstpostgresqlCreateTextSearchParser,sstpostgresqlCreateTextSearchTemplate,
		sstpostgresqlCreateTrigger,sstpostgresqlCreateType,sstpostgresqlCreateUser,
		sstpostgresqlCreateUserMapping,sstpostgresqlCreateView,sstpostgresqlDeallocate,
		sstpostgresqlDeclare,sstpostgresqlDiscard,sstpostgresqlDo,sstpostgresqlDropAggregate,
		sstpostgresqlDropCast,sstpostgresqlDropCollation,sstpostgresqlDropConversion,
		sstpostgresqlDropDatabase,sstpostgresqlDropDomain,sstpostgresqlDropExtension,
		sstpostgresqlDropForeignDataWrapper,sstpostgresqlDropForeignTable,sstpostgresqlDropFunction,
		sstpostgresqlDropGroup,sstpostgresqlDropLanguage,sstpostgresqlDropOperator,
		sstpostgresqlDropOperatorClass,sstpostgresqlDropOperatorFamily,sstpostgresqlDropOwned,
		sstpostgresqlDropRole,sstpostgresqlDropRule,sstpostgresqlDropSchema,sstpostgresqlDropSequence,
		sstpostgresqlDropServer,sstpostgresqlDropTable,sstpostgresqlDropTablespace,sstpostgresqlDropTextSearchConfiguration,
		sstpostgresqlDropTextSearchDictionary,sstpostgresqlDropTextSearchParser,sstpostgresqlDropTextSearchTemplate,
		sstpostgresqlDropTrigger,sstpostgresqlDropType,sstpostgresqlDropUser,sstpostgresqlDropUserMapping,
		sstpostgresqlDropView,sstpostgresqlEnd,sstpostgresqlExecute,sstpostgresqlExplain,sstpostgresqlFetch,
		sstpostgresqlGrant,sstpostgresqlListen,sstpostgresqlLoad,sstpostgresqlLock,sstpostgresqlMove,
		sstpostgresqlNotify,sstpostgresqlPrepare,sstpostgresqlPrepareTransaction,sstpostgresqlReassignOwned,
		sstpostgresqlReindex,sstpostgresqlReleaseSavepoint,sstpostgresqlReset,sstpostgresqlRevoke,
		sstpostgresqlRollback,sstpostgresqlRollbackPrepared,sstpostgresqlSavepoint,sstpostgresqlSecurityLabel,
		sstpostgresqlSet,sstpostgresqlSetConstraints,sstpostgresqlSetRole,sstpostgresqlSetSessionAuthorization,
		sstpostgresqlSetTransaction,sstpostgresqlSetSchema,
		sstpostgresqlTruncate,sstpostgresqlUnlisten,sstpostgresqlValues,
		sstnetezzaAlterDatabase,sstnetezzaAlterGroup,sstnetezzaAlterHistoryConfiguration,
		sstnetezzaAlterSequence,sstnetezzaAlterSynonym,
		sstnetezzaAlterTable,sstnetezzaAlterUser,sstnetezzaAlterView,
		sstnetezzaBegin,sstnetezzaComment,sstnetezzaCommit,sstnetezzaCopy,
		sstnetezzaCreateExternalTable,sstnetezzaCreateGruop,
		sstnetezzaCreateHistoryConfiguration,
		sstnetezzaCreateUser,
		sstnetezzaDropConnection,sstnetezzaDropDatabase,
		sstnetezzaDropGroup,sstnetezzaDropHistoryConfiguration,sstnetezzaDropSequence,
		sstnetezzaDropSession,sstnetezzaDropSynonym,sstnetezzaDropTable,sstnetezzaDropUser,
		sstnetezzaDropView,sstnetezzaExplain,sstnetezzaGenerateExpressStatistics,
		sstnetezzaGenerateStatistics,sstnetezzaGrant,sstnetezzaGroomTable,sstnetezzaReset,
		sstnetezzaRevoke,sstnetezzaRollback,sstnetezzaSet,
		sstnetezzaShow,
    sstinformixAllocateCollection,
    sstinformixAllocateDescriptor,
    sstinformixAlterAccess_Method,
    sstinformixAlterFragment, 
    sstinformixAlterFunction, 
    sstinformixAlterIndex, 
    sstinformixAlterProcedure, 
    sstinformixAlterRoutine, 
    sstinformixAlterSecurityLabelComponent, 
    sstinformixAlterSequence, 
    sstinformixAlterTable, 
    sstinformixAlterTrustedContext, 
    sstinformixAlterUser, 
    sstinformixBegin, 
    sstinformixClose, 
    sstinformixCloseDatabase, 
    sstinformixCommit, 
    sstinformixConnect, 
    sstinformixCreateAccess_Method, 
    sstinformixCreateAggregate, 
    sstinformixCreateCast, 
    sstinformixCreateIndex,
    sstinformixCreateDatabase, 
    sstinformixCreateFunction, 
    
    sstinformixCreateDefaultUser,
    sstinformixCreateDistinctType, 
    sstinformixCreateExternalTable,
    
    sstinformixCreateFunctionFrom,
    
    sstinformixCreateOpaqueType, 
    sstinformixCreateOpclass, 
    
    sstinformixCreateProcedure, 
    sstinformixCreateProcedureFrom, 
	sstinformixAlterTrigger,
    
    sstinformixCreateRole, 
    sstinformixCreateRoutineFrom,
    sstinformixCreateRowType, 
    sstinformixCreateSchema, 
    sstinformixCreateSecurityLabel,
    sstinformixCreateSecurityLabelComponent,
    sstinformixCreateSecurityPolicy, 
    sstinformixCreateSequence, 
    
    sstinformixCreateSynonym, 
    sstinformixCreateTempTable, 
    sstinformixCreateTrigger, 
    sstinformixCreateTrustedContext, 
    sstinformixCreateUser, 
    sstinformixCreateView, 
    sstinformixCreateXaDatasource, 
    sstinformixCreateXaDatasourceType, 
    sstinformixDatabase, 
    sstinformixDeallocateCollection,
    sstinformixDeallocateDescriptor,
    sstinformixDeallocateRow, 
    sstinformixDeclare, 
    sstinformixDescribe,
    sstinformixDescribeInput, 
    sstinformixDisconnect, 
    sstinformixDropAccess_Method,
    sstinformixDropAggregate, 
    sstinformixDropCast, 
    sstinformixDropDatabase, 
    sstinformixDropFunction, 
    sstinformixDropIndex, 
    sstinformixDropOpclass,
    sstinformixDropProcedure,
    sstinformixDropRole,
    sstinformixDropRoutine, 
    sstinformixDropRowType, 
    sstinformixDropSecurity,
    sstinformixDropSequence,
    sstinformixDropSynonym, 
    sstinformixDropTable, 
    sstinformixDropTrigger,
    sstinformixDropTrustedContext,
    sstinformixDropType, 
    sstinformixDropUser, 
    sstinformixDropView, 
    sstinformixDropXaDatasource,
    sstinformixDropXaDatasourceType,
    sstinformixExecute, 
    
    sstinformixExecuteFunction, 
    sstinformixExecuteImmediate,
    sstinformixExecuteProcedure,
    sstinformixFetch, 
    sstinformixFlush, 
    sstinformixFree, 
    sstinformixGetDescriptor, 
    sstinformixGetDiagnostics,
    sstinformixGrant,
    sstinformixGrantFragment, 
    sstinformixInfo, 
    
    sstinformixLoad, 
    sstinformixLockTable, 
    sstinformixOpen,
    sstinformixOutput,
    sstinformixPrepare, 
    sstinformixPut, 
    sstinformixReleaseSavepoint, 
    sstinformixRenameColumn, 
    sstinformixRenameDatabase,
    sstinformixRenameIndex, 
    sstinformixRenameSecurity,
    sstinformixRenameSequence,
    sstinformixRenameTable, 
    sstinformixRenameTrustedContext,
    sstinformixRenameUser,
    sstinformixRevoke,
    sstinformixRevokeFragment, 
    sstinformixRollbackWork, 
    sstinformixSaveExternalDirectives, 
    sstinformixSavepoint, 
    sstinformixSetAutofree,
    sstinformixSetCollation, 
    sstinformixSetConnection,
    sstinformixSetConstraints,
    sstinformixSetDatabaseObject, 
    sstinformixSetDataskip,
    sstinformixSetDebugFile,
    sstinformixSetDeferred_Prepare, 
    sstinformixSetDescriptor, 
    sstinformixSetEncryptionPassword, 
    sstinformixSetEnvironment, 
    sstinformixSetExplain, 
    sstinformixSetIndexes, 
    sstinformixSetIsolation,
    sstinformixSetLockMode, 
    sstinformixSetLog, 
    sstinformixSetOptimization,
    sstinformixSetPDQPriority, 
    sstinformixSetRole, 
    sstinformixSetSessionAuthorization, 
    sstinformixSetStatementCache, 
    sstinformixSetTransaction, 
    sstinformixSetTransactionMode,
    sstinformixSetTriggers, 
    sstinformixSetUserPassword,
    sstinformixStartViolationsTable,
    sstinformixStopViolationsTable, 
    
    sstinformixUnload,
    sstinformixUnlockTable, 
    
    sstinformixUpdateStatistics, 
    sstinformixWhenever

	

    }EStmtType;


typedef enum ETokenStatus {
    ets_original,
    ets_deleted,
	ets_overwrited,
    ets_ignorebyyacc,
    ets_synataxerror,
    ets_ignoredbygetrawstatement,
    //ets_synataxerrorprocessed,
    //ets_addbyhand,
    //ets_addedintokensinscript,
    //ets_markdeletedinppdowhitespace,
    //ets_addedinpp,
    //ets_deletedinpp,
    //ets_alreadlyaddedtolist,
	//ets__not_stmt_terminator,
    ets_dummy_status
}ETokenStatus;

typedef enum EJoinSource{
	ejs_fake,ejs_table,ejs_join
}EJoinSource;

typedef enum  EJoinType {
    ejt_cross,
    ejt_natural,
    ejt_full,
    ejt_left,
    ejt_right,
    ejt_fullouter,
    ejt_leftouter,
    ejt_rightouter,
    ejt_inner,
    ejt_crossapply,
    ejt_outerapply,
    ejt_straight,
    ejt_union,
    ejt_nested,
    ejt_natural_full,//postgresql
    ejt_natural_fullouter,//postgresql
    ejt_natural_inner,//postgresql
    ejt_natural_left, //mysql postgresql
    ejt_natural_right,//mysql postgresql
    ejt_natural_leftouter,//mysql postgresql
    ejt_natural_rightouter//mysql postgresql
}EJoinType;

typedef enum EFireMode{
	efm_before,efm_after,efm_insteadOf,efm_for
}EFireMode;

typedef enum ETriggerMode{
	etm_for,etm_after,etm_insteadOf
}ETriggerMode;

typedef enum EStoreProcedureMode{
	epm_create,epm_declare,epm_define,epm_create_body,
	epm_create_incomplete,epm_create_varray,epm_create_nested_table,
	epm_create_type_placeholder
}EStoreProcedureMode;

typedef enum EParameterMode{
	epa_default,epa_in,epa_out,epa_inout,epa_output,
	epa_readonly,epa_aslocator
}EParameterMode;

typedef	enum EHowtoSetValue{
	ehs_none,ehs_assign,ehs_default
}EHowtoSetValue;


typedef enum EWhatDeclared{
	ewd_variable,ewd_constant,ewd_exception,ewd_subtype,
	ewd_pragma_exception_init,ewd_pragma_autonomous_transaction,
	ewd_pragma_serially_reusable,ewd_pragma_timestamp,
	ewd_pragma_restrict_references,
	ewd_pragma_interface
}EWhatDeclared;

typedef	enum EInsertValue{
	eiv_values,
	eiv_query,
	eiv_values_function,
	eiv_values_empty,
	eiv_values_oracle_record,
	eiv_set_column_value,
	eiv_values_multi_table,
	eiv_default_values,
	eiv_execute
}EInsertValue;

typedef enum EIndexType {
    eit_Normal, eit_Unique, eit_BitMap,eit_Fulltext,eit_Spatial,eit_distinct
}EIndexType;

typedef enum EAggregateType{
	eat_none,eat_distinct,eat_all,eat_unique
}EAggregateType;

typedef enum EAlterTableOptionType {
    eat_Unknown,
    eat_AddColumn,
    eat_ModifyColumn,
    eat_AlterColumn,
    eat_DropColumn,
    eat_SetUnUsedColumn,
    eat_DropUnUsedColumn,
    eat_DropColumnsContinue,
    eat_RenameColumn,
    eat_ChangeColumn,
    eat_RenameTable,
    eat_AddConstraint,
    eat_AddConstraintIndex,
    eat_AddConstraintPK,
    eat_AddConstraintUnique,
    eat_AddConstraintFK,
    eat_ModifyConstraint,
    eat_RenameConstraint,
    eat_DropConstraint,
    eat_DropConstraintPK,
    eat_DropConstraintFK,
    eat_DropConstraintUnique,
    eat_DropConstraintCheck,
    eat_DropConstraintPartitioningKey,
    eat_DropConstraintRestrict,
    eat_DropConstraintIndex,
    eat_DropConstraintKey,
    eat_AlterConstraintFK,
    eat_AlterConstraintCheck,
    eat_CheckConstraint,
    eat_OraclePhysicalAttrs,
    eat_toOracleLogClause,
    eat_OracleTableP,
    eat_MssqlEnableTrigger,
    eat_MySQLTableOptons,
    eat_Db2PartitioningKeyDef,
    eat_Db2RestrictOnDrop,
    eat_Db2Misc,
	eat_ERShadowColumns,
	eat_ModifyExtentSize,
	eat_LockMode,
	eat_PutClause,
	eat_SecurityPolicy,
	eat_dropIndex
}EAlterTableOptionType;

typedef enum ETableSource {
    ets_unknown,
    ets_objectname,
    ets_subquery,
	//ets_table_subquery,
    ets_tableExpr,
    ets_join,
    ets_functionCall, //sql server
    ets_rowList, //sql server, (values MultiTargets)
    ets_containsTable, //sql server
    ets_freetextTable, //sql server
    ets_openrowset, //sql server
    ets_openxml, //sql server
    ets_opendatasource, //sql server
    ets_openquery, //sql server
    ets_datachangeTable, //db2
	ets_informixOuter
}ETableSource;

typedef enum EConstraintType {
    ect_notnull,
    ect_unique,
    ect_primary_key,
    ect_foreign_key,
    ect_check,
    ect_reference,
    ect_default_value, // valid in sql server
    ect_index,//mysql
    ect_key, //mysql
    ect_exclude, //postgresql
	

    /**
     * it's a fake constraint, used only in yacc rule file, should be removed during parsing
     */
    ect_fake_null,
    ect_fake_collate,
    ect_fake_identity,
    ect_fake_rowguidcol,
    ect_fake_auto_increment, //mysql
    ect_fake_comment, //mysql
    ect_fake_db2, //db2
	ect_distinct

}EConstraintType;

typedef enum EKeyReferenceType {
    ekr_no_action,
    ekr_restrict,
    ekr_cascade,
    ekr_set_null,
    ekr_set_default
}EKeyReferenceType;

typedef enum ESetOperator{
	eso_none,
	eso_union,
	eso_unionall,
	eso_intersect,
	eso_intersectall,
	eso_minus,
	eso_minusall,
	eso_except,
	eso_exceptall
}ESetOperator;

typedef enum EDataType{
    edt_unknown,
    /**
     * user defined datetype
     */
    edt_generic,
    edt_bfile,
    /**
     * ansi2003: bigint
     */
    edt_bigint,
    /**
     * ansi2003: blob
     */
    edt_binary,
    edt_binary_float,
    edt_binary_double,
    /**
     * plsql binary_integer
     */
    edt_binary_integer,
    /**
     * binary large object
     * Databases: DB2, Teradata
     */
    edt_binary_large_object,
    edt_bit,
    edt_bit_varying, // = varbit
    edt_blob,
    /**
     * bool, boolean, ansi2003: boolean
     */
    edt_bool,
    edt_box,
    /**
     * teradata: byte
     */
    edt_byte,
    edt_bytea, //ansi2003 blob
    /**
     * Teradata byteint
     */
    edt_byteint,
    /**
     * char, character,  ansi2003: character
     */
    edt_char,
    edt_char_for_bit_data,
    /**
     * teradata: character large object
     */
    edt_char_large_object,
    edt_cidr,
    edt_circle,
    edt_clob,
    edt_cursor,
    edt_datalink,
    edt_date,
    /**
     *  ansi2003: timestamp
     */
    edt_datetime,
    edt_datetimeoffset,// ansi2003: timestamp
    edt_datetime2, //  ansi2003: timestamp with time zone
    /**
     * ansi2003: nclob
     * Databases: DB2
     */
    edt_dbclob,
    /**
     * dec,decimal, ansi2003: decimal
     */
    edt_dec,
    /**
     * double, double precision, ansi2003: float
     */
    edt_double,
    edt_enum,
	edt_small_float,
    edt_float,// ansi2003: double precision
    edt_float4,// ansi2003: float(p)
    edt_float8, // ansi2003 float(p)
    /**
     * ansi2003 blob
     */
    edt_graphic,
    edt_geography,
    edt_geometry,
    edt_hierarchyid,
    edt_image,
    edt_inet,
    /**
     * int, integer, ansi2003: integer
     */
    edt_int,
    edt_int2, // ansi2003: smallint
    edt_int4, // ansi2003: int, integer
    /**
     * Postgresql
     */
    edt_interval,
    /**
     * Teradata: interval day
     */
    edt_interval_day,
    /**
     * teradata: interval day to hour
     */
    edt_interval_day_to_hour,
    /**
     * teradata: interval day to minute
     */
    edt_interval_day_to_minute,
    edt_interval_day_to_second,
    /**
     * teradata: interval hour
     */
    edt_interval_hour,
    /**
     * teradata: interval hour to minute
     */
    edt_interval_hour_to_minute,
    /**
     * teradata: interval hour to second
     */
    edt_interval_hour_to_second,
    /**
     * teradata: interval minute
     */
    edt_interval_minute,
    /**
    * teradata: interval minute to second
    */
    edt_interval_minute_to_second,
    /**
    * teradata: interval month
    */
    edt_interval_month,
    /**
    * teradata:interval second
    */
    edt_interval_second,
    /**
    * Teradata interval year.
    */
    edt_interval_year,
    edt_interval_year_to_month,
    edt_line,
    edt_long,
    edt_long_varchar,
    /**
    * long varbinary, mysql
    * MySQL Connector/ODBC defines BLOB values as LONGVARBINARY and TEXT values as LONGVARCHAR.
    */
    edt_long_varbinary,
    edt_longblob, // ansi2003: blob
    /**
    *  ansi2003: blob
    */
    edt_long_raw,
    edt_long_vargraphic,
    edt_longtext,
    edt_lseg,
    edt_macaddr,
    edt_mediumblob,
    /**
    * mediumint, middleint(MySQL) , ansi2003:  int
    */
    edt_mediumint,
    edt_mediumtext,
    edt_money, // = decimal(9,2)
    /**
    * national_char_varying,nchar_varying,nvarchar, ansi2003: national character varying
    */
    edt_nvarchar,
    /**
    * nchar, national char, national character,ansi2003: national character
    */
    edt_nchar,
    /**
    * ansi2003: nclob
    */
    edt_nclob,
    /**
    * ntext, national text, ansi2003: nclob
    */
    edt_ntext,
    /**
    * nvarchar2(n)
    */
    edt_nvarchar2,
    /**
    * number, num
    */
    edt_number,
    /**
    *  ansi2003: numeric
    */
    edt_numeric,
    edt_oid,
    edt_path,
    /**
    * teradata: period(n)
    */
    edt_period,
    /**
    * plsql pls_integer
    */
    edt_pls_integer,
    edt_point,
    edt_polygon,
    edt_raw,
    /**
    * ansi2003: real
    */
    edt_real,
    edt_rowid,
    edt_rowversion,
	edt_big_serial,
    edt_serial,// = serial4
    edt_serial8,// = bigserial
    /**
    * MySQL: set
    */
    edt_set,
    edt_smalldatetime,
    /**
    * ansi2003: smallint
    */
    edt_smallint,
    edt_smallmoney,
    edt_sql_variant,
    edt_table,
    edt_text,
    /**
    * ansi2003: time
    */
    edt_time,
    /**
    * teradata: time with time zone
    */
    edt_time_with_time_zone,
    edt_timespan, // ansi2003: interval
    edt_timestamp, // ansi2003: timestamp
    /**
    * timestamp with local time zone,
    * Database: Oracle,SQL Server
    */
    edt_timestamp_with_local_time_zone,
    /**
    * timestamp with time zone, timestamptz, ansi2003: timestamp with time zone
    */
    edt_timestamp_with_time_zone,
    /**
    * time with time zone,  ansi2003: time with time zone
    * Databases: Teradata
    */
    edt_timetz,
    edt_tinyblob,
    edt_tinyint,
    edt_tinytext,
    edt_uniqueidentifier,
    edt_urowid,
    /**
    *  ansi2003: blob
    */
    edt_varbinary,
    /**
    * netezza, bit varying
    */
    edt_varbit,
    /**
    * teradata: varbyte
    */
    edt_varbyte,
    /**
    * varchar, char varying, character varying, ansi2003:character varying(n)
    */
    edt_varchar,
    /**
    * ansi2003: character varying
    */
    edt_varchar2,
    edt_varchar_for_bit_data,// ansi2003:    bit varying
    /**
    *  ansi2003: nchar varying
    */
    edt_vargraphic,
	edt_lvarchar,
    /**
    * ansi2003: tinyint
    */
    /**
    * datatypeAttribute in cast function will be treated as a datatype without typename
    * RW_CAST ( expr AS datatypeAttribute )
    */
    edt_no_typename,
    edt_year,
    edt_xml, // ansi2003: xml
    edt_xmltype_t, // ansi2003: xml
	edt_idssecuritylabel,
	edt_row_data_types,
	edt_collection_data_types_collection,
	edt_collection_data_types_set,
	edt_collection_data_types_multiset,
	edt_collection_data_types_list,

}EDataType;

typedef enum EFunctionType{
    eft_unknown,
    eft_udf,
    eft_trim,
    eft_cast,
    eft_convert,
    eft_extract,
    eft_treat,
    eft_contains,
    eft_freetext, //
    eft_casen, //teradata
    eft_rangen, //teradata
    eft_position, /*!< teradata, postgresql */
    eft_translate, //teradata ,oracle
    eft_translate_chk, //teradata
    eft_csum, //teradata
    eft_rank, //teradata
    eft_xmlquery, //oracle
    eft_substring,//mysql
    eft_adddate,//mysql
    eft_date_add,//mysql
    eft_subdate,//mysql
    eft_date_sub,//mysql
    eft_timestampadd,//mysql
    eft_timestampdiff,//mysql
    eft_group_concat,//mysql
    eft_match_against,//mysql
    eft_extractxml,
    eft_ogc,
    eft_interval, //mysql
    eft_overlay,
    eft_case_n,
    eft_range_n,
	eft_extend,
	eft_nullif, /*!< postgresql */
	eft_coalesce, /*!< postgresql */
	eft_greatest, /*!< postgresql */
	eft_least, /*!< postgresql */
	eft_xmlconcat, /*!< postgresql */
	eft_period, /*!< Teradata */
	eft_xmlserialize, /*!< Oracle */
	eft_xmlelement, /*!< Oracle */
	eft_xmlroot, /*!< Oracle */
	 
}EFunctionType;

typedef enum EDBObjectType{
	edb_unknown,
	edb_not_object,
	edb_column,
	edb_column_cte,
	edb_column_alias,
	edb_table,
	edb_table_alias,
	edb_table_cte,
	edb_table_temp,
	edb_table_pivot,
	edb_table_var,
	edb_parameter,
	edb_variable,
	edb_column_method,
	edb_procedure,
	edb_function,
	edb_function_builtin,
	edb_label,
	edb_index,
	edb_materialized_view,
	edb_cursor,
	edb_view,
	edb_constraint,
	edb_property,
	edb_transaction,
	edb_database,
	edb_string_constant,
	edb_trigger,
	edb_alias,
	edb_attribute,
	edb_typename,
	edb_package,
	edb_sequence,
	edb_datatype,
	edb_schema,
	edb_server,
	edb_synonym,
	edb_operator,
	edb_index_type,
	edb_mining_model,
	edb_field,
	edb_positional_parameters,
	edb_oracle_hint,
	// Oracle: connect_by_iscycle, connect_by_isleaf,
	// level, rowid, rownum,currval,nextval
	// xmldata,
	edb_pseudo_column, 
	edb_mixed
}EDBObjectType;

/*
** symbol push into symbol table while analyzing statement
** mainly used to check table and column mapping.
**
** types of symbol: table, parameter, variable
*/
struct gsp_symbol{
	gsp_objectname *name;
	EDBObjectType	objectType;
};

/*!
 * source token of input sql, check gsp_sourcetoken.c for related functions.
 */
typedef struct gsp_sourcetoken{
	//int tokenCode;
	char	*pStr;		/*!< memory where this token start  */
	int		nStrLen;	/*!< length in byte of this token  */
	int		nCode;		/*!< unique identifier number of this token */
	int		nLine;		/*!< line number  */
	int		nColumn;	/*!< column number  */
	int		nOffSet;	/*!< bytes offset, start from  *sqltext of struct gsp_sqlparser */
	ETokenStatus tokenStatus;
	char	*pNewText; /*!< this text is valid only this token was set a new value by gsp_node_set_text() */
	int		bSqlplusCmd;	/*!< is this sqlplus command */
	char	*asText;
	struct gsp_sourcetoken *pNext;
	struct gsp_sourcetoken *pPrev;
	int nPosInList;	/*!< position in source token list created by lexer, this position will not change during lifetime of source token list*/
	EDBObjectType dbObjType;
}gsp_sourcetoken;

/*!
* type of parse tree node
*/
typedef enum ENodeType
{
	t_gsp_invalid = 0,
	t_gsp_dummy,
	t_gsp_join,
	t_gsp_joinItem,
	t_gsp_sql_statement,
	t_gsp_selectStatement,
	t_gsp_insertStatement,
	t_gsp_deleteStatement,
	t_gsp_updateStatement,
	t_gsp_createTableStatement,
	t_gsp_createIndexStatement,
	t_gsp_createViewStatement,
	t_gsp_unknownStatement,
	t_gsp_dropTableStatement,
	t_gsp_dropIndexStatement,
	t_gsp_dropViewStatement,
	t_gsp_dropDatabaseStatement,
	t_gsp_alterDatabaseStatement,
	t_gsp_createDatabaseStatement,
	t_gsp_createSequenceStatement,
	t_gsp_createSynonymStatement,
	t_gsp_createDirectoryStatement,
	t_gsp_alterTableStatement,
	t_gsp_mergeStatement,
	t_gsp_createPackageStatement,
	t_gsp_createProcedureStatement,
	t_gsp_createFunctionStatement,
	t_gsp_blockStatement,
	t_gsp_createTriggerStatement,
	t_gsp_trigger_event,
	t_gsp_arrayAccess,
	t_gsp_valueClause,
	t_gsp_constant,
	t_gsp_list,
	t_gsp_listcell,
	t_gsp_objectAccess,
	t_gsp_objectname ,
	t_gsp_expr,
	t_gsp_resultColumn,
	t_gsp_aliasClause,
	t_gsp_functionCall,
	t_gsp_keepDenseRankClause,
	t_gsp_analyticFunction,
	t_gsp_whenClauseItem,
	t_gsp_caseExpression,
	t_gsp_intervalExpression,
	t_gsp_trimArgument,
	t_gsp_typename,
	t_gsp_precisionScale,
	t_gsp_keyReference,
	t_gsp_keyAction,
	t_gsp_constraint,
	t_gsp_mergeInsertClause,
	t_gsp_mergeUpdateClause,
	t_gsp_mergeDeleteClause,
	t_gsp_mergeWhenClause,
	t_gsp_fromTable,
	t_gsp_multiTarget,
	t_gsp_insertRest,
	t_gsp_insertValuesClause,
	t_gsp_mergeSqlNode,
	t_gsp_alterTableOption,
	t_gsp_alterTableSqlNode,
	t_gsp_createSequenceSqlNode,
	t_gsp_createSynonymSqlNode,
	t_gsp_createDirectorySqlNode,
	t_gsp_dropViewSqlNode,
	t_gsp_dropIndexSqlNode,
	t_gsp_dropTableSqlNode,
	t_gsp_dropDatabaseSqlNode,
	t_gsp_alterDatabaseSqlNode,
	t_gsp_viewAliasItem,
	t_gsp_viewAliasClause,
	t_gsp_createViewSqlNode,
	t_gsp_createMaterializedViewSqlNode,
	t_gsp_createMaterializedViewLogSqlNode,
	t_gsp_orderByItem,
	t_gsp_createIndexSqlNode,
	t_gsp_columnDefinition,
	t_gsp_tableElement,
	t_gsp_table,
	t_gsp_createTableSqlNode,
	t_gsp_returningClause,
	t_gsp_selectSqlNode,
	t_gsp_deleteSqlNode,
	t_gsp_updateSqlNode,
	t_gsp_insertIntoValue,
	t_gsp_insertCondition,
	t_gsp_insertSqlNode,
	t_gsp_whereClause,
	t_gsp_joinExpr,
	t_gsp_tableSamplePart,
	t_gsp_tableSample,
	t_gsp_pxGranule,
	t_gsp_flashback,
	t_gsp_forUpdate,
	t_gsp_groupingSetItem,
	t_gsp_groupingSet,
	t_gsp_rollupCube,
	t_gsp_gruopByItem,
	t_gsp_groupBy,
	t_gsp_orderBy,
	t_gsp_selectDistinct,
	t_gsp_hierarchical,
	t_gsp_intoClause,
	t_gsp_cte,
	t_gsp_commentSqlNode,
	t_gsp_plsqlCreateTypeBody,
	t_gsp_callSpec,
	t_gsp_typeAttribute,
	t_gsp_plsqlCreateType,
	t_gsp_dmlEventClause,
	t_gsp_nonDmlTriggerClause,
	t_gsp_compoundDmlTriggerClause,
	t_gsp_simpleDmlTriggerClause,
	t_gsp_createTriggerSqlNode,
	t_gsp_createPackageSqlNode,
	t_gsp_plsqlVarDeclStmt,
	t_gsp_createFunctionSqlNode,
	t_gsp_parameterDeclaration,
	t_gsp_createProcedureSqlNode,
	t_gsp_plsqlReturnStmt,
	t_gsp_plsqlRaiseStmt,
	t_gsp_plsqlLoopStmt,
	t_gsp_plsqlCaseStmt,
	t_gsp_plsqlForallStmt,
	t_gsp_plsqlElsifStmt,
	t_gsp_plsqlIfStmt,
	t_gsp_plsqlGotoStmt,
	t_gsp_plsqlExitStmt,
	t_gsp_plsqlAssignStmt,
	t_gsp_plsqlCursorDeclStmt,
	t_gsp_plsqlRecordTypeDefStmt,
	t_gsp_plsqlVarrayTypeDefStmt,
	t_gsp_plsqlTableTypeDefStmt,
	t_gsp_plsqlNullStmt,
	t_gsp_plsqlFetchStmt,
	t_gsp_plsqlPipeRowStmt,
	t_gsp_plsqlOpenforStmt,
	t_gsp_plsqlOpenStmt,
	t_gsp_plsqlCloseStmt,
	t_gsp_plsqlBasicStmt,
	t_gsp_blockSqlNode,
	t_gsp_bindArgument,
	t_gsp_execImmeNode,
	t_gsp_exceptionHandler,
	t_gsp_exceptionClause,
	t_gsp_fetchFirstClause,
	t_gsp_optimizeForClause,
	t_gsp_isolationClause,
	t_gsp_valueRowItem,
	t_gsp_dataChangeTable,
	t_gsp_includeColumns,
	t_gsp_db2_signal,
	t_gsp_db2_compoundSqlNode,
	t_gsp_db2_triggerAction,
	t_gsp_db2_callStmtSqlNode,
	t_gsp_db2_forSqlNode,
	t_gsp_db2_ifSqlNode,
	t_gsp_db2_elseIfSqlNode,
	t_gsp_db2_iterateStmtSqlNode,
	t_gsp_db2_leaveStmtSqlNode,
	t_gsp_db2_setSqlNode,
	t_gsp_db2_whileSqlNode,
	t_gsp_db2_repeatSqlNode,
	t_gsp_db2_gotoSqlNode,
	t_gsp_db2_loopSqlNode,
	t_gsp_returnSqlNode,
	t_gsp_continueSqlNode,
	t_gsp_breakSqlNode,
	t_gsp_grantSqlNode,
	t_gsp_fetchSqlNode,
	t_gsp_openSqlNode,
	t_gsp_closeSqlNode,
	t_gsp_mssql_executeAsSqlNode,
	t_gsp_mssql_executeSqlNode,
	t_gsp_mssql_execParameter,
	t_gsp_mssql_dropDbObjectSqlNode,
	t_gsp_dropIndexItem,
	t_gsp_truncateTableSqlNode,
	t_gsp_mssql_setSqlNode,
	t_gsp_mssql_beginTranSqlNode,
	t_gsp_mssql_raiserrorSqlNode,
	t_gsp_mssql_gotoSqlNode,
	t_gsp_mssql_labelSqlNode,
	t_gsp_mssql_killSqlNode,
	t_gsp_mssql_deallocateSqlNode,
	t_gsp_declareSqlNode,
	t_gsp_declareVariable,
	t_gsp_mssql_beginDialogSqlNode,
	t_gsp_mssql_sendOnConversationSqlNode,
	t_gsp_mssql_endConversationSqlNode,
	t_gsp_mssql_revertSqlNode,
	t_gsp_mssql_goSqlNode,
	t_gsp_mssql_useSqlNode,
	t_gsp_mssql_printSqlNode,
	t_gsp_ifSqlNode,
	t_gsp_elseIfSqlNode,
	t_gsp_createTriggerUpdateColumn,
	t_gsp_whileSqlNode,
	t_gsp_mssql_computeClause,
	t_gsp_mssql_computeClauseItem,
	t_gsp_mssql_computeExpr,
	t_gsp_topClause,
	t_gsp_mssql_containsTable,
	t_gsp_mssql_tableHint,
	t_gsp_mssql_freeTable,
	t_gsp_mssql_openXML,
	t_gsp_mssql_openRowSet,
	t_gsp_pivotClause,
	t_gsp_unPivotClause,
	t_gsp_mssql_bulkInsertSqlNode,
	t_gsp_mssql_openQuery,
	t_gsp_mssql_openDatasource,
	t_gsp_mssql_outputClause,
	t_gsp_mssql_updateTextSqlNode,
	t_gsp_commitSqlNode,
	t_gsp_rollbackSqlNode,
	t_gsp_saveTransSqlNode,
	t_gsp_renameColumnSqlNode,
	t_gsp_renameSequenceSqlNode,
	t_gsp_renameTableSqlNode,
	t_gsp_renameIndexSqlNode,
	t_gsp_dropSequenceSqlNode,
	t_gsp_dropSynonymSqlNode,
	t_gsp_dropRowTypeSqlNode,
	t_gsp_alterIndexSqlNode,
	t_gsp_alterIndexStatement,
	t_gsp_intoTableClause,
	t_gsp_informixOuterClause,
	t_gsp_createRowTypeSqlNode,
	t_gsp_subscripts,
	t_gsp_limitClause,
	t_gsp_callSqlNode,
	t_gsp_createDatabaseSqlNode,
	t_gsp_iterateSqlNode,
	t_gsp_leaveSqlNode,
	t_gsp_repeatSqlNode,
	t_gsp_loopSqlNode,
	t_gsp_revokeSqlNode,
	t_gsp_executeSqlNode,
	t_gsp_dropRoleSqlNode,
	t_gsp_dropTriggerSqlNode,
	t_gsp_lockTableSqlNode,
	t_gsp_lockingClause,
	t_gsp_windowClause,
	t_gsp_windowDef,
	t_gsp_partitionClause,
	t_gsp_indices,
	t_gsp_alterSequenceSqlNode,
	t_gsp_alterViewSqlNode,
	t_gsp_collectStatisticsSqlNode, /*!< Teradata */
	t_gsp_teradataWithClause,
	t_gsp_qualifyClause,
	t_gsp_sampleClause,
	t_gsp_expandOnClause,
	t_gsp_datatypeAttribute,
	t_gsp_newVariantTypeArgument,
	t_gsp_outputFormatPhrase,
	t_gsp_useDatabaseSqlNode,
	t_gsp_setSchemaSqlNode, /*!< PostgreSQL set schema */
	t_gsp_mssql_ReconfigureSqlNode,
	t_gsp_mssql_ThrowSqlNode,
	t_gsp_mssql_insertBulkSqlNode,	
} ENodeType;

typedef enum EExpressionType{
    /**
     * expression type not set yet.
     */
    eet_not_initialized_yet,
    eet_unknown,
    eet_simple_source_token,
    eet_simple_object_name,
    eet_simple_constant,
    eet_arithmetic,
    eet_arithmetic_plus,
    eet_arithmetic_minus,
    eet_arithmetic_times,
    eet_arithmetic_divide,
    eet_arithmetic_modulo,
    eet_arithmetic_compound_operator,
    eet_parenthesis,
    eet_concatenate,
    eet_assignment,
    eet_list,
    eet_bitwise,
    eet_bitwise_and,
    eet_bitwise_or,
    eet_bitwise_xor,
    eet_bitwise_exclusive_or,
    eet_bitwise_shift_left,
    eet_bitwise_shift_right,
    eet_scope_resolution,
    eet_exponentiate,
    eet_simple_comparison,
    eet_group_comparison,
    eet_logical,
    eet_unary,
    eet_unary_plus,
    eet_unary_minus,
    eet_unary_prior,
    eet_unary_connect_by_root,
    eet_unary_factorial,
    eet_unary_squareroot,
    eet_unary_cuberoot,
    eet_unary_factorialprefix,
    eet_unary_absolutevalue,
    eet_unary_bitwise_not,
    eet_unary_left_unknown,
    eet_unary_right_unknown,
    eet_unary_binary_operator,
    eet_case,
    eet_function,
    eet_cursor,
    eet_subquery,
    eet_null,
    eet_between,
    eet_exists,
    eet_pattern_matching,//like
    eet_place_holder,
    eet_floating_point,
    eet_logical_and,
    eet_logical_or,
    eet_logical_xor,
    eet_logical_not,
    eet_is,
    eet_in,
    eet_group,// this expression including a TInExpr
    eet_is_of_type,
    eet_range,
    eet_power,
    eet_at_time_zone,
    eet_at_local,
    eet_day_to_second,
    eet_year_to_month,
    eet_new_structured_type, /*!< Teradata */
    eet_new_variant_type, /*!< Teradata */
    eet_period_ldiff,
    eet_period_rdiff,
    eet_period_p_intersect,
    eet_period_p_normalize,
    eet_until_changed,
    eet_is_document,
    eet_is_distinct_from,
    eet_is_unknown,
    eet_is_false,
    eet_is_true,
    eet_collate,
    eet_left_join,
    eet_right_join,
	eet_outer_join,
    eet_ref_arrow,
    eet_typecast,
    eet_arrayaccess,
    eet_sqlserver_proprietary_column_alias,
    eet_left_shift,
    eet_right_shift,
    eet_multiset,
    eet_fieldselection,
    eet_array_constructor, /*!< postgresql */
    eet_row_constructor,
    eet_member_of,
    eet_next_value_for,
    eet_datetime,
    eet_interval,
    eet_model,
    eet_object_access,
    eet_type_constructor,
    eet_xml_t,
	eet_units,
	eet_collection_constructor_set,
	eet_collection_constructor_multiset,
	eet_collection_constructor_list,
	eet_overlaps, /*!< postgresql, teradata overlaps */
}EExpressionType;

/*!
* every parse tree node includes a fragment struct
*/
typedef struct gsp_fragment
{
	gsp_sourcetoken *startToken; /*!< start token  */
	gsp_sourcetoken *endToken; /*!< end token  */
	char *text; /*!< text only valid when new text was set by calling gsp_node_set_text */
	char bEmpty; /*!< there is no token in this node, the first result column in values clause of this sql: INSERT INTO table_1 (column_1, column_2, column_2) VALUES (,222,222);*/
}gsp_fragment;

/*!
* base node of all parse tree nodes
*/
typedef struct Node
{
	ENodeType	nodeType; /*!< node type */
	struct Node *pNext,*pPrev;
	gsp_fragment fragment; /*!< node fragment */
}Node;

typedef struct	Node TParseTreeNode;
typedef struct	Node gsp_node;


/*!
* list to store parse tree nodes
*/
typedef struct gsp_list
{
	ENodeType	nodeType;			/* T_GSP_List */
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	int			length;
	struct gsp_listcell   *head;
	struct gsp_listcell   *tail;
} gsp_list;

typedef struct gsp_listcell
{
	ENodeType	nodeType;			/* T_GSP_List */
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	gsp_node *node;
	struct  gsp_listcell   *nextCell;
}gsp_listcell;





/*!
** databases supported by general sql parser
*/
typedef enum gsp_dbvendor {
	dbvmssql,	/*!< Microsoft SQL Server */
	dbvoracle,	/*!< Oralce */
	dbvmysql,	/*!< MySQL */
	dbvaccess,	/*!< Microsoft ACCESS */
	dbvgeneric,	
	dbvdb2,		/*!< IBM DB2 */
	dbvsybase,	/*!< sybase, not fully support*/
	dbvinformix, /*!< Informix */
	dbvpostgresql, /*!< PostgreSQL */
	dbvfirebird,
	dbvmdx,		/*!< MDX */
	dbvteradata,	/*!< Teradata */
	dbvnetezza		/*!< Netezza */
}gsp_dbvendor;

/*!
** string representation of gsp_dbvendor, such as gsp_dbvendor_name[dbvmssql] is mssql.
*/
extern const char *gsp_dbvendor_name[];

typedef struct gsp_plsqlLabel
{
	//ENodeType	nodeType;
	//gsp_fragment fragment;
	gsp_objectname *labelName;
	gsp_objectname *endlabelName;
}gsp_plsqlLabel;


/*!
** struct represents sql statement, check gsp_sqlstatement.c for related functions.
*/
typedef struct gsp_sql_statement{
	ENodeType	nodeType;
	gsp_node *pNext,*pPrev;
	gsp_fragment fragment;
	EStmtType stmtType;
	gsp_node *parseTree;	/*!< represents both raw parse tree and transformed sql statement */
	gsp_base_statement *stmt;	/*!< statement been transformed, 
								only available for the top level statement, 
								always points to the same memory as parseTree */
	int start_token_pos;	/* start token of this statement, position in source token list generated by lexer */
	int end_token_pos;	/* end token of this statement, position in source token list generated by lexer */
	struct gsp_sqlparser *sqlparser;
	int bCteQuery;
	gsp_plsqlLabel label;
	int isParsed;
	int dummyTag;
}gsp_sql_statement;


/*!
* sql parser
*/
typedef struct gsp_sqlparser {
	gsp_dbvendor db;
	char *sqltext;		/* input sql query, \0 terminated, copied from input sql file or constant sql string */
	int sqltext_len;	/* actual length of input sql query, don't include \0 */
	char *sqlfilename;	/* name of input sql file */
	char *sqltextfromfile;
	struct gsp_lexer *lexer;
	struct gsp_yyparser *yyparser,*plsqlyyparser;
	char *error_msg_out; /* memory will be allocated dynamically when gsp_errmsg() was called */
	char **error_msg;
	int error_msg_num;
	//int errmsg_slots;
	gsp_sourcetoken *sourcetokenlist;
	int number_of_token;
	gsp_sql_statement *pStatement;
	int nStatement; // number of statements
	int nAllocatedStatement; // preallocated statements
	char curdelimiterchar,delimiterchar;

#ifdef EXACTSOLUTION
	void **array_tblname; /*!< array of address of gsp_objectname */
	int array_len;
	int replaceconstants;
	int ntablefound;
#endif

	int (*xDispose)(struct gsp_sqlparser *);
}gsp_sqlparser;


/*!
** how table being accessed in query.
*/
typedef enum EAccessMode{
	eam_select,eam_delete,eam_insert,eam_update
}EAccessMode;

/*!
** location where database objects occurs in query
*/
typedef enum EQeuryClause{
	eqc_select_list,eqc_select_from,eqc_where,eqc_groupby,eqc_orderby
}EQeuryClause;

struct gsp_env{
	char *db;		/*! current used database, typically set by use command in sql server */
	char *schema;	/*! current schema that connect to a database in Oracle */ 
};


typedef struct gsp_builtin_function
{
	const char *name;		
} gsp_builtin_function;


struct gsp_visitor{
	void *context; // define necessary context to collect information while process nodes
	// array of function pointer to function that process related node
	void (*handle_node[])(gsp_node *node, struct gsp_visitor *visitor);
};

void process_expr(gsp_node *node, struct gsp_visitor *visitor);

/*!
**  list of gsp_objectname
*/
struct gsp_objectname_list {
  struct gsp_objectname_list_item {
    gsp_objectname *pName;      /* Name of the identifier */
  } *a;
  int nId;         /* Number of identifiers on the list */
  int nAlloc;      /* Number of entries allocated for a[] below */
};


#define nodeType(nodeptr)		(((gsp_node*)(nodeptr))->nodeType)

#define NodeSetStartToken(nodeptr,t)	(((gsp_node*)(nodeptr))->startToken = (t))
#define NodeSetEndToken(nodeptr,t)	(((gsp_node*)(nodeptr))->endToken = (t))

#define NodeSetFragmentStart(nodeptr,start)	\
	if (start != 0){ \
		(((gsp_node *)(nodeptr))->fragment).startToken = (start); \
	}
	

#define NodeSetFragmentEnd(nodeptr,end)	\
	if (end != 0){ \
		(((gsp_node *)(nodeptr))->fragment).endToken = (end); \
	}


#define NodeSetFragmentByNodeStart(nodeptr,srcNode)  \
	if ((gsp_node *)(srcNode) != 0){	\
		assert( ((gsp_node *)(srcNode))->nodeType != t_gsp_list); \
		(((gsp_node *)(nodeptr))->fragment).startToken = (srcNode->fragment).startToken; \
	}


#define NodeSetFragmentByNodeEnd(nodeptr,srcNode)  \
	if ((gsp_node *)(srcNode) != 0){	\
		assert( ((gsp_node *)(srcNode))->nodeType != t_gsp_list); \
		(((gsp_node *)(nodeptr))->fragment).endToken = (srcNode->fragment).endToken; \
	}


#define NodeSetFragmentByListStart(nodeptr,srcList)  \
	if ((gsp_list*)(srcList) != 0){	\
		assert( srcList->nodeType == t_gsp_list );	\
	 (((gsp_node*)(nodeptr))->fragment).startToken = (((gsp_node *)(gsp_list_first(srcList)))->fragment).startToken; \
	}

#define NodeSetFragmentByListEnd(nodeptr,srcList)  \
	if ((gsp_list*)(srcList) != 0){	\
		assert( srcList->nodeType == t_gsp_list );	\
	(((gsp_node*)(nodeptr))->fragment).endToken = (((gsp_node *)(gsp_list_last(srcList)))->fragment).endToken; \
	}
									

#define NodeSetType(nodeptr,t)	(((gsp_node*)(nodeptr))->nodeType = (t))

#define IsA(nodeptr,_type_)		(((gsp_node*)(nodeptr))->nodeType == t_##_type_)



typedef struct gsp_lexer{

	//The variable yytext contains the current match, yyleng its length.
	//The variable yyline contains the current input line, and yylineno and
	//yycolno denote the current input position (line, column). These values
	//are often used in giving error diagnostics (however, they will only be
	//meaningful if there is no rescanning across line ends).
	
	unsigned char	*yyinput; /* input buffer */
	unsigned char	*yyline;	//current input line
	int		yylineno,yycolno,yycolno_prev;	//current input position
	int		yylineno_p;
	int		yycolno_p;
	unsigned char	*yytext;	//matched text (should be considered r/o)
	int		yyleng;		//length of matched text		

	unsigned char *yyTokenText;
	int yyTokenLen;


	unsigned char	*linePtr;		//pointer to current active char in line
	int		lineLength;	//length of line processing now

	gsp_sourcetoken *pToken;
	int				nAllocatedToken;	// allocated token number
	int				nToken;		// number of token filled by lexer	
	

	int (*resetLexer)(struct gsp_lexer *,char *);
	int (*yylex)(struct gsp_lexer *);
	void (*clearLexer)(struct gsp_lexer *);
	void (*lexer_action)(struct gsp_lexer *, int );
	const struct ScanKeyword * (*keyword_lookup)(register const char *str, register unsigned int len);
	unsigned char* (*getTokenText)(struct gsp_lexer *);
	unsigned int (*getTokenLineNo)(struct gsp_lexer *);
	unsigned int (*getTokenColumnNo)(struct gsp_lexer *);
	unsigned int (*getTokenLineNo_p)(struct gsp_lexer *);
	unsigned int (*getTokenColumnNo_p)(struct gsp_lexer *);
	void (*setTokenLineNo_p)(struct gsp_lexer *,int);
	void (*setTokenColumnNo_p)(struct gsp_lexer *,int);
	//int (*getTokenCodeByText)(gsp_char *,gsp_dbvendor);
	
	gsp_dbvendor dbvendor;

	int insqlpluscmd;
	int nSqlplusStatus;

	const int *yyk;
	const int   *yym;
	const struct YYTRec   *yyt;
	const int   *yykl;
	const int   *yykh;
	const int   *yyml;
	const int   *yymh;
	const int   *yytl;
	const int   *yyth;

	ScanKeyword *ScanKeywords,*ScanSQLPlusCmds;
	int NumScanKeywords,NumScanSQLPlusCmds;

	/*
	** memory used to store current processed sql text line, NULL terminated
	** this memory was reused for each new sql line with memory re-allocated.
	*/
	unsigned char *sqlLine;


	/*
	** buf: after reading a new line into sqlLine, characters in sqlLine was saved
	** in a reverse order into buf from buf[wcslen(sqlLine)] downto buf[1]. buf[0] was not used.
	** 
	** bufsize: if we need to save bufsize's characters in buf,  with size = bufsize + 1
	**
	** bufptr: point to char which will be process, set to wcslen(sqlLine) after reading a new line
	*/
	unsigned char *buf;
	size_t bufsize;
	size_t bufptr;


	/*
	** literallen: current number of characters in literalbuf,
	** literalbuf: buffer to store literal while scanning input sql
	**
	** literalbufsize: total memory allocated, number of characters, literalbufsize >= literallen
	*/
	size_t literallen; 
	char *literalbuf;
	size_t literalbufsize;


	/*
	** yytextlen: current number of characters in yytextbuf, 
	** yytextbuf: buffer to store identifier while scanning input sql
	**
	** yytextbufsize: total memory allocated, number of characters. yytextbufsize >= yytextlen
	*/
	//int		yytextlen;	
	unsigned char	*yytextbuf;
	int		yytextbufsize;

	/*
	** identifier/literal text returned by lexer
	** mmeory of yylvalstr was new allocated
	*/
	unsigned char *yylvalstr;

	unsigned char *dolqstart;//postgresql, start part of Dollar-quoted String Constants
	unsigned char *dolqbuf;
	size_t dolqbufsize;
	size_t dolqlen;


	int		yystate;	//current state of lexical analyzer
	unsigned char	yyactchar;	//current character
	unsigned char	yylastchar;	//last matched character (0 if none)
	int		yyrule;		//matched rule
	int		yyreject;	//current match rejected?
	int		yydone;		//yylex return value set? 
	int		yyretval;	//yylex return value

	// Some state information is maintained to keep track with calls to yymore,
	// yyless, reject, start and yymatch/yymark, and to initialize state
	// information used by the lexical analyzer.

	//contains the initial contents of the yytext variable; this
	//will be the empty string, unless yymore is called which sets yystext
	//to the current yytext
	unsigned char	*yystext;	
	//copy of the original yyleng used to restore state information
	//when reject is used 
	int		yysleng;

	//start state of lexical analyzer (set to 0 during
	//initialization, and modified in calls to the start routine
	int		yysstate;	

	//line state information (1 if at beginning of line, 0 otherwise
	int		yylstate;	

	// 1 based in delphi, Position 0 was not used here
	//stack containing matched rules; yymatches contains the number of matches
	int		yystack[MAX_MATCHES + 1];  
	int		yymatches;

	// 1 based in delphi, Position 0 was not used here
	//for each rule the last marked position (yymark); zeroed when rule
	//has already been considered
	int		yypos[MAX_RULES + 1];      

	int offset;
	
	unsigned char yytablechar;
	int	nchars;
	
	int xcdepth,slashstar,dashdash;
	unsigned char delimiterchar;
	int isqmarktoident;


}gsp_lexer;



typedef void yyparse_error_handle(struct gsp_sqlparser *,EErrorType,char *, char *, int toeknLen,int , int, int);

typedef struct TDatabaseYYSType {
    char *yylzString;
    gsp_sourcetoken *yyTSourceToken;
    gsp_list *yyTParseTreeNodeList;
    gsp_node *yyTParseTreeNode;
} TDatabaseYYSType;

typedef struct YYARec {
                int sym, act;
              }YYARec;

typedef struct YYRRec {
                int len, sym;
              }YYRRec;

typedef struct gsp_yyparser{
	int (*yyparse)(struct gsp_sqlparser *sqlparser,struct gsp_yyparser *);
	int (*plsql_yyparse)(struct gsp_sqlparser *sqlparser,struct gsp_yyparser *);
	void (*yyaction) (struct gsp_sqlparser *,struct gsp_yyparser *,int );

	gsp_sql_statement *sqlstatement;
	struct gsp_sqlparser *sqlparser;
	gsp_node *parseTree;
	int curtokenpos;
	yyparse_error_handle *error_handle;
	TDatabaseYYSType yylval;
	TDatabaseYYSType yyval;
	TDatabaseYYSType yyv[yymaxdepth+1];
	int yysp;
	gsp_sourcetoken *sourcetokenlist;
	gsp_sourcetoken *currentsourcetoken;

	const YYARec *yya;		//yya[1 + yynacts];		1 based, yya[0] was just a placeholder
	const YYARec *yyg;		//yyg[1 + yyngotos];	1 based, yyg[0] was just a placeholder
	const YYRRec *yyr;		//yyr[1 + yynrules];	1 based, yyr[0] was just a placeholder
	const int *yyd;				//yyd[yynstates];		0 based
	const int *yyal;				//yyal[yynstates];		0 based
	const int *yyah;				//yyah[yynstates];		0 based
	const int *yygl;				//yygl[yynstates];		0 based
	const int *yygh;				//yygh[yynstates];		0 based

	//void *pMem[MEMORY_SLOTS_YYPARSE]; // a list of memory used to store parse tree node
	//int nAllocated[MEMORY_SLOTS_YYPARSE]; // a list of number related to pMem[n], means allocated memory
	//int nUsed[MEMORY_SLOTS_YYPARSE]; // a list of number related to pMemp[n], means used memory

	void **pMem; // a list of memory used to store parse tree node
	unsigned int *nUsed; // a list of number related to pMemp[n], means used memory
	unsigned int nMem; // number of memory slots current used
	unsigned int nMemAllocated; // number of memory slots current allocated

	gsp_node *currNode;

}gsp_yyparser;


typedef unsigned int gsp_char_ucs4; // UCS-4, 4 bytes unicode wide char

typedef struct YYTRec {
                const unsigned char *cc;
                int  s;
              }YYTRec;

typedef struct gsp_context{
	struct gsp_context *parentContext;
	gsp_yyparser *yyparser;
	int isTop;
	gsp_objectname *db;
	gsp_objectname *schema;
}gsp_context;

#define GSP_NEWLINE			'\xA'
#define GSP_RETURN			'\xD'
#define GSP_WCHAR_NULL		'\0'
#define GSP_LINEBREAK		"\n\r"
#define GSP_LINEBREAK_LEN   2


extern const unsigned char gsp_UpperToLower[];

# define gsp_Tolower(x)   (gsp_UpperToLower[(unsigned char)(x)])

/*!
* malloc used by gsp
*/
void *gsp_malloc(int n);

/*!
* free memoery used by gsp
*/
void gsp_free(void *p);

/*!
* realloc memory used by gsp
*/
void *gsp_realloc(void *pOld, int n);

int gsp_strICmp(const char *zLeft, const char *zRight);
int gsp_strnICmp(const char *zLeft, const char *zRight, int N);

int		gsp_strlen30(const char *z);
char	*gsp_appendText(char *zIn, char const *zAppend, char quote);
int		gsp_string_startsWith(const char *str,const char *substr);
int		gsp_string_endsWith(const char *str,const char *substr);
int		gsp_wchar_string_startsWith(const wchar_t *str, const wchar_t *substr);
int		gsp_wchar_string_endsWith(const wchar_t *str, const wchar_t *substr);
int		gsp_string_indexOf(const char *str, int len,const char *substr);

char *gsp_local_to_utf8(const char *from_str);
char *gsp_local_to_utf8_win(const char *from_str);
char *gsp_utf8_to_local_win(const char *from_str);
int gsp_utf8_strlen(const char *mbstr);
void gsp_view_memory(void *var, int num_of_byte);
char *gsp_widechar_to_local(wchar_t *from_str);
wchar_t *gsp_local_to_widechar(char *from_str);
void endian_swap_16(unsigned int *x);
void endian_swap_8(unsigned short *x);
char *gsp_widechar_to_local_win(wchar_t *from_str);
char *gsp_local_getline(char *zPrompt, FILE *in);

char *gsp_strstr(const char *str,int len,const char *substr);

#ifdef __cplusplus
}
#endif

#endif   /* GSP_BASE_H */
