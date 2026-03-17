#include <GL/glut.h>
#include<stdlib.h>
#include<math.h>

#define pi 3.1415926

GLint angle_upper = 0, angle_lower = 0, angle_base = 0, move_x = 0, angle_wheels = 0;

void drawWheels(int x, int y, int z) //畫輪胎，可輸入位置
{
    GLUquadricObj* quadricPtr;
    quadricPtr = gluNewQuadric();
    gluQuadricDrawStyle(quadricPtr, GLU_FILL);
    gluQuadricTexture(quadricPtr, GL_FALSE);
    glRotatef(-90.0, 0.0, 1.0, 0.0);

    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0); // black
    glTranslatef(x, y, z);
    glShadeModel(GLU_SMOOTH);
    gluQuadricNormals(quadricPtr, GLU_NONE);
    gluCylinder(quadricPtr, 30.0, 30.0, 20.0, 20.0, 10.0);
    glPopMatrix();


    // 內部圓盤(底座)
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0); // black
    gluQuadricDrawStyle(quadricPtr, GLU_FILL);
    glTranslatef(x, y, z); // 圓柱底部
    gluDisk(quadricPtr, 0.0, 30.0, 20.0, 1.0);
    glPopMatrix();


    // 外部圓盤(線條)
    glPushMatrix();
    glColor3f(0.5, 0.5, 0.5); //gray
    gluQuadricDrawStyle(quadricPtr, GLU_LINE);
    glTranslatef(x, y, z + 20); // 圓柱頂部
    gluDisk(quadricPtr, 0.0, 30.0, 10.0, 1.0);
    glPopMatrix();

}

