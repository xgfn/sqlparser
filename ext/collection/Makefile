LIB_DIR = ..\..\lib
COLLECTION_DIR = .

CC = cl.exe
LIB = lib.exe
CFLAGS = /O2 /I $(COLLECTION_DIR)/includes  /D "_UNICODE" /D "UNICODE" /Fo"$(LIB_DIR)\\" /MT /W3 /c  /TC 


COLLECTION_SOURCE = $(COLLECTION_DIR)\src\c_algo.c \
	$(COLLECTION_DIR)\src\c_function.c \
	$(COLLECTION_DIR)\src\c_iterator.c \
	$(COLLECTION_DIR)\src\c_list.c \
	$(COLLECTION_DIR)\src\c_map.c \
	$(COLLECTION_DIR)\src\c_memory.c \
	$(COLLECTION_DIR)\src\c_pair.c \
	$(COLLECTION_DIR)\src\c_queue.c \
	$(COLLECTION_DIR)\src\c_set.c \
	$(COLLECTION_DIR)\src\c_stack.c \
	$(COLLECTION_DIR)\src\c_tree.c \
	$(COLLECTION_DIR)\src\c_vector.c \
	$(COLLECTION_DIR)\src\comparer.c \
	$(COLLECTION_DIR)\src\lifo_stack.c \
	$(COLLECTION_DIR)\src\linked_list.c \
	$(COLLECTION_DIR)\src\memento.c \
	$(COLLECTION_DIR)\src\tree_map.c \
	$(COLLECTION_DIR)\src\cstring.c
	
COLLECTION_HEADERS = $(COLLECTION_DIR)\includes\c_algo.h \
	$(COLLECTION_DIR)\includes\c_algorithm.h \
	$(COLLECTION_DIR)\includes\c_def.h \
	$(COLLECTION_DIR)\includes\c_function.h \
	$(COLLECTION_DIR)\includes\c_functional.h \
	$(COLLECTION_DIR)\includes\c_iterator.h \
	$(COLLECTION_DIR)\includes\c_list.h \
	$(COLLECTION_DIR)\includes\c_map.h \
	$(COLLECTION_DIR)\includes\c_memory.h \
	$(COLLECTION_DIR)\includes\c_pair.h \
	$(COLLECTION_DIR)\includes\c_queue.h \
	$(COLLECTION_DIR)\includes\c_set.h \
	$(COLLECTION_DIR)\includes\c_stack.h \
	$(COLLECTION_DIR)\includes\c_tree.h \
	$(COLLECTION_DIR)\includes\c_utility.h \
	$(COLLECTION_DIR)\includes\c_vector.h \
	$(COLLECTION_DIR)\includes\comparer.h \
	$(COLLECTION_DIR)\includes\collection.h \
	$(COLLECTION_DIR)\includes\lifo_stack.h \
	$(COLLECTION_DIR)\includes\linked_list.h \
	$(COLLECTION_DIR)\includes\memento.h \
	$(COLLECTION_DIR)\includes\tree_map.h \
	$(COLLECTION_DIR)\includes\cstring.h
 
COLLECTION_OBJECTS = $(LIB_DIR)\c_algo.obj \
	$(LIB_DIR)\c_function.obj \
	$(LIB_DIR)\c_iterator.obj \
	$(LIB_DIR)\c_list.obj \
	$(LIB_DIR)\c_map.obj \
	$(LIB_DIR)\c_memory.obj \
	$(LIB_DIR)\c_pair.obj \
	$(LIB_DIR)\c_queue.obj \
	$(LIB_DIR)\c_set.obj \
	$(LIB_DIR)\c_stack.obj \
	$(LIB_DIR)\c_tree.obj \
	$(LIB_DIR)\c_vector.obj \
	$(LIB_DIR)\comparer.obj \
	$(LIB_DIR)\lifo_stack.obj \
	$(LIB_DIR)\linked_list.obj \
	$(LIB_DIR)\memento.obj \
	$(LIB_DIR)\tree_map.obj \
	$(LIB_DIR)\cstring.obj 
	


target: 	create_build_dir \
	$(COLLECTION_OBJECTS) \
	create_target \
	clean_obj
	
create_target:
	$(LIB) /OUT:"$(LIB_DIR)\gspcollection.lib" $(COLLECTION_OBJECTS)
	
	
create_build_dir:
	-@ if NOT EXIST $(LIB_DIR) mkdir $(LIB_DIR)

remove_build_dir:
	-@ if  EXIST $(LIB_DIR) rmdir /Q $(LIB_DIR)

 
$(LIB_DIR)\c_algo.obj:	$(COLLECTION_DIR)\src\c_algo.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_function.obj:	$(COLLECTION_DIR)\src\c_function.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_iterator.obj:	$(COLLECTION_DIR)\src\c_iterator.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_list.obj:	$(COLLECTION_DIR)\src\c_list.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_map.obj:	$(COLLECTION_DIR)\src\c_map.c
	$(CC) $(CFLAGS) $**
 
$(LIB_DIR)\c_memory.obj:	$(COLLECTION_DIR)\src\c_memory.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_pair.obj:	$(COLLECTION_DIR)\src\c_pair.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_queue.obj:	$(COLLECTION_DIR)\src\c_queue.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_set.obj:	$(COLLECTION_DIR)\src\c_set.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_stack.obj:	$(COLLECTION_DIR)\src\c_stack.c
	$(CC) $(CFLAGS) $**
	
$(LIB_DIR)\c_tree.obj:	$(COLLECTION_DIR)\src\c_tree.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\c_vector.obj:	$(COLLECTION_DIR)\src\c_vector.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\comparer.obj:	$(COLLECTION_DIR)\src\comparer.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\lifo_stack.obj:	$(COLLECTION_DIR)\src\lifo_stack.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\linked_list.obj:	$(COLLECTION_DIR)\src\linked_list.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\memento.obj:	$(COLLECTION_DIR)\src\memento.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\tree_map.obj:	$(COLLECTION_DIR)\src\tree_map.c
	$(CC) $(CFLAGS) $**

$(LIB_DIR)\cstring.obj:	$(COLLECTION_DIR)\src\cstring.c
	$(CC) $(CFLAGS) $**
 
	
clean_obj:
	del /Q $(LIB_DIR)\*.obj $(LIB_DIR)\*.idb $(LIB_DIR)\*.pdb

clean: 
	rmdir /Q /S $(LIB_DIR)