##
## Another sample Makefile for building modules for driver disks
##

MODULES := e1000

SOURCES := \
	e1000_ethtool.c \
	e1000_hw.c \
	e1000_main.c \
	e1000_param.c \
	kcompat.c \
	kcompat_ethtool.c

HEADERS := \
	e1000.h \
	e1000_hw.h \
	e1000_osdep.h \
	kcompat.h

define MODULE_template
	# If $(1)_SOURCES is not defined, presume single module...
	ifeq ($(words $($(1)_SOURCES)),0)
		# If multiple SOURCES...
		ifneq ($(words $(SOURCES)),1)
			# Define $(1)-objs
			$(1)-objs := $(subst .c,.o,$(SOURCES))
		endif
	else
		# Append $(1)_SOURCES to SOURCES
		SOURCES += $$($(1)_SOURCES)

		# If multiple $(1)_SOURCES...
		ifneq ($(words $($(1)_SOURCES)),1)
			# Define $(1)-objs
			$(1)-objs := $(subst .c,.o,$($(1)_SOURCES))
		else
			# If $(1)_SOURCES does not match module name...
			ifneq ($(patsubst %,%.c,$(1)),$($(1)_SOURCES))
				# Define $(1)-objs
				$(1)-objs := $(subst .c,.o,$($(1)_SOURCES))
			endif
		endif
	endif

	# Add module to obj-m
	obj-m += $(patsubst %,%.o,$(1))
endef

$(foreach module,$(MODULES),$(eval $(call MODULE_template,$(module))))

EXTRA_CFLAGS += -I$(PWD)
