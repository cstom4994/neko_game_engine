

/*=============================
// R_IMPL
=============================*/

#include "engine/neko_engine.h"
#include "engine/neko_platform.h"

#ifdef NEKO_GL

#if defined(NEKO_PF_WIN)
static HMODULE libGL;
NEKO_API_DECL void* neko_gl_get_proc_address(const char* name) {
    void* proc = wglGetProcAddress(name);
    if (!proc) {
        proc = GetProcAddress(libGL, name);
    }
    return proc;
}
#elif defined(NEKO_PF_LINUX)
// #include <GL/glx.h>
#include <dlfcn.h>
static void* libGL;
extern "C" void (*glXGetProcAddress(const GLubyte* procname))(void);
NEKO_API_DECL void* neko_gl_get_proc_address(const char* name) { return (void*)glXGetProcAddress((const GLubyte*)name); }
#elif defined(NEKO_PF_APPLE)
#include <dlfcn.h>
static void* libGL;
NEKO_API_DECL void* neko_gl_get_proc_address(const char* name) { return (void*)dlsym(libGL, name); }
#else
#error neko_gl_get_proc_address() not yet implemented for this platform
#endif

PFNGLACTIVESHADERPROGRAMPROC glActiveShaderProgram = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLBEGINCONDITIONALRENDERPROC glBeginConditionalRender = NULL;
PFNGLBEGINQUERYPROC glBeginQuery = NULL;
PFNGLBEGINQUERYINDEXEDPROC glBeginQueryIndexed = NULL;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback = NULL;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = NULL;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = NULL;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation = NULL;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glBindFragDataLocationIndexed = NULL;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = NULL;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = NULL;
PFNGLBINDSAMPLERPROC glBindSampler = NULL;
PFNGLBINDTEXTUREPROC glBindTexture = NULL;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLBLENDCOLORPROC glBlendColor = NULL;
PFNGLBLENDEQUATIONPROC glBlendEquation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = NULL;
PFNGLBLENDEQUATIONSEPARATEIPROC glBlendEquationSeparatei = NULL;
PFNGLBLENDEQUATIONIPROC glBlendEquationi = NULL;
PFNGLBLENDFUNCPROC glBlendFunc = NULL;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = NULL;
PFNGLBLENDFUNCSEPARATEIPROC glBlendFuncSeparatei = NULL;
PFNGLBLENDFUNCIPROC glBlendFunci = NULL;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;
PFNGLCLAMPCOLORPROC glClampColor = NULL;
PFNGLCLEARPROC glClear = NULL;
PFNGLCLEARBUFFERFIPROC glClearBufferfi = NULL;
PFNGLCLEARBUFFERFVPROC glClearBufferfv = NULL;
PFNGLCLEARBUFFERIVPROC glClearBufferiv = NULL;
PFNGLCLEARBUFFERUIVPROC glClearBufferuiv = NULL;
PFNGLCLEARCOLORPROC glClearColor = NULL;
PFNGLCLEARDEPTHPROC glClearDepth = NULL;
PFNGLCLEARDEPTHFPROC glClearDepthf = NULL;
PFNGLCLEARSTENCILPROC glClearStencil = NULL;
PFNGLCLIENTWAITSYNCPROC glClientWaitSync = NULL;
PFNGLCOLORMASKPROC glColorMask = NULL;
PFNGLCOLORMASKIPROC glColorMaski = NULL;
// PFNGLCOLORP3UIPROC glColorP3ui = NULL;
// PFNGLCOLORP3UIVPROC glColorP3uiv = NULL;
// PFNGLCOLORP4UIPROC glColorP4ui = NULL;
// PFNGLCOLORP4UIVPROC glColorP4uiv = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D = NULL;
PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData = NULL;
PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D = NULL;
PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = NULL;
PFNGLCULLFACEPROC glCullFace = NULL;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = NULL;
PFNGLDELETEQUERIESPROC glDeleteQueries = NULL;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = NULL;
PFNGLDELETESAMPLERSPROC glDeleteSamplers = NULL;
PFNGLDELETESHADERPROC glDeleteShader = NULL;
PFNGLDELETESYNCPROC glDeleteSync = NULL;
PFNGLDELETETEXTURESPROC glDeleteTextures = NULL;
PFNGLDELETETRANSFORMFEEDBACKSPROC glDeleteTransformFeedbacks = NULL;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
PFNGLDEPTHFUNCPROC glDepthFunc = NULL;
PFNGLDEPTHMASKPROC glDepthMask = NULL;
PFNGLDEPTHRANGEPROC glDepthRange = NULL;
PFNGLDEPTHRANGEARRAYVPROC glDepthRangeArrayv = NULL;
PFNGLDEPTHRANGEINDEXEDPROC glDepthRangeIndexed = NULL;
PFNGLDEPTHRANGEFPROC glDepthRangef = NULL;
PFNGLDETACHSHADERPROC glDetachShader = NULL;
PFNGLDISABLEPROC glDisable = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLDISABLEIPROC glDisablei = NULL;
PFNGLDRAWARRAYSPROC glDrawArrays = NULL;
PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirect = NULL;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced = NULL;
PFNGLDRAWBUFFERPROC glDrawBuffer = NULL;
PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
PFNGLDRAWELEMENTSPROC glDrawElements = NULL;
PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex = NULL;
PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirect = NULL;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex = NULL;
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glDrawRangeElementsBaseVertex = NULL;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback = NULL;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glDrawTransformFeedbackStream = NULL;
PFNGLENABLEPROC glEnable = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLENABLEIPROC glEnablei = NULL;
PFNGLENDCONDITIONALRENDERPROC glEndConditionalRender = NULL;
PFNGLENDQUERYPROC glEndQuery = NULL;
PFNGLENDQUERYINDEXEDPROC glEndQueryIndexed = NULL;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback = NULL;
PFNGLFENCESYNCPROC glFenceSync = NULL;
PFNGLFINISHPROC glFinish = NULL;
PFNGLFLUSHPROC glFlush = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = NULL;
PFNGLFRAMEBUFFERTEXTURE1DPROC glFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE3DPROC glFramebufferTexture3D = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = NULL;
PFNGLFRONTFACEPROC glFrontFace = NULL;
PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = NULL;
PFNGLGENQUERIESPROC glGenQueries = NULL;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = NULL;
PFNGLGENSAMPLERSPROC glGenSamplers = NULL;
PFNGLGENTEXTURESPROC glGenTextures = NULL;
PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = NULL;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = NULL;
PFNGLGETACTIVESUBROUTINENAMEPROC glGetActiveSubroutineName = NULL;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glGetActiveSubroutineUniformName = NULL;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glGetActiveSubroutineUniformiv = NULL;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = NULL;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glGetActiveUniformBlockName = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = NULL;
PFNGLGETACTIVEUNIFORMNAMEPROC glGetActiveUniformName = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC glGetActiveUniformsiv = NULL;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = NULL;
PFNGLGETBOOLEANI_VPROC glGetBooleani_v = NULL;
PFNGLGETBOOLEANVPROC glGetBooleanv = NULL;
PFNGLGETBUFFERPARAMETERI64VPROC glGetBufferParameteri64v = NULL;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv = NULL;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage = NULL;
PFNGLGETDOUBLEI_VPROC glGetDoublei_v = NULL;
PFNGLGETDOUBLEVPROC glGetDoublev = NULL;
PFNGLGETERRORPROC glGetError = NULL;
PFNGLGETFLOATI_VPROC glGetFloati_v = NULL;
PFNGLGETFLOATVPROC glGetFloatv = NULL;
PFNGLGETFRAGDATAINDEXPROC glGetFragDataIndex = NULL;
PFNGLGETFRAGDATALOCATIONPROC glGetFragDataLocation = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETINTEGER64I_VPROC glGetInteger64i_v = NULL;
PFNGLGETINTEGER64VPROC glGetInteger64v = NULL;
PFNGLGETINTEGERI_VPROC glGetIntegeri_v = NULL;
PFNGLGETINTEGERVPROC glGetIntegerv = NULL;
PFNGLGETMULTISAMPLEFVPROC glGetMultisamplefv = NULL;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glGetProgramPipelineInfoLog = NULL;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv = NULL;
PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETQUERYINDEXEDIVPROC glGetQueryIndexediv = NULL;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v = NULL;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v = NULL;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = NULL;
PFNGLGETQUERYIVPROC glGetQueryiv = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = NULL;
PFNGLGETSAMPLERPARAMETERIIVPROC glGetSamplerParameterIiv = NULL;
PFNGLGETSAMPLERPARAMETERIUIVPROC glGetSamplerParameterIuiv = NULL;
PFNGLGETSAMPLERPARAMETERFVPROC glGetSamplerParameterfv = NULL;
PFNGLGETSAMPLERPARAMETERIVPROC glGetSamplerParameteriv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat = NULL;
PFNGLGETSHADERSOURCEPROC glGetShaderSource = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSTRINGPROC glGetString = NULL;
PFNGLGETSTRINGIPROC glGetStringi = NULL;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex = NULL;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation = NULL;
PFNGLGETSYNCIVPROC glGetSynciv = NULL;
PFNGLGETTEXIMAGEPROC glGetTexImage = NULL;
PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv = NULL;
PFNGLGETTEXPARAMETERIIVPROC glGetTexParameterIiv = NULL;
PFNGLGETTEXPARAMETERIUIVPROC glGetTexParameterIuiv = NULL;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glGetTransformFeedbackVarying = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = NULL;
PFNGLGETUNIFORMINDICESPROC glGetUniformIndices = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLGETUNIFORMSUBROUTINEUIVPROC glGetUniformSubroutineuiv = NULL;
PFNGLGETUNIFORMDVPROC glGetUniformdv = NULL;
PFNGLGETUNIFORMFVPROC glGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC glGetUniformiv = NULL;
PFNGLGETUNIFORMUIVPROC glGetUniformuiv = NULL;
PFNGLGETVERTEXATTRIBIIVPROC glGetVertexAttribIiv = NULL;
PFNGLGETVERTEXATTRIBIUIVPROC glGetVertexAttribIuiv = NULL;
PFNGLGETVERTEXATTRIBLDVPROC glGetVertexAttribLdv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = NULL;
PFNGLHINTPROC glHint = NULL;
PFNGLISBUFFERPROC glIsBuffer = NULL;
PFNGLISENABLEDPROC glIsEnabled = NULL;
PFNGLISENABLEDIPROC glIsEnabledi = NULL;
PFNGLISFRAMEBUFFERPROC glIsFramebuffer = NULL;
PFNGLISPROGRAMPROC glIsProgram = NULL;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline = NULL;
PFNGLISQUERYPROC glIsQuery = NULL;
PFNGLISRENDERBUFFERPROC glIsRenderbuffer = NULL;
PFNGLISSAMPLERPROC glIsSampler = NULL;
PFNGLISSHADERPROC glIsShader = NULL;
PFNGLISSYNCPROC glIsSync = NULL;
PFNGLISTEXTUREPROC glIsTexture = NULL;
PFNGLISTRANSFORMFEEDBACKPROC glIsTransformFeedback = NULL;
PFNGLISVERTEXARRAYPROC glIsVertexArray = NULL;
PFNGLLINEWIDTHPROC glLineWidth = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLLOGICOPPROC glLogicOp = NULL;
PFNGLMAPBUFFERPROC glMapBuffer = NULL;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = NULL;
PFNGLMINSAMPLESHADINGPROC glMinSampleShading = NULL;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays = NULL;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glMultiDrawElementsBaseVertex = NULL;
// PFNGLMULTITEXCOORDP1UIPROC glMultiTexCoordP1ui = NULL;
// PFNGLMULTITEXCOORDP1UIVPROC glMultiTexCoordP1uiv = NULL;
// PFNGLMULTITEXCOORDP2UIPROC glMultiTexCoordP2ui = NULL;
// PFNGLMULTITEXCOORDP2UIVPROC glMultiTexCoordP2uiv = NULL;
// PFNGLMULTITEXCOORDP3UIPROC glMultiTexCoordP3ui = NULL;
// PFNGLMULTITEXCOORDP3UIVPROC glMultiTexCoordP3uiv = NULL;
// PFNGLMULTITEXCOORDP4UIPROC glMultiTexCoordP4ui = NULL;
// PFNGLMULTITEXCOORDP4UIVPROC glMultiTexCoordP4uiv = NULL;
// PFNGLNORMALP3UIPROC glNormalP3ui = NULL;
// PFNGLNORMALP3UIVPROC glNormalP3uiv = NULL;
PFNGLPATCHPARAMETERFVPROC glPatchParameterfv = NULL;
PFNGLPATCHPARAMETERIPROC glPatchParameteri = NULL;
PFNGLPAUSETRANSFORMFEEDBACKPROC glPauseTransformFeedback = NULL;
PFNGLPIXELSTOREFPROC glPixelStoref = NULL;
PFNGLPIXELSTOREIPROC glPixelStorei = NULL;
PFNGLPOINTPARAMETERFPROC glPointParameterf = NULL;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv = NULL;
PFNGLPOINTPARAMETERIPROC glPointParameteri = NULL;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv = NULL;
PFNGLPOINTSIZEPROC glPointSize = NULL;
PFNGLPOLYGONMODEPROC glPolygonMode = NULL;
PFNGLPOLYGONOFFSETPROC glPolygonOffset = NULL;
PFNGLPRIMITIVERESTARTINDEXPROC glPrimitiveRestartIndex = NULL;
PFNGLPROGRAMBINARYPROC glProgramBinary = NULL;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = NULL;
PFNGLPROGRAMUNIFORM1DPROC glProgramUniform1d = NULL;
PFNGLPROGRAMUNIFORM1DVPROC glProgramUniform1dv = NULL;
PFNGLPROGRAMUNIFORM1FPROC glProgramUniform1f = NULL;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv = NULL;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i = NULL;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv = NULL;
PFNGLPROGRAMUNIFORM1UIPROC glProgramUniform1ui = NULL;
PFNGLPROGRAMUNIFORM1UIVPROC glProgramUniform1uiv = NULL;
PFNGLPROGRAMUNIFORM2DPROC glProgramUniform2d = NULL;
PFNGLPROGRAMUNIFORM2DVPROC glProgramUniform2dv = NULL;
PFNGLPROGRAMUNIFORM2FPROC glProgramUniform2f = NULL;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv = NULL;
PFNGLPROGRAMUNIFORM2IPROC glProgramUniform2i = NULL;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv = NULL;
PFNGLPROGRAMUNIFORM2UIPROC glProgramUniform2ui = NULL;
PFNGLPROGRAMUNIFORM2UIVPROC glProgramUniform2uiv = NULL;
PFNGLPROGRAMUNIFORM3DPROC glProgramUniform3d = NULL;
PFNGLPROGRAMUNIFORM3DVPROC glProgramUniform3dv = NULL;
PFNGLPROGRAMUNIFORM3FPROC glProgramUniform3f = NULL;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv = NULL;
PFNGLPROGRAMUNIFORM3IPROC glProgramUniform3i = NULL;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv = NULL;
PFNGLPROGRAMUNIFORM3UIPROC glProgramUniform3ui = NULL;
PFNGLPROGRAMUNIFORM3UIVPROC glProgramUniform3uiv = NULL;
PFNGLPROGRAMUNIFORM4DPROC glProgramUniform4d = NULL;
PFNGLPROGRAMUNIFORM4DVPROC glProgramUniform4dv = NULL;
PFNGLPROGRAMUNIFORM4FPROC glProgramUniform4f = NULL;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv = NULL;
PFNGLPROGRAMUNIFORM4IPROC glProgramUniform4i = NULL;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv = NULL;
PFNGLPROGRAMUNIFORM4UIPROC glProgramUniform4ui = NULL;
PFNGLPROGRAMUNIFORM4UIVPROC glProgramUniform4uiv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glProgramUniformMatrix2dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glProgramUniformMatrix2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glProgramUniformMatrix2x3dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glProgramUniformMatrix2x3fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glProgramUniformMatrix2x4dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glProgramUniformMatrix2x4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glProgramUniformMatrix3dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glProgramUniformMatrix3fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glProgramUniformMatrix3x2dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glProgramUniformMatrix3x2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glProgramUniformMatrix3x4dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glProgramUniformMatrix3x4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glProgramUniformMatrix4dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glProgramUniformMatrix4x2dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glProgramUniformMatrix4x2fv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glProgramUniformMatrix4x3dv = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glProgramUniformMatrix4x3fv = NULL;
PFNGLPROVOKINGVERTEXPROC glProvokingVertex = NULL;
PFNGLQUERYCOUNTERPROC glQueryCounter = NULL;
PFNGLREADBUFFERPROC glReadBuffer = NULL;
PFNGLREADPIXELSPROC glReadPixels = NULL;
PFNGLRELEASESHADERCOMPILERPROC glReleaseShaderCompiler = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = NULL;
PFNGLRESUMETRANSFORMFEEDBACKPROC glResumeTransformFeedback = NULL;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = NULL;
PFNGLSAMPLEMASKIPROC glSampleMaski = NULL;
PFNGLSAMPLERPARAMETERIIVPROC glSamplerParameterIiv = NULL;
PFNGLSAMPLERPARAMETERIUIVPROC glSamplerParameterIuiv = NULL;
PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf = NULL;
PFNGLSAMPLERPARAMETERFVPROC glSamplerParameterfv = NULL;
PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri = NULL;
PFNGLSAMPLERPARAMETERIVPROC glSamplerParameteriv = NULL;
PFNGLSCISSORPROC glScissor = NULL;
PFNGLSCISSORARRAYVPROC glScissorArrayv = NULL;
PFNGLSCISSORINDEXEDPROC glScissorIndexed = NULL;
PFNGLSCISSORINDEXEDVPROC glScissorIndexedv = NULL;
// PFNGLSECONDARYCOLORP3UIPROC glSecondaryColorP3ui = NULL;
// PFNGLSECONDARYCOLORP3UIVPROC glSecondaryColorP3uiv = NULL;
PFNGLSHADERBINARYPROC glShaderBinary = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLSTENCILFUNCPROC glStencilFunc = NULL;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = NULL;
PFNGLSTENCILMASKPROC glStencilMask = NULL;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = NULL;
PFNGLSTENCILOPPROC glStencilOp = NULL;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = NULL;
PFNGLTEXBUFFERPROC glTexBuffer = NULL;
// PFNGLTEXCOORDP1UIPROC glTexCoordP1ui = NULL;
// PFNGLTEXCOORDP1UIVPROC glTexCoordP1uiv = NULL;
// PFNGLTEXCOORDP2UIPROC glTexCoordP2ui = NULL;
// PFNGLTEXCOORDP2UIVPROC glTexCoordP2uiv = NULL;
// PFNGLTEXCOORDP3UIPROC glTexCoordP3ui = NULL;
// PFNGLTEXCOORDP3UIVPROC glTexCoordP3uiv = NULL;
// PFNGLTEXCOORDP4UIPROC glTexCoordP4ui = NULL;
// PFNGLTEXCOORDP4UIVPROC glTexCoordP4uiv = NULL;
PFNGLTEXIMAGE1DPROC glTexImage1D = NULL;
PFNGLTEXIMAGE2DPROC glTexImage2D = NULL;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = NULL;
PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glTexImage3DMultisample = NULL;
PFNGLTEXPARAMETERIIVPROC glTexParameterIiv = NULL;
PFNGLTEXPARAMETERIUIVPROC glTexParameterIuiv = NULL;
PFNGLTEXPARAMETERFPROC glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC glTexParameteriv = NULL;
PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D = NULL;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D = NULL;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings = NULL;
PFNGLUNIFORM1DPROC glUniform1d = NULL;
PFNGLUNIFORM1DVPROC glUniform1dv = NULL;
PFNGLUNIFORM1FPROC glUniform1f = NULL;
PFNGLUNIFORM1FVPROC glUniform1fv = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLUNIFORM1IVPROC glUniform1iv = NULL;
PFNGLUNIFORM1UIPROC glUniform1ui = NULL;
PFNGLUNIFORM1UIVPROC glUniform1uiv = NULL;
PFNGLUNIFORM2DPROC glUniform2d = NULL;
PFNGLUNIFORM2DVPROC glUniform2dv = NULL;
PFNGLUNIFORM2FPROC glUniform2f = NULL;
PFNGLUNIFORM2FVPROC glUniform2fv = NULL;
PFNGLUNIFORM2IPROC glUniform2i = NULL;
PFNGLUNIFORM2IVPROC glUniform2iv = NULL;
PFNGLUNIFORM2UIPROC glUniform2ui = NULL;
PFNGLUNIFORM2UIVPROC glUniform2uiv = NULL;
PFNGLUNIFORM3DPROC glUniform3d = NULL;
PFNGLUNIFORM3DVPROC glUniform3dv = NULL;
PFNGLUNIFORM3FPROC glUniform3f = NULL;
PFNGLUNIFORM3FVPROC glUniform3fv = NULL;
PFNGLUNIFORM3IPROC glUniform3i = NULL;
PFNGLUNIFORM3IVPROC glUniform3iv = NULL;
PFNGLUNIFORM3UIPROC glUniform3ui = NULL;
PFNGLUNIFORM3UIVPROC glUniform3uiv = NULL;
PFNGLUNIFORM4DPROC glUniform4d = NULL;
PFNGLUNIFORM4DVPROC glUniform4dv = NULL;
PFNGLUNIFORM4FPROC glUniform4f = NULL;
PFNGLUNIFORM4FVPROC glUniform4fv = NULL;
PFNGLUNIFORM4IPROC glUniform4i = NULL;
PFNGLUNIFORM4IVPROC glUniform4iv = NULL;
PFNGLUNIFORM4UIPROC glUniform4ui = NULL;
PFNGLUNIFORM4UIVPROC glUniform4uiv = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = NULL;
PFNGLUNIFORMMATRIX2DVPROC glUniformMatrix2dv = NULL;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX2X3DVPROC glUniformMatrix2x3dv = NULL;
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4DVPROC glUniformMatrix2x4dv = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3DVPROC glUniformMatrix3dv = NULL;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX3X2DVPROC glUniformMatrix3x2dv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4DVPROC glUniformMatrix3x4dv = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4DVPROC glUniformMatrix4dv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
PFNGLUNIFORMMATRIX4X2DVPROC glUniformMatrix4x2dv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3DVPROC glUniformMatrix4x3dv = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv = NULL;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv = NULL;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = NULL;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = NULL;
PFNGLVALIDATEPROGRAMPIPELINEPROC glValidateProgramPipeline = NULL;
PFNGLVERTEXATTRIB1DPROC glVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DVPROC glVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1SPROC glVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SVPROC glVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2DPROC glVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DVPROC glVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2SPROC glVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SVPROC glVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3DPROC glVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DVPROC glVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3SPROC glVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SVPROC glVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBVPROC glVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIVPROC glVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSVPROC glVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUBPROC glVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBVPROC glVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIVPROC glVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSVPROC glVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BVPROC glVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4DPROC glVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DVPROC glVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IVPROC glVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4SPROC glVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SVPROC glVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBVPROC glVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIVPROC glVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USVPROC glVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor = NULL;
PFNGLVERTEXATTRIBI1IPROC glVertexAttribI1i = NULL;
PFNGLVERTEXATTRIBI1IVPROC glVertexAttribI1iv = NULL;
PFNGLVERTEXATTRIBI1UIPROC glVertexAttribI1ui = NULL;
PFNGLVERTEXATTRIBI1UIVPROC glVertexAttribI1uiv = NULL;
PFNGLVERTEXATTRIBI2IPROC glVertexAttribI2i = NULL;
PFNGLVERTEXATTRIBI2IVPROC glVertexAttribI2iv = NULL;
PFNGLVERTEXATTRIBI2UIPROC glVertexAttribI2ui = NULL;
PFNGLVERTEXATTRIBI2UIVPROC glVertexAttribI2uiv = NULL;
PFNGLVERTEXATTRIBI3IPROC glVertexAttribI3i = NULL;
PFNGLVERTEXATTRIBI3IVPROC glVertexAttribI3iv = NULL;
PFNGLVERTEXATTRIBI3UIPROC glVertexAttribI3ui = NULL;
PFNGLVERTEXATTRIBI3UIVPROC glVertexAttribI3uiv = NULL;
PFNGLVERTEXATTRIBI4BVPROC glVertexAttribI4bv = NULL;
PFNGLVERTEXATTRIBI4IPROC glVertexAttribI4i = NULL;
PFNGLVERTEXATTRIBI4IVPROC glVertexAttribI4iv = NULL;
PFNGLVERTEXATTRIBI4SVPROC glVertexAttribI4sv = NULL;
PFNGLVERTEXATTRIBI4UBVPROC glVertexAttribI4ubv = NULL;
PFNGLVERTEXATTRIBI4UIPROC glVertexAttribI4ui = NULL;
PFNGLVERTEXATTRIBI4UIVPROC glVertexAttribI4uiv = NULL;
PFNGLVERTEXATTRIBI4USVPROC glVertexAttribI4usv = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer = NULL;
PFNGLVERTEXATTRIBL1DPROC glVertexAttribL1d = NULL;
PFNGLVERTEXATTRIBL1DVPROC glVertexAttribL1dv = NULL;
PFNGLVERTEXATTRIBL2DPROC glVertexAttribL2d = NULL;
PFNGLVERTEXATTRIBL2DVPROC glVertexAttribL2dv = NULL;
PFNGLVERTEXATTRIBL3DPROC glVertexAttribL3d = NULL;
PFNGLVERTEXATTRIBL3DVPROC glVertexAttribL3dv = NULL;
PFNGLVERTEXATTRIBL4DPROC glVertexAttribL4d = NULL;
PFNGLVERTEXATTRIBL4DVPROC glVertexAttribL4dv = NULL;
PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer = NULL;
PFNGLVERTEXATTRIBP1UIPROC glVertexAttribP1ui = NULL;
PFNGLVERTEXATTRIBP1UIVPROC glVertexAttribP1uiv = NULL;
PFNGLVERTEXATTRIBP2UIPROC glVertexAttribP2ui = NULL;
PFNGLVERTEXATTRIBP2UIVPROC glVertexAttribP2uiv = NULL;
PFNGLVERTEXATTRIBP3UIPROC glVertexAttribP3ui = NULL;
PFNGLVERTEXATTRIBP3UIVPROC glVertexAttribP3uiv = NULL;
PFNGLVERTEXATTRIBP4UIPROC glVertexAttribP4ui = NULL;
PFNGLVERTEXATTRIBP4UIVPROC glVertexAttribP4uiv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
// PFNGLVERTEXP2UIPROC glVertexP2ui = NULL;
// PFNGLVERTEXP2UIVPROC glVertexP2uiv = NULL;
// PFNGLVERTEXP3UIPROC glVertexP3ui = NULL;
// PFNGLVERTEXP3UIVPROC glVertexP3uiv = NULL;
// PFNGLVERTEXP4UIPROC glVertexP4ui = NULL;
// PFNGLVERTEXP4UIVPROC glVertexP4uiv = NULL;
PFNGLVIEWPORTPROC glViewport = NULL;
PFNGLVIEWPORTARRAYVPROC glViewportArrayv = NULL;
PFNGLVIEWPORTINDEXEDFPROC glViewportIndexedf = NULL;
PFNGLVIEWPORTINDEXEDFVPROC glViewportIndexedfv = NULL;
PFNGLWAITSYNCPROC glWaitSync = NULL;
PFNGLBUFFERSTORAGEPROC glBufferStorage = NULL;
PFNGLMEMORYBARRIERPROC glMemoryBarrier = NULL;

