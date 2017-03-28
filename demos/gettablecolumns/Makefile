LIB_DIR =  ..\..\lib
BUILD_DIR = ..\build
DEMO_SRC_DIR = .
CORE_DIR = ..\..\core
COLLECTION_DIR = ..\..\ext\collection
EXPRTRAVERSE_DIR = ..\..\ext\expr_traverse
NODEVISITOR_DIR = ..\..\ext\node_visitor
CORE_LIB_NAME = gspcore.lib
COLLECTION_LIB_NAME = gspcollection.lib

CC = cl.exe
LINK = link.exe
CFLAGS = /O2 /I $(COLLECTION_DIR)\includes /I $(EXPRTRAVERSE_DIR) /I $(CORE_DIR) /I $(NODEVISITOR_DIR) /Fo"$(BUILD_DIR)\\" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /Fo"$(BUILD_DIR)\\" /W3 /c /TC 
CLINK =  $(LIB_DIR)\$(CORE_LIB_NAME) $(LIB_DIR)\$(COLLECTION_LIB_NAME)
DEMO_EXE = $(BUILD_DIR)/gettablecolumns.exe

SOURCE = $(DEMO_SRC_DIR)\gettablecolumns.c \
	$(EXPRTRAVERSE_DIR)\expr_traverse.c \
	$(NODEVISITOR_DIR)\node_visitor.c 
	

OBJECTS = $(BUILD_DIR)\gettablecolumns.obj \
	$(BUILD_DIR)\expr_traverse.obj \
	$(BUILD_DIR)\node_visitor.obj


all: $(DEMO_EXE)

$(DEMO_EXE): create_build_dir \
	$(OBJECTS) \
	create_demo_exe \
	clean_demo_obj
	
create_demo_exe:	
	$(LINK) /OUT:"$(DEMO_EXE)" $(CLINK) $(OBJECTS)
	
create_build_dir:
	 -@ if NOT EXIST $(BUILD_DIR) mkdir $(BUILD_DIR)
	 	
$(BUILD_DIR)\gettablecolumns.obj: $(DEMO_SRC_DIR)\gettablecolumns.c
	$(CC) $(CFLAGS) $(DEMO_SRC_DIR)\gettablecolumns.c
	
$(BUILD_DIR)\expr_traverse.obj: $(EXPRTRAVERSE_DIR)\expr_traverse.c
	$(CC) $(CFLAGS) $(EXPRTRAVERSE_DIR)\expr_traverse.c

$(BUILD_DIR)\node_visitor.obj: $(NODEVISITOR_DIR)\node_visitor.c
	$(CC) $(CFLAGS) $(NODEVISITOR_DIR)\node_visitor.c

clean_demo_obj:
	del /Q $(BUILD_DIR)\*.obj $(BUILD_DIR)\*.idb $(BUILD_DIR)\*.pdb
	
clean:
	rmdir /Q /S $(BUILD_DIR)