# HOTP benchmark using the shared free-running LPC55S69 CTIMER0.
#
# Load the hypervisor (VM only), mTower and FreeRTOS symbols before sourcing
# this file. Start native mTower at _start; start the VM setup at the
# hypervisor _reset_handler. The breakpoint is after hotp_end was recorded,
# so debugger latency is outside every measured interval.

set confirm off
set pagination off
set print elements 0

set $record = 0x2000ff00
set $repetition = 0

hbreak hotp_benchmark_complete
commands
  silent
  set $repetition = $repetition + 1
  set $invoke_count = *(unsigned int *)($record + 24)

  printf "BENCH[%u] CTIMER0_HZ=150000000\n", (unsigned int)$repetition
  printf "  Open total=%u prep=%u transition=%u\n", *(unsigned int *)($record + 20) - *(unsigned int *)($record + 12), *(unsigned int *)($record + 16) - *(unsigned int *)($record + 12), *(unsigned int *)($record + 20) - *(unsigned int *)($record + 16)

  set $i = 0
  while $i < $invoke_count && $i < 16
    set $start = *(unsigned int *)($record + 28 + 4 * $i)
    set $gateway = *(unsigned int *)($record + 92 + 4 * $i)
    set $handler = *(unsigned int *)($record + 156 + 4 * $i)
    printf "  Invoke[%u] total=%u prep=%u transition=%u\n", (unsigned int)($i + 1), (unsigned int)($handler - $start), (unsigned int)($gateway - $start), (unsigned int)($handler - $gateway)
    set $i = $i + 1
  end

  printf "  Close total=%u prep=%u transition=%u\n", *(unsigned int *)($record + 228) - *(unsigned int *)($record + 220), *(unsigned int *)($record + 224) - *(unsigned int *)($record + 220), *(unsigned int *)($record + 228) - *(unsigned int *)($record + 224)
  printf "  tee_hotp total=%u\n", *(unsigned int *)($record + 8) - *(unsigned int *)($record + 4)

  if $repetition >= 20
    quit
  end
  continue
end

continue
