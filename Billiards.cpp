// Billiards.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>     
#include <stdlib.h>    
#include <math.h>      
#include <assert.h>    
#include <GL/glut.h>
#include <Cg/cg.h>    
#include <Cg/cgGL.h>

#include "Cg_Matrix_Operations.h"
#define myPi  3.14159265358979323846
static CGcontext   myCgContext;
static CGprofile   myCgVertexProfile,
                   myCgFragmentProfile;
static CGprogram   myCgVertexProgram,
                   myCgFragmentProgram;
static CGparameter myCgVertexParam_modelViewProj,
                   myCgFragmentParam_globalAmbient,
                   myCgFragmentParam_lightColor,
                   myCgFragmentParam_lightPosition,
                   myCgFragmentParam_eyePosition,
                   myCgFragmentParam_Ke,
                   myCgFragmentParam_Ka,
                   myCgFragmentParam_Kd,
                   myCgFragmentParam_Ks,
                   myCgFragmentParam_shininess;

static const char *myProgramName = "Billiards",
                  *myVertexProgramFileName = "FragmentLightening.cg",
				  *myVertexProgramName = "FragmentLightening",
                  *myFragmentProgramFileName = "PhongsLightening.cg",
				  *myFragmentProgramName = "PhongsLightening";

static float myLightAngle = -0.4;   
static float myProjectionMatrix[16];
static float myGlobalAmbient[3] = { 0.1, 0.1, 0.1 };  
static float myLightColor[3] = { 0.95, 0.95, 0.95 };  


int angle=-90;
int stickangle;
float power = 3; 
struct ball
{
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float xspeed;
	float zspeed;
	int flag;
} w,r,b;
static int animating = 0;


int enablestick=1;
int zerospeed=1;
float friction = 0.01;
static float camera[3]={0,200,400};
static void checkForCgError(const char *situation)
{
  CGerror error;
  const char *string = cgGetLastErrorString(&error);

  if (error != CG_NO_ERROR) {
    printf("%s: %s: %s\n",
      myProgramName, situation, string);
    if (error == CG_COMPILER_ERROR) {
      printf("%s\n", cgGetLastListing(myCgContext));
    }
    exit(1);
  }
}

/* Forward declared GLUT callbacks registered by main. */
static void reshape(int width, int height);
static void display(void);
static void keyboard(unsigned char c, int x, int y);
static void menu(int item);
static void mouse(int button, int state, int x, int y);

int main(int argc, char **argv)
{
  glutInitWindowSize(800, 800);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInit(&argc, argv);

  glutCreateWindow(myProgramName);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);

  glClearColor(0.1, 0.1, 0.1, 0);  /* Gray background. */
  glEnable(GL_DEPTH_TEST);         /* Hidden surface removal. */

  myCgContext = cgCreateContext();
  checkForCgError("creating context");
  cgGLSetDebugMode( CG_FALSE );

  myCgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
  cgGLSetOptimalOptions(myCgVertexProfile);
  checkForCgError("selecting vertex profile");

  myCgVertexProgram =
    cgCreateProgramFromFile(
      myCgContext,              /* Cg runtime context */
      CG_SOURCE,                /* Program in human-readable form */
      myVertexProgramFileName,  /* Name of file containing program */
      myCgVertexProfile,        /* Profile: OpenGL ARB vertex program */
      myVertexProgramName,      /* Entry function name */
      NULL);                    /* No extra commyPiler options */
  checkForCgError("creating vertex program from file");
  cgGLLoadProgram(myCgVertexProgram);
  checkForCgError("loading vertex program");

