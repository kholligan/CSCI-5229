/*
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  m          Toggles light movement
 *  >/<        Mannually move the light position
 *  ;/'        Increase/decrease light distance
 *  [/]         Lower/rise light
 *  p          Toggles ortogonal/perspective projection
 *  w          First person move forward
 *  a          First person move left
 *  s          First person move right
 *  d          First person move backwards
 *  +/-        Change field of view of perspective
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  1          Scene 1: Close up of curved bars
 *  2          Scene 2: Close up of "strings"/"wires"
 *  3          Scene 3: Full view of object
 *  ESC        Exit
 */
#include "CSCIx229.h"

int axes=1;       //  Display axes
int mode=1;       //  Projection mode
int presetScene=1; // Scene to display
int move=1;       //  Move light
int th=-105;         //  Azimuth of view angle
int ph=25;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
int light=1;      //  Lighting
double asp=1;     //  Aspect ratio
double dim=9.6;   //  Size of world
// Light values
int one       =   1;  // Unit value
int distance  =   10;  // Light distance
int inc       =  1;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   5;  // Elevation of light
unsigned int texture[3];

float cameraX=0;  
float cameraY=0;  
float cameraZ=0;
float camxRotation=0;
float camyRotation=0;


/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimentions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th, double modify)
{
   double tex1, tex2, tex3, tex4;
   //Reverse the direction of the texture
   if(modify)
   {
      tex1 = 1;
      tex2 = 0;
      tex3 = 1;
      tex4 = 0;
   }
   else
   {
      tex1 = 0;
      tex2 = 1;
      tex3 = 0;
      tex4 = 1;
   }
   //  Set specular color to white
   float white[] = {1,1,1,1};
   float black[] = {0,0,0,1};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[0]);

   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f( 0, 0, 1);
   glTexCoord2f(tex1,tex3); glVertex3f(-1,-1, 1);
   glTexCoord2f(tex2,tex3); glVertex3f(+1,-1, 1);
   glTexCoord2f(tex2,tex4); glVertex3f(+1,+1, 1);
   glTexCoord2f(tex1,tex4); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f( 0, 0,-1);
   glTexCoord2f(tex1,tex3); glVertex3f(+1,-1,-1);
   glTexCoord2f(tex2,tex3); glVertex3f(-1,-1,-1);
   glTexCoord2f(tex2,tex4); glVertex3f(-1,+1,-1);
   glTexCoord2f(tex1,tex4); glVertex3f(+1,+1,-1);
   //  Right
   //Flip the left/right tex coord so it runs same way as top/bottom
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(+1,-1,-1); 
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   //  Left
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(-1,+1,-1);
   //  Top
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   //  Bottom
   glNormal3f( 0,-1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   //  End
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

static void drawRoundedEdge(float radius, float height, float slices, float stacks,
                           float cx, float cy, float cz, float quadX, float quadY)
{
   double heightInc = height / stacks;
   double pointInc = 0.5f * M_PI / slices; //0.5pi to draw quarters
   int i, j;
   float z0 = cz; //Adjust for z position

   glPushMatrix();

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[0]);

   //Front cap (in line with face of rectangle)
   glBegin(GL_TRIANGLE_STRIP);
   for(j=0; j<=slices; j++)
   {
      double a = j * pointInc;
      float x = cx + radius * (quadX*cos(a)); //Define quadrant for corner
      float y = cy + radius * (quadY*sin(a)); //Define quadrant for corner
      glNormal3f(0,0,1); //Front facing corner has vector along z-axis
      glTexCoord2f(0, 0); 
      glVertex3f(cx,cy,z0);
      //Draw the texture in a quarter circle
      glTexCoord2f(cos(a), sin(a));
      glVertex3f(x,y,z0);
   }
   glEnd();

   //Back cap (in line with rear of rectangle)
   glBegin(GL_TRIANGLE_STRIP);
   z0 = -cz;
   for(j=0; j<=slices; j++)
   {
      double a = j * pointInc;
      float x = cx + radius * (quadX*cos(a)); //Define quadrant for corner
      float y = cy + radius * (quadY*sin(a)); //Define quadrant for corner
      glNormal3f(0,0,-1); //Rear face has normal along -z-axis
      glTexCoord2f(0,0);
      glVertex3f(cx,cy,z0);
      glTexCoord2f(cos(a), sin(a));
      glVertex3f(x,y,z0);
   }
   glEnd();

   //Curved side
   //Draw cylinder 
   glBegin(GL_TRIANGLE_STRIP);
   for (i = 0; i < stacks; i++) 
   {
      z0 = 0.5 * height - (i * heightInc);
      float z1 = z0 - heightInc;

      for (j = 0; j <= slices; ++j) 
      {
         double a = j * pointInc;
         float x = cx + radius * (quadX*cos(a));
         float y = cy + radius * (quadY*sin(a));
         //Normal to a cylinder is (cos(theta), sin(theta),0) but we adjust
         //to our positional and quadrant coordinates
         //Tex coordinates are slices of the texture map along the outside of
         //cylinder and then moving up the next slice in the stack
         glNormal3f(x / radius, y / radius, 0.0);
         glTexCoord2f(j / slices, i / stacks);
         glVertex3f(x, y, z0);
         glTexCoord2f(j / slices, (i + 1) / stacks);
         glVertex3f(x, y, z1);
      }
      
   }
   glEnd();

   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

static void drawRoundedCube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   float radius = 0.5;
   float stacks=30;
   float slices=30;
   float height=1.5;

   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(th,0,0,1);
   glScaled(dx,dy,dz);

   // glColor3f(0.5f, 0.35f, 0.05f);
   cube(0,0,0, 0.75,0.75,0.75, 0, 0);  //Center
   cube(-1,0,0, 0.25,0.25,0.75, 0,1); //Left
   cube(1,0,0, 0.25,0.25,0.75, 0, 0);  //Right

   drawRoundedEdge(radius, height, stacks, slices, 0.75, 0.25, 0.75, 1, 1);
   drawRoundedEdge(radius, height, stacks, slices, -0.75, 0.25, 0.75, -1, 1);
   drawRoundedEdge(radius, height, stacks, slices, -0.75, -0.25, 0.75, -1, -1);
   drawRoundedEdge(radius, height, stacks, slices, 0.75, -0.25, 0.75, 1, -1);

   glPopMatrix();

}
/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   double x = -Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glTexCoord2d(th/360.0,ph/180.0+0.5);
   glVertex3d(x,y,z);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void textureBall(double x,double y,double z,double r)
{
   int th,ph;

   //Textures from http://devernay.free.fr/cours/opengl/materials.html
   float ambient[] = {0.135,0.2225,0.1575,1.0};
   float diffuse[] = {0.54,0.89,0.63,1.0};
   float specular[] = {0.316228,0.316228,0.316228,1.0};
   float shine=0.1;
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(-90,1,0,0);
   glScaled(r,r,r);

   glMaterialf(GL_FRONT,GL_SHININESS,shine * 128.0);
   glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
   glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
   glMaterialfv(GL_FRONT,GL_SPECULAR,specular);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[2]);

   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=5)
      {
         Vertex(th,ph);
         Vertex(th,ph+5);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

static void drawCylinder(float radius, float height, float slices, float stacks, float z)
{
   double heightInc = height / stacks;
   double pointInc = 2.0 * M_PI / slices;
   int i, j;

   glPushMatrix();
   glTranslated(0,0,z);

   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   glBindTexture(GL_TEXTURE_2D,texture[1]);

   for (i = 0; i < stacks; i++) 
   {
      float z0 = 0.5f * height - (i * heightInc);
      // float z0 = heightInc;
      float z1 = z0 - heightInc;

      glBegin(GL_TRIANGLE_STRIP); 
      for (j = 0; j <= slices; j++) 
      {
         double a = j * pointInc;
         float x = radius * cos(a);
         float y = radius * sin(a);
         //Normal to a cylinder is (cos(theta), sin(theta),0) but we adjust
         //to our positional and quadrant coordinates
         glNormal3f(x / radius, y / radius, 0.0);
         glTexCoord2f(j / slices, i / stacks);
         glVertex3f(x, y, z0);
         glTexCoord2f(j / slices, (i + 1) / stacks);
         glVertex3f(x, y, z1);
      }
      glEnd();
   }
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

static void drawBar(double x, double y, double z,
                 double dx, double dy, double dz,
                 double barLength)
{
   glPushMatrix();
   glTranslated(x,y,z);
   glRotated(90,0,1,0);
   glScaled(dx,dy,dz);
   //Draw the bar as multiple instances of rounded cubes to increase number of vertices
   for(int i=0; i<barLength; i++)
   {
      drawRoundedCube(0,0,(1.5*i),1,1,1,0);
   }
   glPopMatrix();
}

static void drawString(float n)
{
   // float radius, float height, float slices, float stacks

   for(float i=0; i<n; i++)
   {
      drawCylinder(0.05f, 1, 30.0f, 30.0f,1*i);
   }
}

static void drawSwing()
{
   glPushMatrix();

   //Draw the ball
   textureBall(0,0,0,1);


   glColor3f(1,1,1);
   //Draw one of the string/wire connectors
   glRotatef(-70.0f,1.0f,0.0f,0.0f);
   // glTranslatef(0, 0.0, 3.75);
   drawString(7.5);

   //Draw the other string/wire connector
   glRotatef(-40.0f,1.0f,0.0f,0.0f);
   // glTranslatef(0, 2.5, 1);
   drawString(7.5);

   glPopMatrix();           
}

//Function to initialize and control the camera
void camera()
{
   //Rotate the camera around x-axis (lef and right movement)
   glRotatef(camxRotation,1,0,0);
   //Rotate the camera around the y-axis (up and down movement)
   glRotatef(camyRotation,0,1,0);
   // Move the camera in the coordinate plane 
   glTranslated(-cameraX,-cameraY,-cameraZ);
}
/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   camera();
   //  Perspective - set eye position
   if (mode)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Light switch
   if (light)
   {
        //  Translate intensity to color vectors
        float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
        float Position[] = {ylight, distance*Cos(zh), distance*Sin(zh), 1.0};
        //Change lighting for scenes
        switch(presetScene)
        {
            case 1:
               break;
            case 2:
               Position[0] = 10+distance*Cos(zh);
               Position[1] = ylight;
               Position[2] = distance*Sin(zh);
               break;
            case 3:
               Position[0] = 10+distance*Cos(zh);
               Position[1] = ylight;
               Position[2] = distance*Sin(zh);
               break;
        }
        
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2] , 0.1);
        //  OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        //  Enable lighting
        glEnable(GL_LIGHTING);
        //  Location of viewer for specular calculations
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
        //  glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
   glDisable(GL_LIGHTING);

   // Draw scene
   glColor3f(1,1,1);
   // glColor3f(0.5f, 0.35f, 0.05f); //Brown
   drawBar(1,7,2.55,0.2,0.25,1,9);
   drawBar(1,7,-2.55,0.2,0.25,1,9);

   //Draw 5 ball pieces
   int i;
   for(i=0; i<6; i++)
   {
      // glColor3f(0.137255,0.556863,0.137255);
      glTranslated(2,0,0);
      drawSwing();
   }

   //  No lighting from here on
   glDisable(GL_LIGHTING);

   //  Display parameters
   glWindowPos2i(5,5);
   glColor3f(1,1,1);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s zh:%d",
     th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off",zh);
   if (light)
   {
      glWindowPos2i(5,45);
      Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
      glWindowPos2i(5,25);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
   }

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 1;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 1;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 1;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 1;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(mode?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
   {
      th = ph = 0;
      cameraX = cameraY = cameraZ = camxRotation = camyRotation=0;
   }
   // Toggle through the preset scenes
   else if (ch == '1')
   {
      presetScene = 1;
      th = -105;
      ph = 25;
      dim = 9.6;
      cameraX = cameraY = cameraZ = camxRotation = camyRotation=0;
   }
   else if (ch == '2')
   {
      presetScene = 2;
      th = -85;
      ph = 15;
      dim = 6.6;
      cameraX = cameraY = cameraZ = camxRotation = camyRotation=0;
   }
   else if (ch == '3')
   {
      presetScene = 3;
      th = -70;
      ph = 20;
      dim = 14.3;
      cameraX = cameraY = cameraZ = camxRotation = camyRotation=0;
   }
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P')
      mode = 1-mode;
   //  Toggle light movement
   else if (ch == 'm' || ch == 'M')
      move = 1-move;
   //  Move light
   else if (ch == '<')
      zh += 5;
   else if (ch == '>')
      zh -= 5;
   else if (ch == ';')
      distance++;
   else if (ch == '\'')
      distance--;
   //  Change field of view angle
   else if (ch == '-' && ch>1)
      fov--;
   else if (ch == '+' && ch<179)
      fov++;
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
      ylight += 0.1;
   else if (ch == 'w')
   {
      float camxRotationrad, camyRotationrad;
      camyRotationrad = (camyRotation / 180 * M_PI);
      camxRotationrad = (camxRotation / 180 * M_PI); 
      cameraX += (float)sin(camyRotationrad);
      cameraZ -= (float)cos(camyRotationrad);
      cameraY -= (float)sin(camxRotationrad);
   }
   else if (ch == 's')
   {
      float camxRotationrad, camyRotationrad;
      camyRotationrad = (camyRotation / 180 * M_PI);
      camxRotationrad = (camxRotation / 180 * M_PI); 
      cameraX -= (float)sin(camyRotationrad);
      cameraZ += (float)cos(camyRotationrad);
      cameraY += (float)sin(camxRotationrad);
   }
   else if (ch == 'd')
   {
      float camyRotationrad;
      camyRotationrad = (camyRotation / 180 * M_PI);
      cameraX += (float)cos(camyRotationrad) * 0.7;
      cameraZ += (float)sin(camyRotationrad) * 0.7;
   }
   else if (ch == 'a')
   {
      float camyRotationrad;
      camyRotationrad = (camyRotation / 180 * M_PI);
      cameraX -= (float)cos(camyRotationrad) * 0.7;
      cameraZ -= (float)sin(camyRotationrad) * 0.7;
   }
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(mode?fov:0,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(mode?fov:0,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(400,400);
   glutCreateWindow("hw6 - Kevin Holligan");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);

   //  Load textures
   texture[0] = LoadTexBMP("wood.bmp");
   texture[1] = LoadTexBMP("seamless_metal.bmp");
   texture[2] = LoadTexBMP("silver.bmp");
   // texture[1] = LoadTexBMP("crate.bmp");
   // texture[2] = LoadTexBMP("water.bmp");

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
