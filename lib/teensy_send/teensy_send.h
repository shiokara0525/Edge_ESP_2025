#pragma once


#include<Arduino.h>


#define SEND_MODE 1
#define SEND_STATE 2
#define SEND_MAXSPEED 3
#define SEND_COLOR 4
#define SEND_GETBALL_TH 5
#define SEND_STOPMOTOR 6
#define SEND_ACRESET 7
#define SEND_KICK 8
#define SEND_PS4 9
#define SEND_LINE_TH 10
#define SEND_PS4_R 11
#define SEND_D_NEO 13
#define SEND_TESTFLAG 14
#define SEND_A_NEO 16
#define SEND_SETPLAY_FLAG 17
#define SEND_PS4_CIRCLE 18
#define SEND_PS4_SQUARE 19

#define SEND_CHECKNUM 100


class send_log{
    public:
    send_log(int sort_,int data_);
    send_log(){};
    void set_log(int sort_,int data_);

    int sort = 0;
    int send_data = 3838;
};

class send_teensy{
    private:
    send_log log[20];
    int count = 0;
    void send_onedata(int sort_,int data_);
    void send_sixdata(int sort_,int data_[6]);

    public:
    void setup();
    void set_data(int sort_,int data_);
    void set_data(int sort_,int data_[6]);
    int send_data();
    void print();
    void reset();
};