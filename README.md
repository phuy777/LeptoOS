# LeptoOS

LeptoOS is an experimental operating system project built from the ground up. It is a place to explore how an operating system works, starting with a small freestanding kernel and growing toward a more capable system over time.

The project is at an early stage and will continue to evolve. Its current implementation boots a 64-bit kernel through GRUB and writes a simple greeting to VGA text memory. It is not intended for everyday use.

Features such as device support, memory management, processes, storage, and a user interface may be developed as the project progresses.

## Building

Build the kernel from the project root:

```sh
make
```

Select a compiler with `CC` (`x86_64-elf-gcc` is the default):

```sh
make CC=gcc
make CC=clang
```

All C source files in the project root are included automatically in the kernel build.

Create a bootable ISO image:

```sh
make iso
```

The kernel is written to `build/kernel.elf`, and the ISO image is written to `build/leptoos.iso`.

### Requirements

- GNU Make
- GCC or Clang with x86-64 support (an `x86_64-elf-gcc` cross-compiler is also supported)
- For `make iso`: GRUB utilities, including `grub-mkrescue`, and `xorriso`
- QEMU (`qemu-system-x86_64`) to run the ISO in an emulator

Build the ISO and run it with QEMU:

```sh
make run
```

Override the emulator or its arguments with `QEMU` and `QEMUFLAGS` if needed.

Remove generated build files with:

```sh
make clean
```

## Project layout

| File or directory | Purpose |
| --- | --- |
| `boot.S` | Multiboot2 entry point and initial x86-64 setup |
| `main.c` | Kernel entry routine |
| `kernel.c` / `kernel.h` | Kernel terminal implementation and declarations |
| `linker.ld` | Kernel memory layout |
| `grub/grub.cfg` | GRUB boot menu configuration |
| `Makefile` | Build, ISO creation, and cleanup targets |

## Looking ahead

LeptoOS is intended to be developed incrementally. Future work may include improving console output, handling interrupts, managing memory, supporting hardware, and introducing processes and storage. These are possible directions, not promises of a particular schedule or feature set.

## Contributing

Ideas, bug reports, and contributions are welcome. Since the project is still taking shape, it is helpful to discuss larger changes before starting work on them.

## License

LeptoOS is licensed under the GNU General Public License, version 3 only (GPL-3.0-only). See [LICENSE](LICENSE) for the full license text.
