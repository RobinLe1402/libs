#include "rl/lib/rlGUI/Texture.hpp"
#include "rl/lib/rlGUI/OpenGL.hpp"

namespace rl
{
	namespace GUI
	{

		Texture::Texture(PxSize iWidth, PxSize iHeight, TextureScalingMethod eScalingMethod,
			Color clInit) :
			m_iWidth(iWidth), m_iHeight(iHeight), m_iPixelCount(m_iWidth * m_iHeight),
			m_eScalingMethod(eScalingMethod)
		{
			if (iWidth == 0 || (PxPos)iWidth < 0 || iHeight == 0 || (PxPos)iHeight < 0)
				throw std::exception("rl::GUI::Texture: Invalid size");

			m_upData = std::make_unique<Color[]>(m_iPixelCount);
			for (size_t idx = 0; idx < m_iPixelCount; ++idx)
			{
				m_upData.get()[idx] = clInit;
			}
		}

		Texture::Texture(const Texture& other) :
			m_iWidth(other.m_iWidth), m_iHeight(other.m_iHeight),
			m_iPixelCount(other.m_iPixelCount), m_eScalingMethod(other.m_eScalingMethod)
		{
			m_upData = std::make_unique<Color[]>(m_iPixelCount);

			const size_t iDataSize = m_iPixelCount * sizeof(Color);
			memcpy_s(m_upData.get(), iDataSize, other.m_upData.get(), iDataSize);
		}

		Texture::~Texture()
		{
			// todo: delete uploads
		}

		Color Texture::getPixel(PxPos x, PxPos y) const
		{
			if (x < 0 || x >= (PxPos)m_iWidth || y < 0 || y >= (PxPos)m_iHeight)
				throw std::exception("rl::GUI::Texture: Invalid position");

			return m_upData.get()[y * m_iWidth + x];
		}

		void Texture::setPixel(PxPos x, PxPos y, Color cl)
		{
			if (x < 0 || x >= (PxPos)m_iWidth || y < 0 || y >= (PxPos)m_iHeight)
				return;

			m_upData.get()[y * m_iWidth + x] = cl;

			for (auto& it : m_oUploads)
			{
				it.second.bUpToDate = false;
			}
		}

		void Texture::draw(PxPos iX, PxPos iY)
		{
			const auto hGLRC = wglGetCurrentContext();
			if (hGLRC == NULL)
				return;

			upload();
			glBindTexture(GL_TEXTURE_2D, m_oUploads.at(hGLRC).iTextureID);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f); glVertex3i(iX, iY, 0);
				glTexCoord2f(0.0f, 0.0f); glVertex3i(iX, iY + m_iHeight, 0);
				glTexCoord2f(1.0f, 0.0f); glVertex3i(iX + m_iWidth, iY + m_iHeight, 0);
				glTexCoord2f(1.0f, 1.0f); glVertex3i(iX + m_iWidth, iY, 0);
			}
			glEnd();
		}

		void Texture::drawStretched(PxPos iX, PxPos iY, PxSize iWidth, PxSize iHeight)
		{
			const auto hGLRC = wglGetCurrentContext();

			upload();
			glBindTexture(GL_TEXTURE_2D, m_oUploads.at(hGLRC).iTextureID);
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.0f, 1.0f); glVertex3i(iX, iY, 0);
				glTexCoord2f(0.0f, 0.0f); glVertex3i(iX, iY + iHeight, 0);
				glTexCoord2f(1.0f, 0.0f); glVertex3i(iX + iWidth, iY + iHeight, 0);
				glTexCoord2f(1.0f, 1.0f); glVertex3i(iX + iWidth, iY, 0);
			}
			glEnd();
		}

		void Texture::upload()
		{
			const HGLRC hGLRC = wglGetCurrentContext();
			if (hGLRC == NULL)
				return; // no rendering context

			auto it = m_oUploads.find(hGLRC);
			if (it != m_oUploads.end() && it->second.bUpToDate)
				return; // up to date

			// flip texture vertically
			const size_t iRowSize = m_iWidth * sizeof(Color);
			auto upData = std::make_unique<Color[]>(m_iPixelCount);
			for (size_t iRow = 0; iRow < m_iHeight; ++iRow)
			{
				memcpy_s(upData.get() + ((m_iHeight - iRow - 1) * m_iWidth), iRowSize,
					m_upData.get() + (iRow * m_iWidth), iRowSize);
			}

			// not uploaded to this context yet --> upload now
			if (it == m_oUploads.end())
			{
				Upload oUpload{ .bUpToDate = true, .iTextureID = 0 };
				glGenTextures(1, &oUpload.iTextureID);
				glBindTexture(GL_TEXTURE_2D, oUpload.iTextureID);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iWidth, m_iHeight, 0, GL_RGBA,
					GL_UNSIGNED_BYTE, upData.get());

				GLint iScalingMethod;
				switch (m_eScalingMethod)
				{
				case TextureScalingMethod::Linear:
					iScalingMethod = GL_LINEAR;
					break;
				case TextureScalingMethod::NearestNeighbor:
					iScalingMethod = GL_NEAREST;
					break;

				default:
					iScalingMethod = GL_LINEAR;
					break;
				}

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iScalingMethod);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iScalingMethod);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				m_oUploads[hGLRC] = oUpload;
			}
			// already uploaded --> update
			else
			{
				glBindTexture(GL_TEXTURE_2D, it->second.iTextureID);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_iWidth, m_iHeight, GL_RGBA,
					GL_UNSIGNED_BYTE, upData.get());
				m_oUploads.at(hGLRC).bUpToDate = true;
			}
		}

	}
}
