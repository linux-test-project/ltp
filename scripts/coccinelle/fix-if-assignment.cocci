@@
expression V, E;
@@

+ V = E;
  if (
-	(V = E)
+ 	V
  ) { ... }

@@
expression V, E;
@@

+ V = E;
  if (!
-	(V = E)
+ 	V
  ) { ... }

@@
expression V, E;
binary operator B; 
@@

+ V = E;
  if (
-	(V = E)
+ 	V
  B ...) { ... }
