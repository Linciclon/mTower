############################################################################
# arch/cortex-m23/m2351/src/numaker_pfm_m2351/nonsecure/Makefile
#
# Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved.
# Author: Taras Drozdovskyi t.drozdovsky@samsung.com
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

-include $(TOPDIR)/Make.defs
-include $(TOPDIR)/.config
-include Make.defs

PLATFORM=lpc55s69
CFLAGS += -DPLATFORM_LPC55S69 # This is to remove printfs.h includes
NAME = freertos

###### include boot flow files related to BaremetalRuntime Folder
-include ./BaremetalRuntime/setup.mk

OPT_LEVEL = 1
DEBUG_LEVEL = 3

#### arch files for freertos
# Freertos port arch-specific
freertosarch_dir=$(TOPDIR)/arch/cortex-m33/lpc/src/lpc55s69/nonsecure/arch/
SRC_DIRS+=$(freertosarch_dir)
INC_DIRS+=$(freertosarch_dir)/inc
-include $(freertosarch_dir)/arch.mk
-include $(freertosarch_dir)/sources.mk
C_SRC+=$(addprefix $(freertosarch_dir)/, $(arch_c_srcs))
ASM_SRC+=$(addprefix $(freertosarch_dir)/, $(arch_s_srcs))
####

debug_flags:= -g$(DEBUG_LEVEL) $(arch_debug_flags) $(platform_debug_flags)
GENERIC_FLAGS = $(ARCH_GENERIC_FLAGS) -O$(OPT_LEVEL) $(debug_flags) -static
#Collect include files from setup.mk and add "-I" 
CPPFLAGS += $(ARCH_CPPFLAGS) $(addprefix -I, $(INC_DIRS)) -MD -MF $@.d

$(info O valor de CPPFLAGS é: $(CPPFLAGS))

ifneq ($(STD_ADDR_SPACE),)
CPPFLAGS+=-DSTD_ADDR_SPACE
endif
ifneq ($(MPU),)
CPPFLAGS+=-DMPU
endif
ifneq ($(MEM_BASE),)
CPPFLAGS+=-DMEM_BASE=$(MEM_BASE)
endif
ifneq ($(MEM_SIZE),)
CPPFLAGS+=-DMEM_SIZE=$(MEM_SIZE)
endif
ifneq ($(SINGLE_CORE),)
CPPFLAGS+=-DSINGLE_CORE=y
endif
ifneq ($(NO_FIRMWARE),)
CPPFLAGS+=-DNO_FIRMWARE=y
endif
AFLAGS += $(GENERIC_FLAGS) $(CPPFLAGS) $(ARCH_ASFLAGS) 
CFLAGS += $(GENERIC_FLAGS) $(CPPFLAGS) $(ARCH_CFLAGS) 
LDFLAGS += $(GENERIC_FLAGS) $(ARCH_LDFLAGS) -nostartfiles

LD_FILE:=$(TOPDIR)/arch/cortex-m33/lpc/src/lpc55s69/nonsecure/BaremetalRuntime/src/linker.ld
gen_ld_file:=$(TOPDIR)/linker.ld

######## end of BaremetalRuntime inclusion

CFLAGS += -I../ -I./	
CFLAGS += -I$(TOPDIR)/arch/cortex-m33/lpc/src/CMSIS/inc
CFLAGS += -I$(TOPDIR)/arch/cortex-m33/lpc/src/device/inc
CFLAGS += -I$(TOPDIR)/include/mtower
CFLAGS += -I$(TOPDIR)/freertos/include
CFLAGS += -I$(TOPDIR)/freertos/portable/GCC/ARM_V8M
CFLAGS += -I$(TOPDIR)/tee_client/public
CFLAGS += -I$(TOPDIR)/tee_client/libteec/include/freertos

$(info O valor de AFLAGS é: $(AFLAGS))
$(info O valor de CFLAGS é: $(CFLAGS))

ifeq ($(CONFIG_APPS_HELLO_WORLD),y)
CFLAGS += -I$(TOPDIR)/apps/hello_world
endif

ifeq ($(CONFIG_APPS_AES),y)
CFLAGS += -I$(TOPDIR)/apps/aes
endif

