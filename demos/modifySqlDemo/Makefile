LIB_DIR =  ..\..\lib
BUILD_DIR = ..\build
DEMO_SRC_DIR = .
CORE_DIR = ..\..\core
COLLECTION_DIR = ..\..\ext\collection
EXPRTRAVERSE_DIR = ..\..\ext\expr_traverse
NODEVISITOR_DIR = ..\..\ext\node_visitor
MODIFYSQL_DIR = ..\..\ext\modifysql

CORE_LIB_NAME = gspcore.lib
COLLECTION_LIB_NAME = gspcollection.lib

CC = cl.exe
LINK = link.exe
CFLAGS = /O2 /I $(MODIFYSQL_DIR) /I $(COLLECTION_DIR)\includes /I $(EXPRTRAVERSE_DIR) /I $(CORE_DIR) /I $(NODEVISITOR_DIR) /Fo"$(BUILD_DIR)\\" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /W3 /c /TC 
CLINK =  $(LIB_DIR)\$(CORE_LIB_NAME) $(LIB_DIR)\$(COLLECTION_LIB_NAME)
DEMO_EXE = $(BUILD_DIR)/modifySqlDemo.exe

SOURCE = $(DEMO_SRC_DIR)\modifySqlDemo.c \
	$(MODIFYSQL_DIR)\modifysql.c
	

OBJECTS = $(BUILD_DIR)\modifySqlDemo.obj \
	$(BUILD_DIR)\modifysql.obj


all: $(DEMO_EXE)

$(DEMO_EXE): create_build_dir \
	$(OBJECTS) \
	create_demo_exe \
	clean_demo_obj
	
create_demo_exe:	
	$(LINK) /OUT:"$(DEMO_EXE)" $(CLINK) $(OBJECTS)
	
create_build_dir:
	 -@ if NOT EXIST $(BUILD_DIR) mkdir $(BUILD_DIR)
	 	
$(BUILD_DIR)\modifySqlDemo.obj: $(DEMO_SRC_DIR)\modifySqlDemo.c
	$(CC) $(CFLAGS) $(DEMO_SRC_DIR)\modifySqlDemo.c
	
$(BUILD_DIR)\modifysql.obj: $(MODIFYSQL_DIR)\modifysql.c
	$(CC) $(CFLAGS) $(MODIFYSQL_DIR)\modifysql.c


clean_demo_obj:
	del /Q $(BUILD_DIR)\*.obj $(BUILD_DIR)\*.idb $(BUILD_DIR)\*.pdb
	
clean:
	rmdir /Q /S $(BUILD_DIR)