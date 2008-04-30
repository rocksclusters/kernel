##
## A sample Makefile for the SysKonnect SK-98xx device driver.
##

MODULES := sk98lin

SOURCES :=		\
	skaddr.c	\
	skcsum.c	\
	skdim.c		\
	skethtool.c	\
	skge.c		\
	skgehwt.c	\
	skgeinit.c	\
	skgepnmi.c	\
	skgesirq.c	\
	sklm80.c	\
	skproc.c	\
	skqueue.c	\
	skrlmt.c	\
	sktimer.c	\
	sktwsi.c	\
	skvpd.c		\
	sky2.c	\
	sky2le.c	\
	skxmac2.c

HEADERS :=		\
	skgemib.c	\
	h/lm80.h	\
	h/skaddr.h	\
	h/skcsum.h	\
	h/skdebug.h	\
	h/skdrv1st.h	\
	h/skdrv2nd.h	\
	h/skerror.h	\
	h/skgedrv.h	\
	h/skgehw.h	\
	h/skgehwt.h	\
	h/skgeinit.h	\
	h/skgepnm2.h	\
	h/skgepnmi.h	\
	h/skgesirq.h	\
	h/skgetwsi.h	\
	h/skqueue.h	\
	h/skrlmt.h	\
	h/sktimer.h	\
	h/sktwsi.h	\
	h/sktypes.h	\
	h/skversion.h	\
	h/skvpd.h	\
	h/sky2le.h	\
	h/xmac_ii.h

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

#
# Standalone driver params
# SKPARAM += -DSK_KERNEL_24
# SKPARAM += -DSK_KERNEL_24_26
# SKPARAM += -DSK_KERNEL_26
# SKPARAM += -DSK_KERNEL_22_24

# DBGDEF =  \
# -DDEBUG

ifdef DEBUG
DBGDEF +=  \
-DSK_DEBUG_CHKMOD=0x00000000L \
-DSK_DEBUG_CHKCAT=0x00000000L
endif

# **** possible debug modules for SK_DEBUG_CHKMOD *****************
# SK_DBGMOD_MERR        0x00000001L     /* general module error indication */
# SK_DBGMOD_HWM         0x00000002L     /* Hardware init module */
# SK_DBGMOD_RLMT        0x00000004L     /* RLMT module */
# SK_DBGMOD_VPD         0x00000008L     /* VPD module */
# SK_DBGMOD_I2C         0x00000010L     /* I2C module */
# SK_DBGMOD_PNMI        0x00000020L     /* PNMI module */
# SK_DBGMOD_CSUM        0x00000040L     /* CSUM module */
# SK_DBGMOD_ADDR        0x00000080L     /* ADDR module */
# SK_DBGMOD_DRV         0x00010000L     /* DRV module */

# **** possible debug categories for SK_DEBUG_CHKCAT **************
# *** common modules ***
# SK_DBGCAT_INIT        0x00000001L     module/driver initialization
# SK_DBGCAT_CTRL        0x00000002L     controlling: add/rmv MCA/MAC and other controls (IOCTL)
# SK_DBGCAT_ERR         0x00000004L     error handling paths
# SK_DBGCAT_TX          0x00000008L     transmit path
# SK_DBGCAT_RX          0x00000010L     receive path
# SK_DBGCAT_IRQ         0x00000020L     general IRQ handling
# SK_DBGCAT_QUEUE       0x00000040L     any queue management
# SK_DBGCAT_DUMP        0x00000080L     large data output e.g. hex dump
# SK_DBGCAT_FATAL       0x00000100L     large data output e.g. hex dump

# *** driver (file skge.c) ***
# SK_DBGCAT_DRV_ENTRY           0x00010000      entry points
# SK_DBGCAT_DRV_???             0x00020000      not used
# SK_DBGCAT_DRV_MCA             0x00040000      multicast
# SK_DBGCAT_DRV_TX_PROGRESS     0x00080000      tx path
# SK_DBGCAT_DRV_RX_PROGRESS     0x00100000      rx path
# SK_DBGCAT_DRV_PROGRESS        0x00200000      general runtime
# SK_DBGCAT_DRV_???             0x00400000      not used
# SK_DBGCAT_DRV_PROM            0x00800000      promiscuous mode
# SK_DBGCAT_DRV_TX_FRAME        0x01000000      display tx frames
# SK_DBGCAT_DRV_ERROR           0x02000000      error conditions
# SK_DBGCAT_DRV_INT_SRC         0x04000000      interrupts sources
# SK_DBGCAT_DRV_EVENT           0x08000000      driver events

EXTRA_CFLAGS += -I$(PWD) -DSK_USE_CSUM -DSK_DIAG_SUPPORT \
		-DGENESIS -DYUKON -DYUK2 -DCONFIG_SK98LIN_ZEROCOPY \
		$(DBGDEF) $(SKPARAM)
