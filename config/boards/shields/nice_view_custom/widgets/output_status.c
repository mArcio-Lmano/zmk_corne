#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>

#include "output_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool usb_present;
};

static void render(lv_obj_t *label, struct output_status_state s) {
    char buf[16];

    switch (s.selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        snprintk(buf, sizeof(buf), s.usb_present ? "USB" : "USB?");
        break;
    case ZMK_TRANSPORT_BLE: {
        const char *suffix;
        if (!s.active_profile_bonded) {
            suffix = "open";
        } else if (!s.active_profile_connected) {
            suffix = "...";
        } else {
            suffix = "ok";
        }
        snprintk(buf, sizeof(buf), "BLE %u %s", s.selected_endpoint.ble.profile_index + 1, suffix);
        break;
    }
    default:
        snprintk(buf, sizeof(buf), "?");
        break;
    }

    lv_label_set_text(label, buf);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        render(widget->label, state);
    }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_hid_ready(),
#else
        .usb_present = false,
#endif
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 64, 20);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);

    widget->label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->label, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(widget->label, "...");
    lv_obj_center(widget->label);

    sys_slist_append(&widgets, &widget->node);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
    return widget->obj;
}
