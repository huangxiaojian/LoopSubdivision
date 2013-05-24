
/* This defines makes assert's compile away to nothing.. :> */
#define NDEBUG 1

#include "Subdivision.h"
#include <algorithm>
#include <assert.h>

#include <time.h>

extern float gScale;

#if USE_MEM_POOL
	static LS_Vertex mp_vertex[MP_VERTS];
	static LS_Edge mp_edge[MP_EDGES];
	static LS_Face mp_face[MP_FACES];

	static LS_Vertex* mp_curVertex = mp_vertex;
	static LS_Vertex* mp_maxVertex = &mp_vertex[MP_VERTS];
	static LS_Edge* mp_curEdge = mp_edge;
	static LS_Edge* mp_maxEdge = &mp_edge[MP_EDGES];
	static LS_Face* mp_curFace = mp_face;
	static LS_Face* mp_maxFace = &mp_face[MP_FACES];


void* LS_Vertex::operator new(size_t bytes)
{
	LS_Vertex *startVertex( mp_curVertex );

	while( mp_curVertex->mp_inuse == true )
	{
		mp_curVertex++;
		if ( mp_curVertex == mp_maxVertex )
			mp_curVertex = mp_vertex;
		if ( mp_curVertex == startVertex )
		{
			printf("Out of LS_Vertex memory pool!\n");
			exit(-1);
		}
	}
	mp_curVertex->mp_inuse = true;

	return mp_curVertex;
}

void* LS_Edge::operator new(size_t bytes)
{
	LS_Edge *startEdge( mp_curEdge );

	while( mp_curEdge->mp_inuse == true )
	{
		mp_curEdge++;
		if ( mp_curEdge == mp_maxEdge )
			mp_curEdge = mp_edge;
		if ( mp_curEdge == startEdge )
		{
			printf("Out of LS_Edge memory pool!\n");
			exit(-1);
		}
	}
	mp_curEdge->mp_inuse = true;

	return mp_curEdge;
}

void* LS_Face::operator new(size_t bytes)
{
	LS_Face *startFace( mp_curFace );

	while( mp_curFace->mp_inuse == true )
	{
		mp_curFace++;
		if ( mp_curFace == mp_maxFace )
			mp_curFace = mp_face;
		if ( mp_curFace == startFace )
		{
			printf("Out of LS_Face memory pool!\n");
			exit(-1);
		}
	}
	mp_curFace->mp_inuse = true;

	return mp_curFace;
}

void LS_Vertex::operator delete(void* p)
{
	((LS_Vertex*)p)->mp_inuse = false;
}
void LS_Edge::operator delete(void* p)
{
	((LS_Edge*)p)->mp_inuse = false;
}
void LS_Face::operator delete(void* p)
{
	((LS_Face*)p)->mp_inuse = false;
}
#endif

/*******************************************************************************************
 *
 * LS_Vertex
 *
 *******************************************************************************************/

bool operator!=( LS_Edge*& e, const LS_VertexPair &vp )
{
	return ( !( ( e->vertices[0] == vp.first || e->vertices[0] == vp.second )
		&& ( e->vertices[1] == vp.first || e->vertices[1] == vp.second ) ) );
}

bool operator==( LS_Edge*& e, const LS_VertexPair &vp )
{
	return ( ( e->vertices[0] == vp.first || e->vertices[0] == vp.second )
		&& ( e->vertices[1] == vp.first || e->vertices[1] == vp.second ) );
}

LS_Edge** LS_Vertex::GetEdgePointer( LS_Edge *ptr )
{
	for( int i = 0; i < edges.size(); i++ )
		if ( edges[i] == ptr )
			return &edges[i];
	assert(false);
}

LS_Face** LS_Vertex::GetFacePointer( LS_Face *ptr )
{
	for( int i = 0; i < faces.size(); i++ )
		if ( faces[i] == ptr )
			return &faces[i];
	assert(false);
}

LS_Vertex* LS_Vertex::GetNewVertexInEdge( LS_Vertex *v )
{
	for( int i = 0; i < edges.size(); i++ )
	{	
		if ( edges[i]->oldVertex == v )
			return edges[i]->vertices[1];
	}
	assert(false);
}

