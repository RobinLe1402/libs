#pragma once
#ifndef ROBINLE_LIB_GUI_OPENGL
#define ROBINLE_LIB_GUI_OPENGL





#include <functional>
#include <map>
#include <set>

#include <Windows.h>
#include <gl/GL.h>



namespace rl
{
	namespace GUI
	{

		class OpenGL final
		{
		public: // static methods

			static OpenGL* CreateInstance(HDC hDC, GLsizei iWidth, GLsizei iHeight);
			static void DestroyInstance(HDC hDC);

			static OpenGL* GetCurrent() { return s_pCurrent; }
			static OpenGL* GetInstance(HDC hDC);


		public: // methods

			~OpenGL();

			auto getDC() const { return m_hDC; }
			auto getRenderingContext() { return m_hGLRC; }

			void makeCurrent();
			void setSize(GLsizei iWidth, GLsizei iHeight);
			auto getWidth() const { return m_iWidth; }
			auto getHeight() const { return m_iHeight; }

			GLuint createTexture();
			void createTextures(GLuint* pIDs, GLsizei count);

			void deleteTexture(GLuint iID);
			void deleteTextures(const GLuint* pIDs, GLsizei count);
			void deleteAllTextures();


		private: // methods

			OpenGL(HDC hDC, GLsizei iWidth, GLsizei iHeight);


		private: // variables

			HDC m_hDC;
			HGLRC m_hGLRC = NULL;
			std::set<GLuint> m_oTextureIDs;
			GLsizei m_iWidth, m_iHeight;


		private: // static variables

			static std::map<HDC, OpenGL*> s_oInstances;
			static OpenGL* s_pCurrent;
			static const PIXELFORMATDESCRIPTOR s_oPFD;
			static std::function<BOOL(int interval)> s_fnWglSwapIntervalEXT;
		};

	}
}





#endif // ROBINLE_LIB_GUI_OPENGL