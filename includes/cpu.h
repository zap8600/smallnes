#ifndef CPU_H
#define CPU_H

typedef struct CPU {
    uint16_t pc;
    uint8_t ac, x, y, sr, sp, flags; // In a VRC Shader, the PC and these registers only require half of a pixel to be stored
    uint8_t mem[2048]; // This only takes 128 pixels
    uint8_t *code;
} CPU;

#endif