/*
 * ---------------- www.spacesimulator.net --------------
 *   ---- Space simulators and 3d engine tutorials ----
 *
 * Author: Damiano Vitulli
 *
 * This program is released under the BSD licence
 * By using this program you agree to licence terms on spacesimulator.net copyright page
 *
 *
 * Tutorial 4: 3d engine - 3ds models loader
 *
 *
 *
 * To compile this project you must include the following libraries:
 * opengl32.lib,glu32.lib,glut.lib
 * You need also the header file glut.h in your compiler directory.
 *
 */

 /*
  * 3DS Spaceship created by: Rene Reiter <renereiter@hotmail.com>
  */



#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include "tutorial4.h"
#include "texture.h"
#include "3dsloader.h"
#ifndef pi
#define pi 3.14159265358979323846
#endif
#define MAX_CHAR  128 // 顯示字串最長長度

//星星
#define NUM_POINTS 10
#define R_OUTER 80.0f
#define R_INNER 30.0f
#define DEPTH 30.0f
#include <ctime> // for time tracking

// 時間限制
float startTime = 0.0f;
float countdown = 30.0f; // 30 秒限制

//分數計算
int score = 0;
float lastPosZ = 0.0f; // 紀錄上一次的位置，用來計算距離得分

  /**********************************************************
   *
   * VARIABLES DECLARATION
   *
   *********************************************************/

   // The width and height of your window, change them as you like
int screen_width = 640;
int screen_height = 480;

// Absolute rotation values (0-359 degrees) and rotation increments for each frame
double rotation = 0.0;

// Flag for rendering as lines or filled polygons
int filling = 1; //0=OFF 1=ON

//Now the object is generic, the cube has annoyed us a little bit, or not?
obj_type object;
//obj_type object1;

float posX = 0.0f, posY = 0.0f, posZ = -200.0f; // 火箭起始點 P1
float rx1, ry1, rz1; // 火箭結束點 P2
float rx2 = 0.0f, ry2 = 0.0f, rz2 = 200.0f; // 火箭結束點 P2
//float ballX = 0.0f, ry2 = 0.0f, rz2 = 200.0f; // 火箭結束點 P2
float pos_test = 0.0;
float moveDirX = 0.0f, moveDirY = 0.0f, moveDirZ = 0.0f; //碰撞處理
float damping;

//球體
float radius = 100.0f;
float ballX = 0.0f;
float ballY = 0.0f;
float ballZ = -800.0f;
int sectorCount = 36;  // 經度切分
int stackCount = 18;   // 緯度切分
std::vector<float> vertices;
std::vector<unsigned int> indices;
obj_type sphere;

// 碰撞資訊
float contactX = 0, contactY = 0, contactZ = 0; //碰撞點位置
bool collided = false;
bool justCollided = false;  // 表示這一幀內剛碰撞過，防止重複扣生命


// 球位置
struct Sphere {
    float x, y, z;
    float radius;
};

std::vector<Sphere> sphereList = {
    {  300.0f, -200.0f, -1100.0f, radius },
    { -400.0f, -200.0f, -1200.0f, radius },
    {   10.0f,    0.0f, -1600.0f, radius },
    { -200.0f,   50.0f, -2000.0f, radius },
    {  400.0f,  200.0f, -2400.0f, radius },
    { -300.0f, -200.0f, -2800.0f, radius },
    {  100.0f, -400.0f, -3200.0f, radius }
};

// 遊戲結束
int life = 3;
bool gameOver = false;

// 星星位置與狀態（兩顆）
struct Star {
    float x, y, z;
    bool collected = false;
};
std::vector<Star> stars = {
    { -100.0f, 0.0f, -1200.0f, false },
    { 200.0f, 100.0f, -1800.0f, false }
};


