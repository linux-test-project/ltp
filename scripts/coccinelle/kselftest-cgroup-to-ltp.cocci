@@
expression cgn, cgns;
@@

- cgn = cg_name(..., cgns);
+ cgn = tst_cgroup_group_mk(cg_test, cgns);

@@
expression cg, fname, data;
@@

- if (cg_write(cg, fname, data)) {
-    ...
- }
+ SAFE_CGROUP_PRINT(cg, fname, data);

@@
expression cg;
@@

... when != TST_CGROUP_VER(...)

- SAFE_CGROUP_PRINT(cg, "cgroup.subtree_control", "+memory");
+ if (TST_CGROUP_VER(cg, "memory") != TST_CGROUP_V1)
+    SAFE_CGROUP_PRINT(cg, "cgroup.subtree_control", "+memory");

@@
expression cg, fname, needle;
@@

- cg_read_strstr(cg, fname, needle)
+ !SAFE_CGROUP_OCCURSIN(cg, fname, needle)

@@
identifier l;
expression cg, fname;
@@

- l = cg_read_long(cg, fname);
+ SAFE_CGROUP_SCANF(cg, fname, "%ld", &l);
