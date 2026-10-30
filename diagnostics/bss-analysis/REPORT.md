# Diagnóstico de RAM e `.bss` — RZ/T2M e LPC55S69

Data da análise: 2026-10-28. Os tamanhos em KiB usam 1 KiB = 1024 bytes.

## 1. Conclusão executiva

Factos confirmados pelos ELF e map files gerados:

- A `.bss` da RZ/T2M mede **22 176 B (21,656 KiB)**. Não contém a stack.
- Os dois responsáveis dominantes são `ucHeap` (**16 384 B; 73,882%**) e `irq_handlers` (**4096 B; 18,470%**). Juntos representam **92,352%** da `.bss`.
- A `.bss` da LPC55S69 mede **18 424 B (17,992 KiB)**. A diferença de **3752 B** para a RZ/T2M é explicada quase integralmente por `irq_handlers`: 1024 ponteiros na RZ/T2M contra 91 na LPC.
- Na RZ/T2M existe ainda uma secção `.stack` NOBITS de **65 544 B**: 8 B de alinhamento e quatro stacks de 16 KiB (SVC e IRQ para cada um de dois cores). A stack é maior do que a `.bss`.
- Na LPC, o script não cria uma secção `.stack`: avança apenas o location counter 8 B para alinhamento e mais **32 768 B**. `size -A` e o `PT_LOAD` de RAM não mostram esta reserva.
- A RZ/T2M usa memória unificada: código, constantes, `.data`, `.bss` e stacks ocupam a mesma região de 512 KiB. O segmento LOAD reserva **328 640 B (320,938 KiB)**, dos quais **240 916 B (235,270 KiB)** estão na imagem e o restante é NOBITS/alinhamento.
- Na LPC, código e constantes ficam em ROM e `.data` é copiada para SRAM. A RAM estaticamente comprometida até `_stack_top` é **53 216 B (51,969 KiB)** de uma região de 64 KiB, deixando **12 320 B (12,031 KiB)** depois de `_heap_base`.
- Não existem secções `.noinit` nos dois ELF. Não existem contribuições `COMMON` na `.bss` final.
- Há dois sistemas de alocação: o heap estático FreeRTOS `heap_4` (`ucHeap`, 16 KiB) usado por `TEE_Malloc`, e o malloc da newlib, que cresce a partir de `_heap_base` por `_sbrk`. `_sbrk` não verifica o limite superior da RAM.

As melhores oportunidades, ainda não implementadas, são:

1. medir e redimensionar as quatro stacks da RZ/T2M (poupança condicional de 32 KiB se cada uma puder passar de 16 para 8 KiB);
2. medir o pico/fragmentação de `ucHeap` (até 8 KiB em cada plataforma se for demonstrado que 8 KiB é suficiente);
3. substituir o limite genérico de 1024 entradas de `irq_handlers` por um limite específico e comprovado da RZ/T2M (cerca de 2,7 KiB no cenário de IDs 0..337);
4. dimensionar separadamente as tabelas de descritores de cifra, hash e PRNG (388 B em cada plataforma para a configuração atual).

Nenhuma destas reduções foi implementada nesta fase.

## 2. Builds, toolchain e artefactos

### Comandos identificados

RZ/T2M:

```sh
rm -rf build .config
make PLATFORM=rzt2m create_context
make toolchain
make
```

LPC55S69, confirmado por `configs/lpc55s69/defconfig`, `CONFIG_PLATFORM="lpc55s69"` e pelos Makefiles:

```sh
rm -rf build .config
make PLATFORM=lpc55s69 create_context
make toolchain
make
```

O prefixo efetivo é `arm-none-eabi-`. Embora os defconfigs declarem GCC `arm-none-eabi-6-2017-q2-update-linux`, `configs/*/Make.defs` define `CROSSDEV = arm-none-eabi-`, usando o `PATH`. A execução usou:

