/*
 *
 *  Key bindings:
 *  l          Toggles lighting
 *  m          Toggles light movement
 *  >/<        Mannually move the light position
 *  ;/'        Increase/decrease light distance
 *  [/]         Lower/raise light
 *  w          First person move forward
 *  a          First person move left
 *  s          First person move right
 *  d          First person move backwards
 *  q          Look up
 *  z          Look down
 *  +/-        Change field of view of perspective
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view
 *  1          Scene 1: View of the whole scene
 *  2          Scene 2: Close up of the bezier curve corner
 *  3          Scene 3: Close up of cube map sphere
 *  ESC        Exit
 *
 */

#include "CSCIx229.h"
#include "tga.h"

int n = 32;        //Number of slices for bezier patch

int axes=1;       //  Display axes
int mode=1;       //  Projection mode
int texMode=1;
int presetScene=1;// Scene to display
int move=1;       //  Move light
int th=35;         //  Azimuth of view angle
int ph=20;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
int light=1;      //  Lighting
int segments=4;   //  Number of balls in scene
double asp=1;     //  Aspect ratio
double dim=3;     //  Size of world
int reverse=0;    //  Reverse idle direction

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

unsigned int texture[1];

//Camera controls
float cameraX=1.4;  
float cameraY=0;  
float cameraZ=9;
float camxRotation=0;
float camyRotation=0;


typedef struct {float x,y,z;} Point;

//  Texture coordinates
static struct {float r,s;} tex[2][2] =
{
   {{1,1},{0,1}},
   {{1,0},{0,0}}
};

//Access array for cylinder data
static int Cylinder[][4][4] = 
{
   {{  12,  13,  14,  15},{   8,   9, 10, 11},{  4,  5,  6,  7},{  0,  1,  2,  3}},
   {{  28,  29,  30,  31},{  24,  25, 26, 27},{ 20, 21, 22, 23},{ 16, 17, 18, 19}},
};

//Curved cylindrical connector
static Point data[] = 
{
   //Top half
   { 0      , 0      ,  0},
   { 0      , 1      ,  0},
   { 0      , 2      ,  0},
   { 2      , 2      ,  0},
   { -0.6667, 0      ,  0},
   { -0.6667, 1      ,  0},
   { -0.6667, 2      ,  0},
   { 2      , 2.6667 ,  0},
   { -0.6667, 0      ,  1},
   { -0.6667, 1      ,  1},
   { -0.6667, 2      ,  1},
   { 2      , 2.6667 ,  1},
   { 0      , 0      ,  1},
   { 0      , 1      ,  1},
   { 0      , 2      ,  1},
   { 2      , 2      ,  1},

   //Bottom half
   { 0      , 0      ,  0},
   { 0.6667 , 0      ,  0},
   { 0.6667 , 0      ,  1},
   { 0      , 0      ,  1},
   { 0      , 1      ,  0},
   { 0.6667 , 1      ,  0},
   { 0.6667 , 1      ,  1},
   { 0      , 1      ,  1},
   { 0      , 2      ,  0},
   { 0.6667 , 1.5    ,  0},
   { 0.6667 , 2      ,  1},
   { 0      , 2      ,  1},
   { 2      , 2      ,  0},
   { 2      , 1.3333 ,  0},
   { 2      , 1.3333 ,  1},
   { 2      , 2      ,  1},
};

//Use the TGA image loader to apply textures to specified face of the cube map
void loadCubeMapSide(GLenum sideTarget, char *filename)
{
   FILE *file;
   gliGenericImage *image;

   file = fopen(filename, "rb");
   //Read in the file using TGA image loader
   image = gliReadTGA(file, filename);
   fclose(file);

   //copy image data into the target face of the cube
   glTexImage2D(sideTarget, 0, image->components, image->width, image->height, 0,
      image->format, GL_UNSIGNED_BYTE, image->pixels);
}

//Generate a cube map
//Texture images from Emil Persson (http://www.humus.name/index.php?page=Textures)
//Free to share under creative commons license
void generateCubeMap(void)
{
   //Map the texture to each face of the cubemap
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "posx.tga");
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "negx.tga");
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "posy.tga");
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "negy.tga");
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "posz.tga");
   loadCubeMapSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "negz.tga");

   //Not using minmap, so we set texparam to GL_LINEAR for both minify and magnify
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
   glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);

   //Set wrapping parameters
   //GL_CLAMP_TO_EDGE forces opengl to always return the edge value when sampling
   //between faces of the cubemap
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

   //Enable texture coordination generation to calculate texture coords automatically
   glEnable(GL_TEXTURE_GEN_S);
   glEnable(GL_TEXTURE_GEN_T);
   glEnable(GL_TEXTURE_GEN_R);
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
r = cylinder ring radius
c = cylinder tube radius
rSeg, cSeg = number of segments/detail

