CROSS_COMPILE ?= arm-none-eabi-
ARCH_GENERIC_FLAGS+=-DAARCH32

ARCH_SUB_GENERIC_FLAGS:=-marm -mcpu=cortex-r52 -mfpu=auto -mfloat-abi=hard
