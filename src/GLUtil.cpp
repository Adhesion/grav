/*
 * @file GLUtil.cpp
 * Implementation of GL/miscellaneous 3D utility functions.
 * @author Andrew Ford
 */

#include "GLUtil.h"
#include "VideoSource.h"
#include <string>

GLUtil* GLUtil::instance = NULL;

GLUtil* GLUtil::getInstance()
{
    if ( instance == NULL )
    {
        instance = new GLUtil();
    }
    return instance;
}

bool GLUtil::initGL()
{
    glewInit();

    // check GL version and use shaders if 2.0 or higher
    const char* glVer = (const char*)glGetString( GL_VERSION );
    int glMajorVer, glMinorVer;
    sscanf( glVer, "%d.%d", &glMajorVer, &glMinorVer );
    if ( glMajorVer >= 2 && enableShaders )
    {
        YUV420Program = GLUtil::loadShaders( "GLSL/YUV420toRGB24" );
        if ( YUV420Program )
        {
            YUV420xOffsetID = glGetUniformLocation( YUV420Program, "xOffset" );
            YUV420yOffsetID = glGetUniformLocation( YUV420Program, "yOffset" );
            YUV420alphaID = glGetUniformLocation( YUV420Program, "alpha" );
            shadersAvailable = true;
            printf( "GLUtil::initGL(): shaders are available (GL v%s)\n",
                        glVer );
        }
        else
        {
            shadersAvailable = false;
            printf( "GLUtil::initGL(): shaders NOT available (GL v%s)\n",
                        glVer );
        }
    }
    else
    {
        shadersAvailable = false;
        printf( "GLUtil::initGL(): shaders NOT available (GL v%s)\n", glVer );
    }

    // TODO: could use a good way to find system-specific font dirs
    if ( useBufferFont )
        mainFont = new FTBufferFont( "FreeSans.ttf" );
    else
        mainFont = new FTTextureFont( "FreeSans.ttf" );
    if ( mainFont->Error() )
    {
        printf( "GLUtil::initGL(): ERROR: font failed to load\n" );
        delete mainFont;
        mainFont = NULL;
        return false;
    }
    else
    {
        printf( "GLUtil::initGL(): font created\n" );
        mainFont->FaceSize( 100 );
    }

    // TODO this is platform-specific, see the glxew include in glutil.h
    if ( GLX_SGI_swap_control )
    {
        printf( "GLUtil::have glx sgi swap control\n" );
        glXSwapIntervalSGI( 1 );
    }
    else
        printf( "GLUtil::no swap control\n" );

    glEnable( GL_DEPTH_TEST );

    return true;
}

void GLUtil::cleanupGL()
{
    delete this;
}

void GLUtil::updateMatrices()
{
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
}

void GLUtil::printMatrices()
{
    updateMatrices();

    printf( "printing modelview matrix:\n[" );
    int c = 0;
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", modelview[c%16] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
        if ( c >= 16 ) { c=(c+5)%16; }
        else c+=4;
    }
    c = 0;

    /*printf( "printing modelview matrix row-major (wrong)\n[" );
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", modelview[i] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
    }
    c=0;*/

    printf( "printing projection matrix\n[" );
    for ( int i = 0; i < 16; i++ )
    {
        printf( "%f", projection[c%16] );
        if ( i % 4 == 3 )
            printf( "]\n[" );
        else
            printf( " " );
        c += 4;
        if ( c >= 16 ) { c=(c+5)%16; }
        else c+=4;
    }
    c = 0;

    printf( "printing viewport matrix\n[" );
    for ( int i = 0; i < 4; i++ )
    {
        printf( "%i", viewport[i] );
        if ( i % 2 == 1 ) printf( "]\n" );
        else if ( i == 1 ) printf( "[" );
        else printf( " " );
        if ( c >= 4 ) { c=c%4;c++; }
        else c+=2;
    }
}

void GLUtil::worldToScreen( GLdouble x, GLdouble y, GLdouble z,
                                GLdouble* scrX, GLdouble* scrY, GLdouble* scrZ )
{
    updateMatrices();

    GLint ret = gluProject( x, y, z, modelview, projection, viewport,
                            scrX, scrY, scrZ );
    if ( ret == 0 )
        printf( "gluproject returned false\n" );
}

void GLUtil::worldToScreen( Point worldPoint, Point& screenPoint )
{
    GLdouble screenX, screenY, screenZ;
    worldToScreen( (GLdouble)worldPoint.getX(), (GLdouble)worldPoint.getY(),
            (GLdouble)worldPoint.getZ(), &screenX, &screenY, &screenZ );
    screenPoint.setX( (float)screenX );
    screenPoint.setY( (float)screenY );
    screenPoint.setZ( (float)screenZ );
}

void GLUtil::screenToWorld( GLdouble scrX, GLdouble scrY, GLdouble scrZ,
                                GLdouble* x, GLdouble* y, GLdouble* z )
{
    updateMatrices();

    gluUnProject( scrX, scrY, scrZ, modelview, projection, viewport,
                    x, y, z );
}

void GLUtil::screenToWorld( Point screenPoint, Point& worldPoint )
{
    GLdouble worldX, worldY, worldZ;
    screenToWorld( (GLdouble)screenPoint.getX(), (GLdouble)screenPoint.getY(),
            (GLdouble)screenPoint.getZ(), &worldX, &worldY, &worldZ );
    worldPoint.setX( (float)worldX );
    worldPoint.setY( (float)worldY );
    worldPoint.setZ( (float)worldZ );
}

