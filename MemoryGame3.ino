#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#include <EEPROM.h>

#define MENU 0
#define DISPLAY 1
#define INPUTTING 2
#define MODE_SELECTOR 3
#define DIFFICULTY_SELECTOR 4
#define SEQUENCE_SELECTOR 5
#define M_SELECTOR 6
#define T_SELECTOR 7
#define PLAY_SELECTOR 8
#define HIGHSCORE_VIEW 9
#define VIEW_SELECTOR 10
#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define RED 0x1
#define WHITE 0x7
#define GREEN 0x2

//Defining custom character arrows {Left,Right,Up,Down}
byte UpDown[] = {B00100,B01110,B10101,B00100,B00100,B10101,B01110,B00100};
byte directions[] = {LEFT, RIGHT, UP, DOWN};
byte sequence[50];
String difficultyOptions[] = {"EASY", "AVERAGE", "HARD"};
int score;
int sLength = 1;
int ogsLength;
int maxLength = 50;
int sizeM = 2;
int pos;
bool correct;
bool endGame;
bool pressed = false;
int viewDelay = 500;
String mode = "Story";
int state = MENU;
int menuState = MODE_SELECTOR;
bool messageDisplay = false;
int difficulty = 0;
int hsCount = 0;
int delayTime = 0;
int tDelay = 100;
//Setting EEPROM address for highscore values
int easyHS = 5;
int avgHS = 6;
int hardHS = 7;
String highscores[3];

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  //Initialise custom up-down character
  lcd.createChar(0, UpDown);
  //Display welcome screen before viewing menu
  lcd.setBacklight(GREEN);
  lcd.print("MEMORY");
  lcd.setCursor(0, 1);
  lcd.print("GAME");
  delay(1000);
  lcd.setBacklight(WHITE);
  state = MENU;
  randomSeed(analogRead(0));
  Serial.println("STARTUP COMPLETE");
  //FOR TESTING ONLY: Set all EEPROM highscore values to zero for the purpose of the test plan
  //Please remove after initial use if you would like to test that EEPROM writing works
  EEPROM.write(easyHS, 0);
  EEPROM.write(avgHS, 0);
  EEPROM.write(hardHS, 0);
}

