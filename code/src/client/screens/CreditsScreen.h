#pragma once
#include "Screen.h"
class CreditsScreen :
	public Screen
{
public:
	CreditsScreen(void);
	~CreditsScreen(void);

	virtual void update(float frameTime);
	virtual void draw();

	/**
		@brief event method called when a new touch has begun
		@param touchWay			touch way of the newly begun touch
	 */
	virtual bool touchBegan(const TouchWay & touchWay);
	/**
		@brief event method called when an existing touch has moved
		@param touchWay			touch way of the touch, concerned
	 */
	virtual void touchMoved(const TouchWay & touchWay);
	/**
		@brief event method called when an existing touch has ended
		@param touchWay			touch way of the touch, concerned; must be called by copy,
								because touch way object will be destroyed after call to this method
	 */
	virtual void touchEnded(TouchWay touchWay);
	
	virtual void resize(ResizeEvent event);
};