```text
arm-none-eabi-gcc (Arm GNU Toolchain 13.3.Rel1) 13.3.1 20240614
```

### Flags relevantes

Comuns: GNU99, `-O1`, `-g3`, `-static`, `-ffunction-sections`, `-fdata-sections`, link com `--gc-sections` e `-nostartfiles`.

- RZ/T2M: `-mcpu=cortex-r52 -marm -mlittle-endian -mfpu=auto -mfloat-abi=hard`, `AARCH32`, GICv3 e `-mgeneral-regs-only` para C do runtime.
- LPC55S69: `-mcpu=cortex-m33 -mthumb`, com `-march=armv8-m.base` vindo de `Make.defs` e `-march=armv8-m.main -mfpu=fpv5-sp-d16 -mfloat-abi=soft` vindo do runtime. GCC avisa que as opções `-mcpu`/`-march` conflituam; é preexistente.

Os dois builds têm `CONFIG_TOTAL_HEAP_SIZE=0x4000` e `CONFIG_APPS_BW=y`.

### Linker scripts

- RZ/T2M: `arch/cortex-r52/rzt2m/src/rzt2m/secure/BaremetalRuntime/src/linker.ld`, preprocessado para o diretório do build. Região única `RAM` em `0x10100000`, comprimento `0x80000`.
- LPC55S69: `arch/cortex-m33/lpc/src/lpc55s69/secure/BaremetalRuntime/src/linker.ld`, preprocessado como `linker.ld` na raiz. `ROM` em `0x00010000`, comprimento `0x35000`; `RAM` em `0x20010000`, comprimento `0x10000`.

### Resultado dos builds

- RZ/T2M: build completo com sucesso. O ELF reconstruído é byte a byte igual ao ELF preexistente (mesmo SHA-256), logo a geração de map/dump não alterou o binário.
- LPC55S69: compilação e link de `bl32.elf` concluídos; falha posterior no Makefile da board porque o Makefile secure não copia `bl32.bin` para `$(TOPDIR)/mtower_s.bin`. Esta falha é preexistente e independente da geração de map/dump.

Inconsistências preexistentes adicionais:

- Na LPC, `CONFIG_START_ADDRESS_BL32=0x8000`, mas o ELF tem VMA inicial `0x10000`, imposta por `PLAT_CODE_MEM_BASE`. O resumo do Makefile apresenta o binário como se começasse em `0x8000`.
- A imagem LPC termina em LMA `0x44e5c`; a ROM configurada termina em `0x45000`. Restam apenas **420 B**.
- O único segmento LOAD da RZ/T2M tem permissões RWX.

### Alteração mínima de diagnóstico

Foram alterados apenas os Makefiles secure das duas plataformas:

- o link passa a criar `bl32.map` com `-Wl,-Map=...,--cref`;
- após criar `bl32.elf`, é criado `dump.txt` com `$(SIZE) -A` seguido de `$(OBJDUMP) -x -S`;
- todas as ferramentas usam as variáveis da toolchain e todos os caminhos usam `$(OBJDIR)`.

A tentativa preexistente na RZ/T2M usava `arm-none-eabi-readelf -SW bl32.elf > ./dump.txt`; procurava o ELF no diretório errado e deixou um ficheiro vazio. Esse ficheiro preexistente foi preservado. O dump válido fica junto do ELF no build.

## 3. Contabilidade de memória

### Secções e reservas

