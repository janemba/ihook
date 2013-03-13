CC = gcc.exe
AR = ar.exe
LD = ld.exe

INC 	= -I.
CFLAGS  = -m32 -O3 -Wextra -Wall -std=c99
DEP_SRC = dep\\beaengine\\beaengineSources
DEP_INC = -Idep\\beaengine\\include

INC_RELEASE    =  $(INC) $(DEP_INC)
OBJDIR_RELEASE = obj\\Release
DIR_RELEASE    = bin

OBJ_RELEASE  	   = $(OBJDIR_RELEASE)\\hookit.o $(OBJDIR_RELEASE)\\BeaEngine.o
OUT_RELEASE_STATIC = $(DIR_RELEASE)\\libihook.lib
OUT_RELEASE_SHARED = $(DIR_RELEASE)\\libihook.dll

DELTMP = cmd /c del /Q /f libfoo.o

all: ihook

ihook: static shared

before_release:
	cmd /c if not exist $(OBJDIR_RELEASE) md $(OBJDIR_RELEASE)
	cmd /c if not exist $(DIR_RELEASE) md $(DIR_RELEASE)

static: TYPE_RELEASE = -DIHOOK_ENGINE_LIB -DIHOOK_CALL_STDCALL
static: before_release out_static

shared: TYPE_RELEASE = -DIHOOK_ENGINE_DLL
shared: before_release out_shared

out_static: $(OBJ_RELEASE)
	$(LD) -r -o libfoo.o $(OBJ_RELEASE)
	$(AR) rcs $(OUT_RELEASE_STATIC) libfoo.o
	$(DELTMP)

out_shared: $(OBJ_RELEASE)
	$(LD) -r -o libfoo.o $(OBJ_RELEASE)
	$(CC) libfoo.o -s -shared -Wl,--subsystem,windows -o $(OUT_RELEASE_SHARED)
	$(DELTMP)

$(OBJDIR_RELEASE)\\hookit.o: hookit.c $(OBJDIR_RELEASE)\\BeaEngine.o
	$(CC) $(CFLAGS) $(INC_RELEASE) $(TYPE_RELEASE) -c hookit.c -o $(OBJDIR_RELEASE)\\hookit.o

$(OBJDIR_RELEASE)\\BeaEngine.o: $(DEP_SRC)\\BeaEngine.c
	$(CC) $(CFLAGS) $(INC_RELEASE) -DBEA_ENGINE_STATIC -c $(DEP_SRC)\\BeaEngine.c -o $(OBJDIR_RELEASE)\\BeaEngine.o

clean:
	cmd /c del /Q /f $(OBJ_RELEASE) $(OUT_RELEASE_STATIC) $(OUT_RELEASE_SHARED)
	cmd /c rd $(OBJDIR_RELEASE)

.PHONY: before_release out_static out_shared static shared clean

