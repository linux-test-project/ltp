/* jlist.c : Ballista Linked List - Compiler
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

#include <stdlib.h>
#include <stdio.h>
#include "jlist.h"
#include "butil.h"

struct jelement *jpop(struct jlist *theList)
     /*	Pop removes the first element from theList and
	returns a pointer to that element */
{
  struct jelement *temp1,*temp2;
  
  if (theList == NULL)														/* Check for a NULL list*/
    jerror("Passed a NULL list into jpop");
  
  if (theList->number_of_elements == 0)	/*Check to see if we have anything!*/
    return(NULL);
  
  temp1 = theList->head;
  temp2 = temp1;										/* Save a pointer to the first value*/
  temp2 = temp2->next;							/* Increment our list */
  if (temp2!=NULL)
    temp2->previous = NULL;					/* Point it to nothing */
  theList->head = temp2;						/* store it back in our list structure*/
  
  if (temp2 == NULL)								/* If we get here it means that we poped out last*/
    theList->tail = NULL;						/* element, so make sure our tail is NULL too*/
  
  temp1 ->next = NULL;							/* make sure we can't access anything once we return*/
  
  theList->number_of_elements --;		/* Decrement the element counter */
  return(temp1);
}

void jpush(struct jlist *theList,struct jelement *theElement)
     /* Push changes insert mode to stack_insert, inserts
	the element, then changes mode back to original */
{
  if (theList == NULL)										/* Check for a NULL list*/
    jerror("Passed a NULL list into jpush");
  
  if (theElement == NULL)									/* Test for a valid element to push*/
    jerror("Tried to push a NULL pointer onto the list.");
  
  theList->i_am_sorted = 0;								/* We are no longer a sorted list */
  theElement->previous = NULL;						/* make sure we are not pointing up anywhere */
  theElement->next = theList->head;   		/* link to the last head*/
  if (theList->number_of_elements == 0)		/* if this is the first item in the list */
    theList->tail = theElement;     			/* make sure we also know it is the last element */
  else																		/* If we are not the only element in the list*/
    theList->head->previous = theElement;	/* point the old head up to us*/
  theList->head = theElement;							/* Point the list head to it */
  
  theList->number_of_elements ++;					/* Increment the element counter */
  /* Done */
}

void jenqueue(struct jlist *theList,struct jelement *theElement)
     /* Enqueue inserts theelement to the end of the list */
{
  struct jelement *temp;
  
  if (theList == NULL)										/* Check for a NULL list*/
    jerror("Passed a NULL list into jenqueue");
  
  if (theElement == NULL)									/* Test for a valid element to push*/
    jerror("Tried to enqueue a NULL pointer onto the list.");
  
  theList->i_am_sorted = 0;								/* We are no longer a sorted list */
  theElement->next = NULL;								/* Make sure we don't point down at anything*/
  theElement->previous = theList->tail;		/*Link up to the tail*/
  if (theList->number_of_elements == 0)		/* if this is the first item in the list */
    theList->head = theElement;     			/* make sure we also know it is the first element */
  else																		/* If we are not the only element in the list*/
    {
      temp = theList->tail;					
      temp->next = theElement;						/* point the old tail down to us*/
    }
  theList->tail = theElement;							/* Point the list tail to it */
  
  theList->number_of_elements ++;					/* Increment the element counter */
}

struct jelement *jdequeue(struct jlist *theList)
     /* Dequeue removes the first element from theList and returns
	a pointer to it */
{
  if (theList == NULL)					/* Check for a NULL list*/
    jerror("Passed a NULL list into jdequeue");
  
  return (jpop(theList));				/* This is the same as the POP operation */
}

void insert_element(struct jlist *theList,struct jelement *theElement)
     /* insert_element inserts an element into the list using whatever
	mode we are in */
     /* note to myself -- the insert in order does not take into acount
	starting with an unsorted list... */
{
  struct jelement *temp1,*temp2;
  short vault;
  int m;				/* this variable is a mutiplier for compare, which 
					   causes us to sort/search in the desired direction */
  
  if (theList == NULL)														/* Check for a NULL list*/
    jerror("Passed a NULL list into insert_element");
  
  if (theList->sort_ascending) m=1;					/* ascending/descending multiplier*/
  else m=-1;
  
  switch (theList->insert_mode)							/* Insert in accordance with insert_mode*/
    {
    case (insert_in_order) :								/* Insert in Order */
      if (theList->compare == NULL)					/* check for compare function */
	jerror("Tried to insert an element in order prior to defineing compare function");
      
      vault = theList -> i_am_sorted;				/* save this in case we push or queue*/
      /* because they will set this to 0 */
      
      if (theList->number_of_elements == 0)	/* As the first element it does not*/
	jpush (theList,theElement);					/* Really matter how we put it in */
      else
	{
	  temp1 = theList->head;
	  while ((temp1 !=NULL)&&((theList->compare(theElement,temp1))*m==1))
	    temp1 = temp1->next;						/* Keep rolling until we find the one after us*/
	  if (temp1 == NULL)								/* We are adding to the end so que it*/
	    jenqueue(theList,theElement);
	  else if (temp1 == theList ->head)	/* We are adding to the front so push it*/
	    jpush(theList,theElement);
	  else															/* We are adding to the middle somewhere */
	    {
	      temp2 = temp1->previous;			/* Lets look at where we were just before this*/
	      temp2->next = theElement;			/* Insert theElement into the forward chain*/
	      temp1->previous = theElement;	/* Insert theElement into the backward chain*/
	      theElement->previous = temp2; /* Aim us backwards */
	      theElement->next = temp1;			/* Aim us forwards */
	      theList->number_of_elements ++;		/* Increment the element counter */
	    }
	}
      theList->i_am_sorted = vault;			/* restore in case we screwed i_am_sorted up*/
      break;
    case (stack_insert) :										/* Push it on */
      jpush (theList,theElement);
      break;
    case (queue_insert) :										/* Queue it in */
      jenqueue (theList,theElement);
      break;
    default:
      jerror("Fatal Error: Failed to initialize insert_mode to known value");
    }
}

