#ifndef VECTOR_H
#define VECTOR_H

class Point;

class Vector
{
public:
	Vector() : x(0), y(0), z(0), w(0)										{};
	Vector( double x, double y, double z )	:	x(x), y(y), z(z), w(0)		{};

	void Clear()											{ x = y = z = w = 0; };

	Vector operator+(const Vector a) const;
	Vector& operator+=(const Vector& a);
	
	Vector operator-(const Vector& a) const;
	Vector& operator-=(const Vector& a);

	double Length() const;
	double LengthSquared() const;
	double Normalize();

	Point operator+(const Point& a) const;
	Point operator-(const Point& a) const;
	double operator*(const Vector a) const;		/* Dot Product */
	Vector operator*(const double s) const;
	Vector operator/(const double s) const;
	Vector Cross(const Vector a) const;

	Vector& operator*=(const double s);

	Point ToPoint() const;

	void gl() const;
	void glNormal() const;
	void glTexel() const;

	void Print(float scale = 1.0) const;

public:
	double x, y, z, w;
};

class Point
{
public:
	Point() : x(0), y(0), z(0), w(1)										{};
	Point( double x, double y, double z )	:	x(x), y(y), z(z), w(1)		{};

	void Clear()											{ x = y = z = w = 0; };

	Vector operator-(const Point& a) const;
	Point operator+(const Vector a) const;
	Point operator-(const Vector& a) const;
	const Point& operator+=(const Vector a);
	const Point& operator-=(const Vector a);

	Vector ToVector() const;
	void gl() const;
	void glTexel() const;

	void Print(float scale = 1.0) const;

public:
	double x, y, z, w;
};

#endif
