#include "LSVector.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <GL/glut.h>

/****************************************************************
 * LSVector Methods
 ****************************************************************/

LSVector LSVector::operator+(const LSVector a) const
{
	LSVector ret(*this);

	ret += a;

	return ret;
}
LSVector& LSVector::operator+=(const LSVector& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}
	
LSVector LSVector::operator-(const LSVector& a) const
{
	LSVector ret(*this);

	ret -= a;

	return ret;
}
LSVector& LSVector::operator-=(const LSVector& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

LSPoint LSVector::operator+(const LSPoint& a) const
{
	return ( a + *this );
}

LSPoint LSVector::operator-(const LSPoint& a) const
{
	return ( a - *this );
}

double LSVector::operator*(const LSVector a) const		/* Dot Product */
{
	return ( x * a.x + y * a.y + z * a.z );
}

double LSVector::Length() const
{
	return sqrt( x * x + y * y + z * z );
}

double LSVector::LengthSquared() const
{
	return ( x * x + y * y + z * z );
}

double LSVector::Normalize()
{
	double denom;

	denom = Length();

	if ( denom != 0 ) {
		double d = 1.0 / denom;
		*this = *this * d;
	}

	return denom;
}

LSVector LSVector::Cross(const LSVector a) const
{
	LSVector ret;

	ret.x = y*a.z - a.y*z;
	ret.y = z*a.x - a.z*x;
	ret.z = x*a.y - a.x*y;

	return ret;
}

LSVector LSVector::operator/(const double s) const
{
	LSVector ret(*this);

	double t = 1.0 / s;
	ret.x *= t;
	ret.y *= t;
	ret.z *= t;

	return ret;
}

LSVector LSVector::operator*(const double s) const
{
	LSVector ret(*this);

	ret.x *= s;
	ret.y *= s;
	ret.z *= s;

	return ret;
}

LSVector& LSVector::operator*=(const double s)
{
	*this = *this * s;

	return *this;
}

LSPoint LSVector::ToPoint() const
{
	return LSPoint(x, y, z);
}

void LSVector::gl() const
{
	glVertex3d(x, y, z);
}

void LSVector::glNormal() const
{
	glNormal3d(x, y, z);
}

void LSVector::glTexel() const
{
	glTexCoord2f(x, y);
}

void LSVector::Print(float scale) const
{
	fprintf(stdout, "LSVector: (%1.3f, %1.3f, %1.3f)\n", x/scale, y/scale, z/scale);
	assert( w == 0.0 );
}

/****************************************************************
 * LSPoint Methods
 ****************************************************************/

LSVector LSPoint::operator-(const LSPoint& a) const
{
	LSVector ret;

	ret.x = x - a.x;
	ret.y = y - a.y;
	ret.z = z - a.z;

	return ret;
}

LSPoint LSPoint::operator+(const LSVector a) const
{
	LSPoint ret;

	ret.x = x + a.x;
	ret.y = y + a.y;
	ret.z = z + a.z;

	return ret;
}

LSPoint LSPoint::operator-(const LSVector& a) const
{
	LSPoint ret;

	ret.x = x - a.x;
	ret.y = y - a.y;
	ret.z = z - a.z;

	return ret;
}

const LSPoint& LSPoint::operator+=(const LSVector a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

const LSPoint& LSPoint::operator-=(const LSVector a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

LSVector LSPoint::ToVector() const
{
	return LSVector(x, y, z);
}

void LSPoint::gl() const
{
	glVertex3d(x, y, z);
}

void LSPoint::glTexel() const
{
	glTexCoord2f(x, y);
}

void LSPoint::Print(float scale) const
{
	fprintf(stdout, "LSPoint: (%1.3f, %1.3f, %1.3f)\n", x/scale, y/scale, z/scale);
	assert( w == 1.0 );
}
