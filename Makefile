# We try to use relative paths, using the Make abspath function when we need
# and absolute path
OUT_DIR	 = 	out
GEN_INCLUDE =	$(OUT_DIR)/gen-include
LIB_DIR =	$(OUT_DIR)/lib
TEST_BIN =	$(OUT_DIR)/bin
TOOLS_DIR =	$(OUT_DIR)/tools
TOOLS_BIN =	$(TOOLS_DIR)/bin

_COMMON_BUILD_LIB_DIR = $(LIB_DIR)
include common.mk

srcdir := .
outdir := .

override CFLAGS +=	-Wall -Wextra -Werror
override CFLAGS +=	-Wno-unused-parameter	# Quiet about unused parameters
override CFLAGS +=	-Wno-address		# Quiet about potential address issues
override CFLAGS +=	-g -ggdb
override CFLAGS +=	-fPIC
override CFLAGS +=	-I.
override CFLAGS +=	-Wl,-rpath -Wl,$(LIB_DIR)

override LDFLAGS +=	-L$(LIB_DIR)

LIBDBG_SRCS	:= $(addprefix $(srcdir)/,\
	dbg.c)

LIBDBG_OBJS	:= $(patsubst $(srcdir)/%.c,$(outdir)/%.o, $(LIBDBG_SRCS))

$(call libso_paths,dbg): $(LIBDBG_OBJS)
	$(LD) -shared \
		-soname libdbg.so \
	       	$^ $(LDFLAGS) \
		-o $@

$(call liba_paths,dbg): $(LIBDBG_OBJS)
	$(AR) r $@ $^

# Configuration things

SRCS += $(LIBDBG_SRCS)
OBJS += $(LIBDBG_OBJS)

# The dependencies are those of components that are not part of the included
# directories, i.e. things that come from open source components
#
# This can get hung in an infinite loop. Capture a chunk of the output of
# make -d, then grep for "Must remake". There will be some set of these that
# repeat. See which ones don't exist, but which should. Fix the problems.
$(MAKEDEPEND): $(MAKEDEPEND_DEPENDS)

$(eval $(call gen_makedepend,$(MAKEDEPEND),$(SRCS)))

$(info Ignore messages like: ".makedepend: No such file or directory")
include $(MAKEDEPEND)

clean::
	rm -f $(OBJS)
	rm -rf $(OUT_DIR)

clobber:: clean
	rm -f .makedepend .makedepend.bak

TARGETS	+= $(LIB_DIR)/libdbg.a \
	$(LIB_DIR)/libdbg.so

TARGET_DIRS = $(sort $(dir $(TARGETS)))

SRCS += $(LIBDBG_SRCS)

# Create a directory
# $(1)	Path of directory to create
define mk_target_dir
$(1):
	mkdir -p $$@
endef

$(foreach dir,$(TARGET_DIRS),$(eval $(call mk_target_dir,$(dir))))

LIBS += \
	$(LIB_DIR)/libdbg.a \
	$(LIB_DIR)/libdbg.so \

HEADERS += \
	   include/dbg.h

# Create a directory
# $(1)	Path of directory to create
define mk_target_dir
$(1):
	mkdir -p $$@
endef

$(foreach dir,$(TARGET_DIRS),$(eval $(call mk_target_dir,$(dir))))

dirs: $(TARGET_DIRS)

_all:	dirs $(TARGETS)

$(call install_files,staging, , \
	$(HEADERS), , \
	$(STAGING_DIR)/usr/include)

$(call install_files,staging, , \
	$(LIBS), \
	$(STAGING_DIR)/usr/lib)

$(call install_files,install, , \
	$(LIB_DIR)/libdbg.so, \
	$(TARGET_DIR)/usr/lib)

$(call install_files,host-install, , \
	$(HEADERS), , \
	$(HOST_DIR)/usr/include)

$(call install_files,host-install, , \
	$(LIBS), , \
	$(HOST_DIR)/usr/lib)
