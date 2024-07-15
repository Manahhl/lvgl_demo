/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-07-15 14:43:54
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-07-15 17:47:40
 * @FilePath: \lvgl_demo\src\main.c
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "porting/lv_porting.h"
#include "lvgl.h"
#include "user.h"

void lv_demo_test()
{
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello world!!!!!!");
    lv_obj_center(label);
}


#if LV_BUILD_EXAMPLES && LV_USE_BTN


//************************************************************************************************
    /*Create a button with a label and react on click event*/   
    //example_get_started_1 示例1 设置按键并添加点击事件
    static void btn_event_cb(lv_event_t * e)
    {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t * btn = lv_event_get_target(e);
        if(code == LV_EVENT_CLICKED) {
            static uint8_t cnt = 0;
            cnt++;

            /*Get the first child of the button which is the label and change its text*/
            lv_obj_t * label = lv_obj_get_child(btn, 0);
            lv_label_set_text_fmt(label, "Button: %d", cnt);
        }
    }

    /**
     * Create a button with a label and react on click event.
     */
    void lv_example_get_started_1(void)
    {
        lv_obj_set_style_bg_opa(lv_scr_act(),255,LV_PART_MAIN);
        /*Create a button*/

        lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
        //lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
        //lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
        lv_obj_set_align(btn, LV_ALIGN_CENTER);

        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

        lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
        lv_label_set_text(label, "Button");                     /*Set the labels text*/
        lv_obj_center(label);
    }



//************************************************************************************************
    /*Create a button with a label and react on click event*/
    //example_get_started_2 示例2 设置按键样式，颜色
    static lv_style_t style_btn;
    static lv_style_t style_btn_pressed;
    static lv_style_t style_btn_blue;

    static lv_color_t darken(const lv_color_filter_dsc_t * dsc, lv_color_t color, lv_opa_t opa)
    {
        LV_UNUSED(dsc);
        return lv_color_darken(color, opa);
    }

    static void style_init(void)
    {
        /*Create a simple button style*/
        lv_style_init(&style_btn);
        lv_style_set_radius(&style_btn, 10);
        lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
        lv_style_set_bg_color(&style_btn, lv_palette_lighten(LV_PALETTE_GREY, 3));
        lv_style_set_bg_grad_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
        lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);

        lv_style_set_border_color(&style_btn, lv_color_black());
        lv_style_set_border_opa(&style_btn, LV_OPA_20);
        lv_style_set_border_width(&style_btn, 2);

        lv_style_set_text_color(&style_btn, lv_color_black());

        /*Create a style for the pressed state.
         *Use a color filter to simply modify all colors in this state*/
        static lv_color_filter_dsc_t color_filter;
        lv_color_filter_dsc_init(&color_filter, darken);
        lv_style_init(&style_btn_pressed);
        lv_style_set_color_filter_dsc(&style_btn_pressed, &color_filter);
        lv_style_set_color_filter_opa(&style_btn_pressed, LV_OPA_20);

        /*Create a red style. Change only some colors.*/
        lv_style_init(&style_btn_blue);
        lv_style_set_bg_color(&style_btn_blue, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_bg_grad_color(&style_btn_blue, lv_palette_lighten(LV_PALETTE_BLUE, 3));
    }

    /**
     * Create styles from scratch for buttons.
     */
    void lv_example_get_started_2(void)
    {
        /*Initialize the style*/
        style_init();

        /*Create a button and use the new styles*/
        lv_obj_t * btn = lv_btn_create(lv_scr_act());
        /* Remove the styles coming from the theme
         * Note that size and position are also stored as style properties
         * so lv_obj_remove_style_all will remove the set size and position too */
        lv_obj_remove_style_all(btn);
        lv_obj_set_pos(btn, 10, 10);
        lv_obj_set_size(btn, 120, 50);
        lv_obj_add_style(btn, &style_btn, 0);
        lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

        /*Add a label to the button*/
        lv_obj_t * label = lv_label_create(btn);
        lv_label_set_text(label, "Button");
        lv_obj_center(label);

        /*Create another button and use the red style too*/
        lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
        lv_obj_remove_style_all(btn2);                      /*Remove the styles coming from the theme*/
        lv_obj_set_pos(btn2, 10, 80);
        lv_obj_set_size(btn2, 120, 50);
        lv_obj_add_style(btn2, &style_btn, 0);
        lv_obj_add_style(btn2, &style_btn_blue, 0);
        lv_obj_add_style(btn2, &style_btn_pressed, LV_STATE_PRESSED);
        lv_obj_set_style_radius(btn2, LV_RADIUS_CIRCLE, 0); /*Add a local style too*/

        lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_ALL, NULL);

        label = lv_label_create(btn2);
        lv_label_set_text(label, "Button 2");
        lv_obj_center(label);
    }

