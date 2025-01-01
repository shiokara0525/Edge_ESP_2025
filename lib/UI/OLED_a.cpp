#include<OLED_a.h>

void oled_attack::setup(send_teensy &teensy){
  EEPROM.begin(512);

  address = 0;
  EEPROM.get(address,LINE_level);//EEPROMから読み出し
  teensy.set_data(SEND_LINE_TH,LINE_level);
  addresses[EEPROM_LINE] = address;

  address += sizeof(LINE_level);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,RA_size);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  addresses[EEPROM_RA] = address;

  address += sizeof(RA_size);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,val_max);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  teensy.set_data(SEND_MAXSPEED,val_max);
  addresses[EEPROM_VAL] = address;

  address += sizeof(val_max);  //アドレスを次の変数のアドレスにする
  EEPROM.get(address,Button_selectCF);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
  addresses[EEPROM_BUTTON] = address;

  address += sizeof(Button_selectCF);  //アドレスを次の変数のアドレスにする
  addresses[EEPROM_CHECK] = address;
  for(int i = 0; i < 6; i++){
    addresses[EEPROM_CHECK + i] = address;
    EEPROM.get(address,check_val[i]);//EEPROMから読み出し(前回取り出した変数からアドレスを取得し、次のアドレスをここで入力する)
    address += sizeof(check_val[i]);  //アドレスを次の変数のアドレスにする
  }
  teensy.set_data(SEND_CHECKNUM,check_val);

  EEPROM.get(address,ball_getth);
  teensy.set_data(SEND_GETBALL_TH,ball_getth);
  addresses[EEPROM_BALL] = address;

  address += sizeof(ball_getth);
  EEPROM.get(address,Robot_Mode);
  // teensy.set_data(SEND_MODE,Robot_Mode);
  addresses[EEPROM_MODE] = address;

  Serial.print(" address : ");
  Serial.print(address);

  for(int i = 0; i < OPTION_NUM; i++){
    address += sizeof(option_on[i]);
    EEPROM.get(address,option_on[i]);
    addresses[EEPROM_OPTION + i] = address;
  }

  // if(Robot_Mode == -1){
  //   for(int i = 0; i < EEPROM_SIZE; i++){
  //     EEPROM.put(addresses[i],0);
  //   }
  // }
  if(LINE_level == 0){
    LINE_level = 30;
  }
  EEPROM.commit();

  for(int i = 0; i < 3; i++){
    pinMode(Tact_Switch[i],INPUT);
  }
  pinMode(Toggle_Switch,INPUT);
  pinMode(Bluetooth_pin,INPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    // Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  pixels.begin();
}



int oled_attack::first_select(){
  check_TactSwitch();
  select_Mode();
  if(Sentor){
    EEPROM.put(addresses[EEPROM_MODE],Robot_Mode);
    EEPROM.commit();
    return 1;
  }
  else{
    return 0;
  }
}



void oled_attack::start(){
  Mnone_flag = 0;  //モーター動作ありのフラグ
  A = 0;
  B = 999;
  timer_OLED.reset();

  flash_OLED = 0;
  OLED_select = 1;
  Button_select = 0;

  toogle = digitalRead(Toggle_Switch);

  Sentor = 0;
  Left = 0;
  Right = 0;

  Left_A = 0;
  Right_A = 0;
  Sentor_A = 0;

  Left_t.reset();
  Right_t.reset();
  Sentor_t.reset();

  check_select = 0;
  check_flag = 0;
  end_flag = 0;

  display.display();
  display.clearDisplay();

}



void oled_attack::end(){
  display.fillScreen(BLACK);
  end_flag = 1;
}




