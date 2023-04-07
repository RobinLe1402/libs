#define NOMINMAX

#include "rl/graphics.opengl.texture.hpp"
#include "rl/graphics.opengl.types.hpp"
#include "rl/tools.gdiplus.hpp"

#include <exception>
#include <stdint.h>

// OpenGL
#include <Windows.h>
#include <gl/GL.h>

// GDI+
//#include <gdiplus.h> // --> graphics.opengl.texture.hpp

// SHCreateMemStream
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")





namespace rl
{


	Pixel AddPixels(Pixel bottom, Pixel top, PixelAddMode mode)
	{
		Pixel oResult = bottom;

		switch (mode)
		{
		case PixelAddMode::Copy:
			oResult = top;
			break;

		case PixelAddMode::OpaquePart:
			if (top.alpha > 0)
				oResult = top;
			break;

		case PixelAddMode::OpaqueFull:
			if (top.alpha == 0xFF)
				oResult = top;
			break;

		case PixelAddMode::Transparent:
		{
			const float fTopVisible = top.alpha / 255.0f;
			const float fBottomVisible = 1.0f - fTopVisible;

			oResult.alpha = (uint8_t)std::min<uint16_t>(0xFF, (uint16_t)bottom.alpha + top.alpha);
			oResult.r = (uint8_t)(round((double)fBottomVisible * bottom.r) +
				round((double)fTopVisible * top.r));
			oResult.g = (uint8_t)(round((double)fBottomVisible * bottom.g) +
				round((double)fTopVisible * top.g));
			oResult.b = (uint8_t)(round((double)fBottomVisible * bottom.b) +
				round((double)fTopVisible * top.b));


			break;
		}


		default: // should never be called
			oResult = BlankPixel;
		}

		return oResult;
	}


	/***********************************************************************************************
	struct Pixel
	***********************************************************************************************/


	//==============================================================================================
	// METHODS


	//----------------------------------------------------------------------------------------------
	// PUBLIC METHODS

	void Pixel::setRGB(uint32_t RGB)
	{
		r = uint8_t(RGB >> 16);
		g = uint8_t(RGB >> 8);
		b = uint8_t(RGB);
		alpha = 0xFF;
	}

	void Pixel::setARGB(uint32_t ARGB)
	{
		r = uint8_t(ARGB >> 16);
		g = uint8_t(ARGB >> 8);
		b = uint8_t(ARGB);
		alpha = uint8_t(ARGB >> 24);
	}

	void Pixel::setRGBA(uint32_t RGBA)
	{
		r = uint8_t(RGBA >> 24);
		g = uint8_t(RGBA >> 16);
		b = uint8_t(RGBA >> 8);
		alpha = uint8_t(RGBA);
	}

	uint32_t Pixel::getARGB() const
	{
		uint32_t argb = b;
		argb |= uint16_t(g) << 8;
		argb |= uint32_t(r) << 16;
		argb |= uint32_t(alpha) << 24;

		return argb;
	}

