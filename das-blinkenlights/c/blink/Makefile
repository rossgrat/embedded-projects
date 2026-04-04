
# Makefile Syntax
# $^ = All arguments
# $@ = Target
# $< = First argument

bin/main.o: src/main.c
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -Os -c $< -o $@

bin/startup.o: src/startup.c
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -Os -c $< -o $@

bin/blink.elf: bin/startup.o bin/main.o
	arm-none-eabi-gcc -T src/link.ld -nostdlib  $^ -o $@

bin/blink.bin: bin/blink.elf
	arm-none-eabi-objcopy -O binary $< $@

.PHONY: dump flash
flash: bin/blink.bin
	probe-rs download --chip STM32L4R5ZITx --binary-format bin --base-address 0x08000000 --verify bin/blink.bin
	probe-rs reset --chip STM32L4R5ZITx

dump-main: bin/main.o
	arm-none-eabi-objdump -h $<

dump-elf: bin/blink.elf
	arm-none-eabi-objdump -h $<
	arm-none-eabi-objdump -d bin/blink.elf
