#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "snake.h"
//#include "test.h"

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
    uint8_t a, x, y, sp, status; // In a VRC Shader, the PC and these registers only require half of a pixel to be stored
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

uint8_t pop_u8() {
    cpu.sp += 1;
    return read_u8(0x0100 + cpu.sp);
}

void push_u8(uint8_t data) {
    write_u8(0x0100 + cpu.sp, data);
    cpu.sp -= 1;
}

uint16_t pop_u16() {
    return ((uint16_t)pop_u8()) | (((uint16_t)pop_u8()) << 8);
}

void push_u16(uint16_t data) {
    push_u8(data >> 8);
    push_u8(data & 0xff);
}

void HandleKey(int keycode, int bDown) {
    if(bDown) {
        write_u8(0xFF, (uint8_t)keycode);
    } else {
        write_u8(0xFF, 0x00);
    }
}
void HandleButton(int x, int y, int button, int bDown) { }
void HandleMotion(int x, int y, int mask) { }
int HandleDestroy() { return 0; }


void BRANCH(uint8_t cond) {
    if(cond) {
        int8_t jump = (int8_t)read_u8(cpu.pc);
        cpu.pc += 1 + (uint16_t)jump;
    } else {
        cpu.pc += 1;
    }
}

void update_zero_neg(uint8_t result) {
    if(!result) {
        cpu.status |= 0x2;
    } else {
        cpu.status &= 0xfd;
    }

    if(result & 0x80) {
        cpu.status |= 0x80;
    } else {
        cpu.status &= 0x7f;
    }
}

void LDA(uint8_t value) {
    cpu.a = value;
    update_zero_neg(value);
    //printf("LDA value: 0x%x\n", value);
}

void LDY(uint8_t value) {
    cpu.y = value;
    update_zero_neg(value);
}

void STA(uint16_t addr) {
    write_u8(addr, cpu.a);
    //printf("STA addr: 0x%x, data: 0x%x\n", addr, cpu.a);
}

void INX() {
    cpu.x += 1;
    update_zero_neg(cpu.x);
}

uint8_t INC(uint8_t value) {
    return value + 1;
}

uint8_t DEC(uint8_t value) {
    return value - 1;
}