void oled_attack::OLED(send_teensy &teensy) {
  if(timer_OLED.read_ms() > 500){
    if(flash_OLED == 0){
      flash_OLED = 1;
    }
    else{
      flash_OLED = 0;
    }
    timer_OLED.reset(); //タイマーのリセット(OLED用)
  }
  check_TactSwitch();


  if(A == 0){
    // Serial.print(" A=0 start ");
    if(A != B){
      OLED_select = 1;  //選択画面をデフォルトにする
      B = A;
    }
    display_main();
  }
  else if(A == 10){ //機体の中心となるコート上での0°の位置を決めるところ
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;

      teensy.set_data(SEND_MAXSPEED,val_max);
      teensy.set_data(SEND_COLOR,goal_color);
      teensy.set_data(SEND_GETBALL_TH,ball_getth);
      teensy.set_data(SEND_LINE_TH,LINE_level);
      teensy.set_data(SEND_CHECKNUM,check_val);
    }

    display_start();
    if(Sentor){
      if(Button_select == 0){
        ac_reset = 1;
        teensy.set_data(SEND_ACRESET,1);
        A = 12;  //コート方向判定
      }
      else if(Button_select == 1){
        A = 0;  //メニュー画面に戻る
      }
    }
  }
  else if(A == 11){
    if(A != B){
      B = A;
    }

    select_Mode();
    if(Sentor){
      EEPROM.put(addresses[EEPROM_MODE],Robot_Mode);
      EEPROM.commit();
      A = 12;
    }
  }
  else if(A == 12){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(setDir)をデフォルトにする
      B = A;
    }

    display_selectColor();
    if(Sentor){  //タクトスイッチが手から離れたら
      if(Button_selectCF == 0){
        goal_color = YELLOW;
        teensy.set_data(SEND_COLOR,YELLOW);
        A = 15;  //スタート画面に行く
      }
      else if(Button_selectCF == 2){
        goal_color = BLUE;
        teensy.set_data(SEND_COLOR,BLUE);
        A = 15;  //スタート画面に行く
      }
      else if(Button_selectCF == 1){
        A = 0;  //メニュー画面に戻る
      }
      EEPROM.put(addresses[EEPROM_BUTTON], Button_selectCF);  //EEPROMにボールの閾値を保存
      EEPROM.commit();
    }
  }
  else if(A == 15){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 1;  //ボタンの選択(setDir)をデフォルトにする
      B = A;
    }

    display_waitStart();
    teensy.set_data(SEND_STATE,15);
    if(Sentor){  //タクトスイッチが手から離れたら
      if(Button_select == 0){
        A = 0;  //メニュー画面に戻る
      }
      else if(Button_select == 1){
        ac_reset = 1;
        teensy.set_data(SEND_ACRESET,1);  //姿勢制御の値リセットするぜい
      }
      else if(Button_select == 2){
        A = 16;
      }
      else if(Button_select == 3){
        if(setplay_flag){
          setplay_flag = 0;
        }
        else{
          setplay_flag = 1;
        }
      }
    }
    if(digitalRead(Toggle_Switch) != toogle){
      toogle = digitalRead(Toggle_Switch);
      display.clearDisplay(); //初期化してI2Cバスを解放する
      end();
    }
    if(digitalRead(Bluetooth_pin) == HIGH && option_on[4]){
      toogle = digitalRead(Toggle_Switch);
      display.clearDisplay(); //初期化してI2Cバスを解放する
      end();      
    }
  }
  else if(A == 16){
    if(A != B){
      Button_select = 0;
      B = A;
    }

    display_option();

    if(Sentor){
      A = 15;
      for(int i = 0; i < EEPROM_OPTION; i++){
        EEPROM.put(addresses[EEPROM_OPTION + i],option_on[i]);
      }
      EEPROM.commit();
    }
    if(Right){
      option_flag++;
      if(option_flag > OPTION_NUM - 1){
        option_flag = 0;
      }
    }
    else if(Left){
      option_flag--;
      if(option_flag < 0){
        option_flag = OPTION_NUM - 1;
      }
    }

    if(digitalRead(Toggle_Switch) != toogle){
      toogle = digitalRead(Toggle_Switch);
      for(int i = 0; i < OPTION_NUM; i++){
        if(option_flag == i){
          if(option_on[i] == 0){
            option_on[i] = 1;
            teensy.set_data(SEND_STOPMOTOR,1);
          }
          else{
            option_on[i] = 0;
            teensy.set_data(SEND_STOPMOTOR,0);
          }
        }
      }
    }
  }
  else if(A == 20){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }

    set_Line_Threshold();
    if(Sentor){  //タクトスイッチが手から離れたら
      address = 0x00;  //EEPROMのアドレスを0x00にする
      // LINE_level = 700;  //初めにデータをセットしておかなければならない
      EEPROM.put(addresses[EEPROM_LINE], LINE_level);  //EEPROMにラインの閾値を保存
      EEPROM.commit();
      teensy.set_data(SEND_LINE_TH,LINE_level);

      A = 0;  //メニュー画面へ戻る
    }
  }
  else if(A == 30)  //Check Line
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }
    teensy.set_data(SEND_STATE,30);
    display_Line();
    if(Sentor){
      A = 0;
    }
  }
  else if(A == 40){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }

    set_getBall_Threshold();
    if(Sentor){  //タクトスイッチが手から離れたら
      EEPROM.put(addresses[EEPROM_BALL], ball_getth);  //EEPROMにラインの閾値を保存
      EEPROM.commit();
      teensy.set_data(SEND_GETBALL_TH,ball_getth);
      A = 0;  //メニュー画面へ戻る
    }
  }
  else if(A == 50)  //Check Ball
  {
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }
    teensy.set_data(SEND_STATE,50);
    display_Ball();
    if(Sentor){
      A = 0;
    }
  }
  else if(A == 60){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }
    set_MotorVal();
    if(Sentor){  //タクトスイッチが手から離れたら
      EEPROM.put(addresses[EEPROM_VAL], val_max);  //EEPROMにボールの閾値を保存
      EEPROM.commit();
      teensy.set_data(SEND_MAXSPEED,val_max);
      A = 0;  //メニュー画面へ戻る
    }
  }
  else if(A == 70){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }

    set_Avaliable();
    if(Sentor){  //タクトスイッチが手から離れたら
      if(check_flag == 0){
        if(check_select == -1 || check_select == 6){
          A = 0;  //メニュー画面へ戻る
        }
        else{
          check_flag = 1;
        }

        for(int i = 0; i < 6; i++){
          EEPROM.put(addresses[EEPROM_CHECK + i],check_val[i]);  //EEPROMにボールの閾値を保存
          Serial.printf(" check_val[%d] : %d ",i,check_val[i]);
        }
        EEPROM.commit();
        Serial.println();
        teensy.set_data(SEND_CHECKNUM,check_val);
      }
      else{
        check_flag = 0;
      }
    }
  }
  else if(A == 80){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }
    teensy.set_data(SEND_STATE,80);
    display_Cam();
    if(Sentor){
      A = 0;
    }
  }
  else if(A == 90){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    }
    display_getBall();
    teensy.set_data(SEND_STATE,90);

    if(Sentor){
      A = 0;
    }
  }
  else if(A == 100){
    if(A != B){  //ステートが変わったときのみ実行(初期化)
      Button_select = 0;  //ボタンの選択(next)をデフォルトにする
      B = A;
    };
    int kick_ = 0;

    Kick_test();
    if(digitalRead(Toggle_Switch) != toogle){
      toogle = digitalRead(Toggle_Switch);
      kick_ = 1;
      teensy.set_data(SEND_KICK,1);
    }

    if(Sentor){
      A = 0;
    }
    kick_flag = 1;
  }
  else if(A == 110){
    if(A != B){
      B = A;
      Button_select = 0;
    }
    select_Mode();
    if(Sentor){
      EEPROM.put(addresses[EEPROM_MODE],Robot_Mode);
      EEPROM.commit();
      A = 0;
    }
  }
  else if(A == 120){
    if(A != B){
      B = A;
      Button_select = 0;
    }
    select_testMode();
    if(digitalRead(Toggle_Switch) != toogle){
      if(test_flag != 4){
        toogle = digitalRead(Toggle_Switch);
        display.clearDisplay(); //初期化してI2Cバスを解放する
        test_flag = 1;
        end();
      }
    }
    if(Sentor){
      if(testMode == 4){
        A = 130;
      }
    }
  }
  else if(A == 130){
    if(A != B){
      B = A;
      Button_select = 0;
    }
    kick_HH();
    if(Sentor){
      A = 0;
    }
    if(digitalRead(Toggle_Switch) != toogle){
      toogle = digitalRead(Toggle_Switch);
      display.clearDisplay(); //初期化してI2Cバスを解放する
      test_flag = 1;
      end();
    }
  }
}



