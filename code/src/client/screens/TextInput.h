#pragma once
#include "guiitem.h"
class TextInput :
	public GUIItem
{
public:
	TextInput(Screen* screen, std::function<void()> callback, ci::Vec2f position = ci::Vec2f::zero(), const ci::gl::Texture * texture = nullptr, const ci::gl::Texture * hoverTexture = nullptr, const ci::gl::Texture * clickTexture = nullptr);
	~TextInput(void);

	virtual string const & getValue() { return text; }
	void setValue(string value) { text = value; }

	virtual void onKeyInput(KeyEvent& e);
	virtual void draw();

	void setRenderBox(bool doRender) { renderBox = doRender; }

protected:
	string text;
	bool renderBox;
};

