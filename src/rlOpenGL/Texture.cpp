#include "rl/lib/rlOpenGL/Texture.hpp"

namespace lib = rl::OpenGL;

#include <Windows.h>
#include <gl/GL.h>



lib::Texture& lib::Texture::operator=(const Texture& other)
{
	destroy();
	if (!other)
		return *this;

	m_pData = new Pixel[other.m_iWidth * other.m_iHeight];
	if (!m_pData)
		return *this;

	const size_t iDataSize = (size_t)other.m_iWidth * other.m_iHeight * sizeof(Pixel);

	memcpy_s(m_pData, iDataSize, other.m_pData, iDataSize);

	m_iWidth = other.m_iWidth;
	m_iHeight = other.m_iHeight;
	m_bTransparency = other.m_bTransparency;
	m_fOpacity = other.m_fOpacity;
	m_eDownscalingMethod = other.m_eDownscalingMethod;
	m_eUpscalingMethod = other.m_eUpscalingMethod;
	m_eWrapMethodX = other.m_eWrapMethodX;
	m_eWrapMethodY = other.m_eWrapMethodY;

	return *this;
}

lib::Texture& lib::Texture::operator=(Texture&& rval) noexcept
{
	if (&rval == this)
		return *this;

	destroy();
	if (!rval)
		return *this;

	m_pData = rval.m_pData;
	m_iWidth = rval.m_iWidth;
	m_iHeight = rval.m_iHeight;
	m_iID = rval.m_iID;
	m_bTransparency = rval.m_bTransparency;
	m_fOpacity = rval.m_fOpacity;
	m_eDownscalingMethod = rval.m_eDownscalingMethod;
	m_eUpscalingMethod = rval.m_eUpscalingMethod;
	m_eWrapMethodX = rval.m_eWrapMethodX;
	m_eWrapMethodY = rval.m_eWrapMethodY;

	rval.m_pData = nullptr;
	rval.m_iWidth = rval.m_iHeight = 0;
	rval.m_iID = 0;
	rval.m_bTransparency = true;
	rval.m_fOpacity = 1.0f;
	rval.m_eDownscalingMethod = rval.m_eUpscalingMethod = TextureScalingMethod::Linear;
	rval.m_eWrapMethodX = rval.m_eWrapMethodY = TextureWrapMethod::Repeat;

	return *this;
}

lib::Texture::Texture() :
	m_pData(nullptr), m_iWidth(0), m_iHeight(0), m_iID(0), m_bTransparency(true), m_fOpacity(1.0f),
	m_eDownscalingMethod(TextureScalingMethod::Linear),
	m_eUpscalingMethod(TextureScalingMethod::Linear),
	m_eWrapMethodX(TextureWrapMethod::Repeat),
	m_eWrapMethodY(TextureWrapMethod::Repeat)
{}

lib::Texture::Texture(const Texture& other) :
	m_pData(new Pixel[other.m_iWidth * other.m_iHeight]), m_iWidth(other.m_iWidth), m_iHeight(0),
	m_iID(0), m_bTransparency(other.m_bTransparency), m_fOpacity(other.m_fOpacity),
	m_eDownscalingMethod(other.m_eDownscalingMethod),
	m_eUpscalingMethod(other.m_eUpscalingMethod),
	m_eWrapMethodX(other.m_eWrapMethodX),
	m_eWrapMethodY(other.m_eWrapMethodY)
{
	if (!other)
		return;

	const size_t iDataSize = (size_t)m_iWidth * m_iHeight * sizeof(Pixel);
	memcpy_s(m_pData, iDataSize, other.m_pData, iDataSize);
}

