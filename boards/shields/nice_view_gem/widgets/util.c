#include <zephyr/kernel.h>
#include "util.h"
#include <ctype.h>
#include <string.h>

void to_uppercase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);
    }
}

void rotate_canvas(lv_obj_t *canvas, lv_color_t cbuf[]) {
    static lv_color_t cbuf_tmp[BUFFER_SIZE * BUFFER_SIZE + 8];
    memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));

    lv_color_format_t cf = lv_display_get_color_format(lv_obj_get_display(canvas));
    uint32_t stride = lv_draw_buf_width_to_stride(BUFFER_SIZE, cf);

    lv_img_dsc_t img;
    img.data = (void *)cbuf_tmp;
    img.header.cf = cf;
    img.header.w = BUFFER_SIZE;
    img.header.h = BUFFER_SIZE;
    img.header.stride = stride;
    img.data_size = stride * BUFFER_SIZE + 8;

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    img_dsc.src = &img;
    img_dsc.rotation = 900; /* 90 degrees in 0.1 degree units */
    img_dsc.pivot.x = BUFFER_SIZE / 2;
    img_dsc.pivot.y = BUFFER_SIZE / 2;

    lv_area_t coords = {0, 0, BUFFER_SIZE - 1, BUFFER_SIZE - 1};
    lv_draw_image(&layer, &img_dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}

void fill_background(lv_obj_t *canvas) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);

    lv_area_t coords = {0, 0, BUFFER_SIZE - 1, BUFFER_SIZE - 1};
    lv_draw_rect(&layer, &rect_black_dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}

/* Helper function to draw text on canvas using LVGL 9 API */
void canvas_draw_text(lv_obj_t *canvas, int32_t x, int32_t y, int32_t max_w, const void *dsc,
                      const char *txt) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_draw_label_dsc_t label_dsc = *(const lv_draw_label_dsc_t *)dsc;
    label_dsc.text = txt;

    lv_area_t coords;
    coords.x1 = x;
    coords.y1 = y;
    coords.x2 = x + max_w - 1;
    coords.y2 = y + 100; /* Large enough height */

    lv_draw_label(&layer, &label_dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}

/* Helper function to draw image on canvas using LVGL 9 API */
void canvas_draw_img(lv_obj_t *canvas, int32_t x, int32_t y, const void *src, const void *dsc) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    const lv_img_dsc_t *img = (const lv_img_dsc_t *)src;
    lv_draw_image_dsc_t img_dsc;
    if (dsc) {
        img_dsc = *(const lv_draw_image_dsc_t *)dsc;
    } else {
        lv_draw_image_dsc_init(&img_dsc);
    }
    img_dsc.src = img;

    lv_area_t coords;
    coords.x1 = x;
    coords.y1 = y;
    coords.x2 = x + img->header.w - 1;
    coords.y2 = y + img->header.h - 1;

    lv_draw_image(&layer, &img_dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}

/* Helper function to draw rectangle on canvas using LVGL 9 API */
void canvas_draw_rect(lv_obj_t *canvas, int32_t x, int32_t y, int32_t w, int32_t h,
                      const void *dsc) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    lv_area_t coords;
    coords.x1 = x;
    coords.y1 = y;
    coords.x2 = x + w - 1;
    coords.y2 = y + h - 1;

    lv_draw_rect(&layer, (const lv_draw_rect_dsc_t *)dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}

/* Helper function to draw line on canvas using LVGL 9 API */
void canvas_draw_line(lv_obj_t *canvas, const lv_point_precise_t points[], uint32_t point_cnt,
                      const void *dsc) {
    if (point_cnt < 2) {
        return; /* Need at least 2 points to draw a line */
    }

    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    /* LVGL 9 lv_draw_line only draws a single line between two points,
     * so we need to loop through consecutive points to draw a polyline.
     * The points are stored in the descriptor's p1 and p2 fields.
     * Note: lv_draw_line_dsc_t uses lv_point_t (integer), so we convert
     * from lv_point_precise_t (float) by rounding. */
    lv_draw_line_dsc_t line_dsc = *(const lv_draw_line_dsc_t *)dsc;
    for (uint32_t i = 0; i < point_cnt - 1; i++) {
        line_dsc.p1.x = (int32_t)points[i].x;
        line_dsc.p1.y = (int32_t)points[i].y;
        line_dsc.p2.x = (int32_t)points[i + 1].x;
        line_dsc.p2.y = (int32_t)points[i + 1].y;
        lv_draw_line(&layer, &line_dsc);
    }

    lv_canvas_finish_layer(canvas, &layer);
}

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align) {
    lv_draw_label_dsc_init(label_dsc);
    label_dsc->color = color;
    label_dsc->font = font;
    label_dsc->align = align;
}

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color) {
    lv_draw_rect_dsc_init(rect_dsc);
    rect_dsc->bg_color = bg_color;
}

void init_line_dsc(lv_draw_line_dsc_t *line_dsc, lv_color_t color, uint8_t width) {
    lv_draw_line_dsc_init(line_dsc);
    line_dsc->color = color;
    line_dsc->width = width;
}