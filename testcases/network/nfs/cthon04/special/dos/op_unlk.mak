# External Makefile for Microsoft Visual C++ generated build script - Do not modify

PROJ = OP_UNLK
DEBUG = 1
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = 
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = SUBR.C      
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G2 /W0 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /D "DOS" /FR /Fd"OP_UNLK.PDB"
CFLAGS_R_DEXE = /nologo /Gs /G2 /W0 /AM /Ox /D "NDEBUG" /D "_DOS" /FR 
LFLAGS_D_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /CO /STACK:5120
LFLAGS_R_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /STACK:5120
LIBS_D_DEXE = oldnames mlibce
LIBS_R_DEXE = oldnames mlibce
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = OP_UNLK.SBR


SUBR_DEP = ..\tests.h \
	..\unixdos.h


OP_UNLK_DEP = ..\tests.h \
	..\unixdos.h


all:        $(PROJ).EXE $(PROJ).BSC

SUBR.OBJ:   ..\SUBR.C $(SUBR_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\SUBR.C

OP_UNLK.OBJ:            ..\OP_UNLK.C $(OP_UNLK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\OP_UNLK.C

$(PROJ).EXE::           OP_UNLK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
OP_UNLK.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
..\..\lib\+
..\..\include\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