void loop() {
  //switch statement for FSM
  switch (state) {
    case DISPLAY:
      displayInstructions();
      messageDisplay = false;
      break;
    case INPUTTING:
      input();
      messageDisplay = false;
      break;
    case MENU:
      menu();
      break;
  }
}
//Menu function where user can customise their game
void menu() {
  //Switch statement for different menu states
  switch (menuState) {
    case MODE_SELECTOR:
      modeMenu();
      break;
    case DIFFICULTY_SELECTOR:
      difficultyMenu();
      break;
    case SEQUENCE_SELECTOR:
      sequenceMenu();
      break;
    case M_SELECTOR:
      m_selectorMenu();
      break;
    case T_SELECTOR:
      t_selectorMenu();
      break;
    case VIEW_SELECTOR:
      view_selectorMenu();
      break;
    case PLAY_SELECTOR:
      playMenu();
      break;
    case HIGHSCORE_VIEW:
      highscoreMenu();
  }
}
//User can select either story mode or practice mode
void modeMenu() {
  uint8_t buttons = lcd.readButtons();
  //if mode menu hasn't been viewed yet then set to default values
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("Mode ->");
    lcd.setCursor(0, 1);
    lcd.print(String(mode) + String(" ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Use up and down button to toggle between story and practice mode
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("Mode ->");
    if (mode == "Story") {
      mode = "Practice";
    } else {
      mode = "Story";
    }
    lcd.setCursor(0, 1);
    lcd.print(String(mode) + String(" ") + char(0));
    delay(100);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("Mode ->");
    if (mode == "Practice") {
      mode = "Story";
    } else {
      mode = "Practice";
    }
    lcd.setCursor(0, 1);
    lcd.print(String(mode) + String(" ") + char(0));
    delay(100);
  }
  //User can press right button or select button to proceed to next menu item
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    //If story mode is selected user can select difficulty, else user can customise each aspect of game
    if (mode == "Story") {
      menuState = DIFFICULTY_SELECTOR;
    } else {
      menuState = SEQUENCE_SELECTOR;
    }
  }
}
//If story mode is selected user can select difficulty
void difficultyMenu() {
  uint8_t buttons = lcd.readButtons();
  //If difficulty menu not displayed before then show default values
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<- Difficulty ->");
    lcd.setCursor(0, 1);
    lcd.print(String(difficultyOptions[difficulty]) + String(" ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Use up and down buttons to toggle between Easy, Average and Hard difficulty options
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<- Difficulty ->");
    if (difficulty >= 2) {
      difficulty = 0;
    } else {
      difficulty++;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(difficultyOptions[difficulty]) + String(" ") + char(0));
    delay(100);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<- Difficulty ->");
    if (difficulty < 1) {
      difficulty = 0;
    } else {
      difficulty--;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(difficultyOptions[difficulty]) + String(" ") + char(0));
    delay(100);
  }
  //If easy, initial sequence length = 2, initial viewing time is 1000 and initial input time is 150
  if (difficulty == 0) {
    sLength = 2;
    viewDelay = 1000;
    tDelay = 150;
  }
  //If average, initial sequence length = 3, initial viewing time is 700 and initial input time is 100
  if (difficulty == 1) {
    sLength = 3;
    viewDelay = 700;
    tDelay = 100;
  }
  //If hard, initial sequence length = 5, initial viewing time is 500 and initial input time is 50
  if (difficulty == 2) {
    sLength = 5;
    viewDelay = 500;
    tDelay = 50;
  }
  //Right or select button to move to PLAY GAME menu option
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    menuState = PLAY_SELECTOR;
  }
  //Left button to go back to mode selector
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = MODE_SELECTOR;
  }
}
//Menu to customise sequence length, only accessible in practice mode
void sequenceMenu() {
  uint8_t buttons = lcd.readButtons();
  //If sequence menu not displayed before then set to default values
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<- Sequence ->");
    lcd.setCursor(0, 1);
    lcd.print(String(sLength) + String(" ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Up and down buttons to toggle between sequence length, from 1 to max length
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<- Sequence ->");
    if (sLength >= maxLength) {
      sLength = 1;
    } else {
      sLength++;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(sLength) + String(" ") + char(0));
    delay(100);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<- Sequence ->");
    if (sLength <= 1) {
      sLength = maxLength;
    } else {
      sLength--;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(sLength) + String(" ") + char(0));
    delay(100);
  }
  //Right or select button to continue to M selector menu item
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    menuState = M_SELECTOR;
  }
  //Left button to go back and toggle mode
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = MODE_SELECTOR;
  }
}
//Menu to customise M value, number of potential unique characters to be displayed
void m_selectorMenu() {
  uint8_t buttons = lcd.readButtons();
  //If M menu hasn't been displayed before set to default value
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<No.of options->");
    lcd.setCursor(0, 1);
    lcd.print(String(sizeM) + String(" ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Up and down buttons to toggle between 2-4 for value of M
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<No.of options->");
    if (sizeM >= 4) {
      sizeM = 2;
    } else {
      sizeM++;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(sizeM) + String(" ") + char(0));
    delay(100);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<No.of options->");
    if (sizeM <= 2) {
      sizeM = 4;
    } else {
      sizeM--;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(sizeM) + String(" ") + char(0));
    delay(100);
  }
  //Right or select button to continue to t selector menu
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    menuState = T_SELECTOR;
  }
  //Left button to go back to sequence selector menu
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = SEQUENCE_SELECTOR;
  }
}
//Menu to select value of time user has to input each character
void t_selectorMenu() {
  uint8_t buttons = lcd.readButtons();
  //If menu not previously displayed then set values to default
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<- Time Limit ->");
    lcd.setCursor(0, 1);
    lcd.print(String(tDelay) + String("ms ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Up and down buttons to toggle time limit in increments of 10
  //Max value is 300, minimum value is 40
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<- Time Limit ->");
    if (tDelay >= 300) {
      tDelay = 40;
    } else {
      tDelay += 10;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(tDelay) + String("ms ") + char(0));
    //delay(10);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<- Time Limit ->");
    if (tDelay <= 40) {
      tDelay = 300;
    } else {
      tDelay -= 10;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(tDelay) + String("ms ") + char(0));
    //delay(10);
  }
  //Right or select button to move on to viewing time selector menu
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    menuState = VIEW_SELECTOR;
  }
  //Left button to go back to M selector menu
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = M_SELECTOR;
  }
}
//Menu to select viewing time of each character in the sequence
void view_selectorMenu() {
  uint8_t buttons = lcd.readButtons();
  //If menu not yet displayed then set values to default
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<- View Time ->");
    lcd.setCursor(0, 1);
    lcd.print(String(viewDelay) + String("ms ") + char(0));
    messageDisplay = true;
  }
  lcd.setCursor(0, 1);
  //Up and down buttons to toggle view time in increments of 20
  //Max value is 1500 and min value is 150
  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<- View Time ->");
    if (viewDelay >= 1500) {
      viewDelay = 150;
    } else {
      viewDelay += 20;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(viewDelay) + String("ms ") + char(0));
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<- View Time ->");
    if (viewDelay <= 150) {
      viewDelay = 1500;
    } else {
      viewDelay -= 20;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(viewDelay) + String("ms ") + char(0));
  }
  //Right or select button to move onto PLAY menu option
  if (buttons & (BUTTON_RIGHT | BUTTON_SELECT)) {
    messageDisplay = false;
    delay(300);
    menuState = PLAY_SELECTOR;
  }
  //Left button to go back to input time selector
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = T_SELECTOR;
  }
}
//Menu option to allow user to start game
void playMenu() {
  uint8_t buttons = lcd.readButtons();
  if (!messageDisplay) {
    lcd.clear();
    lcd.print("<- PLAY ->");
    lcd.setCursor(0, 1);
    lcd.print("SELECT to PLAY");
    messageDisplay = true;
  }
  if (buttons & BUTTON_SELECT) {
    //If story mode then record initial sequence length needed for increasing difficulty
    //Set initial M = 2 which is then increased during gameplay
    if (mode == "Story") {
      ogsLength = sLength;
      sizeM = 2;
    }
    //Set initial score to zero and display start game sequence to prepare user
    score = 0;
    lcd.clear();
    lcd.print("Repeat after");
    lcd.setCursor(0, 1);
    lcd.print("sequence finish");
    delay(1500);
    lcd.clear();
    lcd.print("READY? ");
    delay(300);
    lcd.print("3 ");
    delay(600);
    lcd.print("2 ");
    delay(600);
    lcd.print("1...");
    delay(600);
    lcd.clear();
    delay(600);
    state = DISPLAY;
  }
  //Left button to go back to previous menu option
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    //If practice then previous is view selector, else go back to difficulty
    if (mode == "Practice") {
      menuState = VIEW_SELECTOR;
    } else {
      menuState = DIFFICULTY_SELECTOR;
    }
  }
  //Right button to go to highscore viewer
  if (buttons & BUTTON_RIGHT) {
    messageDisplay = false;
    delay(300);
    menuState = HIGHSCORE_VIEW;
  }
}
//Menu to view current highscores stored in EEPROM
void highscoreMenu() {
  uint8_t buttons = lcd.readButtons();
  if (!messageDisplay) {
    lcd.clear();
    highscores[0] = "Easy HS: "+(String)EEPROM.read(easyHS);
    highscores[1] = "Avg HS: "+(String)EEPROM.read(avgHS);
    highscores[2] = "Hard HS: "+(String)EEPROM.read(hardHS);
    lcd.print("<- HIGHSCORES");
    lcd.setCursor(0, 1);
    lcd.print(String(highscores[hsCount]) + String(" ") + char(0));
    messageDisplay = true;
  }

  if (buttons & BUTTON_UP) {
    lcd.clear();
    lcd.print("<- HIGHSCORES");
    if (hsCount >= 2) {
      hsCount = 0;
    } else {
      hsCount++;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(highscores[hsCount]) + String(" ") + char(0));
    delay(100);
  }
  if (buttons & BUTTON_DOWN) {
    lcd.clear();
    lcd.print("<- HIGHSCORES");
    if (hsCount < 1) {
      hsCount = 2;
    } else {
      hsCount--;
    }
    lcd.setCursor(0, 1);
    lcd.print(String(highscores[hsCount]) + String(" ") + char(0));
    delay(100);
  }
  
  //Left button to go back to PLAY
  if (buttons & BUTTON_LEFT) {
    messageDisplay = false;
    delay(300);
    menuState = PLAY_SELECTOR;
  }
}
//Function to display instruction sequence for user to memorise
void displayInstructions() {
  lcd.setBacklight(WHITE);
  //If statement to check if it is time to increase difficult in story mode
  if (sLength >= (ogsLength + 3) && sizeM < 4 && mode == "Story") {
    //Reset sLength to original sLength, increase M and decrease viewing time
    sLength = ogsLength;
    sizeM ++;
    viewDelay *= 0.7;
    lcd.clear();
    Serial.println("Increasing difficulty");
    lcd.print("INCREASING");
    lcd.setCursor(0, 1);
    lcd.print("DIFFICULTY");
    delay(1000);
  }
  pos = 1;
  correct = false;
  pressed = false;
  lcd.clear();
  //Generating, displaying and storing random sequence
  for (int i = 1; i <= sLength; i++) {
    byte instruction;
    //Select random index (between 0 and sizeM) from directions array
    instruction = directions[random(0, sizeM)];

    //If instruction is left or right then concatenate byte with customCharLR (4) to show full arrow
    if (instruction == RIGHT) {
      rightArrow();
    } else if (instruction == LEFT) {
      leftArrow();
    } else if (instruction == UP) {
      upArrow();
    } else {
      downArrow();
    }
    //Add instruction to sequence for validation of user input
    sequence[i] = instruction;
    Serial.println(instruction);
    //Show character on screen for viewDelay
    delay(viewDelay);
    //Clear screen for at least 150ms so user can differentiate between same characters
    lcd.clear();
    if (i != sLength) {
      if (viewDelay > 250) {
        delay(viewDelay / 1.5);
      } else {
        delay(150);
      }
    }
  }
  state = INPUTTING;
}