NEKO_API_DECL void neko_gl_load_all(void) {

#if defined(NEKO_PF_WIN)
    libGL = LoadLibrary(L"opengl32.dll");
#else
#ifdef __APPLE__
    // clang-format off
    static const char *NAMES[] = {
        "../Frameworks/OpenGL.framework/OpenGL",
        "/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL"
    };
    // clang-format on
#else
    static const char* NAMES[] = {"libGL.so.1", "libGL.so"};
#endif

    unsigned int index = 0;
    for (index = 0; index < (sizeof(NAMES) / sizeof(NAMES[0])); index++) {
        libGL = dlopen(NAMES[index], RTLD_NOW | RTLD_GLOBAL);
        if (libGL != NULL) {
            break;
        }
    }
#endif

    glCullFace = (PFNGLCULLFACEPROC)neko_gl_get_proc_address("glCullFace");
    glFrontFace = (PFNGLFRONTFACEPROC)neko_gl_get_proc_address("glFrontFace");
    glHint = (PFNGLHINTPROC)neko_gl_get_proc_address("glHint");
    glLineWidth = (PFNGLLINEWIDTHPROC)neko_gl_get_proc_address("glLineWidth");
    glPointSize = (PFNGLPOINTSIZEPROC)neko_gl_get_proc_address("glPointSize");
    glPolygonMode = (PFNGLPOLYGONMODEPROC)neko_gl_get_proc_address("glPolygonMode");
    glScissor = (PFNGLSCISSORPROC)neko_gl_get_proc_address("glScissor");
    glTexParameterf = (PFNGLTEXPARAMETERFPROC)neko_gl_get_proc_address("glTexParameterf");
    glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)neko_gl_get_proc_address("glTexParameterfv");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)neko_gl_get_proc_address("glTexParameteri");
    glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)neko_gl_get_proc_address("glTexParameteriv");
    glTexImage1D = (PFNGLTEXIMAGE1DPROC)neko_gl_get_proc_address("glTexImage1D");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)neko_gl_get_proc_address("glTexImage2D");
    glDrawBuffer = (PFNGLDRAWBUFFERPROC)neko_gl_get_proc_address("glDrawBuffer");
    glClear = (PFNGLCLEARPROC)neko_gl_get_proc_address("glClear");
    glClearColor = (PFNGLCLEARCOLORPROC)neko_gl_get_proc_address("glClearColor");
    glClearStencil = (PFNGLCLEARSTENCILPROC)neko_gl_get_proc_address("glClearStencil");
    glClearDepth = (PFNGLCLEARDEPTHPROC)neko_gl_get_proc_address("glClearDepth");
    glStencilMask = (PFNGLSTENCILMASKPROC)neko_gl_get_proc_address("glStencilMask");
    glColorMask = (PFNGLCOLORMASKPROC)neko_gl_get_proc_address("glColorMask");
    glDepthMask = (PFNGLDEPTHMASKPROC)neko_gl_get_proc_address("glDepthMask");
    glDisable = (PFNGLDISABLEPROC)neko_gl_get_proc_address("glDisable");
    glEnable = (PFNGLENABLEPROC)neko_gl_get_proc_address("glEnable");
    glFinish = (PFNGLFINISHPROC)neko_gl_get_proc_address("glFinish");
    glFlush = (PFNGLFLUSHPROC)neko_gl_get_proc_address("glFlush");
    glBlendFunc = (PFNGLBLENDFUNCPROC)neko_gl_get_proc_address("glBlendFunc");
    glLogicOp = (PFNGLLOGICOPPROC)neko_gl_get_proc_address("glLogicOp");
    glStencilFunc = (PFNGLSTENCILFUNCPROC)neko_gl_get_proc_address("glStencilFunc");
    glStencilOp = (PFNGLSTENCILOPPROC)neko_gl_get_proc_address("glStencilOp");
    glDepthFunc = (PFNGLDEPTHFUNCPROC)neko_gl_get_proc_address("glDepthFunc");
    glPixelStoref = (PFNGLPIXELSTOREFPROC)neko_gl_get_proc_address("glPixelStoref");
    glPixelStorei = (PFNGLPIXELSTOREIPROC)neko_gl_get_proc_address("glPixelStorei");
    glReadBuffer = (PFNGLREADBUFFERPROC)neko_gl_get_proc_address("glReadBuffer");
    glReadPixels = (PFNGLREADPIXELSPROC)neko_gl_get_proc_address("glReadPixels");
    glGetBooleanv = (PFNGLGETBOOLEANVPROC)neko_gl_get_proc_address("glGetBooleanv");
    glGetDoublev = (PFNGLGETDOUBLEVPROC)neko_gl_get_proc_address("glGetDoublev");
    glGetError = (PFNGLGETERRORPROC)neko_gl_get_proc_address("glGetError");
    glGetFloatv = (PFNGLGETFLOATVPROC)neko_gl_get_proc_address("glGetFloatv");
    glGetIntegerv = (PFNGLGETINTEGERVPROC)neko_gl_get_proc_address("glGetIntegerv");
    glGetString = (PFNGLGETSTRINGPROC)neko_gl_get_proc_address("glGetString");
    glGetTexImage = (PFNGLGETTEXIMAGEPROC)neko_gl_get_proc_address("glGetTexImage");
    glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)neko_gl_get_proc_address("glGetTexParameterfv");
    glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)neko_gl_get_proc_address("glGetTexParameteriv");
    glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)neko_gl_get_proc_address("glGetTexLevelParameterfv");
    glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)neko_gl_get_proc_address("glGetTexLevelParameteriv");
    glIsEnabled = (PFNGLISENABLEDPROC)neko_gl_get_proc_address("glIsEnabled");
    glDepthRange = (PFNGLDEPTHRANGEPROC)neko_gl_get_proc_address("glDepthRange");
    glViewport = (PFNGLVIEWPORTPROC)neko_gl_get_proc_address("glViewport");
    glDrawArrays = (PFNGLDRAWARRAYSPROC)neko_gl_get_proc_address("glDrawArrays");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)neko_gl_get_proc_address("glDrawElements");
    // glGetPointerv = (PFNGLGETPOINTERVPROC)neko_gl_get_proc_address("glGetPointerv");
    glPolygonOffset = (PFNGLPOLYGONOFFSETPROC)neko_gl_get_proc_address("glPolygonOffset");
    glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC)neko_gl_get_proc_address("glCopyTexImage1D");
    glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)neko_gl_get_proc_address("glCopyTexImage2D");
    glCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC)neko_gl_get_proc_address("glCopyTexSubImage1D");
    glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)neko_gl_get_proc_address("glCopyTexSubImage2D");
    glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)neko_gl_get_proc_address("glTexSubImage1D");
    glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)neko_gl_get_proc_address("glTexSubImage2D");
    glBindTexture = (PFNGLBINDTEXTUREPROC)neko_gl_get_proc_address("glBindTexture");
    glDeleteTextures = (PFNGLDELETETEXTURESPROC)neko_gl_get_proc_address("glDeleteTextures");
    glGenTextures = (PFNGLGENTEXTURESPROC)neko_gl_get_proc_address("glGenTextures");
    glIsTexture = (PFNGLISTEXTUREPROC)neko_gl_get_proc_address("glIsTexture");
    glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)neko_gl_get_proc_address("glDrawRangeElements");
    glTexImage3D = (PFNGLTEXIMAGE3DPROC)neko_gl_get_proc_address("glTexImage3D");
    glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)neko_gl_get_proc_address("glTexSubImage3D");
    glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)neko_gl_get_proc_address("glCopyTexSubImage3D");
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)neko_gl_get_proc_address("glActiveTexture");
    glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)neko_gl_get_proc_address("glSampleCoverage");
    glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)neko_gl_get_proc_address("glCompressedTexImage3D");
    glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)neko_gl_get_proc_address("glCompressedTexImage2D");
    glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)neko_gl_get_proc_address("glCompressedTexImage1D");
    glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)neko_gl_get_proc_address("glCompressedTexSubImage3D");
    glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)neko_gl_get_proc_address("glCompressedTexSubImage2D");
    glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)neko_gl_get_proc_address("glCompressedTexSubImage1D");
    glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)neko_gl_get_proc_address("glGetCompressedTexImage");
    glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)neko_gl_get_proc_address("glBlendFuncSeparate");
    glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)neko_gl_get_proc_address("glMultiDrawArrays");
    glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)neko_gl_get_proc_address("glMultiDrawElements");
    glPointParameterf = (PFNGLPOINTPARAMETERFPROC)neko_gl_get_proc_address("glPointParameterf");
    glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)neko_gl_get_proc_address("glPointParameterfv");
    glPointParameteri = (PFNGLPOINTPARAMETERIPROC)neko_gl_get_proc_address("glPointParameteri");
    glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC)neko_gl_get_proc_address("glPointParameteriv");
    glBlendColor = (PFNGLBLENDCOLORPROC)neko_gl_get_proc_address("glBlendColor");
    glBlendEquation = (PFNGLBLENDEQUATIONPROC)neko_gl_get_proc_address("glBlendEquation");
    glGenQueries = (PFNGLGENQUERIESPROC)neko_gl_get_proc_address("glGenQueries");
    glDeleteQueries = (PFNGLDELETEQUERIESPROC)neko_gl_get_proc_address("glDeleteQueries");
    glIsQuery = (PFNGLISQUERYPROC)neko_gl_get_proc_address("glIsQuery");
    glBeginQuery = (PFNGLBEGINQUERYPROC)neko_gl_get_proc_address("glBeginQuery");
    glEndQuery = (PFNGLENDQUERYPROC)neko_gl_get_proc_address("glEndQuery");
    glGetQueryiv = (PFNGLGETQUERYIVPROC)neko_gl_get_proc_address("glGetQueryiv");
    glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)neko_gl_get_proc_address("glGetQueryObjectiv");
    glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)neko_gl_get_proc_address("glGetQueryObjectuiv");
    glBindBuffer = (PFNGLBINDBUFFERPROC)neko_gl_get_proc_address("glBindBuffer");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)neko_gl_get_proc_address("glDeleteBuffers");
    glGenBuffers = (PFNGLGENBUFFERSPROC)neko_gl_get_proc_address("glGenBuffers");
    glIsBuffer = (PFNGLISBUFFERPROC)neko_gl_get_proc_address("glIsBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)neko_gl_get_proc_address("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)neko_gl_get_proc_address("glBufferSubData");
    glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)neko_gl_get_proc_address("glGetBufferSubData");
    glMapBuffer = (PFNGLMAPBUFFERPROC)neko_gl_get_proc_address("glMapBuffer");
    glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)neko_gl_get_proc_address("glUnmapBuffer");
    glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)neko_gl_get_proc_address("glGetBufferParameteriv");
    glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)neko_gl_get_proc_address("glGetBufferPointerv");
    glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)neko_gl_get_proc_address("glBlendEquationSeparate");
    glDrawBuffers = (PFNGLDRAWBUFFERSPROC)neko_gl_get_proc_address("glDrawBuffers");
    glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)neko_gl_get_proc_address("glStencilOpSeparate");
    glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)neko_gl_get_proc_address("glStencilFuncSeparate");
    glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)neko_gl_get_proc_address("glStencilMaskSeparate");
    glAttachShader = (PFNGLATTACHSHADERPROC)neko_gl_get_proc_address("glAttachShader");
    glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)neko_gl_get_proc_address("glBindAttribLocation");
    glCompileShader = (PFNGLCOMPILESHADERPROC)neko_gl_get_proc_address("glCompileShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)neko_gl_get_proc_address("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)neko_gl_get_proc_address("glCreateShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)neko_gl_get_proc_address("glDeleteProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)neko_gl_get_proc_address("glDeleteShader");
    glDetachShader = (PFNGLDETACHSHADERPROC)neko_gl_get_proc_address("glDetachShader");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)neko_gl_get_proc_address("glDisableVertexAttribArray");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)neko_gl_get_proc_address("glEnableVertexAttribArray");
    glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)neko_gl_get_proc_address("glGetActiveAttrib");
    glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)neko_gl_get_proc_address("glGetActiveUniform");
    glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)neko_gl_get_proc_address("glGetAttachedShaders");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)neko_gl_get_proc_address("glGetAttribLocation");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)neko_gl_get_proc_address("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)neko_gl_get_proc_address("glGetProgramInfoLog");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)neko_gl_get_proc_address("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)neko_gl_get_proc_address("glGetShaderInfoLog");
    glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)neko_gl_get_proc_address("glGetShaderSource");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)neko_gl_get_proc_address("glGetUniformLocation");
    glGetUniformfv = (PFNGLGETUNIFORMFVPROC)neko_gl_get_proc_address("glGetUniformfv");
    glGetUniformiv = (PFNGLGETUNIFORMIVPROC)neko_gl_get_proc_address("glGetUniformiv");
    glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)neko_gl_get_proc_address("glGetVertexAttribdv");
    glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)neko_gl_get_proc_address("glGetVertexAttribfv");
    glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)neko_gl_get_proc_address("glGetVertexAttribiv");
    glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)neko_gl_get_proc_address("glGetVertexAttribPointerv");
    glIsProgram = (PFNGLISPROGRAMPROC)neko_gl_get_proc_address("glIsProgram");
    glIsShader = (PFNGLISSHADERPROC)neko_gl_get_proc_address("glIsShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)neko_gl_get_proc_address("glLinkProgram");
    glShaderSource = (PFNGLSHADERSOURCEPROC)neko_gl_get_proc_address("glShaderSource");
    glUseProgram = (PFNGLUSEPROGRAMPROC)neko_gl_get_proc_address("glUseProgram");
    glUniform1f = (PFNGLUNIFORM1FPROC)neko_gl_get_proc_address("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)neko_gl_get_proc_address("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)neko_gl_get_proc_address("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)neko_gl_get_proc_address("glUniform4f");
    glUniform1i = (PFNGLUNIFORM1IPROC)neko_gl_get_proc_address("glUniform1i");
    glUniform2i = (PFNGLUNIFORM2IPROC)neko_gl_get_proc_address("glUniform2i");
    glUniform3i = (PFNGLUNIFORM3IPROC)neko_gl_get_proc_address("glUniform3i");
    glUniform4i = (PFNGLUNIFORM4IPROC)neko_gl_get_proc_address("glUniform4i");
    glUniform1fv = (PFNGLUNIFORM1FVPROC)neko_gl_get_proc_address("glUniform1fv");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)neko_gl_get_proc_address("glUniform2fv");
    glUniform3fv = (PFNGLUNIFORM3FVPROC)neko_gl_get_proc_address("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)neko_gl_get_proc_address("glUniform4fv");
    glUniform1iv = (PFNGLUNIFORM1IVPROC)neko_gl_get_proc_address("glUniform1iv");
    glUniform2iv = (PFNGLUNIFORM2IVPROC)neko_gl_get_proc_address("glUniform2iv");
    glUniform3iv = (PFNGLUNIFORM3IVPROC)neko_gl_get_proc_address("glUniform3iv");
    glUniform4iv = (PFNGLUNIFORM4IVPROC)neko_gl_get_proc_address("glUniform4iv");
    glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)neko_gl_get_proc_address("glUniformMatrix2fv");
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)neko_gl_get_proc_address("glUniformMatrix3fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)neko_gl_get_proc_address("glUniformMatrix4fv");
    glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)neko_gl_get_proc_address("glValidateProgram");
    glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)neko_gl_get_proc_address("glVertexAttrib1d");
    glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)neko_gl_get_proc_address("glVertexAttrib1dv");
    glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)neko_gl_get_proc_address("glVertexAttrib1f");
    glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)neko_gl_get_proc_address("glVertexAttrib1fv");
    glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)neko_gl_get_proc_address("glVertexAttrib1s");
    glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)neko_gl_get_proc_address("glVertexAttrib1sv");
    glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)neko_gl_get_proc_address("glVertexAttrib2d");
    glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)neko_gl_get_proc_address("glVertexAttrib2dv");
    glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)neko_gl_get_proc_address("glVertexAttrib2f");
    glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)neko_gl_get_proc_address("glVertexAttrib2fv");
    glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)neko_gl_get_proc_address("glVertexAttrib2s");
    glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)neko_gl_get_proc_address("glVertexAttrib2sv");
    glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)neko_gl_get_proc_address("glVertexAttrib3d");
    glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)neko_gl_get_proc_address("glVertexAttrib3dv");
    glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)neko_gl_get_proc_address("glVertexAttrib3f");
    glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)neko_gl_get_proc_address("glVertexAttrib3fv");
    glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)neko_gl_get_proc_address("glVertexAttrib3s");
    glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)neko_gl_get_proc_address("glVertexAttrib3sv");
    glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)neko_gl_get_proc_address("glVertexAttrib4Nbv");
    glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)neko_gl_get_proc_address("glVertexAttrib4Niv");
    glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)neko_gl_get_proc_address("glVertexAttrib4Nsv");
    glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)neko_gl_get_proc_address("glVertexAttrib4Nub");
    glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)neko_gl_get_proc_address("glVertexAttrib4Nubv");
    glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)neko_gl_get_proc_address("glVertexAttrib4Nuiv");
    glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)neko_gl_get_proc_address("glVertexAttrib4Nusv");
    glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)neko_gl_get_proc_address("glVertexAttrib4bv");
    glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)neko_gl_get_proc_address("glVertexAttrib4d");
    glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)neko_gl_get_proc_address("glVertexAttrib4dv");
    glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)neko_gl_get_proc_address("glVertexAttrib4f");
    glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)neko_gl_get_proc_address("glVertexAttrib4fv");
    glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)neko_gl_get_proc_address("glVertexAttrib4iv");
    glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)neko_gl_get_proc_address("glVertexAttrib4s");
    glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)neko_gl_get_proc_address("glVertexAttrib4sv");
    glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)neko_gl_get_proc_address("glVertexAttrib4ubv");
    glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)neko_gl_get_proc_address("glVertexAttrib4uiv");
    glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)neko_gl_get_proc_address("glVertexAttrib4usv");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)neko_gl_get_proc_address("glVertexAttribPointer");
    glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)neko_gl_get_proc_address("glUniformMatrix2x3fv");
    glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)neko_gl_get_proc_address("glUniformMatrix3x2fv");
    glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)neko_gl_get_proc_address("glUniformMatrix2x4fv");
    glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)neko_gl_get_proc_address("glUniformMatrix4x2fv");
    glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)neko_gl_get_proc_address("glUniformMatrix3x4fv");
    glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)neko_gl_get_proc_address("glUniformMatrix4x3fv");
    glColorMaski = (PFNGLCOLORMASKIPROC)neko_gl_get_proc_address("glColorMaski");
    glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)neko_gl_get_proc_address("glGetBooleani_v");
    glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)neko_gl_get_proc_address("glGetIntegeri_v");
    glEnablei = (PFNGLENABLEIPROC)neko_gl_get_proc_address("glEnablei");
    glDisablei = (PFNGLDISABLEIPROC)neko_gl_get_proc_address("glDisablei");
    glIsEnabledi = (PFNGLISENABLEDIPROC)neko_gl_get_proc_address("glIsEnabledi");
    glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glBeginTransformFeedback");
    glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glEndTransformFeedback");
    glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)neko_gl_get_proc_address("glBindBufferRange");
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)neko_gl_get_proc_address("glBindBufferBase");
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)neko_gl_get_proc_address("glTransformFeedbackVaryings");
    glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)neko_gl_get_proc_address("glGetTransformFeedbackVarying");
    glClampColor = (PFNGLCLAMPCOLORPROC)neko_gl_get_proc_address("glClampColor");
    glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC)neko_gl_get_proc_address("glBeginConditionalRender");
    glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC)neko_gl_get_proc_address("glEndConditionalRender");
    glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)neko_gl_get_proc_address("glVertexAttribIPointer");
    glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)neko_gl_get_proc_address("glGetVertexAttribIiv");
    glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)neko_gl_get_proc_address("glGetVertexAttribIuiv");
    glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)neko_gl_get_proc_address("glVertexAttribI1i");
    glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)neko_gl_get_proc_address("glVertexAttribI2i");
    glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)neko_gl_get_proc_address("glVertexAttribI3i");
    glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)neko_gl_get_proc_address("glVertexAttribI4i");
    glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC)neko_gl_get_proc_address("glVertexAttribI1ui");
    glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC)neko_gl_get_proc_address("glVertexAttribI2ui");
    glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC)neko_gl_get_proc_address("glVertexAttribI3ui");
    glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)neko_gl_get_proc_address("glVertexAttribI4ui");
    glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC)neko_gl_get_proc_address("glVertexAttribI1iv");
    glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC)neko_gl_get_proc_address("glVertexAttribI2iv");
    glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC)neko_gl_get_proc_address("glVertexAttribI3iv");
    glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)neko_gl_get_proc_address("glVertexAttribI4iv");
    glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC)neko_gl_get_proc_address("glVertexAttribI1uiv");
    glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC)neko_gl_get_proc_address("glVertexAttribI2uiv");
    glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC)neko_gl_get_proc_address("glVertexAttribI3uiv");
    glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)neko_gl_get_proc_address("glVertexAttribI4uiv");
    glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC)neko_gl_get_proc_address("glVertexAttribI4bv");
    glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC)neko_gl_get_proc_address("glVertexAttribI4sv");
    glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC)neko_gl_get_proc_address("glVertexAttribI4ubv");
    glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC)neko_gl_get_proc_address("glVertexAttribI4usv");
    glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)neko_gl_get_proc_address("glGetUniformuiv");
    glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)neko_gl_get_proc_address("glBindFragDataLocation");
    glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)neko_gl_get_proc_address("glGetFragDataLocation");
    glUniform1ui = (PFNGLUNIFORM1UIPROC)neko_gl_get_proc_address("glUniform1ui");
    glUniform2ui = (PFNGLUNIFORM2UIPROC)neko_gl_get_proc_address("glUniform2ui");
    glUniform3ui = (PFNGLUNIFORM3UIPROC)neko_gl_get_proc_address("glUniform3ui");
    glUniform4ui = (PFNGLUNIFORM4UIPROC)neko_gl_get_proc_address("glUniform4ui");
    glUniform1uiv = (PFNGLUNIFORM1UIVPROC)neko_gl_get_proc_address("glUniform1uiv");
    glUniform2uiv = (PFNGLUNIFORM2UIVPROC)neko_gl_get_proc_address("glUniform2uiv");
    glUniform3uiv = (PFNGLUNIFORM3UIVPROC)neko_gl_get_proc_address("glUniform3uiv");
    glUniform4uiv = (PFNGLUNIFORM4UIVPROC)neko_gl_get_proc_address("glUniform4uiv");
    glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)neko_gl_get_proc_address("glTexParameterIiv");
    glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)neko_gl_get_proc_address("glTexParameterIuiv");
    glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)neko_gl_get_proc_address("glGetTexParameterIiv");
    glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)neko_gl_get_proc_address("glGetTexParameterIuiv");
    glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)neko_gl_get_proc_address("glClearBufferiv");
    glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)neko_gl_get_proc_address("glClearBufferuiv");
    glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)neko_gl_get_proc_address("glClearBufferfv");
    glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)neko_gl_get_proc_address("glClearBufferfi");
    glGetStringi = (PFNGLGETSTRINGIPROC)neko_gl_get_proc_address("glGetStringi");
    glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)neko_gl_get_proc_address("glIsRenderbuffer");
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)neko_gl_get_proc_address("glBindRenderbuffer");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)neko_gl_get_proc_address("glDeleteRenderbuffers");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)neko_gl_get_proc_address("glGenRenderbuffers");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)neko_gl_get_proc_address("glRenderbufferStorage");
    glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)neko_gl_get_proc_address("glGetRenderbufferParameteriv");
    glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)neko_gl_get_proc_address("glIsFramebuffer");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)neko_gl_get_proc_address("glBindFramebuffer");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)neko_gl_get_proc_address("glDeleteFramebuffers");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)neko_gl_get_proc_address("glGenFramebuffers");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)neko_gl_get_proc_address("glCheckFramebufferStatus");
    glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)neko_gl_get_proc_address("glFramebufferTexture1D");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)neko_gl_get_proc_address("glFramebufferTexture2D");
    glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)neko_gl_get_proc_address("glFramebufferTexture3D");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)neko_gl_get_proc_address("glFramebufferRenderbuffer");
    glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)neko_gl_get_proc_address("glGetFramebufferAttachmentParameteriv");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)neko_gl_get_proc_address("glGenerateMipmap");
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)neko_gl_get_proc_address("glBlitFramebuffer");
    glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)neko_gl_get_proc_address("glRenderbufferStorageMultisample");
    glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)neko_gl_get_proc_address("glFramebufferTextureLayer");
    glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)neko_gl_get_proc_address("glMapBufferRange");
    glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)neko_gl_get_proc_address("glFlushMappedBufferRange");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)neko_gl_get_proc_address("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)neko_gl_get_proc_address("glDeleteVertexArrays");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)neko_gl_get_proc_address("glGenVertexArrays");
    glIsVertexArray = (PFNGLISVERTEXARRAYPROC)neko_gl_get_proc_address("glIsVertexArray");
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)neko_gl_get_proc_address("glDrawArraysInstanced");
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)neko_gl_get_proc_address("glDrawElementsInstanced");
    glTexBuffer = (PFNGLTEXBUFFERPROC)neko_gl_get_proc_address("glTexBuffer");
    glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC)neko_gl_get_proc_address("glPrimitiveRestartIndex");
    glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)neko_gl_get_proc_address("glCopyBufferSubData");
    glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)neko_gl_get_proc_address("glGetUniformIndices");
    glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)neko_gl_get_proc_address("glGetActiveUniformsiv");
    glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)neko_gl_get_proc_address("glGetActiveUniformName");
    glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)neko_gl_get_proc_address("glGetUniformBlockIndex");
    glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)neko_gl_get_proc_address("glGetActiveUniformBlockiv");
    glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)neko_gl_get_proc_address("glGetActiveUniformBlockName");
    glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)neko_gl_get_proc_address("glUniformBlockBinding");
    glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)neko_gl_get_proc_address("glDrawElementsBaseVertex");
    glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)neko_gl_get_proc_address("glDrawRangeElementsBaseVertex");
    glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)neko_gl_get_proc_address("glDrawElementsInstancedBaseVertex");
    glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)neko_gl_get_proc_address("glMultiDrawElementsBaseVertex");
    glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)neko_gl_get_proc_address("glProvokingVertex");
    glFenceSync = (PFNGLFENCESYNCPROC)neko_gl_get_proc_address("glFenceSync");
    glIsSync = (PFNGLISSYNCPROC)neko_gl_get_proc_address("glIsSync");
    glDeleteSync = (PFNGLDELETESYNCPROC)neko_gl_get_proc_address("glDeleteSync");
    glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)neko_gl_get_proc_address("glClientWaitSync");
    glWaitSync = (PFNGLWAITSYNCPROC)neko_gl_get_proc_address("glWaitSync");
    glGetInteger64v = (PFNGLGETINTEGER64VPROC)neko_gl_get_proc_address("glGetInteger64v");
    glGetSynciv = (PFNGLGETSYNCIVPROC)neko_gl_get_proc_address("glGetSynciv");
    glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)neko_gl_get_proc_address("glGetInteger64i_v");
    glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)neko_gl_get_proc_address("glGetBufferParameteri64v");
    glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)neko_gl_get_proc_address("glFramebufferTexture");
    glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)neko_gl_get_proc_address("glTexImage2DMultisample");
    glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)neko_gl_get_proc_address("glTexImage3DMultisample");
    glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)neko_gl_get_proc_address("glGetMultisamplefv");
    glSampleMaski = (PFNGLSAMPLEMASKIPROC)neko_gl_get_proc_address("glSampleMaski");
    glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)neko_gl_get_proc_address("glBindFragDataLocationIndexed");
    glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC)neko_gl_get_proc_address("glGetFragDataIndex");
    glGenSamplers = (PFNGLGENSAMPLERSPROC)neko_gl_get_proc_address("glGenSamplers");
    glDeleteSamplers = (PFNGLDELETESAMPLERSPROC)neko_gl_get_proc_address("glDeleteSamplers");
    glIsSampler = (PFNGLISSAMPLERPROC)neko_gl_get_proc_address("glIsSampler");
    glBindSampler = (PFNGLBINDSAMPLERPROC)neko_gl_get_proc_address("glBindSampler");
    glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)neko_gl_get_proc_address("glSamplerParameteri");
    glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)neko_gl_get_proc_address("glSamplerParameteriv");
    glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)neko_gl_get_proc_address("glSamplerParameterf");
    glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)neko_gl_get_proc_address("glSamplerParameterfv");
    glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC)neko_gl_get_proc_address("glSamplerParameterIiv");
    glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC)neko_gl_get_proc_address("glSamplerParameterIuiv");
    glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)neko_gl_get_proc_address("glGetSamplerParameteriv");
    glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC)neko_gl_get_proc_address("glGetSamplerParameterIiv");
    glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)neko_gl_get_proc_address("glGetSamplerParameterfv");
    glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC)neko_gl_get_proc_address("glGetSamplerParameterIuiv");
    glQueryCounter = (PFNGLQUERYCOUNTERPROC)neko_gl_get_proc_address("glQueryCounter");
    glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC)neko_gl_get_proc_address("glGetQueryObjecti64v");
    glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)neko_gl_get_proc_address("glGetQueryObjectui64v");
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)neko_gl_get_proc_address("glVertexAttribDivisor");
    glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC)neko_gl_get_proc_address("glVertexAttribP1ui");
    glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC)neko_gl_get_proc_address("glVertexAttribP1uiv");
    glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC)neko_gl_get_proc_address("glVertexAttribP2ui");
    glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC)neko_gl_get_proc_address("glVertexAttribP2uiv");
    glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC)neko_gl_get_proc_address("glVertexAttribP3ui");
    glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC)neko_gl_get_proc_address("glVertexAttribP3uiv");
    glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC)neko_gl_get_proc_address("glVertexAttribP4ui");
    glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC)neko_gl_get_proc_address("glVertexAttribP4uiv");

    glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC)neko_gl_get_proc_address("glMinSampleShading");
    glBlendEquationi = (PFNGLBLENDEQUATIONIPROC)neko_gl_get_proc_address("glBlendEquationi");
    glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC)neko_gl_get_proc_address("glBlendEquationSeparatei");
    glBlendFunci = (PFNGLBLENDFUNCIPROC)neko_gl_get_proc_address("glBlendFunci");
    glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC)neko_gl_get_proc_address("glBlendFuncSeparatei");
    glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)neko_gl_get_proc_address("glDrawArraysIndirect");
    glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)neko_gl_get_proc_address("glDrawElementsIndirect");
    glUniform1d = (PFNGLUNIFORM1DPROC)neko_gl_get_proc_address("glUniform1d");
    glUniform2d = (PFNGLUNIFORM2DPROC)neko_gl_get_proc_address("glUniform2d");
    glUniform3d = (PFNGLUNIFORM3DPROC)neko_gl_get_proc_address("glUniform3d");
    glUniform4d = (PFNGLUNIFORM4DPROC)neko_gl_get_proc_address("glUniform4d");
    glUniform1dv = (PFNGLUNIFORM1DVPROC)neko_gl_get_proc_address("glUniform1dv");
    glUniform2dv = (PFNGLUNIFORM2DVPROC)neko_gl_get_proc_address("glUniform2dv");
    glUniform3dv = (PFNGLUNIFORM3DVPROC)neko_gl_get_proc_address("glUniform3dv");
    glUniform4dv = (PFNGLUNIFORM4DVPROC)neko_gl_get_proc_address("glUniform4dv");
    glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC)neko_gl_get_proc_address("glUniformMatrix2dv");
    glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC)neko_gl_get_proc_address("glUniformMatrix3dv");
    glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC)neko_gl_get_proc_address("glUniformMatrix4dv");
    glUniformMatrix2x3dv = (PFNGLUNIFORMMATRIX2X3DVPROC)neko_gl_get_proc_address("glUniformMatrix2x3dv");
    glUniformMatrix2x4dv = (PFNGLUNIFORMMATRIX2X4DVPROC)neko_gl_get_proc_address("glUniformMatrix2x4dv");
    glUniformMatrix3x2dv = (PFNGLUNIFORMMATRIX3X2DVPROC)neko_gl_get_proc_address("glUniformMatrix3x2dv");
    glUniformMatrix3x4dv = (PFNGLUNIFORMMATRIX3X4DVPROC)neko_gl_get_proc_address("glUniformMatrix3x4dv");
    glUniformMatrix4x2dv = (PFNGLUNIFORMMATRIX4X2DVPROC)neko_gl_get_proc_address("glUniformMatrix4x2dv");
    glUniformMatrix4x3dv = (PFNGLUNIFORMMATRIX4X3DVPROC)neko_gl_get_proc_address("glUniformMatrix4x3dv");
    glGetUniformdv = (PFNGLGETUNIFORMDVPROC)neko_gl_get_proc_address("glGetUniformdv");
    glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)neko_gl_get_proc_address("glGetSubroutineUniformLocation");
    glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC)neko_gl_get_proc_address("glGetSubroutineIndex");
    glGetActiveSubroutineUniformiv = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)neko_gl_get_proc_address("glGetActiveSubroutineUniformiv");
    glGetActiveSubroutineUniformName = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)neko_gl_get_proc_address("glGetActiveSubroutineUniformName");
    glGetActiveSubroutineName = (PFNGLGETACTIVESUBROUTINENAMEPROC)neko_gl_get_proc_address("glGetActiveSubroutineName");
    glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC)neko_gl_get_proc_address("glUniformSubroutinesuiv");
    glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)neko_gl_get_proc_address("glGetUniformSubroutineuiv");
    glGetProgramStageiv = (PFNGLGETPROGRAMSTAGEIVPROC)neko_gl_get_proc_address("glGetProgramStageiv");
    glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)neko_gl_get_proc_address("glPatchParameteri");
    glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)neko_gl_get_proc_address("glPatchParameterfv");
    glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glBindTransformFeedback");
    glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)neko_gl_get_proc_address("glDeleteTransformFeedbacks");
    glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)neko_gl_get_proc_address("glGenTransformFeedbacks");
    glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glIsTransformFeedback");
    glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glPauseTransformFeedback");
    glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glResumeTransformFeedback");
    glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)neko_gl_get_proc_address("glDrawTransformFeedback");
    glDrawTransformFeedbackStream = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)neko_gl_get_proc_address("glDrawTransformFeedbackStream");
    glBeginQueryIndexed = (PFNGLBEGINQUERYINDEXEDPROC)neko_gl_get_proc_address("glBeginQueryIndexed");
    glEndQueryIndexed = (PFNGLENDQUERYINDEXEDPROC)neko_gl_get_proc_address("glEndQueryIndexed");
    glGetQueryIndexediv = (PFNGLGETQUERYINDEXEDIVPROC)neko_gl_get_proc_address("glGetQueryIndexediv");

    glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)neko_gl_get_proc_address("glReleaseShaderCompiler");
    glShaderBinary = (PFNGLSHADERBINARYPROC)neko_gl_get_proc_address("glShaderBinary");
    glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)neko_gl_get_proc_address("glGetShaderPrecisionFormat");
    glDepthRangef = (PFNGLDEPTHRANGEFPROC)neko_gl_get_proc_address("glDepthRangef");
    glClearDepthf = (PFNGLCLEARDEPTHFPROC)neko_gl_get_proc_address("glClearDepthf");
    glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)neko_gl_get_proc_address("glGetProgramBinary");
    glProgramBinary = (PFNGLPROGRAMBINARYPROC)neko_gl_get_proc_address("glProgramBinary");
    glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)neko_gl_get_proc_address("glProgramParameteri");
    glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)neko_gl_get_proc_address("glUseProgramStages");
    glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)neko_gl_get_proc_address("glActiveShaderProgram");
    glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)neko_gl_get_proc_address("glCreateShaderProgramv");
    glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)neko_gl_get_proc_address("glBindProgramPipeline");
    glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)neko_gl_get_proc_address("glDeleteProgramPipelines");
    glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)neko_gl_get_proc_address("glGenProgramPipelines");
    glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)neko_gl_get_proc_address("glIsProgramPipeline");
    glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)neko_gl_get_proc_address("glGetProgramPipelineiv");
    glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)neko_gl_get_proc_address("glProgramParameteri");
    glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)neko_gl_get_proc_address("glProgramUniform1i");
    glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)neko_gl_get_proc_address("glProgramUniform1iv");
    glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)neko_gl_get_proc_address("glProgramUniform1f");
    glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)neko_gl_get_proc_address("glProgramUniform1fv");
    glProgramUniform1d = (PFNGLPROGRAMUNIFORM1DPROC)neko_gl_get_proc_address("glProgramUniform1d");
    glProgramUniform1dv = (PFNGLPROGRAMUNIFORM1DVPROC)neko_gl_get_proc_address("glProgramUniform1dv");
    glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)neko_gl_get_proc_address("glProgramUniform1ui");
    glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)neko_gl_get_proc_address("glProgramUniform1uiv");
    glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)neko_gl_get_proc_address("glProgramUniform2i");
    glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)neko_gl_get_proc_address("glProgramUniform2iv");
    glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)neko_gl_get_proc_address("glProgramUniform2f");
    glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)neko_gl_get_proc_address("glProgramUniform2fv");
    glProgramUniform2d = (PFNGLPROGRAMUNIFORM2DPROC)neko_gl_get_proc_address("glProgramUniform2d");
    glProgramUniform2dv = (PFNGLPROGRAMUNIFORM2DVPROC)neko_gl_get_proc_address("glProgramUniform2dv");
    glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)neko_gl_get_proc_address("glProgramUniform2ui");
    glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)neko_gl_get_proc_address("glProgramUniform2uiv");
    glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)neko_gl_get_proc_address("glProgramUniform3i");
    glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)neko_gl_get_proc_address("glProgramUniform3iv");
    glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)neko_gl_get_proc_address("glProgramUniform3f");
    glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)neko_gl_get_proc_address("glProgramUniform3fv");
    glProgramUniform3d = (PFNGLPROGRAMUNIFORM3DPROC)neko_gl_get_proc_address("glProgramUniform3d");
    glProgramUniform3dv = (PFNGLPROGRAMUNIFORM3DVPROC)neko_gl_get_proc_address("glProgramUniform3dv");
    glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)neko_gl_get_proc_address("glProgramUniform3ui");
    glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)neko_gl_get_proc_address("glProgramUniform3uiv");
    glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)neko_gl_get_proc_address("glProgramUniform4i");
    glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)neko_gl_get_proc_address("glProgramUniform4iv");
    glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)neko_gl_get_proc_address("glProgramUniform4f");
    glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)neko_gl_get_proc_address("glProgramUniform4fv");
    glProgramUniform4d = (PFNGLPROGRAMUNIFORM4DPROC)neko_gl_get_proc_address("glProgramUniform4d");
    glProgramUniform4dv = (PFNGLPROGRAMUNIFORM4DVPROC)neko_gl_get_proc_address("glProgramUniform4dv");
    glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)neko_gl_get_proc_address("glProgramUniform4ui");
    glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)neko_gl_get_proc_address("glProgramUniform4uiv");
    glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2fv");
    glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3fv");
    glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4fv");
    glProgramUniformMatrix2dv = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2dv");
    glProgramUniformMatrix3dv = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3dv");
    glProgramUniformMatrix4dv = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4dv");
    glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2x3fv");
    glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3x2fv");
    glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2x4fv");
    glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4x2fv");
    glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3x4fv");
    glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4x3fv");
    glProgramUniformMatrix2x3dv = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2x3dv");
    glProgramUniformMatrix3x2dv = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3x2dv");
    glProgramUniformMatrix2x4dv = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix2x4dv");
    glProgramUniformMatrix4x2dv = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4x2dv");
    glProgramUniformMatrix3x4dv = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix3x4dv");
    glProgramUniformMatrix4x3dv = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)neko_gl_get_proc_address("glProgramUniformMatrix4x3dv");
    glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)neko_gl_get_proc_address("glValidateProgramPipeline");
    glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)neko_gl_get_proc_address("glGetProgramPipelineInfoLog");
    glVertexAttribL1d = (PFNGLVERTEXATTRIBL1DPROC)neko_gl_get_proc_address("glVertexAttribL1d");
    glVertexAttribL2d = (PFNGLVERTEXATTRIBL2DPROC)neko_gl_get_proc_address("glVertexAttribL2d");
    glVertexAttribL3d = (PFNGLVERTEXATTRIBL3DPROC)neko_gl_get_proc_address("glVertexAttribL3d");
    glVertexAttribL4d = (PFNGLVERTEXATTRIBL4DPROC)neko_gl_get_proc_address("glVertexAttribL4d");
    glVertexAttribL1dv = (PFNGLVERTEXATTRIBL1DVPROC)neko_gl_get_proc_address("glVertexAttribL1dv");
    glVertexAttribL2dv = (PFNGLVERTEXATTRIBL2DVPROC)neko_gl_get_proc_address("glVertexAttribL2dv");
    glVertexAttribL3dv = (PFNGLVERTEXATTRIBL3DVPROC)neko_gl_get_proc_address("glVertexAttribL3dv");
    glVertexAttribL4dv = (PFNGLVERTEXATTRIBL4DVPROC)neko_gl_get_proc_address("glVertexAttribL4dv");
    glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC)neko_gl_get_proc_address("glVertexAttribLPointer");
    glGetVertexAttribLdv = (PFNGLGETVERTEXATTRIBLDVPROC)neko_gl_get_proc_address("glGetVertexAttribLdv");
    glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC)neko_gl_get_proc_address("glViewportArrayv");
    glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC)neko_gl_get_proc_address("glViewportIndexedf");
    glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC)neko_gl_get_proc_address("glViewportIndexedfv");
    glScissorArrayv = (PFNGLSCISSORARRAYVPROC)neko_gl_get_proc_address("glScissorArrayv");
    glScissorIndexed = (PFNGLSCISSORINDEXEDPROC)neko_gl_get_proc_address("glScissorIndexed");
    glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC)neko_gl_get_proc_address("glScissorIndexedv");
    glDepthRangeArrayv = (PFNGLDEPTHRANGEARRAYVPROC)neko_gl_get_proc_address("glDepthRangeArrayv");
    glDepthRangeIndexed = (PFNGLDEPTHRANGEINDEXEDPROC)neko_gl_get_proc_address("glDepthRangeIndexed");
    glGetFloati_v = (PFNGLGETFLOATI_VPROC)neko_gl_get_proc_address("glGetFloati_v");
    glGetDoublei_v = (PFNGLGETDOUBLEI_VPROC)neko_gl_get_proc_address("glGetDoublei_v");
    glBufferStorage = (PFNGLBUFFERSTORAGEPROC)neko_gl_get_proc_address("glBufferStorage");
    glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)neko_gl_get_proc_address("glMemoryBarrier");

