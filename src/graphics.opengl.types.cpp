#include "rl/graphics.opengl.types.hpp"

#include <exception>





namespace rl
{
	
	namespace OpenGL
	{

		OpenGLCoord GetPixelCoord(int ViewportWidth, int ViewportHeight, int x, int y)
		{
			OpenGLCoord result = {};
			const OpenGLCoord oScreenCenter = { ViewportWidth / 2.0f, ViewportHeight / 2.0f };

			if (x >= oScreenCenter.x)
				result.x = (x - oScreenCenter.x) / oScreenCenter.x;
			else
				result.x = -1.0f + x / oScreenCenter.x;

			if (y >= oScreenCenter.y)
				result.y = -1.0f * (y - oScreenCenter.y) / oScreenCenter.y;
			else
				result.y = 1.0f - y / oScreenCenter.y;

			return result;
		}

		OpenGLRect GetPixelRect_Pos(int ViewportWidth, int ViewportHeight,
			int left, int top, int right, int bottom)
		{
			if (right <= left || bottom <= top)
				throw std::exception("rl::OpenGL::GetPixelCoord: Invalid coordinates");

			OpenGLRect result = {};

			// 1. Get top left coordinate
			OpenGLCoord tmp = GetPixelCoord(ViewportWidth, ViewportHeight, left, top);
			result.left = tmp.x;
			result.top = tmp.y;

			// 2. Get bottom right coordinate
			tmp = GetPixelCoord(ViewportWidth, ViewportHeight, right, bottom);
			result.right = tmp.x;
			result.bottom = tmp.y;

			return result;
		}

		OpenGLRect GetPixelRect(int ViewportWidth, int ViewportHeight,
			int left, int top, int width, int height)
		{
			return GetPixelRect_Pos(ViewportWidth, ViewportHeight,
				left, top, left + width, top + height);
		}

	}
	
}