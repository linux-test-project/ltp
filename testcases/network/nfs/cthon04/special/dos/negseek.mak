# External Makefile for Microsoft Visual C++ generated build script - Do not modify

PROJ = NEGSEEK
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
CFLAGS_D_DEXE = /nologo /G2 /W0 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /D "DOS" /FR /Fd"NEGSEEK.PDB"
CFLAGS_R_DEXE = /nologo /Gs /G2 /W0 /AM /Ox /D "NDEBUG" /D "_DOS" /FR 
LFLAGS_D_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /CO /STACK:10240
LFLAGS_R_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /STACK:10240
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
SBRS = NEGSEEK.SBR


SUBR_DEP = ..\tests.h \
	..\unixdos.h


NEGSEEK_DEP = ..\tests.h \
	..\unixdos.h


all:	$(PROJ).EXE $(PROJ).BSC

SUBR.OBJ:	..\SUBR.C $(SUBR_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\SUBR.C

NEGSEEK.OBJ:	..\NEGSEEK.C $(NEGSEEK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\NEGSEEK.C

$(PROJ).EXE::	NEGSEEK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
NEGSEEK.OBJ +
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