obj_type sky_box =
{
    "skybox",
    36,
    12,

    {  //vertex
        // Front face
        // 對於 front face 的 6 個頂點：
        {-4000, -4000, 4000},   // 0 左下
        {4000, -4000, 4000},    // 1 右下
        {4000, 4000, 4000},     // 2 右上 
        {-4000, -4000, 4000},   // 3 左下
        {4000, 4000, 4000},     // 4 右上 
        {-4000, 4000, 4000},    // 5 左上

        // Right face
        {4000, -4000, 4000}, {4000, -4000, -4000}, {4000, 4000, -4000},
        {4000, -4000, 4000}, {4000, 4000, -4000}, {4000, 4000, 4000},
        // Back face
        {4000, -4000, -4000}, {-4000, -4000, -4000}, {-4000, 4000, -4000},
        {4000, -4000, -4000}, {-4000, 4000, -4000}, {4000, 4000, -4000},
        // Left face
        {-4000, -4000, -4000}, {-4000, -4000, 4000}, {-4000, 4000, 4000},
        {-4000, -4000, -4000}, {-4000, 4000, 4000}, {-4000, 4000, -4000},
        // Top face
        {-4000, 4000, 4000}, {4000, 4000, 4000}, {4000, 4000, -4000},
        {-4000, 4000, 4000}, {4000, 4000, -4000}, {-4000, 4000, -4000},
        // Bottom face
        {-4000, -4000, -4000}, {4000, -4000, -4000}, {4000, -4000, 4000},
        {-4000, -4000, -4000}, {4000, -4000, 4000}, {-4000, -4000, 4000}
    },


    {  //polygon
        {0, 1, 2}, {3, 4, 5},       // Front
        {6, 7, 8}, {9, 10,11},      // Right
        {12,13,14}, {15,16,17},     // Back
        {18,19,20}, {21,22,23},     // Left
        {24,25,26}, {27,28,29},     // Top
        {30,31,32}, {33,34,35}      // Bottom
    },
    {  //mapcoord
        // Front
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},

        // Right
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Back
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Left
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Top
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        // Bottom
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
    },
    0
};

//字串顯示
void drawString(const char* str) {

    static int isFirstCall = 1;
    static GLuint lists;

    if (isFirstCall) { // 如果是第一次調用，執行初始化

        // 爲每一個ASCII字符產生一個顯示列表

        isFirstCall = 0;

        // 申請MAX_CHAR個連續的顯示列表編號

        lists = glGenLists(MAX_CHAR);

        // 把每個字符的繪製命令都裝到對應的顯示列表中

        wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);

    }

    // 調用每個字符對應的顯示列表，繪製每個字符

    for (; *str != '\0'; ++str)
        glCallList(lists + *str);

}
void selectFont(int size, int charset, const char* face) {

    HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,

        charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,

        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);

    HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);

    DeleteObject(hOldFont);

}

//球體
void generateSphere()
{
    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = pi / 2 - i * pi / stackCount; // -π/2 ~ π/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * pi / sectorCount;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            float u = (float)j / sectorCount;
            float v = (float)i / stackCount;

            // 頂點：位置 + 貼圖座標
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // 三角形索引
    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = i * (sectorCount + 1) + j;
            int second = first + sectorCount + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void drawSphere()
{
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i += 3) {
        int i1 = indices[i] * 5;
        int i2 = indices[i + 1] * 5;
        int i3 = indices[i + 2] * 5;

        // 頂點 1
        glTexCoord2f(vertices[i1 + 3], vertices[i1 + 4]);
        glVertex3f(vertices[i1], vertices[i1 + 1], vertices[i1 + 2]);

        // 頂點 2
        glTexCoord2f(vertices[i2 + 3], vertices[i2 + 4]);
        glVertex3f(vertices[i2], vertices[i2 + 1], vertices[i2 + 2]);

        // 頂點 3
        glTexCoord2f(vertices[i3 + 3], vertices[i3 + 4]);
        glVertex3f(vertices[i3], vertices[i3 + 1], vertices[i3 + 2]);
    }
    glEnd();
}

/****************************************/
//碰撞偵測
/****************************************/
// P1, P2距離
float distance2(float ax, float ay, float az, float bx, float by, float bz) {
    return (ax - bx) * (ax - bx) + (ay - by) * (ay - by) + (az - bz) * (az - bz);
}

bool checkRocketHitBall(float ballX, float ballY, float ballZ, float ballRadius) {
    //火箭的方向向量（從尾端指向前端）
    float vx = rx2 - posX;
    float vy = ry2 - posY;
    float vz = rz2 - posZ;

    //球心和火箭尾端之間的向量差
    float dx = ballX - posX;
    float dy = ballY - posY;
    float dz = ballZ - posZ;

    float len2 = vx * vx + vy * vy + vz * vz;
    float t = (vx * dx + vy * dy + vz * dz) / len2;

    if (t < 0.0f || t > 1.0f) return false;

    float px = posX + vx * t;
    float py = posY + vy * t;
    float pz = posZ + vz * t;

    float d2 = distance2(px, py, pz, ballX, ballY, ballZ);
    float combinedRadius = radius + 80.0f; // 球半徑+火箭機翼
    if (d2 <= combinedRadius * combinedRadius) {
        contactX = px;
        contactY = py;
        contactZ = pz;
        return true;
    }
    return false;
}



