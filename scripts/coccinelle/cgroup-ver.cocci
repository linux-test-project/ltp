virtual fix

@@
expression cg, ctrl;
@@

- TST_CGROUP_VER(cg, ctrl) == TST_CGROUP_V1
+ TST_CGROUP_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CGROUP_VER(cg, ctrl) != TST_CGROUP_V1
+ !TST_CGROUP_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CGROUP_VER(cg, ctrl) == TST_CGROUP_V2
+ !TST_CGROUP_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CGROUP_VER(cg, ctrl) != TST_CGROUP_V2
+ TST_CGROUP_VER_IS_V1(cg, ctrl)
