#ifndef GSP_LIST_H
#define GSP_LIST_H

/*!
**  \file  gsp_list.h
**  \brief list used to manipulate parse tree nodes
*/

#include "gsp_base.h"

#ifdef __cplusplus
extern "C" {
#endif


#define NIL						((gsp_list *) NULL)

/*!
* get nth node from list
*/
void *gsp_list_nth(gsp_list *list, int n);

gsp_list *new_list(gsp_yyparser *yyparser,ENodeType type);
gsp_list *gsp_list_append(gsp_yyparser *yyparser,gsp_list *list, void *datum);
gsp_listcell *lappend_cell(gsp_yyparser *yyparser,gsp_list *list, gsp_listcell *prev, void *datum);
gsp_list *gsp_list_insert_first_cell(gsp_yyparser *yyparser,void *datum, gsp_list *list);

gsp_list *gsp_list_concat(gsp_list *list1, gsp_list *list2);
gsp_list *gsp_list_truncate(gsp_list *list, int new_size);

int list_member(gsp_list *list, void *datum);
int list_member_ptr(gsp_list *list, void *datum);


gsp_list *list_delete(gsp_list *list, void *datum);
gsp_list *list_delete_ptr(gsp_list *list, void *datum);

gsp_list *list_delete_first(gsp_list *list);
gsp_list *list_delete_cell(gsp_list *list, gsp_listcell *cell, gsp_listcell *prev);

gsp_list *list_intersection(gsp_yyparser *yyparser,gsp_list *list1, gsp_list *list2);

gsp_list *list_append_unique(gsp_yyparser *yyparser,gsp_list *list, void *datum);

gsp_list *list_append_unique_ptr(gsp_yyparser *yyparser,gsp_list *gsp_list, void *datum);


gsp_list *list_concat_unique(gsp_yyparser *yyparser,gsp_list *list1, gsp_list *list2);
gsp_list *list_concat_unique_ptr(gsp_yyparser *yyparser,gsp_list *list1, gsp_list *list2);


void list_free(gsp_list *list);
void list_free_deep(gsp_list *list);

gsp_list *list_copy(gsp_list *list);
gsp_list *list_copy_tail(gsp_list *list, int nskip);


#ifdef USE_INLINE

static inline gsp_listcell *
gsp_list_head(gsp_list *l)
{
	return l ? l->head : NULL;
}

static inline gsp_listcell *
gsp_list_tail(gsp_list *l)
{
	return l ? l->tail : NULL;
}

static inline int
gsp_list_length(gsp_list *l)
{
	return l ? l->length : 0;
}
#else

gsp_listcell *gsp_list_head(gsp_list *l);
gsp_listcell *gsp_list_tail(gsp_list *l);
int	gsp_list_length(gsp_list *l);
#endif   /* USE_INLINE */

#define gsp_list_next(lc)				((lc)->nextCell)
#define gsp_list_celldata(lc)			((lc)->node)

/*!
* get the first node in the list
*/
#define gsp_list_first(l)				gsp_list_celldata(gsp_list_head(l))

/*!
* get the second node in the list
*/
#define gsp_list_second(l)				gsp_list_celldata(gsp_list_next(gsp_list_head(l)))

/*!
* get the third node in the list
*/
#define gsp_list_third(l)				gsp_list_celldata(gsp_list_next(gsp_list_next(gsp_list_head(l))))

/*!
* get the fourth node in the list
*/
#define gsp_list_fourth(l)				gsp_list_celldata(gsp_list_next(gsp_list_next(gsp_list_next(gsp_list_head(l)))))

/*!
* get the last node in the list
*/
#define gsp_list_last(l)				gsp_list_celldata(gsp_list_tail(l))

#define gsp_list_make1(yyparser,x1)				gsp_list_insert_first_cell(yyparser,x1, NIL)
#define gsp_list_make2(yyparser,x1,x2)			gsp_list_insert_first_cell(yyparser,x1, gsp_list_make1(yyparser,x2))
#define gsp_list_make3(yyparser,x1,x2,x3)		gsp_list_insert_first_cell(yyparser,x1, gsp_list_make2(yyparser,x2, x3))
#define gsp_list_make4(yyparser,x1,x2,x3,x4)	gsp_list_insert_first_cell(yyparser,x1, gsp_list_make3(yyparser,x2, x3, x4))


/*
 * foreach -
 *	  a convenience macro which loops through the list
 */
#define foreach(cell, l)	\
	for ((cell) = gsp_list_head(l); (cell) != NULL; (cell) = gsp_list_next(cell))

/*
 * for_each_cell -
 *	  a convenience macro which loops through a list starting from a
 *	  specified cell
 */
#define for_each_cell(cell, initcell)	\
	for ((cell) = (initcell); (cell) != NULL; (cell) = gsp_list_next(cell))

/*
 * forboth -
 *	  a convenience macro for advancing through two linked lists
 *	  simultaneously. This macro loops through both lists at the same
 *	  time, stopping when either list runs out of elements. Depending
 *	  on the requirements of the call site, it may also be wise to
 *	  assert that the lengths of the two lists are equal.
 */
#define forboth(cell1, list1, cell2, list2)							\
	for ((cell1) = gsp_list_head(list1), (cell2) = gsp_list_head(list2);	\
		 (cell1) != NULL && (cell2) != NULL;						\
		 (cell1) = gsp_list_next(cell1), (cell2) = gsp_list_next(cell2))

/*
 * forthree -
 *	  the same for three lists
 */
#define forthree(cell1, list1, cell2, list2, cell3, list3)			\
	for ((cell1) = gsp_list_head(list1), (cell2) = gsp_list_head(list2), (cell3) = gsp_list_head(list3); \
		 (cell1) != NULL && (cell2) != NULL && (cell3) != NULL;		\
		 (cell1) = gsp_list_next(cell1), (cell2) = gsp_list_next(cell2), (cell3) = gsp_list_next(cell3))



#ifdef __cplusplus
}
#endif

#endif   /* GSP_LIST_H */
