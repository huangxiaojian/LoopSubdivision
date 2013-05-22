#ifndef SUBDIVISION_H
#define SUBDIVISION_H

#include "Vector.h"
#include <vector>
#include <string.h>
#include <stdio.h>
#include <GL/glut.h>

class SS_Edge;
class SS_Face;
class SS_Vertex;

using namespace std;

typedef pair<SS_Vertex*, SS_Vertex*> SS_VertexPair;

#define INFO

#define USE_MEM_POOL 1

#if USE_MEM_POOL
	#define MP_VERTS 	45000
	#define MP_EDGES	75000
	#define MP_FACES	50000
#endif

// Feel free to replace this class with one that integrates better with your project
class Vertex
{
public:
	Vertex(){index = -1;}
	Point pos;
	Vector texel;
	int index;
};


class SS_Vertex
{
public:
	/*SS_Vertex() : numEdges(0), numFaces(0), cv(0)
	{ 
		memset( edges, 0, sizeof(void*)*6 );
		memset( faces, 0, sizeof(void*)*6 );
	}*/
	SS_Vertex(){}
	int index;
private:
	//char numEdges;
	//SS_Edge *edges[60];
	vector<SS_Edge*> edges;

	//char numFaces;
	//SS_Face *faces[60];
	vector<SS_Face*> faces;

	Point pos;
	Point newPos;

	Point texel;
	Vector normal;

	int creationLevel;

	const Vertex *cv;

	SS_Edge** GetEdgePointer( SS_Edge *ptr );
	SS_Face** GetFacePointer( SS_Face *ptr );
	SS_Vertex* GetNewVertexInEdge( SS_Vertex *v );

	void UpdateNormal();

	friend class SS_Surface;
	friend class SS_Face;
	friend bool operator!=( SS_Vertex*& v, const Vertex &cv );
	friend bool operator==( SS_Vertex*& v, const Vertex &cv );

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif

#ifdef INFO
	void PrintAround();
	void PrintAround(FILE* fp);
#endif

};
typedef vector<SS_Vertex*> SS_VertexList;

bool operator!=( SS_Vertex*& v, const Vertex &cv );
bool operator==( SS_Vertex*& v, const Vertex &cv );


class SS_Face
{
private:
	SS_Vertex *vertices[3];
	SS_Edge *edges[3];

	Vector centroid;
	Vector normal;

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

	friend class SS_Surface;
	friend class SS_Vertex;

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif
};
typedef vector<SS_Face*> SS_FaceList;

class SS_Edge
{
public:
	SS_Edge() : oldVertex(0)								{}

private:
	SS_Vertex *vertices[2];
	SS_Vertex *oldVertex;

	SS_Vertex** GetVertexPointer( SS_Vertex *v )
	{
		if ( vertices[0] == v )
			return &vertices[0];
		else
			return &vertices[1];
	}

	friend class SS_Surface;
	friend class SS_Vertex;
	friend bool operator!=( SS_Edge*& e, const SS_VertexPair &vp );
	friend bool operator==( SS_Edge*& e, const SS_VertexPair &vp );

#if USE_MEM_POOL
	void* operator new(size_t bytes);
	void operator delete(void* p);
	bool mp_inuse;
#endif
};
typedef vector<SS_Edge*> SS_EdgeList;

bool operator!=( SS_Edge*& e, const SS_VertexPair &vp );
bool operator==( SS_Edge*& e, const SS_VertexPair &vp );


class SS_Surface
{
public:
	SS_Surface() : subdivisionLevel(0)				
	{
#if USE_MEM_POOL
		vertices.reserve( MP_VERTS );
		edges.reserve( MP_EDGES );
		faces.reserve( MP_FACES );
#endif
	}
	~SS_Surface();

	void Reset();

	void ReadObj(const char* filename, float scale = 1.0);
	void WriteObj(const char* filename);

	void AddFace( Vertex *cv1, Vertex *cv2, Vertex *cv3 );
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
	SS_Vertex* _AddVertex( Vertex *cv );
	SS_Edge*  _AddEdge( SS_Vertex *v1, SS_Vertex *v2 );

	void SubdivideEdge( SS_Edge *e );
	void SubdivideFace( SS_Face *f );
	void RepositionVertex( SS_Vertex *v );

	int skipSpace(char *str)
	{
		for(int i = 0; str[i]; i++)
			if(str[i] == ' ' || str[i] == '\t')continue;
			else return i;
		return -1;
	}

protected:
	SS_VertexList vertices;
	SS_EdgeList edges;
	SS_FaceList faces;

	int subdivisionLevel;

	vector<Vertex*> vList;
};

#endif // SUBDIVISION_H
