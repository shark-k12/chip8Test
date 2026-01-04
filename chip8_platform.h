#ifndef PLATFORM_H_ 
#define PLATFORM_H_ 


#define FG_COLOR 0xFFFFFFFF  // 白色（不透明）
#define BG_COLOR 0xFF000000  // 黑色（不透明）

#define AUDIO_SAMPLING_RATE 44100
#define AUDIO_AMPLITUDE 10000

#define SCR_W           64
#define SCR_H           32
#define SCR_SZ          2048
#define SCALE 8

#define KEY_SZ          16

void display_init(void); //初始化
void display_update(void); //刷新显示
void display_destroy(void); //回收
void audio_init(void); //声音初始化
void audio_beep(void); //发出蜂鸣声
void audio_stop(void); //停止蜂鸣声
void audio_destroy(void); //回收 
void input_detect(void); //检测输入

extern char is_running;

#endif