void LS_Vertex::UpdateNormal()
{
	normal.Clear();
	for( int i = 0; i < faces.size(); i++ )
		normal += faces[i]->normal;
	normal.Normalize();
}

/*******************************************************************************************
 *
 * LS_Surface
 *
 *******************************************************************************************/

LS_Surface::~LS_Surface()
{
	Reset();
	vertices.clear();
	edges.clear();
	faces.clear();
}

void LS_Surface::Reset()
{
	// Free up allocated memory and clear the lists

	LS_VertexList::iterator vertex_itr = vertices.begin();
	while( vertex_itr != vertices.end() )
	{
		delete *vertex_itr;
		vertex_itr++;
	}
	vertices.clear();
	
	LS_EdgeList::iterator edge_itr = edges.begin();
	while( edge_itr != edges.end() )
	{
		delete *edge_itr;
		edge_itr++;
	}
	edges.clear();

	LS_FaceList::iterator face_itr = faces.begin();
	while( face_itr != faces.end() )
	{
		delete *face_itr;
		face_itr++;
	}
	faces.clear();

	subdivisionLevel = 0;
#if USE_MEM_POOL
	vertices.reserve( MP_VERTS );
	edges.reserve( MP_EDGES );
	faces.reserve( MP_FACES );
#endif
}

LS_Edge* LS_Surface::_AddEdge( LS_Vertex *v1, LS_Vertex *v2 )
{
	// Make sure these are distinct vertices
	assert( (v1 != v2) && (v1 >= 0 && v1 < vertices.size()) && (v2 >= 0 && v2 < vertices.size()) );

	// Make sure the end points are in the list

	assert ( edges.end() == find( edges.begin(), edges.end(), LS_VertexPair( v1, v2 ) ) );

	LS_Edge *newEdge = new LS_Edge;
	assert( newEdge );
	newEdge->vertices[0] = v1;
	newEdge->vertices[1] = v2;

	// Increment the vertex edge counter
	v1->edges.push_back(newEdge);
	v2->edges.push_back(newEdge);

	edges.push_back( newEdge );

	return newEdge;
}

void LS_Surface::SubdivideEdge( LS_Edge *e )
{
	LS_Vertex *newVertex = new LS_Vertex;
	assert( newVertex );
	LS_Edge *newEdge = new LS_Edge;
	assert( newEdge );

	// Add these new objects to the lists
	edges.push_back( newEdge );
	vertices.push_back( newVertex );

	newVertex->index = vertices.size();

	// Set the creationLevel of the new vertex
	newVertex->creationLevel = subdivisionLevel;

	// Set the position of the new vertex (lerp) 
	newVertex->pos = e->vertices[0]->pos 
			+ ( e->vertices[1]->pos - e->vertices[0]->pos ) / 2;
	// Set the texel of the new vertex (lerp)
	newVertex->texel = e->vertices[0]->texel
			+ ( e->vertices[1]->texel - e->vertices[0]->texel ) / 2;

	// Set the new edge to point to the correct vertices
	newEdge->vertices[0] = newVertex;
	newEdge->vertices[1] = e->vertices[1];
	newEdge->oldVertex = e->vertices[0];
	// Save the vertex that this edge used to connect to
	e->oldVertex = e->vertices[1];
	// Set the old edge to point to the next vertex
	e->vertices[1] = newVertex;

	// Set the new vertex's edge pointers
	newVertex->edges.push_back(e);
	newVertex->edges.push_back(newEdge);
	
	// Set the other vertex's edge pointer
	LS_Edge **e_ptr = NULL;
	e_ptr = newEdge->vertices[1]->GetEdgePointer( e );
	*e_ptr = newEdge;
}

