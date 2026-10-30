# Comandos de reprodução

Executar a partir da raiz do projeto. Antes de qualquer clean, copiar os artefactos existentes para uma diretoria fora de `build/`, como foi feito em `diagnostics/bss-analysis/<plataforma>/preexisting/`.

## Build RZ/T2M

```sh
rm -rf build .config
make PLATFORM=rzt2m create_context
make toolchain
make V=1 2>&1 | tee diagnostics/bss-analysis/rzt2m/build.log
```

Artefactos:

```text
build/secure/arch/cortex-r52/rzt2m/src/rzt2m/secure/bl32.elf
build/secure/arch/cortex-r52/rzt2m/src/rzt2m/secure/bl32.bin
build/secure/arch/cortex-r52/rzt2m/src/rzt2m/secure/bl32.map
build/secure/arch/cortex-r52/rzt2m/src/rzt2m/secure/dump.txt
```

## Build LPC55S69

```sh
rm -rf build .config
make PLATFORM=lpc55s69 create_context
make toolchain
make V=1 2>&1 | tee diagnostics/bss-analysis/lpc55s69/build.log
```

O link gera os quatro artefactos abaixo. O target superior falha depois, no estado atual do projeto, por não existir `$(TOPDIR)/mtower_s.bin`.

```text
build/secure/arch/cortex-m33/lpc/src/lpc55s69/secure/bl32.elf
build/secure/arch/cortex-m33/lpc/src/lpc55s69/secure/bl32.bin
build/secure/arch/cortex-m33/lpc/src/lpc55s69/secure/bl32.map
build/secure/arch/cortex-m33/lpc/src/lpc55s69/secure/dump.txt
```

## Inspeção de um ELF

```sh
arm-none-eabi-size -A bl32.elf > size-A.txt
arm-none-eabi-readelf -SW bl32.elf > readelf-sections.txt
arm-none-eabi-readelf -lW bl32.elf > readelf-program-headers.txt
arm-none-eabi-readelf -sW bl32.elf > readelf-symbols.txt
arm-none-eabi-nm -S -l --size-sort --radix=x bl32.elf > nm-size-sort-lines.txt
arm-none-eabi-objdump -t bl32.elf > objdump-symbols.txt
```

Para confirmar contribuições da `.bss`, usar simultaneamente o índice da secção em `readelf -SW`, o `Ndx` da symbol table, e o intervalo `.bss` do map file. Símbolos marcadores de tamanho zero e aliases não devem ser somados.
