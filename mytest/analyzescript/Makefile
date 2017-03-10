LIB_DIR =  ../../lib
BUILD_DIR = .
DEMO_SRC_DIR = .
CORE_DIR = ../../core
COLLECTION_DIR = ../../ext/collection
EXPRTRAVERSE_DIR = ../../ext/expr_traverse
NODEVISITOR_DIR = ../../ext/node_visitor
MODIFYSQL_DIR = ../../ext/modifysql
GSP_CORE_LIB = gspcore
GSP_COLLECTION_LIB = gspcollection

CC = gcc
CFLAGS = -c -g -I $(COLLECTION_DIR)/includes -I $(EXPRTRAVERSE_DIR) -I $(CORE_DIR) -I $(NODEVISITOR_DIR) -I/home/max/xxx/lktcrypto/openssl-Linux/include


DEMO_EXE = $(BUILD_DIR)/analyzescript
DEMO_LIB = $(BUILD_DIR)/libsqlparser.a

SOURCE = $(DEMO_SRC_DIR)/analyzescript.c \
	$(EXPRTRAVERSE_DIR)/expr_traverse.c \
	$(NODEVISITOR_DIR)/node_visitor.c \
	$(DEMO_SRC_DIR)/sqlparser_for_sqlrelay.c \
	$(MODIFYSQL_DIR)/modifysql.c 

OBJECTS = $(BUILD_DIR)/analyzescript.o \
	$(BUILD_DIR)/expr_traverse.o \
	$(BUILD_DIR)/node_visitor.o \
	$(BUILD_DIR)/sqlparser_for_sqlrelay.o\
	$(BUILD_DIR)/modifysql.o 

all: $(DEMO_EXE)

$(DEMO_EXE): create_build_dir \
	$(OBJECTS) \
	create_demo_exe \
	clean_demo_obj
	
#$(CC)  $(OBJECTS)  -L$(LIB_DIR) -l$(GSP_CORE_LIB) -l$(GSP_COLLECTION_LIB) -lrt -g -o $(DEMO_EXE) -lrt -lpthread  -ldl
create_demo_exe:
	ar -r $(DEMO_LIB) ./sqlparser_for_sqlrelay.o ./modifysql.o
	
create_build_dir:
	@if [ ! -d "$(BUILD_DIR)" ]; \
	then \
	    mkdir $(BUILD_DIR); \
	fi \
	 	
$(BUILD_DIR)/analyzescript.o: $(DEMO_SRC_DIR)/analyzescript.c
	$(CC) $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/expr_traverse.o: $(EXPRTRAVERSE_DIR)/expr_traverse.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/node_visitor.o: $(NODEVISITOR_DIR)/node_visitor.c
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sqlparser_for_sqlrelay.o: $(DEMO_SRC_DIR)/sqlparser_for_sqlrelay.c
	$(CC) $(CFLAGS) -fPIC -c ./sqlparser_for_sqlrelay.c 

$(BUILD_DIR)/modifysql.o: $(MODIFYSQL_DIR)/modifysql.c
	$(CC) $(CFLAGS) -fPIC -c $(MODIFYSQL_DIR)/modifysql.c 

clean_demo_obj:
	rm -f $(BUILD_DIR)/*.o 
	
clean:
	rm -rf $(BUILD_DIR)/analyzescript $(DEMO_LIB)