| Item | RZ/T2M bytes (KiB) | LPC55S69 bytes (KiB) | Observação |
|---|---:|---:|---|
| `.start` | 380 (0,371) | 136 (0,133) | Conteúdo na imagem |
| `.text` | 137 264 (134,047) | 109 344 (106,781) | RAM na RZ; ROM na LPC |
| `.rodata` | 101 192 (98,820) | 101 208 (98,836) | RAM na RZ; ROM na LPC |
| unwind (`.ARM.exidx` + `.eh_frame`) | 52 (0,051) | 8 (0,008) | Conteúdo na imagem |
| `.data` | 2020 (1,973) | 2012 (1,965) | VMA=LMA na RZ; VMA SRAM/LMA ROM na LPC |
| gap antes da `.bss` | 4 (0,004) | 4 (0,004) | Alinhamento, sem símbolo |
| `.bss` | 22 176 (21,656) | 18 424 (17,992) | NOBITS, zerada no arranque |
| alinhamento de stack | incluído: 8 | 8 (0,008) | RZ: dentro de `.stack`; LPC: fora de secção |
| stacks efetivas | 65 536 (64,000) | 32 768 (32,000) | RZ: 2 modos × 2 cores × 16 KiB; LPC: MSP 32 KiB |
| `.noinit` | 0 | 0 | Secção ausente |
| heap estático `ucHeap` | 16 384 (16,000) | 16 384 (16,000) | Já incluído na `.bss`, não somar novamente |
| capacidade após `_heap_base` | 195 648 (191,063) | 12 320 (12,031) | Não é uma secção reservada; é o espaço físico restante |

Na RZ/T2M não se deve somar `ucHeap` à `.bss`, nem `.bss`/`.stack` ao `p_memsz` do segmento: já estão incluídos. A reserva total do segmento é `0x503c0` = 328 640 B.

Na LPC, o `PT_LOAD` RW tem `p_filesz=2012` e `p_memsz=20 440`: inclui `.data`, o gap de 4 B e `.bss`, mas não a stack. A reserva real até `_stack_top` é:

```text
2012 (.data) + 4 (gap) + 18424 (.bss) + 8 (align) + 32768 (stack)
= 53216 B
```

### VMA, LMA e imagem

RZ/T2M:

- `.start` VMA/LMA `0x10100000`;
- `.data` VMA/LMA `0x1013a530`;
- `.bss` `0x1013ad18..0x101403b8`, NOBITS;
- `.stack` `0x101403b8..0x101503c0`, NOBITS;
- `bl32.bin`: 240 916 B; termina antes da `.bss`;
- `bl32.elf`: 983 888 B devido sobretudo a símbolos e DWARF; não representa a imagem carregável.

LPC55S69:

- código VMA/LMA a partir de `0x00010000`;
- `.data` VMA `0x20010000`, LMA `0x00044680`, 2012 B;
- `.bss` VMA `0x200107e0..0x20014fd8`, NOBITS;
- `_stack_base=0x20014fe0`, `_stack_top=0x2001cfe0`;
- `bl32.bin`: 216 668 B, incluindo a cópia inicial da `.data` na ROM;
- `bl32.elf`: 927 132 B, incluindo DWARF.

O ELF de cada plataforma só aloca as regiões acima; não foram somados endereços de teste ou aliases mencionados no código.

## 4. Reconciliação exaustiva da `.bss`

| Categoria | RZ/T2M B | LPC B | Evidência/origem |
|---|---:|---:|---|
| `ucHeap` | 16 384 | 16 384 | `common/heap_4.c:106`, `CONFIG_TOTAL_HEAP_SIZE=0x4000`, `liballoc_s.a(heap_4.o)` |
| `irq_handlers` | 4096 | 364 | `core/irq.c:4`; RZ `IRQ_NUM=1024`, LPC `IRQ_NUM=PLAT_MAX_INTERRUPTS+16=91` |
| estáticos criptográficos | 828 | 828 | `pctx.0`, `context.0`, `key_pad.1`, `jres.0`, `a.1`, `hashcode.0` |
| descritores LibTomCrypt | 408 | 408 | três arrays de `TAB_SIZE=34` ponteiros |
| objetos newlib | 374 | 374 | `__sf`, mallinfo/malloc, locks, `errno`, saída stdio |
| metadados `heap_4` | 32 | 32 | `xStart`, `pxEnd` e seis contadores/estado |
| runtime/driver específico | 25 | 9 | RZ: init, proteção Renesas, timer e GIC; LPC: init |
| globais TEE/crypto pequenas | 16 | 16 | sessão, uref, seed e ponteiro de mempool |
| soma de objetos ELF | **22 163** | **18 415** | símbolos OBJECT cujo índice de secção é 7 |
| padding | **13** | **9** | intervalos `*fill*` confirmados no map |
| `.bss` reconciliada | **22 176** | **18 424** | coincide exatamente com `sh_size` |