lib::Texture::Texture(Texture&& rval) noexcept :
	m_pData(rval.m_pData), m_iWidth(rval.m_iWidth), m_iHeight(rval.m_iHeight), m_iID(rval.m_iID),
	m_bTransparency(rval.m_bTransparency), m_fOpacity(rval.m_fOpacity),
	m_eDownscalingMethod(rval.m_eDownscalingMethod),
	m_eUpscalingMethod(rval.m_eUpscalingMethod),
	m_eWrapMethodX(rval.m_eWrapMethodX),
	m_eWrapMethodY(rval.m_eWrapMethodY)
{
	rval.m_pData = nullptr;
	rval.m_iWidth = 0;
	rval.m_iHeight = 0;
	rval.m_iID = 0;
	rval.m_bTransparency = false;
	rval.m_fOpacity = 1.0f;
	rval.m_eDownscalingMethod = TextureScalingMethod::Linear;
	rval.m_eUpscalingMethod = TextureScalingMethod::Linear;
	rval.m_eWrapMethodX = TextureWrapMethod::Repeat;
	rval.m_eWrapMethodY = TextureWrapMethod::Repeat;
}

lib::Texture::~Texture()
{
	if (!m_pData)
		return;

	if (uploaded())
		undoUpload();
	delete[] m_pData;
}

bool lib::Texture::create(GLuint iWidth, GLuint iHeight)
{
	if (!createUninitialized(iWidth, iHeight))
		return false;

	memset(m_pData, 0, (size_t)iWidth * iHeight * sizeof(Pixel));

	return true;
}

bool lib::Texture::create(GLuint iWidth, GLuint iHeight, const Pixel& pxColor)
{
	if (!createUninitialized(iWidth, iHeight))
		return false;

	const size_t iDataSize = (size_t)m_iWidth * m_iHeight;
	for (size_t i = 0; i < iDataSize; ++i)
		m_pData[i] = pxColor;

	return true;
}

void lib::Texture::destroy()
{
	if (!m_pData)
		return;

	if (uploaded())
		undoUpload();

	delete[] m_pData;
	m_pData = nullptr;

	m_iWidth = 0;
	m_iHeight = 0;

	m_iID = 0;
	m_bTransparency = false;
	m_fOpacity = 1.0f;
}

void lib::Texture::upload()
{
	if (!m_pData)
		return;

	if (m_iID == 0) // new upload
	{
		glGenTextures(1, &m_iID);
		glBindTexture(GL_TEXTURE_2D, m_iID);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			GLfloat(m_bTransparency ? GL_MODULATE : GL_DECAL));
		glColor4f(1.0f, 1.0f, 1.0f, m_fOpacity);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			(m_eUpscalingMethod == TextureScalingMethod::Linear) ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			(m_eDownscalingMethod == TextureScalingMethod::Linear) ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			(m_eWrapMethodX == TextureWrapMethod::Repeat) ? GL_REPEAT : GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			(m_eWrapMethodY == TextureWrapMethod::Repeat) ? GL_REPEAT : GL_CLAMP);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			m_pData);
	}
	else // update existing data
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGBA, GL_UNSIGNED_BYTE,
			m_pData);
	}
}

void lib::Texture::undoUpload()
{
	if (!uploaded())
		return;

	glDeleteTextures(1, &m_iID);
	m_iID = 0;
}

lib::TextureDrawingCoordinates lib::Texture::coordsUnscaled(GLint iX, GLint iY,
	const TexturePixelSize& oViewportSize) const
{
	TextureDrawingCoordinates oResult =
	{
		// screen
		TextureQuad::Rect(
		{
			PixelToViewport_X(iX, oViewportSize.iWidth),
			PixelToViewport_Y(iY, oViewportSize.iHeight)
		},
		{
			PixelToViewport_X(iX + m_iWidth, oViewportSize.iWidth),
			PixelToViewport_Y(iY + m_iHeight, oViewportSize.iHeight)
		}
		),

		// texture
		FullTexture
	};

	return oResult;
}