#define GET_VERTEX_PARAM(name) \
  myCgVertexParam_##name = \
    cgGetNamedParameter(myCgVertexProgram, #name); \
  checkForCgError("could not get " #name " parameter");

  GET_VERTEX_PARAM(modelViewProj);

  myCgFragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
  cgGLSetOptimalOptions(myCgFragmentProfile);
  checkForCgError("selecting fragment profile");

  myCgFragmentProgram =
    cgCreateProgramFromFile(
      myCgContext,              /* Cg runtime context */
      CG_SOURCE,                /* Program in human-readable form */
      myFragmentProgramFileName,
      myCgFragmentProfile,      /* Profile: latest fragment profile */
      myFragmentProgramName,    /* Entry function name */
      NULL); /* No extra commyPiler options */
  checkForCgError("creating fragment program from string");
  cgGLLoadProgram(myCgFragmentProgram);
  checkForCgError("loading fragment program");

#define GET_FRAGMENT_PARAM(name) \
  myCgFragmentParam_##name = \
    cgGetNamedParameter(myCgFragmentProgram, #name); \
  checkForCgError("could not get " #name " parameter");

  GET_FRAGMENT_PARAM(globalAmbient);
  GET_FRAGMENT_PARAM(lightColor);
  GET_FRAGMENT_PARAM(lightPosition);
  GET_FRAGMENT_PARAM(eyePosition);
  GET_FRAGMENT_PARAM(Ke);
  GET_FRAGMENT_PARAM(Ka);
  GET_FRAGMENT_PARAM(Kd);
  GET_FRAGMENT_PARAM(Ks);
  GET_FRAGMENT_PARAM(shininess);

  /* Set light source color parameters once. */
  cgSetParameter3fv(myCgFragmentParam_globalAmbient, myGlobalAmbient);
  cgSetParameter3fv(myCgFragmentParam_lightColor, myLightColor);
  
  stickangle=90+angle;

  w.xspeed=0 ;
  w.zspeed=0;


  w.x=0;w.y=4;w.z=30;
  w.r=1;w.g=1;w.b=1;

  
  r.x=-34;r.y=4;r.z=-80;
  r.r=1;r.g=0;r.b=0;
  
  b.x=26;b.y=4;b.z=-40;
  b.r=0;b.g=0;b.b=0;
  
  	w.flag=1;
	r.flag=1;
	b.flag=1;
	w.xspeed=0;
    w.zspeed=0;
	r.zspeed=0;
	r.xspeed=0;
	b.xspeed=0;
	b.zspeed=0;
    enablestick=1;

	printf("Billiards! By :\n");
	printf("Prabhat Kumar Gupta\t 2007MCS2895\n");
	printf("Nidhi Arora\t\t 2007MCS2913\t\n");
	printf("-----------------------------------------------\n");
	printf("\t\tControls\n");
	printf("-----------------------------------------------\n");
	printf("Control:\t\tAction\n");
	printf("-----------------------------------------------\n");
	printf("LeftClick/Spacebar\tEnable Stick and Strike\n");
	printf("z\t\t\tRotate Stick Left\n");
	printf("x\t\t\tRotate Stick Right\n");
	printf("n\t\t\tZoom In\n");
	printf("m\t\t\tZoom Out\n");
	printf("1\t\t\tCamera 1\n");
	printf("2\t\t\tCamera 2\n");
	printf("3\t\t\tCamera 3\n");
	printf("4\t\t\tCamera 4\n");
	printf("-----------------------------------------------\n");



  glutCreateMenu(menu);
  glutAddMenuEntry("[ ] Animate", ' ');
  glutAddMenuEntry("[ ] Camera1",'1');
  glutAddMenuEntry("[ ] Camera2",'2');
  glutAddMenuEntry("[ ] Camera3",'3');
  glutAddMenuEntry("[ ] Camera4",'4');
  
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutMainLoop();
  return 0;
}


static void reshape(int width, int height)
{
  double aspectRatio = (float) width / (float) height;
  double fieldOfView = 40.0; /* Degrees */

  /* Build projection matrix once. */
  buildPerspectiveMatrix(fieldOfView, aspectRatio,
                         1.0, 1000.0,  /* Znear and Zfar */
                         myProjectionMatrix);
  glViewport(0, 0, width, height);
}


/* Build a row-major (C-style) 4x4 matrix transform based on the
   parameters for gluPerspective. */
 
static void setBrassMaterial(void)
{
  const float brassEmissive[3] = {0.0,  0.0,  0.0},
              brassAmbient[3]  = {0.33, 0.22, 0.03},
              brassDiffuse[3]  = {0.78, 0.57, 0.11},
              brassSpecular[3] = {0.99, 0.91, 0.81},
              brassShininess = 27.8;

  cgSetParameter3fv(myCgFragmentParam_Ke, brassEmissive);
  cgSetParameter3fv(myCgFragmentParam_Ka, brassAmbient);
  cgSetParameter3fv(myCgFragmentParam_Kd, brassDiffuse);
  cgSetParameter3fv(myCgFragmentParam_Ks, brassSpecular);
  cgSetParameter1f(myCgFragmentParam_shininess, brassShininess);
}

static void setRedPlasticMaterial(void)
{
  const float redPlasticEmissive[3] = {0.0,  0.0,  0.0},
              redPlasticAmbient[3]  = {0.0, 0.0, 0.0},
              redPlasticDiffuse[3]  = {0.5, 0.0, 0.0},
              redPlasticSpecular[3] = {0.7, 0.6, 0.6},
              redPlasticShininess = 32.0;

  cgSetParameter3fv(myCgFragmentParam_Ke, redPlasticEmissive);
  checkForCgError("setting Ke parameter");
  cgSetParameter3fv(myCgFragmentParam_Ka, redPlasticAmbient);
  checkForCgError("setting Ka parameter");
  cgSetParameter3fv(myCgFragmentParam_Kd, redPlasticDiffuse);
  checkForCgError("setting Kd parameter");
  cgSetParameter3fv(myCgFragmentParam_Ks, redPlasticSpecular);
  checkForCgError("setting Ks parameter");
  cgSetParameter1f(myCgFragmentParam_shininess, redPlasticShininess);
  checkForCgError("setting shininess parameter");
}

static void setTableMaterial(void)
{
  const float redPlasticEmissive[3] = {0.0,  0.0,  0.0},
              redPlasticAmbient[3]  = {0.2, 0.5, 0.2},
              redPlasticDiffuse[3]  = {0.0, 0.5, 0.0},
              redPlasticSpecular[3] = {0.0, 0.0, 0.0},
              redPlasticShininess = 0.0;

  cgSetParameter3fv(myCgFragmentParam_Ke, redPlasticEmissive);
  checkForCgError("setting Ke parameter");
  cgSetParameter3fv(myCgFragmentParam_Ka, redPlasticAmbient);
  checkForCgError("setting Ka parameter");
  cgSetParameter3fv(myCgFragmentParam_Kd, redPlasticDiffuse);
  checkForCgError("setting Kd parameter");
  cgSetParameter3fv(myCgFragmentParam_Ks, redPlasticSpecular);
  checkForCgError("setting Ks parameter");
  cgSetParameter1f(myCgFragmentParam_shininess, redPlasticShininess);
  checkForCgError("setting shininess parameter");
}

static void setBallMaterial(float x, float y, float z)
{
  const float redPlasticEmissive[3] = {0.0,  0.0,  0.0},
              redPlasticAmbient[3]  = {0.0, 0.0, 0.0},
              redPlasticDiffuse[3]  = {x, y, z},
              redPlasticSpecular[3] = {0.7, 0.6, 0.6},
              redPlasticShininess = 5.0;

  cgSetParameter3fv(myCgFragmentParam_Ke, redPlasticEmissive);
  checkForCgError("setting Ke parameter");
  cgSetParameter3fv(myCgFragmentParam_Ka, redPlasticAmbient);
  checkForCgError("setting Ka parameter");
  cgSetParameter3fv(myCgFragmentParam_Kd, redPlasticDiffuse);
  checkForCgError("setting Kd parameter");
  cgSetParameter3fv(myCgFragmentParam_Ks, redPlasticSpecular);
  checkForCgError("setting Ks parameter");
  cgSetParameter1f(myCgFragmentParam_shininess, redPlasticShininess);
  checkForCgError("setting shininess parameter");
}
static void setStickMaterial()
{
  const float redPlasticEmissive[3] = {0.0,  0.0,  0.0},
              redPlasticAmbient[3]  = {0.0, 0.0, 0.0},
              redPlasticDiffuse[3]  = {0.9,0.4,0.12},
              redPlasticSpecular[3] = {0,0,0},
              redPlasticShininess = 0;

  cgSetParameter3fv(myCgFragmentParam_Ke, redPlasticEmissive);
  checkForCgError("setting Ke parameter");
  cgSetParameter3fv(myCgFragmentParam_Ka, redPlasticAmbient);
  checkForCgError("setting Ka parameter");
  cgSetParameter3fv(myCgFragmentParam_Kd, redPlasticDiffuse);
  checkForCgError("setting Kd parameter");
  cgSetParameter3fv(myCgFragmentParam_Ks, redPlasticSpecular);
  checkForCgError("setting Ks parameter");
  cgSetParameter1f(myCgFragmentParam_shininess, redPlasticShininess);
  checkForCgError("setting shininess parameter");
}
static void setEmissiveLightColorOnly(void)
{
  const float zero[3] = {0.0,  0.0,  0.0};

  cgSetParameter3fv(myCgFragmentParam_Ke, myLightColor);
  checkForCgError("setting Ke parameter");
  cgSetParameter3fv(myCgFragmentParam_Ka, zero);
  checkForCgError("setting Ka parameter");
  cgSetParameter3fv(myCgFragmentParam_Kd, zero);
  checkForCgError("setting Kd parameter");
  cgSetParameter3fv(myCgFragmentParam_Ks, zero);
  checkForCgError("setting Ks parameter");
  cgSetParameter1f(myCgFragmentParam_shininess, 0);
  checkForCgError("setting shininess parameter");
}
float n[3];

void norm(float v0[], float v1[], float v2[])
{
     double d;
	 float d1[3] = {v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]};
     float d2[3] = {v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]};
     n[0] = d1[1] * d2[2] - d1[2] * d2[1]; 
     n[1] = d1[2] * d2[0] - d1[0] * d2[2]; 
     n[2] = d1[0] * d2[1] - d1[1] * d2[0];
     d = sqrt(n[1] * n[1] + n[2] * n[2] + n[3] * n[3]);
     n[0] = n[0] / d;
     n[1] = n[1] / d;
     n[2] = n[2] / d;
}
void drawtable()
{
	float p[121][2];
	float v1[3] , v2[3], v3[3];
	int i;
	for(i=0; i<=120; i++)
	{
		p[i][0] = 8 * (cos((2 * myPi * i)/120));
		p[i][1] = 8 * (sin((2 * myPi * i)/120));
	}
    glNormal3f(0, 1, 0);

	glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0,0,0);
		for (i=0; i<=30; i++)
		{
			glVertex3f(-93+p[i][0], 0, -183+p[i][1]);
		}

		for (i=90; i<120; i++)
		{
			glVertex3f(-93+p[i][0], 0, p[i][1]);
		}
		for(i=0; i<=30; i++)
		{
			glVertex3f(-93+p[i][0], 0, p[i][1]);
		}

		for (i=90; i<=120; i++)
		{
			glVertex3f(-93+p[i][0], 0, 183+p[i][1]);
		}
		for (i=60; i<=90; i++)
		{
			glVertex3f(93+p[i][0], 0, 183+p[i][1]);
		}

        for(i=30; i<60; i++)
		{
			glVertex3f(93+p[i][0], 0, p[i][1]);
		}
		for(i=60; i<=90; i++)
		{
			glVertex3f(93+p[i][0], 0, p[i][1]);
		}

		for (i=30; i<=60; i++)
		{
			glVertex3f(93+p[i][0], 0, -183+p[i][1]);
		}
		glVertex3f(-93+p[0][0], 0, -183+p[0][1]);
	glEnd();
   glBegin(GL_POLYGON);
		v1[0] =85;		v1[1] = 7;		v1[2] = 167;
		v2[0] = 85;		v2[1] = 7;		v2[2] = 16;
		v3[0] = 93;		v3[1] = 0;		v3[2] = 8;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(85, 7, 167);
		glVertex3f(85, 7, 16);
		glVertex3f(93, 0, 8);
		glVertex3f(93, 0, 175);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(85, 7, 167);
		glVertex3f(93, 7, 175);
		glVertex3f(93, 7, 8);
		glVertex3f(85, 7, 16);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 93;		v1[1] = 0;		v1[2] = 175;
		v2[0] = 93;		v2[1] = 7;		v2[2] = 175;
		v3[0] = 85;		v3[1] = 7;		v3[2] = 167;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(93, 0, 175);
		glVertex3f(93, 7, 175);
        glVertex3f(85, 7, 167);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = 93;		v1[1] = 0;		v1[2] = 8;
		v2[0] = 85;		v2[1] = 7;		v2[2] = 16;
		v3[0] = 93;		v3[1] = 7;		v3[2] = 8;
	    norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(93, 0, 8);
		glVertex3f(85, 7, 16);
		glVertex3f(93, 7, 8);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = 85;		v1[1] = 7;		v1[2] = -167;
		v2[0] = 93;		v2[1] = 0;		v2[2] = -175;
		v3[0] = 93;		v3[1] = 0;		v3[2] = -8;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(85, 7, -167);
		glVertex3f(93, 0, -175);
		glVertex3f(93, 0, -8);
		glVertex3f(85, 7, -16);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(85, 7, -167);
		glVertex3f(85, 7, -16);
		glVertex3f(93, 7, -8);
		glVertex3f(93, 7, -175);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 93;		v1[1] = 0;		v1[2] = -167;
		v2[0] = 85;		v2[1] = 7;		v2[2] = -167;
		v3[0] = 93;		v3[1] = 7;		v3[2] = -175;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(93, 0, -175);
		glVertex3f(85, 7, -167);
		glVertex3f(93, 7, -175);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = 93;		v1[1] = 0;		v1[2] = -8;
		v2[0] = 93;		v2[1] = 7;		v2[2] = -8;
		v3[0] = 85;		v3[1] = 7;		v3[2] = -16;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(93, 0, -8);
		glVertex3f(93, 7, -8);
		glVertex3f(85, 7, -16);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -85;		v1[1] = 7;		v1[2] = 167;
		v2[0] = -93;		v2[1] = 0;		v2[2] = 175;
		v3[0] = -93;		v3[1] = 0;		v3[2] = 8;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-85, 7, 167);
		glVertex3f(-93, 0, 175);
		glVertex3f(-93, 0, 8);
		glVertex3f(-85, 7, 16);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(-85, 7, 167);
		glVertex3f(-85, 7, 16);
		glVertex3f(-93, 7, 8);
		glVertex3f(-93, 7, 175);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = -93;		v1[1] = 0;		v1[2] = 175;
		v2[0] = -85;		v2[1] = 7;		v2[2] = 167;
		v3[0] = -93;		v3[1] = 7;		v3[2] = 175;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-93, 0, 175);
        glVertex3f(-85, 7, 167);
		glVertex3f(-93, 7, 175);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -93;		v1[1] = 0;		v1[2] = 8;
		v2[0] = -93;		v2[1] = 7;		v2[2] = 8;
		v3[0] = -93;		v3[1] = 7;		v3[2] = 16;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-93, 0, 8);
		glVertex3f(-93, 7, 8);
		glVertex3f(-85, 7, 16);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -85;		v1[1] = 7;		v1[2] = -167;
		v2[0] = -85;		v2[1] = 7;		v2[2] = -16;
		v3[0] = -93;		v3[1] = 0;		v3[2] = -8;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-85, 7, -167);
		glVertex3f(-85, 7, -16);
		glVertex3f(-93, 0, -8);
		glVertex3f(-93, 0, -175);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(-85, 7, -167);
		glVertex3f(-93, 7, -175);
		glVertex3f(-93, 7, -8);
		glVertex3f(-85, 7, -16);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = -93;		v1[1] = 0;		v1[2] = -175;
		v2[0] = -93;		v2[1] = 7;		v2[2] = -175;
		v3[0] = -85;		v3[1] = 7;		v3[2] = -167;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-93, 0, -175);
		glVertex3f(-93, 7, -175);
		glVertex3f(-85, 7, -167);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -93;		v1[1] = 0;		v1[2] = -8;
		v2[0] = -85;		v2[1] = 7;		v2[2] = -16;
		v3[0] = -93;		v3[1] = 7;		v3[2] = -8;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-93, 0, -8);
		glVertex3f(-85, 7, -16);
		glVertex3f(-93, 7, -8);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 85;		v1[1] = 0;		v1[2] = 183;
		v2[0] = -85;	v2[1] = 0;		v2[2] = 183;
		v3[0] = -77;	v3[1] = 7;		v3[2] = 175;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(85, 0, 183);
		glVertex3f(-85, 0, 183);
		glVertex3f(-77, 7, 175);
		glVertex3f(77, 7, 175);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(-85, 7, 183);
		glVertex3f(85, 7, 183);
		glVertex3f(77, 7, 175);
		glVertex3f(-77, 7, 175);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 77;		v1[1] = 7;		v1[2] = 175;
		v2[0] = 85;		v2[1] = 7;		v2[2] = 183;
		v3[0] = 85;		v3[1] = 0;		v3[2] = 183;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(77, 7, 175);
		glVertex3f(85, 7, 183);
		glVertex3f(85, 0, 183);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -77;		v1[1] = 7;		v1[2] = 175;
		v2[0] = -85;		v2[1] = 0;		v2[2] = 183;
		v3[0] = -85;		v3[1] = 7;		v3[2] = 183;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-77, 7, 175);
		glVertex3f(-85, 0, 183);
		glVertex3f(-85, 7, 183);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 85;		v1[1] = 0;		v1[2] = 183;
		v2[0] = 77;		v2[1] = 7;		v2[2] = -175;
		v3[0] = -77;	v3[1] = 7;		v3[2] = -175;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(85, 0, -183);
		glVertex3f(77, 7, -175);
		glVertex3f(-77, 7, -175);
		glVertex3f(-85, 0, -183);
	glEnd();
	glBegin(GL_POLYGON);
		glNormal3f(0, 1, 0);
		glVertex3f(-85, 7, -183);
        glVertex3f(-77, 7, -175);
		glVertex3f(77, 7, -175);
		glVertex3f(85, 7, -183);
	glEnd();
	glBegin(GL_POLYGON);
		v1[0] = 77;		v1[1] = 7;		v1[2] = -175;
		v2[0] = 85;		v2[1] = 0;		v2[2] = -183;
		v3[0] = 85;		v3[1] = 7;		v3[2] = -183;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(77, 7, -175);
		glVertex3f(85, 0, -183);
		glVertex3f(85, 7, -183);
	glEnd();
    glBegin(GL_POLYGON);
		v1[0] = -77;		v1[1] = 7;		v1[2] = -175;
		v2[0] = -85;		v2[1] = 7;		v2[2] = -183;
		v3[0] = -85;		v3[1] = 0;		v3[2] = -183;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
		glVertex3f(-77, 7, -175);
		glVertex3f(-85, 7, -183);
		glVertex3f(-85, 0, -183);
	glEnd();

}
void renderBitmapString(float x, float y, void *font, char *string) 
{  
  char *c;
  glRasterPos2f(x,y);
  for (c=string; *c != '\0'; c++) 
  {
    glutBitmapCharacter(font, *c);
  }
}
void drawstick()
{ float p[100][2],q[100][2]; float v1[2],v2[2],v3[2];
int EDGES=60; int i;
      glBegin(GL_POLYGON); 
   //   glVertex3f(0.0,0.0,0.0);
      for (i = 0; i <= EDGES; i++) 
      { 
        p[i][0]=cos((2*PI*i)/EDGES);
        p[i][1]=sin((2*PI*i)/EDGES);
        glVertex3f(p[i][0],4+p[i][1],0.0 );
		glNormal3f(0,1,-1);
        }
       glEnd(); 

      glBegin(GL_POLYGON); 
//      glVertex3f(0.0,0.0,0.0);
      for (i = 0; i <= EDGES; i++) 
      { 
        q[i][0]=3*cos((2*PI*i)/EDGES);
        q[i][1]=3*sin((2*PI*i)/EDGES);
        glVertex3f(q[i][0],10+q[i][1],100); 
		glNormal3f(0,1,1);
        }
       glEnd(); 

         
     glBegin(GL_QUAD_STRIP); 
     for (i = 0; i <= EDGES; i++) 
      { 
        glVertex3f(p[i][0], 4+p[i][1],0); 
        glVertex3f(q[i][0], 10+q[i][1],100);
	 }
	glEnd();        
	 for (i = 0; i <= EDGES; i++) 
      { 
		v1[0] =p[i][0];		v1[1] = 10+p[i][1];		v1[2] = 0;
		v2[0] = q[i][0];	v2[1] = 10+q[i][1];		v2[2] = 100;
		v3[0] = p[i+1][0];	v3[1] = 10+p[i][1];		v3[2] = 0;
		norm(v1,v2,v3);
		glNormal3f(n[0],n[1],n[2]);
	   }

       
      glEnd(); 
}

