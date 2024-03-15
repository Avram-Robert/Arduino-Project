// include the library code:
#include <LiquidCrystal.h>

#define DECODE_NEC

#include <Arduino.h>

/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

// Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte Ship[] = {
  B11000,
  B10100,
  B11110,
  B01011,
  B01011,
  B11110,
  B10100,
  B11000
};

byte Obstacle[] = {
  B11110,
  B11011,
  B11111,
  B01010,
  B10110,
  B11011,
  B01101,
  B11110
};
byte Block[] = {
  B01110,
  B10101,
  B10011,
  B10011,
  B10001,
  B10001,
  B10001,
  B01110
};

byte Bullet[] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

const int led1 = 13;
const int led2 = 12;
const int led3 = 11;

class Player
{
  int lives;
  double speed;
  public:
  Player()
  {
    lives = 3;
    speed = 1;
  }
  int getLives()
  {
    return lives;
  }
  void setLives(const int lives)
  {
    this->lives = lives;
  }
  double getSpeed()
  {
    return speed;
  }
  void loseLife()
  {
    lives--;
  }
  void setSpeed(const int speed)
  {
    this->speed = speed;
  }
  void showLed()
  {
    if(lives == 3)
    {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
    }
    else if(lives == 2)
    {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, LOW);
    }
    else if(lives == 1)
    {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
    }
  }
};

Player player;

class Game
{
  int stage;
  int difficulty;
  int map[16][2];
  public:
  int getStage()
  {
    return this->stage;
  }
  void setStage(const int stage)
  {
    this->stage = stage;
  }
  void setDifficulty(const int difficulty)
  {
    this->difficulty = difficulty;
  }
  Game()
  {
    stage = 0;
    difficulty = 1;
    for(int i = 0; i < 16; i++)
    {
      map[0][i] = 0;
      map[1][i] = 0;
    }
    map[0][0] = 1;
  }
  void Map(const int i, const int j, const int type)
  {
    map[i][j] = type;
  }
  void showMap()
  {
    for(int i = 0; i < 16; i++)
    {
      for(int j = 0; j < 2; j++)
      {
          if(map[i][j] == 1)
          {
            lcd.setCursor(i, j);
            lcd.write(1);
          }
          else if(map[i][j] == 2)
          {
            lcd.setCursor(i, j);
            lcd.write(2);
          }
          else if(map[i][j] == 3)
          {
            lcd.setCursor(i, j);
            lcd.write(3);
          }
          else if(map[i][j] == 4)
          {
            lcd.setCursor(i, j);
            lcd.write(4);
          }
      }
    }
  }
  int getDifficulty()
  {
    return difficulty;
  }
  void checkButtons()
  {
    int sensor = analogRead(A0);
    if(sensor > 60 && sensor < 200)
    {
        if(map[0][0] == 3 || map[0][0] == 4)
        {
          player.loseLife();
        }
        map[0][0] = 1; 
        map[0][1] = 0;
    }
    else if(sensor > 200 && sensor < 400)
    {
      if(map[0][1] == 3 || map[0][1] == 4)
        {
          player.loseLife();
        }
        map[0][1] = 1; 
        map[0][0] = 0; 
    }
  }
  int getMap(const int i, const int j)
  {
    return map[i][j];
  }
  void createBullet()
  {
    if(map[0][0] == 1 && map[1][0] == 3)
    {
      map[1][0] = 0;
    }
    else if(map[0][1] == 1 && map[1][1] == 3)
    {
      map[1][1] = 0;
    }
    else if((map[0][0] == 1 && map[1][0] == 2) || (map[0][1] == 1 && map[1][1] == 2))
    {

    }
    else if(map[0][0] == 1)
    {
      map[1][0] = 4;
    }
    else if(map[0][1] == 1)
    {
      map[1][1] = 4;
    }
  }
};

Game start;

const int enable = 9;
int interval = 0;
unsigned long previousMillis = 0;
unsigned long previousMillisBullet = 0;

unsigned long score;

void checkLives()
{
  if(player.getLives() == 0)
  {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    for(int i = 0; i < 16; i++)
    {
      lcd.setCursor(i, 0);
      lcd.write(255);
      delay(75);
    }
    for(int i = 15; i > -1; i--)
    {
      lcd.setCursor(i, 1);
      lcd.write(255);
      delay(75);
    }
    start.setStage(4);
    score = (millis() - score)/100;
  }
}

void placeBlock()
{
  start.Map(15, random(0, 2), random(2, 4));
}

unsigned long count = 0;
unsigned long lastcount = 0;
int difference = 2;
int intervalBullet = 800;

