#include "graphics.opengl.texture.hpp"
#include "tools.gdiplus.hpp"

#include <exception>
#include <stdint.h>

// OpenGL
#include <Windows.h>
#include <gl/GL.h>

// GDI+
#include <gdiplus.h>

// SHCreateMemStream
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")





namespace rl
{

	/***********************************************************************************************
	struct Pixel
	***********************************************************************************************/


	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	Pixel::Pixel()
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0xFF;
	}





	//----------------------------------------------------------------------------------------------
	// STATIC METHODS

	Pixel Pixel::ByRGB(uint32_t RGB)
	{
		Pixel pxResult;
		pxResult.setRGB(RGB);
		return pxResult;
	}

	Pixel Pixel::ByARGB(uint32_t ARGB)
	{
		Pixel pxResult;
		pxResult.setARGB(ARGB);
		return pxResult;
	}

	Pixel Pixel::ByRGBA(uint32_t RGBA)
	{
		Pixel pxResult;
		pxResult.setRGBA(RGBA);
		return pxResult;
	}





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Pixel::setRGB(uint32_t RGB)
	{
		r = uint8_t(RGB >> 16);
		g = uint8_t(RGB >> 8);
		b = uint8_t(RGB);
		a = 0xFF;
	}

	void Pixel::setARGB(uint32_t ARGB)
	{
		r = uint8_t(ARGB >> 16);
		g = uint8_t(ARGB >> 8);
		b = uint8_t(ARGB);
		a = uint8_t(ARGB >> 24);
	}

	void Pixel::setRGBA(uint32_t RGBA)
	{
		r = uint8_t(RGBA >> 24);
		g = uint8_t(RGBA >> 16);
		b = uint8_t(RGBA >> 8);
		a = uint8_t(RGBA);
	}

	uint32_t Pixel::getARGB() const
	{
		uint32_t argb = b;
		argb |= uint16_t(g) << 8;
		argb |= uint32_t(r) << 16;
		argb |= uint32_t(a) << 24;

		return argb;
	}

	uint32_t Pixel::getRGBA() const
	{
		uint32_t rgba = a;
		rgba |= uint16_t(b) << 8;
		rgba |= uint32_t(g) << 16;
		rgba |= uint32_t(r) << 24;

		return rgba;
	}










	/***********************************************************************************************
	class OpenGLTexture
	***********************************************************************************************/


	//----------------------------------------------------------------------------------------------
	// CONSTRUCTORS, DESTRUCTORS

	OpenGLTexture::OpenGLTexture(const OpenGLTexture& other) { assign(other); }

	OpenGLTexture::~OpenGLTexture() { destroy(); }





	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	bool OpenGLTexture::create(GLsizei width, GLsizei height, OpenGLTexture_Config config,
		Pixel InitVal)
	{
		if (width <= 0 || height <= 0)
			return false;

		create(width, height, getFilterInt(config.filterMag), getFilterInt(config.filterMin),
			getWrapInt(config.wrapX), getWrapInt(config.wrapY));

		for (size_t i = 0; i < (size_t)m_iWidth * m_iHeight; i++)
#pragma warning(suppress : 6386) // "Buffer overrun" false positive
			m_pData[i] = InitVal;

		return true;
	}

	void OpenGLTexture::createFromGDIPlusBitmap(Gdiplus::Bitmap* bmp,
		OpenGLTexture_Config config)
	{
		create(bmp->GetWidth(), bmp->GetHeight(), config);

		for (unsigned int iX = 0; iX < (unsigned int)m_iWidth; iX++)
		{
			for (unsigned int iY = 0; iY < (unsigned int)m_iHeight; iY++)
			{
				Gdiplus::Color cl;
				bmp->GetPixel((INT)iX, (INT)iY, &cl);
				setPixel(iX, iY, Pixel::ByARGB(cl.GetValue()));
			}
		}
	}

	bool OpenGLTexture::createFromBitmapResource(int ResourceID, OpenGLTexture_Config config)
	{
		destroy();

		GDIPlus gp;

		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromResource(NULL, MAKEINTRESOURCE(ResourceID));
		bool bResult = (bmp->GetLastStatus() == Gdiplus::Ok);

		if (bResult)
			createFromGDIPlusBitmap(bmp, config);


		return bResult;
	}

	bool OpenGLTexture::createFromImageData(const void* data, uint32_t size,
		OpenGLTexture_Config config)
	{
		destroy();

		GDIPlus gp;

		IStream* stream = SHCreateMemStream((BYTE*)data, size);
		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromStream(stream);
		bool bResult = (bmp->GetLastStatus() == Gdiplus::Ok);
		stream->Release();

		if (bResult)
			createFromGDIPlusBitmap(bmp, config);


		return bResult;
	}

	void OpenGLTexture::destroy()
	{
		if (m_pData == nullptr)
			return;

		removeUpload();
		delete[] m_pData;
		m_pData = nullptr;
		m_iWidth = 0;
		m_iHeight = 0;
	}

	bool OpenGLTexture::assign(const OpenGLTexture& other)
	{
		destroy();

		if (!other.isValid())
			return false;

		create(other.m_iWidth, other.m_iHeight, other.m_iMagFilter, other.m_iMinFilter,
			other.m_iWrapX, other.m_iWrapY);
		memcpy(m_pData, other.m_pData, sizeof(Pixel) * m_iWidth * m_iHeight);

		return true;
	}

	void OpenGLTexture::draw(GLfloat left, GLfloat top, GLfloat right, GLfloat bottom)
	{
		if (m_pData == nullptr || m_iID == 0)
			return;

		glBindTexture(GL_TEXTURE_2D, m_iID);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 1.0);	glVertex3f(left, bottom, 0.0f);
			glTexCoord2f(0.0, 0.0);	glVertex3f(left, top, 0.0f);
			glTexCoord2f(1.0, 0.0);	glVertex3f(right, top, 0.0f);
			glTexCoord2f(1.0, 1.0);	glVertex3f(right, bottom, 0.0f);
		}
		glEnd();
	}

	void OpenGLTexture::setPixel(GLsizei x, GLsizei y, Pixel val)
	{
		if (m_pData == nullptr || x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight)
			throw std::exception("OpenGLTexture: Invalid data writing request");

		m_pData[y * m_iWidth + x] = val;
	}

	Pixel OpenGLTexture::getPixel(GLsizei x, GLsizei y) const
	{
		if (m_pData == nullptr || x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight)
			throw std::exception("OpenGLTexture: Invalid data reading request");

		return m_pData[y * m_iWidth + x];
	}

	void OpenGLTexture::upload(bool transparency)
	{
		if (m_pData == nullptr)
			return;


		if (!m_bUploaded) // not uploaded yet --> create texture
		{
			glGenTextures(1, &m_iID);
			glBindTexture(GL_TEXTURE_2D, m_iID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_iMagFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_iMinFilter);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
				GLfloat(transparency ? GL_MODULATE : GL_DECAL));

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, m_pData);
		}
		else // already uploaded --> replace texture
		{
			glBindTexture(GL_TEXTURE_2D, m_iID);

			if (transparency != m_bUploadTransparecy)
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
					GLfloat(transparency ? GL_MODULATE : GL_DECAL));

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGBA, GL_UNSIGNED_BYTE,
				m_pData);
		}

		m_bUploadTransparecy = transparency;
	}

	void OpenGLTexture::removeUpload()
	{
		if (wglGetCurrentContext() == NULL)
			return;

		glDeleteTextures(1, &m_iID);
		m_iID = 0;
	}

	OpenGLTexture_Config OpenGLTexture::getConfig() const
	{
		OpenGLTexture_Config o;
		o.filterMag = getFilterEnum(m_iMagFilter);
		o.filterMin = getFilterEnum(m_iMinFilter);
		o.wrapX = getWrapEnum(m_iWrapX);
		o.wrapY = getWrapEnum(m_iWrapY);

		return o;
	}

	bool OpenGLTexture::copyToBitmap(Gdiplus::Bitmap** bmp) const
	{
		if (m_pData == nullptr)
			return false;

		*bmp = new Gdiplus::Bitmap(m_iWidth, m_iHeight, PixelFormat32bppARGB);

		if ((*bmp)->GetLastStatus() != Gdiplus::Ok)
		{
			delete (*bmp);
			return false;
		}

		Gdiplus::Color cl;
		for (uint64_t iX = 0; iX < m_iWidth; iX++)
		{
			for (uint64_t iY = 0; iY < m_iHeight; iY++)
			{
				cl.SetValue(getPixel((GLsizei)iX, (GLsizei)iY).getARGB());
				(*bmp)->SetPixel((INT)iX, (INT)iY, cl);
			}
		}

		return true;
	}





	//----------------------------------------------------------------------------------------------
	// OPERATORS

	OpenGLTexture& OpenGLTexture::operator=(const OpenGLTexture& other)
	{
		if (!assign(other))
			throw std::exception("Couldn't assign rl::OpenGLTexture to another one");

		return *this;
	}





	//----------------------------------------------------------------------------------------------
	// PRIVATE METHODS

	void OpenGLTexture::create(GLsizei iWidth, GLsizei iHeight, GLint iFilterMag, GLint iFilterMin,
		GLint iWrapX, GLint iWrapY)
	{
		destroy();

		m_iWidth = iWidth;
		m_iHeight = iHeight;

		m_iMagFilter = iFilterMag;
		m_iMinFilter = iFilterMin;
		m_iWrapX = iWrapX;
		m_iWrapY = iWrapY;


		m_pData = new Pixel[(size_t)m_iWidth * m_iHeight];
	}

	GLint OpenGLTexture::getFilterInt(OpenGLTextureFilter filter)
	{
		if (filter == OpenGLTextureFilter::Linear)
			return GL_LINEAR;
		else
			return GL_NEAREST;
	}

	GLint OpenGLTexture::getWrapInt(OpenGLTextureWrap wrap)
	{
		if (wrap == OpenGLTextureWrap::Clamp)
			return GL_CLAMP;
		else
			return GL_REPEAT;
	}

	OpenGLTextureFilter OpenGLTexture::getFilterEnum(GLint filter)
	{
		if (filter == GL_NEAREST)
			return OpenGLTextureFilter::NearestNeighbor;
		else
			return OpenGLTextureFilter::Linear;
	}

	OpenGLTextureWrap OpenGLTexture::getWrapEnum(GLint wrap)
	{
		if (wrap == GL_CLAMP)
			return OpenGLTextureWrap::Clamp;
		else
			return OpenGLTextureWrap::Repeat;
	}

}