//Function for to take and validate user inputs
void input() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    //Reset time limit for character input each time user inputs a character
    delayTime = 0;
    pressed = true;
    endGame = false;
    //For each button input check if button pressed matches the sequence array
    //pos is initially zero and increases with each button press to match up with sequence array
    if (buttons & BUTTON_LEFT) {
      if (sequence[pos] == LEFT) {
        correct = true;
      } else {
        correct = false;
      }
    }
    if (buttons & BUTTON_RIGHT) {
      if (sequence[pos] == RIGHT) {
        correct = true;
      } else {
        correct = false;
      }
    }
    if (buttons & BUTTON_UP) {
      if (sequence[pos] == UP) {
        correct = true;
      } else {
        correct = false;
      }
    }
    if (buttons & BUTTON_DOWN) {
      if (sequence[pos] == DOWN) {
        correct = true;
      } else {
        correct = false;
      }
    }
    //User can press select at any point to end the game
    if (buttons & BUTTON_SELECT) {
      correct = false;
      endGame = true;
    }
  } else {
    //Increase delay time for each ms user doesn't press a button
    delayTime += 1;
    lcd.clear();
    //Display time left to input a character on screen as total time allowed - current time
    int delayPrint = (tDelay - delayTime);
    lcd.print(delayPrint);
    //If user hasn't inputted character in time then set answer as incorrect
    if (delayTime > tDelay) {
      Serial.println("Time limit exceeded");
      correct = false;
      pressed = true;
      delayTime = 0;
    }

    if (pressed) {
      //If incorrect answer then initialise failure screen
      if (!correct) {
         lcd.clear();
         //If user has chosen to end game then do not display 'red' game over
        if (endGame){
          lcd.print("GAME ENDED");
          delay(2000);
        } else {
        lcd.setBacklight(RED);
        lcd.print("    G A M E    ");
        lcd.setCursor(0, 1);
        lcd.print("    O V E R    ");
        delay(2000);
        }
        lcd.clear();
        lcd.setBacklight(WHITE);
        //Display user score and show current highscore
        lcd.print("SCORE: ");
        lcd.print(score);
        if (mode == "Story"){
          //Check difficulty to decide what highscore to compare to and display
          if(difficulty == 0){        
            int easyHighscore = EEPROM.read(easyHS);
            //Check if current score is higher than difficulty high score
            if(score > easyHighscore){
              EEPROM.write(easyHS, score);
              lcd.clear();
              lcd.setBacklight(GREEN);
              lcd.print("NEW HIGHSCORE");
              lcd.setCursor(0,1);
              lcd.print("Easy Score: ");
              lcd.print(score);
            } else {
              lcd.setCursor(0,1); 
              lcd.print("EASY HS: ");
              lcd.print(easyHighscore);
            }
        } else if (difficulty == 1){
          int avgHighscore = EEPROM.read(avgHS);
            if(score > avgHighscore){
              EEPROM.write(avgHS, score);
              lcd.clear();
              lcd.setBacklight(GREEN);
              lcd.print("NEW HIGHSCORE");
              lcd.setCursor(0,1);
              lcd.print("Avg Score: ");
              lcd.print(score);
            } else {
              lcd.setCursor(0,1); 
              lcd.print("AVERAGE HS: ");
              lcd.print(avgHighscore);
            }
        } else {
          int hardHighscore = EEPROM.read(hardHS);
            if(score > hardHighscore){
              EEPROM.write(hardHS, score);
              lcd.clear();
              lcd.setBacklight(GREEN);
              lcd.print("NEW HIGHSCORE");
              lcd.setCursor(0,1);
              lcd.print("Hard Score: ");
              lcd.print(score);
            } else {
              lcd.setCursor(0,1); 
              lcd.print("HARD HS: ");
              lcd.print(hardHighscore);
            }
        }         
        }
        delay(4000);
        lcd.setBacklight(WHITE);
        lcd.clear();
        //Go back to mode selector once game over
        menuState = MODE_SELECTOR;
                    state = MENU;
      }
      //else answer is correct
      else {
        //If user hasn't inputted all characters from sequence then increase pos
        if (pos != sLength) {
          pos ++;
          pressed = false;
        } else {
          //If all characters from sequence inputted correctly then display CORRECT screen
          lcd.clear();
          lcd.setBacklight(GREEN);
          lcd.print("CORRECT");
          Serial.println("sLength:" + String(sLength));
          Serial.println("pos:" + String(pos));
          delay(1000);
          score += 1;
          //If story mode and max sequence length not reached then increase sequence length
          if ((mode == "Story") && (sLength < (maxLength - 1))) {
            sLength++;
          }
          //Change state to DISPLAY to display character sequence again
          state = DISPLAY;
        }
      }
    }
  }
}

