#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static uint8_t pixels[360*360*4];
static uint8_t frame[360*360*4];
static void flush(lv_display_t *d,const lv_area_t *a,uint8_t *p){
 for(int y=a->y1;y<=a->y2;y++) memcpy(frame+(y*360+a->x1)*4,p+(y-a->y1)*(a->x2-a->x1+1)*4,(a->x2-a->x1+1)*4);
 lv_display_flush_ready(d);
}
static void bmp(const char *path){
 uint32_t h[13]={40,360,(uint32_t)-360,0x00200001,0,360*360*4,0,0,0,0};
 FILE*f=fopen(path,"wb"); if(!f){perror(path);exit(2);} uint16_t magic=0x4d42;uint32_t size=54+360*360*4,zero=0,off=54;
 fwrite(&magic,2,1,f);fwrite(&size,4,1,f);fwrite(&zero,4,1,f);fwrite(&off,4,1,f);fwrite(h,40,1,f);fwrite(frame,sizeof(frame),1,f);fclose(f);
}
typedef struct {const char*name;lv_obj_t*(*create)(void);}Entry;
static Entry entries[]={{"add_task_p1", add_task_p1_create},
{"add_task_p1_b", add_task_p1_b_create},
{"add_task_p1_c", add_task_p1_c_create},
{"add_task_p1_d", add_task_p1_d_create},
{"add_task_p2", add_task_p2_create},
{"add_task_p3", add_task_p3_create},
{"add_task_p4", add_task_p4_create},
{"add_task_p5", add_task_p5_create},
{"add_task_p5_b", add_task_p5_b_create},
{"add_task_p5_c", add_task_p5_c_create},
{"add_task_p5_d", add_task_p5_d_create},
{"add_task_p5_e", add_task_p5_e_create},
{"desglose", desglose_create},
{"desglose_b", desglose_b_create},
{"esta_semana", esta_semana_create},
{"frame_1", frame_1_create},
{"frame_2", frame_2_create},
{"frame_3", frame_3_create},
{"home", home_create},
{"intro_1", intro_1_create},
{"intro_2", intro_2_create},
{"intro_3", intro_3_create},
{"intro_4", intro_4_create},
{"intro_5", intro_5_create},
{"log_in", log_in_create},
{"today", today_create},
{"today_b", today_b_create},
{"today_c", today_c_create},
{"welcome_back", welcome_back_create}};
int main(int argc,char**argv){
 if(argc!=2)return 2;lv_init();lv_display_t*d=lv_display_create(360,360);lv_display_set_color_format(d,LV_COLOR_FORMAT_XRGB8888);lv_display_set_buffers(d,pixels,NULL,sizeof(pixels),LV_DISPLAY_RENDER_MODE_FULL);lv_display_set_flush_cb(d,flush);ui_init("A:ui/");
 for(unsigned i=0;i<sizeof(entries)/sizeof(entries[0]);i++){lv_obj_t*old=lv_screen_active();lv_obj_t*s=entries[i].create();lv_screen_load(s);if(old!=s)lv_obj_delete(old);lv_tick_inc(20);lv_timer_handler();lv_refr_now(d);char path[1024];snprintf(path,sizeof(path),"%s/%s.bmp",argv[1],entries[i].name);bmp(path);puts(entries[i].name);}return 0;
}

