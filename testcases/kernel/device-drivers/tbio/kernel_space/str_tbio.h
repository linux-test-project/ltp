/* 
 * Remember that you want to seperate your header 
 * files between what is needed in kernel space 
 * only, and what will also be needed by a user 
 * space program that is using this module. For 
 * that reason keep all structures that will need
 * kernel space pointers in a seperate header file
 * from where ioctl flags aer kept
 * 
 * author: Sean Ruyle
 * date:   06/11/2003
 *
 */

struct ltpmod_user {

//put any pointers in here that might be needed by other ioctl calls

};
typedef struct ltpmod_user ltpmod_user_t;

