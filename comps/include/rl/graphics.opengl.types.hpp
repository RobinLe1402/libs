/***************************************************************************************************
 FILE:	graphics.opengl.types.hpp
 CPP:	<n/a>
 DESCR:	Some general types for use with OpenGL
***************************************************************************************************/


#pragma once
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