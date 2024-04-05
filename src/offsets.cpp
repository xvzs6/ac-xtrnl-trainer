#include <vector>
#include "offsets.h"


namespace PlayerClass{
	int healthOffsets = 0xEC;
	int yCordOffset = 0x2C;
	int xCordOffset = 0x28;
	int zLegCordOffset = 0x30; 
	int zHeadCordOffset = 0xC;
	int yawViewOffset = 0x34;
	int pitchViewOffset = 0x38;
	int teamOffset = 0x30C;
	int aliveOffset = 0x318;
	int nameOffset = 0x205;
}

namespace basePtrOffset {
	int localPlayerOffset = 0x17E0A8;
	int EntityListOffset = 0x0018AC04;
	int viewMatrixOffset = 0x17DFD0;

}