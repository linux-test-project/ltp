/* jlist.h : Ballista Linked List Header
   Copyright (C) 1995-2001  Carnegie Mellon University

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* Linked List Interface Module 
   John Peter DeVale
   
   Abstract linked list module implementing a double linked list
   and can operate as a circular or terminated list.

   the data is a void casted pointer.

   the list is sortable by built in sorting algorithms
   
   the list uses a variable compare and copydata function

   the list can have a passed in function operate on every node
   which passes a test by a passed in filtering function.  If the passed
   in function is null, it operates on every node.

   the list can be specified to be sorted on insert, or not.

   the list can be specified to be circular or not

*/
#ifndef JLIST_H
#define JLIST_H
#define insert_in_order 1
#define stack_insert 2
#define queue_insert 3
#ifndef NULL
#define NULL 0
#endif

/* Data structure */

struct jelement
{
  void            *theData;
  struct jelement *next;
  struct jelement *previous;
};

struct jlist
{
  long number_of_elements;		/* keep track of the number of elements in our list */
  short sort_ascending;		/* if set to one it sorts ascending vice descending */

  short i_am_sorted;			/* this variable is set to 1 if the list thinks
    				it is sorted, or 0 if it thinks it is not */

  short stay_sorted;			/* this is set to 1 if we want to sort the list
				     if we realize it has somehow become unsorted
				     like from using stack or queue mode operations
				     If we are doing a search, and we know we are
				     unsorted and this is set to 1 we will sort the
				     list prior to searching and reset i_am_sorted
				     otherwise we will execute a brute force search*/

  short circular;		      	/* set to 1 if list is circular, 0 if not */

  short insert_mode;			/* this controls the way the data is inserted into
				     the list.  If insert_in_order it inserts the
				     data in order according to the compare function
				     if stack_insert it inserts the element onto 
				     the head of the list.  if queue insert then
				     it inserts the element on to the end of the 
				     list*/

  int   data_size;		      	/* the size of the data stored */

  int (*compare)();		       	/* compare function takes two jelement pointers and
	     			returns an int. 1 for param1>param2 0 for
				param1 == param2 and -1 for param1 < param2 */
  
  void (*copydata)();			/* the copy data function copys the data from one data element 
				     to another.  it copies param1 into param2.  param1 , param2 are
				     pointers of type jelement, and must be already allocated
				     the copydata procedure must allocate memory for the actual data */

  struct jelement *head;		/* Head of the list */

  struct jelement *tail;		/* Tail of the list*/
};

extern struct jelement *jpop(struct jlist *theList);
	/* Pop removes the first element from theList and
	returns a pointer to that element */

extern void jpush(struct jlist *theList,struct jelement *theElement);
	/* Push changes insert mode to stack_insert, inserts
	the element, then changes mode back to original */

extern void jenqueue(struct jlist *theList,struct jelement *theElement);
	/* Enqueue inserts theelement to the end of the list */

extern struct jelement *jdequeue(struct jlist *theList);
	/* Dequeue removes the first element from theList and returns
	a pointer to it */

extern void insert_element(struct jlist *theList,struct jelement *theElement);
	/* insert_element inserts an element into the list using whatever
	mode we are in */

extern struct jelement *find_element(struct jlist *theList,struct jelement *query);
	/* this function returns a pointer to a record matching query if available
	it will use a binary search algorithm if sorted, or brute force if not.
	If it can't find the element it returns NULL*/

extern void delete_element(struct jlist *theList,struct jelement *trash);
	/* this function removes the element pointed to by trash */

extern struct jlist *sort_list(struct jlist *theList);
	/* this function sorts the list either ascending or descending
	depending on the value of sort_ascending*/

extern void do_action(struct jlist *theList, 
				  int (*func)(struct jelement *),
				  int (*filter)(struct jelement *)
				  );
	/* this function performs an action (func) on each element which gets a 1
	returned from (filter).  if filter = null then (func) is executed on
	each element 
	func declaration should be of type:
	int func(struct jelement *theElement)
	where the return value is 0 if the function may have messed up sorting, 1 otherwise
	filter should be:
	int filter(struct jelement *theElement)
	returning 1 if pass, 0 if filtered
	*/

extern int count_elements(struct jlist *theList);
	/* this function returns the number of elements in the list */

extern struct jelement *next_in_list(struct jlist *theList,struct jelement *theElement);
	/* this function returns a pointer to the next element in the list*/

extern struct jelement *previous_in_list(struct jlist *theList,struct jelement *theElement);
	/* this function returns a pointer to the previous element in the list*/

extern void destroy_list(struct jlist *theList);
	/* this function frees all list memory and sets theList = NULL */

#endif