#if defined(NEKO_PF_WIN)
    if (libGL != NULL) {
        FreeLibrary((HMODULE)libGL);
        libGL = NULL;
    }
#else
    if (libGL != NULL) {
        dlclose(libGL);
        libGL = NULL;
    }
#endif
}

#endif

#if (defined NEKO_PF_WIN || defined NEKO_PF_APPLE || defined NEKO_PF_LINUX)
#define R_IMPL_OPENGL_CORE
#else
#define R_IMPL_OPENGL_ES
#endif

// 图形信息对象查询
neko_render_info_t* neko_render_info() { return &neko_subsystem(render)->info; }

#if (defined R_IMPL_OPENGL_CORE || defined R_IMPL_OPENGL_ES)

// macOS 不支持 OpenGL 4.1+
#if defined(NEKO_PF_APPLE)
#define CHECK_GL_CORE(...)
#else
#define CHECK_GL_CORE(...) __VA_ARGS__
#endif

const_str neko_opengl_string(GLenum e) {

#define XX(x)      \
    case x:        \
        return #x; \
        break;

    switch (e) {
        // shader:
        XX(GL_VERTEX_SHADER);
        XX(GL_GEOMETRY_SHADER);
        XX(GL_FRAGMENT_SHADER);

        // buffer usage:
        XX(GL_STREAM_DRAW);
        XX(GL_STREAM_READ);
        XX(GL_STREAM_COPY);
        XX(GL_STATIC_DRAW);
        XX(GL_STATIC_READ);
        XX(GL_STATIC_COPY);
        XX(GL_DYNAMIC_DRAW);
        XX(GL_DYNAMIC_READ);
        XX(GL_DYNAMIC_COPY);

        // errors:
        XX(GL_NO_ERROR);
        XX(GL_INVALID_ENUM);
        XX(GL_INVALID_VALUE);
        XX(GL_INVALID_OPERATION);
        XX(GL_INVALID_FRAMEBUFFER_OPERATION);
        XX(GL_OUT_OF_MEMORY);

        // #if !defined(NEKO_PF_APPLE)
        //         XX(GL_STACK_UNDERFLOW);
        //         XX(GL_STACK_OVERFLOW);
        // #endif

        // types:
        XX(GL_BYTE);
        XX(GL_UNSIGNED_BYTE);
        XX(GL_SHORT);
        XX(GL_UNSIGNED_SHORT);
        XX(GL_FLOAT);
        XX(GL_FLOAT_VEC2);
        XX(GL_FLOAT_VEC3);
        XX(GL_FLOAT_VEC4);
        XX(GL_DOUBLE);
        XX(GL_DOUBLE_VEC2);
        XX(GL_DOUBLE_VEC3);
        XX(GL_DOUBLE_VEC4);
        XX(GL_INT);
        XX(GL_INT_VEC2);
        XX(GL_INT_VEC3);
        XX(GL_INT_VEC4);
        XX(GL_UNSIGNED_INT);
        XX(GL_UNSIGNED_INT_VEC2);
        XX(GL_UNSIGNED_INT_VEC3);
        XX(GL_UNSIGNED_INT_VEC4);
        XX(GL_BOOL);
        XX(GL_BOOL_VEC2);
        XX(GL_BOOL_VEC3);
        XX(GL_BOOL_VEC4);
        XX(GL_FLOAT_MAT2);
        XX(GL_FLOAT_MAT3);
        XX(GL_FLOAT_MAT4);
        XX(GL_FLOAT_MAT2x3);
        XX(GL_FLOAT_MAT2x4);
        XX(GL_FLOAT_MAT3x2);
        XX(GL_FLOAT_MAT3x4);
        XX(GL_FLOAT_MAT4x2);
        XX(GL_FLOAT_MAT4x3);
        XX(GL_DOUBLE_MAT2);
        XX(GL_DOUBLE_MAT3);
        XX(GL_DOUBLE_MAT4);
        XX(GL_DOUBLE_MAT2x3);
        XX(GL_DOUBLE_MAT2x4);
        XX(GL_DOUBLE_MAT3x2);
        XX(GL_DOUBLE_MAT3x4);
        XX(GL_DOUBLE_MAT4x2);
        XX(GL_DOUBLE_MAT4x3);
        XX(GL_SAMPLER_1D);
        XX(GL_SAMPLER_2D);
        XX(GL_SAMPLER_3D);
        XX(GL_SAMPLER_CUBE);
        XX(GL_SAMPLER_1D_SHADOW);
        XX(GL_SAMPLER_2D_SHADOW);
        XX(GL_SAMPLER_1D_ARRAY);
        XX(GL_SAMPLER_2D_ARRAY);
        XX(GL_SAMPLER_1D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_ARRAY_SHADOW);
        XX(GL_SAMPLER_2D_MULTISAMPLE);
        XX(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_SAMPLER_CUBE_SHADOW);
        XX(GL_SAMPLER_BUFFER);
        XX(GL_SAMPLER_2D_RECT);
        XX(GL_SAMPLER_2D_RECT_SHADOW);
        XX(GL_INT_SAMPLER_1D);
        XX(GL_INT_SAMPLER_2D);
        XX(GL_INT_SAMPLER_3D);
        XX(GL_INT_SAMPLER_CUBE);
        XX(GL_INT_SAMPLER_1D_ARRAY);
        XX(GL_INT_SAMPLER_2D_ARRAY);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_INT_SAMPLER_BUFFER);
        XX(GL_INT_SAMPLER_2D_RECT);
        XX(GL_UNSIGNED_INT_SAMPLER_1D);
        XX(GL_UNSIGNED_INT_SAMPLER_2D);
        XX(GL_UNSIGNED_INT_SAMPLER_3D);
        XX(GL_UNSIGNED_INT_SAMPLER_CUBE);
        XX(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
        XX(GL_UNSIGNED_INT_SAMPLER_BUFFER);
        XX(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

        // #if !defined(NEKO_PF_APPLE)
        //         XX(GL_IMAGE_1D);
        //         XX(GL_IMAGE_2D);
        //         XX(GL_IMAGE_3D);
        //         XX(GL_IMAGE_2D_RECT);
        //         XX(GL_IMAGE_CUBE);
        //         XX(GL_IMAGE_BUFFER);
        //         XX(GL_IMAGE_1D_ARRAY);
        //         XX(GL_IMAGE_2D_ARRAY);
        //         XX(GL_IMAGE_2D_MULTISAMPLE);
        //         XX(GL_IMAGE_2D_MULTISAMPLE_ARRAY);
        //         XX(GL_INT_IMAGE_1D);
        //         XX(GL_INT_IMAGE_2D);
        //         XX(GL_INT_IMAGE_3D);
        //         XX(GL_INT_IMAGE_2D_RECT);
        //         XX(GL_INT_IMAGE_CUBE);
        //         XX(GL_INT_IMAGE_BUFFER);
        //         XX(GL_INT_IMAGE_1D_ARRAY);
        //         XX(GL_INT_IMAGE_2D_ARRAY);
        //         XX(GL_INT_IMAGE_2D_MULTISAMPLE);
        //         XX(GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        //         XX(GL_UNSIGNED_INT_IMAGE_1D);
        //         XX(GL_UNSIGNED_INT_IMAGE_2D);
        //         XX(GL_UNSIGNED_INT_IMAGE_3D);
        //         XX(GL_UNSIGNED_INT_IMAGE_2D_RECT);
        //         XX(GL_UNSIGNED_INT_IMAGE_CUBE);
        //         XX(GL_UNSIGNED_INT_IMAGE_BUFFER);
        //         XX(GL_UNSIGNED_INT_IMAGE_1D_ARRAY);
        //         XX(GL_UNSIGNED_INT_IMAGE_2D_ARRAY);
        //         XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE);
        //         XX(GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY);
        //         XX(GL_UNSIGNED_INT_ATOMIC_COUNTER);
        // #endif
    }

#undef XX

    static char buffer[32];
    sprintf(buffer, "Unknown GLenum: (0x%04x)", e);
    return buffer;
}

const char* gl_get_error_description(GLenum error_code) {
    switch (error_code) {
        case GL_NO_ERROR:
            return "No error detected.";
        case GL_INVALID_ENUM:
            return "Enum argument out of range.";
        case GL_INVALID_VALUE:
            return "Numeric argument out of range.";
        case GL_INVALID_OPERATION:
            return "Operation illegal in current state.";
        case GL_OUT_OF_MEMORY:
            return "Not enough memory left to execute command.";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Framebuffer object is not complete.";
        default:
            return "No description available for UNKNOWN_ERROR.";
    }
}

void neko_render_print_error(const char* file, u32 line) {
    GLenum code = glGetError();

    if (code != GL_NO_ERROR) {
        const_str last_slash = file;

        while (*file) {
            char c = *file;
            if (c == '\\' || c == '/') last_slash = file + 1;
            ++file;
        }

        const_str str = neko_opengl_string(code);
        const_str des = gl_get_error_description(code);
        neko_println("OpenGL Error %s (%u): %u, %s, %s", last_slash, line, code, str, des);
    }
}

struct neko_render_batch_context_t {
    u32 max_draw_calls;
    u32 count;
    neko_render_batch_draw_call_t* calls;
};

neko_render_batch_context_t* neko_render_batch_make_ctx(u32 max_draw_calls) {
    neko_render_batch_context_t* ctx = (neko_render_batch_context_t*)neko_safe_malloc(sizeof(neko_render_batch_context_t));

    ctx->max_draw_calls = max_draw_calls;
    ctx->count = 0;
    ctx->calls = (neko_render_batch_draw_call_t*)neko_safe_malloc(sizeof(neko_render_batch_draw_call_t) * max_draw_calls);
    if (!ctx->calls) {
        neko_safe_free(ctx);
        return 0;
    }
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return ctx;
}

void neko_render_batch_free(void* ctx) {
    neko_render_batch_context_t* context = (neko_render_batch_context_t*)ctx;
    neko_safe_free(context->calls);
    neko_safe_free(context);
}

void neko_render_batch_make_frame_buffer(neko_render_batch_framebuffer_t* fb, neko_render_batch_shader_t* shader, int w, int h, int use_depth_test) {
    // Generate the frame buffer
    GLuint fb_id;
    glGenFramebuffers(1, &fb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);

    // Generate a texture to use as the color buffer.
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach color buffer to frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

    // Generate depth and stencil attachments for the fb using a RenderBuffer.
    GLuint rb_id = (GLuint)~0;
    if (use_depth_test) {
        glGenRenderbuffers(1, &rb_id);
        glBindRenderbuffer(GL_RENDERBUFFER, rb_id);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb_id);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) NEKO_WARN("cute_gl: failed to generate framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Prepare quad
    GLuint quad_id;
    glGenBuffers(1, &quad_id);
    glBindBuffer(GL_ARRAY_BUFFER, quad_id);
    static GLfloat quad[] = {-1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fb->fb_id = fb_id;
    fb->tex_id = tex_id;
    fb->rb_id = rb_id;
    fb->quad_id = quad_id;
    fb->shader = shader;
    fb->w = w;
    fb->h = h;
}

void neko_render_batch_free_frame_buffer(neko_render_batch_framebuffer_t* fb) {
    glDeleteTextures(1, &fb->tex_id);
    glDeleteRenderbuffers(1, &fb->rb_id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fb->fb_id);
    glDeleteBuffers(1, &fb->quad_id);
    memset(fb, 0, sizeof(neko_render_batch_framebuffer_t));
}

void neko_render_batch_make_vertex_data(neko_render_batch_vertex_data_t* vd, u32 buffer_size, u32 primitive, u32 vertex_stride, u32 usage) {
    vd->buffer_size = buffer_size;
    vd->vertex_stride = vertex_stride;
    vd->primitive = primitive;
    vd->usage = usage;
    vd->attribute_count = 0;
}

static u32 gl_get_gl_type_internal(u32 type) {
    switch (type) {
        case GL_INT:
        case GL_INT_VEC2:
        case GL_INT_VEC3:
        case GL_INT_VEC4:
            return R_BATCH_INT;

        case GL_FLOAT:
        case GL_FLOAT_VEC2:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT4:
            return R_BATCH_FLOAT;

        case GL_BOOL:
        case GL_BOOL_VEC2:
        case GL_BOOL_VEC3:
        case GL_BOOL_VEC4:
            return R_BATCH_BOOL;

// seems undefined for GLES
#if GL_SAMPLER_1D
        case GL_SAMPLER_1D:
#endif
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
            return R_BATCH_SAMPLER;

        default:
            return R_BATCH_UNKNOWN;
    }
}

void neko_render_batch_add_attribute(neko_render_batch_vertex_data_t* vd, const char* name, u32 size, u32 type, u32 offset) {
    neko_render_batch_vertex_attribute_t va;
    va.name = name;
    va.hash = neko_hash_str64(name);
    va.size = size;
    va.type = type;
    va.offset = offset;

    NEKO_ASSERT(vd->attribute_count < R_BATCH_ATTRIBUTE_MAX_COUNT);
    vd->attributes[vd->attribute_count++] = va;
}

void neko_render_batch_make_renderable(neko_render_batch_renderable_t* r, neko_render_batch_vertex_data_t* vd) {
    r->data = *vd;
    r->index0 = 0;
    r->index1 = 0;
    r->buffer_number = 0;
    r->need_new_sync = 0;
    r->program = 0;
    r->state.u.key = 0;

    if (vd->usage == GL_STATIC_DRAW) {
        r->buffer_count = 1;
        r->need_new_sync = 1;
    } else
        r->buffer_count = 3;
}

// WARNING: Messes with GL global state via glUnmapBuffer(GL_ARRAY_BUFFER) and
// glBindBuffer(GL_ARRAY_BUFFER, ...), so call gl_map_internal, fill in data, then call gl_unmap_internal.
void* gl_map_internal(neko_render_batch_renderable_t* r, u32 count) {
    // Cannot map a buffer when the buffer is too small
    // Make your buffer is bigger or draw less data
    NEKO_ASSERT(count <= r->data.buffer_size);

    u32 newIndex = r->index1 + count;

    if (newIndex > r->data.buffer_size) {
        // should never overflow a static buffer
        NEKO_ASSERT(r->data.usage != GL_STATIC_DRAW);

        ++r->buffer_number;
        r->buffer_number %= r->buffer_count;
        GLsync fence = r->fences[r->buffer_number];

        // Ensure buffer is not in use by GPU
        // If we stall here we are GPU bound
        GLenum result = glClientWaitSync(fence, 0, (GLuint64)1000000000);
        NEKO_ASSERT(result != GL_TIMEOUT_EXPIRED);
        NEKO_ASSERT(result != GL_WAIT_FAILED);
        glDeleteSync(fence);

        r->index0 = 0;
        r->index1 = count;
        r->need_new_sync = 1;
    }

    else {
        r->index0 = r->index1;
        r->index1 = newIndex;
    }

    glBindBuffer(GL_ARRAY_BUFFER, r->buffers[r->buffer_number]);
    u32 stream_size = (r->index1 - r->index0) * r->data.vertex_stride;
    void* memory = glMapBufferRange(GL_ARRAY_BUFFER, r->index0 * r->data.vertex_stride, stream_size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

#if R_BATCH_DEBUG_CHECKS
    if (!memory) {
        NEKO_WARN("\n%u\n", glGetError());
        NEKO_ASSERT(memory);
    }
#endif

    return memory;
}

void gl_unmap_internal() { glUnmapBuffer(GL_ARRAY_BUFFER); }

void neko_render_batch_set_shader(neko_render_batch_renderable_t* r, neko_render_batch_shader_t* program) {
    // Cannot set the shader of a Renderable more than once
    NEKO_ASSERT(!r->program);

    r->program = program;
    glGetProgramiv(program->program, GL_ACTIVE_ATTRIBUTES, (GLint*)&r->attribute_count);

#if R_BATCH_DEBUG_CHECKS
    if (r->attribute_count != r->data.attribute_count) {
        NEKO_WARN("Mismatch between VertexData attribute count (%d), and shader attribute count (%d).", r->attribute_count, r->data.attribute_count);
    }
#endif

    u32 size;
    u32 type;
    char buffer[256];
    u64 hash;

    // Query and set all attribute locations as defined by the shader linking
    for (u32 i = 0; i < r->attribute_count; ++i) {
        glGetActiveAttrib(program->program, i, 256, 0, (GLint*)&size, (GLenum*)&type, buffer);
        hash = neko_hash_str64(buffer);
        type = gl_get_gl_type_internal(type);

#if R_BATCH_DEBUG_CHECKS
        neko_render_batch_vertex_attribute_t* a = 0;

        // Make sure data.AddAttribute(name, ...) has matching named attribute
        // Also make sure the GL::Type matches
        // This helps to catch common mismatch errors between glsl and C++
        for (u32 j = 0; j < r->data.attribute_count; ++j) {
            neko_render_batch_vertex_attribute_t* b = r->data.attributes + j;

            if (b->hash == hash) {
                a = b;
                break;
            }
        }
#endif

        // Make sure the user did not have a mismatch between VertexData
        // attributes and the attributes defined in the vertex shader
        NEKO_ASSERT(a);
        NEKO_ASSERT(a->type == type);

        a->location = glGetAttribLocation(program->program, buffer);
    }

    // Generate VBOs and initialize fences
    u32 usage = r->data.usage;

    for (u32 i = 0; i < r->buffer_count; ++i) {
        GLuint* data_buffer = (GLuint*)r->buffers + i;

        glGenBuffers(1, data_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, *data_buffer);
        glBufferData(GL_ARRAY_BUFFER, r->data.buffer_size * r->data.vertex_stride, NULL, usage);
        r->fences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint gl_compile_shader_internal(const char* Shader, u32 type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, (const GLchar**)&Shader, NULL);
    glCompileShader(handle);

    u32 compiled;
    glGetShaderiv(handle, GL_COMPILE_STATUS, (GLint*)&compiled);

#if R_BATCH_DEBUG_CHECKS
    if (!compiled) {
        NEKO_WARN("Shader of type %d failed compilation.", type);
        char out[2000];
        GLsizei outLen;
        glGetShaderInfoLog(handle, 2000, &outLen, out);
        NEKO_WARN("%s", out);
        NEKO_ASSERT(0);
    }
#endif

    return handle;
}

void neko_render_batch_load_shader(neko_render_batch_shader_t* s, const char* vertex, const char* pixel) {
    // Compile vertex and pixel Shader
    u32 program = glCreateProgram();
    u32 vs = gl_compile_shader_internal(vertex, GL_VERTEX_SHADER);
    u32 ps = gl_compile_shader_internal(pixel, GL_FRAGMENT_SHADER);
    glAttachShader(program, vs);
    glAttachShader(program, ps);

    // Link the Shader to form a program
    glLinkProgram(program);

    u32 linked;
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&linked);

#if R_BATCH_DEBUG_CHECKS
    if (!linked) {
        NEKO_WARN("Shaders failed to link.");
        char out[2000];
        GLsizei outLen;
        glGetProgramInfoLog(program, 2000, &outLen, out);
        NEKO_WARN("%s", out);
        NEKO_ASSERT(0);
    }
#endif

    glDetachShader(program, vs);
    glDetachShader(program, ps);
    glDeleteShader(vs);
    glDeleteShader(ps);

    // Insert Shader into the Shaders array for future lookups
    s->program = program;

    // Query Uniform information and fill out the Shader Uniforms
    GLint uniform_count;
    u32 nameSize = sizeof(char) * R_BATCH_UNIFORM_NAME_LENGTH;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);
    NEKO_ASSERT(uniform_count < R_BATCH_UNIFORM_MAX_COUNT);
    s->uniform_count = uniform_count;

    for (u32 i = 0; i < (u32)uniform_count; ++i) {
        u32 nameLength;
        neko_render_batch_uniform_t u;

        glGetActiveUniform(program, (GLint)i, nameSize, (GLsizei*)&nameLength, (GLsizei*)&u.size, (GLenum*)&u.type, u.name);

        // Uniform named in a Shader is too long for the UNIFORM_NAME_LENGTH constant
        NEKO_ASSERT(nameLength <= R_BATCH_UNIFORM_NAME_LENGTH);

        u.location = glGetUniformLocation(program, u.name);
        u.type = gl_get_gl_type_internal(u.type);
        u.hash = neko_hash_str64(u.name);
        u.id = i;

        // @TODO: Perhaps need to handle appended [0] to Uniform names?

        s->uniforms[i] = u;
    }

#if R_BATCH_DEBUG_CHECKS
    // prevent hash collisions
    for (u32 i = 0; i < (u32)uniform_count; ++i)
        for (u32 j = i + 1; j < (u32)uniform_count; ++j) NEKO_ASSERT(s->uniforms[i].hash != s->uniforms[j].hash);
#endif
}

void neko_render_batch_free_shader(neko_render_batch_shader_t* s) {
    glDeleteProgram(s->program);
    memset(s, 0, sizeof(neko_render_batch_shader_t));
}

neko_render_batch_uniform_t* gl_find_uniform_internal(neko_render_batch_shader_t* s, const char* name) {
    u32 uniform_count = s->uniform_count;
    neko_render_batch_uniform_t* uniforms = s->uniforms;
    u64 hash = neko_hash_str64(name);

    for (u32 i = 0; i < uniform_count; ++i) {
        neko_render_batch_uniform_t* u = uniforms + i;

        if (u->hash == hash) {
            return u;
        }
    }

    return 0;
}

void neko_render_batch_set_active_shader(neko_render_batch_shader_t* s) { glUseProgram(s->program); }

void neko_render_batch_deactivate_shader() { glUseProgram(0); }

void neko_render_batch_send_f32(neko_render_batch_shader_t* s, const char* uniform_name, u32 size, float* floats, u32 count) {
    neko_render_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        NEKO_WARN("Unable to find uniform: %s", uniform_name);
        return;
    }

    NEKO_ASSERT(size == u->size);
    NEKO_ASSERT(u->type == R_BATCH_FLOAT);

    neko_render_batch_set_active_shader(s);
    switch (count) {
        case 1:
            glUniform1f(u->location, floats[0]);
            break;

        case 2:
            glUniform2f(u->location, floats[0], floats[1]);
            break;

        case 3:
            glUniform3f(u->location, floats[0], floats[1], floats[2]);
            break;

        case 4:
            glUniform4f(u->location, floats[0], floats[1], floats[2], floats[3]);
            break;

        default:
            NEKO_ASSERT(0);
            break;
    }
    neko_render_batch_deactivate_shader();
}

void neko_render_batch_send_matrix(neko_render_batch_shader_t* s, const char* uniform_name, float* floats) {
    neko_render_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        NEKO_WARN("Unable to find uniform: %s", uniform_name);
        return;
    }

    NEKO_ASSERT(u->size == 1);
    NEKO_ASSERT(u->type == R_BATCH_FLOAT);

    neko_render_batch_set_active_shader(s);
    glUniformMatrix4fv(u->id, 1, 0, floats);
    neko_render_batch_deactivate_shader();
}

