#ifndef __LCD_H__
#define __LCD_H__

#include "stm32f10x.h"

#define FSMC_SRAM_BASE_ADDR  0x60000000

#define FSMC_SRAM1_BASE_ADDR (FSMC_SRAM_BASE_ADDR)
#define FSMC_SRAM2_BASE_ADDR (FSMC_SRAM_BASE_ADDR | 0x4000000)
#define FSMC_SRAM3_BASE_ADDR (FSMC_SRAM_BASE_ADDR | 0x8000000)
#define FSMC_SRAM4_BASE_ADDR (FSMC_SRAM_BASE_ADDR | 0xC000000)

#define LCD_ADDR_BASE        (FSMC_SRAM4_BASE_ADDR | 0x07FE)

typedef struct {
    __IO uint16_t reg;
    __IO uint16_t ram;
} LCD_TypeDef;

#define LCD ((LCD_TypeDef *)LCD_ADDR_BASE)

void lcd_init(void);
void lcd_set_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void LCD_WR_REG(uint16_t regval);
void LCD_WR_DATA(uint16_t data);
uint16_t LCD_RD_DATA(void);
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);

void LCD_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t size);
void LCD_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t * colors);

#endif