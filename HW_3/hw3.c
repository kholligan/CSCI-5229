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

int th=-220;         //  Azimuth of view angle
int ph=30;         //  Elevation of view angle
double dim=3.0;   //  Size of world
double asp=1;     //  Aspect ratio

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
   //  Perspective transformation
   //  Orthogonal projection
   glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

// Draw legs (skinny 3D rectangles)
// Take inputs to scale and translate legs
static void legs(double x, double y, double z,
                 double dx, double dy, double dz,
                 double legHeight)
{
   const double depth=0.05;
   const double height=-legHeight;

   glPushMatrix();

   glTranslated(x,y,z);
   glTranslated(depth,-height,depth);
   glScaled(dx,dy,dz);

   //Legs
   glColor3f(1,0,1);

   glBegin(GL_QUADS);
   //Front
   glVertex3d(depth,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(depth,-height,depth);
   
   //Back
   glColor3f(1,0,0);
   glVertex3d(depth,height,-depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(depth,-height,-depth);  
   
   //Side
   glColor3f(0,1,0);
   glVertex3d(depth,height,depth);
   glVertex3d(depth,-height,depth);
   glVertex3d(depth,-height,-depth);
   glVertex3d(depth,height,-depth);  
   
   //Side
   glColor3f(0,0,1);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(-depth,height,-depth);

   //Base
   glColor3f(0,1,1);
   glVertex3d(depth,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(depth,height,-depth);
   
   //Top
   glColor3f(1,1,0);
   glVertex3d(depth,-height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(depth,-height,-depth);

   glEnd();
   glPopMatrix();
}

// Draw the seat back
// Inputs to scale and translate the seat back
// Inputs to determine seat back position relative to other elements
static void seatBack(double x,double y,double z,
                  double dx,double dy,double dz,
                  double legSpacing, double legHeight, double seatHeight)
{
   
   const double width=legSpacing;
   const double depth=0.1;
   const double height=-(legHeight*1.10);

   glPushMatrix();

   glTranslated(x,y,z);
   glTranslated(depth,legHeight+legHeight*2+seatHeight,depth);
   glScaled(dx,dy,dz);
   
   
   //Seat
   glColor3f(1,0,1);
   glBegin(GL_QUADS);
   //Front
   glVertex3d(width,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(width,-height,depth);
   
   //Back
   glColor3f(1,0,0);
   glVertex3d(width,height,-depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(width,-height,-depth);  
   
   //Side
   glColor3f(0,1,0);
   glVertex3d(width,height,depth);
   glVertex3d(width,-height,depth);
   glVertex3d(width,-height,-depth);
   glVertex3d(width,height,-depth);  
   
   //Side
   glColor3f(0,0,1);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(-depth,height,-depth);

   //Base
   glColor3f(0,1,1);
   glVertex3d(width,height,depth);
   glVertex3d(-depth,height,depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(width,height,-depth);
   
   //Top
   glColor3f(1,1,0);
   glVertex3d(width,-height,depth);
   glVertex3d(-depth,-height,depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(width,-height,-depth);
   glEnd();

   glPopMatrix();
}

// Draw the seat of the chair
// Take inputs to determine how tall and wide the seat is, plus where to place
// it in relation to the chair legs
static void seat(double x,double y,double z,
                  double dx,double dy,double dz,
                  double legSpacing, double legHeight, double seatHeight)
{
   const double width=legSpacing;
   const double depth=0.1;
   const double height=seatHeight;

   glPushMatrix();

   glTranslated(x,y,z);
   glTranslated(depth,legHeight*2,depth);
   glScaled(dx,dy,dz);
   

   //Legs
   glColor3f(1,0,1);

   glBegin(GL_QUADS);
   //Front
   glVertex3d(width,height,width);
   glVertex3d(-depth,height,width);
   glVertex3d(-depth,-height,width);
   glVertex3d(width,-height,width);
   
   //Back
   glColor3f(1,0,0);
   glVertex3d(width,height,-depth);
   glVertex3d(-depth,height,-depth);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(width,-height,-depth);  
   
   //Side
   glColor3f(0,1,0);
   glVertex3d(width,height,width);
   glVertex3d(width,-height,width);
   glVertex3d(width,-height,-depth);
   glVertex3d(width,height,-depth);  
   
   //Side
   glColor3f(0,0,1);
   glVertex3d(-depth,height,width);
   glVertex3d(-depth,-height,width);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(-depth,height,-depth);

   //Base
   glColor3f(0,1,1);
   glVertex3d(width,height,width);
   glVertex3d(-depth,height,width);
   glVertex3d(-depth,height,-depth);
   glVertex3d(width,height,-depth);
   
   //Top
   glColor3f(1,1,0);
   glVertex3d(width,-height,width);
   glVertex3d(-depth,-height,width);
   glVertex3d(-depth,-height,-depth);
   glVertex3d(width,-height,-depth);

   glEnd();
   glPopMatrix();
}


// Draw a chair
// Take inputs to scale, rotate, and translate table
// Take inputs to determine how tall and wide the table is
static void chair(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th, double ux, double uy, double uz,
                  double legSpacing, double legHeight, double seatHeight)
{
   glPushMatrix();
   glTranslated(x,y,z);
   glRotated(th,ux,uy,uz);
   glScaled(dx,dy,dz);

   //Draw four legs
   legs(0,0,0,1,1,1,legHeight);
   legs(legSpacing,0,0,1,1,1,legHeight);
   legs(legSpacing,0,legSpacing,1,1,1,legHeight);
   legs(0,0,legSpacing,1,1,1,legHeight);

   //Draw seat
   seat(0,0,0,1,1,1,legSpacing,legHeight, seatHeight);

   //Draw seat back
   seatBack(0,0,0,1,1,1,legSpacing,legHeight, seatHeight);

   glPopMatrix();   
}

// Draw a table
// Take inputs to scale, rotate, and translate table
// Take inputs to determine how tall and wide the table is
static void table(double x,double y,double z,
                  double dx,double dy,double dz,
                  double th,
                  double legSpacing, double legHeight, double seatHeight)
{
   glPushMatrix();
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Draw four legs
   legs(0,0,0,1,1,1,legHeight);
   legs(legSpacing,0,0,1,1,1,legHeight);
   legs(legSpacing,0,legSpacing,1,1,1,legHeight);
   legs(0,0,legSpacing,1,1,1,legHeight);

   //Draw table top
   seat(0,0,0,1,1,1,legSpacing,legHeight, seatHeight);

   glPopMatrix(); 
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
   //  Set view angle
   glRotatef(ph,1,0,0);
   glRotatef(th,0,1,0);

   //Draw table and four chairs
   table( 0,0,0, 0.7,0.7,0.7, 0, 1.25, 0.5, 0.05);
   chair( -0.2,0,0.7, 
          0.5,0.5,0.5, 
          90, 0, 1, 0,
          0.8, 0.5, 0.05);
   chair( 0.2,0,0, 
          0.5,0.5,0.5, 
          -90,1,0,0,
          0.8,0.5,0.05);
   chair( 1.9,0,0.2, 
          0.7,0.7,0.7, 
          -90,0,1,0,
          0.8, 0.5, 0.05);
   chair( 0.2,0,1.5, 
          0.5,0.5,0.5, 
          90,1,1,0,
          0.8, 0.5, 0.05);


   glColor3f(1,1,1);
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5,5);
   //  Print the text string
   Print("Angle=%d,%d Dim=%.1f",th,ph,dim);
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
      th = ph = 0;
   //  Change field of view angle
   else if (ch == 'a')
      dim-=0.1;
   else if (ch == 'd')
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
   glutCreateWindow("hw3 - Kevin Holligan");
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