void neko_render_batch_send_texture(neko_render_batch_shader_t* s, const char* uniform_name, u32 index) {
    neko_render_batch_uniform_t* u = gl_find_uniform_internal(s, uniform_name);

    if (!u) {
        NEKO_WARN("Unable to find uniform: %s", uniform_name);
        return;
    }

    NEKO_ASSERT(u->type == R_BATCH_SAMPLER);

    neko_render_batch_set_active_shader(s);
    glUniform1i(u->location, index);
    neko_render_batch_deactivate_shader();
}

static u32 gl_call_sort_pred_internal(neko_render_batch_draw_call_t* a, neko_render_batch_draw_call_t* b) { return a->r->state.u.key < b->r->state.u.key; }

static void neko_render_batch_qsort_internal(neko_render_batch_draw_call_t* items, u32 count) {
    if (count <= 1) return;

    neko_render_batch_draw_call_t pivot = items[count - 1];
    u32 low = 0;
    for (u32 i = 0; i < count - 1; ++i) {
        if (gl_call_sort_pred_internal(items + i, &pivot)) {
            neko_render_batch_draw_call_t tmp = items[i];
            items[i] = items[low];
            items[low] = tmp;
            low++;
        }
    }

    items[count - 1] = items[low];
    items[low] = pivot;
    neko_render_batch_qsort_internal(items, low);
    neko_render_batch_qsort_internal(items + low + 1, count - 1 - low);
}

void neko_render_batch_push_draw_call(void* ctx, neko_render_batch_draw_call_t call) {
    neko_render_batch_context_t* context = (neko_render_batch_context_t*)ctx;
    NEKO_ASSERT(context->count < context->max_draw_calls);
    context->calls[context->count++] = call;
}

u32 gl_get_enum(u32 type) {
    switch (type) {
        case R_BATCH_FLOAT:
            return GL_FLOAT;
            break;

        case R_BATCH_INT:
            return GL_UNSIGNED_BYTE;
            break;

        default:
            NEKO_ASSERT(0);
            return (u32)~0;
    }
}

void gl_do_map_internal(neko_render_batch_draw_call_t* call, neko_render_batch_renderable_t* render) {
    u32 count = call->vert_count;
    void* driver_memory = gl_map_internal(render, count);
    memcpy(driver_memory, call->verts, render->data.vertex_stride * count);
    gl_unmap_internal();
}

static void neko_render_batch_render_internal(neko_render_batch_draw_call_t* call) {
    neko_render_batch_renderable_t* render = call->r;
    u32 texture_count = call->texture_count;
    u32* textures = call->textures;

    if (render->data.usage == GL_STATIC_DRAW) {
        if (render->need_new_sync) {
            render->need_new_sync = 0;
            gl_do_map_internal(call, render);
        }
    } else
        gl_do_map_internal(call, render);

    neko_render_batch_vertex_data_t* data = &render->data;
    neko_render_batch_vertex_attribute_t* attributes = data->attributes;
    u32 vertexStride = data->vertex_stride;
    u32 attributeCount = data->attribute_count;

    neko_render_batch_set_active_shader(render->program);

    u32 bufferNumber = render->buffer_number;
    u32 buffer = render->buffers[bufferNumber];
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    for (u32 i = 0; i < attributeCount; ++i) {
        neko_render_batch_vertex_attribute_t* attribute = attributes + i;

        u32 location = attribute->location;
        u32 size = attribute->size;
        u32 type = gl_get_enum(attribute->type);
        u32 offset = attribute->offset;

        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, size, type, GL_FALSE, vertexStride, (void*)((size_t)offset));
    }

    for (u32 i = 0; i < texture_count; ++i) {
        u32 gl_id = textures[i];

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, gl_id);
    }

    u32 streamOffset = render->index0;
    u32 streamSize = render->index1 - streamOffset;
    glDrawArrays(data->primitive, streamOffset, streamSize);

    if (render->need_new_sync) {
        // @TODO: This shouldn't be called for static buffers, only needed for streaming.
        render->fences[bufferNumber] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        render->need_new_sync = 0;
    }

    for (u32 i = 0; i < attributeCount; ++i) {
        neko_render_batch_vertex_attribute_t* attribute = attributes + i;

        u32 location = attribute->location;
        glDisableVertexAttribArray(location);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void neko_render_batch_present_internal(void* context, neko_render_batch_framebuffer_t* fb, int w, int h) {
    neko_render_batch_context_t* ctx = (neko_render_batch_context_t*)context;
    neko_render_batch_qsort_internal(ctx->calls, ctx->count);

    if (fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->fb_id);
        glViewport(0, 0, fb->w, fb->h);
    }

    // 将所有绘制调用刷新到 GPU
    for (u32 i = 0; i < ctx->count; ++i) {
        neko_render_batch_draw_call_t* call = ctx->calls + i;
        neko_render_batch_render_internal(call);
    }

    if (fb) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        neko_render_batch_set_active_shader(fb->shader);
        glBindBuffer(GL_ARRAY_BUFFER, fb->quad_id);
        glBindTexture(GL_TEXTURE_2D, fb->tex_id);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, R_BATCH_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, R_BATCH_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        neko_render_batch_deactivate_shader();
    }
}

