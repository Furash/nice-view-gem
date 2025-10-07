#include <lvgl.h>

LV_IMG_DECLARE(frontier_01);

void draw_animation(lv_obj_t *canvas) {
    lv_obj_t *art = lv_img_create(canvas);
    lv_img_set_src(art, &frontier_01);
    lv_obj_align(art, LV_ALIGN_TOP_LEFT, 36, 16);
}