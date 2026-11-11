core_c_srcs:=irq.c retarget.c

ifneq ($(RUN_INSIDE_VM),)
core_c_srcs+=virtio_console.c
endif
