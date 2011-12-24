#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
BUILD		:=	release_build
MODULES     := . 
SOURCES		:=	$(MODULES)
INCLUDES	:=	../include ../include/modes ../include/books ../../libogg-1.3.0/include
LIBDIR		:=	../lib

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS		:=	-Os -Wall $(MACHDEP) $(INCLUDE) -DWORDS_BIGENDIAN
CXXFLAGS	:=	$(CFLAGS)
export EXT2BIN	:=	$(LIBDIR)/$(PLATFORM)/libvorbis.a


#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS		:=

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS		:=

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))


export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
			$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
			-L$(LIBOGC_LIB)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(LIBDIR)

install:
	cp ../include/vorbis/vorbisenc.h $(DEVKITPRO)/libogc/include/vorbisenc.h
	cp ../include/vorbis/vorbisfile.h $(DEVKITPRO)/libogc/include/vorbisfile.h
	cp ../lib/libvorbis.a $(DEVKITPRO)/libogc/lib/wii

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(EXT2BIN): $(OFILES) $(LIBDIR)/$(PLATFORM)
	@rm -f "../$(EXT2BIN)"
	@$(AR) rcs "../$(EXT2BIN)" $(OFILES)
	@echo built ... $(notdir $@)

$(LIBDIR)/$(PLATFORM):
	mkdir -p ../$(LIBDIR)/$(PLATFORM)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------

