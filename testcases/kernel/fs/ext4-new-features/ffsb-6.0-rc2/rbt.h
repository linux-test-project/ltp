/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef RED_BLACK_TREE_H
#define RED_BLACK_TREE_H

/*
 * ***************************************************************************
 *
 * Container class for a red-black tree ......
 *
 * A binary tree that satisfies the following properties:
 *
 * 1. Every node is either red or black
 * 2. The root node is black
 * 3. Every leaf (NIL) is black
 * 4. If a node is red, both its children are black
 * 5. For each node, all paths from the node to descendant leaf nodes
 *    contain the same number of black nodes
 *
 * Due to points 4 & 5, the depth of a red-black tree containing n nodes
 * is bounded by 2*log2(n+1) (WC).
 *
 *
 * The rb_tree template requires two additional parmeters:
 *
 * - The contained TYPE class represents the objects stored in the tree.
 *   It has to support the copy constructor and the assignment operator (opr)
 * - cmp is a functor used to define the order of objects of class TYPE:
 *   This class has to support an operator() that recieves two objects from
 *   the TYPE class and returns a negative, 0, or a positive integer,
 *   depending on the comparison result.
 *
 * Dominique Heger, S. Rao
 *
 * ***************************************************************************
 */

/* Color enumeration for nodes of red-black tree */
/* ********************************************* */

#include "filelist.h"

typedef struct ffsb_file *datatype;

#define COMP_NODES(a, b) ((a)->num - (b)->num)

typedef enum red_black_color {red, black} rb_color;

/*! Representation of a node in a red-black tree */
typedef struct red_black_node {
	datatype object;                      /* the stored object */
	rb_color color;                       /* the color of the node */
	struct red_black_node *parent;       /* points to the parent node */
	struct red_black_node *right;        /* points to the right child */
	struct red_black_node *left;         /* points to the left child */
} rb_node;

typedef int(cmp)(datatype, datatype);
typedef void(opr)(void *);
typedef void(destructor)(datatype);

/* Construct of a red-black tree node
 * - The object stored in the node
 * - The color of the node
 */

extern rb_node *rbnode_construct(datatype object, rb_color color);

/* Recursive destructor for the entire sub-tree */
/* ******************************************** */

extern void rbnode_destruct(rb_node *node, destructor d);

/* Calculate the depth of the sub-tree spanned by the given node
 * - The sub-tree root
 * - The sub-tree depth
 */

extern int rbnode_depth(rb_node *node);

/* Get the leftmost node in the sub-tree spanned by the given node
 * - The sub-tree root
 * - The sub-tree minimum
 */

extern rb_node *rbnode_minimum(rb_node *node);

/* Get the rightmost node in the sub-tree spanned by the given node
 * - The sub-tree root
 * - The sub-tree maximum
 */

extern rb_node *rbnode_maximum(rb_node *node);

/* Replace the object */
/* ****************** */

extern void rbnode_replace(rb_node *node, datatype object);

/* Get the next node in the tree (according to the tree order)
 * - The current node
 * - The successor node, or NULL if node is the tree maximum
 */

extern rb_node *rbnode_successor(rb_node *node);

/* Get the previous node in the tree (according to the tree order)
 * - The current node
 * - The predecessor node, or NULL if node is the tree minimum
 */

extern rb_node *rbnode_predecessor(rb_node *node);

/* Duplicate the entire sub-tree rooted at the given node
 * - The sub-tree root
 * - A pointer to the duplicated sub-tree root
 */

extern rb_node *rbnode_duplicate(rb_node *node);

/* Traverse a red-black sub-tree
 * - The sub-tree root
 * - The operation to perform on each object in the sub-tree
 */
extern void rbnode_traverse(rb_node *node, opr *op);

/* Representation of a red-black tree */
/* ********************************** */

typedef struct red_black_tree {
	rb_node *root;                /* pointer to the tree root */
	int isize;                     /* number of objects stored */
	/*   cmp * comp; */                   /* compare function */
} rb_tree;

/* Initialize a red-black tree with a comparision function
 * - The tree
 * - The comparision function
 */

void rbtree_init(rb_tree *tree);

/* Construct a red-black tree with a comparison object
 * - A pointer to the comparison object to be used by the tree
 * - The newly constructed  tree
 */

