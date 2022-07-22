#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

#define NEED_DATA 1
#define NO_NEED 0

//数据：1、CPU Temperature Avg 2、CPU Load Total  3、RAM Load 4、RAM Used 5、RAM Free  6、GPU Temp 7、GPU Load
#define DATA_LEN 7

void serial_get_data_timerCB(lv_timer_t *_timer);
String fenge(String str,String fen,int index);
void content_updateCB(lv_timer_t *_timer);
void bar_style_init(void);

//接收十进制整数
int pc_data[DATA_LEN];

//static控件
static lv_obj_t *label_cpu;
static lv_obj_t *label_cpu_load;
static lv_obj_t *label_ram;

static lv_obj_t *bar_cpu;
static lv_obj_t *bar_cpu_load;
static lv_obj_t *bar_ram_load;


TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

/*屏幕的宽高在这里修改*/
static const uint32_t screenWidth  = 160;
static const uint32_t screenHeight = 128;

//LVGL V7 statement:
// static lv_disp_buf_t draw_buf;
//LVGL V8 statement:
static lv_disp_draw_buf_t draw_buf;
//static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

//开启日志后调用的函数，启用该功能需要修改lvgl_conf.h的对应功能
#if LV_USE_LOG != 0
/* Serial debugging */
void my_print( lv_log_level_t level, const char * file, uint32_t line, const char * fn_name, const char * dsc )
{
   Serial.printf( "%s(%s)@%d->%s\r\n", file, fn_name, line, dsc );
   Serial.flush();
}
#endif

/* 刷新屏幕 */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.startWrite();
   tft.setAddrWindow( area->x1, area->y1, w, h );
   tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
   tft.endWrite();

   lv_disp_flush_ready( disp );
}




void setup()
{
   Serial.begin( 115200 ); /* prepare for possible serial debug */
   Serial.println( "Wryyyyyyyyyyyyyy" );


   lv_init();

#if LV_USE_LOG != 0
   lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

   tft.begin();          /* TFT init */
   tft.setRotation( 3 ); /* 旋转屏幕，n * 90度 ，3表示270度*/
  lv_disp_draw_buf_init(&draw_buf,buf,NULL,screenWidth*10);

   /*初始化屏幕*/
   //lvgl7:
   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init( &disp_drv );
   
   /*Change the following line to your display resolution*/
   disp_drv.hor_res = screenWidth;
   disp_drv.ver_res = screenHeight;
   disp_drv.flush_cb = my_disp_flush;

   disp_drv.draw_buf=&draw_buf;
   lv_disp_drv_register( &disp_drv );

   /*初识化输入设备*/
   static lv_indev_drv_t indev_drv;
   lv_indev_drv_init( &indev_drv );
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   lv_indev_drv_register( &indev_drv );

#if 1

    //主屏幕Flex布局
    lv_obj_set_flex_flow(lv_scr_act(),LV_FLEX_FLOW_COLUMN);
    //参数：父级对象，Y轴分布方式，X轴分布方式，如何分配轨道。
    lv_obj_set_flex_align(lv_scr_act(),LV_FLEX_ALIGN_SPACE_AROUND,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);

    //内容更新任务
    //user_data传结构体指针就会出错，所以把所有要展示的内容放在同一个控件中即可
    //创建控件
    label_cpu=lv_label_create(lv_scr_act());
    bar_cpu=lv_bar_create(lv_scr_act());
    label_cpu_load=lv_label_create(lv_scr_act());
    bar_cpu_load=lv_bar_create(lv_scr_act());
    label_ram=lv_label_create(lv_scr_act());
    bar_ram_load=lv_bar_create(lv_scr_act());
    

        //初始化timer任务
    //参数：回调函数，周期，user_data
    lv_timer_t *timer_setup=lv_timer_create(serial_get_data_timerCB,1000,NULL);
    //设置重复次数 -1为无穷
    lv_timer_set_repeat_count(timer_setup,-1);
    //数据更新回调
    lv_timer_t *timer_update=lv_timer_create(content_updateCB,500,NULL);
    lv_timer_set_repeat_count(timer_update,-1);


    //初始label的style
    // static lv_style_t label_style_default;
    // lv_style_init(&label_style_default);
    // lv_style_set_border_width(&label_style_default,1);
    // lv_style_set_border_color(&label_style_default,lv_color_hex(0xFFA500));
    // // lv_style_set_text_font(&label_style_default,&lv_font_montserrat_10);    

    // lv_obj_add_style(label_cpu,&label_style_default,LV_STATE_DEFAULT);
    // lv_obj_add_style(label_ram,&label_style_default,LV_STATE_DEFAULT);


    /*A base style*/
    static lv_style_t style_base;
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_palette_main(LV_PALETTE_LIGHT_BLUE));
    lv_style_set_border_color(&style_base, lv_palette_darken(LV_PALETTE_LIGHT_BLUE, 3));
    lv_style_set_border_width(&style_base, 2);
    //圆角矩形
    lv_style_set_radius(&style_base, 4);
    //阴影参数
    lv_style_set_shadow_width(&style_base, 5);
    lv_style_set_shadow_ofs_y(&style_base, 3);
    lv_style_set_shadow_opa(&style_base, LV_OPA_50);

    // lv_style_set_bg_color(&style_base,lv_color_hex())
    // lv_style_set_bg_color(&style_base,lv_palette_darken(LV_PALETTE_LIGHT_BLUE, 3));
    lv_style_set_text_color(&style_base, lv_palette_darken(LV_PALETTE_LIGHT_BLUE, 3));
    lv_style_set_width(&style_base, LV_SIZE_CONTENT);
    lv_style_set_height(&style_base, LV_SIZE_CONTENT);
    /*Set only the properties that should be different*/
    //在Base有阴影和border的基础上叠加黄色
    static lv_style_t style_warning;
    lv_style_init(&style_warning);
    lv_style_set_bg_color(&style_warning, lv_palette_main(LV_PALETTE_YELLOW));
    lv_style_set_border_color(&style_warning, lv_palette_darken(LV_PALETTE_YELLOW, 3));
    lv_style_set_text_color(&style_warning, lv_palette_darken(LV_PALETTE_YELLOW, 4));
    //style赋值
    lv_obj_add_style(label_cpu,&style_base,LV_STATE_DEFAULT);
    lv_obj_add_style(label_cpu_load,&style_base,LV_STATE_DEFAULT);

    lv_obj_add_style(label_ram,&style_base,LV_STATE_DEFAULT);
    lv_obj_add_style(label_ram,&style_warning,LV_STATE_DEFAULT);

    //bar style
    //默认的范围是0~100所以不需要改
    lv_obj_set_size(bar_cpu, 100, 10);
    lv_obj_set_size(bar_cpu_load, 100, 10);
    lv_obj_set_size(bar_ram_load, 100, 10);
    bar_style_init();
    // //label布局 使用Flex布局之后无效
    // lv_obj_align(label_cpu,LV_ALIGN_TOP_MID,0,0);
    // lv_obj_align(label_ram,LV_ALIGN_CENTER,0,0);
    


    
    

