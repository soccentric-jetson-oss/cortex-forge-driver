savedcmd_cortex-forge.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o cortex-forge.o @cortex-forge.mod  ; /usr/src/linux-headers-7.0.0-28-generic/tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --retpoline --rethunk --sls --stackval --static-call --uaccess --prefix=16  --link  --module cortex-forge.o

cortex-forge.o: $(wildcard /usr/src/linux-headers-7.0.0-28-generic/tools/objtool/objtool)
