#include "gt911.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "systick.h"
#include "string.h"

#define gt911_CLK_SET()   (GPIO_SetBits(GPIOB, GPIO_Pin_1))
#define gt911_CLK_RESET() (GPIO_ResetBits(GPIOB, GPIO_Pin_1))
#define gt911_SDA_SET()   (GPIO_SetBits(GPIOF, GPIO_Pin_9))
#define gt911_SDA_RESET() (GPIO_ResetBits(GPIOF, GPIO_Pin_9))
#define gt911_SDA_READ()  (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_9))
#define gt911_iic_delay() (delay_us(1))

const uint8_t GT91x_CFG_TBL[] =
    {
        0X00,
        0XE0,
        0X01,
        0X20,
        0X03,
        0X05,
        0X35,
        0X00,
        0X02,
        0X08,
        0X1E,
        0X08,
        0X50,
        0X3C,
        0X0F,
        0X05,
        0X00,
        0X00,
        0XFF,
        0X67,
        0X50,
        0X00,
        0X00,
        0X18,
        0X1A,
        0X1E,
        0X14,
        0X89,
        0X28,
        0X0A,
        0X30,
        0X2E,
        0XBB,
        0X0A,
        0X03,
        0X00,
        0X00,
        0X02,
        0X33,
        0X1D,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X32,
        0X00,
        0X00,
        0X2A,
        0X1C,
        0X5A,
        0X94,
        0XC5,
        0X02,
        0X07,
        0X00,
        0X00,
        0X00,
        0XB5,
        0X1F,
        0X00,
        0X90,
        0X28,
        0X00,
        0X77,
        0X32,
        0X00,
        0X62,
        0X3F,
        0X00,
        0X52,
        0X50,
        0X00,
        0X52,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X0F,
        0X0F,
        0X03,
        0X06,
        0X10,
        0X42,
        0XF8,
        0X0F,
        0X14,
        0X00,
        0X00,
        0X00,
        0X00,
        0X1A,
        0X18,
        0X16,
        0X14,
        0X12,
        0X10,
        0X0E,
        0X0C,
        0X0A,
        0X08,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X29,
        0X28,
        0X24,
        0X22,
        0X20,
        0X1F,
        0X1E,
        0X1D,
        0X0E,
        0X0C,
        0X0A,
        0X08,
        0X06,
        0X05,
        0X04,
        0X02,
        0X00,
        0XFF,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0X00,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
        0XFF,
};

const uint16_t GT911_TOUCH_ADDR_REG[] = {0X8150, 0X8158, 0X8160, 0X8168, 0X8170};

GPIO_InitTypeDef gt911_gpio_attr;

GT911_touch_point gt911_touch_point[5];

void gt911_iic_start(void);
void gt911_iic_write(uint8_t);
uint8_t gt911_iic_read(void);
void gt911_iic_sendAck(uint8_t);
uint8_t gt911_iic_readAck(void);
void gt911_iic_end(void);
void gt911_config(void);

void gt911_checkId(void);

void gt911_write(uint16_t addr, const uint8_t *buf, uint16_t len);
void gt911_read(uint16_t addr, uint8_t *buf, uint16_t len);

/**
 * 状态寄存器（0X814E）
 * 坐标数据寄存器 (0X8150 + 29)
 */
GT911_touch_point *gt911_scan(void)
{

    uint8_t buf[4];
    gt911_read(0x814E, buf, 1);
    uint8_t flag  = buf[0] & 0x80;
    uint8_t count = buf[0] & 0x0F;
    if (!(flag && count)) {
        buf[0] = 0;
        gt911_write(0x814E, buf, 1);
        return NULL;
    }
    for (uint8_t i = 0; i < 5; i++) {
        gt911_touch_point[i].isTouched = 0;
    }
    for (uint8_t i = 0; i < count; i++) {

        gt911_read(GT911_TOUCH_ADDR_REG[i], buf, 4);

        gt911_touch_point[i].isTouched = 1;
        gt911_touch_point[i].x         = buf[1];
        gt911_touch_point[i].x         = gt911_touch_point[i].x << 8;
        gt911_touch_point[i].x         = gt911_touch_point[i].x + buf[0];

        gt911_touch_point[i].y         = buf[3];
        gt911_touch_point[i].y         = gt911_touch_point[i].y << 8;
        gt911_touch_point[i].y         = gt911_touch_point[i].y + buf[2];
    }
    buf[0] = 0;
    gt911_write(0x814E, buf, 1);
    return gt911_touch_point;
}

void gt911_init(void)
{
    //时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOB, ENABLE);

    // io口使能
    gt911_gpio_attr.GPIO_Mode  = GPIO_Mode_Out_PP;
    gt911_gpio_attr.GPIO_Pin   = GPIO_Pin_1;
    gt911_gpio_attr.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gt911_gpio_attr);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);

    gt911_gpio_attr.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOF, &gt911_gpio_attr);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);
    GPIO_SetBits(GPIOB, GPIO_Pin_10);
    GPIO_SetBits(GPIOB, GPIO_Pin_11);

    /** 复位并设置gt911与IIC的通讯地址*/
    //硬件复位
    GPIO_ResetBits(GPIOF, GPIO_Pin_11);
    delay_ms(10);
    GPIO_SetBits(GPIOF, GPIO_Pin_11);
    delay_ms(10);

    //将INT转为浮空输入模式
    gt911_gpio_attr.GPIO_Mode = GPIO_Mode_IPD;
    gt911_gpio_attr.GPIO_Pin  = GPIO_Pin_10;
    GPIO_Init(GPIOF, &gt911_gpio_attr);

    delay_ms(100);

    gt911_checkId();
    gt911_config();
}

