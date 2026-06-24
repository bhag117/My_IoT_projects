/*
=========================================================
 DOOM SLAYER (SASTHA VERSION SAAB)
 FULL UPGRADED VERSION
 ESP32 + SSD1306 OLED
=========================================================

FEATURES
- 3 LEVELS
- FINAL BOSS (20 HITS)
- PLAYER DIES AFTER 5 BOSS HITS
- ENEMIES SHOOT PLAYER
- DOOM STYLE GUN
- TEXTURED WALLS
- HEALTH BAR
- LEVEL SELECT
- MAIN MENU
- QUIT SCREEN
- DETAILED ENEMIES
- ENEMY PROJECTILES
- STABLE RAYCASTER
- AUTO RETURN TO MENU

=========================================================
 CONNECTIONS
=========================================================

OLED SDA -> GPIO 21
OLED SCL -> GPIO 22
OLED VCC -> 3.3V
OLED GND -> GND

FORWARD BUTTON  -> GPIO 32
BACKWARD BUTTON -> GPIO 33
LEFT BUTTON     -> GPIO 25
RIGHT BUTTON    -> GPIO 26
SHOOT BUTTON    -> GPIO 27

Buttons connect between GPIO and GND.

=========================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// =====================================================
// BUTTONS
// =====================================================

#define BTN_FORWARD 32
#define BTN_BACKWARD 33
#define BTN_LEFT 25
#define BTN_RIGHT 26
#define BTN_SHOOT 27

// =====================================================
// STATES
// =====================================================

#define STATE_MENU 0
#define STATE_GAME 1
#define STATE_LEVEL_SELECT 2
#define STATE_QUIT 3

int gameState = STATE_MENU;

int menuOption = 0;

// =====================================================
// MAPS
// =====================================================

#define MAP_W 10
#define MAP_H 10

int level1[MAP_H][MAP_W] =
{
{1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,0,1,1,0,1},
{1,0,0,0,1,0,0,1,0,1},
{1,0,1,0,1,1,0,1,0,1},
{1,0,1,0,0,0,0,1,0,1},
{1,0,1,1,1,1,0,1,0,1},
{1,0,0,0,0,1,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1}
};

int level2[MAP_H][MAP_W] =
{
{1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,1,1,1,0,1},
{1,0,1,0,0,0,0,1,0,1},
{1,0,1,0,1,1,0,1,0,1},
{1,0,1,0,0,1,0,1,0,1},
{1,0,1,1,0,1,0,1,0,1},
{1,0,0,0,0,1,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1}
};

int level3[MAP_H][MAP_W] =
{
{1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1}
};

int (*worldMap)[MAP_W] = level1;

// =====================================================
// PLAYER
// =====================================================

float posX = 1.5;
float posY = 1.5;

float dirX = 1;
float dirY = 0;

float planeX = 0;
float planeY = 0.66;

int health = 100;

int playerHits = 0;

// =====================================================
// ENEMIES
// =====================================================

#define MAX_ENEMIES 4

struct Enemy
{
  float x;
  float y;
  bool alive;
  int hp;
  unsigned long lastShot;
};

Enemy enemies[MAX_ENEMIES];

bool bossFight = false;

// =====================================================
// SETTINGS
// =====================================================

float moveSpeed = 0.08;
float rotSpeed = 0.05;

bool shooting = false;

unsigned long shootTimer = 0;

unsigned long quitTimer = 0;

int currentLevel = 0;

// =====================================================
// LOAD LEVEL
// =====================================================

void loadLevel(int lvl)
{
  currentLevel = lvl;

  if(lvl == 0) worldMap = level1;
  if(lvl == 1) worldMap = level2;
  if(lvl == 2) worldMap = level3;

  posX = 1.5;
  posY = 1.5;

  dirX = 1;
  dirY = 0;

  planeX = 0;
  planeY = 0.66;

  health = 100;

  playerHits = 0;

  bossFight = false;

  for(int i=0;i<MAX_ENEMIES;i++)
  {
    enemies[i].alive = false;
  }

  // LEVEL 1
  if(lvl == 0)
  {
    enemies[0] = {5.5,5.5,true,1,0};
    enemies[1] = {7.5,2.5,true,1,0};
  }

  // LEVEL 2
  if(lvl == 1)
  {
    enemies[0] = {4.5,4.5,true,2,0};
    enemies[1] = {7.5,7.5,true,2,0};
    enemies[2] = {8.5,2.5,true,2,0};
  }

  // BOSS LEVEL
  if(lvl == 2)
  {
    bossFight = true;

    enemies[0] = {5.5,5.5,true,20,0};
  }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  pinMode(BTN_FORWARD, INPUT_PULLUP);
  pinMode(BTN_BACKWARD, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SHOOT, INPUT_PULLUP);

  Wire.begin(21,22);

  Wire.setClock(100000);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    while(true)
    {
      delay(1);
    }
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(12,20);
  display.println("DOOM SLAYER");

  display.setCursor(2,35);
  display.println("(SASTHA VERSION SAAB)");

  display.display();

  delay(2000);

  loadLevel(0);
}

// =====================================================
// ROTATE
// =====================================================

void rotatePlayer(float angle)
{
  float oldDirX = dirX;

  dirX = dirX*cos(angle)-dirY*sin(angle);
  dirY = oldDirX*sin(angle)+dirY*cos(angle);

  float oldPlaneX = planeX;

  planeX = planeX*cos(angle)-planeY*sin(angle);
  planeY = oldPlaneX*sin(angle)+planeY*cos(angle);
}

// =====================================================
// INPUT
// =====================================================

void handleInput()
{
  if(digitalRead(BTN_FORWARD)==LOW)
  {
    float nx = posX + dirX*moveSpeed;
    float ny = posY + dirY*moveSpeed;

    if(worldMap[(int)posY][(int)nx]==0)
      posX = nx;

    if(worldMap[(int)ny][(int)posX]==0)
      posY = ny;
  }

  if(digitalRead(BTN_BACKWARD)==LOW)
  {
    float nx = posX - dirX*moveSpeed;
    float ny = posY - dirY*moveSpeed;

    if(worldMap[(int)posY][(int)nx]==0)
      posX = nx;

    if(worldMap[(int)ny][(int)posX]==0)
      posY = ny;
  }

  if(digitalRead(BTN_LEFT)==LOW)
  {
    rotatePlayer(-rotSpeed);
  }

  if(digitalRead(BTN_RIGHT)==LOW)
  {
    rotatePlayer(rotSpeed);
  }
}

// =====================================================
// SHOOTING
// =====================================================

void handleShooting()
{
  if(digitalRead(BTN_SHOOT)==LOW)
  {
    shooting = true;

    shootTimer = millis();

    for(int i=0;i<MAX_ENEMIES;i++)
    {
      if(!enemies[i].alive)
        continue;

      float dx = enemies[i].x-posX;
      float dy = enemies[i].y-posY;

      float dist = sqrt(dx*dx + dy*dy);

      dx /= dist;
      dy /= dist;

      float dot = dx*dirX + dy*dirY;

      if(dot > 0.96 && dist < 5)
      {
        enemies[i].hp--;

        if(enemies[i].hp <= 0)
        {
          enemies[i].alive = false;
        }
      }
    }
  }

  if(millis()-shootTimer > 100)
  {
    shooting = false;
  }
}

// =====================================================
// ENEMY ATTACK
// =====================================================

void enemyAttack()
{
  for(int i=0;i<MAX_ENEMIES;i++)
  {
    if(!enemies[i].alive)
      continue;

    float dx = posX - enemies[i].x;
    float dy = posY - enemies[i].y;

    float dist = sqrt(dx*dx + dy*dy);

    if(dist < 4.5)
    {
      if(millis() - enemies[i].lastShot > 1000)
      {
        enemies[i].lastShot = millis();

        if(currentLevel != 2)
        {
          health -= 8;
        }
        else
        {
          health -= 20;

          playerHits++;
        }

        // HIT FLASH
        display.fillRect(0,0,128,64,SSD1306_WHITE);
        display.display();

        delay(40);

        if(health <= 0 || playerHits >= 5)
        {
          gameState = STATE_MENU;

          loadLevel(0);

          return;
        }
      }
    }
  }
}

// =====================================================
// WALLS
// =====================================================

void drawWall(int x,int start,int end,bool dark)
{
  display.drawLine(x,start,x,end,SSD1306_WHITE);

  // TOP TEXTURE
  display.drawPixel(x,start,BLACK);
  display.drawPixel(x,start+1,BLACK);

  // BOTTOM TEXTURE
  display.drawPixel(x,end,BLACK);
  display.drawPixel(x,end-1,BLACK);

  // SHADOWS
  if(dark)
  {
    for(int y=start;y<end;y+=4)
    {
      display.drawPixel(x,y,BLACK);
    }
  }
}

// =====================================================
// ENEMY
// =====================================================

void drawEnemy(int sx,int sy,int size,bool boss)
{
  display.drawRect(
    sx-size/3,
    sy-size/2,
    size/1.5,
    size,
    SSD1306_WHITE
  );

  display.fillCircle(
    sx,
    sy-size/3,
    size/6,
    SSD1306_WHITE
  );

  // EYES
  display.drawPixel(sx-2,sy-size/3,BLACK);
  display.drawPixel(sx+2,sy-size/3,BLACK);

  // HORNS
  display.drawLine(sx-4,sy-size/2,sx-7,sy-size/2-4,SSD1306_WHITE);
  display.drawLine(sx+4,sy-size/2,sx+7,sy-size/2-4,SSD1306_WHITE);

  // BODY
  display.fillRect(
    sx-size/5,
    sy-size/8,
    size/2,
    size/2,
    SSD1306_WHITE
  );

  // ARMS
  display.drawLine(sx-size/3,sy,sx+size/3,sy,SSD1306_WHITE);

  // LEGS
  display.drawLine(sx-3,sy+size/4,sx-5,sy+size/2,SSD1306_WHITE);
  display.drawLine(sx+3,sy+size/4,sx+5,sy+size/2,SSD1306_WHITE);

  // BOSS DETAILS
  if(boss)
  {
    display.drawRect(
      sx-size/2,
      sy-size/2,
      size,
      size,
      SSD1306_WHITE
    );

    display.drawLine(
      sx-size/2,
      sy-size/2,
      sx+size/2,
      sy+size/2,
      SSD1306_WHITE
    );

    display.drawLine(
      sx+size/2,
      sy-size/2,
      sx-size/2,
      sy+size/2,
      SSD1306_WHITE
    );
  }
}

// =====================================================
// ENEMY PROJECTILES
// =====================================================

void drawEnemyProjectiles()
{
  for(int i=0;i<MAX_ENEMIES;i++)
  {
    if(!enemies[i].alive)
      continue;

    float spriteX = enemies[i].x - posX;
    float spriteY = enemies[i].y - posY;

    float invDet =
      1.0 / (planeX * dirY - dirX * planeY);

    float transformX =
      invDet * (dirY * spriteX - dirX * spriteY);

    float transformY =
      invDet * (-planeY * spriteX + planeX * spriteY);

    if(transformY <= 0)
      continue;

    int screenX =
      int((SCREEN_WIDTH / 2) *
      (1 + transformX / transformY));

    int size = abs(int(10 / transformY));

    if(size < 2)
      size = 2;

    display.fillCircle(
      screenX,
      SCREEN_HEIGHT / 2,
      size,
      SSD1306_WHITE
    );
  }
}

// =====================================================
// ENEMY RENDER
// =====================================================

void renderEnemies()
{
  bool allDead = true;

  for(int i=0;i<MAX_ENEMIES;i++)
  {
    if(!enemies[i].alive)
      continue;

    allDead = false;

    float spriteX = enemies[i].x-posX;
    float spriteY = enemies[i].y-posY;

    float invDet =
      1.0/(planeX*dirY-dirX*planeY);

    float transformX =
      invDet*(dirY*spriteX-dirX*spriteY);

    float transformY =
      invDet*(-planeY*spriteX+planeX*spriteY);

    if(transformY <= 0)
      continue;

    int screenX =
      int((SCREEN_WIDTH/2)*
      (1+transformX/transformY));

    int size = abs(int(42/transformY));

    if(size > 80)
      size = 80;

    drawEnemy(
      screenX,
      SCREEN_HEIGHT/2,
      size,
      currentLevel==2
    );
  }

  if(allDead)
  {
    if(currentLevel < 2)
    {
      loadLevel(currentLevel+1);
    }
    else
    {
      gameState = STATE_MENU;

      loadLevel(0);
    }
  }
}

// =====================================================
// GUN
// =====================================================

void drawGun()
{
  int x = 88;
  int y = shooting ? 44 : 48;

  display.fillRect(x,y,24,10,SSD1306_WHITE);

  display.fillRect(x+8,y-14,10,14,SSD1306_WHITE);

  display.fillRect(x+6,y+10,8,8,SSD1306_WHITE);

  display.drawLine(x,y+3,x+24,y+3,BLACK);
  display.drawLine(x,y+7,x+24,y+7,BLACK);

  if(shooting)
  {
    display.drawLine(x+13,y-18,x+4,y-26,SSD1306_WHITE);
    display.drawLine(x+13,y-18,x+22,y-26,SSD1306_WHITE);
    display.drawLine(x+13,y-18,x+13,y-30,SSD1306_WHITE);
  }
}

// =====================================================
// HUD
// =====================================================

void drawHUD()
{
  display.drawRect(2,2,52,8,SSD1306_WHITE);

  int hpWidth = health/2;

  display.fillRect(4,4,hpWidth,4,SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(60,2);

  display.print("LVL ");
  display.print(currentLevel+1);
}

// =====================================================
// MENU
// =====================================================

void drawMenu()
{
  display.clearDisplay();

  // BIG TITLE
  display.fillRect(8,4,110,18,SSD1306_WHITE);

  display.setTextColor(BLACK);
  display.setTextSize(2);

  display.setCursor(18,7);
  display.println("DOOM");

  display.setCursor(22,24);

  display.setTextColor(SSD1306_WHITE);

  display.println("SLAYER");

  // SUBTITLE
  display.setTextSize(1);

  display.setCursor(12,40);
  display.println("(SASTHA VERSION SAAB)");

  // MENU
  display.setCursor(22,50);
  display.println(menuOption==0 ? "> PLAY" : "  PLAY");

  display.setCursor(22,58);
  display.println(menuOption==1 ? "> LEVEL" : "  LEVEL");

  display.setCursor(78,58);
  display.println(menuOption==2 ? "> QUIT" : "  QUIT");

  display.display();
}

// =====================================================
// LEVEL SELECT
// =====================================================

void drawLevelSelect()
{
  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(25,15);
  display.println("SELECT LEVEL");

  display.setCursor(40,35);

  display.print("LEVEL ");
  display.print(currentLevel+1);

  display.setCursor(5,55);
  display.println("LEFT/RIGHT + SHOOT");

  display.display();

  if(digitalRead(BTN_LEFT)==LOW)
  {
    currentLevel--;

    if(currentLevel<0)
      currentLevel=2;

    delay(200);
  }

  if(digitalRead(BTN_RIGHT)==LOW)
  {
    currentLevel++;

    if(currentLevel>2)
      currentLevel=0;

    delay(200);
  }

  if(digitalRead(BTN_SHOOT)==LOW)
  {
    loadLevel(currentLevel);

    gameState = STATE_GAME;

    delay(300);
  }
}

// =====================================================
// QUIT
// =====================================================

void drawQuit()
{
  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(8,24);
  display.println("Looks like your mom");

  display.setCursor(18,40);
  display.println("wasted 9 months");

  display.display();

  if(millis()-quitTimer > 15000)
  {
    gameState = STATE_MENU;
  }
}

// =====================================================
// RENDER
// =====================================================

void render3D()
{
  display.clearDisplay();

  for(int x=0;x<SCREEN_WIDTH;x+=2)
  {
    float cameraX =
      2*x/float(SCREEN_WIDTH)-1;

    float rayDirX =
      dirX + planeX*cameraX;

    float rayDirY =
      dirY + planeY*cameraX;

    int mapX = int(posX);
    int mapY = int(posY);

    float sideDistX;
    float sideDistY;

    float deltaDistX;
    float deltaDistY;

    if(rayDirX == 0)
      deltaDistX = 1e30;
    else
      deltaDistX = abs(1/rayDirX);

    if(rayDirY == 0)
      deltaDistY = 1e30;
    else
      deltaDistY = abs(1/rayDirY);

    float perpWallDist;

    int stepX;
    int stepY;

    int hit = 0;
    int side;

    if(rayDirX < 0)
    {
      stepX = -1;
      sideDistX = (posX-mapX)*deltaDistX;
    }
    else
    {
      stepX = 1;
      sideDistX = (mapX+1.0-posX)*deltaDistX;
    }

    if(rayDirY < 0)
    {
      stepY = -1;
      sideDistY = (posY-mapY)*deltaDistY;
    }
    else
    {
      stepY = 1;
      sideDistY = (mapY+1.0-posY)*deltaDistY;
    }

    while(hit == 0)
    {
      if(sideDistX < sideDistY)
      {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      }
      else
      {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }

      if(worldMap[mapY][mapX] > 0)
      {
        hit = 1;
      }
    }

    if(side == 0)
      perpWallDist = sideDistX-deltaDistX;
    else
      perpWallDist = sideDistY-deltaDistY;

    if(perpWallDist <= 0)
      perpWallDist = 0.1;

    int lineHeight =
      int(SCREEN_HEIGHT/perpWallDist);

    int drawStart =
      -lineHeight/2+SCREEN_HEIGHT/2;

    if(drawStart < 0)
      drawStart = 0;

    int drawEnd =
      lineHeight/2+SCREEN_HEIGHT/2;

    if(drawEnd >= SCREEN_HEIGHT)
      drawEnd = SCREEN_HEIGHT-1;

    drawWall(x,drawStart,drawEnd,side);
    drawWall(x+1,drawStart,drawEnd,side);
  }

  renderEnemies();

  drawEnemyProjectiles();

  drawGun();

  drawHUD();

  display.display();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  // MENU
  if(gameState == STATE_MENU)
  {
    drawMenu();

    if(digitalRead(BTN_LEFT)==LOW)
    {
      menuOption--;

      if(menuOption < 0)
        menuOption = 2;

      delay(200);
    }

    if(digitalRead(BTN_RIGHT)==LOW)
    {
      menuOption++;

      if(menuOption > 2)
        menuOption = 0;

      delay(200);
    }

    if(digitalRead(BTN_SHOOT)==LOW)
    {
      if(menuOption == 0)
      {
        loadLevel(0);

        gameState = STATE_GAME;
      }

      if(menuOption == 1)
      {
        gameState = STATE_LEVEL_SELECT;
      }

      if(menuOption == 2)
      {
        gameState = STATE_QUIT;

        quitTimer = millis();
      }

      delay(300);
    }
  }

  // GAME
  else if(gameState == STATE_GAME)
  {
    handleInput();

    handleShooting();

    enemyAttack();

    render3D();
  }

  // LEVEL SELECT
  else if(gameState == STATE_LEVEL_SELECT)
  {
    drawLevelSelect();
  }

  // QUIT
  else if(gameState == STATE_QUIT)
  {
    drawQuit();
  }
}