void oled_attack::check_TactSwitch(){
  int Tact_0 = digitalRead(Tact_Switch[0]);
  int Tact_1 = digitalRead(Tact_Switch[1]);
  int Tact_2 = digitalRead(Tact_Switch[2]);
  Left = 0;
  Right = 0;
  Sentor = 0;
  if(Left_A == 0){
    if(Tact_0 == LOW){
      Left_A = 1;
      Left_t.reset();
    }
  }
  else if(Left_A == 1){
    if(100 < Left_t.read_ms()){
      if(Tact_0 == HIGH){
        Left_A = 2;
      }
    }
    if(800 < Left_t.read_ms()){
      if(Tact_0 == LOW){
        Left_A = 5;
      }
    }
  }
  else if(Left_A == 2){
    Left = 1;
    Left_A = 0;
  }
  else if(Left_A == 5){
    if(50 < Left_t.read_ms()){
      if(Tact_0 == LOW){
        Left = 1;
        Left_t.reset();
      }
      else{
        Left_A = 0;
      }
    }
  }

  if(Right_A == 0){
    if(Tact_2 == LOW){
      Right_A = 1;
      Right_t.reset();
    }
  }
  else if(Right_A == 1){
    if(100 < Right_t.read_ms()){
      if(Tact_2 == HIGH){
        Right_A = 2;
      }
    }
    if(800 < Right_t.read_ms()){
      if(Tact_2 == LOW){
        Right_A = 5;
      }
    }
  }
  else if(Right_A == 2){
    Right = 1;
    Right_A = 0;
  }
  else if(Right_A == 5){
    if(50 < Right_t.read_ms()){
      if(Tact_2 == LOW){
        Right = 1;
        Right_t.reset();
      }
      else{
        Right_A = 0;
      }
    }
  }

  if(Sentor_A == 0){
    if(Tact_1 == LOW){
      Sentor_A = 1;
      Sentor_t.reset();
    }
  }
  else if(Sentor_A == 1){
    if(100 < Sentor_t.read_ms()){
      if(Tact_1 == HIGH){
        Sentor_A = 2;
      }
    }
    if(800 < Sentor_t.read_ms()){
      if(Tact_1 == LOW){
        Sentor_A = 5;
      }
    }
  }
  else if(Sentor_A == 2){
    Sentor = 1;
    Sentor_A = 0;
  }
  else if(Sentor_A == 5){
    if(50 < Sentor_t.read_ms()){
      if(Tact_1 == LOW){
        Sentor = 1;
        Sentor_t.reset();
      }
      else{
        Sentor_A = 0;
      }
    }
  }

  if(Right == 1 && Left == 1){
    Sentor = 1;
  }

  // Serial.print(" Left : ");
  // Serial.print(Left);
  // Serial.print(" Sentor : ");
  // Serial.print(Sentor);
  // Serial.print(" Right : ");
  // Serial.println(Right);
}



