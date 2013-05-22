#include "Vector.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <GL/glut.h>

/****************************************************************
 * Vector Methods
 ****************************************************************/

Vector Vector::operator+(const Vector a) const
{
	Vector ret(*this);

	ret += a;

	return ret;
}
Vector& Vector::operator+=(const Vector& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}
	
Vector Vector::operator-(const Vector& a) const
{
	Vector ret(*this);

	ret -= a;

	return ret;
}
Vector& Vector::operator-=(const Vector& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

Point Vector::operator+(const Point& a) const
{
	return ( a + *this );
}

Point Vector::operator-(const Point& a) const
{
	return ( a - *this );
}

double Vector::operator*(const Vector a) const		/* Dot Product */
{
	return ( x * a.x + y * a.y + z * a.z );
}

double Vector::Length() const
{
	/*return sqrt( LengthSquared() );*/
	return sqrt( x * x + y * y + z * z );
}

double Vector::LengthSquared() const
{
	return ( x * x + y * y + z * z );
}

double Vector::Normalize()
{
	double denom;

	denom = Length();

	if ( denom != 0 ) {
		double d = 1.0 / denom;
		*this = *this * d;
	}

	return denom;
}

Vector Vector::Cross(const Vector a) const
{
	Vector ret;

	ret.x = y*a.z - a.y*z;
	ret.y = z*a.x - a.z*x;
	ret.z = x*a.y - a.x*y;

	return ret;
}

Vector Vector::operator/(const double s) const
{
	Vector ret(*this);

	double t = 1.0 / s;
	ret.x *= t;
	ret.y *= t;
	ret.z *= t;

	return ret;
}

Vector Vector::operator*(const double s) const
{
	Vector ret(*this);

	ret.x *= s;
	ret.y *= s;
	ret.z *= s;

	return ret;
}

Vector& Vector::operator*=(const double s)
{
	*this = *this * s;

	return *this;
}

Point Vector::ToPoint() const
{
	return Point(x, y, z);
}

void Vector::gl() const
{
	glVertex3d(x, y, z);
}

void Vector::glNormal() const
{
	glNormal3d(x, y, z);
}

void Vector::glTexel() const
{
	glTexCoord2f(x, y);
}

void Vector::Print(float scale) const
{
	fprintf(stdout, "Vector: (%1.3f, %1.3f, %1.3f)\n", x/scale, y/scale, z/scale);
	assert( w == 0.0 );
}

/****************************************************************
 * Point Methods
 ****************************************************************/

Vector Point::operator-(const Point& a) const
{
	Vector ret;

	ret.x = x - a.x;
	ret.y = y - a.y;
	ret.z = z - a.z;

	return ret;
}

Point Point::operator+(const Vector a) const
{
	Point ret;

	ret.x = x + a.x;
	ret.y = y + a.y;
	ret.z = z + a.z;

	return ret;
}

Point Point::operator-(const Vector& a) const
{
	Point ret;

	ret.x = x - a.x;
	ret.y = y - a.y;
	ret.z = z - a.z;

	return ret;
}

const Point& Point::operator+=(const Vector a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

const Point& Point::operator-=(const Vector a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

Vector Point::ToVector() const
{
	return Vector(x, y, z);
}

void Point::gl() const
{
	glVertex3d(x, y, z);
}

void Point::glTexel() const
{
	glTexCoord2f(x, y);
}

void Point::Print(float scale) const
{
	fprintf(stdout, "Point: (%1.3f, %1.3f, %1.3f)\n", x/scale, y/scale, z/scale);
	assert( w == 1.0 );
}
