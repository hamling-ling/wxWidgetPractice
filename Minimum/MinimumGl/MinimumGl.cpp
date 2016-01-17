// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "MinimumGl.h"
#include "Texture.h"
#include "Matrix4x4f.h"
#include <GL/wglew.h>

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// control ids
enum
{
    SpinTimer = wxID_HIGHEST + 1
};

// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void CheckGLError()
{
    GLenum errLast = GL_NO_ERROR;

    for ( ;; )
    {
        GLenum err = glGetError();
        if ( err == GL_NO_ERROR )
            return;

        // normally the error is reset by the call to glGetError() but if
        // glGetError() itself returns an error, we risk looping forever here
        // so check that we get a different error than the last time
        if ( err == errLast )
        {
            wxLogError(wxT("OpenGL error state couldn't be reset."));
            return;
        }

        errLast = err;

        wxLogError(wxT("OpenGL error %d"), err);
    }
}

// function to draw the texture for cube faces
static wxImage DrawDice(int size, unsigned num)
{
    wxASSERT_MSG( num >= 1 && num <= 6, wxT("invalid dice index") );

    const int dot = size/16;        // radius of a single dot
    const int gap = 5*size/32;      // gap between dots

    wxBitmap bmp(size, size);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);

    // the upper left and lower right points
    if ( num != 1 )
    {
        dc.DrawCircle(gap + dot, gap + dot, dot);
        dc.DrawCircle(size - gap - dot, size - gap - dot, dot);
    }

    // draw the central point for odd dices
    if ( num % 2 )
    {
        dc.DrawCircle(size/2, size/2, dot);
    }

    // the upper right and lower left points
    if ( num > 3 )
    {
        dc.DrawCircle(size - gap - dot, gap + dot, dot);
        dc.DrawCircle(gap + dot, size - gap - dot, dot);
    }

    // finally those 2 are only for the last dice
    if ( num == 6 )
    {
        dc.DrawCircle(gap + dot, size/2, dot);
        dc.DrawCircle(size - gap - dot, size/2, dot);
    }

    dc.SelectObject(wxNullBitmap);

    return bmp.ConvertToImage();
}

// ============================================================================
// implementation
// ============================================================================

static void LoadShaderSource(GLuint shader, const char* fileName)
{
	FILE* fp = NULL;
	int size;
	char* buf;

	errno_t err = fopen_s(&fp, fileName, "rb");
	if (err != 0)
		return;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	buf = (char *)malloc(size);
	fseek(fp, 0, SEEK_SET);

	fread(buf, 1, size, fp);

	glShaderSource(shader, 1, (const GLchar **)&buf, &size);

	free(buf);
	fclose(fp);

}
// ----------------------------------------------------------------------------
// TestGLContext
// ----------------------------------------------------------------------------

TestGLContext::TestGLContext(wxGLCanvas *canvas)
             : wxGLContext(canvas)
{
    SetCurrent(*canvas);

	CheckGLError();
	glewInit();
	CheckGLError();
#if 0
#else
	g_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	LoadShaderSource(g_vertexShader, "Texture.vert");
	glCompileShader(g_vertexShader);

	g_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	LoadShaderSource(g_fragmentShader, "Texture.frag");
	glCompileShader(g_fragmentShader);

	g_shaderProgram = glCreateProgram();
	glAttachShader(g_shaderProgram, g_vertexShader);
	glAttachShader(g_shaderProgram, g_fragmentShader);

	glLinkProgram(g_shaderProgram);

	GLint vertexLocation = glGetAttribLocation(g_shaderProgram, "Vertex");
	GLint normalLocation = glGetAttribLocation(g_shaderProgram, "Normal");
	GLint texCoordLocation = glGetAttribLocation(g_shaderProgram, "TexCoord");

	glDeleteShader(g_vertexShader);
	glDeleteShader(g_fragmentShader);

	CTexture texture;
	texture.LoadBitmapFile("texture.bmp");
	pOrigObj = new SimpleObject();
	CheckGLError();
	pOrigObj->BindBuffer(vertexLocation, normalLocation, texCoordLocation,
		&(normalsAndVertices[0][0]), 3,
		texture);
	CheckGLError();

	glDisableVertexAttribArray(glGetAttribLocation(g_shaderProgram, "Vertex"));
	glDisableVertexAttribArray(glGetAttribLocation(g_shaderProgram, "Normal"));
	glDisableVertexAttribArray(glGetAttribLocation(g_shaderProgram, "TexCoord"));

	glUseProgram(g_shaderProgram);
	GLint tex = glGetUniformLocation(g_shaderProgram, "surfaceTexture");
	glUniform1i(tex, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

	const GLfloat lightPosition[4] = { 3.0f, 4.0f, 0.0f, 0.0f };
	const GLfloat lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat lightAmbient[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
	const GLfloat lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	const GLfloat cubeDiffuse[4] = { 0.75f, 0.0f, 1.0f, 1.0f };
	const GLfloat cubeAmbient[4] = { 0.3f, 0.25f, 0.4f, 1.0f };
	const GLfloat cubeSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat cubeShininess[1] = { 32.0f };

	glUseProgram(g_shaderProgram);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "lightPosition"), 1, lightPosition);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ld"), 1, lightDiffuse);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "La"), 1, lightAmbient);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ls"), 1, lightSpecular);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Kd"), 1, cubeDiffuse);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ka"), 1, cubeAmbient);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ks"), 1, cubeSpecular);
	glUniform1fv(glGetUniformLocation(g_shaderProgram, "shininess"), 1, cubeShininess);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, pOrigObj->GetTextureObject());
	glBindVertexArray(pOrigObj->GetVertexArrayObject());
	glDrawArrays(GL_TRIANGLES, 0, pOrigObj->GetVertexArrayLen());
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFlush();

	glUseProgram(0);