/*星星*/
void DrawStar3D() {
    glRotatef(20, 0.0f, 0.0f, 1.0f);
    // 頂點計算
    float star[2][NUM_POINTS][3]; // [front/back][point][xyz]
    for (int side = 0; side < 2; ++side) {
        float z = (side == 0) ? DEPTH : -DEPTH;
        for (int i = 0; i < NUM_POINTS; ++i) {
            float a = i * 3.14159f / 5;
            float r = (i % 2 == 0) ? R_OUTER : R_INNER;
            star[side][i][0] = cos(a) * r;
            star[side][i][1] = sin(a) * r;
            star[side][i][2] = z;
        }
    }

    // 前面
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, DEPTH);
    for (int i = 0; i <= NUM_POINTS; ++i) {
        glVertex3fv(star[0][i % NUM_POINTS]);
    }
    glEnd();

    // 後面
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(0, 0, -DEPTH);
    for (int i = NUM_POINTS - 1; i >= 0; --i) {
        glVertex3fv(star[1][i]);
    }
    glEnd();

    // 側邊
    glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 0.0f);
    for (int i = 0; i < NUM_POINTS; ++i) {
        int next = (i + 1) % NUM_POINTS;
        glVertex3fv(star[0][i]);
        glVertex3fv(star[0][next]);
        glVertex3fv(star[1][next]);
        glVertex3fv(star[1][i]);
    }
    glEnd();
}