bool GLUtil::screenToRectIntersect( GLdouble x, GLdouble y, RectangleBase rect,
                                        Point& intersect )
{
    Point nearScreen( x, y, 0.0f );
    Point farScreen( x, y, 0.5f );
    Point near, far;
    GLUtil::getInstance()->screenToWorld( nearScreen, near );
    GLUtil::getInstance()->screenToWorld( farScreen, far );

    Ray r;
    r.location = near;
    r.direction = far - near;
    return rect.findRayIntersect( r, intersect );
}

GLuint GLUtil::loadShaders( const char* location )
{
    /*std::string vertLoc;
    std::string fragLoc;

    vertLoc = std::string(location) + std::string(".vert");
    fragLoc = std::string(location) + std::string(".frag");*/

    GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
    GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    glShaderSource( vertexShader, 1, (const GLchar**)&vert420, NULL );
    glShaderSource( fragmentShader, 1, (const GLchar**)&frag420, NULL );

    glCompileShader( vertexShader );

    // get error info for compiling vertex shader
    GLint vertexCompiled = 0;
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vertexCompiled );

    GLint logLength; GLint numWritten;
    glGetShaderiv( vertexShader, GL_INFO_LOG_LENGTH, &logLength );
    char* message = new char[logLength];
    glGetShaderInfoLog( vertexShader, logLength, &numWritten, message );
    printf( "GLUtil::loadShaders: vertex compilation info log (%i): %s\n",
                numWritten, message );
    delete message;

    glCompileShader( fragmentShader );

    // get error info for compiling fragment shader
    GLint fragmentCompiled = 0;
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &fragmentCompiled );

    glGetShaderiv( fragmentShader, GL_INFO_LOG_LENGTH, &logLength );
    message = new char[logLength];
    glGetShaderInfoLog( fragmentShader, logLength, &numWritten, message );
    printf( "GLUtil::loadShaders: fragment compilation info log (%i): %s\n",
                numWritten, message );
    delete message;

    if ( vertexCompiled == 0 || fragmentCompiled == 0 )
    {
        printf( "GLUtil::loadShaders: shaders failed to compile\n" );
        return 0;
    }
    printf( "GLUtil::loadShaders: shaders compiled (vertex: %i, fragment: %i); "
            "linking...\n", vertexCompiled, fragmentCompiled );

    // create program & attempt to link
    GLuint program = glCreateProgram();

    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );

    glLinkProgram( program );
    GLint linked = 0;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );

    glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );
    message = new char[logLength];
    glGetProgramInfoLog( program, logLength, &numWritten, message );
    printf( "GLUtil::loadShaders: program info log (%i): %s\n",
                numWritten, message );
    delete message;

    if ( linked == 0 )
    {
        printf( "GLUtil::loadShaders: shaders failed to link\n" );
        return 0;
    }
    printf( "GLUtil::loadShaders: linked (status: %i), returning program %i\n",
                linked, program );

    return program;
}

GLuint GLUtil::getYUV420Program()
{
    return YUV420Program;
}

GLuint GLUtil::getYUV420xOffsetID()
{
    return YUV420xOffsetID;
}

GLuint GLUtil::getYUV420yOffsetID()
{
    return YUV420yOffsetID;
}

GLuint GLUtil::getYUV420alphaID()
{
    return YUV420alphaID;
}

FTFont* GLUtil::getMainFont()
{
    return mainFont;
}

bool GLUtil::areShadersAvailable()
{
    return shadersAvailable;
}

void GLUtil::setShaderEnable( bool es )
{
    enableShaders = es;
}

void GLUtil::setBufferFontUsage( bool buf )
{
    useBufferFont = buf;
}

GLUtil::GLUtil()
{
    enableShaders = false;
    useBufferFont = false;

    frag420 =
    "uniform sampler2D texture;\n"
    "uniform float alpha;\n"
    "\n"
    "varying vec2 yCoord;\n"
    "varying vec2 uCoord;\n"
    "varying vec2 vCoord;\n"
    "\n"
    "void main( void )\n"
    "{\n"
    "    float y = texture2D( texture, yCoord ).r;\n"
    "    float u = texture2D( texture, uCoord ).r;\n"
    "    float v = texture2D( texture, vCoord ).r;\n"
    "\n"
    "    float cb = u - 0.5;\n"
    "    float cr = v - 0.5;\n"
    "\n"
    "    gl_FragColor = vec4( y + (cr*1.3874),\n"
    "                         y - (cb*0.7109) - (cr*0.3438),\n"
    "                         y + (cb*1.7734),\n"
    "                         alpha );\n"
    "}\n";

    vert420 =
    "uniform float xOffset;\n"
    "uniform float yOffset;\n"
    "\n"
    "varying vec2 yCoord;\n"
    "varying vec2 uCoord;\n"
    "varying vec2 vCoord;\n"
    "\n"
    "void main( void )\n"
    "{\n"
    "    yCoord.s = gl_MultiTexCoord0.s;\n"
    "    yCoord.t = yOffset - gl_MultiTexCoord0.t;\n"
    "\n"
    "    uCoord.s = (gl_MultiTexCoord0.s/2.0);\n"
    "    uCoord.t = (3.0*yOffset/2.0) - (gl_MultiTexCoord0.t/2.0);\n"
    "\n"
    "    vCoord.s = uCoord.s + xOffset/2.0;\n"
    "    vCoord.t = uCoord.t;\n"
    "\n"
    "    gl_Position = ftransform();\n"
    "}\n";
}

GLUtil::~GLUtil()
{
    delete mainFont;
}