Não foram contados `__bss_start`, `__bss_end`, símbolos SECTION, `$d` ou aliases como objetos. Não há bytes de linker script além do padding na `.bss` e não há `COMMON` final.

### Consumidores por ordem decrescente

| Símbolo/grupo | RZ B (KiB; % `.bss`) | LPC B | Declaração e objeto | Finalidade, configuração e plataformas |
|---|---:|---:|---|---|
| `ucHeap` | 16 384 (16,000; 73,882%) | 16 384 | `common/heap_4.c:106`, `liballoc_s.a(heap_4.o)` | heap usado por `TEE_Malloc`/`TEE_Free`; `CONFIG_TOTAL_HEAP_SIZE`; ambas |
| `irq_handlers` | 4096 (4,000; 18,470%) | 364 | `BaremetalRuntime/src/core/irq.c:4`, `irq.o` | tabela indexada diretamente pelo ID; RZ `IRQ_NUM=1024`; LPC 91 |
| `pctx.0` | 328 (0,320; 1,479%) | 328 | `apps/bw/ta/bip39.c:63`, `bip39.o` | contexto PBKDF2-HMAC-SHA512 estático; app BW; ambas |
| `__sf` | 312 (0,305; 1,407%) | 312 | newlib `libc.a(findfp.o)` | três estruturas `FILE`; chamadas printf/stdio; ambas |
| `context.0` | 208 (0,203; 0,938%) | 208 | `apps/bw/ta/crypto/hmac.c:152`, `hmac.o` | contexto SHA-512 temporário estático; ambas |
| `cipher_descriptor` | 136 (0,133; 0,613%) | 136 | `crypt_cipher_descriptor.c`, `libcrypt_s.a` | `TAB_SIZE=34`; configuração atual regista AES; ambas |
| `hash_descriptor` | 136 (0,133; 0,613%) | 136 | `crypt_hash_descriptor.c`, `libcrypt_s.a` | `TAB_SIZE=34`; SHA-1/SHA-256/SHA-512 ativos; ambas |
| `prng_descriptor` | 136 (0,133; 0,613%) | 136 | `crypt_prng_descriptor.c`, `libcrypt_s.a` | `TAB_SIZE=34`; configuração atual regista um PRNG; ambas |
| `key_pad.1` | 128 (0,125; 0,577%) | 128 | `apps/bw/ta/crypto/hmac.c:148`, `hmac.o` | pad HMAC-SHA512 estático e apagado após uso; ambas |
| `jres.0` | 108 (0,105; 0,487%) | 108 | `apps/bw/ta/crypto/ecdsa.c:392`, `ecdsa.o` | ponto Jacobiano durante multiplicação escalar; ambas |
| `__malloc_current_mallinfo` | 40 (0,039; 0,180%) | 40 | newlib `libc.a(mallocr.o)` | metadados do segundo allocator; ambas |
| `a.1` | 36 (0,035; 0,162%) | 36 | `apps/bw/ta/crypto/ecdsa.c:386`, `ecdsa.o` | inteiro temporário de multiplicação escalar; ambas |
| `hashcode.0` | 20 (0,020; 0,090%) | 20 | `ta_ripemd160.c:282` | resultado estático RIPEMD-160 devolvido por ponteiro; ambas |
| metadados `heap_4` | 32 (0,031; 0,144%) | 32 | `common/heap_4.c`, `liballoc_s.a` | free-list, high-water e contadores; ambas |
| restantes newlib | 62 (0,061; 0,280%) | 62 | `mallocr.o`, `findfp.o`, `lock.o`, `reent.o` | malloc/stdio/errno; ambas |
| RZ driver/runtime | 25 (0,024; 0,113%) | — | `retarget.o`, `rzt2m_uart.o`, `timer.o`, `gicv3.o` | locks/estado de init, proteção FSP, timer e GIC |
| LPC runtime init | — | 9 | `retarget.c:96-97`, `retarget.o` | `spinlock_t` de 8 B + booleano |
| TEE/crypto pequenas | 16 (0,016; 0,072%) | 16 | `tee_svc.c`, `tee_ta_manager.c`, `rand.c`, `bignum.c` | uref, sessão atual, PRNG de teste e mempool |
| padding | 13 (0,013; 0,059%) | 9 | map file | alinhamento entre objetos; não é variável |

