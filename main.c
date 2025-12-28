#include <stdio.h>
#include <string.h>

#include "chip8_cpu.h"
#include "chip8_platform.h"

int main(int argc, char *argv[]) 
{
    //通过命令行参数加载ROM

    //display init
    //audio init

    //借助SDL_GetTicks控制while循环每秒执行次数
    //AGAIN：CPU的定时器是60Hz，编程要注意
    while (is_running) 
    {
        //input_detect()
        //cycle()

        //如果要刷新 display_update()
        //如果要发声 audio_beep()
    } 

    //清理现场
    //display_destroy
    //audio_destroy
    //destroy
    return 0;
}
