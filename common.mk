# Make file to include in recursive build framework

MAKEDEPEND	:=	.makedepend

SRCS	:=
OBJS	:=

# Translate to uppercase
# $(1)	String to translate
define uppercase
$(shell echo '$(1)' | tr '[a-z]' '[A-Z]')
endef

# install_file - install a file in a directory
# $(1)	Arguments to install
# $(2)	Name of file
# $(3)	Name of directory
define install_file
$(3)/$(notdir $(2)): $(2)
	@echo installing $(2) in $$@
	install $(1) -D $(2) $$@
endef

# install_setuid_file - install a file in a directory
# $(1)	Arguments to install
# $(2)	Name of file
# $(3)	Name of directory
define install_setuid_file
$(3)/$(notdir $(2)): $(2)
	@echo installing SETUID $(2) in $$@
	install $(1) -D $(2) $$@
	chmod u+s $$(@)
endef

# install_files - install a list of files in a directory. The list of files
#	can be in a mixed back of directories but will all wind up in the
#	same directory.
# $(1)	Make target that will be used to make the files
# $(2)	Arguments to install
# $(3)	List of files to be installed
# $(4)	List of files to be installed as setuid
# $(5)	Destination directory
define install_files
$(1): $(foreach file,$(3) $(4),$(5)/$(notdir $(strip $(file))))

$(foreach file,$(3),$(eval $(call install_file,$(2),$(strip $(file)),$(5))))
$(foreach file,$(4),\
	$(eval $(call install_setuid_file,$(2),$(strip $(file)),$(5))))
endef

# Generate a simple rule where a target depends upon a dependency
# $(1)	List of target files
# $(2)	List of dependency files
define gen_simple_rule
$(join $(addsuffix :,$(1)),$(2))$(call eol)
endef

# Define rules where one file depends on another file, like a.o depends upon
# a.c
# $(1)	List of target files
# $(2)	List of dependency files
define simple_rule
$(foreach rule,$(call gen_simple_rule,$(1),$(2)),$(eval $(rule)))
endef

# Do work in a subdirectory
# $(1)	Name of subdirectory
define subdir_work
srcdir	:= $(1)
outdir	:= $(1)
this	:= $(subst /,_,$(1))
# It would seem that this could be simplified by using $(this), but it's not
# clear why this doesn't work
THIS	:= $(call uppercase,$(subst /,_,$(1)))

$(info including $(1)/$(notdir $(1)).mk)
include $(1)/$(notdir $(1)).mk

SRCS	+= $$($$(THIS)_SRCS)
OBJS	+= $$($$(THIS)_OBJS)
THIS	:=
endef

# Do work for all subdirectories
# $(1)	List of subdirectories
define subdirs_work
$(foreach subdir,$(1),$(eval $(call subdir_work,$(subdir))))
endef

# Generate makedepend output file
# $(1)	Name of output file
# $(2)	List of files to be processed. This may be a combination of header and
#	C files. The header file names allow explicity specification but
#	only the C files are passed to makedepend.
define gen_makedepend
$(1): $(2)
	@echo -------------------- start makedepend
	@echo Dependencies: $$(filter-out %.h,$$^)
	@echo --------------------
	touch $$@
	makedepend -f $$@ -- $$(CFLAGS) $$(addprefix -I,$$(PLAT_INCLUDES)) \
		$$(filter-out %.h,$$^)
	@echo -------------------- end makedepend
endef

# Provide path names for shared libraries. This can be used in target
# dependency lists
# $1	List of library base names. So, libxyz.so should be xyz
define libso_paths
$(addsuffix .so,$(addprefix $(_COMMON_BUILD_LIB_DIR)/lib,$(1)))
endef

# Provide path names for static libraries. This can be used in target
# dependency lists
# $1	List of library base names. So, libxyz.a should be xyz
define liba_paths
$(addsuffix .a,$(addprefix $(_COMMON_BUILD_LIB_DIR)/lib,$(1)))
endef

# Provide the flags for linking with shared libraries. These are for
# supplying on a command line.
# $1	List of library base names. So, libxyz.so should be xyz
define libso_flags
$(addprefix -l,$(1))
endef

# Provide the flags for linking with statically linked libraries. These are
# for supplying on a command line.
# $1	List of library base names. So, libxyz.a should be xyz
define liba_flags
$(addprefix -l,$(1))
endef

%.i: %.c
	$(CC) $(CFLAGS) -P -o $^ $@

all:	_all

# provide a dependency file to make a directory
# $(1)	Directory path
define dirdirdep
$(1)/.stampdir
endef

# Provide a dependency file to make a directory in which a file is to be stored
# $(1)	File path
define dirdep
$(call dirdirdep,$(dir $(1)))
endef

# Print a note in the output using underlines
# $(1)	Note to print
define print_note_ul
$(info $(shell echo '$(1)' | sed -e 's/./_/g' -e 's/.*/ _&_/')) \
$(info $(shell echo '$(1)' | sed -e 's/./ /g' -e 's/.*/| & |/')) \
$(info $(shell echo '$(1)' | sed -e 's/.*/| & |/')) \
$(info $(shell echo '$(1)' | sed -e 's/./_/g' -e 's/.*/|_&_|/'))
endef

# Print a note in the output using dashes
# $(1)	Note to print
define print_note_dash
$(info $(shell echo '$(1)' | sed -e 's/./-/g' -e 's/.*/ -&-/')) \
$(info $(shell echo '$(1)' | sed -e 's/.*/| & |/')) \
$(info $(shell echo '$(1)' | sed -e 's/./-/g' -e 's/.*/ -&-/'))
endef

# Print a note in the output using underlines
# $(1)	Note to print
define print_note
$(call print_note_dash,$(1))
endef

# Define things for using the Linux kernel configurator. It would be nice to
# do this as a macro but it seems that, because we want this for making the
# makedepend target, the expansion doesn't happen at the right time. So,
# you need to define several variables before including this file (or use the
# defaults):
# $(1)	Root of tree of configurator input files
# $(2)	Path to C header file
# $(3)	Path to default configuration file
# $(4)	Output file name. There is probably no need to ever use anything other
#	than .config.
define __use_kconfig
menuconfig: $(4)
	CONFIG_=CONFIG_ \
		kconfig-mconf $(1)

xconfig: $(4)
	CONFIG_=CONFIG_ \
		kconfig-qconf $(1)

oldconfig: $(4)
	CONFIG_=CONFIG_ \
		kconfig-conf --silentoldconfig $(1)

savedefconfig: $(4)
	CONFIG_=CONFIG_ \
		kconfig-conf --savedefconfig defconfig $(1)

$(4): $(3)
	cp $$^ $$@

$(2): $(4)
	mkdir -p $$(@D)
	sed	-e '/^[ 	]*#/d' \
		-e 's/\([A-Za-z_][A-Za-z_0-9]*\)=\(.*\)/#define \1	\2/' \
		$(4) >$$@

.PHONY: menuconfig xconfig
endef

# Define things for using the Linux kernel configurator. It would be nice to
# do this as a macro but it seems that, because we want this for making the
# makedepend target, the expansion doesn't happen at the right time. So,
# you need to define several variables before including this file (or use the
# defaults):
# FIXME: clean this up
#			set this differently from the default, which is .config
# $(1)	Path to directory containing configurator binaries
# $(2)	Root of tree of configurator input files
# $(3)	Path to C header file
# $(4)	Path to default configuration file
define use_kconfig
$(eval $(call __use_kconfig,$(1),$(2),$(3),.config))
endef