void oled_attack::display_main(){
  display.display();
  // Serial.print(" Cat0.5 ");
  display.clearDisplay();
  // Serial.print(" Cat1 ");

  //選択画面だということをしらせる言葉を表示
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hi! bro!");
  display.setCursor(0,10);
  display.println("What's up?");

  display.setCursor(60,0);
  display.println("Mode:");
  display.setCursor(90,0);
  if(Robot_Mode == 0){
    display.println("Attack");
  }
  else if(Robot_Mode == 1){
    display.println("Defence");
  }
  else if(Robot_Mode == 2){
    display.println("Test");
  }
  //文字と選択画面の境目の横線を表示
  display.drawLine(0, 21, 128, 21, WHITE);
  // Serial.print(" select start ");

  //選択画面の表示
  if(OLED_select == 1)  //STARTを選択しているとき
  {
    // Serial.print(" select=0 start ");
    //START値を調整
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,35);
    display.println("START");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(90,35);
    display.println("Set");
    display.setCursor(88,44);
    display.println("Motar");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 10;
    }
    // Serial.print(" select=1 end ");
  }
  else if(OLED_select == 2)  //Set Motarを選択しているとき
  {
    //Motar値を調整
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(12,27);
    display.println("Set");
    display.setCursor(0,44);
    display.println("Motar");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(90,35);
    display.println("Set");
    display.setCursor(88,45);
    display.println("Line");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 60;
    }
  }
  else if(OLED_select == 3)  //Set Lineを選択しているとき
  {
    //Line値を調整
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(12,27);
    display.println("Set");
    display.setCursor(6,44);
    display.println("Line");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Check");
    display.setCursor(88,45);
    display.println("Line");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 20;
    }
  }
  else if(OLED_select == 4)  //Check Lineを選択しているとき
  {
    //Check Lineの文字設定
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("Check");
    display.setCursor(6,44);
    display.println("Line");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(90,35);
    display.println("Set");
    display.setCursor(94,45);
    display.println("Ball");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 30;
    }
  }
  else if(OLED_select == 5)  //Set RA（回り込みの大きさ）を選択しているとき
  {
    //回り込みの大きさを調整
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(12,27);
    display.println("Set");
    display.setCursor(18,44);
    display.println("Ball");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Check");
    display.setCursor(88,45);
    display.println("Ball");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 40;
    }
  }
  else if(OLED_select == 6)  //Check Ballを選択しているとき
  {
    //Check Ballの文字設定
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("Check");
    display.setCursor(6,44);
    display.println("Ball");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Check");
    display.setCursor(88,45);
    display.println("B_get");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 50;
    }
  }
  else if(OLED_select == 7){
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("check");
    display.setCursor(10,44);
    display.println("B_get");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,40);
    display.println("setAny");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 90;
    }
  }
  else if(OLED_select == 8)  //勝手にデバックしてもろて
  {
    //Check anyの文字設定
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("set");
    display.setCursor(10,44);
    display.println("Any");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,40);
    display.println("checkCam");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 70;
    }
  }
  else if(OLED_select == 9){
    //Check Ballの文字設定
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("Check");
    display.setCursor(6,44);
    display.println("Cam");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Kick");
    display.setCursor(88,45);
    display.println("");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 80;
    }
  }
  else if(OLED_select == 10){
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,35);
    display.println("Kick");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("select");
    display.setCursor(88,45);
    display.println("Mode");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 100;
    }
  }
  else if(OLED_select == 11){
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,27);
    display.println("sel");
    display.setCursor(6,44);
    display.println("Mode");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Test");
    display.setCursor(88,45);
    display.println("");

    //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
    if(Sentor){
      A = 110;
    }
  }
  else if(OLED_select == 12){
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,35);
    display.println("Test");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Option");
    display.setCursor(88,45);
    display.println("");

    if(Sentor){
      A = 120;
    }
  }
  else if(OLED_select == 13){
    display.setTextSize(2);
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.setCursor(0,35);
    display.println("Option");

    //選択画面で矢印マークを中央に表示
    display.fillTriangle(70, 43, 64, 37, 64, 49, WHITE);  //▶の描画

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,35);
    display.println("Start");
    display.setCursor(88,45);
    display.println("");

    if(Sentor){
      A = 16;
    }
  }
  

  if(Right == 1){
    OLED_select++;  //次の画面へ
    if(OLED_select > 13){
      OLED_select = 1;
    }
  }
}



void oled_attack::select_Mode(){
  display.display();
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,30);
  display.print("mode:");

  display.setCursor(30,25);
  display.setTextSize(2);
  if(Robot_Mode == 0){
    display.println("Attack");
  }

  display.setTextColor(WHITE);
  if(Robot_Mode == 1){
    display.println("Defence");
  }
  

  if(Right == 1){
    if(Robot_Mode < 1){
      Robot_Mode++;  //next 
    }
    else{
      Robot_Mode = 0;
    }
  }

  if(Left == 1){
    if(Robot_Mode > 0){
      Robot_Mode--;  //next
    }
    else{
      Robot_Mode = 1;
    }
  }
}