rb_tree *rbtree_construct(void);

/* Clean a red-black tree [takes O(n) operations]
 * - The tree
 */

extern void rbtree_clean(rb_tree *tree, destructor d);

/* Destruct a red-black tree
 * - The tree
 */

extern void rbtree_destruct(rb_tree *tree, destructor d);

/* Get the size of the tree [takes O(1) operations]
 * - The tree
 * - The number of objects stored in the tree
 */

extern int rbtree_size(rb_tree *tree);

/* Get the depth of the tree [takes O(n) operations]
 * - The tree
 * - The length of the longest path from the root to a leaf node
 */

extern int rbtree_depth(rb_tree *tree);

/* Check whether the tree contains an object [takes O(log n) operations]
 * - The tree
 * - The query object
 * - (true) if an equal object is found in the tree, otherwise (false)
 */

extern int rbtree_contains(rb_tree *tree, datatype object);

/* Insert an object to the tree [takes O(log n) operations]
 * - The tree
 * - The object to be inserted
 * - Return the inserted object node
 */

extern rb_node *rbtree_insert(rb_tree *tree, datatype object);

/* Insert a new object to the tree as the a successor of a given node
 * - The tree
 * - The new node
 */

extern rb_node *insert_successor_at(rb_tree *tree, rb_node *at_node,
				    datatype object);

/* Insert a new object to the tree as the a predecessor of a given node
 * - The tree
 * - The new node
 */

extern rb_node *insert_predecessor_at(rb_tree *tree, rb_node *at_node,
				      datatype object);

/* Remove an object from the tree [takes O(log n) operations]
 * - The tree
 * - The object to be removed
 * - The object should be contained in the tree
 */

extern void rbtree_remove(rb_tree *tree, datatype object, destructor d);

/* Get a handle to the tree minimum [takes O(log n) operations]
 * - The tree
 * - Return the minimal object in the tree, or a NULL if the tree is empty
 */

extern rb_node *rbtree_minimum(rb_tree *tree);

/* Get a handle to the tree maximum [takes O(log n) operations]
 * - The tree
 * - Return the maximal object in the tree, or a NULL if the tree is empty
 */

extern rb_node *rbtree_maximum(rb_tree *tree);

/* Get the next node in the tree (according to the tree order)
 * - [takes O(log n) operations at worst-case, but only O(1) amortized]
 * - The tree
 * - The current object
 * - The successor node, or a NULL, if we are at the tree maximum
 */
extern rb_node *rbtree_successor(rb_tree *tree, rb_node *node);

/* Get the previous node in the tree (according to the tree order)
 * - [takes O(log n) operations at worst-case, but only O(1) amortized]
 * - The tree
 * - The current object
 * - The predecessor node, or a NULL, if we are at the tree minimum
 */

extern rb_node *rbtree_predecessor(rb_tree *tree, rb_node *node);

/* Find a node that contains the given object
 * - The tree
 * - The desired object
 * - Return a node that contains the given object, or NULL if no such object
 *   is found in the tree
 */

extern rb_node *rbtree_find(rb_tree *tree, datatype object);

/* Remove the object stored in the given tree node
 * - The tree
 * - The node storing the object to be removed from the tree
 */

extern void rbtree_remove_at(rb_tree *tree, rb_node *node, destructor d);

/* Left-rotate the sub-tree spanned by the given node
 * - The tree
 * - The sub-tree root
 */

extern void rbtree_rotate_left(rb_tree *tree, rb_node *node);

/* Right-rotate the sub-tree spanned by the given node
 * - The tree
 * - The sub-tree root
 */

extern void rbtree_rotate_right(rb_tree *tree, rb_node *node);

/*
 * Fix the red-black tree properties after an insertion operation
 * - The tree
 * - The node that has just been inserted to the tree
 * - The color of node must be red
 */

extern void rbtree_insert_fixup(rb_tree *tree, rb_node *node);

/* Fix the red-black tree properties after a removal operation
 * - The tree
 * - The child of the node that has just been removed from the tree
 */

extern void rbtree_remove_fixup(rb_tree *tree, rb_node *node);

/* Traverse a red-black tree
 * - The tree
 * - The operation to perform on every object of the tree (according to
 *   the tree order)
 */

extern void rbtree_traverse(rb_tree *tree, opr *op);

#endif
