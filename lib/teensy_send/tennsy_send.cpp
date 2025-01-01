#include<teensy_send.h>

send_log::send_log(int sort_,int data_){
    sort = sort_;
    send_data = data_;
}

void send_log::set_log(int sort_,int data_){
    sort = sort_;
    send_data = data_;
}


void send_teensy::setup(){
    Serial2.begin(115200);
}

void send_teensy::set_data(int sort_,int data_){
    log[count].set_log(sort_,data_);
    count++;
}

void send_teensy::set_data(int sort_,int data_[6]){
    for(int i = 0; i < 6; i++){
        log[count].set_log(sort_ + i,data_[i]);
        count++;
    }
}

void send_teensy::send_onedata(int sort_,int data_){
    uint8_t send_byte[5] = {38,0,0,0,37};
    send_byte[1] = sort_;
    send_byte[2] = byte( data_ >> 8 ); //ビットシフトで上位側の８Bitを取り出し、バイト型に型変換をする。
    send_byte[3] = byte( data_ & 0xFF ); //論理和で下位側の８Bitを取り出し、バイト型に型変換をする。
    Serial2.write(send_byte,5);
}

void send_teensy::send_sixdata(int sort_,int data_[6]){
    for(int i = 0; i < 6; i++){
        send_onedata(sort_ + i,data_[i]);
    }
}


int send_teensy::send_data(){
    for(int i = 0; i < count; i++){
        send_onedata(log[i].sort,log[i].send_data);
    }
    reset();
    return count;
}


void send_teensy::reset(){
    for(int i = 0; i < count; i++){
        log[i].set_log(0,0);
    }
    count = 0;
}


void send_teensy::print(){
    for(int i = 0; i < count; i++){
        Serial.print(i);
        Serial.print(" : ");
        Serial.print(" sort : ");
        Serial.print(log[i].sort);
        Serial.print(" data : ");
        Serial.print(log[i].send_data);
        Serial.println(" ");
    }
}