void LS_Surface::SubdivideFace( LS_Face *f )
{
	// Store the original face pointer
	LS_Face *origFace = f;

	// Find the new vertices that were created inside this face
	LS_Vertex *v1 = NULL, *v2 = NULL, *v3 = NULL;
	v1 = f->vertices[0]->GetNewVertexInEdge( f->vertices[1] );
	if ( v1 == f->vertices[0] )
	{
		v1 = f->vertices[1]->GetNewVertexInEdge( f->vertices[0] );
		assert( v1 );
	}
	v2 = f->vertices[1]->GetNewVertexInEdge( f->vertices[2] );
	if ( v2 == f->vertices[1] )
	{
		v2 = f->vertices[2]->GetNewVertexInEdge( f->vertices[1] );
		assert( v2 );
	}
	v3 = f->vertices[2]->GetNewVertexInEdge( f->vertices[0] );
	if ( v3 == f->vertices[2] )
	{
		v3 = f->vertices[0]->GetNewVertexInEdge( f->vertices[2] );
		assert( v3 );
	}

	// Store the original face corners
	LS_Vertex *c1, *c2, *c3;
	c1 = f->vertices[0];
	c2 = f->vertices[1];
	c3 = f->vertices[2];

	assert( *c1->GetFacePointer( origFace ) == origFace );
	assert( *c2->GetFacePointer( origFace ) == origFace );
	assert( *c3->GetFacePointer( origFace ) == origFace );
	
	// Create the new face (c1, v1, v3)
	f->vertices[0] = c1;
	f->vertices[1] = v1;
	f->vertices[2] = v3;
	f->UpdateCentroid();
	_AddEdge( v1, v3 );
	// Fix the face pointers
	v1->faces.push_back(f);
	v3->faces.push_back(f);
	
	// Create the new face (v1, c2, v2)
	f = new LS_Face;
	assert( f );
	faces.push_back( f );
	f->vertices[0] = v1;
	f->vertices[1] = c2;
	f->vertices[2] = v2;
	f->UpdateCentroid();
	_AddEdge( v1, v2 );
	// Fix the face pointers
	v1->faces.push_back(f);
	v2->faces.push_back(f);

	*c2->GetFacePointer( origFace ) = f;

	// Create the new face (v2, c3, v3)
	f = new LS_Face;
	assert( f );
	faces.push_back( f );
	f->vertices[0] = v2;
	f->vertices[1] = c3;
	f->vertices[2] = v3;
	f->UpdateCentroid();
	_AddEdge( v2, v3 );
	// Fix the face pointers
	v2->faces.push_back(f);
	v3->faces.push_back(f);
	
	*c3->GetFacePointer( origFace ) = f;

	// Create the new face (v1, v2, v3)
	f = new LS_Face;
	assert( f );
	faces.push_back( f );
	f->vertices[0] = v1;
	f->vertices[1] = v2;
	f->vertices[2] = v3;
	f->UpdateCentroid();
	// Fix the face pointers
	v1->faces.push_back(f);
	v2->faces.push_back(f);
	v3->faces.push_back(f);
}

void LS_Surface::RepositionVertex( LS_Vertex *v )
{
	assert( v->numFaces > 0 );
	LSVector cf;

	cf = v->pos.ToVector() * ( v->faces.size() * 0.25 );
	for( int i = 0; i < v->faces.size(); i++ )
	{
		int curFaceVert = 0;
		if ( v->faces[i]->vertices[ curFaceVert ] == v )
			curFaceVert++;
		cf += v->faces[i]->vertices[ curFaceVert ]->pos.ToVector() * 0.375;

		curFaceVert++;
		if ( v->faces[i]->vertices[ curFaceVert ] == v )
			curFaceVert++;
		cf += v->faces[i]->vertices[ curFaceVert ]->pos.ToVector() * 0.375;
	}

	cf = cf / (double)v->faces.size();
	v->newPos = cf.ToPoint();
}

