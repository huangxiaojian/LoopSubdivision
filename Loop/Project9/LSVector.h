#ifndef VECTOR_H
#define VECTOR_H

class LSPoint;

class LSVector
{
public:
	LSVector() : x(0), y(0), z(0), w(0)										{};
	LSVector( double x, double y, double z )	:	x(x), y(y), z(z), w(0)		{};

	void Clear()											{ x = y = z = w = 0; };

	LSVector operator+(const LSVector a) const;
	LSVector& operator+=(const LSVector& a);
	
	LSVector operator-(const LSVector& a) const;
	LSVector& operator-=(const LSVector& a);

	double Length() const;
	double LengthSquared() const;
	double Normalize();

	LSPoint operator+(const LSPoint& a) const;
	LSPoint operator-(const LSPoint& a) const;
	double operator*(const LSVector a) const;		/* Dot Product */
	LSVector operator*(const double s) const;
	LSVector operator/(const double s) const;
	LSVector Cross(const LSVector a) const;

	LSVector& operator*=(const double s);

	LSPoint ToPoint() const;

	void gl() const;
	void glNormal() const;
	void glTexel() const;

	void Print(float scale = 1.0) const;

public:
	double x, y, z, w;
};

class LSPoint
{
public:
	LSPoint() : x(0), y(0), z(0), w(1)										{};
	LSPoint( double x, double y, double z )	:	x(x), y(y), z(z), w(1)		{};

	void Clear()											{ x = y = z = w = 0; };

	LSVector operator-(const LSPoint& a) const;
	LSPoint operator+(const LSVector a) const;
	LSPoint operator-(const LSVector& a) const;
	const LSPoint& operator+=(const LSVector a);
	const LSPoint& operator-=(const LSVector a);

	LSVector ToVector() const;
	void gl() const;
	void glTexel() const;

	void Print(float scale = 1.0) const;

public:
	double x, y, z, w;
};

#endif
