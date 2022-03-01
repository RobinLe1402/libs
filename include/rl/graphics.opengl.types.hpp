/***************************************************************************************************
 FILE:	graphics.opengl.types.hpp
 CPP:	graphics.opengl.types.cpp (if ROBINLE_OPENGL_FUNCTIONS is defined)
 DESCR:	Some general types for use with OpenGL
***************************************************************************************************/


#ifndef ROBINLE_GRAPHICS_OPENGL_TYPES
#define ROBINLE_GRAPHICS_OPENGL_TYPES





namespace rl
{

	/// <summary>
	/// OpenGL coordinates for placing textures
	/// </summary>
	struct OpenGLCoord
	{
		float x;
		float y;
	};

	/// <summary>
	/// OpenGL rectangle for placing textures
	/// </summary>
	struct OpenGLRect
	{
		float left;
		float top;
		float right;
		float bottom;
	};



	namespace OpenGL
	{
		/// <summary>
		/// The top left coordinate of an OpenGL viewport
		/// </summary>
		const OpenGLCoord TopLeft = { -1, 1 };
		/// <summary>
		/// The bottom right coordinate of an OpenGL viewport
		/// </summary>
		const OpenGLCoord BottomRight = { 1, -1 };

		/// <summary>
		/// The full OpenGL viewport as a rectangle
		/// </summary>
		const OpenGLRect FullScreen = { -1, 1, 1, -1 };

	}

}





#endif // ROBINLE_GRAPHICS_OPENGL_TYPES





#ifdef ROBINLE_OPENGL_FUNCTIONS
	#ifndef _ROBINLE_GRAPHICS_OPENGL_TYPES_FN
	#define _ROBINLE_GRAPHICS_OPENGL_TYPES_FN
namespace rl
{
	namespace OpenGL
	{

		/// <summary>
		/// Convert pixel coordinates to OpenGL coordinates
		/// </summary>
		OpenGLCoord GetPixelCoord(int ViewportWidth, int ViewportHeight, int x, int y);

		/// <summary>
		/// Converts pixel rectangle to OpenGL rectangle
		/// </summary>
		OpenGLRect GetPixelRect_Pos(int ViewportWidth, int ViewportHeight,
			int left, int top, int right, int bottom);

		/// <summary>
		/// Converts pixel rectangle to OpenGL rectangle
		/// </summary>
		OpenGLRect GetPixelRect(int ViewportWidth, int ViewportHeight,
			int left, int top, int width, int height);
	}
}
	#endif // _ROBINLE_GRAPHICS_OPENGL_TYPES_FN
#endif // ROBINLE_OPENGL_FUNCTIONS