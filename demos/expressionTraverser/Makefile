LIB_DIR =  ..\..\lib
BUILD_DIR = ..\build
DEMO_SRC_DIR = .
COLLECTION_DIR = ..\..\ext\collection
EXPRTRAVERSE_DIR = ..\..\ext\expr_traverse
CORE_DIR = ..\..\core
CORE_LIB_NAME = gspcore.lib
COLLECTION_LIB_NAME = gspcollection.lib

CC = cl.exe
LINK = link.exe
CFLAGS = /Od /I $(COLLECTION_DIR)\includes /I $(EXPRTRAVERSE_DIR) /I $(CORE_DIR) /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /Fo"$(BUILD_DIR)\\" /W3 /c /TC 
CLINK =  $(LIB_DIR)\$(CORE_LIB_NAME) $(LIB_DIR)\$(COLLECTION_LIB_NAME)
DEMO_EXE = $(BUILD_DIR)/expressionTraverser.exe

SOURCE = $(DEMO_SRC_DIR)\expressionTraverser.c \
	$(EXPRTRAVERSE_DIR)\expr_traverse.c \
	

OBJECTS = $(BUILD_DIR)\expressionTraverser.obj \
	$(BUILD_DIR)\expr_traverse.obj 


all: $(DEMO_EXE)

$(DEMO_EXE): create_build_dir \
	$(OBJECTS) \
	create_demo_exe \
	clean_demo_obj
	
create_demo_exe:	
	$(LINK) /OUT:"$(DEMO_EXE)" $(CLINK) $(OBJECTS)
	
create_build_dir:
	 -@ if NOT EXIST $(BUILD_DIR) mkdir $(BUILD_DIR)
	 	
$(BUILD_DIR)\expressionTraverser.obj: $(DEMO_SRC_DIR)\expressionTraverser.c
	$(CC) $(CFLAGS) $(DEMO_SRC_DIR)\expressionTraverser.c
	
$(BUILD_DIR)\expr_traverse.obj: $(EXPRTRAVERSE_DIR)\expr_traverse.c
	$(CC) $(CFLAGS) $(EXPRTRAVERSE_DIR)\expr_traverse.c


clean_demo_obj:
	del /Q $(BUILD_DIR)\*.obj $(BUILD_DIR)\*.idb $(BUILD_DIR)\*.pdb
	
clean:
	rmdir /Q /S $(BUILD_DIR)