virtual fix

@@
expression cg, ctrl;
@@

- TST_CG_VER(cg, ctrl) == TST_CG_V1
+ TST_CG_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CG_VER(cg, ctrl) != TST_CG_V1
+ !TST_CG_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CG_VER(cg, ctrl) == TST_CG_V2
+ !TST_CG_VER_IS_V1(cg, ctrl)

@@
expression cg, ctrl;
@@

- TST_CG_VER(cg, ctrl) != TST_CG_V2
+ TST_CG_VER_IS_V1(cg, ctrl)
