#include "../../rlOpenGLWin.hpp"

#include "resource.h"

#include <Windows.h>
#include <gl/GL.h>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


struct pixel
{
	union
	{
		uint32_t n;
		struct
		{
			uint8_t r;
			uint8_t g;
			uint8_t b;
			uint8_t a;
		};
	};
};

class OpenGLImage
{
private:

	Gdiplus::GdiplusStartupInput m_gpSi = 0;
	ULONG_PTR m_gpTk = NULL;

	uint32_t* m_pData = nullptr;
	uint32_t m_iWidth = 0, m_iHeight = 0;


	static uint32_t WinToOGL(uint32_t iCol)
	{
		// iCol = ARGB
		// result = RGBA

		pixel px;

		px.a = iCol >> 24;
		px.r = iCol >> 16;
		px.g = iCol >> 8;
		px.b = iCol;

		return px.n;
	}


public:

	OpenGLImage(HINSTANCE hInstance, const WCHAR* szResourceName)
	{
		Gdiplus::GdiplusStartup(&m_gpTk, &m_gpSi, NULL);

		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromResource(hInstance, szResourceName);

		m_iWidth = bmp->GetWidth();
		m_iHeight = bmp->GetHeight();

		m_pData = new uint32_t[(size_t)m_iWidth * m_iHeight];

		for (uint32_t iX = 0; iX < m_iWidth; iX++)
		{
			for (uint32_t iY = 0; iY < m_iHeight; iY++)
			{
				Gdiplus::Color col;
				bmp->GetPixel(iX, iY, &col);
				m_pData[iY * m_iWidth + iX] = WinToOGL(col.GetValue());
			}
		}

		delete bmp;
	}

	OpenGLImage(const wchar_t* szPath)
	{
		Gdiplus::GdiplusStartup(&m_gpTk, &m_gpSi, NULL);

		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(szPath);
		m_iWidth = bmp->GetWidth();
		m_iHeight = bmp->GetHeight();

		m_pData = new uint32_t[(size_t)m_iWidth * m_iHeight];

		for (uint32_t iX = 0; iX < m_iWidth; iX++)
		{
			for (uint32_t iY = 0; iY < m_iHeight; iY++)
			{
				Gdiplus::Color col;
				bmp->GetPixel(iX, iY, &col);
				m_pData[iY * m_iWidth + iX] = WinToOGL(col.GetValue());
			}
		}

		delete bmp;
	}

	~OpenGLImage()
	{
		Gdiplus::GdiplusShutdown(m_gpTk);

		delete[] m_pData;
	}

	inline uint32_t getWidth() { return m_iWidth; }
	inline uint32_t getHeight() { return m_iHeight; }

	inline uint32_t* getData() { return m_pData; }

};





class TestWin : public rl::OpenGLWin
{
public:

	TestWin(HINSTANCE hInstance) : m_hInstance(hInstance) {}
	virtual ~TestWin() {}


private:

	HINSTANCE m_hInstance;

	GLuint m_iTex = 0;
	GLuint m_iPx = 0;
	double m_dHeight = 1;
	bool m_bHeightInc = false;
	bool m_bUpsideDown = false;
	bool m_bAnim = false;
	bool m_bAnimRunning = false;

	const double dScaleFactor = 1; // scaling per second

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONUP: // toggle image "rotation"
			m_bAnim = !m_bAnim;
			if (m_bAnim)
				m_bAnimRunning = true;
			break;

