#pragma once

#include <vector>
#include <cinder/Vector.h>
#include <cinder/gl/Texture.h>

class GUIItem
{
public:
	GUIItem(std::function<void()> callback, ci::Vec2f position = ci::Vec2f::zero(), const ci::gl::Texture * texture = nullptr, const ci::gl::Texture * hoverTexture = nullptr, const ci::gl::Texture * clickTexture = nullptr);

	GUIItem * addSubItem(std::function<void()> callback, ci::Vec2f position, const ci::gl::Texture * texture = nullptr, const ci::gl::Texture * hoverTexture = nullptr, const ci::gl::Texture * clickTexture = nullptr);
	GUIItem * parent();

	bool isMouseOverItem(ci::Vec2f position);
	bool isMouseDownOnItem(ci::Vec2f position);
	void isMouseUp();

	void setPosition(ci::Vec2f newPosition);
	void setVisible(bool visible, bool recursively = false);

	void draw();
protected:
	bool isPositionInItem(ci::Vec2f position);

	std::function<void()> callback;

	ci::Vec2f position;

	const ci::gl::Texture * currentTexture;
	const ci::gl::Texture * const texture;
	const ci::gl::Texture * const hoverTexture;
	const ci::gl::Texture * const clickTexture;

	bool isVisible;

	GUIItem * parentItem;
	std::vector<GUIItem> subItems;
};