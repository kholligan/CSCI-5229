#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

int th=90;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
double dim=15.0;   //  Size of world
double asp=1;     //  Aspect ratio
int mode = 1;
int fov = 55;
char perspectiveMode[20] = "Orthogonal";
float cameraX=0;
float cameraY=0;
float cameraZ=0;
float cameraXRotation=0;
float cameraYRotation=0;

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.1415927/180))
#define Sin(x) (sin((x)*3.1415927/180))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   switch(mode)
   {
      case 1:
         //  Orthogonal projection
         glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
         break;
      case 2:
         //  Perspective transformation
         gluPerspective(fov,asp,dim/4,4*dim);
         break;
   }
   
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}


static void drawSphere(double r, int lats, int longs) {
   int i, j;
   for(i = 0; i <= lats; i++) 
   {
      double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
      double z0  = sin(lat0);
      double zr0 =  cos(lat0);

      double lat1 = M_PI * (-0.5 + (double) i / lats);
      double z1 = sin(lat1);
      double zr1 = cos(lat1);

      glBegin(GL_QUAD_STRIP);
      for(j = 0; j <= longs; j++) 
      {

         double lng = 2 * M_PI * (double) (j - 1) / longs;
         glColor3f(cos(lng),0.5, sin(lng));
         double x = cos(lng);
         double y = sin(lng);

         glVertex3f(x * zr0, y * zr0, z0);
         glVertex3f(x * zr1, y * zr1, z1);
      }
      glEnd();
   }
}

static void drawCylinder(float radius, float height, float slices, float stacks)
{
   double majorStep = height / stacks;
   double minorStep = 2.0 * M_PI / slices;
   int i, j;

   for (i = 0; i < stacks; ++i) 
   {
      float z0 = 0.5 * (height - i) * majorStep;
      float z1 = z0 - majorStep;

      glBegin(GL_TRIANGLE_STRIP); 
      for (j = 0; j <= slices; ++j) 
      {
         double a = j * minorStep;
         float x = radius * cos(a);
         float y = radius * sin(a);
         glVertex3f(x, y, z0);
         glVertex3f(x, y, z1);
      }
      glEnd();
   }
}

