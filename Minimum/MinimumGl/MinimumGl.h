/////////////////////////////////////////////////////////////////////////////
// Name:        cube.h
// Purpose:     wxGLCanvas demo program
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CUBE_H_
#define _WX_CUBE_H_

#include <GL/glew.h>
#include "wx/glcanvas.h"
#include "SimpleObject.h"

// the rendering context used by all GL canvases
class TestGLContext : public wxGLContext
{
public:
    TestGLContext(wxGLCanvas *canvas);
	~TestGLContext();
    // render the cube showing it at given angles
    void DrawRotatedCube(float xangle, float yangle);

private:
    // textures for the cube faces
    GLuint m_textures[6];

	GLuint g_vertexShader;
	GLuint g_fragmentShader;
	GLuint g_shaderProgram;

	const GLfloat normalsAndVertices[3][8] =
	{
		{ -0.5f, -0.5f, 0.5f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f },
		{ 0.5f, -0.5f, 0.5f, 0.0f,  0.0f,  1.0f, 1.0f, 0.0f },
		{ 0.0f,  1.0f, 0.5f, 0.0f,  0.0f,  1.0f, 0.5f, 1.0f },
	};

	SimpleObject* pOrigObj;
	vector<SimpleObject*> objects;
};

// Define a new application type
class MyApp : public wxApp
{
public:
    MyApp() { m_glContext = NULL;}

    // Returns the shared context used by all frames and sets it as current for
    // the given canvas.
    TestGLContext& GetContext(wxGLCanvas *canvas);

    // virtual wxApp methods
    virtual bool OnInit() wxOVERRIDE;
    virtual int OnExit() wxOVERRIDE;

private:
    // the GL context we use for all our mono rendering windows
    TestGLContext *m_glContext;
};

// Define a new frame type
class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    void OnClose(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

class TestGLCanvas : public wxGLCanvas
{
public:
    TestGLCanvas(wxWindow *parent);

private:
    void OnPaint(wxPaintEvent& event);
    void Spin(float xSpin, float ySpin);
    void OnKeyDown(wxKeyEvent& event);
    void OnSpinTimer(wxTimerEvent& WXUNUSED(event));

    // angles of rotation around x- and y- axis
    float m_xangle,
          m_yangle;

    wxTimer m_spinTimer;

    wxDECLARE_EVENT_TABLE();
};

enum
{
    NEW_STEREO_WINDOW = wxID_HIGHEST + 1
};

#endif // _WX_CUBE_H_