#endif
	CheckGLError();
}

TestGLContext::~TestGLContext()
{
	delete pOrigObj;
	glDeleteProgram(g_shaderProgram);
}

void TestGLContext::DrawRotatedCube(float nWidth, float nHeight)
{
	CheckGLError();
#if 0
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

#else
	CMatrix4x4f perspective;;

	float l, r, b, t, n = 0.1f, f = 10.0f;
	t = n * tanf(PI * (45.0f / 2.0f) / 180.0f);
	b = -t;
	r = t * (float)nWidth / (float)nHeight;
	l = -r;
	perspective.MakePerspective(l, r, b, t, n, f);
	float projection[16];
	perspective.GetGLMat(projection);

	CMatrix4x4f rotationX;
	CMatrix4x4f rotationY;
	CMatrix4x4f translation;
	CMatrix4x4f lookAt;

	float distance = sqrtf(3.0f * 2.5f * 2.5f);
	float angle = atan2f(1.0f, sqrtf(2.0f)) * 180.0f / PI;

	translation.MakeTranslation(CVector3f(0.0f, 0.0f, -distance));
	rotationX.MakeRotation(CVector3f(1.0f, 0.0f, 0.0f), angle);
	rotationY.MakeRotation(CVector3f(0.0f, 1.0f, 0.0f), 0.0f);
	lookAt = translation * rotationX * rotationY;
	float modelViewMatrix[16];
	float viewMatrix[16];
	lookAt.GetGLMat(modelViewMatrix);
	lookAt.GetGLMat(viewMatrix);

	CMatrix4x4f lookAtPerspective;
	lookAtPerspective = perspective * lookAt;
	float modelViewProjectionMatrix[16];
	lookAtPerspective.GetGLMat(modelViewProjectionMatrix);

	CheckGLError();
	glUseProgram(g_shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(g_shaderProgram, "modelViewProjectionMatrix"), 1, GL_FALSE, modelViewProjectionMatrix);
	glUniformMatrix4fv(glGetUniformLocation(g_shaderProgram, "modelViewMatrix"), 1, GL_FALSE, modelViewMatrix);
	glUniformMatrix4fv(glGetUniformLocation(g_shaderProgram, "viewMatrix"), 1, GL_FALSE, viewMatrix);

	const GLfloat lightPosition[4] = { 3.0f, 4.0f, 0.0f, 0.0f };
	const GLfloat lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat lightAmbient[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
	const GLfloat lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	const GLfloat cubeDiffuse[4] = { 0.75f, 0.0f, 1.0f, 1.0f };
	const GLfloat cubeAmbient[4] = { 0.3f, 0.25f, 0.4f, 1.0f };
	const GLfloat cubeSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const GLfloat cubeShininess[1] = { 32.0f };
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "lightPosition"), 1, lightPosition);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ld"), 1, lightDiffuse);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "La"), 1, lightAmbient);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ls"), 1, lightSpecular);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Kd"), 1, cubeDiffuse);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ka"), 1, cubeAmbient);
	glUniform4fv(glGetUniformLocation(g_shaderProgram, "Ks"), 1, cubeSpecular);
	glUniform1fv(glGetUniformLocation(g_shaderProgram, "shininess"), 1, cubeShininess);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, pOrigObj->GetTextureObject());
	glBindVertexArray(pOrigObj->GetVertexArrayObject());
	glDrawArrays(GL_TRIANGLES, 0, pOrigObj->GetVertexArrayLen());
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	glUseProgram(0);
#endif
	CheckGLError();
}


// ----------------------------------------------------------------------------
// MyApp: the application object
// ----------------------------------------------------------------------------

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    new MyFrame();

    return true;
}