	uint32_t Pixel::getRGBA() const
	{
		uint32_t rgba = alpha;
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

	OpenGLTexture::OpenGLTexture(const OpenGLTexture& other) { *this = other; }

	OpenGLTexture::OpenGLTexture(OpenGLTexture&& rval) noexcept { *this = std::move(rval); }

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

		Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL),
			MAKEINTRESOURCE(ResourceID));
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

	void OpenGLTexture::drawToScreen(OpenGLRect rect)  const
	{
		if (m_pData == nullptr || m_iID == 0)
			return;

		glBindTexture(GL_TEXTURE_2D, m_iID);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 1.0);	glVertex3f(rect.left, rect.bottom, 0.0f);
			glTexCoord2f(0.0, 0.0);	glVertex3f(rect.left, rect.top, 0.0f);
			glTexCoord2f(1.0, 0.0);	glVertex3f(rect.right, rect.top, 0.0f);
			glTexCoord2f(1.0, 1.0);	glVertex3f(rect.right, rect.bottom, 0.0f);
		}
		glEnd();
	}

	void OpenGLTexture::drawToScreen(OpenGLRect rect, GLfloat opacity)  const
	{
		if (m_pData == nullptr || m_iID == 0)
			return;

		glBindTexture(GL_TEXTURE_2D, m_iID);



		// apply opacity

		if (opacity <= 0.0f)
			return; // don't draw invisible texture

		if (opacity > 1.0f)
			opacity = 1.0f;

		glColor4f(1.0f, 1.0f, 1.0f, opacity);



		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 1.0);	glVertex3f(rect.left, rect.bottom, 0.0f);
			glTexCoord2f(0.0, 0.0);	glVertex3f(rect.left, rect.top, 0.0f);
			glTexCoord2f(1.0, 0.0);	glVertex3f(rect.right, rect.top, 0.0f);
			glTexCoord2f(1.0, 1.0);	glVertex3f(rect.right, rect.bottom, 0.0f);
		}
		glEnd();



		// reset opacity
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	}

	void OpenGLTexture::draw(const OpenGLTexture& texture, GLsizei x, GLsizei y, bool alpha)
	{
		if (m_pData == nullptr)
			return; // no data in destination texture

		if (!texture.isValid())
			return; // no data in source texture


		const GLsizei iSrcWidth = texture.getWidth();
		const GLsizei iSrcHeight = texture.getHeight();

		if ((x < 0 && -x <= iSrcWidth) || (y < 0 && -y <= iSrcHeight) ||
			x >= m_iWidth || y >= m_iHeight)
			return; // outside of visible area


		const GLsizei iStartX = std::max(0, x);
		const GLsizei iStartY = std::max(0, y);
		const GLsizei iStopX = std::min(m_iWidth - 1, x + iSrcWidth - 1);
		const GLsizei iStopY = std::min(m_iHeight - 1, y + iSrcHeight - 1);

		const GLsizei iOffsetX = std::max(0, -x);
		const GLsizei iOffsetY = std::max(0, -y);

		for (GLsizei iX = iStartX; iX <= iStopX; iX++)
		{
			for (GLsizei iY = iStartY; iY <= iStopY; iY++)
			{
				Pixel px = texture.getPixel(iX - iStartX, iY - iStartY);
				if (!alpha)
					setPixel(iX, iY, px);
				else
					drawPixel(iX, iY, px);
			}
		}
	}

	void OpenGLTexture::clear(Pixel val)
	{
		if (m_pData == nullptr)
			throw std::exception("OpenGLTexture: Invalid data writing request");

		for (size_t i = 0; i < (size_t)m_iHeight * m_iWidth; ++i)
			m_pData[i] = val;
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

	void OpenGLTexture::drawPixel(GLsizei x, GLsizei y, Pixel val)
	{
		if (m_pData == nullptr || x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight)
			throw std::exception("OpenGLTexture: Invalid data writing request");

		const size_t index = (size_t)y * m_iWidth + x;
		Pixel pxDest = m_pData[index];

		m_pData[index] = AddPixels(pxDest, val, PixelAddMode::Transparent);
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

			m_bUploaded = true;
		}
		else // already uploaded --> replace texture
		{
			glBindTexture(GL_TEXTURE_2D, m_iID);

			if (transparency != m_bUploadTransparency)
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
					GLfloat(transparency ? GL_MODULATE : GL_DECAL));

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGBA, GL_UNSIGNED_BYTE,
				m_pData);
		}

		m_bUploadTransparency = transparency;
	}

	void OpenGLTexture::removeUpload()
	{
		if (wglGetCurrentContext() == NULL)
			return;

		glDeleteTextures(1, &m_iID);
		m_iID = 0;
		m_bUploaded = false;
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
		destroy();

		if (!other.isValid())
			return *this;

		create(other.m_iWidth, other.m_iHeight, other.m_iMagFilter, other.m_iMinFilter,
			other.m_iWrapX, other.m_iWrapY);
		memcpy(m_pData, other.m_pData, sizeof(Pixel) * m_iWidth * m_iHeight);

		return *this;
	}

	OpenGLTexture& OpenGLTexture::operator=(OpenGLTexture&& rval) noexcept
	{
		destroy();


		// transfer data
		m_bUploaded = rval.m_bUploaded;
		m_bUploadTransparency = rval.m_bUploadTransparency;
		m_iHeight = rval.m_iHeight;
		m_iID = rval.m_iID;
		m_iMagFilter = rval.m_iMagFilter;
		m_iMinFilter = rval.m_iMinFilter;
		m_iWidth = rval.m_iWidth;
		m_iWrapX = rval.m_iWrapX;
		m_iWrapY = rval.m_iWrapY;
		m_pData = rval.m_pData;
		
		// clear source
		rval.m_bUploaded = false;
		rval.m_bUploadTransparency = false;
		rval.m_iHeight = 0;
		rval.m_iID = 0;
		rval.m_iMagFilter = 0;
		rval.m_iMinFilter = 0;
		rval.m_iWidth = 0;
		rval.m_iWrapX = 0;
		rval.m_iWrapY = 0;
		rval.m_pData = nullptr;


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