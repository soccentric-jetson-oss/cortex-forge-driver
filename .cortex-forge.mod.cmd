savedcmd_cortex-forge.mod := printf '%s\n'   src/main.o src/task_manager.o src/accel_manager.o src/chardev.o | awk '!x[$$0]++ { print("./"$$0) }' > cortex-forge.mod