#else
   // 取消注释会启用对应的案例
   //lv_demo_music();
   //lv_demo_widgets();            // OK
   // lv_demo_benchmark();          // OK
   // lv_demo_keypad_encoder();     // works, but I haven't an encoder
    //lv_demo_music();              // NOK
   // lv_demo_printer();
   // lv_demo_stress();             // seems to be OK
#endif
   Serial.println( "Setup done" );
}

void serial_get_data_timerCB(lv_timer_t *_timer){

    Serial.print(NEED_DATA);
    if(Serial.available()>0){
        delay(30);
        String String_received=Serial.readString();
        //等待清空接收缓冲区
        while(Serial.read() >= 0);
        String temp;
        //循环次数看上位机传多少个数据而定
        for(uint8_t j=0;j<=6;j++){
            //差点忘记需要Sting to C String
            temp=fenge(String_received,",",j);
            //如果发送的数据数量小于6 最后会显示-1
            // lv_label_set_text(label_data, temp.c_str());
            //转为int
            pc_data[j]=temp.toInt();
        }
    }
    
}


void content_updateCB(lv_timer_t *_timer){

    // lv_obj_t *label_cpu=(lv_obj_t *)_timer->user_data;


    String temp="";
    temp=temp+"CPU Temp : "+pc_data[0]/100.0+"°C";
    lv_label_set_text(label_cpu,temp.c_str());

    temp="";
    temp=temp+"CPU Load : "+pc_data[1]/100.0+"%";
    lv_label_set_text(label_cpu_load,temp.c_str());

    temp="";
    temp=temp+"RAM Load : "+pc_data[2]/100.0+"%";
    lv_label_set_text(label_ram,temp.c_str());

    lv_bar_set_value(bar_cpu,pc_data[0]/100,LV_ANIM_ON);
    lv_bar_set_value(bar_cpu_load,pc_data[1]/100,LV_ANIM_ON);
    lv_bar_set_value(bar_ram_load,pc_data[2]/100,LV_ANIM_ON);
}

String fenge(String str,String fen,int index)
{
 int weizhi;
 String temps[str.length()];
 int i=0;
 do
 {
    weizhi = str.indexOf(fen);
    if(weizhi != -1)
    {
      temps[i] =  str.substring(0,weizhi);
      str = str.substring(weizhi+fen.length(),str.length());
      i++;
      }
      else {
        if(str.length()>0)
        temps[i] = str;
      }
 }
  while(weizhi>=0);

  if(index>i)return "-1";
  return temps[index];
}

void bar_style_init(void){
    static lv_style_t style_indic;

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    //官方例程为垂直 需要自己设置渐变方向为水平
    //https://blog.csdn.net/believe666/article/details/121849062
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);

    lv_obj_add_style(bar_cpu,&style_indic,LV_PART_INDICATOR);
    lv_obj_add_style(bar_cpu_load,&style_indic,LV_PART_INDICATOR);
    lv_obj_add_style(bar_ram_load,&style_indic,LV_PART_INDICATOR);
}

void loop()
{
//     lv_task_handler();
// //   lv_timer_handler(); /* 在循环中让lvgl处理一些相应的事件 */
//    delay( 5 );
    long last_time=millis();
    // lv_task_handler();
    lv_timer_handler();
    delay(10);
    lv_tick_inc(int(millis()-last_time));
}