/*
控制命令寄存器（0X8040）
配置寄存器组（0X8047~0X8100）
该寄存器可以写入不同值，实现不同的控制，我们一般使用 0 和 2 这两个值，写入 2，即
可软复位 gt911，在硬复位之后，一般要往该寄存器写 2，实行软复位。然后，写入 0，即可
正常读取坐标数据（并且会结束软复位）
*/
void gt911_config(void)
{
    uint8_t buf[1] = {2};
    gt911_write(0x8040, buf, 1);
    gt911_write(0x8047, GT91x_CFG_TBL, sizeof(GT91x_CFG_TBL) / sizeof(u8));
    buf[0] = 0;
    gt911_write(0x8040, buf, 1);
}

void gt911_checkId(void)
{
    uint8_t buf[4];
    gt911_read(0x8140, buf, 3);
    buf[3] = '\0';
    if (strcmp((char *)buf, "911")) {
       // BEEP_ALARM(5);
    }
}

/** gt911 发送写指令 **/
void gt911_write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    gt911_iic_start();
    gt911_iic_write(0xBA);
    gt911_iic_readAck();

    gt911_iic_write(addr >> 8);
    gt911_iic_readAck();

    gt911_iic_write(addr & 0xFF);
    gt911_iic_readAck();

    for (uint16_t i = 0; i < len; i++) {
        gt911_iic_write(buf[i]);
        gt911_iic_readAck();
    }

    gt911_iic_end();
}

/** gt911 读取从设备数据 **/
void gt911_read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    gt911_write(addr, NULL, 0);
    gt911_iic_start();
    gt911_iic_write(0xBB);
    gt911_iic_readAck();

    for (uint16_t i = 0; i < len; i++) {
        buf[i] = gt911_iic_read();
        gt911_iic_sendAck(0);
    }

    gt911_iic_end();
}

/** IIC 通信函数 **/

typedef enum {
    gt911_IIC_MODE_READ,
    gt911_IIC_MODE_WRITE,
} gt911_IIC_MODE;

void gt911_iic_switch_mode(gt911_IIC_MODE mode)
{
    gt911_gpio_attr.GPIO_Mode = (mode == gt911_IIC_MODE_READ ? GPIO_Mode_IPU : GPIO_Mode_Out_PP);
    gt911_gpio_attr.GPIO_Pin  = GPIO_Pin_9;
    GPIO_Init(GPIOF, &gt911_gpio_attr);
}

void gt911_iic_start(void)
{
    // 切换IO模式为写模式
    gt911_iic_switch_mode(gt911_IIC_MODE_WRITE);

    // 先确保数据和时钟置高位
    gt911_SDA_SET();
    gt911_CLK_SET();
    gt911_iic_delay();

    // 数据拉低 -> 延时 0.6us -> 时钟拉低 开启IIC通信
    gt911_SDA_RESET();
    gt911_iic_delay();
    gt911_CLK_RESET();
}

void gt911_iic_end(void)
{

    // 切换IO模式为写模式
    gt911_iic_switch_mode(gt911_IIC_MODE_WRITE);

    // 时钟先拉高
    gt911_CLK_SET();
    gt911_iic_delay();

    // 数据拉高 -> 延时 0.6us -> 时钟拉高 结束IIC通信
    gt911_SDA_RESET();
    gt911_iic_delay();
    gt911_SDA_SET();
}

void gt911_iic_write(uint8_t wr)
{

    // 切换IO模式为写模式
    gt911_iic_switch_mode(gt911_IIC_MODE_WRITE);

    gt911_CLK_RESET();
    gt911_iic_delay();
    for (int8_t i = 7; i >= 0; i--) {
        if ((wr >> i) & 0x01) {
            gt911_SDA_SET();
        } else {
            gt911_SDA_RESET();
        }
        gt911_CLK_SET();
        gt911_iic_delay();
        gt911_CLK_RESET();
        gt911_iic_delay();
    }
}

uint8_t gt911_iic_read(void)
{
    // 切换IO模式为读模式
    gt911_iic_switch_mode(gt911_IIC_MODE_READ);
    gt911_iic_delay();

    uint8_t res = 0;
    for (int8_t i = 7; i >= 0; i--) {
        gt911_CLK_RESET();
        gt911_iic_delay();
        gt911_CLK_SET();
        res |= (gt911_SDA_READ() << i);
    }
    return res;
}

// flag 为1 代表无效应答
void gt911_iic_sendAck(uint8_t flag)
{
    gt911_CLK_RESET();
    gt911_iic_delay();
    // 切换IO模式为写模式
    gt911_iic_switch_mode(gt911_IIC_MODE_WRITE);
    gt911_iic_delay();
    if (flag) {
        gt911_SDA_SET();
    } else {
        gt911_SDA_RESET();
    }
    gt911_iic_delay();
    gt911_CLK_SET();
    gt911_iic_delay();
    gt911_CLK_RESET();
}

// flag 为1 代表无效应答
uint8_t gt911_iic_readAck(void)
{
    // 切换IO模式为读模式
    gt911_iic_switch_mode(gt911_IIC_MODE_READ);

    gt911_CLK_SET();
    gt911_iic_delay();

    for (uint8_t i = 0; i < 200; i++) {
        if (gt911_SDA_READ() == 0) {
            gt911_CLK_RESET();
            return 0;
        }
        gt911_iic_delay();
    }

    // BEEP_ALARM(1);
    return 1;
}
