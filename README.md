General SQL parser C/C++ version
===========
General SQL Parser C/C++ version is written in ANSI-C, so this SQL library can
be used by the most C/C++ compilers including but not limited to GCC, 
Objective-C, Mircosoft VC, Borland C++ builder. This SQL library can be used 
under various platforms such as Liunx, HP-UX, IBM AIX, SUN solaris, MAC-OS 
and windows.

General SQL Parser C/C++ version is valuable because it provides an in-depth 
and detailed analysis of SQL scripts for various databases, including Oracle, 
SQL Server, DB2, MySQL,PostgreSQL, Teradata,netezza,informix,Sybase and Access. 
Without a complete query parser like this, such a task will be impossible. 
You now have the chance to fully incorporate this Ansi C/C++ SQL parser into 
your products, instantly adding a powerful SQL processing capability to your 
C and C++ programs.


Here are some of most appealing features:
==========
* Table, column, variable retrieval
* Modify sql parse tree and rebuild SQL
* Find affected DB object
* CURD report
* Query builder
* Impact analysis
* Table/Column data lineage
* Anti SQL injection
* Convert SQL between different DBs
* Generate XML output of SQL parse tree
* Others


Directory:
=========
 core/
 	header files for sql parser core
 demos/
 	demos illustrate how to use SQL library
 ext/
 	extension of sql parser core, with source code included
 lib/
 	static library file including gsp core and collection inside ext/collection


Compile the first demo(first.c) under windows using Visual C
=======
Set environment for using Microsoft Visual Studio 2008 x86 tools by running:

C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat
then, generate first.exe by running:

cl first.c  /I ../core /link ../lib/gspcore.lib


compile this simple C program under Linux using gcc
=======
gcc -O2 first.c -I../core -L../lib -lgspcore -o first

compile this simple C program under Linux using gcc for .so library
=======
gcc -O2 first.c -I../core -L../lib -lgspcore -o first
export LD_LIBRARY_PATH=../lib
./first

How to build demo using microsoft visual studio
=============
1. open a dos command, enter into demos directory
2. modify env.bat to set correct path while microsoft visual studio was installed
3. nmake -f Makefile , all demos execute file will be created under demos/build directory

how to build demo using gcc
=============
1. open a shell, enter into demos directory
2. make -f Makefile.linux , all demos execute file will be created under demos/build directory



Sincerely,

The Gudu Software Development Team

Copyright 2002-2014 Gudu Software, Inc.
http://www.sqlparser.com
