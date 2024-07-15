#include "lcd.h"
#include "systick.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_fsmc.h"
#include "stm32f10x_gpio.h"

void lcd_gpio_init(void);
void lcd_reg_init(void);

void lcd_init(void)
{

    // 等待上电
    for (uint32_t i = 0; i < 72 * 1000 * 100; i++) {}

    // GPIO初始化
    lcd_gpio_init();

    // 开启 总线时钟-FSMC 使能
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    /** 初始 FSMC - SRAM 配置 **/
    // 初始化 FSMC 模式
    FSMC_NORSRAMInitTypeDef FSMC_InitAttr;
    FSMC_NORSRAMTimingInitTypeDef FSMC_ReadTimingAttr;
    FSMC_InitAttr.FSMC_ReadWriteTimingStruct = &FSMC_ReadTimingAttr;
    FSMC_InitAttr.FSMC_WriteTimingStruct     = &FSMC_ReadTimingAttr;
    FSMC_NORSRAMStructInit(&FSMC_InitAttr);

    FSMC_InitAttr.FSMC_Bank            = FSMC_Bank1_NORSRAM4;
    FSMC_InitAttr.FSMC_MemoryType      = FSMC_MemoryType_SRAM;
    FSMC_InitAttr.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_InitAttr.FSMC_WriteOperation  = FSMC_WriteOperation_Enable;
    FSMC_InitAttr.FSMC_ExtendedMode    = FSMC_ExtendedMode_Disable;

    // 初始化 FSMC 读时间配置
    FSMC_ReadTimingAttr.FSMC_BusTurnAroundDuration = 0;
    FSMC_ReadTimingAttr.FSMC_AccessMode            = FSMC_AccessMode_A;
    FSMC_ReadTimingAttr.FSMC_AddressSetupTime      = 0;
    FSMC_ReadTimingAttr.FSMC_DataSetupTime         = 2;

    // 调用 FSMC 初始化函数
    FSMC_NORSRAMInit(&FSMC_InitAttr);

    // 使能 FSMC分组
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);

    // delay 50 ms
    delay_ms(50);

    // 开启背光
    GPIO_SetBits(GPIOB, GPIO_Pin_0);

    // 发送厂家秘钥读取id
    LCD_WriteReg(0xF000, 0x0055);
    LCD_WriteReg(0xF001, 0x00AA);
    LCD_WriteReg(0xF002, 0x0052);
    LCD_WriteReg(0xF003, 0x0008);
    LCD_WriteReg(0xF004, 0x0001);

    LCD_WR_REG(0xC500);
    uint32_t id = LCD_RD_DATA();
    id <<= 8;

    LCD_WR_REG(0xC501);
    id |= LCD_RD_DATA();

    if (id != 0x5510) {
        return;
    }

    // 配置初始化
    lcd_reg_init();
}

// 写指令
void LCD_WR_REG(uint16_t regval)
{
    LCD->reg = regval; // 写入要写的寄存器序号
}

// 写LCD数据
void LCD_WR_DATA(uint16_t data)
{
    LCD->ram = data;
}

// 读LCD数据
uint16_t LCD_RD_DATA(void)
{
    return LCD->ram;
}

// 写指令 + 数据
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    LCD->reg = LCD_Reg;      // 写入要写的寄存器序号
    LCD->ram = LCD_RegValue; // 写入数据
}

void lcd_gpio_init(void)
{

    RCC_APB2PeriphClockCmd(
        /** 地址口 FSMC_A 0-18 **/
        /** 数据口 FSMC_D 0-15 **/
        /** 通信控制使能口 NE3(CS)、NBL(UB、LB)、NOE(OE)、NWE(WE)**/
        RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitAttr;

    GPIO_InitAttr.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitAttr.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitAttr.GPIO_Pin   = GPIO_Pin_0;
    GPIO_Init(GPIOB, &GPIO_InitAttr); /** LCD_BL背光 */

    GPIO_InitAttr.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitAttr.GPIO_Pin  = GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_0 | GPIO_Pin_1 |
                             GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                             GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOD, &GPIO_InitAttr); /**  FSMC_D 0 - 3, FSMC_D 13 - 15, 控制 FSMC_NOE, NWE*/

    GPIO_InitAttr.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
                             GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
                             GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitAttr); /** 初始化 FSMC_D 4 - 12*/

    GPIO_InitAttr.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;
    GPIO_Init(GPIOG, &GPIO_InitAttr); /** 初始化 FSMC_A 10, 控制 FSMC_NE4*/
}