void oled_attack::display_start(){
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30,0);
  display.println("Please");
  display.setCursor(0,20);
  display.println("CAL");
  display.setCursor(40,20);
  display.println("&");
  display.setCursor(56,20);
  display.println("SetDir");

  display.setTextSize(1);
  display.setCursor(38,40);
  display.println("of BNO055");

  display.setTextColor(WHITE);
  if(Button_select == 1)  //exitが選択されていたら
  {
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(0,56);
  display.println("Exit");

  display.setTextColor(WHITE);
  if(Button_select == 0)  //nextが選択されていたら（デフォルトはこれ）
  {
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(104,56);
  display.println("Next");

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  if(Right == 1){
    Button_select = 0;  //next
  }
  else if(Left== 1){
    Button_select = 1;  //exit
  }
}



void oled_attack::display_selectColor(){
  display.display();
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30,10);
  display.println("Mode:");
  display.setCursor(70,10);
  if(Robot_Mode == 0){
    display.println("Attack");
  }
  else if(Robot_Mode == 1){
    display.println("Defence");
  }

  display.setTextColor(WHITE);
  if(Button_selectCF == 0)  //exitが選択されていたら
  {
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(0,30);
  display.println("Yellow");

  display.setTextColor(WHITE);
  if(Button_selectCF == 1)  //exitが選択されていたら
  {
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(50,55);
  display.println("Exit");

  display.setTextColor(WHITE);
  if(Button_selectCF == 2)  //exitが選択されていたら
  {
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(90,30);
  display.println("Blue");

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)

  if(Right == 1){
    Button_selectCF++;  //next
    if(Button_selectCF < 3){
    }
    else{
      Button_selectCF = 0;
    }
  }

  if(Left== 1){
    if(Button_selectCF  > 0){
      Button_selectCF--;  //next
    }
  }
}



void oled_attack::display_waitStart(){
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);
  display.setCursor(38,35);
  display.println("Dir :");
  display.setTextSize(2);
  display.setCursor(80,30);
  display.println(ac_dir);

  setplay_flag = 0;

  //角度を再設定させるか、もとの選択画面に戻るかを決めるスイッチについての設定
  display.setTextSize(1);
  display.setTextColor(WHITE);
  if(Button_select == 0){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(0,55);
  display.println("Exit");

  display.setTextColor(WHITE);
  if(Button_select == 1){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(50,55);
  display.println("SetDir");

  display.setTextColor(WHITE);
  if(Button_select == 2){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(92,55);
  display.println("NoneM");

  display.setTextColor(WHITE);
  display.setTextSize(3);
  if(Button_select == 3){
    display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
    setplay_flag = 1;
  }

  display.setTextColor(WHITE);
  if(Button_select == 4){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setTextSize(3);
  display.setCursor(22,0);
  display.println("START");

  if(Button_select == 5){
    display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画
    setplay_flag = 2;
  }

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)

  if(Right == 1){
    if(Button_select < 5){
      Button_select++;  //next
    }
    else{
      Button_select = 0;
    }
  }
  else if(Left== 1){
    if(Button_select  > 0){
      Button_select--;  //next
    }
    else{
      Button_select = 4;
    }
  }
}



void oled_attack::set_MotorVal(){
  display.display();
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(14,0);
  display.println("Set Motar");

  display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
  display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

  //数字を中央揃えにするためのコード
  display.setTextSize(3);
  display.setTextColor(WHITE);
  if(val_max >= 1000){      //4桁の場合
    display.setCursor(28,22);
  }else if(val_max >= 100){ //3桁の場合
    display.setCursor(40,22);
  }else if(val_max >= 10){  //2桁の場合
    display.setCursor(48,22);
  }else{                       //1桁の場合
    display.setCursor(56,22);
  }
  display.println(val_max);  //ラインの閾値を表示

  display.setTextSize(1);
  if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
    display.setTextColor(BLACK, WHITE);
  }
  else{
    display.setTextColor(WHITE);
  }
  display.setCursor(44,56);
  display.println("Confirm");

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る
  if(Right == 1){
    if(val_max < 1023){
      val_max++;
    }
  }
  if(Left== 1){
    if(val_max > 0) {
      val_max--;
    }
  }
}




void oled_attack::set_Line_Threshold(){
  display.display();
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(16,0);
  display.println("Set Line");

  display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
  display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

  //数字を中央揃えにするためのコード
  display.setTextSize(3);
  display.setTextColor(WHITE);
  if(LINE_level >= 1000){      //4桁の場合
    display.setCursor(28,22);
  }else if(LINE_level >= 100){ //3桁の場合
    display.setCursor(40,22);
  }else if(LINE_level >= 10){  //2桁の場合
    display.setCursor(48,22);
  }else{                       //1桁の場合
    display.setCursor(56,22);
  }
  display.println(LINE_level);  //ラインの閾値を表示

  display.setTextSize(1);
  if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
    display.setTextColor(BLACK, WHITE);
  }
  else{
    display.setTextColor(WHITE);
  }
  display.setCursor(44,56);
  display.println("Confirm");

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る

  if(Right == 1){
    if(LINE_level < 1023){
      LINE_level++;
    }
  }
  if(Left== 1){
    if(LINE_level > 0){
      LINE_level--;
    }
  }
}



void oled_attack::display_Line(){
  display.display();
  display.clearDisplay();
  //ラインの位置状況マップを表示する
  display.drawCircle(32, 32, 20, WHITE);  //○ 20
  display.drawCircle(32, 32, 30, WHITE);  //○ 20

  //ラインの直線と円の交点の座標を求める
  float line_y = line_vec.return_y();  //ラインのx座標
  float line_x = line_vec.return_x();  //ラインのy座標
  float Ax,Ay,Bx,By;

  if(abs(line_y) < 0.0000001){
    Ax = 0;
    Ay = 2;
    Bx = 0;
    By = -2;
  }
  else{
    float s = line_x / line_y;
    float t = pow(line_vec.return_magnitude(), 2) / line_y;

    Ax = (-s*t + sqrt(pow(s*t , 2) - (s + 1) * (t*t - 4))) / (1 + s*s);
    Ay = s * Ax + t;
    Bx = (-s*t - sqrt(pow(s*t , 2) - (s + 1) * (t*t - 4))) / (1 + s*s);
    By = s * Bx + t;
  }


  // line_vec.print();
  // Serial.printf(" (x1,y1) = (%.2f, %.2f) , (x2,y2) = (%.2f, %.2f)",Ax,Ay,Bx,By);



  //ラインの線の座標をOLEDでの座標に変換(-1~1の値を0~60の値に変換)
  int OLED_line_ax = Ax * 15.0 + 30;
  int OLED_line_ay = Ay * 15.0 + 30;
  int OLED_line_bx = Bx * 15.0 + 30;
  int OLED_line_by = By * 15.0 + 30;
  // Serial.printf(" (x1,y1) = (%.2f, %.2f) , (x2,y2) = (%.2f, %.2f)\n",OLED_line_ax,OLED_line_ay,OLED_line_bx,OLED_line_by);

  if(line_vec.return_magnitude() != 0){  //ラインがロボットの下にある
    //ラインの線を表示
    display.drawLine((62 - OLED_line_ax), (62 - OLED_line_ay), (62 - OLED_line_bx), (62 - OLED_line_by), WHITE);
  }

  //"Line"と表示する
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(68,0);
  display.println("Line");

  //ここから下のコードのテキストをsize1にする
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //ラインの角度を表示する
  display.setCursor(68,25);
  display.println("Dir:");
  if(line_vec.return_magnitude() != 0){  //ラインがロボットの下にある
    display.setCursor(96,25);
    display.println(int(line_vec.return_azimuth()));
  }
  else{  //ラインがロボットの下にない
    display.fillRect(96, 25, 34, 10, WHITE);
  }

  //ラインの距離を表示する
  display.setCursor(68,39);
  display.println("far:");
  if(line_vec.return_magnitude() != 0){  //ラインがロボットの下にある
    display.setCursor(96,39);
    display.println((line_vec.return_magnitude()));
  }
  else{  //ラインがロボットの下にない
    display.fillRect(96, 39, 34, 10, WHITE);
  }

  display.setCursor(68,53);
  display.println("sub:");
  display.setCursor(96,53);
  display.println(line_side_flag);

  if(line_on){
    pixels.clear();
    for(int i = 0; i < 8; i++){
      for(int j = 0; j < 3; j++){
        int number = i * 3 + j;
        if(line_on_all[number]){
          int num_neo[2] = {i * 2 - 4,i * 2 - 3};
          for(int i = 0; i < 2; i++){
            if(num_neo[i] < 0){
              num_neo[i] += 16;
            }
          }
          pixels.setPixelColor(num_neo[0],pixels.Color(0,100,0));
          pixels.setPixelColor(num_neo[1],pixels.Color(0,100,0));
          // Serial.printf(" !!! %d",number);
          break;
        }
      }
    }
    pixels.show();
  }

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る
}



void oled_attack::set_getBall_Threshold(){
  display.display();
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(16,0);
  display.println("Set Ball");

  display.fillTriangle(110, 33, 104, 27, 104, 39, WHITE);  //▶の描画
  display.fillTriangle(18, 33, 24, 27, 24, 39, WHITE);  //◀の描画

  //数字を中央揃えにするためのコード
  display.setTextSize(3);
  display.setTextColor(WHITE);
  if(ball_getth >= 1000){      //4桁の場合
    display.setCursor(28,22);
  }else if(ball_getth >= 100){ //3桁の場合
    display.setCursor(40,22);
  }else if(ball_getth >= 10){  //2桁の場合
    display.setCursor(48,22);
  }else{                       //1桁の場合
    display.setCursor(56,22);
  }
  display.println(ball_getth);  //ラインの閾値を表示

  display.setTextSize(1);
  if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
    display.setTextColor(BLACK, WHITE);
  }
  else{
    display.setTextColor(WHITE);
  }
  display.setCursor(44,56);
  display.println("Confirm");

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
//タクトスイッチが押されたら、メニューに戻る

  if(Right == 1){
    if(ball_getth < 1023){
      ball_getth++;
    }
  }

  if(Left== 1){
    if(ball_getth > 0){
      ball_getth--;
    }
  }
}



void oled_attack::display_Ball(){
  display.display();
  display.clearDisplay();

  //ボールの座標をOLED用にする（無理やりint型にしてOLEDのドットに合わせる）
  int OLED_ball_x = map(ball_vec.return_x(), -150, 150, 0, 60);  //
  int OLED_ball_y = map(ball_vec.return_y(), -150, 150, 0, 60);  //

  //ボールの位置状況マップを表示する
  display.drawCircle(32, 32, 30, WHITE);  //○ 30
  display.drawCircle(32, 32, 20, WHITE);  //○ 20
  display.drawCircle(32, 32, 10, WHITE);  //○ 10
  display.drawLine(2, 32, 62, 32, WHITE); //-
  display.drawLine(32, 2, 32, 62, WHITE); //|

  //ボールの位置を表示する
  if(1)  //ボールがあれば
  {
    display.fillCircle((OLED_ball_x + 2), (62 - OLED_ball_y), 5, WHITE);
  }

  //"Ball"と表示する
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(68,0);
  display.println("Ball");

  //ここから下のコードのテキストをsize1にする
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //ボールの角度を表示する
  display.setCursor(68,24);
  display.println("Dir:");
  if(ball_vec.return_magnitude() != 0){  //ボールがあれば値を表示
    display.setCursor(96,24);
    display.println(int(ball_vec.return_azimuth()));
  }
  else{  //ボールがなければ白い四角形を表示
    display.fillRect(96, 24, 34, 10, WHITE);
  }

  //ボールの距離を表示する
  display.setCursor(68,38);
  display.println("far:");
  if(ball_vec.return_magnitude() != 0){  //ボールがあれば値を表示
    display.setCursor(96,38);
    display.println(int(ball_vec.return_magnitude()));
  }
  else{  //ボールがなければ白い四角形を表示
    display.fillRect(96, 38, 34, 10, WHITE);
  }

  int ball_pos = 0;
  pixels.clear();
  ball_pos = ((ball_vec.return_azimuth()) + 180) / 22.5 - 4;
  if(ball_pos < 0){
    ball_pos += 16;
  }
  Serial.print(" b_p : ");
  Serial.print(ball_pos);
  pixels.setPixelColor(ball_pos,pixels.Color(0,150,0));
  pixels.show();

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る
}




void oled_attack::set_Avaliable(){
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  //テキストサイズと色の設定
  display.setTextSize(1);
  display.setTextColor(WHITE);
  if(check_select == 0){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }
  display.setCursor(0,0);  //1列目
  display.println("ang_0");  //この中に変数名を入力
  display.setCursor(60,0);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,0);
  display.println(check_val[0]);    //この中に知りたい変数を入力

  if(check_select == 1){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }

  display.setCursor(0,10);  //2列目
  display.println("neo_th");  //この中に変数名を入力
  display.setCursor(60,10);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,10);
  display.println(check_val[1]);    //この中に知りたい変数を入力

  if(check_select == 2){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }

  display.setCursor(0,20); //3列目
  display.println("ang_30");  //この中に変数名を入力
  display.setCursor(60,20);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,20);
  display.println(check_val[2]);    //この中に知りたい変数を入力

  if(check_select == 3){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }

  display.setCursor(0,30); //4列目
  display.println("ang_45");  //この中に変数名を入力
  display.setCursor(60,30);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,30);
  display.println(check_val[3]);    //この中に知りたい変数を入力

  if(check_select == 4){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }

  display.setCursor(0,40); //5列目
  display.println("D");  //この中に変数名を入力
  display.setCursor(60,40);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,40);
  display.println(check_val[4]);    //この中に知りたい変数を入力

  if(check_select == 5){
    if(flash_OLED == 0){  //白黒反転　何秒かの周期で白黒が変化するようにタイマーを使っている（flash_OLEDについて調べたらわかる）
      display.setTextColor(BLACK, WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
  }

  display.setCursor(0,50); //6列目
  display.println("F");  //この中に変数名を入力
  display.setCursor(60,50);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,50);
  display.println(check_val[5]);    //この中に知りたい変数を入力

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る

  if(check_flag == 0){
    if(Left == 1){
      check_select++;
      if(check_select > 6){
        check_select = 0;
      }
    }
    else if(Right == 1){
      check_select--;
      if(check_select < -1){
        check_select = 5;
      }
    }
  }
  else{
    if(Right == 1){
      check_val[check_select]++;
    }
    else if(Left == 1){
      check_val[check_select]--;
    }
  }
}



