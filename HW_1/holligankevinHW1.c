/*
* Kevin Holligan
* CSCI 5229
* HW2 Lorenz Attractor
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//  Globals
int th=0;         // Azimuth of view angle
int ph=0;         // Elevation of view angle
double dim=125;   // Perspective dimensions
double s=10;      // Lorenz parameter
double b=2.6666;  // Lorenz parameter
double r=28;      // Lorenz parameter
double colorR = 0.5; //Default color value
double colorG = 0.2; //Default color value
double colorB = 0.7; //Default color value
int lorenzPoints = 50000;  //Number of points to generate in Lorenz attractor

//Method for passing 3D coords between functions
struct pointCoordinates3D {
   double xCoord; 
   double yCoord; 
   double zCoord;
};

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  // Maximum length of text string
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

/* 
 *  Generate a single Lorenz attractor point
 */
struct pointCoordinates3D generateLorenz(double x, double y, double z)
{

   struct pointCoordinates3D returnPoints;

   /*  Time step  */
   double dt = 0.001;

   //Generate the lorenz attractor point   
   double dx = s*(y-x);
   double dy = x*(r-z)-y;
   double dz = x*y - b*z;
   //Assign values to struct
   returnPoints.xCoord = x + dt*dx;
   returnPoints.yCoord = y + dt*dy;
   returnPoints.zCoord = z + dt*dz;

   return returnPoints;
}

//Generate a random color
void colorGenerator(double *Red, double *Green, double *Blue)
{
   //Values are between 0 and 1 (i.e. RAND_MAX)
   *Red = (double)rand() / (double)RAND_MAX;
   *Green = (double)rand() / (double)RAND_MAX;
   *Blue = (double)rand() / (double)RAND_MAX;
}

/*
 *  Display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // Save transformation
   glPushMatrix();
   //  Reset previous transforms
   glLoadIdentity();
   //  Set view angle
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);

   //Draw lorenz
   //create struct for the points, and initialize
   struct pointCoordinates3D drawPoints;
   drawPoints.xCoord = 1;
   drawPoints.yCoord = 1;
   drawPoints.zCoord = 1;

   //Set color to generated colors
   glColor3f(colorR, colorG, colorB);
   //GL_LINE_STRIP for drawing lines
   glBegin (GL_LINE_STRIP);

   //Generate a number of Lorenz attractor points and draw them
   int i;
   for (i=0;i<lorenzPoints;i++)
   {
      drawPoints = generateLorenz(drawPoints.xCoord, drawPoints.yCoord, 
         drawPoints.zCoord);
      glVertex3d(drawPoints.xCoord, drawPoints.yCoord, drawPoints.zCoord);
   }
   glEnd();

   // White text
   glColor3f(1,1,1);
   //  Display parameters
   glWindowPos2i(5,5);
   //Output text to see variable values
   Print("View Angle=%d,%d s=%.0f, b=%.4f, r=%.0f, Points=%d, Color=(%.2f,%.2f,%.2f)",
      th,ph,s, b, r, lorenzPoints, colorR,colorG,colorB);

   //  Flush and swap
   glFlush();
   glutSwapBuffers();

   glPopMatrix();
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
      s=10;
      b=2.6666;
      r=28;
      colorR = 0.5;
      colorG = 0.2;
      colorB = 0.7;
      lorenzPoints = 50000; 
   } 
   //Create a new color
   else if (ch == 'c')
      colorGenerator(&colorR, &colorG, &colorB);
   //Increment s value
   else if (ch == 's')
      s+=1;
   //Decrement s value
   else if (ch == 'S')
      s-=1;
   //Increment b value
   else if (ch == 'b')
      b+=0.5;
   //Decrement b value
   else if (ch == 'B')
      b-=0.5;
   //Increment r value
   else if (ch == 'r')
      r+=2;
   //Decrement r value
   else if (ch == 'R')
      r-=2;
   //Increment number of points
   else if (ch == 'p')
      lorenzPoints+=1000;
   //Decrement number of points
   else if (ch == 'P')
      lorenzPoints-=1000;

   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease azimuth by 5 degrees
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
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   // GLfloat h = (GLfloat) height / (GLfloat) width;
   //  Ratio of the width to the height of the window
   double w2h = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, (GLint) width, (GLint) height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   glOrtho(-dim*w2h,+dim*w2h, -dim,+dim, -dim,+dim);

   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -60.0);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
  //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window 
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(500,500);
   //  Create the window
   glutCreateWindow("HW2 - Kevin Holligan");
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
   //  Return code
   return 0;
}