//************************************************************************************************
//example 3 示例3创建一个滑块并在标签上写入其值。

    static lv_obj_t * label;

    static void slider_event_cb(lv_event_t * e)
    {
        lv_obj_t * slider = lv_event_get_target(e);

        /*Refresh the text*/
        lv_label_set_text_fmt(label, "%"LV_PRId32, lv_slider_get_value(slider));
        lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
    }

    /**
     * Create a slider and write its value on a label.
     */
    void lv_example_get_started_3(void)
    {
        /*Create a slider in the center of the display*/
        lv_obj_t * slider = lv_slider_create(lv_scr_act());
        lv_obj_set_width(slider, 200);                          /*Set the width*/
        lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
        lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

        /*Create a label above the slider*/
        label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "0");
        lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
    }


//************************************************************************************************
//example 4 大小样式

    void lv_example_style_1(void)
    {
        static lv_style_t style;
        lv_style_init(&style);
        lv_style_set_radius(&style, 23);

        /*Make a gradient*/
        lv_style_set_width(&style, 150);
        lv_style_set_height(&style, LV_SIZE_CONTENT);

        lv_style_set_pad_ver(&style, 20);
        lv_style_set_pad_left(&style, 5);

        lv_style_set_x(&style, lv_pct(50));
        lv_style_set_y(&style, lv_pct(50));

        /*Create an object with the new style*/
        lv_obj_t * obj = lv_obj_create(lv_scr_act());
        lv_obj_add_style(obj, &style, 0);

        lv_obj_t * label = lv_label_create(obj);
        lv_label_set_text(label, "Hello");
    }

//************************************************************************************************
//example 5 设置背景样式

    void lv_example_style_2(void)
    {
        static lv_style_t style;
        lv_style_init(&style);
        lv_style_set_radius(&style, 5);

        /*Make a gradient*/
        lv_style_set_bg_opa(&style, LV_OPA_COVER);
        static lv_grad_dsc_t grad;
        grad.dir = LV_GRAD_DIR_VER;
        grad.stops_count = 2;
        grad.stops[0].color = lv_palette_lighten(LV_PALETTE_GREY, 1);
        grad.stops[1].color = lv_palette_main(LV_PALETTE_BLUE);

        /*Shift the gradient to the bottom*/
        grad.stops[0].frac  = 128;
        grad.stops[1].frac  = 192;

        lv_style_set_bg_grad(&style, &grad);

        /*Create an object with the new style*/
        lv_obj_t * obj = lv_obj_create(lv_scr_act());
        lv_obj_add_style(obj, &style, 0);
        lv_obj_center(obj);
    }












#endif

int main(int argc, char const *argv[])
{
    lv_porting_init();

    //lv_demo_test();
    //lv_example_get_started_1();
    //lv_example_get_started_2();
    //lv_example_get_started_3();

    //lv_example_style_1();
    lv_example_style_2();
    while (1) {
        lv_timer_handler();

        //5ms 刷新lvgl任务链
        lv_porting_delay();
    }
}