void oled_attack::display_Cam(){
  display.display();
  display.clearDisplay();


  //ボールの位置状況マップを表示する
  // display.drawLine(2, 12, 62, 12, WHITE); //-
  // display.drawLine(2, 52, 62, 52, WHITE); //|
  // display.drawLine(2, 12, 2, 52, WHITE); //|
  // display.drawLine(62, 12, 62, 52, WHITE); //|
  display.drawRect(2,12,60,45,WHITE);

  if(cam_is_front == 1){
    display.drawRect(2 + (cam_front_x1 * 0.375),12 + (cam_front_y1 * 0.1875),cam_front_w * 0.375,cam_front_h * 0.1875,WHITE);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(68,0);
    display.println("Cam_front");
  }
  else{
    display.drawRect(2 + (cam_back_x1 * 0.375),12 + (cam_back_y1 * 0.1875),cam_back_w * 0.375,cam_back_h * 0.1875,WHITE);

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(68,0);
    display.println("Cam_back");
  }

  // Serial.print(" cam_front_ang : ");
  // Serial.println(cam_front_ang);


  display.setTextSize(1);
  if(cam_is_front == 1){
    display.setCursor(68,24);
    display.println("Dir:");
    if(cam_on){  //ボールがあれば値を表示
      display.setCursor(96,24);
      display.println(int(cam_front_ang));
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 24, 34, 10, WHITE);
    }

    //ボールの距離を表示する
    display.setCursor(68,38);
    display.println("Size:");
    if(cam_on){  //ボールがあれば値を表示
      display.setCursor(96,38);
      display.println(int(cam_front_size));
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 38, 34, 10, WHITE);
    }
  }
  else{
    display.setCursor(68,24);
    display.println("Dir:");
    if(1){  //ボールがあれば値を表示
      display.setCursor(96,24);
      display.println(int(cam_back_ang));
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 24, 34, 10, WHITE);
    }

    display.setCursor(68,38);
    display.println("Size:");
    if(1){  //ボールがあれば値を表示
      display.setCursor(96,38);
      display.println(int(cam_back_size));
    }
    else{  //ボールがなければ白い四角形を表示
      display.fillRect(96, 38, 34, 10, WHITE);
    }
  }

  //ここから下のコードのテキストをsize1にする
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pixels.clear();
  if(cam_on){
    int ball_pos = 0;
    ball_pos = (cam_front_ang + 180) / 22.5 - 4;
    if(ball_pos < 0){
      ball_pos += 16;
    }
    if(goal_color == BLUE){
      pixels.setPixelColor(ball_pos,pixels.Color(0,0,100));
    }
    else if(goal_color == YELLOW){
      pixels.setPixelColor(ball_pos,pixels.Color(100,100,0));
    }
  }
  if(cam_back_on){
    int ball_pos = 0;
    ball_pos = -(cam_back_ang / 22.5) + 12;
    if(ball_pos < 0){
      ball_pos += 16;
    }
    if(goal_color == BLUE){
      pixels.setPixelColor(ball_pos,pixels.Color(100,100,0));
    }
    else if(goal_color == YELLOW){
      pixels.setPixelColor(ball_pos,pixels.Color(0,0,100));
    }
  }

  if(cam_is_front == 0){
    if(Left == 1 || Right == 1){
      cam_is_front = 1;
    }
  }
  else{
    if(Left == 1 || Right == 1){
      cam_is_front = 0;
    }
  }

  pixels.show();
  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る
}



