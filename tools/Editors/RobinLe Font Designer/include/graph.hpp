#pragma once
#ifndef ROBINLE_FONT_DESIGNER__GRAPH
#define ROBINLE_FONT_DESIGNER__GRAPH





using uint8_t = unsigned char;



struct Rect
{
	int left, top, right, bottom;
};



enum class ButtonState
{
	Normal,
	Disabled,
	Hovering,
	Clicked
};

struct Button
{
	Rect rect{};
	std::string sCaption;
	ButtonState state;
};

struct Graph
{
	uint8_t iPixelScale;
	std::string sStatus;

	Button btPrev, btNext;
};






#endif // ROBINLE_FONT_DESIGNER__GRAPH
