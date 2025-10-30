# Modules
Modules are VeilOS' way of implementing certain features a normal monolithic kernel may contain such as, but not limited to: filesystems, vfs, etc

## How to add them to the kernel
VeilOS must require at least 1 partition to have a [Voidelle](https://github.com/Preciseful/Voidelle) filesystem. This will contain necessary stuff to boot.
1. Run `make modules`.
2. Run `sudo voidelle [YOUR PARTITION HERE] mkdir /modules`
3. Run `sudo voidelle [YOUR PARTITION HERE] touch /modules/Voidelle`
4. Run `sudo voidelle [YOUR PARTITION HERE] ecp /modules/Voidelle modules/voidelle/module` from the root of the project (this is what makes it executable, by copying the `.elf` data in it)