void drawQuarterCylinder(float radius, float height, int slices) { // 1/4圓柱體 (駕駛座+挖土的鏟子)，圓柱半徑、高度、切割數量
    float angleStep = (pi / 2) / slices; // 每個切片的角度（90度範圍）
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; i++) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        glVertex3f(x, y, 0.0f);  // 底部點
        glVertex3f(x, y, height); // 對應的頂部點
    }
    glEnd();

    // 繪製底部圓
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f); // 圓心
    for (int i = 0; i <= slices; i++) {
        float angle = i * angleStep;
        glVertex3f(radius * cos(angle), radius * sin(angle), 0.0f);
    }
    glEnd();

    // 繪製頂部圓
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, height); // 圓心
    for (int i = 0; i <= slices; i++) {
        float angle = i * angleStep;
        glVertex3f(radius * cos(angle), radius * sin(angle), height);
    }
    glEnd();

    // 繪製長方形
    glBegin(GL_QUADS);
    glColor3f(0.0, 0.0, 0.0); // black
    glVertex3f(0.0f, 0.0f, 0.0f); // 左下
    glVertex3f(0.0f, radius, 0.0f); // 右下
    glVertex3f(0.0f, radius, height); // 右上
    glVertex3f(0.0f, 0.0f, height); // 左上
    glEnd();

}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLUquadricObj* quadricPtr;
    quadricPtr = gluNewQuadric();
    gluQuadricDrawStyle(quadricPtr, GLU_FILL);
    gluQuadricTexture(quadricPtr, GL_FALSE);

    glLoadIdentity();
    gluLookAt(200, 300, 200, 0, 0, 0, 0, 1, 0);

    glPushMatrix(); // 讓整台挖土機沿 X 軸移動
        glTranslatef(move_x, 0.0, 0.0);

        glPushMatrix(); //base 旋轉，但不影響 lower_arm 和 upper_arm 的相對運動
            // 1. 將「中間轉盤」移到原點
            glTranslatef(-40.0, -60.0, 0.0);
            // 2. 進行旋轉
            glRotatef(angle_base, 0.0, 1.0, 0.0);
            // 3. 再將物件移回原來的位置
            glTranslatef(40.0, 60.0, 0.0);

            // 機器部分(上)
            glPushMatrix();
            glColor3f(1.0, 1.0, 0.0);
            glTranslatef(-40.0, -15.0, 0.0);
            glShadeModel(GLU_SMOOTH);
            gluQuadricNormals(quadricPtr, GLU_NONE);
            glScalef(70.0, 50.0, 70.0);
            glutSolidCube(1.0);
            glPopMatrix();

            // 機器部分(上，駕駛座)
            glPushMatrix();
            glColor3f(1.0, 0.75, 0.0); // 
            glTranslatef(-40.0, 10.0, 0.0);
            drawQuarterCylinder(40, 30, 20);
            glPopMatrix();

            // 機器部分(中間轉盤)
            glPushMatrix();
            glColor3f(0.0, 0.0, 0.0); // red
            glTranslatef(-40.0, -60.0, 0.0);
            glRotatef(-90.0, 1.0, 0.0, 0.0);
            glShadeModel(GLU_SMOOTH);
            gluQuadricNormals(quadricPtr, GLU_NONE);
            gluCylinder(quadricPtr, 30.0, 30.0, 20.0, 20.0, 10.0);
            glPopMatrix();


            glPushMatrix(); //lower arm, upper arm, 挖土的 一起旋轉
                glTranslatef(-15.0, 0.0, -15.0); // 設定 lower arm 的 pivot 為尾端
                glRotatef(angle_lower, 0.0, 0.0, 1.0); // 以 Z 軸旋轉 lower arm
                glTranslatef(15.0, 0.0, 15.0); // 移回原位置

                // lower_arm
                glPushMatrix();
                glColor3f(1.0, 0.75, 0.0); // orange
                glTranslatef(-25.0, 50.0, -15.0);
                glShadeModel(GLU_SMOOTH);
                gluQuadricNormals(quadricPtr, GLU_NONE);
                glScalef(20.0, 100.0, 20.0);
                glutSolidCube(1.0);
                glPopMatrix();

                glPushMatrix();//upper arm, 挖土的 一起旋轉
                
                    glTranslatef(-30.0, 86.0, -15.0);
                    glRotatef(angle_upper, 0.0, 0.0, 1.0); // 以 Z 軸旋轉，可依需求調整
                    glTranslatef(30.0, -86.0, 15.0);

                    // upper_arm
                    glPushMatrix();
                    glColor3f(1.0, 1.0, 0.0); // yellow
                    glTranslatef(30.0, 86.0, -15.0);
                    glShadeModel(GLU_SMOOTH);
                    gluQuadricNormals(quadricPtr, GLU_NONE);
                    glScalef(100.0, 19.0, 19.0);
                    glutSolidCube(1.0);
                    glPopMatrix();

                    // 挖土的
                    glPushMatrix();
                    glColor3f(0.0, 0.0, 0.0);
                    glTranslatef(70.0, 75.0, -30.0);
                    drawQuarterCylinder(30, 30, 20);
                    glPopMatrix();

                glPopMatrix(); //upper arm, 挖土的 一起旋轉

            glPopMatrix(); //lower arm, upper arm, 挖土的 一起旋轉

        glPopMatrix(); //base旋轉

        // 機器部分(下)
        glPushMatrix();
        glColor3f(1.0, 0.75, 0.0);
        glTranslatef(-40.0, -80.0, 0.0);
        glShadeModel(GLU_SMOOTH);
        gluQuadricNormals(quadricPtr, GLU_NONE);
        glScalef(100.0, 50.0, 100.0);
        glutSolidCube(1.0);
        glPopMatrix();

        //畫輪子(後面後輪)
        glPushMatrix();
        glTranslatef(-80, -105, -50);  // 先移動到指定位置
        glRotatef(angle_wheels, 0.0, 0.0, 1.0);
        glRotatef(90.0, 0.0, 1.0, 0.0); // 沿 Y 軸旋轉 90 度
        glRotatef(180.0, 0.0, 0.0, 1.0); // 沿 Z 軸旋轉 180 度
        drawWheels(0, 0, 0);  // 原點繪製輪子，確保變換正確
        glPopMatrix();


        //畫輪子(後面前輪)
        glPushMatrix();
        glTranslatef(-10, -105, -50);  // 先移動到指定位置
        glRotatef(angle_wheels, 0.0, 0.0, 1.0);
        glRotatef(90.0, 0.0, 1.0, 0.0);
        glRotatef(180.0, 0.0, 0.0, 1.0);
        drawWheels(0, 0, 0);
        glPopMatrix();


        //畫輪子(前面後輪)
        glPushMatrix();
        glTranslatef(-80, -105, 50);  // 先移動到指定位置
        glRotatef(angle_wheels, 0.0, 0.0, 1.0);
        glRotatef(90.0, 0.0, 1.0, 0.0);
        drawWheels(0, 0, 0);
        glPopMatrix();

        //畫輪子(前面前輪)
        glPushMatrix();
        glTranslatef(-10, -105, 50);  // 先移動到指定位置
        glRotatef(angle_wheels, 0.0, 0.0, 1.0);
        glRotatef(90.0, 0.0, 1.0, 0.0);
        drawWheels(0, 0, 0);
        glPopMatrix();

    glPopMatrix(); // 讓整台挖土機沿 X 軸移動

    glutSwapBuffers();
}
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'r') angle_base += 1.0; //base以Y軸逆時鐘旋轉
    else if (key == 't') angle_base -= 1.0; //base以y軸順時鐘旋轉
    else if (key == 'a') //lower arm 以z軸逆時鐘旋轉
    {
        if(angle_lower <= 45.0) angle_lower += 1.0;
    }
    else if (key == 'd') //lower arm 以z軸順時鐘旋轉
    {
        if (angle_lower >= -50.0) angle_lower -= 1.0;
    }
    else if (key == 's') //upper arm 以z軸逆時鐘旋轉
    {
        if (angle_upper <= 45.0) angle_upper += 1.0;
    }
    else if (key == 'w') //upper arm 以z軸順時鐘旋轉
    {
        if (angle_upper >= -45.0) angle_upper -= 1.0;
    }

    glutPostRedisplay();
}


void SpecialKey(GLint key, GLint x, GLint y)
{
    float wheelRotationSpeed = 5.0;  // 控制輪胎轉動的速度
    float wheelRadius = 30.0; // 輪胎半徑 (與 drawWheels() 的參數一致)

    if (key == GLUT_KEY_LEFT)
    {
        if (move_x >= -150)
        {
            move_x -= 2.0;
            angle_wheels += (2.0 / (2 * pi * wheelRadius)) * 360; // 依移動距離計算輪胎角度
        }
    }
    
    else if (key == GLUT_KEY_RIGHT)
    {
        if (move_x <= 40)
        {
            move_x += 2.0;
            angle_wheels -= (2.0 / (2 * pi * wheelRadius)) * 360; // 依移動距離計算輪胎角度
        }
    }

    glutPostRedisplay();
}


void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 0.0, 0.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Excavator");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(SpecialKey);

    glutMainLoop();
    return 0;
}