		case WM_KEYUP:
			if (wParam == 'F') // toggle fullscreen
			{
				if (getFullscreen())
					setWindowed();
				else
					setFullscreen(MonitorFromWindow(getHWND(), MONITOR_DEFAULTTONEAREST));
			}
			return true;
		}
		return false;
	}

	bool OnCreate() override
	{
		OpenGLImage img(m_hInstance, MAKEINTRESOURCEW(IDB_TEXTURE));

		uint32_t iColor[] = { 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000 };

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &m_iTex);
		glBindTexture(GL_TEXTURE_2D, m_iTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_RGBA,
			GL_UNSIGNED_BYTE, img.getData());



		// draw pixel block to test pixel coordinate calculation
		glGenTextures(1, &m_iPx);
		glBindTexture(GL_TEXTURE_2D, m_iPx);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

		pixel px;
		px.a = 0xFF;
		px.r = 0xFF;
		px.g = 0;
		px.b = 0;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &px);

		//setWindowedSize(img.getWidth(), img.getHeight());



		OnUpdate(0);

		return true;
	}

	bool OnDestroy() override
	{
		glDeleteTextures(1, &m_iTex);
		glDeleteTextures(1, &m_iPx);

		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



		if (m_bAnimRunning)
		{
			if (m_bHeightInc)
			{
				m_dHeight += fElapsedTime * dScaleFactor;
				if (m_dHeight >= 1)
				{
					m_dHeight = 1;
					m_bHeightInc = false;

					if (!m_bAnim && !m_bUpsideDown)
						m_bAnimRunning = false;
				}
			}
			else
			{
				m_dHeight -= fElapsedTime * dScaleFactor;
				if (m_dHeight <= 0)
				{
					m_dHeight = 0;
					m_bHeightInc = true;
					m_bUpsideDown = !m_bUpsideDown;
				}
			}
		}


		GLfloat fTexBottom;
		GLfloat fTexTop;

		if (!m_bUpsideDown)
		{
			fTexBottom = 0.0f;
			fTexTop = 1.0f;
		}
		else
		{
			fTexBottom = 1.0f;
			fTexTop = 0.0f;
		}

		glBindTexture(GL_TEXTURE_2D, m_iTex);
		glBegin(GL_QUADS);
		{
			// static texture
			/*glTexCoord2f(0.0f, 1.0);		glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0);		glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0);		glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0);		glVertex3f(1.0f, -1.0f, 0.0f);*/

			// animated texture
			glTexCoord2f(0.0f, fTexTop);	glVertex3f(-1.0f, -(GLfloat)m_dHeight, 0.0f);
			glTexCoord2f(0.0f, fTexBottom);	glVertex3f(-1.0f, (GLfloat)m_dHeight, 0.0f);
			glTexCoord2f(1.0f, fTexBottom);	glVertex3f(1.0f, (GLfloat)m_dHeight, 0.0f);
			glTexCoord2f(1.0f, fTexTop);	glVertex3f(1.0f, -(GLfloat)m_dHeight, 0.0f);
		}
		glEnd();



		const rl::OpenGLCoord crd1 = getPixelCoord(getWidth() - 5, 0);
		const rl::OpenGLCoord crd2 = getPixelCoord(getWidth(), 5);

		for (uint8_t i = 0; i < 255; i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_iPx);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0);	glVertex3f(crd1.x, crd2.y, 0.0f);
				glTexCoord2f(0.0f, 0.0);	glVertex3f(crd1.x, crd1.y, 0.0f);
				glTexCoord2f(1.0f, 0.0);	glVertex3f(crd2.x, crd1.y, 0.0f);
				glTexCoord2f(1.0f, 1.0);	glVertex3f(crd2.x, crd2.y, 0.0f);
			}
			glEnd();
		}

		//setTitle(std::to_wstring(fElapsedTime).c_str());
		//OutputDebugStringA((std::to_string(fElapsedTime) + '\n').c_str());

		return true;
	}

};


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR szCmdLine,
	_In_ int iCmdShow)
{

	TestWin win(hInstance);
	rl::OpenGLWin_Config config;
	config.bVSync = true;
	config.bInitialFullscreen = false;
	config.bResizable = true;
	config.iWidth = 256 * 4;
	config.iHeight = 240 * 4;
	config.szWinClassName = L"rlOpenGLWin_TestWin";
	config.szInitialCaption = L"OpenGL Test";
	config.hIconBig = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ROBINLE));

	win.run(config);

	return 0;
}
