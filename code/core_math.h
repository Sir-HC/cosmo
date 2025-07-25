#if !defined(CORE_MATH_H)



union v2{
	struct{
		real32 X, Y;
	};
	real32 E[2];
};

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

#define CORE_MATH_H
#endif