lib::TextureDrawingCoordinates lib::Texture::coordsUnscaled(const TexturePixelRect& rect,
	GLint iX, GLint iY, const TexturePixelSize& oViewportSize) const
{
	TextureDrawingCoordinates oResult =
	{
		// screen
		TextureQuad::Rect(
		{
			PixelToViewport_X(iX, oViewportSize.iWidth),
			PixelToViewport_Y(iY, oViewportSize.iHeight)
		},
		{
			PixelToViewport_X(iX + (rect.iRight - rect.iLeft), oViewportSize.iWidth),
			PixelToViewport_Y(iY + (rect.iBottom - rect.iTop), oViewportSize.iHeight)
		}
		),

		// texture
		oResult.texture = TextureQuad::Rect(
		{
			PixelToTexture_X(rect.iLeft, m_iWidth),
			PixelToTexture_Y(rect.iTop, m_iHeight)
		},
		{
			PixelToTexture_X(rect.iRight, m_iWidth),
			PixelToTexture_Y(rect.iBottom, m_iHeight)
		}
		)
	};

	return oResult;
}

lib::TextureDrawingCoordinates lib::Texture::coordsScaled(const TexturePixelRect& oDestinationRect,
	const TexturePixelSize& oViewportSize) const
{
	TextureDrawingCoordinates oResult =
	{
		// screen
		TextureQuad::Rect(
		{
			PixelToViewport_X(oDestinationRect.iLeft, oViewportSize.iWidth),
			PixelToViewport_Y(oDestinationRect.iTop, oViewportSize.iHeight)
		},
		{
			PixelToViewport_X(oDestinationRect.iRight, oViewportSize.iWidth),
			PixelToViewport_Y(oDestinationRect.iBottom, oViewportSize.iHeight)
		}
		),

		// texture
		FullTexture
	};

	return oResult;
}

lib::TextureDrawingCoordinates lib::Texture::coordsScaled(const TexturePixelRect& oSourceRect,
	const TexturePixelRect& oDestinationRect, const TexturePixelSize& oViewportSize) const
{
	TextureDrawingCoordinates oResult =
	{
		// screen
		TextureQuad::Rect(
		{
			PixelToViewport_X(oDestinationRect.iLeft, oViewportSize.iWidth),
			PixelToViewport_Y(oDestinationRect.iTop, oViewportSize.iHeight)
		},
		{
			PixelToViewport_X(oDestinationRect.iRight, oViewportSize.iWidth),
			PixelToViewport_Y(oDestinationRect.iBottom, oViewportSize.iHeight)
		}
		),

		// texture
		oResult.texture = TextureQuad::Rect(
		{
			PixelToTexture_X(oSourceRect.iLeft, m_iWidth),
			PixelToTexture_Y(oSourceRect.iTop, m_iHeight)
		},
		{
			PixelToTexture_X(oSourceRect.iRight, m_iWidth),
			PixelToTexture_Y(oSourceRect.iBottom, m_iHeight)
		}
		)
	};

	return oResult;
}

void lib::Texture::draw(const TextureDrawingCoordinates& coords) const
{
	if (!uploaded())
		return;

	glBindTexture(GL_TEXTURE_2D, m_iID);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(coords.texture.bottomLeft.fX, coords.texture.bottomLeft.fY);
		glVertex3f(coords.screen.bottomLeft.fX, coords.screen.bottomLeft.fY, 0.0f);

		glTexCoord2f(coords.texture.bottomRight.fX, coords.texture.bottomRight.fY);
		glVertex3f(coords.screen.bottomRight.fX, coords.screen.bottomRight.fY, 0.0f);

		glTexCoord2f(coords.texture.topRight.fX, coords.texture.topRight.fY);
		glVertex3f(coords.screen.topRight.fX, coords.screen.topRight.fY, 0.0f);

		glTexCoord2f(coords.texture.topLeft.fX, coords.texture.topLeft.fY);
		glVertex3f(coords.screen.topLeft.fX, coords.screen.topLeft.fY, 0.0f);
	}
	glEnd();
}