Created by: Ng Guoyou
https://gist.github.com/gyng/8939105
*/
void drawTorus(double x, double y, double z,
                  double rotDeg, double rotX, double rotY, double rotZ)
{
   int rSeg = 16;
   int cSeg = 40;
   double r = 0.0325;
   double c = 0.125;

   glPushMatrix();

   glFrontFace(GL_CW);

   glTranslated(x, y, z);
   glRotated(rotDeg, rotX, rotY, rotZ);

   if(texMode)
   {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, texture[0]);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  
   }
   else
      glEnable(GL_TEXTURE_CUBE_MAP);

   const double PI = 3.1415926535897932384626433832795;
   const double TAU = 2*PI;

   for (int i = 0; i < rSeg; i++) 
   {
      glBegin(GL_QUAD_STRIP);
      for (int j = 0; j <= cSeg; j++) 
      {
         for (int k = 0; k <= 1; k++) 
         {
            double s = (i + k) % rSeg + 0.5;
            double t = j % (cSeg + 1);

            double x = (c + r * cos(s * TAU / rSeg)) * cos(t * TAU / cSeg);
            double y = (c + r * cos(s * TAU / rSeg)) * sin(t * TAU / cSeg);
            double z = r * sin(s * TAU / rSeg);

            double u = (i + k) / (float) rSeg;
            double v = t / (float) cSeg;

            glTexCoord2d(u, v);
            glNormal3f(2 * x, 2 * y, 2 * z);
            glVertex3d(2 * x, 2 * y, 2 * z);
         }
      }
      glEnd();
   }

   glPopMatrix();

   glFrontFace(GL_CCW);

   texMode ? glDisable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_CUBE_MAP);
}

//Draw the cross beams and base bars
// Z0 is the height
// (x, y, z) coordinate position
// (dx, dy, dz) size
// (th, rt, ry, rz) rotation
void drawBar(float height, float x, float y, float z,
               double rotA, double rt, double ry, double rz)
{
   int angle;
   float radius = 0.25;
   float increment;

   glPushMatrix();
   glTranslatef(x,y,z);
   glRotated(rotA, rt, ry, rz);

   if(texMode)
   {
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

      // Use linear filtering when magnifying image
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // Use linear blend of two mipmaps when minifying the image
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glGenerateMipmap(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D,texture[0]);
   }
   else
      glEnable(GL_TEXTURE_CUBE_MAP);

   glBegin(GL_QUAD_STRIP);

   for (angle=0;angle<=360;angle+=5)
   {
      increment = angle/360.0;
      glNormal3f(Cos(angle),Sin(angle),0);
      glTexCoord2f(0, increment);
      glVertex3f(radius*Cos(angle),radius*Sin(angle),0);
      glTexCoord2f(1, increment);
      glVertex3f(radius*Cos(angle),radius*Sin(angle),height);
   }

   glEnd();

   glPopMatrix();

   texMode ? glDisable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_CUBE_MAP);
}

//Generate a bezier patch of the specified data points
static void Patch(int patch[4][4],float Sx,float Sy , int type,int mesh,int norm ,
   double x, double y, double z, double rotAngle, double rotX, double rotY, double rotZ, double dx, double dy, double dz)
{
   int j,k;
   Point p[4][4];

   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glRotated(rotAngle, rotX, rotY, rotZ);
   glScaled(dx,dy,dz);

   //  Enable 3D vertexes and 2D textures
   texMode ? glEnable(GL_TEXTURE_2D) : glEnable(GL_TEXTURE_CUBE_MAP);

   glEnable(GL_MAP2_VERTEX_3);
   glEnable(GL_MAP2_TEXTURE_COORD_2);
   //  Evaluate on n x n grid
   glMapGrid2f(n,0,1,n,0,1);
   //  Texture coordinates
   glMap2f(GL_MAP2_TEXTURE_COORD_2, 0,1,2,2, 0,1,4,2,(void*)tex);

   //  Copy data with reflection
   for (k=0;k<4;k++)
   {
      int K = Sx*Sy<0 ? 3-k : k;
      for (j=0;j<4;j++)
      {
         int l = patch[j][K];
         p[j][k].x = Sx*data[l].x;
         p[j][k].y = Sy*data[l].y;
         p[j][k].z = data[l].z;
      }
   }

   //  Set color
   glColor3f(1,1,1);

   //  Evaluate using mesh
   glMap2f(GL_MAP2_VERTEX_3,0,1,3,4, 0,1,12,4,(void*)p);
   glEvalMesh2(type,0,n,0,n);

   texMode ? glDisable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_CUBE_MAP);
   glPopMatrix();
}

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
static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   const int inc=1;
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