void neko_render_batch_flush(void* ctx, neko_render_batch_framebuffer_t* fb, int w, int h) {
    neko_render_batch_present_internal(ctx, fb, w, h);
    neko_render_batch_context_t* context = (neko_render_batch_context_t*)ctx;
    context->count = 0;
}

int neko_render_batch_draw_call_count(void* ctx) {
    neko_render_batch_context_t* context = (neko_render_batch_context_t*)ctx;
    return context->count;
}

void neko_render_batch_perspective(float* m, float y_fov_radians, float aspect, float n, float f) {
    float a = 1.0f / (float)tanf(y_fov_radians / 2.0f);

    m[0] = a / aspect;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;

    m[4] = 0;
    m[5] = a;
    m[6] = 0;
    m[7] = 0;

    m[8] = 0;
    m[9] = 0;
    m[10] = -((f + n) / (f - n));
    m[11] = -1.0f;

    m[12] = 0;
    m[13] = 0;
    m[14] = -((2.0f * f * n) / (f - n));
    m[15] = 0;
}

void neko_render_batch_ortho_2d(float w, float h, float x, float y, float* m) {
    float left = -w / 2;
    float right = w / 2;
    float top = h / 2;
    float bottom = -h / 2;
    float far_ = 1000.0f;
    float near_ = -1000.0f;

    memset(m, 0, sizeof(float) * 4 * 4);

    // ortho
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far_ - near_);
    m[15] = 1.0f;

    // translate
    m[12] = -x;
    m[13] = -y;
}

void neko_render_batch_copy(float* dst, float* src) {
    for (int i = 0; i < 16; ++i) dst[i] = src[i];
}

void neko_render_batch_mul(float* a, float* b, float* out) {
    float c[16];

    c[0 + 0 * 4] = a[0 + 0 * 4] * b[0 + 0 * 4] + a[0 + 1 * 4] * b[1 + 0 * 4] + a[0 + 2 * 4] * b[2 + 0 * 4] + a[0 + 3 * 4] * b[3 + 0 * 4];
    c[0 + 1 * 4] = a[0 + 0 * 4] * b[0 + 1 * 4] + a[0 + 1 * 4] * b[1 + 1 * 4] + a[0 + 2 * 4] * b[2 + 1 * 4] + a[0 + 3 * 4] * b[3 + 1 * 4];
    c[0 + 2 * 4] = a[0 + 0 * 4] * b[0 + 2 * 4] + a[0 + 1 * 4] * b[1 + 2 * 4] + a[0 + 2 * 4] * b[2 + 2 * 4] + a[0 + 3 * 4] * b[3 + 2 * 4];
    c[0 + 3 * 4] = a[0 + 0 * 4] * b[0 + 3 * 4] + a[0 + 1 * 4] * b[1 + 3 * 4] + a[0 + 2 * 4] * b[2 + 3 * 4] + a[0 + 3 * 4] * b[3 + 3 * 4];
    c[1 + 0 * 4] = a[1 + 0 * 4] * b[0 + 0 * 4] + a[1 + 1 * 4] * b[1 + 0 * 4] + a[1 + 2 * 4] * b[2 + 0 * 4] + a[1 + 3 * 4] * b[3 + 0 * 4];
    c[1 + 1 * 4] = a[1 + 0 * 4] * b[0 + 1 * 4] + a[1 + 1 * 4] * b[1 + 1 * 4] + a[1 + 2 * 4] * b[2 + 1 * 4] + a[1 + 3 * 4] * b[3 + 1 * 4];
    c[1 + 2 * 4] = a[1 + 0 * 4] * b[0 + 2 * 4] + a[1 + 1 * 4] * b[1 + 2 * 4] + a[1 + 2 * 4] * b[2 + 2 * 4] + a[1 + 3 * 4] * b[3 + 2 * 4];
    c[1 + 3 * 4] = a[1 + 0 * 4] * b[0 + 3 * 4] + a[1 + 1 * 4] * b[1 + 3 * 4] + a[1 + 2 * 4] * b[2 + 3 * 4] + a[1 + 3 * 4] * b[3 + 3 * 4];
    c[2 + 0 * 4] = a[2 + 0 * 4] * b[0 + 0 * 4] + a[2 + 1 * 4] * b[1 + 0 * 4] + a[2 + 2 * 4] * b[2 + 0 * 4] + a[2 + 3 * 4] * b[3 + 0 * 4];
    c[2 + 1 * 4] = a[2 + 0 * 4] * b[0 + 1 * 4] + a[2 + 1 * 4] * b[1 + 1 * 4] + a[2 + 2 * 4] * b[2 + 1 * 4] + a[2 + 3 * 4] * b[3 + 1 * 4];
    c[2 + 2 * 4] = a[2 + 0 * 4] * b[0 + 2 * 4] + a[2 + 1 * 4] * b[1 + 2 * 4] + a[2 + 2 * 4] * b[2 + 2 * 4] + a[2 + 3 * 4] * b[3 + 2 * 4];
    c[2 + 3 * 4] = a[2 + 0 * 4] * b[0 + 3 * 4] + a[2 + 1 * 4] * b[1 + 3 * 4] + a[2 + 2 * 4] * b[2 + 3 * 4] + a[2 + 3 * 4] * b[3 + 3 * 4];
    c[3 + 0 * 4] = a[3 + 0 * 4] * b[0 + 0 * 4] + a[3 + 1 * 4] * b[1 + 0 * 4] + a[3 + 2 * 4] * b[2 + 0 * 4] + a[3 + 3 * 4] * b[3 + 0 * 4];
    c[3 + 1 * 4] = a[3 + 0 * 4] * b[0 + 1 * 4] + a[3 + 1 * 4] * b[1 + 1 * 4] + a[3 + 2 * 4] * b[2 + 1 * 4] + a[3 + 3 * 4] * b[3 + 1 * 4];
    c[3 + 2 * 4] = a[3 + 0 * 4] * b[0 + 2 * 4] + a[3 + 1 * 4] * b[1 + 2 * 4] + a[3 + 2 * 4] * b[2 + 2 * 4] + a[3 + 3 * 4] * b[3 + 2 * 4];
    c[3 + 3 * 4] = a[3 + 0 * 4] * b[0 + 3 * 4] + a[3 + 1 * 4] * b[1 + 3 * 4] + a[3 + 2 * 4] * b[2 + 3 * 4] + a[3 + 3 * 4] * b[3 + 3 * 4];

    neko_render_batch_copy(out, c);
}