bool lib::Texture::setPixel(GLuint iX, GLuint iY, const Pixel& px)
{
	if (!m_pData || iX >= m_iWidth || iY >= m_iHeight)
		return false;

	m_pData[(m_iHeight - 1 - iY) * m_iWidth + iX] = px;
	return true;
}

void lib::Texture::setTransparency(bool bTransparency)
{
	if (!m_pData)
		return; // no data

	if ((m_bTransparency && bTransparency) || (!m_bTransparency && !bTransparency))
		return; // no change

	m_bTransparency = bTransparency;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
			GLfloat(m_bTransparency ? GL_MODULATE : GL_DECAL));
	}
}

void lib::Texture::setOpacity(GLfloat fOpacity)
{
	if (!m_pData)
		return; // no data

	if (m_fOpacity == fOpacity)
		return; // no change

	// restrict opacity to between 0.0 and 1.0
	if (fOpacity > 1.0f)
		fOpacity = 1.0f;
	else if (fOpacity < 0.0f)
		fOpacity = 0.0f;

	m_fOpacity = fOpacity;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);
		glColor4f(1.0f, 1.0f, 1.0f, m_fOpacity);
	}
}

void lib::Texture::setUpscalingMethod(TextureScalingMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eUpscalingMethod == eMethod)
		return; // no change

	m_eUpscalingMethod = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			(eMethod == TextureScalingMethod::Linear) ? GL_LINEAR : GL_NEAREST);
	}
}

void lib::Texture::setDownscalingMethod(TextureScalingMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eDownscalingMethod == eMethod)
		return; // no change

	m_eDownscalingMethod = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			(eMethod == TextureScalingMethod::Linear) ? GL_LINEAR : GL_NEAREST);
	}
}

void lib::Texture::setScalingMethod(TextureScalingMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eUpscalingMethod == eMethod && m_eDownscalingMethod == eMethod)
		return; // no change

	m_eDownscalingMethod = m_eUpscalingMethod = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);

		const GLint iFilter =
			(eMethod == TextureScalingMethod::Linear) ? GL_LINEAR : GL_NEAREST;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iFilter);
	}
}

void lib::Texture::setWrapMethodX(TextureWrapMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eWrapMethodX == eMethod)
		return; // no change

	m_eWrapMethodX = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			(eMethod == TextureWrapMethod::Repeat) ? GL_REPEAT : GL_CLAMP);
	}
}

void lib::Texture::setWrapMethodY(TextureWrapMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eWrapMethodY == eMethod)
		return; // no change

	m_eWrapMethodY = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			(eMethod == TextureWrapMethod::Repeat) ? GL_REPEAT : GL_CLAMP);
	}
}

void lib::Texture::setWrapMethod(TextureWrapMethod eMethod)
{
	if (!m_pData)
		return; // no data

	if (m_eWrapMethodX == eMethod && m_eWrapMethodY == eMethod)
		return; // no change

	m_eWrapMethodX = m_eWrapMethodY = eMethod;
	if (uploaded())
	{
		glBindTexture(GL_TEXTURE_2D, m_iID);

		const GLint iWrap =
			(eMethod == TextureWrapMethod::Repeat) ? GL_REPEAT : GL_CLAMP;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, iWrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, iWrap);
	}
}

lib::Pixel lib::Texture::getPixel(GLuint iX, GLuint iY) const
{
	if (!m_pData || iX >= m_iWidth || iY >= m_iHeight)
		return Color::Blank;

	return m_pData[(m_iHeight - 1 - iY) * m_iWidth + iX];
}

bool lib::Texture::createUninitialized(GLuint iWidth, GLuint iHeight)
{
	destroy();

	if (iWidth == 0 || iHeight == 0)
		return false;



	m_pData = new Pixel[iWidth * iHeight];
	if (!m_pData)
		return false;

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	return true;
}