ifeq ($(CONFIG_APPS_BW),y)
CFLAGS += -I $(TOPDIR)/apps/bw/ca/include/
CFLAGS += -I $(TOPDIR)/apps/bw/
endif

ifeq ($(CONFIG_APPS_HOTP),y)
CFLAGS += -I$(TOPDIR)/apps/hotp
endif

ifeq ($(CONFIG_APPS_TEST),y)
CFLAGS += -I$(TOPDIR)/apps/test
endif

CFLAGS += -DDEBUG_PORT=$(subst ",,$(CONFIG_NONSECURE_DEBUG_UART))

CFLAGS += -g 

OBJDIR = $(TOPDIR)/build/nonsecure$(subst $(TOPDIR),,$(shell pwd))

MTOWER_NS = $(TOPDIR)/mtower_ns$(EXEEXT)

#collect all ASM files, including those coming from baremetal-runtime 
ASRCS_NS = $(ASM_SRC) $(CHIP_ASRCS_NS) $(CMN_ASRCS)
AOBJS_NS = $(addprefix $(OBJDIR)/, $(ASRCS_NS:.S=$(OBJEXT)))

#collect all C files, including those coming from baremetal-runtime 
CSRCS_NS = $(C_SRC) $(CHIP_CSRCS_NS) $(CMN_CSRCS)
COBJS_NS = $(addprefix $(OBJDIR)/, $(CSRCS_NS:.c=$(OBJEXT)))

OBJS_NS = $(AOBJS_NS) $(COBJS_NS)

LIBPATHS = -L. -L"$(TOPDIR)/lib"

#LIBS =  -lm -lc -lnosys -lm2351_StdDriver_ns -lnsc -lFreeRTOS_ns
LIBS =  -lm -lc -lnosys -lnsc -lFreeRTOS_ns

$(gen_ld_file): $(LD_FILE)
	@echo "Preprocessing Linker Files"
	$(Q) $(CC) $(CPPFLAGS) -E -x assembler-with-cpp $< | grep "^[^#;]" > $@

$(AOBJS_NS): $(OBJDIR)/%$(OBJEXT): %.S
	$(Q) mkdir -p $(OBJDIR)/$(dir $<)
	@echo "AS: $<"
	$(Q) $(CC) -c $(AFLAGS) -DHeap_Size=0x00000500 -DStack_Size=0x00000B00 $< -o $@ 
#here was heap = 0x700

$(COBJS_NS): $(OBJDIR)/%$(OBJEXT): %.c
	$(Q) mkdir -p $(OBJDIR)/$(dir $<)
	@echo "CC: $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

mtower_ns$(EXEEXT): $(OBJS_NS) $(gen_ld_file)
	$(Q) $(MAKE) -C $(TOPDIR)/freertos/ TOPDIR="$(TOPDIR)" FREE_RTOS_ARCH=ARM_V8M libFreeRTOS_ns.a
#	$(Q) $(MAKE) -C ../../StdDriver/src/ TOPDIR="$(TOPDIR)" libm2351_StdDriver_ns.a
	@echo "LD: mTower_ns$(EXEEXT)"
#joao from $(Q) $(CC) $(CFLAGS) -Wl,--defsym,START_ADDRESS_BL33=$(CONFIG_START_ADDRESS_BL33) -Tnonsecure.ld $(OBJS_NS) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl33.elf
	$(Q) $(CC) $(CFLAGS) -Wl,--defsym,START_ADDRESS_BL33=$(CONFIG_START_ADDRESS_BL33) $(LDFLAGS) -T$(gen_ld_file) $(OBJS_NS) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl33.elf
#	$(Q) $(CC) $(CFLAGS) -Wl,--section-start=.text=$(CONFIG_START_ADDRESS_BL33) -Tnonsecure.ld $(OBJS_NS) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl33.elf
	$(Q) $(OBJCOPY) -S -O binary $(OBJDIR)/bl33.elf $(OBJDIR)/bl33.bin

clean:
	$(Q) rm -f ../../StdDriver/src/*$(OBJEXT) 

distclean: clean