As listagens completas, incluindo endereço, binding, índice de secção, bytes, KiB e percentagem, estão em `rzt2m/baseline/bss-symbols.tsv` e `lpc55s69/baseline/bss-symbols.tsv`. Os map files identificam o objeto/biblioteca de cada contribuição.

### Observações de segurança e concorrência

- `pctx`, os temporários ECC/HMAC e `hashcode` são estáticos locais. A análise de C confirma uso e limpeza, mas esta forma não é reentrante.
- `current_session` também é global. Logo, não se pode inferir que operações simultâneas nos dois cores sejam suportadas apenas porque a RZ/T2M reserva stacks para ambos.
- `heap_4.c` tem as chamadas de exclusão (`vTaskSuspendAll`/`xTaskResumeAll`) comentadas. Redimensionar ou unificar heaps exige primeiro definir e testar o modelo de concorrência.
- `random32()` contém a mensagem de compilação “NOT SUITABLE FOR PRODUCTION USE” e usa um `seed` global; isto não explica tamanho significativo de RAM, mas é uma condição de segurança a tratar fora desta redução.
- A pesquisa estática encontrou handlers registados para IPI 0, timer 27 e UART 321 na RZ/T2M. Isto não prova que IDs superiores nunca serão necessários por outros drivers, firmware, interrupções de segurança ou evolução futura.

## 5. Propostas, benefício e risco

As poupanças abaixo são estimativas condicionais; não demonstram preservação funcional.

### P1 — Medir e redimensionar stacks específicas da RZ/T2M

- Causa/evidência: `STACK_SIZE=0x4000`; o linker reserva SVC+IRQ por core e `PLAT_CPU_COUNT=2`, total 64 KiB.
- Alteração candidata: manter configuração separada por plataforma e reduzir somente stacks da RZ/T2M depois de medir cada stack/modo/core. Exemplo meramente quantitativo: 8 KiB por stack.
- Poupança estimada: **32 768 B RZ**, 0 B LPC, se quatro stacks passarem de 16 para 8 KiB. Se o produto/partição for comprovadamente single-core, `SINGLE_CORE` pouparia 32 KiB, mas removeria capacidade do core 1 e não é aceitável sem requisito explícito.
- Impacto: reduz RAM total e `p_memsz`; não reduz os bytes carregáveis NOBITS. O ELF físico muda pouco.
- Riscos: exceções aninhadas, chamadas crypto/printf profundas, dois modos de CPU, dois cores, hard faults, latência e margem de segurança.
- Validação: canários/high-water independentes nos quatro intervalos, testes de pior caso com IRQ nesting e carga simultânea nos dois cores, margem documentada e hardware RZ/T2M.

### P2 — Dimensionar `ucHeap` por medição, sem impor o mesmo limite às boards

