/*
CSCI 420 Computer Graphics, USC
Assignment 1: Height Fields
C++ starter code

Student username: <tankevin>
*/
#include <string>
#include <iostream>
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"
#include <ctime>
#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;
int  mapState = 1; //1=point, 2=line, 3=triangle
// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;
ImageIO * otherImage;
float* positions;	//vertices
float* colors;		//colors associated with vertices
int x, y;

OpenGLMatrix * matrix;
GLuint buffer;		//main buffer
GLuint program;
BasicPipelineProgram* pipelineProgram;
GLuint vao;			//vertex array object
int numVertices;	//number of vertices that will be in positions array
GLuint* pointIndices;//indices for point state
GLuint* lineIndices;//indices for line state
GLuint* triangleIndices;//indices for triangle state
int positionsSize;//size of positions array
int colorsSize;//size of colors array
GLuint pointBuffer;
GLuint lineBuffer;
GLuint triangleBuffer;
bool stop;//press space to stop it from spinning
bool photoMode;//for multiple pictures
unsigned int counter = 0;

void initVBO()//initializing vertex buffer object
{
	//initializing values
	int width = heightmapImage->getWidth();
	int height = heightmapImage->getHeight();
	numVertices = width * height;
	positions = new float[numVertices * 3];
	colors = new float[numVertices * 4];
	positionsSize = (numVertices * sizeof(float)* 3);
	colorsSize = (numVertices * sizeof(float)* 4);

	for (int i = 0; i < width; i++)	{//fill positions and colors array
		for (int j = 0; j < height; j++){
			positions[3 * (i*height + j)] = i - width / 2; // ranges from -width/2 to width/2
			positions[3 * (i*height + j) + 1] = j - height / 2; //ranges from -height/2 to height/2
			positions[3 * (i*height + j) + 2] = 0.2*(height / 255)*heightmapImage->getPixel(i, j, 0); // brightness of pixel determines height

			// fill colors array
			colors[4 * (i*height + j)] = otherImage->getPixel(i, j, 0) / (float)(255); // red
			colors[4 * (i*height + j) + 1] = otherImage->getPixel(i, j, 0) / (float)(255); // green
			colors[4 * (i*height + j) + 2] = otherImage->getPixel(i, j, 0) / (float)(255); // blue
			colors[4 * (i*height + j) + 3] = 1;
		}
	}
	//fill the three indices arrays
	pointIndices = new GLuint[numVertices];
	for (int i = 0; i<numVertices; i++){//fill point indices
		pointIndices[i] = i;
	}
	lineIndices = new GLuint[numVertices * 10];//fill line indices
	for (int i = 0; i < numVertices - width; i++){
		lineIndices[i * 2] = i;
		lineIndices[(i * 2) + 1] = i + width;
	}
	for (int i = numVertices; i < numVertices * 2; i++){
		if ((i%height) + 1< height){
			lineIndices[(i * 2)] = i - numVertices;
			lineIndices[(i * 2) + 1] = i + 1 - numVertices;
		}
	}
	triangleIndices = new GLuint[numVertices * 10];//fill triangle indices
	for (int i = 0; i < numVertices - width; i++){
		if ((i%height) + 1< height && (i%height)>1){
			triangleIndices[i * 5] = i;
			triangleIndices[(i * 5) + 1] = i + width;
			triangleIndices[(i * 5) + 2] = i + 1;
			triangleIndices[(i * 5) + 3] = i + 1;
			triangleIndices[(i * 5) + 4] = i + width;
		}
		else{
			triangleIndices[i * 5] = i;
			triangleIndices[(i * 5) + 1] = i;
			triangleIndices[(i * 5) + 2] = i;
			triangleIndices[(i * 5) + 3] = i;
			triangleIndices[(i * 5) + 4] = i;
		}
	}
	// create the VBO: 
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, positionsSize + colorsSize, NULL, GL_STATIC_DRAW); // init buffer’s size, but don’t assign any data to it
	// upload position data
	glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, positions);
	// upload color data
	glBufferSubData(GL_ARRAY_BUFFER, positionsSize, colorsSize, colors);

	// upload pointIndices data
	glGenBuffers(1, &pointBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pointBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices, pointIndices, GL_STATIC_DRAW);
	//upload lineIndices data
	glGenBuffers(1, &lineBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices * 10, lineIndices, GL_STATIC_DRAW);
	//upload triangleIndices data
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices * 10, triangleIndices, GL_STATIC_DRAW);
}
void initVAO(){
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);//bind the VAO

	GLuint loc = glGetAttribLocation(program, "position");	//LOCATION
	glEnableVertexAttribArray(loc);//enable the "position" attribute
	const void * offset = (const void*)0; GLsizei stride = 0; GLboolean normalized = GL_FALSE;
	glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, offset);//set the layout of the "position" attribute data

	loc = glGetAttribLocation(program, "color"); //color
	glEnableVertexAttribArray(loc); //enable the "color" attribute
	offset = (const void*)positionsSize;
	glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, offset);

	glBindVertexArray(0);//unbind the VAO
}
void initPipelineProgram(){
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
	program = pipelineProgram->GetProgramHandle();

}

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}
void bindProgram()
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer); // so that glVertexAttribPointer refers to the correct buffer
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	const void * offset = (const void*)0;
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);

	GLuint loc2 = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(loc2);
	const void * offset2 = (const void*) sizeof(positions);
	glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);

}
void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->LoadIdentity();
	matrix->LookAt(0, 0, 500,
		0, 0, -1, 0, 1, 0); // camera 500 up in z axis looking down in negative z axis. positive y axis is up
	matrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);//just transformations here
	matrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
	matrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
	matrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
	matrix->Scale(landScale[0], landScale[1], landScale[2]);

	bindProgram();

	GLint h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
	float m[16];	//modelview array
	matrix->GetMatrix(m);
	GLboolean isRowMajor = GL_FALSE;
	glUniformMatrix4fv(h_modelViewMatrix, 1, isRowMajor, m);	//upload m to the GPU

	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	GLint h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
	float p[16];//projection array
	matrix->GetMatrix(p);
	glUniformMatrix4fv(h_projectionMatrix, 1, isRowMajor, p); //upload p to GPU

	pipelineProgram->Bind();//activating this pipeline program
	glBindVertexArray(vao);//bind VAO
	if (mapState == 1)//points
		glDrawElements(GL_POINTS, numVertices, GL_UNSIGNED_INT, pointIndices);
	else if (mapState == 2)//lines
		//glDrawElements(GL_LINES, numVertices * 10, GL_UNSIGNED_INT, lineIndices);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, heightmapImage->getHeight()*heightmapImage->getWidth() * 10);
	else//triangles
		glDrawElements(GL_TRIANGLE_STRIP, numVertices * 5, GL_UNSIGNED_INT, triangleIndices);
	glBindVertexArray(0);//unbind VAO
	glutSwapBuffers();//dual buffer swap
}


