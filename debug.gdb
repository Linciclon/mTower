target remote :3333
file /home/mjs/CROSSCON/TEEsinMCUs/CROSSCON-Hypervisor/bin/lpc55s69/pervmtee-m/crossconhyp.elf
add-symbol-file /home/mjs/CROSSCON/TEEsinMCUs/CROSSCON_FirstTaks_BW+mtower+Nuvoton/mTower/build/secure/arch/cortex-m33/lpc/src/lpc55s69/secure/bl32.elf
add-symbol-file /home/mjs/CROSSCON/TEEsinMCUs/freertos-over-bao/build/lpc55s69/freertos.elf
set $pc=_reset_handler
set mem inaccessible-by-default off
tui enable
b ioctl
b tee_ioctl_open_session
c