void LCD_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t size)
{
    uint8_t xdir   = x2 > x1;
    uint16_t vx    = xdir ? x2 - x1 : x1 - x2;
    uint8_t ydir   = y2 > y1;
    uint16_t vy    = ydir ? y2 - y1 : y1 - y2;

    uint32_t count = vx * vy;
    if (vx == 0) {
        count = vy;
    }
    if (vy == 0) {
        count = vx;
    }

    uint8_t flag = 1;
    for (uint32_t i = 0; i < count; i++) {

        if (i % vx == 0) {
            y1 += ydir ? 1 : -1;
            flag = 1;
        }

        if (i % vy == 0) {
            x1 += xdir ? 1 : -1;
            flag = 1;
        }

        if (flag) {
            flag = 0;
            lcd_set_window(x1, y1, size, size);
            LCD_WR_REG(0x2C00);
            for (uint8_t j = 0; j < size * size; j++) {
                LCD_WR_DATA(color);
            }
        }
    }
}

void LCD_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *colors)
{
    // 选中窗口
    uint16_t width  = x2 - x1 + 1;
    uint16_t height = y2 - y1 + 1;
    lcd_set_window(x1, y1, width, height);

    // 写入颜色
    LCD_WR_REG(0x2C00);
    for (uint32_t i = 0; i < width * height; i++) {
        LCD_WR_DATA(*colors);
        colors++;
    }
}

