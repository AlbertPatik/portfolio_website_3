#include <LiquidCrystal_I2C.h>

void update_frame();

LiquidCrystal_I2C lcd(32, 16, 2); 
const int BUTTON_PIN = 7;

byte player_char[8] = {
  B00000, // Empty
  B01010, //  # #  (Eyes)
  B00000, // Empty
  B01110, //  ###  (Mouth)
  B00000, // Empty
  B00000, // Empty
  B00000, // Empty
  B00000  // Empty
};

byte obstacle_char[8] = {
  B11111, // #####
  B11111, // #####
  B11111, // #####
  B11111, // #####
  B11111, // #####
  B11111, // #####
  B11111, // #####
  B11111  // #####
};

// Update these constants to use the custom character IDs
const uint8_t PLAYER_SPRITE_ID = 0; // ID for custom player character
const uint8_t OBSTACLE_SPRITE_ID = 1; // ID for custom obstacle character

void setup() {
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // use the noise from unconnected analog pin as seed
  randomSeed(analogRead(A0));
  
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.backlight();
  lcd.display();
  
  lcd.print("GET READY");
  
  lcd.createChar(0, player_char);
  lcd.createChar(1, obstacle_char);
  
  delay(2000);
  update_frame();
}

unsigned int P_COL = 1;
unsigned int P_ROW = 1;

char PLAYER = 'X';
char OBSTACLE = 'O';

unsigned int speed = 150; // milliseconds to obstacle move

unsigned long t0 = 0;
unsigned long t_accumulated = speed + 1;

struct OBSTACLE {
  int row;
  int col;
  
  OBSTACLE(int _col, int _row) {
  	
    row = _row;
    col = _col;
  }
};

struct OBSTACLES {

  unsigned int len;
  struct OBSTACLE* obstacles[4];
};

struct OBSTACLES obstacles = {0, {nullptr, nullptr, nullptr, nullptr}};

bool is_alive = true;

bool currBPressed = false;
bool lastBPressed = false;

void loop() {

  Serial.println(t_accumulated);
  
  if (!is_alive) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("GAME OVER!");
    delay(1000);
    return;
  }
  
  bool data_changed = false;
  unsigned int delta_t = millis() - t0;
  t0 = millis();
  
  currBPressed = digitalRead(BUTTON_PIN);
  
  // handle player logic

  if (currBPressed != lastBPressed) {
  	if (P_ROW == 0) P_ROW = 1;
    else P_ROW = 0;
    
    data_changed = true;
  }
  
  // spawn obstacles
  
  if (obstacles.len == 0 || obstacles.obstacles[obstacles.len - 1]->col < 11) {
  	int row = random(0, 2);
    obstacles.obstacles[obstacles.len] = new struct OBSTACLE(15, row);
    obstacles.len += 1;
  }
  
  // update obstacles
  
  t_accumulated += delta_t;
  
  if (t_accumulated > speed) data_changed = true;
  
  unsigned int delta_x = 0;
  while (t_accumulated > speed) {
    t_accumulated -= speed;
    delta_x += 1;
  }
  
  if (delta_x > 0) {
  	for (int i = 0; i < obstacles.len; i++) {
    	struct OBSTACLE* curr_o = obstacles.obstacles[i];
    	curr_o->col -= delta_x;
  	}
  }
  
  // update_frame
  
  if (data_changed) {
    update_frame();
  }
  
  // check for collisions
  
  for (int i = 0; i < obstacles.len; i++) {
    struct OBSTACLE* curr_o = obstacles.obstacles[i];
    if (curr_o->row == P_ROW && curr_o->col == P_COL) is_alive = false;
  }
  
  // remove out of bounds obstacles
  
  if (obstacles.len > 0) {
  	struct OBSTACLE* first_o = obstacles.obstacles[0];
    if (first_o->col < 0) {
      delete first_o;
      for (int i = 1; i < obstacles.len; i++) {
      	obstacles.obstacles[i - 1] = obstacles.obstacles[i];
      }
      obstacles.len -= 1;
    }
  }
  
  // update_data
  lastBPressed = currBPressed;
  
}

void update_frame() {
 
  lcd.clear();
  
  // print player 
  lcd.setCursor(P_COL, P_ROW); 
  lcd.write(PLAYER_SPRITE_ID);
  
  // upadte obstacles
  for (int i = 0; i < obstacles.len; i++) {
    struct OBSTACLE* curr_o = obstacles.obstacles[i];
    lcd.setCursor(curr_o->col, curr_o->row);
	lcd.write(OBSTACLE_SPRITE_ID);
  }
}