void gl_mulv(float* a, float* b) {
    float result[4];

    result[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    result[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    result[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    result[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];

    b[0] = result[0];
    b[1] = result[1];
    b[2] = result[2];
    b[3] = result[3];
}

void neko_render_batch_identity(float* m) {
    memset(m, 0, sizeof(float) * 16);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

void neko_gl_reset_data_cache(neko_gl_data_cache_t* cache) {
    cache->ibo.id = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = neko_handle_invalid(neko_render_pipeline_t);
    neko_dyn_array_clear(cache->vdecls);
}

void neko_gl_pipeline_state() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    // CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    CHECK_GL_CORE(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS););
    glDisable(GL_MULTISAMPLE);

    CHECK_GL_CORE(neko_render_info_t* info = neko_render_info(); if (info->compute.available){
            // NEKO_INVOKE_ONCE(NEKO_TRACE("Compute shader available: %s", NEKO_BOOL_STR(info->compute.available)););
            // glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    });
}

/* Utilities */
s32 neko_gl_buffer_usage_to_gl_enum(neko_render_buffer_usage_type type) {
    s32 mode = GL_STATIC_DRAW;
    switch (type) {
        default:
        case R_BUFFER_USAGE_STATIC:
            mode = GL_STATIC_DRAW;
            break;
        case R_BUFFER_USAGE_STREAM:
            mode = GL_STREAM_DRAW;
            break;
        case R_BUFFER_USAGE_DYNAMIC:
            mode = GL_DYNAMIC_DRAW;
            break;
    }
    return mode;
}

u32 neko_gl_access_type_to_gl_access_type(neko_render_access_type type) {
    CHECK_GL_CORE(u32 access = GL_WRITE_ONLY; switch (type) {
        case R_ACCESS_WRITE_ONLY:
            access = GL_WRITE_ONLY;
            break;
        case R_ACCESS_READ_ONLY:
            access = GL_READ_ONLY;
            break;
        case R_ACCESS_READ_WRITE:
            access = GL_READ_WRITE;
            break;
        default:
            break;
    } return access;)
    return 0;
}

u32 neko_gl_texture_wrap_to_gl_texture_wrap(neko_render_texture_wrapping_type type) {
    u32 wrap = GL_REPEAT;
    switch (type) {
        default:
        case R_TEXTURE_WRAP_REPEAT:
            wrap = GL_REPEAT;
            break;
        case R_TEXTURE_WRAP_MIRRORED_REPEAT:
            wrap = GL_MIRRORED_REPEAT;
            break;
        case R_TEXTURE_WRAP_CLAMP_TO_EDGE:
            wrap = GL_CLAMP_TO_EDGE;
            break;
            CHECK_GL_CORE(case R_TEXTURE_WRAP_CLAMP_TO_BORDER : wrap = GL_CLAMP_TO_BORDER; break;)
    };

    return wrap;
}

u32 neko_gl_texture_format_to_gl_data_type(neko_render_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_A8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_UNSIGNED_INT;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_UNSIGNED_BYTE;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_FLOAT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_UNSIGNED_INT_24_8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
    }
    return format;
}

u32 neko_gl_texture_format_to_gl_texture_format(neko_render_texture_format_type type) {
    u32 dt = GL_RGBA;
    switch (type) {
        default:
        case R_TEXTURE_FORMAT_RGBA8:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_A8:
            dt = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            dt = GL_RED_INTEGER;
            break;
        case R_TEXTURE_FORMAT_R32F:
            dt = GL_RED;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            dt = GL_RGB;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            dt = GL_RGBA;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            dt = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            dt = GL_DEPTH_STENCIL;
            break;
    }
    return dt;
}

u32 neko_gl_texture_format_to_gl_texture_internal_format(neko_render_texture_format_type type) {
    u32 format = GL_UNSIGNED_BYTE;
    switch (type) {
        case R_TEXTURE_FORMAT_A8:
            format = GL_ALPHA;
            break;
        case R_TEXTURE_FORMAT_R8:
            format = GL_RED;
            break;
        case R_TEXTURE_FORMAT_R32:
            format = GL_R32UI;
            break;
        case R_TEXTURE_FORMAT_R32F:
            format = GL_R32F;
            break;
        case R_TEXTURE_FORMAT_RGB8:
            format = GL_RGB8;
            break;
        case R_TEXTURE_FORMAT_RGBA8:
            format = GL_RGBA8;
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            format = GL_RGBA16F;
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            format = GL_RGBA32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            format = GL_DEPTH_COMPONENT;
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            format = GL_DEPTH_COMPONENT16;
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            format = GL_DEPTH_COMPONENT24;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            format = GL_DEPTH_COMPONENT32F;
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            format = GL_DEPTH24_STENCIL8;
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            format = GL_DEPTH32F_STENCIL8;
            break;
    }
    return format;
}

u32 neko_gl_shader_stage_to_gl_stage(neko_render_shader_stage_type type) {
    u32 stage = GL_VERTEX_SHADER;
    switch (type) {
        default:
        case R_SHADER_STAGE_VERTEX:
            stage = GL_VERTEX_SHADER;
            break;
        case R_SHADER_STAGE_FRAGMENT:
            stage = GL_FRAGMENT_SHADER;
            break;
            // CHECK_GL_CORE(case R_SHADER_STAGE_COMPUTE : stage = GL_COMPUTE_SHADER; break;)
    }
    return stage;
}

u32 neko_gl_primitive_to_gl_primitive(neko_render_primitive_type type) {
    u32 prim = GL_TRIANGLES;
    switch (type) {
        default:
        case R_PRIMITIVE_TRIANGLES:
            prim = GL_TRIANGLES;
            break;
        case R_PRIMITIVE_LINES:
            prim = GL_LINES;
            break;
            CHECK_GL_CORE(case R_PRIMITIVE_QUADS : prim = GL_QUADS; break;)
    }
    return prim;
}

u32 neko_gl_blend_equation_to_gl_blend_eq(neko_render_blend_equation_type eq) {
    u32 beq = GL_FUNC_ADD;
    switch (eq) {
        default:
        case R_BLEND_EQUATION_ADD:
            beq = GL_FUNC_ADD;
            break;
        case R_BLEND_EQUATION_SUBTRACT:
            beq = GL_FUNC_SUBTRACT;
            break;
        case R_BLEND_EQUATION_REVERSE_SUBTRACT:
            beq = GL_FUNC_REVERSE_SUBTRACT;
            break;
        case R_BLEND_EQUATION_MIN:
            beq = GL_MIN;
            break;
        case R_BLEND_EQUATION_MAX:
            beq = GL_MAX;
            break;
    };

    return beq;
}

u32 neko_gl_blend_mode_to_gl_blend_mode(neko_render_blend_mode_type type, u32 def) {
    u32 mode = def;
    switch (type) {
        case R_BLEND_MODE_ZERO:
            mode = GL_ZERO;
            break;
        case R_BLEND_MODE_ONE:
            mode = GL_ONE;
            break;
        case R_BLEND_MODE_SRC_COLOR:
            mode = GL_SRC_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_SRC_COLOR:
            mode = GL_ONE_MINUS_SRC_COLOR;
            break;
        case R_BLEND_MODE_DST_COLOR:
            mode = GL_DST_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_DST_COLOR:
            mode = GL_ONE_MINUS_DST_COLOR;
            break;
        case R_BLEND_MODE_SRC_ALPHA:
            mode = GL_SRC_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_SRC_ALPHA:
            mode = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case R_BLEND_MODE_DST_ALPHA:
            mode = GL_DST_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_DST_ALPHA:
            mode = GL_ONE_MINUS_DST_ALPHA;
            break;
        case R_BLEND_MODE_CONSTANT_COLOR:
            mode = GL_CONSTANT_COLOR;
            break;
        case R_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR:
            mode = GL_ONE_MINUS_CONSTANT_COLOR;
            break;
        case R_BLEND_MODE_CONSTANT_ALPHA:
            mode = GL_CONSTANT_ALPHA;
            break;
        case R_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA:
            mode = GL_ONE_MINUS_CONSTANT_ALPHA;
            break;
    }
    return mode;
}

u32 neko_gl_cull_face_to_gl_cull_face(neko_render_face_culling_type type) {
    u32 fc = GL_BACK;
    switch (type) {
        default:
        case R_FACE_CULLING_BACK:
            fc = GL_BACK;
            break;
        case R_FACE_CULLING_FRONT:
            fc = GL_FRONT;
            break;
        case R_FACE_CULLING_FRONT_AND_BACK:
            fc = GL_FRONT_AND_BACK;
            break;
    }
    return fc;
}

u32 neko_gl_winding_order_to_gl_winding_order(neko_render_winding_order_type type) {
    u32 wo = GL_CCW;
    switch (type) {
        case R_WINDING_ORDER_CCW:
            wo = GL_CCW;
            break;
        case R_WINDING_ORDER_CW:
            wo = GL_CW;
            break;
    }
    return wo;
}

u32 neko_gl_depth_func_to_gl_depth_func(neko_render_depth_func_type type) {
    u32 func = GL_LESS;
    switch (type) {
        default:
        case R_DEPTH_FUNC_LESS:
            func = GL_LESS;
            break;
        case R_DEPTH_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case R_DEPTH_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case R_DEPTH_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case R_DEPTH_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case R_DEPTH_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case R_DEPTH_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case R_DEPTH_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

bool neko_gl_depth_mask_to_gl_mask(neko_render_depth_mask_type type) {
    bool ret = true;
    switch (type) {
        default:
        case R_DEPTH_MASK_ENABLED:
            ret = true;
            break;
        case R_DEPTH_MASK_DISABLED:
            ret = false;
            break;
    }
    return ret;
}

u32 neko_gl_stencil_func_to_gl_stencil_func(neko_render_stencil_func_type type) {
    u32 func = GL_ALWAYS;
    switch (type) {
        default:
        case R_STENCIL_FUNC_LESS:
            func = GL_LESS;
            break;
        case R_STENCIL_FUNC_NEVER:
            func = GL_NEVER;
            break;
        case R_STENCIL_FUNC_EQUAL:
            func = GL_EQUAL;
            break;
        case R_STENCIL_FUNC_LEQUAL:
            func = GL_LEQUAL;
            break;
        case R_STENCIL_FUNC_GREATER:
            func = GL_GREATER;
            break;
        case R_STENCIL_FUNC_NOTEQUAL:
            func = GL_NOTEQUAL;
            break;
        case R_STENCIL_FUNC_GEQUAL:
            func = GL_GEQUAL;
            break;
        case R_STENCIL_FUNC_ALWAYS:
            func = GL_ALWAYS;
            break;
    }
    return func;
}

u32 neko_gl_stencil_op_to_gl_stencil_op(neko_render_stencil_op_type type) {
    u32 op = GL_KEEP;
    switch (type) {
        default:
        case R_STENCIL_OP_KEEP:
            op = GL_KEEP;
            break;
        case R_STENCIL_OP_ZERO:
            op = GL_ZERO;
            break;
        case R_STENCIL_OP_REPLACE:
            op = GL_REPLACE;
            break;
        case R_STENCIL_OP_INCR:
            op = GL_INCR;
            break;
        case R_STENCIL_OP_INCR_WRAP:
            op = GL_INCR_WRAP;
            break;
        case R_STENCIL_OP_DECR:
            op = GL_DECR;
            break;
        case R_STENCIL_OP_DECR_WRAP:
            op = GL_DECR_WRAP;
            break;
        case R_STENCIL_OP_INVERT:
            op = GL_INVERT;
            break;
    }
    return op;
}

neko_gl_uniform_type neko_gl_uniform_type_to_gl_uniform_type(neko_render_uniform_type gstype) {
    neko_gl_uniform_type type = NEKO_GL_UNIFORMTYPE_FLOAT;
    switch (gstype) {
        default:
        case R_UNIFORM_FLOAT:
            type = NEKO_GL_UNIFORMTYPE_FLOAT;
            break;
        case R_UNIFORM_INT:
            type = NEKO_GL_UNIFORMTYPE_INT;
            break;
        case R_UNIFORM_VEC2:
            type = NEKO_GL_UNIFORMTYPE_VEC2;
            break;
        case R_UNIFORM_VEC3:
            type = NEKO_GL_UNIFORMTYPE_VEC3;
            break;
        case R_UNIFORM_VEC4:
            type = NEKO_GL_UNIFORMTYPE_VEC4;
            break;
        case R_UNIFORM_MAT4:
            type = NEKO_GL_UNIFORMTYPE_MAT4;
            break;
        case R_UNIFORM_USAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case R_UNIFORM_SAMPLER2D:
            type = NEKO_GL_UNIFORMTYPE_SAMPLER2D;
            break;
        case R_UNIFORM_SAMPLERCUBE:
            type = NEKO_GL_UNIFORMTYPE_SAMPLERCUBE;
            break;
    }
    return type;
}

u32 neko_gl_index_buffer_size_to_gl_index_type(size_t sz) {
    u32 type = GL_UNSIGNED_INT;
    switch (sz) {
        default:
        case 4:
            type = GL_UNSIGNED_INT;
            break;
        case 2:
            type = GL_UNSIGNED_SHORT;
            break;
        case 1:
            type = GL_UNSIGNED_BYTE;
            break;
    }
    return type;
}

size_t neko_gl_get_byte_size_of_vertex_attribute(neko_render_vertex_attribute_type type) {
    size_t byte_size = 0;
    switch (type) {
        case R_VERTEX_ATTRIBUTE_FLOAT4: {
            byte_size = sizeof(f32) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT3: {
            byte_size = sizeof(f32) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT2: {
            byte_size = sizeof(f32) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_FLOAT: {
            byte_size = sizeof(f32) * 1;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT4: {
            byte_size = sizeof(u32) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT3: {
            byte_size = sizeof(u32) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT2: {
            byte_size = sizeof(u32) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_UINT: {
            byte_size = sizeof(u32) * 1;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE4: {
            byte_size = sizeof(uint8_t) * 4;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE3: {
            byte_size = sizeof(uint8_t) * 3;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE2: {
            byte_size = sizeof(uint8_t) * 2;
        } break;
        case R_VERTEX_ATTRIBUTE_BYTE: {
            byte_size = sizeof(uint8_t) * 1;
        } break;
    }

    return byte_size;
}

size_t neko_gl_calculate_vertex_size_in_bytes(neko_render_vertex_attribute_desc_t* layout, u32 count) {
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (u32 i = 0; i < count; ++i) {
        neko_render_vertex_attribute_type type = layout[i].format;
        sz += neko_gl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t neko_gl_get_vertex_attr_byte_offest(neko_dyn_array(neko_render_vertex_attribute_desc_t) layout, u32 idx) {
    // Recursively calculate offset
    size_t total_offset = 0;

    // Base case
    if (idx == 0) {
        return total_offset;
    }

    // Calculate total offset up to this point
    for (u32 i = 0; i < idx; ++i) {
        total_offset += neko_gl_get_byte_size_of_vertex_attribute(layout[i].format);
    }

    return total_offset;
}

size_t neko_gl_uniform_data_size_in_bytes(neko_render_uniform_type type) {
    size_t sz = 0;
    switch (type) {
        case R_UNIFORM_FLOAT:
            sz = sizeof(float);
            break;
        case R_UNIFORM_INT:
            sz = sizeof(s32);
            break;
        case R_UNIFORM_VEC2:
            sz = 2 * sizeof(float);
            break;
        case R_UNIFORM_VEC3:
            sz = 3 * sizeof(float);
            break;
        case R_UNIFORM_VEC4:
            sz = 4 * sizeof(float);
            break;
        case R_UNIFORM_MAT4:
            sz = 16 * sizeof(float);
            break;
        case R_UNIFORM_SAMPLER2D:
            sz = sizeof(neko_handle(neko_render_texture_t));
            break;  // handle size
        case R_UNIFORM_USAMPLER2D:
            sz = sizeof(neko_handle(neko_render_texture_t));
            break;  // handle size
        case R_UNIFORM_SAMPLERCUBE:
            sz = sizeof(neko_handle(neko_render_texture_t));
            break;  // handle size
        default: {
            sz = 0;
        } break;
    }
    return sz;
}

// neko_thread_mutex_t g_graphics_mutex;

NEKO_API_DECL neko_render_t* neko_render_create() {
    // 构建新的图形界面
    neko_render_t* gfx = neko_malloc_init(neko_render_t);

    // 为OpenGL构建内部数据
    gfx->user_data = neko_malloc_init(neko_gl_data_t);

    // thread_mutex_init(&g_graphics_mutex);

    return gfx;
}

NEKO_API_DECL void neko_render_destroy(neko_render_t* render) {
    // 释放所有资源 (假设它们已从GPU中释放)
    if (render == NULL) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)render->user_data;

#define OGL_FREE_DATA(SA, T, FUNC)                                                                                    \
    do {                                                                                                              \
        for (neko_slot_array_iter it = 1; neko_slot_array_iter_valid(SA, it); neko_slot_array_iter_advance(SA, it)) { \
            neko_handle(T) hndl = NEKO_DEFAULT_VAL();                                                                 \
            hndl.id = it;                                                                                             \
            FUNC(hndl);                                                                                               \
        }                                                                                                             \
    } while (0)

    // Free all gl data
    if (ogl->pipelines) OGL_FREE_DATA(ogl->pipelines, neko_render_pipeline_t, neko_render_pipeline_destroy);
    if (ogl->shaders) OGL_FREE_DATA(ogl->shaders, neko_render_shader_t, neko_render_shader_destroy);
    if (ogl->vertex_buffers) OGL_FREE_DATA(ogl->vertex_buffers, neko_render_vertex_buffer_t, neko_render_vertex_buffer_destroy);
    if (ogl->index_buffers) OGL_FREE_DATA(ogl->index_buffers, neko_render_index_buffer_t, neko_render_index_buffer_destroy);
    if (ogl->renderpasses) OGL_FREE_DATA(ogl->renderpasses, neko_render_renderpass_t, neko_render_renderpass_destroy);
    if (ogl->frame_buffers) OGL_FREE_DATA(ogl->frame_buffers, neko_render_framebuffer_t, neko_render_framebuffer_destroy);
    if (ogl->textures) OGL_FREE_DATA(ogl->textures, neko_render_texture_t, neko_render_texture_destroy);
    if (ogl->uniforms) OGL_FREE_DATA(ogl->uniforms, neko_render_uniform_t, neko_render_uniform_destroy);
    if (ogl->uniform_buffers) OGL_FREE_DATA(ogl->uniform_buffers, neko_render_uniform_buffer_t, neko_render_uniform_buffer_destroy);
    // if (ogl->storage_buffers)   OGL_FREE_DATA(ogl->storage_buffers, neko_render_storage_buffer_t, neko_render_storage_buffer_destroy);

    neko_slot_array_free(ogl->shaders);
    neko_slot_array_free(ogl->vertex_buffers);
    neko_slot_array_free(ogl->index_buffers);
    neko_slot_array_free(ogl->frame_buffers);
    neko_slot_array_free(ogl->uniforms);
    neko_slot_array_free(ogl->textures);
    neko_slot_array_free(ogl->pipelines);
    neko_slot_array_free(ogl->renderpasses);
    neko_slot_array_free(ogl->uniform_buffers);
    neko_slot_array_free(ogl->storage_buffers);

    // Free uniform data array
    neko_dyn_array_free(ogl->uniform_data.mat4);
    neko_dyn_array_free(ogl->uniform_data.vec4);
    neko_dyn_array_free(ogl->uniform_data.vec3);
    neko_dyn_array_free(ogl->uniform_data.vec2);
    neko_dyn_array_free(ogl->uniform_data.flt);
    neko_dyn_array_free(ogl->uniform_data.i32);
    neko_dyn_array_free(ogl->uniform_data.ui32);

    // Free data cache
    neko_dyn_array_free(ogl->cache.vdecls);

    neko_free(render);
    render = NULL;

    // thread_mutex_term(&g_graphics_mutex);
}

NEKO_API_DECL void neko_render_shutdown(neko_render_t* render) {}

neko_gl_texture_t gl_texture_update_internal(const neko_render_texture_desc_t* desc, u32 hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();
    if (hndl) tex = neko_slot_array_get(ogl->textures, hndl);
    u32 width = desc->width;
    u32 height = desc->height;
    void* data = desc->data[0];

    if (!hndl) {
        glGenTextures(1, &tex.id);
    }

    GLenum target = 0x00;
    switch (desc->type) {
        default:
        case R_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case R_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }

    glBindTexture(target, tex.id);

    u32 cnt = R_TEXTURE_DATA_MAX;
    switch (desc->type) {
        case R_TEXTURE_2D:
            cnt = 1;
            break;
    }

    for (u32 i = 0; i < cnt; ++i) {
        GLenum itarget = 0x00;
        data = desc->data[i];
        switch (desc->type) {
            default:
            case R_TEXTURE_2D: {
                itarget = GL_TEXTURE_2D;
            } break;
            case R_TEXTURE_CUBEMAP: {
                itarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
            } break;
        }

        if (tex.desc.width * tex.desc.height < width * height) {
            // Construct texture based on appropriate format
            switch (desc->format) {
                case R_TEXTURE_FORMAT_A8:
                    glTexImage2D(itarget, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R8:
                    glTexImage2D(itarget, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case R_TEXTURE_FORMAT_RG8:
                    glTexImage2D(itarget, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_RGB8:
                    glTexImage2D(itarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA8:
                    glTexImage2D(itarget, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA16F:
                    glTexImage2D(itarget, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA32F:
                    glTexImage2D(itarget, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH8:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH16:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH24:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH32F:
                    glTexImage2D(itarget, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexImage2D(itarget, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: 因为 Apple 是一家垃圾公司 所以必须将其分开并仅提供对 4.1 功能的支持。
                // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        } else {
            // Subimage
            switch (desc->format) {
                case R_TEXTURE_FORMAT_A8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R32:
                    glTexImage2D(itarget, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
                    break;
                case R_TEXTURE_FORMAT_RG8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RG, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_RGB8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    break;
                case R_TEXTURE_FORMAT_R32F:
                    glTexImage2D(itarget, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA16F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_RGBA32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_RGBA, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH16:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH24:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH32F:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
                    break;
                case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
                    glTexSubImage2D(itarget, 0, (u32)desc->offset.x, (u32)desc->offset.y, width, height, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data);
                    break;

                // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
                // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
                default:
                    break;
            }
        }
    }

    s32 mag_filter = desc->mag_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    s32 min_filter = desc->min_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == R_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        } else {
            min_filter = desc->mip_filter == R_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    const u32 texture_wrap_s = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_s);
    const u32 texture_wrap_t = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_t);
    const u32 texture_wrap_r = neko_gl_texture_wrap_to_gl_texture_wrap(desc->wrap_r);

    if (desc->num_mips) {
        glGenerateMipmap(target);
    }

    // Need to make sure this is available before being able to use

    // CHECK_GL_CORE(float aniso = 0.0f; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso); glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso););

    glTexParameteri(target, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, texture_wrap_t);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, texture_wrap_r);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);

    // Unbind buffers
    glBindTexture(target, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Set description
    tex.desc = *desc;

    // Add texture to internal resource pool and return handle
    return tex;
}

/* Resource Creation */
NEKO_API_DECL neko_handle(neko_render_texture_t) neko_render_texture_create_impl(const neko_render_texture_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_texture_t tex = gl_texture_update_internal(&desc, 0);
    // Add texture to internal resource pool and return handle
    return (neko_handle_create(neko_render_texture_t, neko_slot_array_insert(ogl->textures, tex)));
}

NEKO_API_DECL neko_handle(neko_render_uniform_t) neko_render_uniform_create_impl(const neko_render_uniform_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Assert if data isn't named
    if (desc.name == NULL) {
        NEKO_WARN("Uniform must be named for OpenGL.");
        return neko_handle_invalid(neko_render_uniform_t);
    }

    u32 ct = !desc.layout ? 0 : !desc.layout_size ? 1 : (u32)desc.layout_size / (u32)sizeof(neko_render_uniform_layout_desc_t);
    if (ct < 1) {
        NEKO_WARN("Uniform layout description must not be empty for: %s.", desc.name);
        return neko_handle_invalid(neko_render_uniform_t);
    }

    // Construct list for uniform handles
    neko_gl_uniform_list_t ul = NEKO_DEFAULT_VAL();
    memcpy(ul.name, desc.name, 64);

    // Iterate layout, construct individual handles
    for (u32 i = 0; i < ct; ++i) {
        // Uniform to fill out
        neko_gl_uniform_t u = NEKO_DEFAULT_VAL();
        // Cache layout
        neko_render_uniform_layout_desc_t* layout = &desc.layout[i];

        memcpy(u.name, layout->fname, 64);
        u.type = neko_gl_uniform_type_to_gl_uniform_type(layout->type);
        u.size = neko_gl_uniform_data_size_in_bytes(layout->type);
        u.count = layout->count ? layout->count : 1;
        u.location = UINT32_MAX;

        // Add to size of ul
        ul.size += u.size * u.count;

        // Push uniform into list
        neko_dyn_array_push(ul.uniforms, u);
    }

    return neko_handle_create(neko_render_uniform_t, neko_slot_array_insert(ogl->uniforms, ul));
}

NEKO_API_DECL neko_handle(neko_render_vertex_buffer_t) neko_render_vertex_buffer_create_impl(const neko_render_vertex_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_handle(neko_render_vertex_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Vertex buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        NEKO_ASSERT(false);
    }

    glGenBuffers(1, &buffer.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    glBufferData(GL_ARRAY_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_render_vertex_buffer_t, neko_slot_array_insert(ogl->vertex_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_render_index_buffer_t) neko_render_index_buffer_create_impl(const neko_render_index_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_handle(neko_render_index_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};

    // Assert if data isn't filled for vertex data when static draw enabled
    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Index buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        NEKO_ASSERT(false);
    }

    glGenBuffers(1, &buffer.id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    hndl = neko_handle_create(neko_render_index_buffer_t, neko_slot_array_insert(ogl->index_buffers, buffer));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_render_uniform_buffer_t) neko_render_uniform_buffer_create_impl(const neko_render_uniform_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_handle(neko_render_uniform_buffer_t) hndl = NEKO_DEFAULT_VAL();

    // Assert if data isn't named
    if (desc.name == NULL) {
        NEKO_WARN("Uniform buffer must be named for Opengl.");
    }

    neko_gl_uniform_buffer_t u = NEKO_DEFAULT_VAL();
    memcpy(u.name, desc.name, 64);
    u.size = desc.size;
    u.location = UINT32_MAX;

    // Generate buffer (if needed)
    glGenBuffers(1, &u.ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, u.ubo);
    glBufferData(GL_UNIFORM_BUFFER, u.size, 0, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    hndl = neko_handle_create(neko_render_uniform_buffer_t, neko_slot_array_insert(ogl->uniform_buffers, u));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_render_storage_buffer_t) neko_render_storage_buffer_create_impl(const neko_render_storage_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_handle(neko_render_storage_buffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_storage_buffer_t sbo = NEKO_DEFAULT_VAL();

    if (desc.name == NULL) {
        NEKO_WARN("Storage buffer must be named for Opengl.");
    }

    if (desc.usage == R_BUFFER_USAGE_STATIC && !desc.data) {
        neko_println("Error: Storage buffer desc must contain data when R_BUFFER_USAGE_STATIC set.");
        NEKO_ASSERT(false);
    }

    glGenBuffers(1, &sbo.buffer);

    // CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer); glBufferData(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    //               glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););

    // CHECK_GL_CORE(
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo.buffer);

    // 检查 desc 标志以映射缓冲区
    GLbitfield flags = 0x00;
    if (desc.flags & R_BUFFER_FLAG_MAP_PERSISTENT) {
        flags |= GL_MAP_PERSISTENT_BIT;
    }
    if (desc.flags & R_BUFFER_FLAG_MAP_COHERENT) {
        flags |= GL_MAP_PERSISTENT_BIT;
        flags |= GL_MAP_COHERENT_BIT;
    }
    if (desc.access & R_ACCESS_READ_ONLY) {
        flags |= GL_MAP_READ_BIT;
    } else if (desc.access & R_ACCESS_WRITE_ONLY) {
        flags |= GL_MAP_WRITE_BIT;
    } else if (desc.access & R_ACCESS_READ_WRITE) {
        flags |= (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
    }

    GLbitfield store_flags = flags;
    if (desc.usage == R_BUFFER_USAGE_DYNAMIC) {
        store_flags |= GL_DYNAMIC_STORAGE_BIT;
    }

    // 现在只进行读/写访问
    // flags |= GL_MAP_READ_BIT; flags |= GL_MAP_WRITE_BIT;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, store_flags);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, desc.size, desc.data, neko_gl_buffer_usage_to_gl_enum(desc.usage));
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // 存储映射
    if (flags & GL_MAP_PERSISTENT_BIT) {
        sbo.map = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, desc.size, flags);
    }

    GLenum err = glGetError();
    while ((err = glGetError()) != GL_NO_ERROR) {
        neko_println("GL ERROR: 0x%x: %s", err, glGetString(err));
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // );

    memcpy(sbo.name, desc.name, 64);
    sbo.access = desc.access;
    sbo.size = desc.size;
    sbo.block_idx = UINT32_MAX;

    hndl = neko_handle_create(neko_render_storage_buffer_t, neko_slot_array_insert(ogl->storage_buffers, sbo));

    return hndl;
}

NEKO_API_DECL neko_handle(neko_render_framebuffer_t) neko_render_framebuffer_create_impl(const neko_render_framebuffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_handle(neko_render_framebuffer_t) hndl = NEKO_DEFAULT_VAL();
    neko_gl_buffer_t buffer = {0};
    glGenFramebuffers(1, &buffer.id);
    hndl = neko_handle_create(neko_render_framebuffer_t, neko_slot_array_insert(ogl->frame_buffers, buffer));
    return hndl;
}

#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX 0x01
#define NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE 0x02
#define NEKO_GL_GRAPHICS_MAX_SID 128

NEKO_API_DECL neko_handle(neko_render_shader_t) neko_render_shader_create_impl(const neko_render_shader_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_shader_t shader = {0};
    u32 pip = 0x00;

    u32 sid_ct = 0;
    u32 sids[NEKO_GL_GRAPHICS_MAX_SID] = NEKO_DEFAULT_VAL();

    // Create shader program
    shader.id = glCreateProgram();

    u32 ct = (u32)desc.size / (u32)sizeof(neko_render_shader_source_desc_t);
    for (u32 i = 0; i < ct; ++i) {
        if (desc.sources[i].type == R_SHADER_STAGE_VERTEX) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc.sources[i].type == R_SHADER_STAGE_COMPUTE) pip |= NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & NEKO_GL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            neko_println("Error: Cannot have compute and render stages for shader program.");
            NEKO_ASSERT(false);
        }

        u32 stage = neko_gl_shader_stage_to_gl_stage(desc.sources[i].type);
        u32 sid = glCreateShader(stage);

        if (!sid) {
            neko_println("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}", desc.name);
            NEKO_ASSERT(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc.sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        // Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE) {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)neko_safe_malloc(max_len);
            memset(log, 0, max_len);

            // The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);

            // Delete shader.
            glDeleteShader(shader.id);

            // Provide the infolog
            neko_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc.name, log, desc.sources[i].source);

            neko_safe_free(log);
            log = NULL;

            // NEKO_ASSERT(false);
            return neko_handle_invalid(neko_render_shader_t);
        }

        // Attach shader to program
        glAttachShader(shader.id, sid);

        // Add to shader array
        sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader.id);

    // Create info log for errors
    s32 is_good = 0;
    glGetProgramiv(shader.id, GL_LINK_STATUS, (s32*)&is_good);
    if (is_good == GL_FALSE) {
        GLint max_len = 0;
        glGetProgramiv(shader.id, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)neko_safe_malloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader.id, max_len, &max_len, log);

        // Print error
        NEKO_ERROR("Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc.name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader.id);

        neko_safe_free(log);
        log = NULL;

        // Just assert for now
        NEKO_ASSERT(false);
    }

    glValidateProgram(shader.id);
    glGetProgramiv(shader.id, GL_VALIDATE_STATUS, &is_good);
    if (!is_good) {
        NEKO_ERROR("Failed to validate shader: '%s'", desc.name);
    }

    // Free shaders after use
    for (u32 i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Iterate over uniforms
    /*
    {
        char tmp_name[256] = NEKO_DEFAULT_VAL();
        s32 count = 0;
        glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &count);
        neko_println("Active Uniforms: %d\n", count);

        for (u32 i = 0; i < count; i++) {
            s32 sz = 0;
            u32 type;
            glGetActiveUniform(shader, (GLuint)i, 256, NULL, &sz, &type, tmp_name);
            neko_println("Uniform #%d Type: %u the_name: %s\n", i, type, tmp_name);
        }
    }
    */

    // Add to pool and return handle
    return (neko_handle_create(neko_render_shader_t, neko_slot_array_insert(ogl->shaders, shader)));
}

NEKO_API_DECL neko_handle(neko_render_renderpass_t) neko_render_renderpass_create_impl(const neko_render_renderpass_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    neko_gl_renderpass_t pass = NEKO_DEFAULT_VAL();

    // Set fbo
    pass.fbo = desc.fbo;

    // Set color attachments
    u32 ct = (u32)desc.color_size / (u32)sizeof(neko_handle(neko_render_texture_t));
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pass.color, desc.color[i]);
    }
    // Set depth attachment
    pass.depth = desc.depth;

    // Create handle and return
    return (neko_handle_create(neko_render_renderpass_t, neko_slot_array_insert(ogl->renderpasses, pass)));
}

NEKO_API_DECL neko_handle(neko_render_pipeline_t) neko_render_pipeline_create_impl(const neko_render_pipeline_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    neko_gl_pipeline_t pipe = NEKO_DEFAULT_VAL();

    // Add states
    pipe.blend = desc.blend;
    pipe.depth = desc.depth;
    pipe.raster = desc.raster;
    pipe.stencil = desc.stencil;
    pipe.compute = desc.compute;

    // Add layout
    u32 ct = (u32)desc.layout.size / (u32)sizeof(neko_render_vertex_attribute_desc_t);
    neko_dyn_array_reserve(pipe.layout, ct);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(pipe.layout, desc.layout.attrs[i]);
    }

    // Create handle and return
    return (neko_handle_create(neko_render_pipeline_t, neko_slot_array_insert(ogl->pipelines, pipe)));
}

// Resource Destruction
NEKO_API_DECL void neko_render_texture_destroy_impl(neko_handle(neko_render_texture_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) return;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    glDeleteTextures(1, &tex->id);
    neko_slot_array_erase(ogl->textures, hndl.id);
}

NEKO_API_DECL void neko_render_uniform_destroy_impl(neko_handle(neko_render_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniforms, hndl.id)) return;
    neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, hndl.id);
    neko_dyn_array_free(ul->uniforms);
    neko_slot_array_erase(ogl->uniforms, hndl.id);
}

NEKO_API_DECL void neko_render_shader_destroy_impl(neko_handle(neko_render_shader_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->shaders, hndl.id)) return;
    glDeleteProgram(neko_slot_array_get(ogl->shaders, hndl.id).id);
    neko_slot_array_erase(ogl->shaders, hndl.id);
}

NEKO_API_DECL void neko_render_vertex_buffer_destroy_impl(neko_handle(neko_render_vertex_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->vertex_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glDeleteBuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->vertex_buffers, hndl.id);
}

NEKO_API_DECL void neko_render_index_buffer_destroy_impl(neko_handle(neko_render_index_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->index_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    glDeleteBuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->index_buffers, hndl.id);
}

NEKO_API_DECL void neko_render_uniform_buffer_destroy_impl(neko_handle(neko_render_uniform_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->uniform_buffers, hndl.id)) return;
    neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &u->ubo);

    // Delete from slot array
    neko_slot_array_erase(ogl->uniform_buffers, hndl.id);
}

NEKO_API_DECL void neko_render_storage_buffer_destroy_impl(neko_handle(neko_render_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) return;
    neko_gl_storage_buffer_t* sb = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // Delete buffer (if needed)
    glDeleteBuffers(1, &sb->buffer);

    // Delete from slot array
    neko_slot_array_erase(ogl->storage_buffers, hndl.id);
}

NEKO_API_DECL void neko_render_framebuffer_destroy_impl(neko_handle(neko_render_framebuffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->frame_buffers, hndl.id)) return;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->frame_buffers, hndl.id);
    glDeleteFramebuffers(1, &buffer.id);
    neko_slot_array_erase(ogl->frame_buffers, hndl.id);
}

NEKO_API_DECL void neko_render_renderpass_destroy_impl(neko_handle(neko_render_renderpass_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->renderpasses, hndl.id)) return;
    // TODO: erase all color attachments from renderpasss
    neko_slot_array_erase(ogl->renderpasses, hndl.id);
}

NEKO_API_DECL void neko_render_pipeline_destroy_impl(neko_handle(neko_render_pipeline_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->pipelines, hndl.id)) return;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Free layout
    neko_dyn_array_free(pip->layout);

    // Erase handles from slot arrays
    neko_slot_array_erase(ogl->pipelines, hndl.id);
}

// Resource Query
NEKO_API_DECL void neko_render_pipeline_desc_query(neko_handle(neko_render_pipeline_t) hndl, neko_render_pipeline_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, hndl.id);

    // Add states
    out->blend = pip->blend;
    out->depth = pip->depth;
    out->raster = pip->raster;
    out->stencil = pip->stencil;
    out->compute = pip->compute;

    // Add layout
    u32 ct = neko_dyn_array_size(pip->layout);
    for (u32 i = 0; i < ct; ++i) {
        neko_dyn_array_push(out->layout.attrs, pip->layout[i]);
    }
}

NEKO_API_DECL void neko_render_texture_desc_query(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* out) {
    if (!out) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);

    // Read back pixels
    if (out->data && out->read.width && out->read.height) {
        u32 type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
        u32 format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
        // CHECK_GL_CORE(glActiveTexture(GL_TEXTURE0); glGetTextureSubImage(tex->id, 0, out->read.x, out->read.y, 0, out->read.width, out->read.height, 1, format, type, out->read.size, out->data););
    }

    *out = tex->desc;
}

NEKO_API_DECL size_t neko_render_uniform_size_query(neko_handle(neko_render_uniform_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_uniform_list_t* u = neko_slot_array_getp(ogl->uniforms, hndl.id);
    return u->uniforms[0].size;
}

// Resource Updates (main thread only)
NEKO_API_DECL void neko_render_texture_update_impl(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc) {
    if (!desc) return;

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        NEKO_WARN("Texture handle invalid: %zu", hndl.id);
        return;
    }
    gl_texture_update_internal(desc, hndl.id);
}

NEKO_API_DECL void neko_render_vertex_buffer_update_impl(neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t* desc) {
    /*
    void __neko_render_update_buffer_internal(neko_command_buffer_t* cb,
        u32 id,
        neko_render_buffer_type type,
        neko_render_buffer_usage_type usage,
        size_t sz,
        size_t offset,
        neko_render_buffer_update_type update_type,
        void* data)
    {
        // Write command
        neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
        cb->num_commands++;

        // Write handle id
        neko_byte_buffer_write(&cb->commands, u32, id);
        // Write type
        neko_byte_buffer_write(&cb->commands, neko_render_buffer_type, type);
        // Write usage
        neko_byte_buffer_write(&cb->commands, neko_render_buffer_usage_type, usage);
        // Write data size
        neko_byte_buffer_write(&cb->commands, size_t, sz);
        // Write data offset
        neko_byte_buffer_write(&cb->commands, size_t, offset);
        // Write data update type
        neko_byte_buffer_write(&cb->commands, neko_render_buffer_update_type, update_type);
        // Write data
        neko_byte_buffer_write_bulk(&cb->commands, data, sz);
    }
    __neko_render_update_buffer_internal(cb, hndl.id, R_BUFFER_VERTEX, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, hndl.id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    switch (desc->update.type) {
        case R_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_render_index_buffer_update_impl(neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, hndl.id);
    s32 glusage = neko_gl_buffer_usage_to_gl_enum(desc->usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
    switch (desc->update.type) {
        case R_BUFFER_UPDATE_SUBDATA:
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, desc->update.offset, desc->size, desc->data);
            break;
        default:
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, glusage);
            break;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

NEKO_API_DECL void neko_render_storage_buffer_update_impl(neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    if (!sbo) {
        NEKO_WARN("Storage buffer %zu not found.", hndl.id);
        return;
    }

    // CHECK_GL_CORE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, sbo->buffer); switch (desc->update.type) {
    //     case R_BUFFER_UPDATE_SUBDATA:
    //         glBufferSubData(GL_SHADER_STORAGE_BUFFER, desc->update.offset, desc->size, desc->data);
    //         break;
    //     default:
    //         glBufferData(GL_SHADER_STORAGE_BUFFER, desc->size, desc->data, neko_gl_buffer_usage_to_gl_enum(desc->usage));
    // } glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    //               glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0););
}

NEKO_API_DECL void neko_render_texture_read_impl(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!desc) return;
    if (!neko_slot_array_handle_valid(ogl->textures, hndl.id)) {
        NEKO_WARN("Texture handle invalid: %zu", hndl.id);
    }

    neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, hndl.id);
    // Bind texture
    GLenum target = 0x00;
    switch (tex->desc.type) {
        default:
        case R_TEXTURE_2D: {
            target = GL_TEXTURE_2D;
        } break;
        case R_TEXTURE_CUBEMAP: {
            target = GL_TEXTURE_CUBE_MAP;
        } break;
    }
    u32 gl_format = neko_gl_texture_format_to_gl_texture_format(tex->desc.format);
    u32 gl_type = neko_gl_texture_format_to_gl_data_type(tex->desc.format);
    glBindTexture(target, tex->id);
    glReadPixels(desc->read.x, desc->read.y, desc->read.width, desc->read.height, gl_format, gl_type, *desc->data);
    glBindTexture(target, 0x00);
}

NEKO_API_DECL void* neko_render_storage_buffer_map_get_impl(neko_handle(neko_render_storage_buffer_t) hndl) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        NEKO_WARN("Storage buffer handle invalid: %zu", hndl.id);
        return NULL;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);
    return sbo->map;
}

NEKO_API_DECL void neko_render_storage_buffer_unlock_impl(neko_handle(neko_render_storage_buffer_t) hndl) {
    // 解锁并返回映射指针
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        NEKO_WARN("Storage buffer handle invalid: %zu", hndl.id);
        return;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // 已经解锁
    if (sbo->sync) {
        while (1) {
            GLenum wait = glClientWaitSync(sbo->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
                break;
            }
        }
    }
}

NEKO_API_DECL void* neko_render_storage_buffer_lock_impl(neko_handle(neko_render_storage_buffer_t) hndl) {
    // 锁
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    if (!neko_slot_array_handle_valid(ogl->storage_buffers, hndl.id)) {
        NEKO_WARN("Storage buffer handle invalid: %zu", hndl.id);
        return NULL;
    }
    neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, hndl.id);

    // 已经锁定
    if (sbo->sync) {
        glDeleteSync(sbo->sync);
    }
    sbo->sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    if (sbo->sync) {
        while (1) {
            GLenum wait = glClientWaitSync(sbo->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED) {
                break;
            }
        }
    }
    return sbo->map;
}

#define __ogl_push_command(CB, OP_CODE, ...)                                       \
    do {                                                                           \
        neko_gl_data_t* DATA = (neko_gl_data_t*)neko_subsystem(render)->user_data; \
        neko_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE);                  \
        __VA_ARGS__                                                                \
        CB->num_commands++;                                                        \
    } while (0)

/* Command Buffer Ops: Pipeline / Pass / Bind / Draw */
NEKO_API_DECL void neko_render_renderpass_begin(neko_command_buffer_t* cb, neko_handle(neko_render_renderpass_t) hndl) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_BEGIN_RENDER_PASS, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

NEKO_API_DECL void neko_render_renderpass_end(neko_command_buffer_t* cb) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_END_RENDER_PASS,
                       {
                               // Nothing...
                       });
}

NEKO_API_DECL void neko_render_clear(neko_command_buffer_t* cb, neko_render_clear_action_t action) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = 1;
        neko_byte_buffer_write(&cb->commands, u32, count);
        neko_byte_buffer_write(&cb->commands, neko_render_clear_action_t, action);
    });
}

NEKO_API_DECL void neko_render_clear_ex(neko_command_buffer_t* cb, neko_render_clear_desc_t* desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_CLEAR, {
        u32 count = !desc->actions ? 0 : !desc->size ? 1 : (u32)((size_t)desc->size / (size_t)sizeof(neko_render_clear_action_t));
        neko_byte_buffer_write(&cb->commands, u32, count);
        for (u32 i = 0; i < count; ++i) {
            neko_byte_buffer_write(&cb->commands, neko_render_clear_action_t, desc->actions[i]);
        }
    });
}

NEKO_API_DECL void neko_render_set_viewport(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEWPORT, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_render_set_view_scissor(neko_command_buffer_t* cb, u32 x, u32 y, u32 w, u32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_SET_VIEW_SCISSOR, {
        neko_byte_buffer_write(&cb->commands, u32, x);
        neko_byte_buffer_write(&cb->commands, u32, y);
        neko_byte_buffer_write(&cb->commands, u32, w);
        neko_byte_buffer_write(&cb->commands, u32, h);
    });
}

NEKO_API_DECL void neko_render_texture_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t desc) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE);
    cb->num_commands++;

    u32 num_comps = 0;
    size_t data_type_size = 0;
    size_t total_size = 0;
    switch (desc.format) {
        default:
        case R_TEXTURE_FORMAT_RGBA8:
            num_comps = 4;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_RGB8:
            num_comps = 3;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_A8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_R8:
            num_comps = 1;
            data_type_size = sizeof(uint8_t);
            break;
        case R_TEXTURE_FORMAT_R32:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case R_TEXTURE_FORMAT_RGBA16F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_RGBA32F:
            num_comps = 4;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH8:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH16:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH24:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH32F:
            num_comps = 1;
            data_type_size = sizeof(float);
            break;
        case R_TEXTURE_FORMAT_DEPTH24_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(u32);
            break;
        case R_TEXTURE_FORMAT_DEPTH32F_STENCIL8:
            num_comps = 1;
            data_type_size = sizeof(float) + sizeof(uint8_t);
            break;

            // NOTE: Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
            // case R_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
    }
    total_size = desc.width * desc.height * num_comps * data_type_size;
    neko_byte_buffer_write(&cb->commands, u32, hndl.id);
    neko_byte_buffer_write(&cb->commands, neko_render_texture_desc_t, desc);
    neko_byte_buffer_write(&cb->commands, size_t, total_size);
    neko_byte_buffer_write_bulk(&cb->commands, *desc.data, total_size);
}

void __neko_render_update_buffer_internal(neko_command_buffer_t* cb, u32 id, neko_render_buffer_type type, neko_render_buffer_usage_type usage, size_t sz, size_t offset,
                                          neko_render_buffer_update_type update_type, void* data) {
    // Write command
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE);
    cb->num_commands++;

    // Write handle id
    neko_byte_buffer_write(&cb->commands, u32, id);
    // Write type
    neko_byte_buffer_write(&cb->commands, neko_render_buffer_type, type);
    // Write usage
    neko_byte_buffer_write(&cb->commands, neko_render_buffer_usage_type, usage);
    // Write data size
    neko_byte_buffer_write(&cb->commands, size_t, sz);
    // Write data offset
    neko_byte_buffer_write(&cb->commands, size_t, offset);
    // Write data update type
    neko_byte_buffer_write(&cb->commands, neko_render_buffer_update_type, update_type);
    // Write data
    neko_byte_buffer_write_bulk(&cb->commands, data, sz);
}

NEKO_API_DECL void neko_render_vertex_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_render_update_buffer_internal(cb, hndl.id, R_BUFFER_VERTEX, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

NEKO_API_DECL void neko_render_index_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_render_update_buffer_internal(cb, hndl.id, R_BUFFER_INDEX, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

NEKO_API_DECL void neko_render_uniform_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_uniform_buffer_t) hndl, neko_render_uniform_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_render_update_buffer_internal(cb, hndl.id, R_BUFFER_UNIFORM, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

NEKO_API_DECL void neko_render_storage_buffer_request_update(neko_command_buffer_t* cb, neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t desc) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Return if handle not valid
    if (!hndl.id) return;

    __neko_render_update_buffer_internal(cb, hndl.id, R_BUFFER_SHADER_STORAGE, desc.usage, desc.size, desc.update.offset, desc.update.type, desc.data);
}

void neko_render_apply_bindings(neko_command_buffer_t* cb, neko_render_bind_desc_t* binds) {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Increment commands
    neko_byte_buffer_write(&cb->commands, u32, (u32)NEKO_OPENGL_OP_APPLY_BINDINGS);
    cb->num_commands++;

    // __ogl_push_command(cb, NEKO_OPENGL_OP_APPLY_BINDINGS,
    {
        // Get counts from buffers
        u32 vct = binds->vertex_buffers.desc ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(neko_render_bind_vertex_buffer_desc_t) : 1 : 0;
        u32 ict = binds->index_buffers.desc ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(neko_render_bind_index_buffer_desc_t) : 1 : 0;
        u32 uct = binds->uniform_buffers.desc ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(neko_render_bind_uniform_buffer_desc_t) : 1 : 0;
        u32 pct = binds->uniforms.desc ? binds->uniforms.size ? binds->uniforms.size / sizeof(neko_render_bind_uniform_desc_t) : 1 : 0;
        u32 ibc = binds->image_buffers.desc ? binds->image_buffers.size ? binds->image_buffers.size / sizeof(neko_render_bind_image_buffer_desc_t) : 1 : 0;
        u32 sbc = binds->storage_buffers.desc ? binds->storage_buffers.size ? binds->storage_buffers.size / sizeof(neko_render_bind_storage_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        u32 ct = vct + ict + uct + pct + ibc + sbc;
        neko_byte_buffer_write(&cb->commands, u32, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        neko_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (u32 i = 0; i < vct; ++i) {
            neko_render_bind_vertex_buffer_desc_t* decl = &binds->vertex_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_VERTEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, size_t, decl->offset);
            neko_byte_buffer_write(&cb->commands, neko_render_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (u32 i = 0; i < ict; ++i) {
            neko_render_bind_index_buffer_desc_t* decl = &binds->index_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_INDEX_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
        }

        // Uniform buffers
        for (u32 i = 0; i < uct; ++i) {
            neko_render_bind_uniform_buffer_desc_t* decl = &binds->uniform_buffers.desc[i];

            u32 id = decl->buffer.id;
            size_t sz = (size_t)(neko_slot_array_getp(ogl->uniform_buffers, id))->size;
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_UNIFORM_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.offset);
            neko_byte_buffer_write(&cb->commands, size_t, decl->range.size);
        }

        // Image buffers
        for (u32 i = 0; i < ibc; ++i) {
            neko_render_bind_image_buffer_desc_t* decl = &binds->image_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_IMAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->tex.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write(&cb->commands, neko_render_access_type, decl->access);
        }

        // Uniforms
        for (u32 i = 0; i < pct; ++i) {
            neko_render_bind_uniform_desc_t* decl = &binds->uniforms.desc[i];

            // Get size from uniform list
            size_t sz = neko_slot_array_getp(ogl->uniforms, decl->uniform.id)->size;
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_UNIFORM);
            neko_byte_buffer_write(&cb->commands, u32, decl->uniform.id);
            neko_byte_buffer_write(&cb->commands, size_t, sz);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
            neko_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
        }

        // Storage buffers
        CHECK_GL_CORE(for (u32 i = 0; i < sbc; ++i) {
            neko_render_bind_storage_buffer_desc_t* decl = &binds->storage_buffers.desc[i];
            neko_byte_buffer_write(&cb->commands, neko_render_bind_type, R_BIND_STORAGE_BUFFER);
            neko_byte_buffer_write(&cb->commands, u32, decl->buffer.id);
            neko_byte_buffer_write(&cb->commands, u32, decl->binding);
        });
    };
}

void neko_render_pipeline_bind(neko_command_buffer_t* cb, neko_handle(neko_render_pipeline_t) hndl) {
    // TODO: 不确定这将来是否安全 因为管道的数据位于主线程上 并且可能会在单独的线程上被篡改
    __ogl_push_command(cb, NEKO_OPENGL_OP_BIND_PIPELINE, { neko_byte_buffer_write(&cb->commands, u32, hndl.id); });
}

void neko_render_draw(neko_command_buffer_t* cb, neko_render_draw_desc_t desc) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW, {
        neko_byte_buffer_write(&cb->commands, u32, desc.start);
        neko_byte_buffer_write(&cb->commands, u32, desc.count);
        neko_byte_buffer_write(&cb->commands, u32, desc.instances);
        neko_byte_buffer_write(&cb->commands, u32, desc.base_vertex);
        neko_byte_buffer_write(&cb->commands, u32, desc.range.start);
        neko_byte_buffer_write(&cb->commands, u32, desc.range.end);
    });
}

void neko_render_draw_batch(neko_command_buffer_t* cb, neko_render_batch_context_t* ctx, neko_render_batch_framebuffer_t* fb, s32 w, s32 h) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DRAW_BATCH, {
        neko_byte_buffer_write(&cb->commands, neko_render_batch_context_ptr, ctx);
        neko_byte_buffer_write(&cb->commands, neko_render_batch_framebuffer_ptr, fb);
        neko_byte_buffer_write(&cb->commands, s32, w);
        neko_byte_buffer_write(&cb->commands, s32, h);
    });
}

void neko_render_dispatch_compute(neko_command_buffer_t* cb, u32 num_x_groups, u32 num_y_groups, u32 num_z_groups) {
    __ogl_push_command(cb, NEKO_OPENGL_OP_DISPATCH_COMPUTE, {
        neko_byte_buffer_write(&cb->commands, u32, num_x_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_y_groups);
        neko_byte_buffer_write(&cb->commands, u32, num_z_groups);
    });
}

/* Submission (Main Thread) */
void neko_render_command_buffer_submit_impl(neko_command_buffer_t* cb) {
    /*
        // Structure of command:
            - Op code
            - Data packet
    */

    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

    // Set read position of buffer to beginning
    neko_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    NEKO_FOR_RANGE(cb->num_commands) {
        // Read in op code of command
        neko_byte_buffer_readc(&cb->commands, neko_opengl_op_code_type, op_code);

        switch (op_code) {
            case NEKO_OPENGL_OP_BEGIN_RENDER_PASS: {
                // Bind render pass stuff
                neko_byte_buffer_readc(&cb->commands, u32, rpid);

                // If render pass exists, then we'll bind frame buffer and attachments
                if (rpid && neko_slot_array_exists(ogl->renderpasses, rpid)) {
                    neko_gl_renderpass_t* rp = neko_slot_array_getp(ogl->renderpasses, rpid);

                    // Bind frame buffer since it actually exists
                    if (rp->fbo.id && neko_slot_array_exists(ogl->frame_buffers, rp->fbo.id)) {
                        // Bind frame buffer
                        glBindFramebuffer(GL_FRAMEBUFFER, neko_slot_array_get(ogl->frame_buffers, rp->fbo.id).id);

                        // Bind color attachments
                        for (u32 r = 0; r < neko_dyn_array_size(rp->color); ++r) {
                            u32 cid = rp->color[r].id;
                            if (cid && neko_slot_array_exists(ogl->textures, cid)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, cid);

                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + r, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }

                        // Bind depth attachment
                        {
                            u32 depth_id = rp->depth.id;
                            if (depth_id && neko_slot_array_exists(ogl->textures, depth_id)) {
                                neko_gl_texture_t* rt = neko_slot_array_getp(ogl->textures, depth_id);
                                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rt->id, 0);
                            }
                        }
                    }
                }
            } break;

            case NEKO_OPENGL_OP_END_RENDER_PASS: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
                neko_gl_reset_data_cache(&ogl->cache);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                // CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0););
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            } break;

            case NEKO_OPENGL_OP_CLEAR: {
                // Actions
                neko_byte_buffer_readc(&cb->commands, u32, action_count);
                for (u32 j = 0; j < action_count; ++j) {
                    neko_byte_buffer_readc(&cb->commands, neko_render_clear_action_t, action);

                    // No clear
                    if (action.flag & R_CLEAR_NONE) {
                        continue;
                    }

                    u32 bit = 0x00;

                    if (action.flag & R_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & R_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & R_CLEAR_STENCIL || action.flag == 0x00) {
                        bit |= GL_STENCIL_BUFFER_BIT;
                        glStencilMask(~0);
                    }

                    glClear(bit);
                }
            } break;

            case NEKO_OPENGL_OP_SET_VIEWPORT: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glViewport(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_SET_VIEW_SCISSOR: {
                neko_byte_buffer_readc(&cb->commands, u32, x);
                neko_byte_buffer_readc(&cb->commands, u32, y);
                neko_byte_buffer_readc(&cb->commands, u32, w);
                neko_byte_buffer_readc(&cb->commands, u32, h);

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            } break;

            case NEKO_OPENGL_OP_APPLY_BINDINGS: {
                neko_byte_buffer_readc(&cb->commands, u32, ct);

                // Determine if need to clear any previous vertex buffers here
                neko_byte_buffer_readc(&cb->commands, bool, clear_vertex_buffers);

                // Clear previous vertex decls if necessary
                if (clear_vertex_buffers) {
                    neko_dyn_array_clear(ogl->cache.vdecls);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                for (u32 i = 0; i < ct; ++i) {
                    neko_byte_buffer_readc(&cb->commands, neko_render_bind_type, type);
                    switch (type) {
                        case R_BIND_VERTEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            neko_byte_buffer_readc(&cb->commands, size_t, offset);
                            neko_byte_buffer_readc(&cb->commands, neko_render_vertex_data_type, data_type);

                            if (!id || !neko_slot_array_exists(ogl->vertex_buffers, id)) {
                                NEKO_TIMED_ACTION(1000, {
                                    NEKO_WARN("Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            neko_gl_buffer_t vbo = neko_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            neko_gl_vertex_buffer_decl_t vbo_decl = NEKO_DEFAULT_VAL();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            neko_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case R_BIND_INDEX_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, id);

                            if (!neko_slot_array_exists(ogl->index_buffers, id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                                */
                            } else {
                                neko_gl_buffer_t ibo = neko_slot_array_get(ogl->index_buffers, id);

                                // Store in cache
                                ogl->cache.ibo.id = id;
                            }
                        } break;

                        case R_BIND_UNIFORM: {
                            // Get size from uniform list
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read data size for uniform list
                            neko_byte_buffer_readc(&cb->commands, size_t, sz);
                            // Read binding from uniform list (could make this a binding list? not sure how to handle this)
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniforms, id)) {
                                NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Uniform:Uniform %d does not exist.", id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                neko_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_list_t* ul = neko_slot_array_getp(ogl->uniforms, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            for (u32 ui = 0; ui < neko_dyn_array_size(ul->uniforms); ++ui) {
                                neko_gl_uniform_t* u = &ul->uniforms[ui];

                                // Searching for location if not bound or sid doesn't match previous use
                                if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                    if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {

                                        NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Uniform:Shader %d does not exist.", sid); });

                                        // Advance by size of uniform
                                        neko_byte_buffer_advance_position(&cb->commands, sz);
                                        continue;
                                    }

                                    neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                    // Construct temp name, concat with base name + uniform field name
                                    char name[256] = NEKO_DEFAULT_VAL();
                                    memcpy(name, ul->name, 64);
                                    if (u->name) {
                                        neko_snprintfc(UTMP, 256, "%s%s", ul->name, u->name);
                                        memcpy(name, UTMP, 256);
                                    }

                                    // Grab location of uniform based on name
                                    u->location = glGetUniformLocation(shader.id, name ? name : "__EMPTY_UNIFORM_NAME");

                                    if (u->location >= UINT32_MAX) {
                                        NEKO_WARN("Bind Uniform: Uniform not found: \"%s\"", name);
                                        u->location = UINT32_MAX - 1;
                                    }

                                    u->sid = pip->raster.shader.id;
                                }

                                // Switch on uniform type to upload data
                                switch (u->type) {
                                    case NEKO_GL_UNIFORMTYPE_FLOAT: {
                                        // Need to read bulk data for array.
                                        NEKO_ASSERT(u->size == sizeof(float));
                                        neko_dyn_array_clear(ogl->uniform_data.flt);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, float, v);
                                            neko_dyn_array_push(ogl->uniform_data.flt, v);
                                        }
                                        glUniform1fv(u->location, ct, ogl->uniform_data.flt);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_INT: {
                                        NEKO_ASSERT(u->size == sizeof(s32));
                                        neko_dyn_array_clear(ogl->uniform_data.i32);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, s32, v);
                                            neko_dyn_array_push(ogl->uniform_data.i32, v);
                                        }
                                        glUniform1iv(u->location, ct, ogl->uniform_data.i32);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC2: {
                                        NEKO_ASSERT(u->size == sizeof(neko_vec2));
                                        neko_dyn_array_clear(ogl->uniform_data.vec2);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec2, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec2, v);
                                        }
                                        glUniform2fv(u->location, ct, (float*)ogl->uniform_data.vec2);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC3: {
                                        NEKO_ASSERT(u->size == sizeof(neko_vec3));
                                        neko_dyn_array_clear(ogl->uniform_data.vec3);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec3, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec3, v);
                                        }
                                        glUniform3fv(u->location, ct, (float*)ogl->uniform_data.vec3);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_VEC4: {
                                        NEKO_ASSERT(u->size == sizeof(neko_vec4));
                                        neko_dyn_array_clear(ogl->uniform_data.vec4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_vec4, v);
                                            neko_dyn_array_push(ogl->uniform_data.vec4, v);
                                        }
                                        glUniform4fv(u->location, ct, (float*)ogl->uniform_data.vec4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_MAT4: {
                                        NEKO_ASSERT(u->size == sizeof(neko_mat4));
                                        neko_dyn_array_clear(ogl->uniform_data.mat4);
                                        u32 ct = u->count ? u->count : 1;
                                        size_t sz = ct * u->size;
                                        NEKO_FOR_RANGE(ct) {
                                            neko_byte_buffer_readc(&cb->commands, neko_mat4, v);
                                            neko_dyn_array_push(ogl->uniform_data.mat4, v);
                                        }
                                        glUniformMatrix4fv(u->location, ct, false, (float*)ogl->uniform_data.mat4);
                                    } break;

                                    case NEKO_GL_UNIFORMTYPE_SAMPLERCUBE:
                                    case NEKO_GL_UNIFORMTYPE_SAMPLER2D: {
                                        NEKO_ASSERT(u->size == sizeof(neko_handle(neko_render_texture_t)));
                                        u32 ct = u->count ? u->count : 1;
                                        s32 binds[128] = NEKO_DEFAULT_VAL();
                                        for (u32 i = 0; (i < ct && i < 128); ++i)  // Max of 128 texture binds. Get real.
                                        {
                                            neko_byte_buffer_read_bulkc(&cb->commands, neko_handle(neko_render_texture_t), v, u->size);

                                            // Get texture, also need binding, but will worry about that in a bit
                                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, v.id);

                                            // Activate texture slot
                                            glActiveTexture(GL_TEXTURE0 + binding);

                                            // Bind texture
                                            GLenum target = 0x00;
                                            switch (tex->desc.type) {
                                                default:
                                                case R_TEXTURE_2D: {
                                                    target = GL_TEXTURE_2D;
                                                } break;
                                                case R_TEXTURE_CUBEMAP: {
                                                    target = GL_TEXTURE_CUBE_MAP;
                                                } break;
                                            }

                                            glBindTexture(target, tex->id);

                                            binds[i] = (s32)binding++;
                                        }

                                        // Bind uniforms
                                        glUniform1iv(u->location, ct, (s32*)binds);

                                    } break;

                                    default: {
                                        // Shouldn't hit here
                                        neko_println("Assert: Bind Uniform: Invalid uniform type specified.");
                                        NEKO_ASSERT(false);
                                    } break;
                                }
                            }

                        } break;

                        case R_BIND_UNIFORM_BUFFER: {
                            // Read slot id of uniform buffer
                            neko_byte_buffer_readc(&cb->commands, u32, id);
                            // Read binding
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            // Read range offset
                            neko_byte_buffer_readc(&cb->commands, size_t, range_offset);
                            // Read range size
                            neko_byte_buffer_readc(&cb->commands, size_t, range_size);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !neko_slot_array_exists(ogl->uniform_buffers, id)) {
                                NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Uniform Buffer:Uniform %d does not exist.", id); });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                NEKO_TIMED_ACTION(1000, { neko_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id); });
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location.
                            // If UINT32_T max, then must construct and place location, or if shader id doesn't match previously used shader with this uniform
                            // TODO: To avoid constant lookups in this case, allow for shaders to hold uniform handles references instead.
                            if ((u->location == UINT32_MAX && u->location != UINT32_MAX - 1) || u->sid != pip->raster.shader.id) {
                                if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                    NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Uniform Buffer:Shader %d does not exist.", sid); });
                                    continue;
                                }

                                neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                                // Get uniform location based on name and bound shader
                                u->location = glGetUniformBlockIndex(shader.id, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                // Set binding for uniform block
                                glUniformBlockBinding(shader.id, u->location, binding);

                                if (u->location >= UINT32_MAX) {
                                    NEKO_WARN("Bind Uniform Buffer: Uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }

                                u->sid = pip->raster.shader.id;
                            }

                            glBindBufferRange(GL_UNIFORM_BUFFER, binding, u->ubo, range_offset, range_size ? range_size : u->size);

                        } break;

                        case R_BIND_STORAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, sb_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);

                            // Grab storage buffer from id
                            if (!sb_slot_id || !neko_slot_array_exists(ogl->storage_buffers, sb_slot_id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Storage Buffer %d does not exist.", sb_slot_id);
                                });
                                */
                                continue;
                            }

                            // Grab currently bound pipeline (TODO: assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Storage Buffer:Pipeline %d does not exist or is not bound.", ogl->cache.pipeline.id);
                                });
                                */
                                continue;
                            }

                            neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            neko_gl_storage_buffer_t* sbo = neko_slot_array_getp(ogl->storage_buffers, sb_slot_id);

                            // Get bound shader from pipeline (either compute or raster)
                            u32 sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            if (!sid || !neko_slot_array_exists(ogl->shaders, sid)) {
                                /*
                                NEKO_TIMED_ACTION(1000, {
                                    neko_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                });
                                */
                                continue;
                            }

                            neko_gl_shader_t shader = neko_slot_array_get(ogl->shaders, sid);

                            if ((sbo->block_idx == UINT32_MAX && sbo->block_idx != UINT32_MAX - 1)) {
                                // Get uniform location based on name and bound shader
                                // CHECK_GL_CORE({
                                //     sbo->block_idx = glGetProgramResourceIndex(shader, GL_SHADER_STORAGE_BLOCK, sbo->name ? sbo->name : "__EMPTY_BUFFER_NAME");
                                //     s32 params[1];
                                //     GLenum props[1] = {GL_BUFFER_BINDING};
                                //     glGetProgramResourceiv(shader, GL_SHADER_STORAGE_BLOCK, sbo->block_idx, 1, props, 1, NULL, params);
                                //     sbo->location = (u32)params[0];
                                //     NEKO_WARN("Bind Storage Buffer: Binding \"%s\" to location %u, block index: %u, binding: %u", sbo->name, sbo->location, sbo->block_idx, binding);
                                // });

                                if (sbo->block_idx >= UINT32_MAX) {
                                    NEKO_WARN("Bind Storage Buffer: Buffer not found: \"%s\"", sbo->name);
                                    sbo->block_idx = UINT32_MAX - 1;
                                }
                            }

                            if (sbo->block_idx < UINT32_MAX - 1) {
                                // Not sure what this actually does atm...
                                // CHECK_GL_CORE(glShaderStorageBlockBinding(shader, sbo->block_idx, sbo->location););
                            }

                            // This is required
                            // CHECK_GL_CORE(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, sbo->buffer););

                        } break;

                        case R_BIND_IMAGE_BUFFER: {
                            neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                            neko_byte_buffer_readc(&cb->commands, u32, binding);
                            neko_byte_buffer_readc(&cb->commands, neko_render_access_type, access);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                                NEKO_TIMED_ACTION(1000, { NEKO_WARN("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                                continue;
                            }

                            neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                            u32 gl_access = neko_gl_access_type_to_gl_access_type(access);
                            u32 gl_format = neko_gl_texture_format_to_gl_texture_internal_format(tex->desc.format);

                            NEKO_TIMED_ACTION(
                                    60, neko_render_info_t* info = neko_render_info(); if (info->major_version < 4) {
                                        NEKO_ERROR("%s", "OpenGL4 not available, failed to call glBindImageTexture.");
                                        continue;
                                    });

                            // Bind image texture
                            // CHECK_GL_CORE(glBindImageTexture(binding, tex->id, 0, GL_FALSE, 0, gl_access, gl_format);)
                        } break;

                        default:
                            NEKO_ASSERT(false);
                            break;
                    }
                }

            } break;

            case NEKO_OPENGL_OP_BIND_PIPELINE: {
                // Bind pipeline stuff
                neko_byte_buffer_readc(&cb->commands, u32, pipid);

                // Make sure pipeline exists
                if (!pipid || !neko_slot_array_exists(ogl->pipelines, pipid)) {

                    NEKO_TIMED_ACTION(1000, { NEKO_WARN("Pipeline %d does not exist.", pipid); });

                    continue;
                }

                // Reset cache
                neko_gl_reset_data_cache(&ogl->cache);

                // Reset state as well
                neko_gl_pipeline_state();

                /* Cache pipeline id */
                ogl->cache.pipeline = neko_handle_create(neko_render_pipeline_t, pipid);

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, pipid);

                /* Compute */
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id) {
                    /* Shader */
                    if (pip->compute.shader.id && neko_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(neko_slot_array_get(ogl->shaders, pip->compute.shader.id).id);
                    } else {
                        NEKO_TIMED_ACTION(1000, { NEKO_WARN("Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id); });
                    }

                    continue;
                }

                /* Depth */
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                } else {
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(neko_gl_depth_func_to_gl_depth_func(pip->depth.func));
                }
                glDepthMask(neko_gl_depth_mask_to_gl_mask(pip->depth.mask));

                /* Stencil */
                if (!pip->stencil.func) {
                    // If no stencil function (default), then disable
                    glDisable(GL_STENCIL_TEST);
                } else {
                    glEnable(GL_STENCIL_TEST);
                    u32 func = neko_gl_stencil_func_to_gl_stencil_func(pip->stencil.func);
                    u32 sfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.sfail);
                    u32 dpfail = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dpfail);
                    u32 dppass = neko_gl_stencil_op_to_gl_stencil_op(pip->stencil.dppass);
                    glStencilFunc(func, pip->stencil.ref, pip->stencil.comp_mask);
                    glStencilMask(pip->stencil.write_mask);
                    glStencilOp(sfail, dpfail, dppass);
                }

                /* Blend */
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(neko_gl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(neko_gl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), neko_gl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));
                }

                /* Raster */
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);
                } else {
                    glEnable(GL_CULL_FACE);
                    glCullFace(neko_gl_cull_face_to_gl_cull_face(pip->raster.face_culling));
                }

                // Winding order
                glFrontFace(neko_gl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                /* Shader */
                if (pip->raster.shader.id && neko_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(neko_slot_array_get(ogl->shaders, pip->raster.shader.id).id);
                } else {
                    NEKO_TIMED_ACTION(1000, { NEKO_WARN("Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id); });
                }
            } break;

            case NEKO_OPENGL_OP_DISPATCH_COMPUTE: {
                neko_byte_buffer_readc(&cb->commands, u32, num_x_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_y_groups);
                neko_byte_buffer_readc(&cb->commands, u32, num_z_groups);

                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !neko_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    /*
                    NEKO_TIMED_ACTION(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    /*
                    NEKO_TIMED_ACTION(1000, {
                        neko_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    */
                    continue;
                }

                // Dispatch shader
                // CHECK_GL_CORE(
                //         // Memory barrier (TODO: make this specifically set in the pipeline state)
                //         glDispatchCompute(num_x_groups, num_y_groups, num_z_groups); glMemoryBarrier(GL_ALL_BARRIER_BITS);)
            } break;

            case NEKO_OPENGL_OP_DRAW: {
                // Grab currently bound pipeline (TODO: assert if this isn't valid)
                neko_gl_pipeline_t* pip = neko_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // Must have a vertex buffer bound to draw
                if (neko_dyn_array_empty(ogl->cache.vdecls)) {
                    NEKO_TIMED_ACTION(1000, { neko_println("Error:Opengl:Draw: No vertex buffer bound."); });
                    // NEKO_ASSERT(false);
                }

                // Keep track whether or not the data is to be instanced
                bool is_instanced = false;

                for (u32 i = 0; i < neko_dyn_array_size(pip->layout); ++i) {
                    // Vertex buffer to bind
                    u32 vbo_idx = i;  // pip->layout[i].buffer_idx;
                    neko_gl_vertex_buffer_decl_t vdecl = vbo_idx < neko_dyn_array_size(ogl->cache.vdecls) ? ogl->cache.vdecls[vbo_idx] : ogl->cache.vdecls[0];
                    neko_gl_buffer_t vbo = vdecl.vbo;

                    // Manual override. If you manually set divisor/stride/offset, then will not automatically calculate any of those.
                    bool is_manual = pip->layout[i].stride | pip->layout[i].divisor | pip->layout[i].offset | vdecl.data_type == R_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo.id);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : neko_gl_calculate_vertex_size_in_bytes(pip->layout, neko_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == R_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset : is_manual ? pip->layout[i].offset : neko_gl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format) {
                        case R_VERTEX_ATTRIBUTE_FLOAT4:
                            glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT3:
                            glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT2:
                            glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_FLOAT:
                            glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT4:
                            glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT3:
                            glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT2:
                            glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_UINT:
                            glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE:
                            glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE2:
                            glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE3:
                            glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;
                        case R_VERTEX_ATTRIBUTE_BYTE4:
                            glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, NEKO_INT2VOIDP(offset));
                            break;

                        // Shouldn't get here
                        default: {
                            NEKO_ASSERT(false);
                        } break;
                    }
                    // Set up divisor (for instancing)
                    glVertexAttribDivisor(i, pip->layout[i].divisor);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                // Bind all vertex buffers after setting up data and pointers
                for (u32 i = 0; i < neko_dyn_array_size(ogl->cache.vdecls); ++i) {
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo.id);
                }

                // Draw based on bound primitive type in raster
                neko_byte_buffer_readc(&cb->commands, u32, start);
                neko_byte_buffer_readc(&cb->commands, u32, count);
                neko_byte_buffer_readc(&cb->commands, u32, instance_count);
                neko_byte_buffer_readc(&cb->commands, u32, base_vertex);
                neko_byte_buffer_readc(&cb->commands, u32, range_start);
                neko_byte_buffer_readc(&cb->commands, u32, range_end);

                range_end = (range_end && range_end < range_start) ? range_end : count;

                // Bind element buffer ranged
                if (ogl->cache.ibo.id) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, neko_slot_array_get(ogl->index_buffers, ogl->cache.ibo.id).id);
                }

                // If instance count > 1, do instanced drawing
                is_instanced |= (instance_count > 1);

                u32 prim = neko_gl_primitive_to_gl_primitive(pip->raster.primitive);
                u32 itype = neko_gl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo.id) {
#ifdef R_IMPL_OPENGL_CORE
                    if (is_instanced)
                        glDrawElementsInstancedBaseVertex(prim, count, itype, NEKO_INT2VOIDP(start), instance_count, base_vertex);
                    else
                        glDrawRangeElementsBaseVertex(prim, range_start, range_end, count, itype, NEKO_INT2VOIDP(start), base_vertex);
#else
                    if (is_instanced)
                        glDrawElementsInstanced(prim, count, itype, NEKO_INT2VOIDP(start), instance_count);
                    else
                        glDrawElements(prim, count, itype, NEKO_INT2VOIDP(start));
#endif
                } else {
                    if (is_instanced)
                        glDrawArraysInstanced(prim, start, count, instance_count);
                    else
                        glDrawArrays(prim, start, count);
                }

            } break;

            case NEKO_OPENGL_OP_DRAW_BATCH: {
                neko_byte_buffer_readc(&cb->commands, neko_render_batch_context_ptr, batch_ctx);
                neko_byte_buffer_readc(&cb->commands, neko_render_batch_framebuffer_ptr, fb);
                neko_byte_buffer_readc(&cb->commands, s32, w);
                neko_byte_buffer_readc(&cb->commands, s32, h);

                // neko_gl_pipeline_state();

                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                neko_render_batch_flush(batch_ctx, fb, w, h);
            } break;

            case NEKO_OPENGL_OP_REQUEST_TEXTURE_UPDATE: {
                neko_byte_buffer_readc(&cb->commands, u32, tex_slot_id);
                neko_byte_buffer_readc(&cb->commands, neko_render_texture_desc_t, desc);
                neko_byte_buffer_readc(&cb->commands, size_t, data_size);

                // Update texture with data, depending on update type (for now, just stream new data)

                // Grab texture from sampler id
                if (!tex_slot_id || !neko_slot_array_exists(ogl->textures, tex_slot_id)) {
                    NEKO_TIMED_ACTION(60, { NEKO_WARN("Bind Image Buffer: Texture %d does not exist.", tex_slot_id); });
                    neko_byte_buffer_advance_position(&cb->commands, data_size);
                }

                neko_gl_texture_t* tex = neko_slot_array_getp(ogl->textures, tex_slot_id);
                u32 int_format = neko_gl_texture_format_to_gl_texture_internal_format(desc.format);
                u32 format = neko_gl_texture_format_to_gl_texture_format(desc.format);
                u32 dt = neko_gl_texture_format_to_gl_data_type(desc.format);
                *desc.data = (cb->commands.data + cb->commands.position);
                *tex = gl_texture_update_internal(&desc, tex_slot_id);

                // Bind texture
                // glBindTexture(GL_TEXTURE_2D, tex->id);
                // // Update texture data
                // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (cb->commands.data, cb->commands.position));
                // // glTexImage2D(GL_TEXTURE_2D, 0, int_format, desc.width, desc.height, 0, format, dt, (cb->commands.data, cb->commands.position));
                // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.width, desc.height, format, dt, (cb->commands.data + cb->commands.position));
                // glBindTexture(GL_TEXTURE_2D, 0);
                neko_byte_buffer_advance_position(&cb->commands, data_size);
            } break;

            case NEKO_OPENGL_OP_REQUEST_BUFFER_UPDATE: {
                neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;

                // Read handle id
                neko_byte_buffer_readc(&cb->commands, u32, id);
                // Read type
                neko_byte_buffer_readc(&cb->commands, neko_render_buffer_type, type);
                // Read usage
                neko_byte_buffer_readc(&cb->commands, neko_render_buffer_usage_type, usage);
                // Read data size
                neko_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                neko_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                neko_byte_buffer_readc(&cb->commands, neko_render_buffer_update_type, update_type);

                s32 glusage = neko_gl_buffer_usage_to_gl_enum(usage);

                switch (type) {
                    // Vertex Buffer
                    default:
                    case R_BUFFER_VERTEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer.id);
                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case R_BUFFER_INDEX: {
                        neko_gl_buffer_t buffer = neko_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id);
                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA:
                                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                                break;
                            default:
                                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                                break;
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;

                    case R_BUFFER_UNIFORM: {
                        // Have to
                        neko_gl_uniform_buffer_t* u = neko_slot_array_getp(ogl->uniform_buffers, id);

                        glBindBuffer(GL_UNIFORM_BUFFER, u->ubo);

                        switch (update_type) {
                            case R_BUFFER_UPDATE_SUBDATA: {
                                glBufferSubData(GL_UNIFORM_BUFFER, offset, sz, (cb->commands.data + cb->commands.position));
                            } break;
                            default: {
                                // Reset uniform size
                                u->size = sz;
                                // Recreate buffer
                                glBufferData(GL_UNIFORM_BUFFER, sz, (cb->commands.data + cb->commands.position), glusage);
                            } break;
                        }

                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    } break;

                    case R_BUFFER_SHADER_STORAGE: {
                        neko_render_storage_buffer_desc_t desc = {
                                .data = cb->commands.data + cb->commands.position,
                                .size = sz,
                                .usage = usage,
                                .update =
                                        {
                                                .type = update_type,
                                                .offset = offset,
                                        },
                        };
                        neko_handle(neko_render_storage_buffer_t) hndl;
                        hndl.id = id;
                        neko_render_storage_buffer_update(hndl, &desc);

                    } break;
                }

                // Advance past data
                neko_byte_buffer_advance_position(&cb->commands, sz);

            } break;

            default: {
                // Op code not supported yet!
                neko_println("Op code not supported yet: %zu", (u32)op_code);
                NEKO_ASSERT(false);
            }
        }
    }

    // Clear byte buffer of commands
    neko_byte_buffer_clear(&cb->commands);

    // Set num commands to 0
    cb->num_commands = 0;
}