//Functions to create custom chars and print a left,right,up,down arrows

void rightArrow() {
  byte topBlock[] = {B00000,B00000,B00000,B00000,B00000,B00000,B11111,B11111};
  byte bottomBlock[] = {B11111,B11111,B00000,B00000,B00000,B00000,B00000,B00000};
  byte bottomArrow[] = {B11111,B11110,B11100,B11000,B10000,B00000,B00000,B00000};
  byte topArrow[] = {B00000,B00000,B00000,B10000,B11000,B11100,B11110,B11111};
  lcd.createChar(1, topBlock);
  lcd.createChar(2, bottomBlock);
  lcd.createChar(3, bottomArrow);
  lcd.createChar(4,topArrow);
  lcd.setCursor(0,0);
  lcd.write(1);
  lcd.setCursor(0,1);
  lcd.write(2);
  lcd.setCursor(1,0);
  lcd.write(1);
  lcd.setCursor(1,1);
  lcd.write(2);
  lcd.setCursor(2,0);
  lcd.write(1);
  lcd.setCursor(2,1);
  lcd.write(2);
  lcd.setCursor(3,0);
  lcd.write(4);
  lcd.setCursor(3,1);
  lcd.write(3);
}

void leftArrow() {
  byte topBlock[] = {B00000,B00000,B00000,B00000,B00000,B00000,B11111,B11111};
  byte bottomBlock[] = {B11111,B11111,B00000,B00000,B00000,B00000,B00000,B00000};
  byte bottomArrow[] = {B11111,B01111,B00111,B00011,B00001,B00000,B00000,B00000};
  byte topArrow[] = {B00000,B00000,B00000,B00001,B00011,B000111,B01111,B11111};
  lcd.createChar(1, topBlock);
  lcd.createChar(2, bottomBlock);
  lcd.createChar(3, bottomArrow);
  lcd.createChar(4, topArrow);
  lcd.setCursor(0,0);
  lcd.write(4);
  lcd.setCursor(0,1);
  lcd.write(3);
  lcd.setCursor(1,0);
  lcd.write(1);
  lcd.setCursor(1,1);
  lcd.write(2);
  lcd.setCursor(2,0);
  lcd.write(1);
  lcd.setCursor(2,1);
  lcd.write(2);
  lcd.setCursor(3,0);
  lcd.write(1);
  lcd.setCursor(3,1);
  lcd.write(2);
}