void updateMap()
{
  if(start.getMap(14, 0) == 4)
  {
    start.Map(14, 0, 0);
  }
  if(start.getMap(14, 1) == 4)
  {
    start.Map(14, 1, 0);
  }
  if(start.getMap(0, 0) != 0 && start.getMap(0, 0) != 1)
  {
    start.Map(0, 0, 0);
  }
  if(start.getMap(0, 1) != 0 && start.getMap(0, 1) != 1)
  {
    start.Map(0, 1, 0);
  }
  if((start.getMap(1, 0) == 2 || start.getMap(1, 0) == 3) && start.getMap(0, 0) == 1)
  {
    start.Map(1, 0, 0);
    player.loseLife();
  }
  else if((start.getMap(1, 1) == 2 || start.getMap(1, 1) == 3) && start.getMap(0, 1) == 1)
  {
    start.Map(1, 1, 0);
    player.loseLife();
  }
  
  for(int j = 0; j < 2; j++)
  {
    for(int i = 1; i < 16; i++)
    {
      if(start.getMap(i, j) == 4)
      {
        if(start.getMap(i + 1, j) == 3)
        {
          start.Map(i + 1, j, 0);
          start.Map(i, j, 0);
        }
        else if(start.getMap(i + 1, j) == 2)
        {
          start.Map(i, j, 0);
        }
        else
        {
          start.Map(i, j, 0);
          start.Map(i + 1, j, 4);
          i++;
        }
      }
      if(start.getMap(i, j) == 2)
      {
        start.Map(i - 1, j, 2);
        start.Map(i, j, 0);
      }
      if(start.getMap(i, j) == 3 && start.getMap(i - 1, j) == 4)
      {
        start.Map(i - 1, j, 0);
        start.Map(i, j, 0);
      }
      else if(start.getMap(i, j) == 3 && start.getMap(i - 1, j) == 0)
      {
        start.Map(i - 1, j, 3);
        start.Map(i, j, 0);
      }
    }
  }
  count++;
  if(count - lastcount >= difference)
  {
      placeBlock();
      difference = random(2, 6);
      lastcount = count;
  }
  checkLives();
}

void setup() 
{
	// set up the LCD's number of columns and rows:
  Serial.begin(9600);
	lcd.begin(16, 2);
  pinMode(9, OUTPUT);
	// Clears the LCD screen
	lcd.clear();
  digitalWrite(9, LOW);
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
  lcd.createChar(1, Ship);
  lcd.createChar(2, Obstacle);
  lcd.createChar(3, Block);
  lcd.createChar(4, Bullet);
  randomSeed(millis());
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
}

void loop() 
{
  if(start.getStage() == 0)
  {
      lcd.setCursor(0, 0);
    lcd.print("Start game");
    lcd.setCursor(0, 1);
    lcd.print("Press play");
    if (IrReceiver.decode()) 
    {

      /*
        * Print a short summary of received data
        */
      IrReceiver.printIRResultShort(&Serial);
      IrReceiver.printIRSendUsage(&Serial);
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
          Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
          // We have an unknown protocol here, print more info
          IrReceiver.printIRResultRawFormatted(&Serial, true);
      }
      Serial.println();

      /*
        * Print the button code
        */
      Serial.print(F("Button Code: 0x"));
      Serial.println(IrReceiver.decodedIRData.command, HEX);

      /*
        * !!!Important!!! Enable receiving of the next value,
        * since receiving has stopped after the end of the current received data packet.
        */
      IrReceiver.resume(); // Enable receiving of the next value

      /*
        * Finally, check the received data and perform actions according to the received command
        */
      if (IrReceiver.decodedIRData.command == 0x43) {
          start.setStage(1);
          lcd.clear();
          delay(200);
      }
    }
  }
  else if(start.getStage() == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Press");
    lcd.setCursor(0, 1);
    lcd.print("Select");
    delay(2500);
    start.setStage(2);
  }
  else if(start.getStage() == 2)
  {
      int sensorReading = analogRead(A1);
    //Serial.println(sensorReading);
    // Print a message to the LCD.
    lcd.setCursor(0, 0);
    lcd.print("Select difficulty");

    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // Print a message to the LCD.
    if(sensorReading > 682)
    {
        start.setDifficulty(3);
        lcd.print("Difficulty is 3");
    }
    else if(sensorReading > 340)
    {
      start.setDifficulty(2);
      lcd.print("Difficulty is 2");
    }
    else 
    {
      start.setDifficulty(1);
        lcd.print("Difficulty is 1");
    }
    int selectRead = analogRead(A0);
    if(selectRead > 600 && selectRead < 800)
    {
      Serial.println(selectRead);
      delay(500);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("You have");
      lcd.setCursor(0, 1);
      lcd.print("3 lives");
      delay(2000);
      lcd.clear();
      start.setStage(3);
      score = millis();
      player.setSpeed(start.getDifficulty());
      if(player.getSpeed() == 1)
      {
        interval = 300;
      }
      else if(player.getSpeed() == 2)
      {
        interval = 200;
      }
      else if(player.getSpeed() == 3)
      {
        interval = 150;
      }
    }
  }
  else if(start.getStage() == 3)
  {
      start.showMap();
      //player.showLives();
      start.checkButtons();
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillis >= interval)
      {
        previousMillis = currentMillis;
        updateMap();
      }
      if((currentMillis - previousMillisBullet >= intervalBullet) && analogRead(A0) > 0 && analogRead(A0) < 60)
      {
        start.createBullet();
        previousMillisBullet = currentMillis;
      }
      player.showLed();
      delay(50);
      lcd.clear();
  }
  else if(start.getStage() == 4)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You lost +Select");
    lcd.setCursor(0, 1);
    lcd.print("Score:");
    lcd.print(score);
    if(analogRead(A0) > 600 && analogRead(A0) < 800)
    {
      player.setLives(3);
      for(int i = 0; i < 16; i++)
      {
        for(int j = 0; j < 2; j++)
        {
          start.Map(i, j, 0);
        }
      }
      start.Map(0, 0, 1);
      start.setStage(0);
      lcd.clear();
    }
    delay(200);
    //player.updateScore();
    //lcd.print(player.getScore());
  }
  //Serial.println(start.getStage());
}