struct jelement *find_element(struct jlist *theList,struct jelement *query)
     /* this function returns a pointer to a record matching query if available
	it will use a binary search algorithm if sorted, or brute force if not.
	If it can't find the element it returns NULL*/
     /* Note to myself -- Duh! -- you can't do a binary search on a dynamic list!!*/
{
	struct jelement *temp1;
	int tflag=1;
	int m;				/* this variable is a mutiplier for compare, which 
									 causes us to sort/search in the desired direction */

	if (theList == NULL)														/* Check for a NULL list*/
		jerror("Passed a NULL list into find_element");

	if (theList->compare == NULL)
		jerror("Tried to find an element prior to defineing compare function");

	if (theList->sort_ascending) m=1;
	else m=-1;
	if (theList->head == NULL) return(NULL);	/*Can't be in an empty list*/
	temp1 = theList->head;										/* Need a temp variable */
	if (theList->i_am_sorted)
	  {
	    tflag = theList->compare(query,temp1);
	    /* we can just look until we are greater than temp*/
	    while (((tflag*m==1) && temp1 !=NULL))
	      {
		temp1 = temp1->next;
		if (temp1!=NULL)
		  tflag = theList->compare(query,temp1);	
		/* Keep rolling until we find it*/
	      }
	    if (tflag !=0) temp1 = NULL;
	  }
	else	 			     /* exhaustive search*/
	  while (((theList->compare(query,temp1))*m!=0) && temp1 !=NULL)
	    temp1 = temp1->next;      /* Keep rolling until we find it*/
	return (temp1);
}

void delete_element(struct jlist *theList,struct jelement *trash)
       /* this function removes the element pointed to by trash */
       /* Note to myself -- add in code to ensure trash is in thelist*/
{
	struct jelement *temp1,*temp2;

	/* Check for valid parameters */
	if (trash == NULL)
		jerror("Tried to delete a NULL pointer element");
	if (theList->head == NULL)
		jerror("Tried to delete from a NULL list");
	if (theList->number_of_elements == NULL)
		jerror("Tried to delete from a list with 0 elements but a non-null head pointer!");
	
	/* First we have to remove it from the list */
	if (theList->head == trash)					/* its the head of the list, lets pop it off */
	  trash = jpop(theList);						/* note if only 1 element it is also the tail */
	else if (theList->tail == trash)		/* it is the tail*/
	  {																	/* more that one elements must be in list */
	    temp1 = trash->previous;				/* get the element before it */
	    temp1->next = NULL;							/* terminate the list there */
	    theList->tail = temp1;					/* point tail to it */
	    theList->number_of_elements --;	/* Decrement the element counter */
	  }
	else																/* we are deleting from the middle somewhere*/
	  {	
	    temp1 = trash->previous;				/* get the element before it */
	    temp2 = trash->next;						/* get the element after it */
	    temp1->next = temp2;						/* establish forward link */
	    temp2->previous = temp1;				/* establish backward link */
	    theList->number_of_elements --;	/* Decrement the element counter */
	  }
	free(trash->theData);								/* release data memory*/
	free(trash);												/* release element memory */
      }