void upArrow() {
  byte middleArrow[] = {B00100,B01110,B11111,B11111,B11111,B11111,B01110,B01110};
  byte leftArrow[] = {B00000,B00000,B00000,B00001,B00011,B00111,B00000,B00000};
  byte rightArrow[] = {B00000,B00000,B00000,B10000,B11000,B11100,B00000,B00000};
  byte bottomBlock[] = {B01110,B01110,B01110,B01110,B01110,B01110,B01110,B01110};
  lcd.createChar(1, middleArrow);
  lcd.createChar(2, leftArrow);
  lcd.createChar(3, rightArrow);
  lcd.createChar(4, bottomBlock);
  lcd.setCursor(0,0);
  lcd.write(2);
  lcd.setCursor(1,0);
  lcd.write(1);
  lcd.setCursor(2,0);
  lcd.write(3);
  lcd.setCursor(1,1);
  lcd.write(4);
}

void downArrow() {
 byte middleArrow[] = {B01110,B01110,B11111,B11111,B11111,B11111,B01110,B00100};
  byte leftArrow[] = {B00000,B00000,B00111,B00011,B00001,B00000,B00000,B00000};
  byte rightArrow[] = {B00000,B00000,B11100,B11000,B10000,B00000,B00000,B00000};
  byte topBlock[] = {B01110,B01110,B01110,B01110,B01110,B01110,B01110,B01110};
  lcd.createChar(1, middleArrow);
  lcd.createChar(2, leftArrow);
  lcd.createChar(3, rightArrow);
  lcd.createChar(4, topBlock);
  lcd.setCursor(0,1);
  lcd.write(2);
  lcd.setCursor(1,1);
  lcd.write(1);
  lcd.setCursor(2,1);
  lcd.write(3);
  lcd.setCursor(1,0);
  lcd.write(4);
}
