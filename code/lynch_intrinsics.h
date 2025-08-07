#if !defined(LYNCH_INTRINSICS_H)

#include "math.h"

inline int32 RoundReal32ToInt32(real32 Real32){
	int32 Result = (int32)roundf(Real32);
	return(Result);
}

inline uint32 RoundReal32ToUInt32(real32 Real32){
	uint32 Result = (uint32)roundf(Real32);
	return(Result);
}

inline int32 TruncateReal32ToInt32(real32 Real32){
	int32 Result = (int32)Real32;
	return(Result);
}

inline int32 FloorReal32ToInt32(real32 Real32){
	int32 Result = (int32)floorf(Real32);
	return(Result);
}

inline real32 Sin(real32 Angle){
	real32 Result = sinf(Angle);
	return(Result);
}

inline real32 Cos(real32 Angle){
	real32 Result = cosf(Angle);
	return(Result);
}

inline real32 ArcTan2(real32 Y, real32 X){
	real32 Result = atan2f(Y, X);
	return(Result);
}

struct bit_scan_result{
	bool32 Found;
	uint32 Index;
};

internal bit_scan_result FindLeastSigSetBit(uint32 Value){
	bit_scan_result Res = {};
#if COMPILER_MSVC
	Res.Found = _BitScanForward((unsigned long *)&Res.Index, Value);
#else
	for(uint32 x = 0; x < 32; ++x){
		if (Value & (1 << x)){
			Res.Index = x;
			Res.Found = true;
			break;
		} 
	}
#endif
	return(Res);
}

#define LYNCH_INTRINSICS_H
#endif