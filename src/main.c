#include <stdio.h>

void put_byte(unsigned char *buffer, int *pos, unsigned char byte)
{
    buffer[*pos] = byte;
    (*pos)++;
}

// writes a 16-bit word into the buffer
void put_word(unsigned char *buffer, int *pos, unsigned short val)
{
    put_byte(buffer, pos, val & 0xFF);        // lower 8 bits
    put_byte(buffer, pos, (val >> 8) & 0xFF); // upper 8 bits
}

// B8 = MOV AX, val
void mov_ax(unsigned char *buffer, int *pos, unsigned short val)
{
    put_byte(buffer, pos, 0xB8);
    put_word(buffer, pos, val);
}

// BB = MOV AX, val
void mov_bx(unsigned char *buffer, int *pos, unsigned short val)
{
    put_byte(buffer, pos, 0xBB);
    put_word(buffer, pos, val);
}

// 89 C4 = MOV SP, AX
void mov_sp_ax(unsigned char *buffer, int *pos)
{
    put_byte(buffer, pos, 0x89);
    put_byte(buffer, pos, 0xC4);
}

// CD 10 = INT 0x10
void bios_interrupt(unsigned char *buffer, int *pos)
{
    put_byte(buffer, pos, 0xCD);
    put_byte(buffer, pos, 0x10);
}

// 90 = NOP
// F4 = HLT
void halt(unsigned char *buffer, int *pos)
{
    put_byte(buffer, pos, 0x90);
    put_byte(buffer, pos, 0xF4);
}

// EB = short jump
// FC = jump backwards by 4 bytes
void infinite_loop(unsigned char *buffer, int *pos)
{
    put_byte(buffer, pos, 0xEB);
    put_byte(buffer, pos, 0xFC);
}

// we make AX so that AH = 0x0E and AL = char to print
void print_char(unsigned char *buffer, int *pos, char c)
{
    unsigned short ax_val = ((unsigned short)0x0E << 8) | (unsigned char)c;
    mov_ax(buffer, pos, ax_val);
    bios_interrupt(buffer, pos);
}

// each char is printed individually using BIOS interrupt 10h
void print_string(unsigned char *buffer, int *pos, const char *text)
{
    int i = 0;
    while (text[i] != '\0')
    {
        print_char(buffer, pos, text[i]);
        i++;
    }
    print_char(buffer, pos, '\r');
    print_char(buffer, pos, '\n');
}

// creates the boot sector of exactly 512 bytes
int create_boot_sector(unsigned char *buffer, const char *msg)
{
    int pos = 0;
    int padding;

    mov_ax(buffer, &pos, 0xFBFF);
    mov_sp_ax(buffer, &pos);
    mov_bx(buffer, &pos, 0x0000);

    print_string(buffer, &pos, msg);
    halt(buffer, &pos);
    infinite_loop(buffer, &pos);

    padding = 510 - pos;

    while (padding > 0)
    {
        put_byte(buffer, &pos, 0x90);
        padding--;
    }

    // boot signature
    put_byte(buffer, &pos, 0x55);
    put_byte(buffer, &pos, 0xAA);

    return pos;
}

int main(void)
{
    printf("Program started...\n");

    unsigned char boot_sector[512];
    FILE *file;
    const char *output_img_path = "build/boot.img";

    int size = create_boot_sector(boot_sector, "Hello, World!");

    if (size != 512)
    {
        printf("Error: boot sector is %d bytes\n", size);
        return 1;
    }

    file = fopen(output_img_path, "wb");

    if (file == NULL)
    {
        printf("Error: failed to open %s\n", output_img_path);
        return 1;
    }

    fwrite(boot_sector, 1, 512, file);
    fclose(file);

    printf("Written to file %s\n", output_img_path);
    printf("Size: %d bytes\n", size);

    return 0;
}