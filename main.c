#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CNFG_IMPLEMENTATION
#include "rawdraw_sf.h"


void setColor(uint8_t byte) {
    uint32_t color;
    switch(byte) {
        case 0: color = 0x00000000; break;
        case 1: color = 0xffffffff; break;
        case 2:
        case 9:
        color = 0x808080FF; break;
        case 3:
        case 10:
        color = 0xff0000ff; break;
        case 4:
        case 11:
        color = 0x00ff00ff; break;
        case 5:
        case 12:
        color = 0x0000ffff; break;
        case 6:
        case 13:
        color = 0xff00ffff; break;
        case 7:
        case 14:
        color = 0xffff00ff; break;
        default: color = 0x00ffffff; break;
    }
    CNFGColor(color);
}


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


void HandleKey(int keycode, int bDown) {
    if(bDown) {
        write_u8(0xFF, (uint8_t)keycode);
    }
}
void HandleButton(int x, int y, int button, int bDown) { }
void HandleMotion(int x, int y, int mask) { }
int HandleDestroy() { return 0; }


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

void STA(uint16_t addr) {
    write_u8(addr, cpu.a);
}

void TAX() {
    cpu.x = cpu.a;
    update_zero_neg(cpu.x);
}

void INX() {
    cpu.x += 1;
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

    fread(&(cpu.mem[0x0600]), fileLen, 1, rom);
    write_u16(0xFFFC, 0x0600);

    fclose(rom);

    cpu.pc = read_u16(0xFFFC);

    CNFGSetup("GPU NES", 320, 320);

    while(CNFGHandleInput()) { // TODO: Make this terminate on end.
        
        CNFGClearFrame();

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
            case 0xB5:
            {
                LDA(read_u8((uint16_t)(read_u8(cpu.pc) + cpu.x)));
                cpu.pc += 1;

                break;
            }
            case 0xAD:
            {
                LDA(read_u8(read_u16(cpu.pc)));
                cpu.pc += 2;

                break;
            }
            case 0xBD:
            {
                LDA(read_u8((read_u16(cpu.pc) + cpu.x)));
                cpu.pc += 2;

                break;
            }
            case 0xB9:
            {
                LDA(read_u8((read_u16(cpu.pc) + cpu.y)));
                cpu.pc += 2;

                break;
            }
            case 0xA1:
            {
                uint8_t base = read_u8(cpu.pc) + cpu.x;
                LDA(read_u8(((uint16_t)read_u8(base + 1) << 8) | ((uint16_t)read_u8(base))));
                cpu.pc += 1;

                break;
            }
            case 0xB1:
            {
                uint8_t base = read_u8(cpu.pc);
                LDA(read_u8((((uint16_t)read_u8(base) << 8) | ((uint16_t)read_u8(base))) + cpu.y));
                cpu.pc += 1;

                break;
            }

            case 0x85:
            {
                STA(read_u8((uint16_t)(read_u8(cpu.pc))));
                cpu.pc += 1;

                break;
            }
            case 0x95:
            {
                STA(read_u8((uint16_t)(read_u8(cpu.pc) + cpu.x)));
                cpu.pc += 1;

                break;
            }
            case 0x8D:
            {
                STA(read_u8(read_u16(cpu.pc)));
                cpu.pc += 2;

                break;
            }
            case 0x9D:
            {
                STA(read_u8((read_u16(cpu.pc) + cpu.x)));
                cpu.pc += 2;

                break;
            }
            case 0x99:
            {
                STA(read_u8((read_u16(cpu.pc) + cpu.y)));
                cpu.pc += 2;

                break;
            }
            case 0x81:
            {
                uint8_t base = read_u8(cpu.pc) + cpu.x;
                STA(read_u8(((uint16_t)read_u8(base + 1) << 8) | ((uint16_t)read_u8(base))));
                cpu.pc += 1;

                break;
            }
            case 0x91:
            {
                uint8_t base = read_u8(cpu.pc);
                STA(read_u8((((uint16_t)read_u8(base) << 8) | ((uint16_t)read_u8(base))) + cpu.y));
                cpu.pc += 1;

                break;
            }

            case 0xAA:
            {
                TAX();
                break;
            }

            case 0xE8:
            {
                INX();
                break
            }

            case 0x00:
            {
                return 0;
            }

            default:
            {
                fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
                return 1;
            }
        }
        
        CNFGSwapBuffers();
    }
}