void drawSwingSegment(double dx)
{
   glPushMatrix();

   glTranslated(2*dx,0,0);

   int height = 4;
   float x=-height/2, y=4, z=2;

   glEnable(GL_TEXTURE_CUBE_MAP);

   //Rotate the texture when the world rotates
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(th,0.0,1.0,0.0);
   glRotatef(ph,-1.0,0.0,0.0);
   glMatrixMode(GL_MODELVIEW);

   ball(0,0,0,1);

   glDisable(GL_TEXTURE_CUBE_MAP);

   //Reverse rotate the texture when the world rotates
   glMatrixMode(GL_TEXTURE);
   glLoadIdentity();
   glRotatef(th,0.0,-1.0,0.0);
   glRotatef(ph,1.0,0.0,0.0);
   glMatrixMode(GL_MODELVIEW);


   drawBar(height, x, y, z, 90, 0,1,0);
   drawBar(height, x, y, -z, 90, 0,1,0);

   drawTorus(0,4,2, 90,0,1,0);
   drawTorus(0,4,-2, 90,0,1,0);
   glColor3f(0.4, 0.4, 0.4);
   glBegin(GL_LINE_STRIP);
      glVertex3d(0,1,0);
      glVertex3d(0,y,z);
   glEnd();

   glBegin(GL_LINE_STRIP);
      glVertex3d(0,1,0);
      glVertex3d(0,y,-z);
   glEnd();

   glColor3f(1,1,1);

   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   int i;
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   //Load the view camera
   camera();
   //  Perspective - set eye position
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);

   //  OpenGL should generate normals
   glEnable(GL_AUTO_NORMAL);
   glEnable(GL_NORMALIZE);

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
               distance=25;
               Position[0] = distance*Cos(zh);
               Position[1] = 4.4;
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

   for(i=0; i<segments; i++)
   {
      drawSwingSegment(i);
   }

   //Top bars
   Patch(Cylinder[0],+1,+1 , GL_FILL,1,0, -3, 3, 1.75, 0, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],+1,+1 , GL_FILL,1,0, -3, 3, 1.75, 0, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],+1,+1 , GL_FILL,1,0, -3, 3, -2.25, 0, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],+1,+1 , GL_FILL,1,0, -3, 3, -2.25, 0, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],+1,+1 , GL_FILL,1,0, (segments*2+1), 3, 2.25, 180, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],+1,+1 , GL_FILL,1,0, (segments*2+1), 3, 2.25, 180, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],+1,+1 , GL_FILL,1,0, (segments*2+1), 3, -1.75, 180, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],+1,+1 , GL_FILL,1,0, (segments*2+1), 3, -1.75, 180, 0, 1, 0, 0.5, 0.5, 0.5);

   //Bottom
   Patch(Cylinder[0],-1,-1 , GL_FILL,1,0, -2.75, -1, 2, -90, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],-1,-1 , GL_FILL,1,0, -2.75, -1, 2, -90, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],-1,-1 , GL_FILL,1,0, -3.25, -1, -2, 90, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],-1,-1 , GL_FILL,1,0, -3.25, -1, -2, 90, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],-1,-1 , GL_FILL,1,0, (segments*2+1.25), -1, 2, -90, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],-1,-1 , GL_FILL,1,0, (segments*2+1.25), -1, 2, -90, 0, 1, 0, 0.5, 0.5, 0.5);

   Patch(Cylinder[0],-1,-1 , GL_FILL,1,0, (segments*2+0.75), -1, -2, 90, 0, 1, 0, 0.5, 0.5, 0.5);
   Patch(Cylinder[1],-1,-1 , GL_FILL,1,0, (segments*2+0.75), -1, -2, 90, 0, 1, 0, 0.5, 0.5, 0.5);

   int height = 4;
   //Draw vertical bars
   drawBar(height, -3, 3, 2, 90, 1,0,0);
   drawBar(height, -3, 3, -2, 90, 1,0,0);
   drawBar(height, segments*2+1, 3, 2, 90, 1,0,0);
   drawBar(height, segments*2+1, 3, -2, 90, 1,0,0);

   //Draw horizontal bases
   drawBar(height/2, -3, -2, -1, 0,0,0,0);
   drawBar(height/2, segments*2+1, -2,-1, 0,0,0,0);

   //  No lighting from here on
   glDisable(GL_LIGHTING);

   // //  Draw axes (white)
   // glColor3f(1,1,1);
   // double len = 2.0;
   // if (axes)
   // {
   //    glBegin(GL_LINES);
   //    glVertex3d(0.0,0.0,0.0);
   //    glVertex3d(len,0.0,0.0);
   //    glVertex3d(0.0,0.0,0.0);
   //    glVertex3d(0.0,len,0.0);
   //    glVertex3d(0.0,0.0,0.0);
   //    glVertex3d(0.0,0.0,len);
   //    glEnd();
   //    //  Label axes
   //    glRasterPos3d(len,0.0,0.0);
   //    Print("X");
   //    glRasterPos3d(0.0,len,0.0);
   //    Print("Y");
   //    glRasterPos3d(0.0,0.0,len);
   //    Print("Z");
   // }

   // //  Display parameters
   // glWindowPos2i(5,5);
   // glColor3f(1,1,1);
   // Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s zh:%d",
   //   th,ph,dim,fov,mode?"Perpective":"Orthogonal",light?"On":"Off",zh);
   // if (light)
   // {
   //    glWindowPos2i(5,45);
   //    Print("camxRot=%0.1f camyRot=%0.1f Distance=%d Elevation=%.1f",camxRotation,camyRotation,distance,ylight);
   //    glWindowPos2i(5,25);
   //    Print("camX=%0.3f  camY=%0.3f camZ=%0.3f Emission=%d Shininess=%.0f",cameraX,cameraY,cameraZ,emission,shiny);
   // }

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

   if(presetScene == 3)
   {
      if(reverse)
         th -= 1;
      else
         th += 1;
      if(th == -25)
         reverse = 0;
      if(th == 215)
         reverse = 1;
   }

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
      th = 40;
      ph = 25;
      dim = 10;
      light = 1;
      cameraX = cameraY = cameraZ = camxRotation = camyRotation=0;
   }
   // Toggle through the preset scenes
   else if (ch == '1')
   {
      presetScene = 1;
      light = 1;
      th = 35;
      ph = 20;
      dim = 3;
      cameraX = 1.4;
      cameraY = 0;
      cameraZ = 9;
      camxRotation = camyRotation=0;
   }
   else if (ch == '2')
   {
      presetScene = 2;
      light = 1;
      th = -155;
      ph = 40;
      dim = 3;
      cameraX = 3.5;
      cameraY = 1.011;
      cameraZ = -1;
      camxRotation = -30;
      camyRotation=0;
   }
   else if (ch == '3')
   {
      presetScene = 3;
      light = 0;
      th = -25;
      ph = 10;
      dim = 3;
      cameraX = cameraY = camxRotation = camyRotation=0;
      cameraZ = -3;
   }
   //  Toggle cubemap textures
   else if (ch == 'r' || ch == 'R')
   {
      texMode++;
      texMode = texMode%2;
   }
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
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
   else if (ch=='y')
   {
      segments++;
      if(segments > 5)
      {
         segments = 3;
      }
   }
   else if (ch=='t')
   {
      segments--;
      if(segments < 3)
      {
         segments = 5;
      }
   }
   //First person camera controls
   else if (ch == 'w')
   {
      cameraX += Sin(camyRotation);
      cameraZ -= Cos(camyRotation);
      cameraY -= Sin(camxRotation);
   }
   else if (ch == 's')
   {
      cameraX -= Sin(camyRotation);
      cameraZ += Cos(camyRotation);
      cameraY += Sin(camxRotation);
   }
   else if (ch == 'd')
   {
      cameraX += Cos(camyRotation) * 0.7;
      cameraZ += Sin(camyRotation) * 0.7;
   }
   else if (ch == 'a')
   {
      cameraX -= Cos(camyRotation) * 0.7;
      cameraZ -= Sin(camyRotation) * 0.7;
   }
   //Look down
   else if (ch == 'q')
   {
      camxRotation -= 3;
      if (camxRotation < -360) 
         camxRotation += 360;
   }
   //Look up
   else if (ch == 'z')
   {
      camxRotation += 3;
      if (camxRotation > 360)
         camxRotation -= 360;
   }
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(fov,asp,dim);
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
   // Project(mode?fov:0,asp,dim);
   Project(fov,asp,dim);
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
   glutCreateWindow("Final - Kevin Holligan");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);

   //Make the cube map
   generateCubeMap();

   //  Load textures
   texture[0] = LoadTexBMP("aluminum.bmp");

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}