GLfloat delta = .1;
GLint axis = 2;
void idleFunc()
{
	// do some stuff... 

	// for example, here, you can save the screenshots to disk (to make the animation)

	// make the screen update 
	// spin the quad delta degrees around the selected axis
	if (!stop){
		landRotate[axis] += delta;
		if (landRotate[axis] > 360.0)
			landRotate[axis] -= 360.0;
	}
	if (photoMode){
		if ((int)landRotate[axis] % 7 == 0){
			std::string s = std::to_string(counter);
			if (s.length() == 1){
				s = "00" + s;
			}
			else if (s.length() == 2){
				s = "0" + s;
			}
			s.append(".jpg");
			char const *pchar = s.c_str();
			saveScreenshot(pchar);
			counter++;
		}

	}
	//display result (do not forget this!)
	glutPostRedisplay();
}
void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);
	// setup perspective matrix...
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->LoadIdentity();
	matrix->Perspective(60, aspect, 1, 2000);//our viewpoint
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
	case TRANSLATE:
		if (leftMouseButton)
		{
			// control x,y translation via the left mouse button
			landTranslate[0] += mousePosDelta[0] * 0.5f;
			landTranslate[1] -= mousePosDelta[1] * 0.5f;
		}
		if (middleMouseButton)
		{
			// control z translation via the middle mouse button
			landTranslate[2] += mousePosDelta[1] * 0.5f;
		}
		break;

		// rotate the landscape
	case ROTATE:
		if (leftMouseButton)
		{
			// control x,y rotation via the left mouse button
			landRotate[0] += mousePosDelta[1];
			landRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control z rotation via the middle mouse button
			landRotate[2] += mousePosDelta[1];
		}
		break;

		// scale the landscape
	case SCALE:
		if (leftMouseButton)
		{
			// control x,y scaling via the left mouse button
			landScale[0] *= 1.0f + mousePosDelta[0] * .01f;
			landScale[1] *= 1.0f - mousePosDelta[1] * .01f;
		}
		if (middleMouseButton)
		{
			// control z scaling via the middle mouse button
			landScale[2] *= 1.0f - mousePosDelta[1] * .1f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
	default:
		controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case ' ':
		stop = !stop;
		break;

	case 'x':
		// take a screenshot
		saveScreenshot("screenshot.jpg");
		break;
	case 'z':
		//multiple screenshots till stopped
		photoMode = !photoMode;
		counter = 0;
		break;

	case '1':
		//point view
		mapState = 1;
		break;

	case '2':
		//line view
		mapState = 2;
		break;
	case '3':
		//triangle view
		mapState = 3;
		break;
	case '=':
		//speed up rotation
		delta = delta + .1;
		break;
	case'-':
		//slow down rotation
		delta = delta - .1;
		break;
	}
}

void initScene(int argc, char *argv[])
{
	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}
	otherImage = new ImageIO();// happy face
	otherImage->loadJPEG("./heightmap/smile.jpeg");
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	matrix = new OpenGLMatrix();
	stop = false;		//initially it is not stopped (its rotating)
	photoMode = false;
	initVBO();
	initPipelineProgram();
	initVAO();
	glShadeModel(GL_SMOOTH);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);
	cout << "Initializing OpenGL..." << endl;
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	glutDisplayFunc(displayFunc);	// tells glut to use a particular display function to redraw 
	glutIdleFunc(idleFunc);	// perform animation inside idleFunc
	glutMotionFunc(mouseMotionDragFunc);	// callback for mouse drags
	glutPassiveMotionFunc(mouseMotionFunc);	// callback for idle mouse movement
	glutMouseFunc(mouseButtonFunc);	// callback for mouse button changes
	glutReshapeFunc(reshapeFunc);	// callback for resizing the window
	glutKeyboardFunc(keyboardFunc);	// callback for pressing the keys on the keyboard
	// init glew
#ifdef __APPLE__
	// nothing is needed on Apple
#else
	// Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);
	// sink forever into the glut loop
	glutMainLoop();
}


