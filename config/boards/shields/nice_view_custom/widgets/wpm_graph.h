#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

#define WPM_GRAPH_W 60
#define WPM_GRAPH_H 40

struct zmk_widget_wpm_graph {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *label;
    lv_obj_t *canvas;
    uint8_t canvas_buf[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(WPM_GRAPH_W, WPM_GRAPH_H)];
};

int zmk_widget_wpm_graph_init(struct zmk_widget_wpm_graph *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_wpm_graph_obj(struct zmk_widget_wpm_graph *widget);