- Causa/evidência: `CONFIG_TOTAL_HEAP_SIZE=0x4000` em ambas; comentário no defconfig indica aumento anterior de `0x2000` para `0x4000`.
- Alteração candidata: valores por plataforma, derivados de pico simultâneo e fragmentação. Não escolher 8 KiB apenas pelo comentário histórico.
- Poupança estimada: fórmula `16384 - novo_tamanho`. Se 8 KiB for comprovado: **8192 B RZ e 8192 B LPC**.
- Impacto: reduz `.bss`, RAM total e `p_memsz`; não reduz `bl32.bin`; o ELF só perde NOBITS.
- Riscos: falhas tardias em TEE/crypto, fragmentação, concorrência e caminhos raros. O high-water de bytes livres não mede o maior bloco contíguo.
- Validação: registar `xPortGetMinimumEverFreeHeapSize`, maior bloco livre, número/tamanho máximo de alocações e falhas; executar todos os comandos BW, erros, sessões, limites de chave/dados e concorrência. Corrigir/definir locking antes de teste multicore.

### P3 — Limite de interrupções específico da RZ/T2M

- Causa/evidência: header ARMv8 genérico fixa `IRQ_NUM=1024`; o projeto atual regista IDs 0, 27 e 321. A LPC já usa limite específico 91.
- Alteração candidata: mover o tamanho para `plat.h`, baseado no maior INTID implementado/requerido pelo SKU e validado contra `GICD_TYPER` e a lista oficial de interrupções. Não alterar a LPC.
- Poupança estimada: se for necessário cobrir 0..337, tabela de 338 ponteiros: **2744 B RZ**; se 0..321 bastar: **2808 B RZ**. LPC: 0 B.
- Impacto: reduz `.bss` e RAM total; não altera imagem carregável.
- Riscos: perder silenciosamente handlers acima do limite, variantes de package/SKU, drivers futuros e interrupções de segurança. O manual de hardware oficial existe no portal Renesas, mas requer login; o limite exato não ficou confirmado nesta análise.
- Validação: enumerar todos os `irq_set_handler`, ler `GICD_TYPER`, testar todos os periféricos/IRQs usados e fazer falhar o build/arranque se um ID exceder a capacidade.

Uma tabela esparsa poderia aproximar-se de 4 KiB de poupança, mas acrescenta lookup, locking e variação temporal no caminho de IRQ; tem risco superior ao limite direto.

### P4 — Tabelas LibTomCrypt dimensionadas por classe

- Causa/evidência: três arrays usam `TAB_SIZE=34`, embora a configuração compile AES, SHA-1/SHA-256/SHA-512 e um PRNG.
- Alteração candidata: capacidades distintas e configuráveis para cipher/hash/PRNG, com erro explícito se o registo exceder o limite.
- Poupança estimada para capacidades 1/3/1: `(3*34 - 5)*4` = **388 B em cada plataforma**.
- Impacto: reduz `.bss` e RAM total; `bl32.bin` não muda por causa dos arrays NOBITS; código de validação pode alterar ligeiramente a imagem.
- Riscos: algoritmos ativados noutras configurações, registo dinâmico e testes futuros.
- Validação: matriz de configurações crypto, testes de todos os algoritmos e teste negativo de tabela cheia.

### P5 — Unificar os dois allocators

- Causa/evidência: `TEE_Malloc` usa `pvPortMalloc`, enquanto partes de crypto/mempool chamam `malloc`; newlib mantém cerca de **1096 B** de metadados estáticos de malloc (`.data` + `.bss`) e pode consumir RAM adicional após `_heap_base`.
- Alteração candidata: uma única implementação com wrappers completos e semântica documentada (`malloc/free/calloc/realloc` e variantes `_r`), ou encaminhar todos os chamadores para um allocator seguro comum.
- Poupança estimada: até **1096 B estáticos por plataforma**, mais redução de headroom duplicado que só pode ser quantificada com medição combinada. Não contar a simples deslocação de alocações entre heaps como poupança.
- Impacto: pode reduzir `.data`, `.bss`, código newlib e imagem; o heap comum terá de suportar o pico agregado.
- Riscos altos: reentrância, errno, stdio, fragmentação, ABI newlib, ISR, multicore e o locking atualmente comentado em `heap_4`.
- Validação: testes de allocator, stress/fragmentação, crypto e printf, falhas de alocação, concorrência e comparação de latência.