void LS_Surface::Subdivide()
{
	subdivisionLevel++;
	clock_t start, finish;
	double duration;
	// Linear subdivision of edges
	LS_EdgeList::iterator edge_itr = edges.begin();
	int numEdges = edges.size();
	int numVertices = vertices.size();
	for( int i = 0; i < numEdges; i++, edge_itr++ )
	{
		// Only subdivide old edges (ones not attached to a new vertex)
		if ( (*edge_itr)->vertices[0]->creationLevel != subdivisionLevel
					&& (*edge_itr)->vertices[1]->creationLevel != subdivisionLevel )
			SubdivideEdge( *edge_itr );
	}

	// Create the new faces / Connect the new vertices
	LS_FaceList::iterator face_itr = faces.begin();
	int orinumfaces = faces.size();
	for( int i = 0; i < orinumfaces; i++, face_itr++ )
	{
		// Connect the new vertices into faces
		SubdivideFace( *face_itr );
	}

	// Reposition new vertices
	LS_VertexList::iterator vertex_itr = vertices.begin();
	for( int i = 0; i < vertices.size(); i++, vertex_itr++ )
	{
		RepositionVertex( *vertex_itr );
	}

	vertex_itr = vertices.begin();
	for( int i = 0; i < vertices.size(); i++, vertex_itr++ )
		(*vertex_itr)->pos = (*vertex_itr)->newPos;

	UpdateNormals();
}

void LS_Surface::UpdateNormals()
{
	LS_FaceList::iterator face_itr = faces.begin();
	while( face_itr != faces.end() )
	{
		(*face_itr)->UpdateNormal();
		face_itr++;
	}

	LS_VertexList::iterator vertex_itr = vertices.begin();
	while( vertex_itr != vertices.end() )
	{
		(*vertex_itr)->UpdateNormal();
		vertex_itr++;
	}
}

void LS_Surface::Draw( DRAWFLAGS flags )
{
	if ( flags & WIREFRAME )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			
	if ( flags & SOLID && flags & FLAT )
	{
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		LS_FaceList::iterator f_itr = faces.begin();
		glColor3f(1, 1, 1);
		while( f_itr != faces.end() )
		{
			(*f_itr)->normal.glNormal();
			(*f_itr)->vertices[0]->texel.glTexel();
			(*f_itr)->vertices[0]->pos.gl();
			(*f_itr)->vertices[1]->texel.glTexel();
			(*f_itr)->vertices[1]->pos.gl();
			(*f_itr)->vertices[2]->texel.glTexel();
			(*f_itr)->vertices[2]->pos.gl();
	
			f_itr++;
		}
		glEnd();
	}
	else if ( flags & SOLID )
	{
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		LS_FaceList::iterator f_itr = faces.begin();
		glColor3f(1, 1, 1);
		while( f_itr != faces.end() )
		{
			(*f_itr)->vertices[0]->texel.glTexel();
			(*f_itr)->vertices[0]->normal.glNormal();
			(*f_itr)->vertices[0]->pos.gl();
			(*f_itr)->vertices[1]->texel.glTexel();
			(*f_itr)->vertices[1]->normal.glNormal();
			(*f_itr)->vertices[1]->pos.gl();
			(*f_itr)->vertices[2]->texel.glTexel();
			(*f_itr)->vertices[2]->normal.glNormal();
			(*f_itr)->vertices[2]->pos.gl();
	
			f_itr++;
		}
		glEnd();
	}

	GLboolean oldLights;
	glGetBooleanv(GL_LIGHTING, &oldLights);
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_ALWAYS);

	if ( flags & NORMALS )
	{
		LS_VertexList::iterator v_itr = vertices.begin();
		glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		while( v_itr != vertices.end() )
		{
			(*v_itr)->pos.gl();
			( (*v_itr)->pos + (*v_itr)->normal * 3 ).gl();
			v_itr++;
		}
		glEnd();
	}

	if ( flags & POINTS )
	{
		glPointSize(5);
		glBegin(GL_POINTS);
		LS_VertexList::iterator v_itr = vertices.begin();
		glColor3f(0, 1, 0);
		while( v_itr != vertices.end() )
		{
			(*v_itr)->pos.gl();
			v_itr++;
		}
		glEnd();
	}

	if ( oldLights )
		glEnable(GL_LIGHTING);
	glDepthFunc(GL_LESS);

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void LS_Surface::WriteObj(const char* filename)
{
#ifdef _CRT_SECURE_NO_WARNINGS
	FILE* fp = fopen(filename, "r");
#else
	FILE* fp;
	fopen_s(&fp, filename, "w");
#endif
	fprintf(fp, "# loop\n");
	for(LS_VertexList::iterator vitr = vertices.begin(); vitr != vertices.end(); vitr++)
	{
		fprintf(fp, "v %f %f %f\n", (*vitr)->pos.x, (*vitr)->pos.y, (*vitr)->pos.z);
	}
	for(LS_FaceList::iterator fitr = faces.begin(); fitr != faces.end(); fitr++)
	{
		fprintf(fp, "f %d %d %d\n", (*fitr)->vertices[0]->index, (*fitr)->vertices[1]->index, (*fitr)->vertices[2]->index);
	}
	fclose(fp);
}

