LIB_DIR =  ../../lib
BUILD_DIR = .
DEMO_SRC_DIR = .
CORE_DIR = ../../core
COLLECTION_DIR = ../../ext/collection
EXPRTRAVERSE_DIR = ../../ext/expr_traverse
NODEVISITOR_DIR = ../../ext/node_visitor
GSP_CORE_LIB = gspcore
GSP_COLLECTION_LIB = gspcollection

CC = gcc
CFLAGS = -c -g -I $(COLLECTION_DIR)/includes -I $(EXPRTRAVERSE_DIR) -I $(CORE_DIR) -I $(NODEVISITOR_DIR)


DEMO_EXE = $(BUILD_DIR)/analyzescript

SOURCE = $(DEMO_SRC_DIR)/analyzescript.c \
	$(EXPRTRAVERSE_DIR)/expr_traverse.c \
	$(NODEVISITOR_DIR)/node_visitor.c 
	

OBJECTS = $(BUILD_DIR)/analyzescript.o \
	$(BUILD_DIR)/expr_traverse.o \
	$(BUILD_DIR)/node_visitor.o


all: $(DEMO_EXE)

$(DEMO_EXE): create_build_dir \
	$(OBJECTS) \
	create_demo_exe \
	clean_demo_obj
	
create_demo_exe:
	$(CC)  $(OBJECTS)  -L$(LIB_DIR) -l$(GSP_CORE_LIB) -l$(GSP_COLLECTION_LIB) -o $(DEMO_EXE)
	
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

clean_demo_obj:
	rm -f $(BUILD_DIR)/*.o 
	
clean:
	rm -rf $(BUILD_DIR)/analyzescript