/**********************************************************
 *
 * SUBROUTINE init()
 *
 * Used to initialize OpenGL and to setup our world
 *
 *********************************************************/

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0); // This clear the background color to black
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons

    // Viewport transformation
    glViewport(0, 0, screen_width, screen_height);

    // Projection transformation
    glMatrixMode(GL_PROJECTION); // Specifies which matrix stack is the target for matrix operations 
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 10.0f, 10000.0f); // We define the "viewing volume"

    glEnable(GL_DEPTH_TEST); // We enable the depth test (also called z buffer)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)

    glEnable(GL_TEXTURE_2D); // This Enable the Texture mapping

    generateSphere();
    Load3DS(&object, "spaceship.3ds");
    //Load3DS(&object1, "101.3ds");

    object.id_texture = LoadBitmap("spaceshiptexture.bmp"); // The Function LoadBitmap() return the current texture ID
    //object1.id_texture = LoadBitmap("101.bmp");

    //skybox的圖片
    sky_box.id_texture2 = LoadBitmap("front.bmp");
    sky_box.id_texture3 = LoadBitmap("right.bmp");
    sky_box.id_texture4 = LoadBitmap("back.bmp");
    sky_box.id_texture5 = LoadBitmap("left.bmp");
    sky_box.id_texture6 = LoadBitmap("top.bmp");
    sky_box.id_texture7 = LoadBitmap("bottom.bmp");

    sphere.id_texture8 = LoadBitmap("rock.bmp");

    // If the last function returns -1 it means the file was not found so we exit from the program
    if (object.id_texture == -1)
    {
        MessageBox(NULL, "Image file: spaceshiptexture.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    //檢查skybox
    if (sky_box.id_texture2 == -1)
    {
        MessageBox(NULL, "Image file: texture2.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (sky_box.id_texture3 == -1)
    {
        MessageBox(NULL, "Image file: texture3.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (sky_box.id_texture4 == -1)
    {
        MessageBox(NULL, "Image file: texture4.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (sky_box.id_texture5 == -1)
    {
        MessageBox(NULL, "Image file: texture5.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (sky_box.id_texture6 == -1)
    {
        MessageBox(NULL, "Image file: texture6.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }
    if (sky_box.id_texture7 == -1)
    {
        MessageBox(NULL, "Image file: texture7.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }

    if (sphere.id_texture8 == -1)
    {
        MessageBox(NULL, "Image file: texture8.bmp not found", "Zetadeck", MB_OK | MB_ICONERROR);
        exit(0);
    }

    //設定只初始化一次
    static bool timerStarted = false;
    if (!timerStarted) {
        startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        timerStarted = true;
    }

}



/**********************************************************
 *
 * SUBROUTINE resize(int,int)
 *
 * This routine must be called everytime we resize our window.
 *
 *********************************************************/

void resize(int width, int height)
{
    screen_width = width; // We obtain the new screen width values and store it
    screen_height = height; // Height value

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // We clear both the color and the depth buffer so to draw the next frame
    glViewport(0, 0, screen_width, screen_height); // Viewport transformation

    glMatrixMode(GL_PROJECTION); // Projection transformation
    glLoadIdentity(); // We initialize the projection matrix as identity
    gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 10.0f, 10000.0f);

    glutPostRedisplay(); // This command redraw the scene (it calls the same routine of glutDisplayFunc)
}


/**********************************************************
 *
 * SUBROUTINE display()
 *
 * This is our main rendering subroutine, called each frame
 *
 *********************************************************/

void display(void)
{
    selectFont(48, ANSI_CHARSET, "Comic Sans MS");

    int l_index;
    // rx1 是火箭當前位置（posX, posY, posZ）
    rx1 = posX;
    ry1 = posY;
    rz1 = posZ;

    //設定火箭要飛的方向
    rx2 = posX;
    ry2 = posY;
    rz2 = posZ - 300.0f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // This clear the background color to dark blue
    glMatrixMode(GL_MODELVIEW); // Modeling transformation
    glLoadIdentity(); // Initialize the model matrix as identity
    
    gluLookAt(posX + pos_test, posY + 50, posZ + 600, posX, posY, posZ, 0, 1, 0);
    
    //飛機
    glBindTexture(GL_TEXTURE_2D, object.id_texture); // We set the active texture 
    
    glPushMatrix();
    damping = (float)rand() / (float)(RAND_MAX) * 0.3f;
    glRotatef(rotation, 0.0, 80.0, 0.0);
    glTranslatef(posX + damping, posY + damping, posZ + damping - 50.0);
    glRotatef(180, 0.0, 0.0, 1.0);
    glRotatef(90, 1.0, 0.0, 0.0);
    glScalef(0.6, 0.6, 0.6);
    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for (l_index = 0; l_index < object.polygons_qty; l_index++)
    {
        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].a].u,
            object.mapcoord[object.polygon[l_index].a].v);
        // Coordinates of the first vertex
        glVertex3f(object.vertex[object.polygon[l_index].a].x,
            object.vertex[object.polygon[l_index].a].y,
            object.vertex[object.polygon[l_index].a].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].b].u,
            object.mapcoord[object.polygon[l_index].b].v);
        // Coordinates of the second vertex
        glVertex3f(object.vertex[object.polygon[l_index].b].x,
            object.vertex[object.polygon[l_index].b].y,
            object.vertex[object.polygon[l_index].b].z);

        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f(object.mapcoord[object.polygon[l_index].c].u,
            object.mapcoord[object.polygon[l_index].c].v);
        // Coordinates of the Third vertex
        glVertex3f(object.vertex[object.polygon[l_index].c].x,
            object.vertex[object.polygon[l_index].c].y,
            object.vertex[object.polygon[l_index].c].z);
    }
    glEnd();
    glPopMatrix();
    
    // skybox
    //front
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture2);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();

    //right
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture3);
    glBegin(GL_TRIANGLES);
    for (int i = 2; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();

    //back
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture4);
    glBegin(GL_TRIANGLES);
    for (int i = 4; i < 6; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();

    //left
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture5);
    glBegin(GL_TRIANGLES);
    for (int i = 6; i < 8; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();

    //top
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture6);
    glBegin(GL_TRIANGLES);
    for (int i = 8; i < 10; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();

    //botton
    glBindTexture(GL_TEXTURE_2D, sky_box.id_texture7);
    glBegin(GL_TRIANGLES);
    for (int i = 10; i < 12; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = (j == 0) ? sky_box.polygon[i].a :
                (j == 1) ? sky_box.polygon[i].b :
                sky_box.polygon[i].c;

            glTexCoord2f(sky_box.mapcoord[index].u, sky_box.mapcoord[index].v);
            glVertex3f(sky_box.vertex[index].x, sky_box.vertex[index].y, sky_box.vertex[index].z);
        }
    }
    glEnd();
    
    
    //球體
    std::vector<vertex_type> sphere_vertices;
    std::vector<mapcoord_type> sphere_mapcoords;
    std::vector<polygon_type> sphere_polygons;

    // 每5個 float：x, y, z, u, v
    for (size_t i = 0; i < vertices.size(); i += 5) {
        vertex_type v;
        v.x = vertices[i];
        v.y = vertices[i + 1];
        v.z = vertices[i + 2];
        sphere_vertices.push_back(v);

        mapcoord_type uv;
        uv.u = vertices[i + 3];
        uv.v = vertices[i + 4];
        sphere_mapcoords.push_back(uv);
    }

    // 每3個 index 為一個三角形
    for (size_t i = 0; i < indices.size(); i += 3) {
        polygon_type p;
        p.a = indices[i];
        p.b = indices[i + 1];
        p.c = indices[i + 2];
        sphere_polygons.push_back(p);
    }

    // 設定名字
    strncpy(sphere.name, "sphere", sizeof(sphere.name) - 1);
    sphere.name[sizeof(sphere.name) - 1] = '\0';

    // 設定頂點與多邊形數量
    sphere.vertices_qty = static_cast<int>(sphere_vertices.size());
    sphere.polygons_qty = static_cast<int>(sphere_polygons.size());

    // 複製頂點資料，確保不超過 MAX_VERTICES
    int vertex_count = (sphere.vertices_qty < MAX_VERTICES)? sphere.vertices_qty : MAX_VERTICES;
    for (int i = 0; i < vertex_count; i++) {
        sphere.vertex[i] = sphere_vertices[i];
    }

    // 複製多邊形資料，確保不超過 MAX_POLYGONS
    int polygon_count = (sphere.polygons_qty  < MAX_POLYGONS)? sphere.polygons_qty : MAX_POLYGONS;
    for (int i = 0; i < polygon_count; i++) {
        sphere.polygon[i] = sphere_polygons[i];
    }

    // 複製貼圖座標資料，確保不超過 MAX_VERTICES
    for (int i = 0; i < vertex_count; i++) {
        sphere.mapcoord[i] = sphere_mapcoords[i];
    }

    //畫隕石，障礙(共7個)
    for (const auto& s : sphereList) {
        glPushMatrix();
        glTranslatef(s.x, s.y, s.z);
        glBindTexture(GL_TEXTURE_2D, sphere.id_texture8);
        drawSphere();
        glPopMatrix();
    }
    
    
    //星星
    glDisable(GL_TEXTURE_2D);
    for (const auto& star : stars) {
        if (!star.collected) {
            glPushMatrix();
            glTranslatef(star.x, star.y, star.z);
            DrawStar3D();
            glPopMatrix();
        }
    }
    glEnable(GL_TEXTURE_2D);


    //分數和計時
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float timeLeft = 30.0f - (now - startTime);
    if (timeLeft < 0.0f) timeLeft = 0.0f;

    // 計分邏輯（每前進 30 單位加 1 分）
    float dz = lastPosZ - posZ;
    if (dz >= 30.0f) {
        score += (int)(dz / 30.0f);
        lastPosZ = posZ;
    }

    // 設定 2D 投影座標
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screen_width, 0, screen_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D); // 禁用貼圖
    glColor3f(1.0f, 1.0f, 0.0f); // 金黃色
    selectFont(20, ANSI_CHARSET, "Consolas");

    // 顯示倒數計時
    char buffer[64];
    sprintf(buffer, "Time: %.1f", timeLeft);
    glRasterPos2i(screen_width - 180, screen_height - 30);
    drawString(buffer);

    // 顯示分數
    sprintf(buffer, "Score: %d", score);
    glRasterPos2i(screen_width - 180, screen_height - 60);
    drawString(buffer);


    // 顯示生命值
    sprintf(buffer, "Life: %d", life);
    glRasterPos2i(screen_width - 180, screen_height - 90);
    drawString(buffer);

    // 顯示鍵盤作用
    glRasterPos2i(screen_width - 180, screen_height - 120);
    drawString("w: front, s: back");
    glRasterPos2i(screen_width - 180, screen_height - 150);
    drawString("a: right, d: left");
    glRasterPos2i(screen_width - 180, screen_height - 180);
    drawString("z: up, x :down");


    glEnable(GL_TEXTURE_2D); // 恢復貼圖

    glPopMatrix(); // 還原模型矩陣
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    //遊戲結束判斷
    if (!gameOver) {
        // 判斷星星是否被吃到
        for (auto& star : stars) {
            if (!star.collected) {
                float d2 = distance2(posX, posY, posZ, star.x, star.y, star.z);
                if (d2 < 100.0f * 100.0f) { // 接觸判定
                    star.collected = true;
                    score += 20;
                }
            }
        }
        
        // 時間結束
        if (life == 0.0f) {
            gameOver = true;
        }


        // 時間結束
        if (timeLeft <= 0.0f) {
            gameOver = true;
        }
    }

    if (gameOver) {
        glutIdleFunc(NULL); //遊戲結束時，時間可以暫停，因為不更新畫面

        // 確保在 Game Over 畫文字之前，重新設置投影與模型矩陣
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screen_width, 0, screen_height);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 0.0f, 0.0f);
        selectFont(50, ANSI_CHARSET, "Comic Sans MS");  // 確保你有選字體

        glRasterPos2i(screen_width / 2 - 50, screen_height / 2);
        drawString("GAME OVER");

        sprintf(buffer, "Final Score: %d", score);
        glRasterPos2i(screen_width / 2 - 70, screen_height / 2 - 40);
        drawString(buffer);

        glEnable(GL_TEXTURE_2D);
        glPopMatrix(); // 模型矩陣
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

    }


    glFlush(); // This force the execution of OpenGL commands
    glutSwapBuffers(); // In double buffered mode we invert the positions of the visible buffer and the writing buffer
}

void mykeyboard(unsigned char key, int x, int y)
{
    if (gameOver) return; // 結束後不再處理按鍵
    //撞到後可以後退
    float nextX = posX;
    float nextY = posY;
    float nextZ = posZ;

    switch (key)
    {
    case 's': {
        nextZ += 5.0f; 
        break;
    }
    case 'w': {
        if (nextZ >= -3500)
            nextZ -= 5.0f; 
        break;
    }
    case 'a':
    {
        if(nextX >= -600)
            nextX -= 5.0f;
        break;
    }
    case 'd':
    {
        if (nextX <= 600)
            nextX += 5.0f;
        break;
    }
    case 'z':
    {
        if (nextY <= 500)
            nextY += 5.0f; 
        break;
    }
    case 'x':
    {
        if (nextY >= -500)
            nextY -= 5.0f; 
        break;
    }
    case 'k': pos_test += 5.0f; break;
    }

    if (key == 'w' || key == 's' || key == 'a' || key == 'd' || key == 'z' || key == 'x')
    {
        rx2 = nextX;
        ry2 = nextY;
        rz2 = nextZ - 300.0f;

        bool willCollide = false;
        const Sphere* collidedSphere = nullptr; // 假設你有 sphereList 裡的 Sphere 結構

        for (const auto& sphere : sphereList) {
            if (checkRocketHitBall(sphere.x, sphere.y, sphere.z, sphere.radius)) {
                willCollide = true;
                collidedSphere = &sphere;
                break;
            }
        }

        if (!willCollide) {
            posX = nextX;
            posY = nextY;
            posZ = nextZ;
            justCollided = false;  // 沒碰撞時，重置碰撞旗標
        }
        else {
            if (collidedSphere != nullptr)
            {
                if (!justCollided)
                {
                    life--;
                }
                float dx = rx2 - collidedSphere->x;
                if (dx < 0) {
                    posX -= 2.0f;
                }
                else {
                    posX += 2.0f;
                }
                justCollided = true;  // 只扣一次
            }
        }
    }

    glutPostRedisplay();
}

void idle() {
    glutPostRedisplay();  // 叫 display() 重新繪製畫面
}



/**********************************************************
 *
 * The main routine
 *
 *********************************************************/

int main(int argc, char** argv)
{
    lastPosZ = posZ;

    // We use the GLUT utility to initialize the window, to handle the input and to interact with the windows system
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width, screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("www.spacesimulator.net - 3d engine tutorials: Tutorial 4");
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(mykeyboard);
    glutIdleFunc(idle);

    init();
    glutMainLoop();

    return(0);
}