NEKO_API_DECL neko_gl_data_t* neko_render_ogl() {
    neko_gl_data_t* ogl = (neko_gl_data_t*)neko_subsystem(render)->user_data;
    return ogl;
}

NEKO_API_DECL void neko_render_init(neko_render_t* render) {
    // Push back 0 handles into slot arrays (for 0 init validation)
    neko_gl_data_t* ogl = (neko_gl_data_t*)render->user_data;

    neko_slot_array_insert(ogl->shaders, (neko_gl_shader_t){0});
    neko_slot_array_insert(ogl->vertex_buffers, (neko_gl_buffer_t){0});
    neko_slot_array_insert(ogl->index_buffers, (neko_gl_buffer_t){0});
    neko_slot_array_insert(ogl->frame_buffers, (neko_gl_buffer_t){0});

    neko_gl_uniform_list_t ul = NEKO_DEFAULT_VAL();
    neko_gl_uniform_buffer_t ub = NEKO_DEFAULT_VAL();
    neko_gl_pipeline_t pip = NEKO_DEFAULT_VAL();
    neko_gl_renderpass_t rp = NEKO_DEFAULT_VAL();
    neko_gl_texture_t tex = NEKO_DEFAULT_VAL();
    neko_gl_storage_buffer_t sb = NEKO_DEFAULT_VAL();

    neko_slot_array_insert(ogl->uniforms, ul);
    neko_slot_array_insert(ogl->pipelines, pip);
    neko_slot_array_insert(ogl->renderpasses, rp);
    neko_slot_array_insert(ogl->uniform_buffers, ub);
    neko_slot_array_insert(ogl->textures, tex);
    neko_slot_array_insert(ogl->storage_buffers, sb);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao.id);
    glBindVertexArray(ogl->cache.vao.id);

    // Reset data cache for rendering ops
    neko_gl_reset_data_cache(&ogl->cache);

    // Init info object
    neko_render_info_t* info = &neko_subsystem(render)->info;

    // OpenGL 主机信息
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);
    info->version = (const_str)glGetString(GL_VERSION);
    info->vendor = (const_str)glGetString(GL_RENDERER);

    // Max texture units
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&info->max_texture_units);

    // Compute shader info
    info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
    if (info->compute.available) {
        // CHECK_GL_CORE({
        //     // Work group counts
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (s32*)&info->compute.max_work_group_count[0]);
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (s32*)&info->compute.max_work_group_count[1]);
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (s32*)&info->compute.max_work_group_count[2]);
        //     // Work group sizes
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (s32*)&info->compute.max_work_group_size[0]);
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (s32*)&info->compute.max_work_group_size[1]);
        //     glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (s32*)&info->compute.max_work_group_size[2]);
        //     // Work group invocations
        //     glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (s32*)&info->compute.max_work_group_invocations);
        // });
    } else {
        // NEKO_WARN("opengl compute shaders not available");
    }

