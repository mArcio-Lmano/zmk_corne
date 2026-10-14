#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/wpm.h>

#include "wpm_graph.h"

#define WPM_MAX 120

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static uint8_t wpm_samples[WPM_GRAPH_W] = {0};

struct wpm_graph_state {
    uint8_t wpm;
};

static void draw_graph(struct zmk_widget_wpm_graph *widget) {
    lv_canvas_fill_bg(widget->canvas, lv_color_white(), LV_OPA_COVER);
    for (int x = 0; x < WPM_GRAPH_W; x++) {
        uint8_t v = wpm_samples[x];
        if (v > WPM_MAX) {
            v = WPM_MAX;
        }
        int h = (v * (WPM_GRAPH_H - 1)) / WPM_MAX;
        for (int y = WPM_GRAPH_H - 1 - h; y < WPM_GRAPH_H; y++) {
            lv_canvas_set_px_color(widget->canvas, x, y, lv_color_black());
        }
    }
}

static void wpm_graph_update_cb(struct wpm_graph_state state) {
    for (int i = 0; i < WPM_GRAPH_W - 1; i++) {
        wpm_samples[i] = wpm_samples[i + 1];
    }
    wpm_samples[WPM_GRAPH_W - 1] = state.wpm;

    char buf[12];
    snprintk(buf, sizeof(buf), "WPM %u", state.wpm);

    struct zmk_widget_wpm_graph *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        lv_label_set_text(widget->label, buf);
        draw_graph(widget);
    }
}

static struct wpm_graph_state wpm_graph_get_state(const zmk_event_t *eh) {
    const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    return (struct wpm_graph_state){
        .wpm = ev != NULL ? ev->state : (uint8_t)zmk_wpm_get_state(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_graph, struct wpm_graph_state, wpm_graph_update_cb,
                            wpm_graph_get_state)
ZMK_SUBSCRIPTION(widget_wpm_graph, zmk_wpm_state_changed);

int zmk_widget_wpm_graph_init(struct zmk_widget_wpm_graph *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 68, 54);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);

    widget->label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->label, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(widget->label, "WPM 0");
    lv_obj_align(widget->label, LV_ALIGN_TOP_MID, 0, 0);

    widget->canvas = lv_canvas_create(widget->obj);
    lv_canvas_set_buffer(widget->canvas, widget->canvas_buf, WPM_GRAPH_W, WPM_GRAPH_H,
                         LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(widget->canvas, 0, lv_color_white());
    lv_canvas_set_palette(widget->canvas, 1, lv_color_black());
    lv_canvas_fill_bg(widget->canvas, lv_color_white(), LV_OPA_COVER);
    lv_obj_align(widget->canvas, LV_ALIGN_BOTTOM_MID, 0, 0);

    sys_slist_append(&widgets, &widget->node);

    widget_wpm_graph_init();
    return 0;
}

lv_obj_t *zmk_widget_wpm_graph_obj(struct zmk_widget_wpm_graph *widget) {
    return widget->obj;
}
