# VeilOS

VeilOS is a Raspberry Pi 4B OS with no virtual memory.

## Setup

For proper IO, you must use a [USB to TLL Serial cable](https://www.adafruit.com/product/954). Although screen printing is active, it's terribly implemented.

1. Flash the Raspberry Pi's SD card with the normal Raspberry Pi OS image, then run `make setup` (make sure the RPI_PATH variable in the makefile is correct). <br/> RPIPATH is the path to your Raspberry Pi's `bootfs` location. Example: `/media/USER/bootfs`.
2. The aarch64-none-elf compiler must be installed. You can find it [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
   - Make sure the compiler's bin path is an environment variable (example: `/PATH/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-elf/bin`)
   * OR modify the Makefile's `ARMGNU` variable to point to the installation's location.
3. Run `make`. This should compile the kernel and automatically copy it to the Raspberry Pi path.
4. Using PuTTY (or any other tool you'd like) set the following settings: (they can be found under Connection, Serial in PuTTY)

   - Speed (baud) = 115200
   - Data bits = 8
   - Stop bits = 1
   - Parity = None
   - Flow control = None

5. If you're using PuTTY, don't forget to save the settings for convenience.

## Modules

To create a module that the kernel will run, you must first meet the following conditions:

- The compiler used must be the same aarch64-none-elf used for compiling.
- The language should probably be C++, as some of the stdlib is built around C++ rather than C. However, feel free to make the C variants of the stdlib since it should be simple.

- The compile arguments must be as follow (gcc):
  `-ffreestanding`
  `-fPIE`
  `-nostdlib`
  `-fno-exceptions`
- The linker arguments (gcc):
  `-nostartfiles`
  `-pie`
  `-Wl,--emit-relocs`
  `-Wl,-e,main`
  `-Wl,-z,now`
- The entrance to the file must be of type `int main()`.
- In the VeilOS directory, run `make dyn`. You now have a `libveil.so` file that's a library you should link to your module. <br/>Make sure you also include the `stdlib` directory in general, some are just simple header files that don't need an `.so` file
- When you're done compiling, copy the new `.ELF` file you built to the `modules` folder found in the Raspberry Pi's `bootfs`.
- Modify the kernel at [this line](https://github.com/Preciseful/VeilOS/blob/master/src/modules.cpp#L33) following the example in the comment. You just simply enter the module's name. (for example, a module named `Luna.elf`, will just be simply `Luna` in the `init_module`).
  - There is also 2 arguments you can add,`KERNEL_FLAG` and `SHELL_FLAG`.
    - `KERNEL_FLAG` is for EL1 processes (i would advise using this, since EL0 might be wonky).
    - The `SHELL_FLAG` is reserved for shells, ensuring that across context switches no other process accidentally steals I/O from the keyboard.

## Future

I'm pausing progress on this project for now so feel free to mess around with it and create pull requests if you want. (i honestly dont really care what you do with it)
