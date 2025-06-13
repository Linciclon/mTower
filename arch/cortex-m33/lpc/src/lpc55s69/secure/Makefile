############################################################################
# arch/cortex-m23/m2351/src/numaker_pfm_m2351/secure/Makefile
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


###### include boot flow files related to BaremetalRuntime Folder
-include ./BaremetalRuntime/setup.mk



OPT_LEVEL = 1
DEBUG_LEVEL = 3

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
LDFLAGS += $(GENERIC_FLAGS) $(ARCH_LDFLAGS) -nostartfiles  # se são genericas significam que são para todas as compilações, desde .s ou .c

LD_FILE:=$(TOPDIR)/arch/cortex-m33/lpc/src/lpc55s69/secure/BaremetalRuntime/src/linker.ld
gen_ld_file:=$(TOPDIR)/linker.ld

$(info O valor de AFLAGS é: $(AFLAGS))
$(info O valor de CFLAGS é: $(CFLAGS))
#$(info O valor de LDFLAGS é: $(LDFLAGS))

######## end of BaremetalRuntime inclusion

CFLAGS += -I../ -I./
CFLAGS += -I$(TOPDIR)/include/mtower
#CFLAGS += -I../../CMSIS/Include
#CFLAGS += -I../../StdDriver/inc
#CFLAGS += -I../../Device/Nuvoton/M2351/Include # NEED ATENTION HERE
CFLAGS += -I$(TOPDIR)/tee_client/public
CFLAGS += -I$(TOPDIR)/tee_client/libteec/include/freertos
CFLAGS += -I$(TOPDIR)/tee/lib/libutee/include
CFLAGS += -I$(TOPDIR)/tee/lib/libutee/trezor-crypto
CFLAGS += -I$(TOPDIR)/tee/lib/libutils/ext/include
CFLAGS += -I$(TOPDIR)/tee/lib/libutils/isoc/include
CFLAGS += -I$(TOPDIR)/tee/lib/libmbedtls/include
CFLAGS += -I$(TOPDIR)/tee/lib/libmbedtls
CFLAGS += -I$(TOPDIR)/tee/lib/libmbedtls/mbedtls/include/mbedtls
CFLAGS += -I$(TOPDIR)/tee/include

ifeq ($(CONFIG_APPS_HELLO_WORLD),y)
CFLAGS += -I$(TOPDIR)/apps/hello_world
endif

ifeq ($(CONFIG_APPS_AES),y)
CFLAGS += -I$(TOPDIR)/apps/aes
endif