#if defined(NEKO_DEBUG_BUILD) && 0
    int numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    neko_printf("Supported extensions:");
    for (int i = 0; i < numExtensions; i++) {
        const GLubyte* extension = glGetStringi(GL_EXTENSIONS, i);
        neko_printf("%s;", extension);
    }
    neko_printf("\n");
#endif

    // 设置Render上下文的所有函数指针

    // Create
    render->api.texture_create = neko_render_texture_create_impl;
    render->api.uniform_create = neko_render_uniform_create_impl;
    render->api.shader_create = neko_render_shader_create_impl;
    render->api.vertex_buffer_create = neko_render_vertex_buffer_create_impl;
    render->api.index_buffer_create = neko_render_index_buffer_create_impl;
    render->api.uniform_buffer_create = neko_render_uniform_buffer_create_impl;
    render->api.storage_buffer_create = neko_render_storage_buffer_create_impl;
    render->api.framebuffer_create = neko_render_framebuffer_create_impl;
    render->api.renderpass_create = neko_render_renderpass_create_impl;
    render->api.pipeline_create = neko_render_pipeline_create_impl;

    // Destroy
    render->api.texture_destroy = neko_render_texture_destroy_impl;
    render->api.uniform_destroy = neko_render_uniform_destroy_impl;
    render->api.shader_destroy = neko_render_shader_destroy_impl;
    render->api.vertex_buffer_destroy = neko_render_vertex_buffer_destroy_impl;
    render->api.index_buffer_destroy = neko_render_index_buffer_destroy_impl;
    render->api.uniform_buffer_destroy = neko_render_uniform_buffer_destroy_impl;
    render->api.storage_buffer_destroy = neko_render_storage_buffer_destroy_impl;
    render->api.framebuffer_destroy = neko_render_framebuffer_destroy_impl;
    render->api.renderpass_destroy = neko_render_renderpass_destroy_impl;
    render->api.pipeline_destroy = neko_render_pipeline_destroy_impl;

    // Resource Updates (main thread only)
    render->api.vertex_buffer_update = neko_render_vertex_buffer_update_impl;
    render->api.index_buffer_update = neko_render_index_buffer_update_impl;
    render->api.storage_buffer_update = neko_render_storage_buffer_update_impl;
    render->api.texture_update = neko_render_texture_update_impl;
    render->api.texture_read = neko_render_texture_read_impl;

    // Util
    render->api.storage_buffer_map_get = neko_render_storage_buffer_map_get_impl;
    render->api.storage_buffer_lock = neko_render_storage_buffer_lock_impl;
    render->api.storage_buffer_unlock = neko_render_storage_buffer_unlock_impl;

    // Submission (Main Thread)
    render->api.command_buffer_submit = neko_render_command_buffer_submit_impl;
}

#endif  // R_IMPL_OPENGL

NEKO_API_DECL neko_gl_data_t* neko_render_userdata() { return neko_render_ogl(); }

// 资源构造函数
NEKO_API_DECL neko_handle(neko_render_texture_t) neko_render_texture_create(const neko_render_texture_desc_t desc) { return neko_render()->api.texture_create(desc); }

NEKO_API_DECL neko_handle(neko_render_uniform_t) neko_render_uniform_create(const neko_render_uniform_desc_t desc) { return neko_render()->api.uniform_create(desc); }

NEKO_API_DECL neko_handle(neko_render_shader_t) neko_render_shader_create(const neko_render_shader_desc_t desc) { return neko_render()->api.shader_create(desc); }

NEKO_API_DECL neko_handle(neko_render_vertex_buffer_t) neko_render_vertex_buffer_create(const neko_render_vertex_buffer_desc_t desc) { return neko_render()->api.vertex_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_render_index_buffer_t) neko_render_index_buffer_create(const neko_render_index_buffer_desc_t desc) { return neko_render()->api.index_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_render_uniform_buffer_t) neko_render_uniform_buffer_create(const neko_render_uniform_buffer_desc_t desc) { return neko_render()->api.uniform_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_render_storage_buffer_t) neko_render_storage_buffer_create(const neko_render_storage_buffer_desc_t desc) { return neko_render()->api.storage_buffer_create(desc); }

NEKO_API_DECL neko_handle(neko_render_framebuffer_t) neko_render_framebuffer_create(const neko_render_framebuffer_desc_t desc) { return neko_render()->api.framebuffer_create(desc); }

NEKO_API_DECL neko_handle(neko_render_renderpass_t) neko_render_renderpass_create(const neko_render_renderpass_desc_t desc) { return neko_render()->api.renderpass_create(desc); }

NEKO_API_DECL neko_handle(neko_render_pipeline_t) neko_render_pipeline_create(const neko_render_pipeline_desc_t desc) { return neko_render()->api.pipeline_create(desc); }

// Destroy
NEKO_API_DECL void neko_render_texture_destroy(neko_handle(neko_render_texture_t) hndl) { neko_render()->api.texture_destroy(hndl); }

NEKO_API_DECL void neko_render_uniform_destroy(neko_handle(neko_render_uniform_t) hndl) { neko_render()->api.uniform_destroy(hndl); }

NEKO_API_DECL void neko_render_shader_destroy(neko_handle(neko_render_shader_t) hndl) { neko_render()->api.shader_destroy(hndl); }

NEKO_API_DECL void neko_render_vertex_buffer_destroy(neko_handle(neko_render_vertex_buffer_t) hndl) { neko_render()->api.vertex_buffer_destroy(hndl); }

NEKO_API_DECL void neko_render_index_buffer_destroy(neko_handle(neko_render_index_buffer_t) hndl) { neko_render()->api.index_buffer_destroy(hndl); }

NEKO_API_DECL void neko_render_uniform_buffer_destroy(neko_handle(neko_render_uniform_buffer_t) hndl) { neko_render()->api.uniform_buffer_destroy(hndl); }

NEKO_API_DECL void neko_render_storage_buffer_destroy(neko_handle(neko_render_storage_buffer_t) hndl) { neko_render()->api.storage_buffer_destroy(hndl); }

NEKO_API_DECL void neko_render_framebuffer_destroy(neko_handle(neko_render_framebuffer_t) hndl) { neko_render()->api.framebuffer_destroy(hndl); }

NEKO_API_DECL void neko_render_renderpass_destroy(neko_handle(neko_render_renderpass_t) hndl) { neko_render()->api.renderpass_destroy(hndl); }

NEKO_API_DECL void neko_render_pipeline_destroy(neko_handle(neko_render_pipeline_t) hndl) { neko_render()->api.pipeline_destroy(hndl); }

// 资源更新 (仅限主线程)
NEKO_API_DECL void neko_render_vertex_buffer_update(neko_handle(neko_render_vertex_buffer_t) hndl, neko_render_vertex_buffer_desc_t* desc) { neko_render()->api.vertex_buffer_update(hndl, desc); }

NEKO_API_DECL void neko_render_index_buffer_update(neko_handle(neko_render_index_buffer_t) hndl, neko_render_index_buffer_desc_t* desc) { neko_render()->api.index_buffer_update(hndl, desc); }

NEKO_API_DECL void neko_render_storage_buffer_update(neko_handle(neko_render_storage_buffer_t) hndl, neko_render_storage_buffer_desc_t* desc) { neko_render()->api.storage_buffer_update(hndl, desc); }

NEKO_API_DECL void neko_render_texture_update(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc) { neko_render()->api.texture_update(hndl, desc); }

NEKO_API_DECL void neko_render_texture_read(neko_handle(neko_render_texture_t) hndl, neko_render_texture_desc_t* desc) { neko_render()->api.texture_read(hndl, desc); }

NEKO_API_DECL void* neko_render_storage_buffer_map_get(neko_handle(neko_render_storage_buffer_t) hndl) { return neko_render()->api.storage_buffer_map_get(hndl); }

NEKO_API_DECL void neko_render_storage_buffer_unlock(neko_handle(neko_render_storage_buffer_t) hndl) { return neko_render()->api.storage_buffer_unlock(hndl); }

NEKO_API_DECL void* neko_render_storage_buffer_lock(neko_handle(neko_render_storage_buffer_t) hndl) { return neko_render()->api.storage_buffer_lock(hndl); }