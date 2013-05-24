#ifndef SUBDIVISION_H
#define SUBDIVISION_H

#include "LSVector.h"
#include <vector>
#include <string.h>
#include <stdio.h>
#include <GL/glut.h>

class LS_Edge;
class LS_Face;
class LS_Vertex;

using namespace std;

typedef pair<LS_Vertex*, LS_Vertex*> LS_VertexPair;

#define USE_MEM_POOL 1

#if USE_MEM_POOL
	#define MP_VERTS 	45000
	#define MP_EDGES	75000
	#define MP_FACES	50000
#endif

class LS_Vertex
{
public:
	LS_Vertex(){}
	int index;
private:
	vector<LS_Edge*> edges;

	vector<LS_Face*> faces;

	LSPoint pos;
	LSPoint newPos;

	LSPoint texel;
	LSVector normal;

	int creationLevel;

	LS_Edge** GetEdgePointer( LS_Edge *ptr );
	LS_Face** GetFacePointer( LS_Face *ptr );
	LS_Vertex* GetNewVertexInEdge( LS_Vertex *v );

	void UpdateNormal();

	friend class LS_Surface;
	friend class LS_Face;

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif
};
typedef vector<LS_Vertex*> LS_VertexList;


class LS_Face
{
private:
	LS_Vertex *vertices[3];
	LS_Edge *edges[3];

	LSVector centroid;
	LSVector normal;

	void UpdateCentroid()
	{
		centroid = ( vertices[0]->pos.ToVector() + vertices[1]->pos.ToVector() 
					+ vertices[2]->pos.ToVector() ) / 3;
	}

	void UpdateNormal()
	{
		normal = ( vertices[1]->pos - vertices[0]->pos ).Cross(
						vertices[2]->pos - vertices[0]->pos );
		normal.Normalize();
	}

	friend class LS_Surface;
	friend class LS_Vertex;

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif
};
typedef vector<LS_Face*> LS_FaceList;

class LS_Edge
{
public:
	LS_Edge() : oldVertex(0)								{}

private:
	LS_Vertex *vertices[2];
	LS_Vertex *oldVertex;

	LS_Vertex** GetVertexPointer( LS_Vertex *v )
	{
		if ( vertices[0] == v )
			return &vertices[0];
		else
			return &vertices[1];
	}

	friend class LS_Surface;
	friend class LS_Vertex;
	friend bool operator!=( LS_Edge*& e, const LS_VertexPair &vp );
	friend bool operator==( LS_Edge*& e, const LS_VertexPair &vp );

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif
};
typedef vector<LS_Edge*> LS_EdgeList;

bool operator!=( LS_Edge*& e, const LS_VertexPair &vp );
bool operator==( LS_Edge*& e, const LS_VertexPair &vp );


class LS_Surface
{
public:
	LS_Surface() : subdivisionLevel(0)				
	{
#if USE_MEM_POOL
		vertices.reserve( MP_VERTS );
		edges.reserve( MP_EDGES );
		faces.reserve( MP_FACES );
#endif
	}
	~LS_Surface();

	void Reset();
	//void ReadObjFromFile(const char* filename, float scale = 1.0);
	void ReadObj(const char* filename, float scale = 1.0);
	void WriteObj(const char* filename);

	void AddFaceOfIndex(int i1,int i2, int i3);
	void UpdateNormals();

	void Subdivide();

	enum DRAWFLAGS {
		SOLID 		=  1,
		NORMALS 	=  2,
		POINTS 		=  4,
		FLAT		=  8,
		WIREFRAME	= 16
	};
	void Draw( DRAWFLAGS flags = SOLID );

protected:
	LS_Edge*  _AddEdge( LS_Vertex *v1, LS_Vertex *v2 );

	void SubdivideEdge( LS_Edge *e );
	void SubdivideFace( LS_Face *f );
	void RepositionVertex( LS_Vertex *v );

	int skipSpace(char *str)
	{
		for(int i = 0; str[i]; i++)
			if(str[i] == ' ' || str[i] == '\t')continue;
			else return i;
		return -1;
	}

protected:
	LS_VertexList vertices;
	LS_EdgeList edges;
	LS_FaceList faces;

	int subdivisionLevel;
};

#endif // SUBDIVISION_H
