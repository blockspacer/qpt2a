#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
class CreateCellErrorCode 
{
private:
	unsigned int value;
public:
	enum Type {
		Invalid,					/// Determines that this is an invalid error code
		SpotAlreadyTaken,			/// This position is already occupied by another cell
		IdAreadyTaken,				/// The given identifier is already used by another cell
		OutsideGameArea,			/// The specified spot is outside the gamearea
		PlayerIsSpectator,			/// The player is already dead and/or a spectator
		Last_Value					/// Is the last valid error code
	};
	
	CreateCellErrorCode(Type value);
	CreateCellErrorCode(unsigned int networkValue);
	Type getType();
	unsigned int getNetworkType();
};