static void reset()
{
	w.flag=1;
	r.flag=1;
	b.flag=1;
	w.xspeed=0;
    w.zspeed=0;
	r.zspeed=0;
	r.xspeed=0;
	b.xspeed=0;
	b.zspeed=0;
	enablestick=1;
	animating = 0;
	glutPostRedisplay();
}


void board()
{

	if(w.x>81)
	{
		if((w.z<166.5 && w.z>3) || (w.z<-3 && w.z>-166.5))
		{w.xspeed = w.xspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
		else {w.y=-5000; 
		glColor3f(1,0,0);
        renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "Foul");

	}
	}
	if(w.x<-81)
	{
		if((w.z<166.5 && w.z>3) || (w.z<-3 && w.z>-166.5))
		{w.xspeed = w.xspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
		else { w.y=-5000;
		glColor3f(1,0,0);
		renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "Foul");
		 }
	}
	if(w.z>171)
	{
		if(w.x<77.5 && w.x>-77.5)
		{w.zspeed = w.zspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
		else{ w.y=-5000;
		glColor3f(1,0,0);
		renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "Foul");
		 }
	}
	if(w.z<-171)
	{
		if(w.x<77.5 && w.x>-77.5)
		{w.zspeed = w.zspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
		else { w.y=-5000;
		glColor3f(1,0,0);
		renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "Foul");
	}
	}

}

