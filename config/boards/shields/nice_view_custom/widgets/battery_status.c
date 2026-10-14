#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/usb.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/events/peripheral_battery_state_changed.h>
#endif

#include "battery_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_status_state {
    uint8_t own_level;
    bool charging;
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    uint8_t peer_level;
    bool peer_present;
#endif
};

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static uint8_t last_peer_level = 0;
static bool last_peer_present = false;
#endif

static void render(struct zmk_widget_battery_status *widget, struct battery_status_state state) {
    char buf[12];
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    snprintk(buf, sizeof(buf), "L %s%u%%", state.charging ? "+" : "", state.own_level);
#else
    snprintk(buf, sizeof(buf), "R %s%u%%", state.charging ? "+" : "", state.own_level);
#endif
    lv_label_set_text(widget->own_label, buf);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (state.peer_present) {
        snprintk(buf, sizeof(buf), "R %u%%", state.peer_level);
    } else {
        snprintk(buf, sizeof(buf), "R  --");
    }
    lv_label_set_text(widget->peer_label, buf);
#endif
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_battery_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
        render(widget, state);
    }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    struct battery_status_state s = {
        .own_level = zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .charging = zmk_usb_is_powered(),
#else
        .charging = false,
#endif
    };

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    const struct zmk_peripheral_battery_state_changed *peer_ev =
        as_zmk_peripheral_battery_state_changed(eh);
    if (peer_ev != NULL) {
        last_peer_level = peer_ev->state_of_charge;
        last_peer_present = true;
    }
    s.peer_level = last_peer_level;
    s.peer_present = last_peer_present;
#endif

    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_peripheral_battery_state_changed);
#endif

int zmk_widget_battery_status_init(struct zmk_widget_battery_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 64, 32);
    lv_obj_set_style_pad_all(widget->obj, 2, LV_PART_MAIN);
    lv_obj_set_style_border_width(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, LV_PART_MAIN);

    widget->own_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->own_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->own_label, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(widget->own_label, "L  --");
    lv_obj_align(widget->own_label, LV_ALIGN_TOP_LEFT, 0, 0);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    widget->peer_label = lv_label_create(widget->obj);
    lv_obj_set_style_text_font(widget->peer_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(widget->peer_label, lv_color_black(), LV_PART_MAIN);
    lv_label_set_text(widget->peer_label, "R  --");
    lv_obj_align(widget->peer_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
#endif

    sys_slist_append(&widgets, &widget->node);

    widget_battery_status_init();
    return 0;
}

lv_obj_t *zmk_widget_battery_status_obj(struct zmk_widget_battery_status *widget) {
    return widget->obj;
}
