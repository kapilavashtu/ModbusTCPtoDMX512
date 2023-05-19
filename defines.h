#ifndef DEFINES_H
#define DEFINES_H

#include <inttypes.h>


//BITS OPS
#define SET_BIT(reg, bit, type)         	reg |= ((type)   1 << bit)
#define CLEAR_BIT(reg, bit, type)      		reg &= (~((type) 1 << bit))
#define INVERT_BIT(reg, bit, type)        	reg ^= ((type)   1 << bit)
#define BIT_IS_SET(reg, bit, type)        	((reg & ((type)  1 << bit)) != 0)
#define BIT_IS_CLEAR(reg, bit, type)      	((reg & ((type)  1 << bit)) == 0)

#define SET_PULSE_ON          (uint16_t) 0b0000000000001101
#define SET_PULSE_OFF         (uint16_t) 0b0000000000001001

#define PULSE_NUM_MASK        (uint16_t) 0b0000111100000000
#define PULSE_CHANNEL_MASK    (uint16_t) 0b0000000011111111

#endif