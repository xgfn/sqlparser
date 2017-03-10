#-print-filei!/bin/bash
g++ `sqlrserver-config --cflags` `rudiments-config --cflags` -shared -o sqlrtranslation_sqlparser.so sqlparser.cpp `sqlrserver-config --libs`  `rudiments-config --libs` -L../../../lib/ -L./../analyzescript/  -L/home/max/oss/sqlparser/mytest/analyzescript  -L../openssl-1.0.1e/lib/ -Wl,-whole-archive -lgspcore -lgspcollection -lsqlparser -lcrypto -Wl,-no-whole-archive 
sudo cp sqlrtranslation_sqlparser.so /usr/local/firstworks/libexec/sqlrelay/