static void drawBar(double x, double y, double z,
                 double dx, double dy, double dz,
                 double barLength)
{
   const double depth=0.25;
   const double height=-barLength;

   glPushMatrix();

   glRotated(90,0,0,1);
   glTranslated(x,y,z);
   glTranslated(depth,-height,depth);
   glScaled(dx,dy,dz);

   glColor3f(0.5f,0.35f,0.05f);

   glBegin(GL_QUADS);
   //Front
   glVertex3d(depth,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(depth,-height,depth);
   
   //Back
   glVertex3d(depth,height,-depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(depth,-height,-depth);  
   
   //Side
   glVertex3d(depth,height,depth);
   glVertex3d(depth,-height,depth);
   glVertex3d(depth,-height,-depth);
   glVertex3d(depth,height,-depth);  
   
   //Side
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(-depth,height,-depth);

   //Base
   glVertex3d(depth,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(depth,height,-depth);
   
   //Top
   glVertex3d(depth,-height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(depth,-height,-depth);

   glEnd();
   glPopMatrix();
}

static void drawSwing(double r, int lats, int longs,
                     float radius, float height, float slices, float stacks)
{
   glPushMatrix();

   drawSphere(r, 100, 100);


   glRotatef(-70.0f,1.0f,0.0f,0.0f);
   glTranslatef(0, 0.0, 0.0);
   glColor3f(0.9f, 0.9f, 0.8f);
   drawCylinder(0.05f, 15, 15.0f, 15.0f);


   glRotatef(-40.0f,1.0f,0.0f,0.0f);
   glTranslatef(0, 0.0, 0.0);
   glColor3f(0.9f, 0.9f, 0.8f);
   drawCylinder(0.05f, 15, 15.0f, 15.0f);

   glPopMatrix();           
}

//Function to initialize and control the camera
void camera()
{
   //Rotate the camera around x-axis (lef and right movement)
   glRotatef(cameraXRotation,1,0,0);
   //Rotate the camera around the y-axis (up and down movement)
   glRotatef(cameraYRotation,0,1,0);
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
   //  Enable smooth shading
   glShadeModel(GL_SMOOTH);

   //  Undo previous transformations
   glLoadIdentity();
   camera();

   // Set the eye position of the perspective transformation
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);

   //Switch between perspective and orthogonal mode
   switch(mode)
   {
      case 1:
         glRotatef(ph,1,0,0);
         glRotatef(th,0,1,0);
         break;
      case 2:
         gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
         break;
   }
   drawBar(7,-13.5,2.35,1,1,1,6.5);
   drawBar(7,-13.5,-2.75,1,1,1,6.5);

   int radius=1;
   int i;
   for(i=0; i<6; i++)
   {
      glTranslated(2,0,0);
      drawSwing(radius, 0, 0, 0, 0, 0, 0);
   }

   glColor3f(1,1,1);
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5,5);
   //  Print the text string
   Print("Angle=%d,%d Dim=%.1f Mode: %s Cam:%.1fX,%.1fZ, %1.fXRot, %.1fYRot",
      th,ph,dim,perspectiveMode,cameraX,cameraZ, cameraXRotation, cameraYRotation);
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}



/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;

   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

// Camera movement is adapted from the swiftless.com tutorial on cameras
// To move forwards, backwards, or look around we are adding and subtracting
// our current vector from our position vector
// The positive z-axis is towards the user, would subtract from it to move forward
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   else if (ch == '1')
   {
      mode=1;
      sprintf(perspectiveMode, "Orthogonal");
   }
   else if (ch == '2')
   {
      mode=2;
      sprintf(perspectiveMode, "Perspective");
   }
   //Look down
   else if (ch == 'q')
   {
      cameraXRotation += 1;
      if (cameraXRotation >360) 
         cameraXRotation -= 360;
   }
   //Look up
   else if (ch == 'z')
   {
      cameraXRotation -= 1;
      if (cameraXRotation < -360)
         cameraXRotation += 360;
   }
   //Look right
   else if (ch == 'e')
   {
      cameraYRotation += 1;
      if (cameraYRotation >360) 
         cameraYRotation -= 360;
   }
   //Look left
   else if (ch == 'c')
   {
      cameraYRotation -= 1;
      if (cameraYRotation >360) 
         cameraYRotation += 360;
   }
   //Move forward
   else if (ch == 'w')
   {
      float xrotrad, yrotrad;
      yrotrad = (cameraYRotation / 180 * M_PI);
      xrotrad = (cameraXRotation / 180 * M_PI); 
      cameraX += (float)sin(yrotrad);
      cameraZ -= (float)cos(yrotrad);
      cameraY -= (float)sin(xrotrad);
   }
   //Move backwards
   else if (ch == 's')
   {
      float xrotrad, yrotrad;
      yrotrad = (cameraYRotation / 180 * M_PI);
      xrotrad = (cameraXRotation / 180 * M_PI); 
      cameraX -= (float)sin(yrotrad);
      cameraZ += (float)cos(yrotrad);
      cameraY += (float)sin(xrotrad);
   }
   //Strafe lefts
   else if (ch == 'd')
   {
      float yrotrad;
      yrotrad = (cameraYRotation / 180 * M_PI);
      cameraX += (float)cos(yrotrad) * 0.7;
      cameraZ += (float)sin(yrotrad) * 0.7;
   }
   //Strafe right
   else if (ch == 'a')
   {
      float yrotrad;
      yrotrad = (cameraYRotation / 180 * M_PI);
      cameraX -= (float)cos(yrotrad) * 0.7;
      cameraZ -= (float)sin(yrotrad) * 0.7;
   }
   //  Change field of view angle
   else if (ch == 'o')
      dim-=0.1;
   else if (ch == 'p')
      dim+=0.1;

   Project();

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
   Project();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("hw4 - Kevin Holligan");
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
