#pragma once

#include<Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <timer.h>
#include<Arduino.h>
#include<myVector.h>
#include<teensy_send.h>

extern Adafruit_NeoPixel pixels;

#define NUMPIXELS 16
#define PIN 32


/*------------------------------------------------------------------------------------------------------------*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3C for 128x64, 0x3D for 128x32
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define EEPROM_SIZE 12
#define DEF_NUM 3838

#define EEPROM_LINE 0
#define EEPROM_RA 1
#define EEPROM_VAL 2
#define EEPROM_BUTTON 3
#define EEPROM_CHECK 4
#define EEPROM_BALL 10
#define EEPROM_MODE 11
#define EEPROM_OPTION 12

#define OPTION_NUM 5

#define BLUE 0
#define YELLOW 1

class oled_attack{
    public:
    Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    const int bluetooth = 11;
    timer timer_OLED;
    unsigned int address = 0x00;  //EEPROMのアドレス
    int toogle = 0;  //トグルスイッチの値を記録（トグルを引くときに使う）
    int goDir;  //loop関数ないしか使えないangle go_ang.degressの値をぶち込んでグローバルに使うために作った税
    int end_flag = 0;

    char CHECK_str [6][8] = {"CHECK_0","CHECK_1","CHECK_2","CHECK_3","CHECK_4","CHECK_5"};

    void setup(send_teensy &teensy);
    void OLED(send_teensy &teensy);
    void start();
    void end();
    int first_select();

    void display_main();
    void display_start();
    void select_Mode();

    void select_Option();

    void display_selectColor();
    void display_waitStart();
    void set_Line_Threshold();
    void display_Line();
    void set_getBall_Threshold();
    void display_Ball();
    void set_MotorVal();
    void set_Avaliable();
    void display_Cam();
    void display_getBall();
    void Kick_test();
    void select_testMode();
    void display_option();
    void kick_HH();

    void OLED_moving();

    void check_TactSwitch();
    int Button_selectCF = 0;  //コートの方向を決めるときに特殊なことをするので、セレクト変数を変えときますぜよ
    int Robot_Mode = 0; //デフォルトはアタッカー
    int test_flag = 0;
    int Sentor_A = 0;
    int Left_A = 0;
    int Right_A = 0;

    int Target_dir;

    int Left;
    int Sentor;
    int Right;
    timer Left_t;
    timer Sentor_t;
    timer Right_t;

    int A = 0;
    int B = 999;  //ステート初期化のための変数
    int aa = 0;  //タクトスイッチのルーレット状態防止用変数

    int flash_OLED = 0;  //ディスプレイの中で白黒点滅させたいときにつかう
    int OLED_select = 1;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によって選択画面の表示が変化する)
    int Button_select = 0;  //スイッチが押されたときにどこを選択しているかを示す変数(この数字によってexitかnextかが決まる)


    int check_select = 0;
    int check_flag = 0;
    int check_val[6];

    int testMode = 0;

    int option_flag = 0;
    uint8_t option_on[OPTION_NUM] = {0,0,0,0,0};

    const int Tact_Switch[3] = {18,35,19};
    const int Toggle_Switch = 34;  //スイッチのピン番号
    const int Bluetooth_pin = 36;

    int RA_size;
    int val_max = 200;
    int LINE_level = 50;
    int ball_getth = 100;
    int goal_color = 0;
    int Mnone_flag = 0;
    int ac_reset = 0;
    int kick_flag = 0;

    int ang_vel;

    int LINE_level_check;

    int setplay_flag = 0;

    int ball_catch_val = 0;

    int state;

    int line_on = 0;
    int line_on_all[27];
    int line_side_flag;
    int ball_on = 0;

    int cam_on = 0;
    int cam_back_on = 0;

    int cam_front_x1,cam_front_y1,cam_front_w,cam_front_h;
    int cam_back_x1,cam_back_y1,cam_back_w,cam_back_h;
    int cam_is_front = 1;

    int cam_front_ang,cam_back_ang;
    int cam_front_size,cam_back_size;


    Vector2D line_vec;
    Vector2D ball_vec;
    Vector2D cam_vec;
    Vector2D cam_back_vec;
    int ac_dir;

    int addresses[EEPROM_SIZE];
};
/*------------------------------------------------------------------------------------------------------------*/