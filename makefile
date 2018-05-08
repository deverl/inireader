
PLATFORM := $(shell uname)

CPP := g++-8

CPP_FLAGS = -c -Wall -pedantic --std=c++11


ifdef DEBUG
    CPP_FLAGS += -g3 -O0 -DDEBUG -D_DEBUG
    OBJDIR := $(PLATFORM)_objd
else
    CPP_FLAGS += -O3 -DNDEBUG -DRELEASE
    OBJDIR := $(PLATFORM)_objn
endif

ifeq ($(PLATFORM),Linux)
    CPP_FLAGS += -DLINUX -D_LINUX -D__LINUX__
endif

.DEFAULT : all

all : $(OBJDIR)/inireader

.PHONY : clean test install


dep : $(DEP_FILES)

-include $(OBJ_FILES:.o=.d)

CPP_SRC_FILES := inireader.cpp 

OBJ_LIST := $(CPP_SRC_FILES:.cpp=.o) $(C_SRC_FILES:.c=.o)
OBJ_FILES := $(addprefix $(OBJDIR)/, $(OBJ_LIST))
DEP_FILES := $(OBJ_FILES:.o=.d)


$(OBJDIR)/inireader : $(OBJ_FILES) makefile
	@if [ ! -d $(@D) ] ; then mkdir -p $(@D) ; fi
	@echo "Linking $@"
	@$(CPP) -o $@ $(OBJ_FILES)

$(OBJDIR)/%.o : %.cpp makefile $(OBJDIR)/%.d
	@if [ ! -d $(@D) ] ; then mkdir -p $(@D) ; fi
	@echo "Compiling $<"
	@$(CPP) $(CPP_FLAGS) -o $@ $<

$(OBJDIR)/%.d : %.cpp makefile
	@if [ ! -d $(@D) ] ; then mkdir -p $(@D) ; fi
	@echo "Generating dependencies for $<"
	@$(CPP) $(CPP_FLAGS) -MM -MT $@ $< > $(@:.o=.d)


clean:
	rm -rf inireader *.o inireader.dSYM $(OBJDIR)


test: $(OBJDIR)/inireader
	$(OBJDIR)/inireader sample.ini  CLIENT   phone


# Currently only works on the mac platform.
install: $(OBJDIR)/inireader
	cp $(OBJDIR)/inireader ~/nexus/bin/mac