### P6 — Otimizar código residente em RAM na RZ/T2M

- Causa/evidência: a RZ/T2M executa toda a imagem na região RAM; usa `-O1` fixo no Makefile secure.
- Alteração candidata: variante `-Os`/LTO, mantendo a LPC em configuração própria se os requisitos de tempo diferirem.
- Poupança: não estimada sem build comparativo; afeta `.text`/`.rodata`, não `.bss`.
- Impacto: reduz RAM total e imagem na RZ; na LPC reduz ROM, não SRAM.
- Riscos: WCET, latência de IRQ, debug e alterações de inlining.
- Validação: tamanho antes/depois, benchmarks e WCET/latência em hardware.

### Não contabilizado como poupança

- mover os 828 B de estáticos crypto para stack/heap;
- inicializar dados a valor não-zero para os mover para `.data`;
- criar outra secção NOBITS;
- remover apenas símbolos de debug do ELF;
- somar a capacidade livre após `_heap_base` como se fosse RAM já consumida.

## 6. Plano de implementação e validação

1. **Instrumentação sem redução**
   - canários e high-water das quatro stacks RZ e da stack LPC;
   - métricas de ambos os allocators, fragmentação e limites seguros em `_sbrk`;
   - máximo ID registado/recebido e leitura de `GICD_TYPER`;
   - contagem dos registos de cipher/hash/PRNG;
   - definir e testar o modelo de locking do heap e dos estáticos crypto.
2. **Mudanças pequenas e determinísticas**
   - aplicar capacidade de IRQ específica da RZ após confirmação documental/hardware;
   - separar capacidades LibTomCrypt;
   - build e comparação byte/secção/map nas duas plataformas.
3. **Reduções guiadas por high-water**
   - reduzir `ucHeap` com margem formal por plataforma;
   - reduzir cada stack RZ separadamente; não alterar a LPC por arrasto.
4. **Mudanças arquiteturais de maior risco**
   - unificar allocators e, se necessário, tornar crypto reentrante;
   - avaliar `-Os`/LTO com medições temporais.

Para cada passo:

- gerar `size -A`, `readelf -SW/-lW`, `nm -S`, map e dump antes/depois;
- construir `PLATFORM=rzt2m` e `PLATFORM=lpc55s69` a partir de clean, arquivando antes os artefactos;
- executar testes funcionais TEE/BW/crypto e todos os formatos de logging necessários;
- testar limites, falhas, fragmentação, interrupções aninhadas e concorrência multicore;
- validar boot, VMA/LMA, cópia de `.data`, limpeza da `.bss`, stack e interrupções em ambas as boards.

Nesta sessão não houve hardware. Uma compilação bem-sucedida e a reconciliação dos ELF não comprovam preservação funcional em execução.

## 7. Artefactos reproduzíveis

- `rzt2m/preexisting/`: ELF/config preservados antes de clean.
- `rzt2m/baseline/`: ELF, BIN, map, dump, linker preprocessado e listagens completas.
- `lpc55s69/baseline/`: conjunto equivalente da LPC.
- `rzt2m/build.log` e `lpc55s69/build.log`: logs completos.
- `sha256sums.txt`: hashes dos artefactos principais.
- `COMMANDS.md`: comandos para repetir a recolha.

Fonte externa consultada apenas para confirmar a existência/versão do manual e características gerais do produto: página oficial [Renesas RZ/T2M](https://www.renesas.com/en/products/rz-t2m). A tabela completa de interrupções do manual requer autenticação; por isso o limite proposto de IRQ permanece hipótese condicionada, não facto.
