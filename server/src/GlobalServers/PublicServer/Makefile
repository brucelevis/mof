#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.118                       #
#------------------------------------------------------------------------------#


WRKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = 
CFLAGS =  -fshort-wchar -DLINUX_
RESINC = 
LIBDIR = 
LIB = 
LDFLAGS = 

INC_DEBUG =  $(INC) -I../Utils
CFLAGS_DEBUG =  $(CFLAGS) -Wall -g -W -O0 -D_DEBUG -D_CONSOLE
RESINC_DEBUG =  $(RESINC) -I../Utils
RCFLAGS_DEBUG =  $(RCFLAGS)
LIBDIR_DEBUG =  $(LIBDIR) -L../Debug/
LIB_DEBUG = $(LIB) ../Debug/libutils.a -ldl -lpthread -lmysqlcppconn
LDFLAGS_DEBUG =  $(LDFLAGS)
OBJDIR_DEBUG = Debug
DEP_DEBUG = 
OUT_DEBUG = ../Debug/PublicServer

INC_RELEASE =  $(INC) -I../Utils
CFLAGS_RELEASE =  $(CFLAGS) -O2 -g -W -DNDEBUG -D_CONSOLE
RESINC_RELEASE =  $(RESINC) -I../Utils
RCFLAGS_RELEASE =  $(RCFLAGS)
LIBDIR_RELEASE =  $(LIBDIR) -L../Release/
LIB_RELEASE = $(LIB) ../Release/libutils.a -ldl -lpthread -lmysqlcppconn
LDFLAGS_RELEASE =  $(LDFLAGS)
OBJDIR_RELEASE = Release
DEP_RELEASE = 
OUT_RELEASE = ../Release/PublicServer

OBJ_DEBUG = $(OBJDIR_DEBUG)/system.o $(OBJDIR_DEBUG)/stdafx.o $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/RegisterIOProc.o $(OBJDIR_DEBUG)/PlayerManager.o $(OBJDIR_DEBUG)/IoEntry.o $(OBJDIR_DEBUG)/GameConfig.o $(OBJDIR_DEBUG)/Game.o $(OBJDIR_DEBUG)/DataBaseFun.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/system.o $(OBJDIR_RELEASE)/stdafx.o $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/RegisterIOProc.o $(OBJDIR_RELEASE)/PlayerManager.o $(OBJDIR_RELEASE)/IoEntry.o $(OBJDIR_RELEASE)/GameConfig.o $(OBJDIR_RELEASE)/Game.o $(OBJDIR_RELEASE)/DataBaseFun.o

all: Debug Release

clean: clean_Debug clean_Release

before_Debug: 
	test -d ../Debug || mkdir -p ../Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_Debug: 

Debug: before_Debug out_Debug after_Debug

out_Debug: $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) $(LDFLAGS_DEBUG) $(LIBDIR_DEBUG) $(OBJ_DEBUG) $(LIB_DEBUG) -o $(OUT_DEBUG)

$(OBJDIR_DEBUG)/system.o: system.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c system.cpp -o $(OBJDIR_DEBUG)/system.o

$(OBJDIR_DEBUG)/stdafx.o: stdafx.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c stdafx.cpp -o $(OBJDIR_DEBUG)/stdafx.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

$(OBJDIR_DEBUG)/RegisterIOProc.o: RegisterIOProc.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c RegisterIOProc.cpp -o $(OBJDIR_DEBUG)/RegisterIOProc.o

$(OBJDIR_DEBUG)/PlayerManager.o: PlayerManager.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c PlayerManager.cpp -o $(OBJDIR_DEBUG)/PlayerManager.o

$(OBJDIR_DEBUG)/IoEntry.o: IoEntry.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c IoEntry.cpp -o $(OBJDIR_DEBUG)/IoEntry.o

$(OBJDIR_DEBUG)/GameConfig.o: GameConfig.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c GameConfig.cpp -o $(OBJDIR_DEBUG)/GameConfig.o

$(OBJDIR_DEBUG)/Game.o: Game.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c Game.cpp -o $(OBJDIR_DEBUG)/Game.o

$(OBJDIR_DEBUG)/DataBaseFun.o: DataBaseFun.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c DataBaseFun.cpp -o $(OBJDIR_DEBUG)/DataBaseFun.o

clean_Debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf ../Debug
	rm -rf $(OBJDIR_DEBUG)

before_Release: 
	test -d ../Release || mkdir -p ../Release
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_Release: 

Release: before_Release out_Release after_Release

out_Release: $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) $(LDFLAGS_RELEASE) $(LIBDIR_RELEASE) $(OBJ_RELEASE) $(LIB_RELEASE) -o $(OUT_RELEASE)

$(OBJDIR_RELEASE)/system.o: system.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c system.cpp -o $(OBJDIR_RELEASE)/system.o

$(OBJDIR_RELEASE)/stdafx.o: stdafx.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c stdafx.cpp -o $(OBJDIR_RELEASE)/stdafx.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/RegisterIOProc.o: RegisterIOProc.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c RegisterIOProc.cpp -o $(OBJDIR_RELEASE)/RegisterIOProc.o

$(OBJDIR_RELEASE)/PlayerManager.o: PlayerManager.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c PlayerManager.cpp -o $(OBJDIR_RELEASE)/PlayerManager.o

$(OBJDIR_RELEASE)/IoEntry.o: IoEntry.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c IoEntry.cpp -o $(OBJDIR_RELEASE)/IoEntry.o

$(OBJDIR_RELEASE)/GameConfig.o: GameConfig.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c GameConfig.cpp -o $(OBJDIR_RELEASE)/GameConfig.o

$(OBJDIR_RELEASE)/Game.o: Game.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c Game.cpp -o $(OBJDIR_RELEASE)/Game.o

$(OBJDIR_RELEASE)/DataBaseFun.o: DataBaseFun.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c DataBaseFun.cpp -o $(OBJDIR_RELEASE)/DataBaseFun.o

clean_Release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf ../Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_Debug after_Debug clean_Debug before_Release after_Release clean_Release

