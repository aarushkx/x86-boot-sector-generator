#include <stdlib.h>

int main(void)
{
    system("qemu-system-i386 -drive format=raw,file=build/boot.img");
    return 0;
}