int main(int argc, char** argv) {
    /*
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
    */
    srand(time(0));

    memcpy(&(cpu.mem[0x0600]), snake_bin, snake_bin_len);
    write_u16(0xFFFC, 0x0600);

    cpu.pc = read_u16(0xFFFC);
    cpu.sp = 0xff;

    CNFGSetup("GPU NES Reference", 320, 320);

    while(CNFGHandleInput()) { // TODO: Make this terminate on end.
        
        CNFGClearFrame();

        uint8_t opcode = cpu.mem[cpu.pc];
        //printf("before run:\npc 0x%x op 0x%x pc+1 0x%x pc+2 0x%x\na 0x%x x 0x%x y 0x%x sp 0x%x status 0x%x\n\n", cpu.pc, opcode, cpu.mem[cpu.pc + 1], cpu.mem[cpu.pc + 2], cpu.a, cpu.x, cpu.y, cpu.sp, cpu.status);
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
                LDA(read_u8((((uint16_t)read_u8(base + 1) << 8) | ((uint16_t)read_u8(base))) + cpu.y));
                cpu.pc += 1;

                break;
            }

            case 0x85:
            {
                STA((uint16_t)(read_u8(cpu.pc)));
                cpu.pc += 1;

                break;
            }
            case 0x95:
            {
                STA((uint16_t)(read_u8(cpu.pc) + cpu.x));
                cpu.pc += 1;

                break;
            }
            case 0x8D:
            {
                STA(read_u16(cpu.pc));
                cpu.pc += 2;

                break;
            }
            case 0x9D:
            {
                STA((read_u16(cpu.pc) + cpu.x));
                cpu.pc += 2;

                break;
            }
            case 0x99:
            {
                STA((read_u16(cpu.pc) + cpu.y));
                cpu.pc += 2;

                break;
            }
            case 0x81:
            {
                uint8_t base = read_u8(cpu.pc) + cpu.x;
                STA(((uint16_t)read_u8(base + 1) << 8) | ((uint16_t)read_u8(base)));
                cpu.pc += 1;

                break;
            }
            case 0x91:
            {   
                uint8_t base = read_u8(cpu.pc);
                STA((((uint16_t)read_u8(base + 1) << 8) | ((uint16_t)read_u8(base))) + cpu.y);
                cpu.pc += 1;

                break;
            }

            case 0xAA:
            {
                cpu.x = cpu.a;
                update_zero_neg(cpu.x);
                break;
            }

            case 0x00:
            {
                //printf("program terminated\n");
                return 0;
                break;
            }

            case 0x20:
            {
                push_u16(cpu.pc + 1);
                cpu.pc = read_u16(cpu.pc);
                break;
            }

            case 0x60:
            {
                //printf("RTS\n");
                cpu.pc = pop_u16() + 1;
                break;
            }

            case 0x29:
            {
                cpu.a = read_u8(cpu.pc) & cpu.a;
                update_zero_neg(cpu.a);
                cpu.pc += 1;
                break;
            }

            case 0x18:
            {
                cpu.status &= 0xfe;
                break;
            }

            case 0x69:
            {
                uint8_t value = read_u8(cpu.pc);
                uint16_t sum = ((uint16_t)cpu.a) + ((uint16_t)value) + (cpu.status & 1);
                if(sum > 0xff) {
                    cpu.status |= 1;
                } else {
                    cpu.status &= 0xfe;
                }
                uint8_t result = (uint8_t)sum;
                if(((value ^ result) & (result ^ cpu.a) & 0x80) != 0) {
                    cpu.status |= 0x40;
                } else {
                    cpu.status &= 0xbf;
                }
                update_zero_neg(result);
                cpu.a = result;
                cpu.pc += 1;
                break;
            }

            case 0x4c:
            {
                cpu.pc = read_u16(cpu.pc);
                break;
            }

            case 0xc9:
            {
                uint8_t value = read_u8(cpu.pc);
                if(value <= cpu.a) {
                    cpu.status |= 1;
                } else {
                    cpu.statud &= 0xfe;
                }
                update_zero_neg(cpu.a - value);
                cpu.pc += 1;
                break;
            }
            case 0xc5:
            {
                uint8_t value = read_u8(((uint16_t)read_u8(cpu.pc)));
                if(value <= cpu.a) {
                    cpu.status |= 1;
                } else {
                    cpu.statud &= 0xfe;
                }
                update_zero_neg(cpu.a - value);
                cpu.pc += 1;
                break;
            }

            case 0xf0:
            {
                //printf("BEQ\n");
                BRANCH(cpu.status & 2);
                break;
            }
            case 0xd0:
            {
                BRANCH(!(cpu.status & 2));
                break;
            }
            case 0x10:
            {
                BRANCH(!(cpu.status & 0x80));
                break;
            }
            case 0xb0:
            {
                BRANCH(cpu.status & 1);
                break;
            }
            case 0x90:
            {
                BRANCH(!(cpu.status & 1));
                break;
            }

            case 0x24:
            {
                uint8_t value = read_u8(((uint16_t)read_u8(cpu.pc)));
                if(!(cpu.a & value)) {
                    cpu.status |= 2;
                } else {
                    cpu.status &= 0xfd;
                }

                if(value & 0x80) {
                    cpu.status |= 0x80;
                } else {
                    cpu.status &= 0x7f;
                }
                if(value & 0x40) {
                    cpu.status |= 0x40;
                } else {
                    cpu.status &= 0xbf;
                }
                cpu.pc += 1;
                break;
            }

            case 0xe6:
            {
                cpu.mem[(uint16_t)(read_u8(cpu.pc))] += 1;
                update_zero_neg(cpu.mem[(uint16_t)(read_u8(cpu.pc))]);
                cpu.pc += 1;
                break;
            }

            case 0xe8:
            {
                cpu.x += 1;
                update_zero_neg(cpu.x);
                break;
            }

            case 0xa2:
            {
                cpu.x = read_u8(cpu.pc);
                update_zero_neg(cpu.x);
                cpu.pc += 1;
                break;
            }
            case 0xa6:
            {
                cpu.x = read_u8(((uint16_t)read_u8(cpu.pc)));
                update_zero_neg(cpu.x);
                cpu.pc += 1;
                break;
            }

            case 0xe4:
            {
                uint8_t value = read_u8(((uint16_t)read_u8(cpu.pc)));
                if(value <= cpu.x) {
                    cpu.status |= 1;
                } else {
                    cpu.statud &= 0xfe;
                }
                update_zero_neg(cpu.x - value);
                cpu.pc += 1;
                break;
            }

            case 0xca:
            {
                cpu.x -= 1;
                //printf("DEX x: 0x%x, x - 1: 0x%x\n", cpu.x + 1, cpu.x);
                update_zero_neg(cpu.x);
                break;
            }

            case 0x8a:
            {
                cpu.a = cpu.x;
                update_zero_neg(cpu.a);
                break;
            }

            case 0x4a:
            {
                cpu.status |= (cpu.a & 1);
                cpu.a = cpu.a >> 1;
                update_zero_neg(cpu.a);
                break;
            }

            case 0x38:
            {
                cpu.status |= 1;
                break;
            }

            case 0xe9:
            {
                uint8_t value = read_u8(cpu.pc);
                uint16_t sum = ((uint16_t)cpu.a) + ((uint16_t)((-((int8_t)value)) - 1)) + (cpu.status & 1);
                if(sum > 0xff) {
                    cpu.status |= 1;
                } else {
                    cpu.status &= 0xfe;
                }
                uint8_t result = (uint8_t)sum;
                if(((value ^ result) & (result ^ cpu.a) & 0x80) != 0) {
                    cpu.status |= 0x40;
                } else {
                    cpu.status &= 0xbf;
                }
                update_zero_neg(result);
                cpu.a = result;
                cpu.pc += 1;
                break;
            }

            case 0xc6:
            {
                cpu.mem[(uint16_t)(read_u8(cpu.pc))] -= 1;
                update_zero_neg(cpu.mem[(uint16_t)(read_u8(cpu.pc))]);
                cpu.pc += 1;
                break;
            }

            case 0xa0:
            {
                cpu.y = read_u8(cpu.pc);
                update_zero_neg(cpu.y);
                cpu.pc += 1;
                break;
            }

            case 0xea:
            {
                break;
            }

            default:
            {
                fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
                return 1;
            }
        }

        write_u8(0xFE, (rand() % 256)); // 256 being right above the 8-bit int max, so 0-255
        for(uint16_t i = 0x0200; i < 0x0600; i++) {
            setColor(read_u8(i));
            uint32_t y1 = ((i - 0x0200) / 32) * 10;
            uint32_t x1 = ((i - 0x0200) % 32) * 10;
            uint32_t x2 = x1 + 10;
            uint32_t y2 = y1 + 10;
            CNFGTackRectangle(x1, y1, x2, y2); 
        }
        
        CNFGSwapBuffers();

        struct timespec remaining, request = { 0, 50000000 };

        int response = 0; //nanosleep(&request, &remaining);

        if(response) {
            fprintf(stderr, "nanosleep failed with return %d!\n", response);
            return 1;
        }
    }
}