#if !defined(CORE_MATH_H)



union v2{
	struct{
		real32 X, Y;
	};
	real32 E[2];
};

inline v2
V2(real32 X, real32 Y)
{
    v2 Result;

    Result.X = X;
    Result.Y = Y;

    return(Result);
}

inline v2 operator-(v2 A){
	v2 Res;
	Res.X = -A.X;
	Res.Y = -A.Y;
	return(Res);
}

inline v2 operator+(v2 A, v2 B){
	v2 Res;
	Res.X = A.X + B.X;
	Res.Y = A.Y + B.Y;
	return(Res);
}

inline v2 operator-(v2 A, v2 B){
	v2 Res;
	Res.X = A.X - B.X;
	Res.Y = A.Y - B.Y;
	return(Res);
}

inline v2 operator*(real32 M, v2 A){
	v2 Res;
	Res.X = M * A.X;
	Res.Y = M * A.Y;
	return(Res);
}

inline v2 operator*(v2 A, real32 M){
	v2 Res = M * A;
	return(Res);
}

inline v2 &operator*=(v2 &B, real32 M){
	B = M * B;
	return(B);
}


inline v2 &operator+=(v2 &A, v2 B){
	A = A + B;
	return(A);
}

inline real32 Square(real32 A){
	real32 Res = A * A;
	return(Res);
}

inline real32 Inner(v2 A, v2 B){
	real32 Res = A.X*B.X + A.Y*B.Y;
	return(Res);
}

inline real32 LengthSq(v2 A){
	real32 Res = Inner(A, A);
	return(Res);
}

struct rectangle2{
	v2 Min;
	v2 Max;
};

inline v2 GetMinCorner(rectangle2 Rect){
	v2 Res = Rect.Min;
	return(Res);
}

inline v2 GetMaxCorner(rectangle2 Rect){
	v2 Res = Rect.Max;
	return(Res);
}

inline v2 GetCenter(rectangle2 Rect){
	v2 Res = 0.5f * (Rect.Min + Rect.Max);
	return(Res);
}

inline rectangle2 RectMinMax(v2 Min, v2 Max){
	rectangle2 Res;
	Res.Min = Min;
	Res.Max = Max;
	return(Res);
}

inline rectangle2 RectMinDim(v2 min, v2 dim){
	rectangle2 Res;
	Res.Min = min;
	Res.Max = min + dim;
	return(Res);
}


inline rectangle2 RectCenterHalfDim(v2 center, v2 halfdim){
	rectangle2 Res;
	Res.Min = center - halfdim;
	Res.Max = center + halfdim;
	return(Res);
}

inline rectangle2 RectCenterDim(v2 center, v2 dim){
	rectangle2 Res = RectCenterHalfDim(center, 0.5f*dim);
	return(Res);
}

inline bool32 IsInRectangle(rectangle2 Rec, v2 Test){
	bool32 Res = ((Test.X >= Rec.Min.X) && 
				  (Test.Y >= Rec.Min.Y) && 
				  (Test.X < Rec.Max.X) && 
				  (Test.Y < Rec.Max.Y));
	return(Res);
}

#define CORE_MATH_H
#endif
