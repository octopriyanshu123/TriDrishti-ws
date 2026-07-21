#include <GL/glut.h>
#include <string>
#include <cstdio>

using namespace std;

class Panel
{
public:

    int windowW = 1280;
    int windowH = 800;

    string radiusText = "10";
    string heightText = "20";

    bool radiusActive = false;
    bool heightActive = false;
    bool applyPressed = false;

    //----------------------------------------------------------
    void drawString(int x,int y,const string& s)
    {
        glRasterPos2i(x,y);

        for(char c : s)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);
    }

    //----------------------------------------------------------
    void drawTextBox(
        int x,
        int y,
        int w,
        int h,
        const string& label,
        const string& value,
        bool active)
    {
        drawString(x,y-10,label);

        if(active)
            glColor3f(0.95f,0.95f,1.0f);
        else
            glColor3f(1,1,1);

        glBegin(GL_QUADS);

        glVertex2i(x,y);
        glVertex2i(x+w,y);
        glVertex2i(x+w,y+h);
        glVertex2i(x,y+h);

        glEnd();

        glColor3f(0,0,0);

        glBegin(GL_LINE_LOOP);

        glVertex2i(x,y);
        glVertex2i(x+w,y);
        glVertex2i(x+w,y+h);
        glVertex2i(x,y+h);

        glEnd();

        drawString(x+6,y+22,value);
    }

    //----------------------------------------------------------
    void drawButton()
    {
        glColor3f(0.15f,0.45f,0.85f);

        glBegin(GL_QUADS);

        glVertex2i(20,190);
        glVertex2i(180,190);
        glVertex2i(180,230);
        glVertex2i(20,230);

        glEnd();

        glColor3f(1,1,1);

        drawString(70,215,"Apply"); 
    }

    //----------------------------------------------------------
    void draw()
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);


        glMatrixMode(GL_PROJECTION);

        glPushMatrix();

        glLoadIdentity();

        gluOrtho2D(
            0,
            windowW,
            windowH,
            0);


        glMatrixMode(GL_MODELVIEW);

        glPushMatrix();

        glLoadIdentity();


        glColor3f(0.12f,0.13f,0.16f);

        glBegin(GL_QUADS);

        glVertex2i(0,0);
        glVertex2i(220,0);
        glVertex2i(220,windowH);
        glVertex2i(0,windowH);

        glEnd();

        //------------------------------------------------------

        glColor3f(1,1,1);

        drawString(20,35,"Tank Editor");

        //------------------------------------------------------

        drawTextBox(
            20,
            70,
            180,
            35,
            "Radius",
            radiusText,
            radiusActive);

        drawTextBox(
            20,
            130,
            180,
            35,
            "Height",
            heightText,
            heightActive);

        drawButton();

        //------------------------------------------------------

        glPopMatrix();

        glMatrixMode(GL_PROJECTION);

        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);

        glPopAttrib();
    }

    //----------------------------------------------------------
    bool mouse(int button,int state,int x,int y)
    {
        if(button!=GLUT_LEFT_BUTTON)
return false;
        if(state!=GLUT_DOWN)
return false;
        radiusActive=false;
        heightActive=false;

        //---------------- Radius ----------------

        if(x>=20 && x<=200 &&
           y>=70 && y<=105)
        {
            radiusActive=true;
        }

        //---------------- Height ----------------

        if(x>=20 && x<=200 &&
           y>=130 && y<=165)
        {
            heightActive=true;
        }

        //---------------- Button ----------------

        if(x>=20 && x<=180 &&
           y>=190 && y<=230)
        {
            printf("\n----- Apply -----\n");
            printf("Radius : %s\n",radiusText.c_str());
            printf("Height : %s\n",heightText.c_str());

            return true;
        }

        glutPostRedisplay();
        return false;
    }

    //----------------------------------------------------------
    void keyboard(unsigned char key)
    {
        string *text=nullptr;

        if(radiusActive)
            text=&radiusText;

        if(heightActive)
            text=&heightText;

        if(text==nullptr)
            return;

        if(key==8)
        {
            if(!text->empty())
                text->pop_back();
        }
        else if((key>='0' && key<='9') || key=='.')
        {
            text->push_back(key);
        }

        glutPostRedisplay();
    }

    //----------------------------------------------------------
    void reshape(int w,int h)
    {
        windowW=w;
        windowH=h;
    }
};

Panel panel;