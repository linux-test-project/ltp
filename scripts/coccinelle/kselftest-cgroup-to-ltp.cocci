@@
expression cgn, cgns;
@@

- cgn = cg_name(..., cgns);
+ cgn = tst_cg_group_mk(cg_test, cgns);

@@
expression cg, fname, data;
@@

- if (cg_write(cg, fname, data)) {
-    ...
- }
+ SAFE_CG_PRINT(cg, fname, data);

@@
expression cg;
@@

... when != TST_CG_VER(...)

- SAFE_CG_PRINT(cg, "cgroup.subtree_control", "+memory");
+ if (TST_CG_VER(cg, "memory") != TST_CG_V1)
+    SAFE_CG_PRINT(cg, "cgroup.subtree_control", "+memory");

@@
expression cg, fname, needle;
@@

- cg_read_strstr(cg, fname, needle)
+ !SAFE_CG_OCCURSIN(cg, fname, needle)

@@
identifier l;
expression cg, fname;
@@

- l = cg_read_long(cg, fname);
+ SAFE_CG_SCANF(cg, fname, "%ld", &l);