void LS_Surface::ReadObj(const char* filename, float scale)
{
	Reset();
#ifdef _CRT_SECURE_NO_WARNINGS
	FILE* fp = fopen(filename, "r");
#else
	FILE* fp;
	fopen_s(&fp, filename, "r");
#endif
	if(!fp)
	{
		printf("open %s failed\n", filename);
		exit(0);
	}
	char str[256];
	while(!feof(fp))
	{
		fgets(str, 255, fp);
		int i = skipSpace(str);
		if(i == -1 || str[i] == '\n' || str[i] == '#')
			continue;
		if(str[i] == 'v')
		{
			//LSPoint p;
			LS_Vertex* v = new LS_Vertex();
#ifdef _CRT_SECURE_NO_WARNINGS
			sscanf(str+i+1, "%lf%lf%lf", &p.x, &p.y, &p.z);
#else
			sscanf_s(str+i+1, "%lf%lf%lf", &v->pos.x, &v->pos.y, &v->pos.z);
#endif
			v->pos.x *= scale;
			v->pos.y *= scale;
			v->pos.z *= scale;

			vertices.push_back(v);
			v->creationLevel = subdivisionLevel;
			v->index = vertices.size();
		}
		else if(str[i] = 'f')
		{
			int v1, v2, v3;
#ifdef _CRT_SECURE_NO_WARNINGS
			sscanf(str+i+1, "%d%d%d", &v1, &v2, &v3);
#else
			sscanf_s(str+i+1, "%d%d%d", &v1, &v2, &v3);
#endif
			AddFaceOfIndex(v1-1, v2-1, v3-1);
		}
	}
	fclose(fp);
}

void LS_Surface::AddFaceOfIndex(int i1, int i2, int i3)
{
	LS_Vertex *v1 = vertices[i1], *v2 = vertices[i2], *v3 = vertices[i3];
	LS_VertexList::iterator v_itr;
	LS_Edge *e1, *e2, *e3;
	LS_EdgeList::iterator e_itr;

	// Find the edges in the list, or insert them
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), LS_VertexPair( v1, v2 ) ) ) )
		e1 = _AddEdge( v1, v2 );
	else
		e1 = *e_itr;
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), LS_VertexPair( v2, v3 ) ) ) )
		e2 = _AddEdge( v2, v3 );
	else
		e2 = *e_itr;
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), LS_VertexPair( v3, v1 ) ) ) )
		e3 = _AddEdge( v3, v1 );
	else
		e3 = *e_itr;

	LS_Face *newFace = new LS_Face;
	assert( newFace );
	newFace->vertices[0] = v1;
	newFace->vertices[1] = v2;
	newFace->vertices[2] = v3;
	newFace->edges[0] = e1;
	newFace->edges[1] = e2;
	newFace->edges[2] = e3;
	newFace->UpdateCentroid();
	faces.push_back( newFace );

	// Calculate the face normal
	LSVector normal;
	normal = ( v2->pos - v1->pos ).Cross( v3->pos - v1->pos );
	normal.Normalize();
	// Set the face normals
	v1->normal = normal;
	v2->normal = normal;
	v3->normal = normal;

	v1->faces.push_back(newFace);
	v2->faces.push_back(newFace);
	v3->faces.push_back(newFace);
}