ifeq ($(CONFIG_APPS_BW),y)
#Includes
CFLAGS += -I $(TOPDIR)/apps/bw/
CFLAGS += -I $(TOPDIR)/apps/bw/ta/
CFLAGS += -I $(TOPDIR)/apps/bw/ta/include/
CFLAGS += -I $(TOPDIR)/apps/bw/ta/crypto/
CFLAGS += -I $(TOPDIR)/crypto/libtomcrypt/include
$(info I'm compiling BW!!)
endif

ifeq ($(CONFIG_APPS_HOTP),y)
CFLAGS += -I$(TOPDIR)/apps/hotp
endif

ifeq ($(CONFIG_APPS_TEST),y)
CFLAGS += -I$(TOPDIR)/apps/test
endif

CFLAGS += -DDEBUG_PORT=$(subst ",,$(CONFIG_SECURE_DEBUG_UART)) 

OBJDIR = $(TOPDIR)/build/secure$(subst $(TOPDIR),,$(shell pwd))

MTOWER_S = $(TOPDIR)/mtower_s$(EXEEXT)

#collect all ASM files, including those coming from baremetal-runtime 
ASRCS_S = $(ASM_SRC) $(CHIP_ASRCS_S) $(CMN_ASRCS)
AOBJS_S = $(addprefix $(OBJDIR)/, $(ASRCS_S:.S=$(OBJEXT)))

#collect all C files, including those coming from baremetal-runtime 
CSRCS_S = $(C_SRC) $(CHIP_CSRCS_S) $(CMN_CSRCS)
COBJS_S = $(addprefix $(OBJDIR)/, $(CSRCS_S:.c=$(OBJEXT)))

$(info O valor de AFLAGS é: $(AFLAGS))
$(info O valor de CFLAGS é: $(CFLAGS))
#$(info O valor de LDFLAGS é: $(LDFLAGS))

OBJS_S = $(AOBJS_S) $(COBJS_S)

LIBPATHS = -L. -L"$(TOPDIR)/lib"

LIBS =  -lm -lc -lnosys -lalloc_s -lcrypt_s

$(gen_ld_file): $(LD_FILE)
	@echo "Preprocessing Linker Files"
	$(Q) $(CC) $(CPPFLAGS) -E -x assembler-with-cpp $< | grep "^[^#;]" > $@

$(AOBJS_S): $(OBJDIR)/%$(OBJEXT): %.S
	$(Q) mkdir -p $(OBJDIR)/$(dir $<)
	@echo "AS: $<"
	$(Q) $(CC) -c $(AFLAGS) $< -o $@
#	$(Q) $(CC) -c $(AFLAGS) -mcmse -DHeap_Size=0x00001400 -DStack_Size=0x00000B00 $< -o $@ //joao change to what is uncommented - this will make changes in startup_M2351.S, como era para m2351, tirei fora ate porque quem esta a fazer o startup sou eu

$(COBJS_S): $(OBJDIR)/%$(OBJEXT): %.c
	$(Q) mkdir -p $(OBJDIR)/$(dir $<)
	@echo "CC: $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

mtower_s$(EXEEXT): $(OBJS_S) $(gen_ld_file)
#	$(Q) $(MAKE) -C ../../StdDriver/src/ TOPDIR="$(TOPDIR)" WORLD=secure libm2351_StdDriver_s.a # THIS NEEDS ATETION 
	$(Q) $(MAKE) -C $(TOPDIR)/common/ TOPDIR="$(TOPDIR)" WORLD=secure liballoc_s.a
	$(Q) $(MAKE) -C $(TOPDIR)/crypto/ TOPDIR="$(TOPDIR)" WORLD=secure libcrypt_s.a
	@echo "CC: mTower_s$(EXEEXT)"
#	$(Q) $(CC) $(CFLAGS) -mcmse -Wl,--defsym,START_ADDRESS_BL32=$(CONFIG_START_ADDRESS_BL32) -Tsecure.ld $(OBJS_S) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl32.elf //Comentei isto como backup de como estava inicialmente com as flags de quando nao estava sobre uma vm
	$(Q) $(CC) $(CFLAGS) -Wl,--defsym,START_ADDRESS_BL32=$(CONFIG_START_ADDRESS_BL32) $(LDFLAGS) -T$(gen_ld_file) $(OBJS_S) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl32.elf
	@echo "PASS"
	$(Q) $(CC) $(CFLAGS) -Wl,--defsym,START_ADDRESS_BL32=$(CONFIG_START_ADDRESS_BL32) $(LDFLAGS) -T$(gen_ld_file) $(OBJS_S) $(LIBPATHS) $(LIBS)
	@echo "Esta quase"
#	$(Q) $(CC) $(CFLAGS) -mcmse -Wl,--section-start=.text=$(CONFIG_START_ADDRESS_BL32) -Tsecure.ld $(OBJS_S) $(LIBPATHS) $(LIBS) -o $(OBJDIR)/bl32.elf
#	$(Q) $(CC) $(CFLAGS) -Wl,--unresolved-symbols=ignore-all -Wl,--section-start=.gnu.sgstubs=0x3f000 -Wl,--out-implib=$(OBJDIR)/libnsc$(LIBEXT) -Wl,--cmse-implib $(OBJS_S) $(LIBPATHS) $(LIBS)
#	$(Q) $(CC) $(CFLAGS) -Wl,--section-start=.text=$(CONFIG_START_ADDRESS_BL32) -Wl,--unresolved-symbols=ignore-all -Wl,--out-implib=$(OBJDIR)/libnsc$(LIBEXT) -Wl,--cmse-implib -Tsecure.ld $(OBJS_S) $(LIBPATHS) $(LIBS)
	$(Q) rm -f a.out
	$(Q) $(OBJCOPY) -S -O binary $(OBJDIR)/bl32.elf $(OBJDIR)/bl32.bin
	@echo "Vou copiar agr para o Build DIR"
#	$(Q) cp -f $(OBJDIR)/libnsc$(LIBEXT) $(TOPDIR)/lib/
#	@echo "Fiz tudo o que tinah para fazer aqui"



clean:
#	$(Q) $(MAKE) -C ../../NuBL2 TOPDIR="$(TOPDIR)" clean

distclean: clean

