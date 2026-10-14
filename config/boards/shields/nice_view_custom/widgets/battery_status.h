#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

struct zmk_widget_battery_status {
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *own_label;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    lv_obj_t *peer_label;
#endif
};

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget);