int MyApp::OnExit()
{
    delete m_glContext;

    return wxApp::OnExit();
}

TestGLContext& MyApp::GetContext(wxGLCanvas *canvas)
{
    TestGLContext *glContext;
    if ( !m_glContext )
    {
        // Create the OpenGL context for the first mono window which needs it:
        // subsequently created windows will all share the same context.
        m_glContext = new TestGLContext(canvas);
    }
    glContext = m_glContext;

    glContext->SetCurrent(*canvas);

    return *glContext;
}

// ----------------------------------------------------------------------------
// TestGLCanvas
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_KEY_DOWN(TestGLCanvas::OnKeyDown)
    EVT_TIMER(SpinTimer, TestGLCanvas::OnSpinTimer)
wxEND_EVENT_TABLE()

static const int attributes[] = {
	WGL_DOUBLE_BUFFER_ARB,
	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	WGL_CONTEXT_MINOR_VERSION_ARB, 3,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0
};

TestGLCanvas::TestGLCanvas(wxWindow *parent)
    // With perspective OpenGL graphics, the wxFULL_REPAINT_ON_RESIZE style
    // flag should always be set, because even making the canvas smaller should
    // be followed by a paint event that updates the entire canvas with new
    // viewport settings.
    : wxGLCanvas(parent, wxID_ANY, attributes,
                 wxDefaultPosition, wxDefaultSize,
                 wxFULL_REPAINT_ON_RESIZE),
      m_xangle(30.0),
      m_yangle(30.0),
      m_spinTimer(this,SpinTimer)
{
}

void TestGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    // This is required even though dc is not used otherwise.
    wxPaintDC dc(this);

    // Set the OpenGL viewport according to the client size of this canvas.
    // This is done here rather than in a wxSizeEvent handler because our
    // OpenGL rendering context (and thus viewport setting) is used with
    // multiple canvases: If we updated the viewport in the wxSizeEvent
    // handler, changing the size of one canvas causes a viewport setting that
    // is wrong when next another canvas is repainted.
    const wxSize ClientSize = GetClientSize();

    TestGLContext& canvas = wxGetApp().GetContext(this);
    glViewport(0, 0, ClientSize.x, ClientSize.y);
	canvas.DrawRotatedCube(ClientSize.x, ClientSize.y);
    SwapBuffers();
}

void TestGLCanvas::Spin(float xSpin, float ySpin)
{
    m_xangle += xSpin;
    m_yangle += ySpin;

    Refresh(false);
}

void TestGLCanvas::OnKeyDown(wxKeyEvent& event)
{
    float angle = 5.0;

    switch ( event.GetKeyCode() )
    {
        case WXK_RIGHT:
            Spin( 0.0, -angle );
            break;

        case WXK_LEFT:
            Spin( 0.0, angle );
            break;

        case WXK_DOWN:
            Spin( -angle, 0.0 );
            break;

        case WXK_UP:
            Spin( angle, 0.0 );
            break;

        case WXK_SPACE:
            if ( m_spinTimer.IsRunning() )
                m_spinTimer.Stop();
            else
                m_spinTimer.Start( 25 );
            break;

        default:
            event.Skip();
            return;
    }
}

void TestGLCanvas::OnSpinTimer(wxTimerEvent& WXUNUSED(event))
{
    Spin(0.0, 4.0);
}

wxString glGetwxString(GLenum name)
{
    const GLubyte *v = glGetString(name);
    if ( v == 0 )
    {
        // The error is not important. It is GL_INVALID_ENUM.
        // We just want to clear the error stack.
        glGetError();

        return wxString();
    }

    return wxString((const char*)v);
}


// ----------------------------------------------------------------------------
// MyFrame: main application window
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnNewWindow)
    EVT_MENU(wxID_CLOSE, MyFrame::OnClose)
wxEND_EVENT_TABLE()

MyFrame::MyFrame()
       : wxFrame(NULL, wxID_ANY, wxT("wxWidgets OpenGL Cube Sample"))
{
    //int stereoAttribList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_STEREO, 0 };

    new TestGLCanvas(this);

    SetIcon(wxICON(sample));

    // Make a menubar
    wxMenu *menu = new wxMenu;
    menu->Append(wxID_NEW);
    menu->AppendSeparator();
    menu->Append(wxID_CLOSE);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menu, wxT("&Cube"));

    SetMenuBar(menuBar);

    CreateStatusBar();

    SetClientSize(400, 400);
    Show();

    // test IsDisplaySupported() function:
    static const int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };
    wxLogStatus("Double-buffered display %s supported",
                wxGLCanvas::IsDisplaySupported(attribs) ? "is" : "not");
}

void MyFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnNewWindow( wxCommandEvent& WXUNUSED(event) )
{
    new MyFrame();
}

