<assertions>
  <assertion id="1" tag="ref:XSH6:33838:33844">
   The function

   int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict mutex, 
	int *restrict prioceiling);

  returns the current prioceiling of the mutex.
  </assertion>  
  <assertion id="2" tag="ref:XSH6:33853:33854">
  Upon success, it returns 0, and stores the value of the prioceiling in 'prioceiling'.
  </assertion>
  <assertion id="3" tag="ref:XSH6:33855:33860">
  It MAY fail if:

  [EINVAL] - The priority requested by 'prioceiling' is out of range
  [EINVAL] - 'mutex' doesn't refer to an existing mutex
  [EPERM] - The caller doesn't have the privilege to perform the operation.

  </assertion>
</assertions>
