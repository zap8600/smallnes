#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct CPU {
    uint16_t pc;
    uint8_t a, x, y, sr, sp, status; // In a VRC Shader, the PC and these registers only require half of a pixel to be stored
    uint8_t mem[0xFFFF]; // When set to the size of the NES RAM (2048B), this will only takes 128 pixels
} CPU;

CPU cpu;


uint8_t read_u8(uint16_t addr) {
    return cpu.mem[addr];
}

void write_u8(uint16_t addr, uint8_t data) {
    cpu.mem[addr] = data;
}

uint16_t read_u16(uint16_t addr) {
    return (uint16_t)(cpu.mem[addr + 1] << 8) | (uint16_t)(cpu.mem[addr]);
}

void write_u16(uint16_t addr, uint16_t data) {
    cpu.mem[addr] = data & 0xFF;
    cpu.mem[addr + 1] = data >> 8;
}


void update_zero_neg(uint8_t result) {
    if(!result) {
        cpu.status |= 0x2;
    } else {
        cpu.status |= 0xfd;
    }

    if(result & 0x80) {
        cpu.status |= 0x80;
    } else {
        cpu.status |= 0xef;
    }
}

void LDA(uint8_t value) {
    cpu.a = value;
    update_zero_neg(value);
}

void TAX() {
    cpu.x = cpu.a;
    update_zero_neg(cpu.x);
}

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s [NES ROM File]\n", argv[0]);
        return 1;
    }

    FILE* rom = fopen(argv[1], "rb");

    fseek(rom, 0, SEEK_END);
    unsigned long fileLen = ftell(rom);
    fseek(rom, 0, SEEK_SET);

    fread(&(cpu.mem[0x8000]), fileLen, 1, rom);
    write_u16(0xFFFC, 0x8000);

    fclose(rom);

    cpu.pc = read_u16(0xFFFC);

    while(1) { // TODO: Make this terminate on end.
        uint8_t opcode = cpu.mem[cpu.pc];
        cpu.pc += 1;

        switch(opcode) {
            case 0xA9:
            {
                LDA(read_u8(cpu.pc));
                cpu.pc += 1;

                break;
            }
            case 0xA5:
            {
                LDA(read_u8((uint16_t)(read_u8(cpu.pc))));
                cpu.pc += 1;

                break;
            }
            case 0xAD:
            {
                LDA(read_u8(read_u16(cpu.pc)));
                cpu.pc += 1;

                break;
            }
            case 0xB5:
            {
                LDA(read_u8((uint16_t)(read_u8(cpu.pc) + cpu.x)));
                cpu.pc += 1;

                break;
            }
            case 0xB5:
            {
                LDA(read_u8((uint16_t)(read_u8(cpu.pc) + cpu.x)));
                cpu.pc += 1;

                break;
            }
            case 0x00:
            {
                return 0;
            }
            case 0xAA:
            {
                TAX();
                break;
            }
        }
    }
}