void oled_attack::display_getBall(){
  display.display();
  display.clearDisplay();

  //テキストサイズと色の設定
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);  //1列目
  display.println("get");  //この中に変数名を入力
  display.setCursor(60,0);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,0);
  display.println(ball_catch_val);    //この中に知りたい変数を入力


  display.setCursor(0,10);  //2列目
  display.println("Right");  //この中に変数名を入力
  display.setCursor(60,10);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,10);
  display.println();    //この中に知りたい変数を入力


  display.setCursor(0,20); //3列目
  display.println("sum");  //この中に変数名を入力
  display.setCursor(60,20);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,20);
  display.println();    //この中に知りたい変数を入力


  display.setCursor(0,30); //4列目
  display.println("A");  //この中に変数名を入力
  display.setCursor(60,30);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,30);
  display.println();    //この中に知りたい変数を入力


  display.setCursor(0,40); //5列目
  display.println("B");  //この中に変数名を入力
  display.setCursor(60,40);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,40);
  display.println();    //この中に知りたい変数を入力


  display.setCursor(0,50); //6列目
  display.println("C");  //この中に変数名を入力
  display.setCursor(60,50);
  display.setTextColor(WHITE);
  display.println(":");    //この中に知りたい変数を入力
  display.setCursor(66,50);
  display.println();    //この中に知りたい変数を入力

  //タクトスイッチが押されたら(手を離されるまで次のステートに行かせたくないため、変数aaを使っている)
  //タクトスイッチが押されたら、メニューに戻る
}



