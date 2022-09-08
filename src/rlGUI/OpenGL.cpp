#include "rl/lib/rlGUI/OpenGL.hpp"

namespace lib = rl::GUI;



std::map<HDC, lib::OpenGL*> lib::OpenGL::s_oInstances;
lib::OpenGL* lib::OpenGL::s_pCurrent = nullptr;
const PIXELFORMATDESCRIPTOR lib::OpenGL::s_oPFD =
{
	sizeof(PIXELFORMATDESCRIPTOR), 1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	PFD_MAIN_PLANE, 0, 0, 0, 0
};
std::function<BOOL(int)> lib::OpenGL::s_fnWglSwapIntervalEXT;



lib::OpenGL* lib::OpenGL::CreateInstance(HDC hDC, GLsizei iWidth, GLsizei iHeight)
{
	try
	{
		// check for already existing instance
		const auto it = s_oInstances.find(hDC);
		if (it != s_oInstances.end())
			return it->second;

		s_oInstances[hDC] = new lib::OpenGL(hDC, iWidth, iHeight);
		auto& oInstance = *s_oInstances.at(hDC);
		oInstance.setSize(iWidth, iHeight);

		// not the first rendering context --> share list with previous instance
		if (s_oInstances.size() > 1)
		{
			for (const auto& it : s_oInstances)
			{
				if (it.first == hDC)
					continue;

				// todo: delete all previous textures (rebuild on painting them)
				wglShareLists(oInstance.getRenderingContext(), it.second->getRenderingContext());
				break;
			}
		}
		return &oInstance;
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Exception", MB_ICONERROR | MB_SYSTEMMODAL);
		return nullptr;
	}
}

void lib::OpenGL::DestroyInstance(HDC hDC)
{
	const auto it = s_oInstances.find(hDC);
	if (it == s_oInstances.end())
		return; // no instance

	auto pInstance = it->second;
	s_oInstances.erase(it);
	delete pInstance;
}

lib::OpenGL* lib::OpenGL::GetInstance(HDC hDC)
{
	const auto it = s_oInstances.find(hDC);

	if (it == s_oInstances.end())
		return nullptr;

	return it->second;
}

lib::OpenGL::~OpenGL()
{
	deleteAllTextures();
	s_oInstances.erase(m_hDC);
	wglDeleteContext(m_hGLRC);
}

void lib::OpenGL::makeCurrent()
{
	if (s_pCurrent == this)
		return; // is already current

	s_pCurrent = this;
	if (!wglMakeCurrent(m_hDC, m_hGLRC))
		throw std::exception("rl::GUI::OpenGL: Failed to make OpenGL context current");
}

void lib::OpenGL::setSize(GLsizei iWidth, GLsizei iHeight)
{
	makeCurrent();

	if (m_iWidth == iWidth && m_iHeight == iHeight)
		return; // no change

	
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	glViewport(0, 0, m_iWidth, m_iHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, m_iWidth, m_iHeight, 0.0f, 0.0f, 1.0f);
}

GLuint lib::OpenGL::createTexture()
{
	makeCurrent();
	GLuint iResult;
	glGenTextures(1, &iResult);
	m_oTextureIDs.insert(iResult);
	return iResult;
}

void lib::OpenGL::createTextures(GLuint* pIDs, GLsizei count)
{
	makeCurrent();
	glGenTextures(count, pIDs);

	for (size_t i = 0; i < count; ++i)
	{
		m_oTextureIDs.insert(pIDs[i]);
	}
}

void lib::OpenGL::deleteTexture(GLuint iID)
{
	makeCurrent();

	if (!m_oTextureIDs.contains(iID))
		return; // texture doesn't exist


	m_oTextureIDs.erase(iID);
	glDeleteTextures(1, &iID);
}

void lib::OpenGL::deleteTextures(const GLuint* pIDs, GLsizei count)
{
	makeCurrent();

	for (size_t i = 0; i < count; ++i)
	{
		if (m_oTextureIDs.contains(pIDs[i]))
			m_oTextureIDs.erase(pIDs[i]);
	}
	glDeleteTextures(count, pIDs);
}

void lib::OpenGL::deleteAllTextures()
{
	makeCurrent();

	for (auto iTexture : m_oTextureIDs)
	{
		glDeleteTextures(1, &iTexture);
	}
	m_oTextureIDs.clear();
}

lib::OpenGL::OpenGL(HDC hDC, GLsizei iWidth, GLsizei iHeight) :
	m_hDC(hDC), m_iWidth(iWidth), m_iHeight(iHeight)
{
	int pf = ChoosePixelFormat(m_hDC, &s_oPFD);
	if (!SetPixelFormat(m_hDC, pf, &s_oPFD))
		throw std::exception("rl::GUI::OpenGL: Failed to set pixel format");

	m_hGLRC = wglCreateContext(m_hDC);
	if (m_hGLRC == NULL)
		throw std::exception("rl::GUI::OpenGL: Failed to create OpenGL context");

	makeCurrent();



	if (s_fnWglSwapIntervalEXT == nullptr)
	{
		s_fnWglSwapIntervalEXT = reinterpret_cast<BOOL(__stdcall*)(int)>(
			wglGetProcAddress("wglSwapIntervalEXT"));

		if (s_fnWglSwapIntervalEXT == nullptr)
			throw std::exception(
				"rl::GUI::OpenGL: Failed to get the address of wglSwapIntervalEXT");
	}

	s_fnWglSwapIntervalEXT(1);

	glViewport(0, 0, m_iWidth, m_iHeight);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, m_iWidth, m_iHeight, 0.0f, 0.0f, 1.0f);
}