void redboard()
{
		if(r.x>81)
		{
			if((r.z<166.5 && r.z>3) || (r.z<-3 && r.z>-166.5))
			{r.xspeed = r.xspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
			else {r.y=-1500;
			glColor3f(1,0,0);
		    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
		 }
		}
	if(r.x<-81)
	{
			if((r.z<166.5 && r.z>3) || (r.z<-3 && r.z>-166.5))
			{r.xspeed = r.xspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
			else { r.y=-1500;
			glColor3f(1,0,0);
		    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
		 }
	}
	if(r.z>171)
	{
		if(r.x<77.5 && r.x>-77.5)
			{r.zspeed = r.zspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
			else { r.y=-1500;
			glColor3f(1,0,0);
		    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
		 }
	}
	if(r.z<-171)
	{
			if(r.x<77.5 && r.x>-77.5)
			{ r.zspeed = r.zspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
			else{ r.y=-1500;
			glColor3f(1,0,0);
		    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
	}
	}
}
void blackboard()
{
		if(b.x>81)
		{
			if((b.z<166.5 && b.z>3) || (b.z<-3 && b.z>-166.5))
			{b.xspeed = b.xspeed*-1; w.flag=1;r.flag=1;b.flag=1;}
			else { b.y=-1500;
			glColor3f(1,0,0);
		    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
		 }
		}
	if(b.x<-81)
	{
		if((b.z<166.5 && b.z>3) || (b.z<-3 && b.z>-166.5))
		{b.xspeed = b.xspeed*-1; w.flag=1;r.flag=1;b.flag=1;}
		else{ b.y=-1500;
		glColor3f(1,0,0);
		renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
	}
	}
	if(b.z>171)
	{
		if(b.x<77.5 && b.x>-77.5)
		{b.zspeed = b.zspeed*-1;w.flag=1;r.flag=1;b.flag=1;}
		else{ b.y=-1500;
		glColor3f(1,0,0);
		renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
	}
	}
	if(b.z<-171)
	{
		if(b.x<77.5 && b.x>-77.5)
		{b.zspeed = b.zspeed*-1; w.flag=1;r.flag=1;b.flag=1;}
		else{ b.y=-1500;
		glColor3f(1,0,0);
	    renderBitmapString(0,0, GLUT_BITMAP_TIMES_ROMAN_24, "GOAL!");
	}
	}
}

int moving;
void collision()
{
	if (((w.x>r.x-9)&&(w.x<r.x+9)) && ((w.z>r.z-9)&&(w.z<r.z+9)) && ((r.flag==1)&&(w.flag==1)))
	  if (moving == 0)
	{
		
 	    r.xspeed=w.xspeed;
		r.zspeed=w.zspeed;
		w.xspeed=0;
		w.zspeed=0;
        r.flag=0;
		w.flag=0;
		b.flag=1;
		moving=1;
	}
	else
	{
		 
 	    w.xspeed=r.xspeed;
		w.zspeed=r.zspeed;
		r.xspeed=0;
		r.zspeed=0;
        r.flag=0;
		w.flag=0;
		b.flag=1;
		moving=0;
	}
	else
	if (((w.x>b.x-9)&&(w.x<b.x+9)) && ((w.z>b.z-9)&&(w.z<b.z+9)) && ((w.flag==1)&&(b.flag==1)))
	  if (moving == 0)
	{
		 
 	    b.xspeed=w.xspeed;
		b.zspeed=w.zspeed;
		w.xspeed=0;
		w.zspeed=0;
        r.flag=1;
		w.flag=0;
		b.flag=0;
		moving=2;
	}
	else
	{
		 
 	    w.xspeed=b.xspeed;
		w.zspeed=b.zspeed;
		b.xspeed=0;
		b.zspeed=0;
        r.flag=1;
		w.flag=0;
		b.flag=0;
		moving=0;
	}
	else
	
	if (((b.x>r.x-9)&&(b.x<r.x+9)) && ((b.z>r.z-9)&&(b.z<r.z+9)) && ((b.flag==1)&&(r.flag==1)))
	  if (moving == 2)
	{
		 
 	    r.xspeed=b.xspeed;
		r.zspeed=b.zspeed;
		b.xspeed=0;
		b.zspeed=0;
        r.flag=0;
		w.flag=1;
		b.flag=0;
		moving=1;
	}
	else
	{
		 
 	    b.xspeed=r.xspeed;
		b.zspeed=r.zspeed;
		r.xspeed=0;
		r.zspeed=0;
        r.flag=0;
		w.flag=1;
		b.flag=0;
		moving=2;
	}
}

static void display(void)
{
  /* World-space positions for light and eye. */
  const float eyePosition[4] = { camera[0], camera[1], camera[2], 1 };
  const float lightPosition[4] = { 0,100,0, 1 };

  float translateMatrix[16], rotateMatrix[16],
        modelMatrix[16], invModelMatrix[16], viewMatrix[16],
        modelViewMatrix[16], modelViewProjMatrix[16];
  float objSpaceEyePosition[4], objSpaceLightPosition[4];

  buildLookAtMatrix(eyePosition[0], eyePosition[1], eyePosition[2],
                    0, 0, 0,
                    0, 1, 0,
                    viewMatrix);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  cgGLBindProgram(myCgVertexProgram);
  checkForCgError("binding vertex program");

  cgGLEnableProfile(myCgVertexProfile);
  checkForCgError("enabling vertex profile");

  cgGLBindProgram(myCgFragmentProgram);
  checkForCgError("binding fragment program");

  cgGLEnableProfile(myCgFragmentProfile);
  checkForCgError("enabling fragment profile");
  

  /*** Render able ***/

   
  setTableMaterial();

  /* modelView = rotateMatrix * translateMatrix */
  makeRotateMatrix(0, 1, 1, 1, rotateMatrix);
  makeTranslateMatrix(0, 0 , 0, translateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);
  drawtable();
  
  collision();

  /*** Render White Ball ***/
  setBallMaterial(w.r, w.g, w.b);
  
  if (w.xspeed>0) { w.xspeed=w.xspeed-friction; if (w.xspeed<=0){ w.xspeed=0;}else w.x=w.x+(w.xspeed);}
  else if(w.xspeed<0){ w.xspeed=w.xspeed+friction; if (w.xspeed>=0) {w.xspeed=0;}else w.x=w.x+(w.xspeed);}
	else  { w.xspeed = 0; 
			}
	if (w.zspeed>0) {w.zspeed=w.zspeed-friction; if (w.zspeed<=0) {w.zspeed=0;}else w.z=w.z+(w.zspeed);}
	else if (w.zspeed<0){ w.zspeed=w.zspeed+friction; if (w.zspeed>=0) {w.zspeed=0;}else w.z=w.z+(w.zspeed);}
	else { w.zspeed=0;
	}
  if (w.xspeed==0 && w.zspeed==0)
 {b.flag=1;
 w.flag=1;
 r.flag=1;
glutPostRedisplay(); 
 }
 
  board();
  
  /* modelView = viewMatrix * translateMatrix */
  makeTranslateMatrix(w.x, w.y, w.z, translateMatrix);
  makeRotateMatrix(90, 1, 0, 0, rotateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);

  glutWireSphere(4, 30, 30);

  /*** Render Red Ball ***/
  setBallMaterial(r.r,r.g,r.b);

  collision();
  //checkzerospeed();
  if (r.xspeed>0){r.xspeed=r.xspeed-friction; if (r.xspeed<=0) { r.xspeed=0;}else r.x=r.x+(r.xspeed);}
  else if(r.xspeed<0){r.xspeed=r.xspeed+friction; if (r.xspeed>=0) {r.xspeed=0;}else r.x=r.x+(r.xspeed);}
	else  { r.xspeed = 0;
			}
	if (r.zspeed>0){r.zspeed=r.zspeed-friction; if (r.zspeed<=0){ r.zspeed=0;}else r.z=r.z+(r.zspeed);}
	else if (r.zspeed<0){r.zspeed=r.zspeed+friction; if (r.zspeed>=0) { r.zspeed=0;}else r.z=r.z+(r.zspeed);}
	else r.zspeed=0;

	if (r.xspeed==0 && r.zspeed==0)
 {b.flag=1;
 w.flag=1;
 r.flag=1;
glutPostRedisplay(); 
 }
  redboard();
  /* modelView = viewMatrix * translateMatrix */
  makeTranslateMatrix(r.x, r.y, r.z, translateMatrix);
  makeRotateMatrix(90, 1, 0, 0, rotateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);

  glutSolidSphere(4, 30, 30); 

 /*** Render Black Ball ***/
  setBallMaterial(b.r,b.g,b.b);

  collision();
 
  if (b.xspeed>0){b.xspeed=b.xspeed-friction; if (b.xspeed<=0) {b.xspeed=0;}else  b.x=b.x+(b.xspeed);}
  else if(b.xspeed<0){b.xspeed=b.xspeed+friction; if (b.xspeed>=0) { b.xspeed=0;}else  b.x=b.x+(b.xspeed);}
	else  { b.xspeed = 0;
			}
	if (b.zspeed>0){b.zspeed=b.zspeed-friction; if (b.zspeed<=0){ b.zspeed=0;}else b.z=b.z+(b.zspeed);}
	else if (b.zspeed<0){b.zspeed=b.zspeed+friction; if (b.zspeed>=0) { b.zspeed=0;}else b.z=b.z+(b.zspeed);}
	else b.zspeed=0;

 if (b.xspeed==0 && b.zspeed==0)
 {b.flag=1;
 w.flag=1;
 r.flag=1;
glutPostRedisplay(); 
 }
  blackboard();
 
  /* modelView = viewMatrix * translateMatrix */
  makeTranslateMatrix(b.x, b.y, b.z, translateMatrix);
  makeRotateMatrix(90, 1, 0, 0, rotateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);

  glutSolidSphere(4, 30, 30);

    /*** Render Stick***/
  if (enablestick)
  
  { //printf("stick enabled");
  setStickMaterial();
  /* modelView = viewMatrix * translateMatrix */
  makeTranslateMatrix(w.x,0,w.z, translateMatrix);
  makeRotateMatrix(stickangle, 0, 1, 0, rotateMatrix);
  multMatrix(modelMatrix, translateMatrix, rotateMatrix);

  /* invModelMatrix = inverse(modelMatrix) */
  invertMatrix(invModelMatrix, modelMatrix);

  /* Transform world-space eye and light positions to sphere's object-space. */
  transform(objSpaceEyePosition, invModelMatrix, eyePosition);
  cgSetParameter3fv(myCgFragmentParam_eyePosition, objSpaceEyePosition);
  transform(objSpaceLightPosition, invModelMatrix, lightPosition);
  cgSetParameter3fv(myCgFragmentParam_lightPosition, objSpaceLightPosition);

  /* modelViewMatrix = viewMatrix * modelMatrix */
  multMatrix(modelViewMatrix, viewMatrix, modelMatrix);

  /* modelViewProj = projectionMatrix * modelViewMatrix */
  multMatrix(modelViewProjMatrix, myProjectionMatrix, modelViewMatrix);

  /* Set matrix parameter with row-major matrix. */
  cgSetMatrixParameterfr(myCgVertexParam_modelViewProj, modelViewProjMatrix);
drawstick();
  
/*  glBegin(GL_LINES);
  glVertex3f(0,0,0);
  glVertex3f(0,20,84);
  glEnd();
 */
  }

  glutSwapBuffers();
}

static void idle(void)
{
  myLightAngle += 0.008;  /* Add a small angle (in radians). */
  if (myLightAngle > 2*myPi) {
    myLightAngle -= 2*myPi;
  }
  glutPostRedisplay();
}


static void keyboard(unsigned char c, int x, int y)
{
  

  switch (c) {
  case ' ':
    animating = !animating; /* Toggle */
    if (animating) {
	  enablestick=0;
	  moving=0;
	    w.xspeed=power*cos(2*angle*myPi/360); 
		w.zspeed=power*sin(2*angle*myPi/360);
      glutIdleFunc(idle);
    } else {
      reset();
      glutIdleFunc(NULL);
    }    
    break;
  case '1':
	  camera[0]=0; camera[1]=200; camera[2]=400;
  	  angle = -90;
	  stickangle = 0;

	glutPostRedisplay();
	  break;
  case  '2':
	  camera[0]=500; camera[1]=200; camera[2]=0;
	  angle = 180;
	  stickangle = 270;
	glutPostRedisplay();
	  break;
  case '3':
	  camera[0]=-500; camera[1]=200; camera[2]=0;
	  angle = 0;
	  stickangle = 90;
	  glutPostRedisplay();
	  break;
  case '4':
	  camera[0]=0; camera[1]=200; camera[2]=-400;
	  angle = 90;
	  stickangle=180;
	  glutPostRedisplay();
	break;
  case 'z':
	 angle = angle + 2;
	 stickangle=90+angle;
	  glutPostRedisplay();
      break;
  case 'x':
	angle = angle - 2;
	stickangle=90+angle;
	  glutPostRedisplay();
       break;
  case 'n':
		  if (camera[0]==0 && camera[1]<=200 && camera[1]>=50 && camera[2]<=400 && camera[2]>100)
	  {
		 camera[1]-=25;
		 camera[2]-=50;
		 glutPostRedisplay();
	  }
	  	  if (camera[0]==0 && camera[1]<=200 && camera[1]>=50 && camera[2]>=-400 && camera[2]<100)
	  {
		 camera[1]-=25;
		 camera[2]+=50;
		 glutPostRedisplay();
	  }
		  if (camera[0]<=500 && camera[0]>100 && camera[1]<=200 && camera[1]>50 && camera[2]==0)
	  {
		 camera[0]-=50;
		 camera[1]-=25;
		 glutPostRedisplay();
	  }
		  if (camera[0]>=-500 && camera[0]<=-100 && camera[1]<=200 && camera[1]>50 && camera[2]==0)
	  {
		 camera[0]+=50;
		 camera[1]-=25;
		 glutPostRedisplay();
	  }
      break;
 case 'm':

	  if (camera[0]==0 && camera[1]<=175 && camera[1]>=25 && camera[2]<=350 && camera[2]>75)
	  {
		 camera[1]+=25;
		 camera[2]+=50;
		 glutPostRedisplay();
	  }
	  	  if (camera[0]==0 && camera[1]<=175 && camera[1]>=25 && camera[2]>=-350 && camera[2]<50)
	  {
		 camera[1]+=25;
		 camera[2]-=50;
		 glutPostRedisplay();
	  }
		  if (camera[0]<=450 && camera[0]>=50 && camera[1]<=175 && camera[1]>25 && camera[2]==0)
	  {
		 camera[0]+=50;
		 camera[1]+=25;
		 glutPostRedisplay();
	  }
		  if (camera[0]>=-450 && camera[0]<=-50 && camera[1]<=175 && camera[1]>25 && camera[2]==0)
	  {
		 camera[0]-=50;
		 camera[1]+=25;
		 glutPostRedisplay();
	  }
      break;
  case 27:  /* Esc key */
    /* Demonstrate proper deallocation of Cg runtime data structures.
       Not strictly necessary if we are simply going to exit. */
    cgDestroyProgram(myCgVertexProgram);
    cgDestroyContext(myCgContext);
    exit(0);
    break;
  }
}
static void idle2(void)
{

	 keyboard(' ',0,0);
	glutPostRedisplay();
}

static void menu(int item)
{
  /* Pass menu item character code to keyboard callback. */
  keyboard((unsigned char)item, 0, 0);
}
static void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) 
	{	     glutIdleFunc(idle2);
	
	}
	
}