struct jlist *sort_list(struct jlist *theList)
     /* this function sorts the list either ascending or descending
	depending on the value of sort_ascending
	In fact, it is pretty inefficient, it uses insertion sort due
	to the nature of the data structure.  I could have done a bubble sort,
	but that isn't really any faster.  besides, we are not talking
	about dealing with more than just a few thousand nodes here.*/
{
  struct jlist *templist = NULL;
  struct jelement *myel = NULL;
  struct jelement *tempel = NULL;

  /*
  struct mydata *datap = NULL;
  
  int j;
  */

  if (theList == NULL)														/* Check for a NULL list*/
    jerror("Passed a NULL list into sort_list");
  
  if (theList->compare == NULL)
    jerror("Tried to sort a list prior to defineing compare function");
  if (theList->copydata == NULL)
    jerror("Tried to sort a list prior to defineing copydata function");
  
  if (theList->i_am_sorted) return (theList);					/* it is already sorted, so return*/
  
  templist = malloc(sizeof(struct jlist));						/* Allocate space for a new list*/
  if (templist == NULL) jerror("Error.  Could not allocate memory for base structure in SORT");
  
  templist->number_of_elements = 0;										/* Zero elements initially */
  templist->sort_ascending = theList->sort_ascending;	/* Sort in the desired direction*/								
  templist->i_am_sorted = 1;													/* empty therefore sorted*/
  templist->stay_sorted = theList->stay_sorted;				/* save this value*/
  templist->circular = theList->circular;							/* save this value as well */
  templist->insert_mode = insert_in_order;						/* Cause we are sorting */
	templist->compare = theList->compare;								/* Use the same compare function*/
  templist->copydata = theList->copydata;							/* Use the same copydata function*/
  
  templist->head = NULL;															/* No head yet*/
  templist->tail = NULL;															/* No tail yet*/
  
  tempel = theList->head;															/* Start adding at the head*/
  while (tempel != NULL)															/* Iterate until done with theList */
    {
      myel = malloc(sizeof(struct jelement));					/* Allocate a new element */
      if (myel == NULL)																/* Too little memory? */
	jerror("Error.  Could not allocate memory for element structure in SORT");
      
      theList->copydata (tempel,myel);								/* copy in the data */
      myel->next = NULL;															/* make sure we don't point yet*/
      myel->previous = NULL;
      insert_element(templist,myel);									/* insert the element */
      tempel = tempel->next;													/* go to the next node */
    }
  templist -> insert_mode = theList -> insert_mode;		/* keep this the same */
  destroy_list(theList);
  return (templist);
}

void do_action(struct jlist *theList, 
	       int (*func)(struct jelement *),
	       int (*filter)(struct jelement *)
	       )
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
{
  struct jelement *temp;
  
  if (theList == NULL)														/* Check for a NULL list*/
    jerror("Passed a NULL list into do_action");
  if (func == NULL)																/* Check for a NULL action procedure*/
    jerror("Passed a NULL action procedure into do_action");
  
  temp = theList->head;
  
  if (filter == NULL)															/* if there is no filter proc*/
    while (temp != NULL)
      {
	if (!func(temp))													/* do the function and check if we*/
	  theList->i_am_sorted = 0;								/* messed up the sort order */
	temp = temp->next;			
      }
  else
    while (temp != NULL)
      {
	if (filter(temp))													/* did we pass through the filter?*/
	  if (!func(temp))
	    theList->i_am_sorted = 0;
	temp = temp->next;
      }
  
}

int count_elements(struct jlist *theList)
     /* this function returns the number of elements in the list */
{
  if (theList == NULL)														/* Check for a NULL list*/
    jerror("Passed a NULL list into count_elements");
  return (theList->number_of_elements);
}

struct jelement *next_in_list(struct jlist *theList,struct jelement *theElement)
     /* this function returns a pointer to the next element in the list*/
{	
  struct jelement *temp;
  
  if (theList == NULL)												/* Check for a NULL list*/
    jerror("Passed a NULL list into next_in_list");
  if (theElement == NULL)											/* Check for a NULL element*/
    jerror("Passed a NULL element into next_in_list");
  
  if (theList->head==NULL) return (NULL);			/* Does the list exist*/
  if (theElement == NULL) return (NULL);			/* Does the element exist? */
  temp = theElement->next;										/* Go to the next one */
  if ((temp == NULL) && (theList->circular))	/* if we reached the end and are circular*/
    temp = theList->head;											/* then go to the head */
  return (temp);															/* return the value */
}

struct jelement *previous_in_list(struct jlist *theList,struct jelement *theElement)
     /* this function returns a pointer to the previous element in the list*/
{
  struct jelement *temp;
  
  if (theList == NULL)												/* Check for a NULL list*/
    jerror("Passed a NULL list into previous in list");
  if (theElement == NULL)											/* Check for a NULL list*/
    jerror("Passed a NULL element into previous in list");
  
  if (theList->head==NULL) return (NULL);			/* Does the list exist*/
  if (theElement == NULL) return (NULL);			/* Does the element exist? */
  temp = theElement->previous;								/* Go to the previous one */
  if ((temp == NULL) && (theList->circular))	/* if we passed the top and are circular*/
    temp = theList->tail;											/* then go to the tail */
  return (temp);															/* return the value */
}

void destroy_list(struct jlist *theList)
     /* this function frees all list memory and sets theList = NULL */
{
  struct jelement *temp;
  
  if (theList == NULL)												/* Check for a NULL list*/
    jerror("Passed a NULL list into destrol_list");
  
  while (theList->number_of_elements >0)		/* while there are still items in the list*/
    {
      temp = theList->head;									/* point to the head */
      delete_element(theList,temp);					/* and trash it */
    }
}