void oled_attack::Kick_test(){
  display.display();
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(22,0);
  display.println("Kick");
}



void oled_attack::kick_HH(){
  display.display();
  display.clearDisplay();

  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(22,0);
  display.println("!!Kick_HH!!");
}



void oled_attack::select_testMode(){
  display.display();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25,30);
  if(testMode == 0){
    display.println("Ball");
  }
  else if(testMode == 1){
    display.println("Motor");
  }
  else if(testMode == 2){
    display.println("Control");
  }
  else if(testMode == 3){
    display.println(" PID ");
  }
  else if(testMode == 4){
    display.println("KickHH");
  }
  else if(testMode == 5){
    display.println("PS");
  }
  else if(testMode == 6){
    display.println("oudann");
  }  
  else if(testMode == 7){
    display.println("naname");
  }
  else if(testMode == 8){
    display.println("block");
  }


  if(Right){
    testMode++;
    if(testMode > 8){
      testMode = 0;
    }
  }
  else if(Left){
    testMode--;
    if(testMode < 0){
      testMode = 8;
    }
  }
}


void oled_attack::display_option(){
  display.display();
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(25,30);
  if(option_flag == 0){
    if(option_on[0]){
      display.setTextColor(BLACK,WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.println("NoneM");
  }
  else if(option_flag == 1){
    if(option_on[1]){
      display.setTextColor(BLACK,WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.println("printany");
  }
  else if(option_flag == 2){
    if(option_on[2]){
      display.setTextColor(BLACK,WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.println("LINE");
  }
  else if(option_flag == 3){
    if(option_on[3]){
      display.setTextColor(BLACK,WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.println("NEOP");
  }
  else if(option_flag == 4){
    if(option_on[4]){
      display.setTextColor(BLACK,WHITE);
    }
    else{
      display.setTextColor(WHITE);
    }
    display.println("BT");
  }
}


void oled_attack::OLED_moving(){
  //OLEDの初期化
  display.display();
  display.clearDisplay();

  //テキストサイズと色の設定
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);  //1列目
  display.println("dir_n");  //現在向いてる角度
  display.setCursor(30,0);
  display.println(":");
  display.setCursor(36,0);
  display.println();    //現在向いてる角度を表示

  display.setCursor(0,10);  //2列目
  display.println("dir_t");  //この中に変数名を入力
  display.setCursor(30,10);
  display.println(":");
  display.setCursor(36,10);
  display.println();    //この中に知りたい変数を入力a
  display.setCursor(0,20); //3列目 
  display.println("dir");  //この中に変数名を入力
  display.setCursor(30,20);
  display.println(":");
  display.setCursor(36,20);
  display.println();    //この中に知りたい変数を入力

  display.setCursor(0,30); //4列目
  display.println("A");  //この中に変数名を入力
  display.setCursor(30,30);
  display.println(":");
  display.setCursor(36,30);
  display.println();    //この中に知りたい変数を入力

  display.setCursor(0,40); //5列目
  display.println("M_F");  //この中に変数名を入力
  display.setCursor(30,40);
  display.println(":");
  display.setCursor(36,40);
  display.println();    //この中に知りたい変数を入力

  display.setCursor(0,50); //6列目
  display.println("B_c");  //この中に変数名を入力
  display.setCursor(30,50);
  display.println(":");
  display.setCursor(36,50);
  display.println();    //この中に知りたい変数を入力
}