void lcd_reg_init(void)
{
    /** LCD 初始化配置 **/
    LCD_WriteReg(0xF000, 0x55);
    LCD_WriteReg(0xF001, 0xAA);
    LCD_WriteReg(0xF002, 0x52);
    LCD_WriteReg(0xF003, 0x08);
    LCD_WriteReg(0xF004, 0x01);
    // AVDD Set AVDD 5.2V
    LCD_WriteReg(0xB000, 0x0D);
    LCD_WriteReg(0xB001, 0x0D);
    LCD_WriteReg(0xB002, 0x0D);
    // AVDD ratio
    LCD_WriteReg(0xB600, 0x34);
    LCD_WriteReg(0xB601, 0x34);
    LCD_WriteReg(0xB602, 0x34);
    // AVEE -5.2V
    LCD_WriteReg(0xB100, 0x0D);
    LCD_WriteReg(0xB101, 0x0D);
    LCD_WriteReg(0xB102, 0x0D);
    // AVEE ratio
    LCD_WriteReg(0xB700, 0x34);
    LCD_WriteReg(0xB701, 0x34);
    LCD_WriteReg(0xB702, 0x34);
    // VCL -2.5V
    LCD_WriteReg(0xB200, 0x00);
    LCD_WriteReg(0xB201, 0x00);
    LCD_WriteReg(0xB202, 0x00);
    // VCL ratio
    LCD_WriteReg(0xB800, 0x24);
    LCD_WriteReg(0xB801, 0x24);
    LCD_WriteReg(0xB802, 0x24);
    // VGH 15V (Free pump)
    LCD_WriteReg(0xBF00, 0x01);
    LCD_WriteReg(0xB300, 0x0F);
    LCD_WriteReg(0xB301, 0x0F);
    LCD_WriteReg(0xB302, 0x0F);
    // VGH ratio
    LCD_WriteReg(0xB900, 0x34);
    LCD_WriteReg(0xB901, 0x34);
    LCD_WriteReg(0xB902, 0x34);
    // VGL_REG -10V
    LCD_WriteReg(0xB500, 0x08);
    LCD_WriteReg(0xB501, 0x08);
    LCD_WriteReg(0xB502, 0x08);
    LCD_WriteReg(0xC200, 0x03);
    // VGLX ratio
    LCD_WriteReg(0xBA00, 0x24);
    LCD_WriteReg(0xBA01, 0x24);
    LCD_WriteReg(0xBA02, 0x24);
    // VGMP/VGSP 4.5V/0V
    LCD_WriteReg(0xBC00, 0x00);
    LCD_WriteReg(0xBC01, 0x78);
    LCD_WriteReg(0xBC02, 0x00);
    // VGMN/VGSN -4.5V/0V
    LCD_WriteReg(0xBD00, 0x00);
    LCD_WriteReg(0xBD01, 0x78);
    LCD_WriteReg(0xBD02, 0x00);
    // VCOM
    LCD_WriteReg(0xBE00, 0x00);
    LCD_WriteReg(0xBE01, 0x64);
    // Gamma Setting
    LCD_WriteReg(0xD100, 0x00);
    LCD_WriteReg(0xD101, 0x33);
    LCD_WriteReg(0xD102, 0x00);
    LCD_WriteReg(0xD103, 0x34);
    LCD_WriteReg(0xD104, 0x00);
    LCD_WriteReg(0xD105, 0x3A);
    LCD_WriteReg(0xD106, 0x00);
    LCD_WriteReg(0xD107, 0x4A);
    LCD_WriteReg(0xD108, 0x00);
    LCD_WriteReg(0xD109, 0x5C);
    LCD_WriteReg(0xD10A, 0x00);
    LCD_WriteReg(0xD10B, 0x81);
    LCD_WriteReg(0xD10C, 0x00);
    LCD_WriteReg(0xD10D, 0xA6);
    LCD_WriteReg(0xD10E, 0x00);
    LCD_WriteReg(0xD10F, 0xE5);
    LCD_WriteReg(0xD110, 0x01);
    LCD_WriteReg(0xD111, 0x13);
    LCD_WriteReg(0xD112, 0x01);
    LCD_WriteReg(0xD113, 0x54);
    LCD_WriteReg(0xD114, 0x01);
    LCD_WriteReg(0xD115, 0x82);
    LCD_WriteReg(0xD116, 0x01);
    LCD_WriteReg(0xD117, 0xCA);
    LCD_WriteReg(0xD118, 0x02);
    LCD_WriteReg(0xD119, 0x00);
    LCD_WriteReg(0xD11A, 0x02);
    LCD_WriteReg(0xD11B, 0x01);
    LCD_WriteReg(0xD11C, 0x02);
    LCD_WriteReg(0xD11D, 0x34);
    LCD_WriteReg(0xD11E, 0x02);
    LCD_WriteReg(0xD11F, 0x67);
    LCD_WriteReg(0xD120, 0x02);
    LCD_WriteReg(0xD121, 0x84);
    LCD_WriteReg(0xD122, 0x02);
    LCD_WriteReg(0xD123, 0xA4);
    LCD_WriteReg(0xD124, 0x02);
    LCD_WriteReg(0xD125, 0xB7);
    LCD_WriteReg(0xD126, 0x02);
    LCD_WriteReg(0xD127, 0xCF);
    LCD_WriteReg(0xD128, 0x02);
    LCD_WriteReg(0xD129, 0xDE);
    LCD_WriteReg(0xD12A, 0x02);
    LCD_WriteReg(0xD12B, 0xF2);
    LCD_WriteReg(0xD12C, 0x02);
    LCD_WriteReg(0xD12D, 0xFE);
    LCD_WriteReg(0xD12E, 0x03);
    LCD_WriteReg(0xD12F, 0x10);
    LCD_WriteReg(0xD130, 0x03);
    LCD_WriteReg(0xD131, 0x33);
    LCD_WriteReg(0xD132, 0x03);
    LCD_WriteReg(0xD133, 0x6D);
    LCD_WriteReg(0xD200, 0x00);
    LCD_WriteReg(0xD201, 0x33);
    LCD_WriteReg(0xD202, 0x00);
    LCD_WriteReg(0xD203, 0x34);
    LCD_WriteReg(0xD204, 0x00);
    LCD_WriteReg(0xD205, 0x3A);
    LCD_WriteReg(0xD206, 0x00);
    LCD_WriteReg(0xD207, 0x4A);
    LCD_WriteReg(0xD208, 0x00);
    LCD_WriteReg(0xD209, 0x5C);
    LCD_WriteReg(0xD20A, 0x00);

    LCD_WriteReg(0xD20B, 0x81);
    LCD_WriteReg(0xD20C, 0x00);
    LCD_WriteReg(0xD20D, 0xA6);
    LCD_WriteReg(0xD20E, 0x00);
    LCD_WriteReg(0xD20F, 0xE5);
    LCD_WriteReg(0xD210, 0x01);
    LCD_WriteReg(0xD211, 0x13);
    LCD_WriteReg(0xD212, 0x01);
    LCD_WriteReg(0xD213, 0x54);
    LCD_WriteReg(0xD214, 0x01);
    LCD_WriteReg(0xD215, 0x82);
    LCD_WriteReg(0xD216, 0x01);
    LCD_WriteReg(0xD217, 0xCA);
    LCD_WriteReg(0xD218, 0x02);
    LCD_WriteReg(0xD219, 0x00);
    LCD_WriteReg(0xD21A, 0x02);
    LCD_WriteReg(0xD21B, 0x01);
    LCD_WriteReg(0xD21C, 0x02);
    LCD_WriteReg(0xD21D, 0x34);
    LCD_WriteReg(0xD21E, 0x02);
    LCD_WriteReg(0xD21F, 0x67);
    LCD_WriteReg(0xD220, 0x02);
    LCD_WriteReg(0xD221, 0x84);
    LCD_WriteReg(0xD222, 0x02);
    LCD_WriteReg(0xD223, 0xA4);
    LCD_WriteReg(0xD224, 0x02);
    LCD_WriteReg(0xD225, 0xB7);
    LCD_WriteReg(0xD226, 0x02);
    LCD_WriteReg(0xD227, 0xCF);
    LCD_WriteReg(0xD228, 0x02);
    LCD_WriteReg(0xD229, 0xDE);
    LCD_WriteReg(0xD22A, 0x02);
    LCD_WriteReg(0xD22B, 0xF2);
    LCD_WriteReg(0xD22C, 0x02);
    LCD_WriteReg(0xD22D, 0xFE);
    LCD_WriteReg(0xD22E, 0x03);
    LCD_WriteReg(0xD22F, 0x10);
    LCD_WriteReg(0xD230, 0x03);
    LCD_WriteReg(0xD231, 0x33);
    LCD_WriteReg(0xD232, 0x03);
    LCD_WriteReg(0xD233, 0x6D);
    LCD_WriteReg(0xD300, 0x00);
    LCD_WriteReg(0xD301, 0x33);
    LCD_WriteReg(0xD302, 0x00);
    LCD_WriteReg(0xD303, 0x34);
    LCD_WriteReg(0xD304, 0x00);
    LCD_WriteReg(0xD305, 0x3A);
    LCD_WriteReg(0xD306, 0x00);
    LCD_WriteReg(0xD307, 0x4A);
    LCD_WriteReg(0xD308, 0x00);
    LCD_WriteReg(0xD309, 0x5C);
    LCD_WriteReg(0xD30A, 0x00);

    LCD_WriteReg(0xD30B, 0x81);
    LCD_WriteReg(0xD30C, 0x00);
    LCD_WriteReg(0xD30D, 0xA6);
    LCD_WriteReg(0xD30E, 0x00);
    LCD_WriteReg(0xD30F, 0xE5);
    LCD_WriteReg(0xD310, 0x01);
    LCD_WriteReg(0xD311, 0x13);
    LCD_WriteReg(0xD312, 0x01);
    LCD_WriteReg(0xD313, 0x54);
    LCD_WriteReg(0xD314, 0x01);
    LCD_WriteReg(0xD315, 0x82);
    LCD_WriteReg(0xD316, 0x01);
    LCD_WriteReg(0xD317, 0xCA);
    LCD_WriteReg(0xD318, 0x02);
    LCD_WriteReg(0xD319, 0x00);
    LCD_WriteReg(0xD31A, 0x02);
    LCD_WriteReg(0xD31B, 0x01);
    LCD_WriteReg(0xD31C, 0x02);
    LCD_WriteReg(0xD31D, 0x34);
    LCD_WriteReg(0xD31E, 0x02);
    LCD_WriteReg(0xD31F, 0x67);
    LCD_WriteReg(0xD320, 0x02);
    LCD_WriteReg(0xD321, 0x84);
    LCD_WriteReg(0xD322, 0x02);
    LCD_WriteReg(0xD323, 0xA4);
    LCD_WriteReg(0xD324, 0x02);
    LCD_WriteReg(0xD325, 0xB7);
    LCD_WriteReg(0xD326, 0x02);
    LCD_WriteReg(0xD327, 0xCF);
    LCD_WriteReg(0xD328, 0x02);
    LCD_WriteReg(0xD329, 0xDE);
    LCD_WriteReg(0xD32A, 0x02);
    LCD_WriteReg(0xD32B, 0xF2);
    LCD_WriteReg(0xD32C, 0x02);
    LCD_WriteReg(0xD32D, 0xFE);
    LCD_WriteReg(0xD32E, 0x03);
    LCD_WriteReg(0xD32F, 0x10);
    LCD_WriteReg(0xD330, 0x03);
    LCD_WriteReg(0xD331, 0x33);
    LCD_WriteReg(0xD332, 0x03);
    LCD_WriteReg(0xD333, 0x6D);
    LCD_WriteReg(0xD400, 0x00);
    LCD_WriteReg(0xD401, 0x33);
    LCD_WriteReg(0xD402, 0x00);
    LCD_WriteReg(0xD403, 0x34);
    LCD_WriteReg(0xD404, 0x00);
    LCD_WriteReg(0xD405, 0x3A);
    LCD_WriteReg(0xD406, 0x00);
    LCD_WriteReg(0xD407, 0x4A);
    LCD_WriteReg(0xD408, 0x00);
    LCD_WriteReg(0xD409, 0x5C);
    LCD_WriteReg(0xD40A, 0x00);
    LCD_WriteReg(0xD40B, 0x81);

    LCD_WriteReg(0xD40C, 0x00);
    LCD_WriteReg(0xD40D, 0xA6);
    LCD_WriteReg(0xD40E, 0x00);
    LCD_WriteReg(0xD40F, 0xE5);
    LCD_WriteReg(0xD410, 0x01);
    LCD_WriteReg(0xD411, 0x13);
    LCD_WriteReg(0xD412, 0x01);
    LCD_WriteReg(0xD413, 0x54);
    LCD_WriteReg(0xD414, 0x01);
    LCD_WriteReg(0xD415, 0x82);
    LCD_WriteReg(0xD416, 0x01);
    LCD_WriteReg(0xD417, 0xCA);
    LCD_WriteReg(0xD418, 0x02);
    LCD_WriteReg(0xD419, 0x00);
    LCD_WriteReg(0xD41A, 0x02);
    LCD_WriteReg(0xD41B, 0x01);
    LCD_WriteReg(0xD41C, 0x02);
    LCD_WriteReg(0xD41D, 0x34);
    LCD_WriteReg(0xD41E, 0x02);
    LCD_WriteReg(0xD41F, 0x67);
    LCD_WriteReg(0xD420, 0x02);
    LCD_WriteReg(0xD421, 0x84);
    LCD_WriteReg(0xD422, 0x02);
    LCD_WriteReg(0xD423, 0xA4);
    LCD_WriteReg(0xD424, 0x02);
    LCD_WriteReg(0xD425, 0xB7);
    LCD_WriteReg(0xD426, 0x02);
    LCD_WriteReg(0xD427, 0xCF);
    LCD_WriteReg(0xD428, 0x02);
    LCD_WriteReg(0xD429, 0xDE);
    LCD_WriteReg(0xD42A, 0x02);
    LCD_WriteReg(0xD42B, 0xF2);
    LCD_WriteReg(0xD42C, 0x02);
    LCD_WriteReg(0xD42D, 0xFE);
    LCD_WriteReg(0xD42E, 0x03);
    LCD_WriteReg(0xD42F, 0x10);
    LCD_WriteReg(0xD430, 0x03);
    LCD_WriteReg(0xD431, 0x33);
    LCD_WriteReg(0xD432, 0x03);
    LCD_WriteReg(0xD433, 0x6D);
    LCD_WriteReg(0xD500, 0x00);
    LCD_WriteReg(0xD501, 0x33);
    LCD_WriteReg(0xD502, 0x00);
    LCD_WriteReg(0xD503, 0x34);
    LCD_WriteReg(0xD504, 0x00);
    LCD_WriteReg(0xD505, 0x3A);
    LCD_WriteReg(0xD506, 0x00);
    LCD_WriteReg(0xD507, 0x4A);
    LCD_WriteReg(0xD508, 0x00);
    LCD_WriteReg(0xD509, 0x5C);
    LCD_WriteReg(0xD50A, 0x00);
    LCD_WriteReg(0xD50B, 0x81);

    LCD_WriteReg(0xD50C, 0x00);
    LCD_WriteReg(0xD50D, 0xA6);
    LCD_WriteReg(0xD50E, 0x00);
    LCD_WriteReg(0xD50F, 0xE5);
    LCD_WriteReg(0xD510, 0x01);
    LCD_WriteReg(0xD511, 0x13);
    LCD_WriteReg(0xD512, 0x01);
    LCD_WriteReg(0xD513, 0x54);
    LCD_WriteReg(0xD514, 0x01);
    LCD_WriteReg(0xD515, 0x82);
    LCD_WriteReg(0xD516, 0x01);
    LCD_WriteReg(0xD517, 0xCA);
    LCD_WriteReg(0xD518, 0x02);
    LCD_WriteReg(0xD519, 0x00);
    LCD_WriteReg(0xD51A, 0x02);
    LCD_WriteReg(0xD51B, 0x01);
    LCD_WriteReg(0xD51C, 0x02);
    LCD_WriteReg(0xD51D, 0x34);
    LCD_WriteReg(0xD51E, 0x02);
    LCD_WriteReg(0xD51F, 0x67);
    LCD_WriteReg(0xD520, 0x02);
    LCD_WriteReg(0xD521, 0x84);
    LCD_WriteReg(0xD522, 0x02);
    LCD_WriteReg(0xD523, 0xA4);
    LCD_WriteReg(0xD524, 0x02);
    LCD_WriteReg(0xD525, 0xB7);
    LCD_WriteReg(0xD526, 0x02);
    LCD_WriteReg(0xD527, 0xCF);
    LCD_WriteReg(0xD528, 0x02);
    LCD_WriteReg(0xD529, 0xDE);
    LCD_WriteReg(0xD52A, 0x02);
    LCD_WriteReg(0xD52B, 0xF2);
    LCD_WriteReg(0xD52C, 0x02);
    LCD_WriteReg(0xD52D, 0xFE);
    LCD_WriteReg(0xD52E, 0x03);
    LCD_WriteReg(0xD52F, 0x10);
    LCD_WriteReg(0xD530, 0x03);
    LCD_WriteReg(0xD531, 0x33);
    LCD_WriteReg(0xD532, 0x03);
    LCD_WriteReg(0xD533, 0x6D);
    LCD_WriteReg(0xD600, 0x00);
    LCD_WriteReg(0xD601, 0x33);
    LCD_WriteReg(0xD602, 0x00);
    LCD_WriteReg(0xD603, 0x34);
    LCD_WriteReg(0xD604, 0x00);
    LCD_WriteReg(0xD605, 0x3A);
    LCD_WriteReg(0xD606, 0x00);
    LCD_WriteReg(0xD607, 0x4A);
    LCD_WriteReg(0xD608, 0x00);
    LCD_WriteReg(0xD609, 0x5C);
    LCD_WriteReg(0xD60A, 0x00);
    LCD_WriteReg(0xD60B, 0x81);

    LCD_WriteReg(0xD60C, 0x00);
    LCD_WriteReg(0xD60D, 0xA6);
    LCD_WriteReg(0xD60E, 0x00);
    LCD_WriteReg(0xD60F, 0xE5);
    LCD_WriteReg(0xD610, 0x01);
    LCD_WriteReg(0xD611, 0x13);
    LCD_WriteReg(0xD612, 0x01);
    LCD_WriteReg(0xD613, 0x54);
    LCD_WriteReg(0xD614, 0x01);
    LCD_WriteReg(0xD615, 0x82);
    LCD_WriteReg(0xD616, 0x01);
    LCD_WriteReg(0xD617, 0xCA);
    LCD_WriteReg(0xD618, 0x02);
    LCD_WriteReg(0xD619, 0x00);
    LCD_WriteReg(0xD61A, 0x02);
    LCD_WriteReg(0xD61B, 0x01);
    LCD_WriteReg(0xD61C, 0x02);
    LCD_WriteReg(0xD61D, 0x34);
    LCD_WriteReg(0xD61E, 0x02);
    LCD_WriteReg(0xD61F, 0x67);
    LCD_WriteReg(0xD620, 0x02);
    LCD_WriteReg(0xD621, 0x84);
    LCD_WriteReg(0xD622, 0x02);
    LCD_WriteReg(0xD623, 0xA4);
    LCD_WriteReg(0xD624, 0x02);
    LCD_WriteReg(0xD625, 0xB7);
    LCD_WriteReg(0xD626, 0x02);
    LCD_WriteReg(0xD627, 0xCF);
    LCD_WriteReg(0xD628, 0x02);
    LCD_WriteReg(0xD629, 0xDE);
    LCD_WriteReg(0xD62A, 0x02);
    LCD_WriteReg(0xD62B, 0xF2);
    LCD_WriteReg(0xD62C, 0x02);
    LCD_WriteReg(0xD62D, 0xFE);
    LCD_WriteReg(0xD62E, 0x03);
    LCD_WriteReg(0xD62F, 0x10);
    LCD_WriteReg(0xD630, 0x03);
    LCD_WriteReg(0xD631, 0x33);
    LCD_WriteReg(0xD632, 0x03);
    LCD_WriteReg(0xD633, 0x6D);
    // LV2 Page 0 enable
    LCD_WriteReg(0xF000, 0x55);
    LCD_WriteReg(0xF001, 0xAA);
    LCD_WriteReg(0xF002, 0x52);
    LCD_WriteReg(0xF003, 0x08);
    LCD_WriteReg(0xF004, 0x00);
    // Display control
    LCD_WriteReg(0xB100, 0xCC);
    LCD_WriteReg(0xB101, 0x00);
    // Source hold time
    LCD_WriteReg(0xB600, 0x05);
    // Gate EQ control
    LCD_WriteReg(0xB700, 0x70);
    LCD_WriteReg(0xB701, 0x70);
    // Source EQ control (Mode 2)
    LCD_WriteReg(0xB800, 0x01);
    LCD_WriteReg(0xB801, 0x03);
    LCD_WriteReg(0xB802, 0x03);
    LCD_WriteReg(0xB803, 0x03);
    // Inversion mode (2-dot)
    LCD_WriteReg(0xBC00, 0x02);
    LCD_WriteReg(0xBC01, 0x00);
    LCD_WriteReg(0xBC02, 0x00);
    // Timing control 4H w/ 4-delay
    LCD_WriteReg(0xC900, 0xD0);
    LCD_WriteReg(0xC901, 0x02);
    LCD_WriteReg(0xC902, 0x50);
    LCD_WriteReg(0xC903, 0x50);
    LCD_WriteReg(0xC904, 0x50);
    LCD_WriteReg(0x3500, 0x00);
    LCD_WriteReg(0x3A00, 0x55); // 16-bit/pixel
    LCD_WR_REG(0x1100);

    // delay 120 us
    for (uint32_t i = 0; i < 72 * 120; i++) {}

    LCD_WR_REG(0x2900); // display on

    // 设置方向
    LCD_WriteReg(0X3600, (0 << 7) | (0 << 6) | (0 << 5));

    lcd_set_window(0, 0, 480, 800);
}

void lcd_set_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    // 设置窗口 X
    LCD_WR_REG(0x2A00);
    LCD_WR_DATA(x >> 8);
    LCD_WR_REG(0x2A01);
    LCD_WR_DATA(x & 0xFF);
    LCD_WR_REG(0x2A02);
    LCD_WR_DATA((x + width - 1) >> 8);
    LCD_WR_REG(0x2A03);
    LCD_WR_DATA((x + width - 1) & 0xFF);

    // 设置窗口 Y
    LCD_WR_REG(0x2B00);
    LCD_WR_DATA(y >> 8);
    LCD_WR_REG(0x2B01);
    LCD_WR_DATA(y & 0xFF);
    LCD_WR_REG(0x2B02);
    LCD_WR_DATA((y + height - 1) >> 8);
    LCD_WR_REG(0x2B03);
    LCD_WR_DATA((y + height - 1) & 0xFF);
}