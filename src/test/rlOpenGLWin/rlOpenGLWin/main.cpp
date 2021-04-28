#include "../../../rlOpenGLWin.hpp"

#include <Windows.h>
#include <gl/GL.h>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")


// TODO: Header + CPP für einfachere Verwendung von WinAPI-GDI+ (zum Laden von Grafiken/Texturen)

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
	
	TestWin() {}
	virtual ~TestWin() {}


private:

	GLuint m_iTex = 0;
	double dHeight = 1;
	bool bHeightInc = false;

	bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONUP:
			setIcon(LoadIconW(NULL, IDI_ERROR));
			break;

		case WM_KEYUP:
			if (wParam == 'F')
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
		OpenGLImage img(LR"(E:\Bilder\Anderes\;n;.png)");

		uint32_t iColor[] = { 0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF000000 };

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &m_iTex);
		glBindTexture(GL_TEXTURE_2D, m_iTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getWidth(), img.getHeight(), 0, GL_RGBA,
			GL_UNSIGNED_BYTE, img.getData());

		//setWindowedSize(img.getWidth(), img.getHeight());

		return true;
	}

	bool OnDestroy() override
	{
		glDeleteTextures(1, &m_iTex);

		return true;
	}

	bool OnUpdate(float fElapsedTime) override
	{
		if (bHeightInc)
		{
			dHeight += fElapsedTime * 0.5;
			if (dHeight >= 1)
			{
				dHeight = 1;
				bHeightInc = false;
			}
		}
		else
		{
			dHeight -= fElapsedTime * 0.5;
			if (dHeight <= 0)
			{
				dHeight = 0;
				bHeightInc = true;
			}
		}


		glBindTexture(GL_TEXTURE_2D, m_iTex);
		glBegin(GL_QUADS);
		{
			/*glTexCoord2f(0.0f, 1.0);		glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0);		glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0);		glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0);		glVertex3f(1.0f, -1.0f, 0.0f);*/

			glTexCoord2f(0.0f, dHeight);	glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0);		glVertex3f(-1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0);		glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(1.0f, dHeight);	glVertex3f(1.0f, -1.0f, 0.0f);
		}
		glEnd();

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

	TestWin win;
	rl::OpenGLWin_Config config;
	config.bVSync = true;
	config.bInitialFullscreen = false;
	config.bResizable = true;
	config.iWidth = 256 * 4;
	config.iHeight = 240 * 4;
	config.szWinClassName = L"rlOpenGLWin_TestWin";
	config.szInitialCaption = L"OpenGL Test";

	win.run(config);

	return 0;
}
