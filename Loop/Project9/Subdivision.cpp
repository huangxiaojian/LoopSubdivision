
/* This defines makes assert's compile away to nothing.. :> */
#define NDEBUG 1

#include "Subdivision.h"
#include <algorithm>
#include <assert.h>

extern float gScale;

#if USE_MEM_POOL
	static SS_Vertex mp_vertex[MP_VERTS];
	static SS_Edge mp_edge[MP_EDGES];
	static SS_Face mp_face[MP_FACES];

	static SS_Vertex* mp_curVertex = mp_vertex;
	static SS_Vertex* mp_maxVertex = &mp_vertex[MP_VERTS];
	static SS_Edge* mp_curEdge = mp_edge;
	static SS_Edge* mp_maxEdge = &mp_edge[MP_EDGES];
	static SS_Face* mp_curFace = mp_face;
	static SS_Face* mp_maxFace = &mp_face[MP_FACES];


#ifdef INFO
void SS_Vertex::PrintAround()
{
	printf("{\n%d:\tnumEages = %d\n", index, edges.size());
	for(int i = 0; i < edges.size(); i++)
	{
		printf("[%d], [%d]\n", edges[i]->vertices[0]->index, edges[i]->vertices[1]->index);
	}
	printf("}\n");
}
void SS_Vertex::PrintAround(FILE* fp)
{
	fprintf(fp, "{\n\t#%d:numEages = %d\n", index, edges.size());
	for(int i = 0; i < edges.size(); i++)
	{
		fprintf(fp, "\t[%d], [%d]\n", edges[i]->vertices[0]->index, edges[i]->vertices[1]->index);
	}
	fprintf(fp, "}\n");
}

#endif

void* SS_Vertex::operator new(size_t bytes)
{
	SS_Vertex *startVertex( mp_curVertex );

	while( mp_curVertex->mp_inuse == true )
	{
		mp_curVertex++;
		if ( mp_curVertex == mp_maxVertex )
			mp_curVertex = mp_vertex;
		if ( mp_curVertex == startVertex )
		{
			printf("Out of SS_Vertex memory pool!\n");
			exit(-1);
		}
	}
	mp_curVertex->mp_inuse = true;

	return mp_curVertex;
}
void* SS_Edge::operator new(size_t bytes)
{
	SS_Edge *startEdge( mp_curEdge );

	while( mp_curEdge->mp_inuse == true )
	{
		mp_curEdge++;
		if ( mp_curEdge == mp_maxEdge )
			mp_curEdge = mp_edge;
		if ( mp_curEdge == startEdge )
		{
			printf("Out of SS_Edge memory pool!\n");
			exit(-1);
		}
	}
	mp_curEdge->mp_inuse = true;

	return mp_curEdge;
}
void* SS_Face::operator new(size_t bytes)
{
	SS_Face *startFace( mp_curFace );

	while( mp_curFace->mp_inuse == true )
	{
		mp_curFace++;
		if ( mp_curFace == mp_maxFace )
			mp_curFace = mp_face;
		if ( mp_curFace == startFace )
		{
			printf("Out of SS_Face memory pool!\n");
			exit(-1);
		}
	}
	mp_curFace->mp_inuse = true;

	return mp_curFace;
}

void SS_Vertex::operator delete(void* p)
{
	((SS_Vertex*)p)->mp_inuse = false;
}
void SS_Edge::operator delete(void* p)
{
	((SS_Edge*)p)->mp_inuse = false;
}
void SS_Face::operator delete(void* p)
{
	((SS_Face*)p)->mp_inuse = false;
}
#endif

/*******************************************************************************************
 *
 * SS_Vertex
 *
 *******************************************************************************************/

bool operator!=( SS_Vertex*& v, const Vertex &cv )
{
	return ( v->cv != &cv );
}
bool operator!=( SS_Edge*& e, const SS_VertexPair &vp )
{
	return ( !( ( e->vertices[0] == vp.first || e->vertices[0] == vp.second )
		&& ( e->vertices[1] == vp.first || e->vertices[1] == vp.second ) ) );
}
bool operator==( SS_Vertex*& v, const Vertex &cv )
{
	return ( v->cv == &cv );
}
bool operator==( SS_Edge*& e, const SS_VertexPair &vp )
{
	return ( ( e->vertices[0] == vp.first || e->vertices[0] == vp.second )
		&& ( e->vertices[1] == vp.first || e->vertices[1] == vp.second ) );
}

SS_Edge** SS_Vertex::GetEdgePointer( SS_Edge *ptr )
{
	for( int i = 0; i < edges.size(); i++ )
		if ( edges[i] == ptr )
			return &edges[i];
	assert(false);
}

SS_Face** SS_Vertex::GetFacePointer( SS_Face *ptr )
{
	for( int i = 0; i < faces.size(); i++ )
		if ( faces[i] == ptr )
			return &faces[i];
	assert(false);
}

SS_Vertex* SS_Vertex::GetNewVertexInEdge( SS_Vertex *v )
{
	for( int i = 0; i < edges.size(); i++ )
	{	
		if ( edges[i]->oldVertex == v )
			return edges[i]->vertices[1];
	}
	assert(false);
}

void SS_Vertex::UpdateNormal()
{
	normal.Clear();
	for( int i = 0; i < faces.size(); i++ )
		normal += faces[i]->normal;
	normal.Normalize();
}

/*******************************************************************************************
 *
 * SS_Surface
 *
 *******************************************************************************************/

SS_Surface::~SS_Surface()
{
	Reset();
}

void SS_Surface::Reset()
{
	// Free up allocated memory and clear the lists

	SS_VertexList::iterator vertex_itr = vertices.begin();
	while( vertex_itr != vertices.end() )
	{
		delete *vertex_itr;
		vertex_itr++;
	}
	vertices.clear();
	
	SS_EdgeList::iterator edge_itr = edges.begin();
	while( edge_itr != edges.end() )
	{
		delete *edge_itr;
		edge_itr++;
	}
	edges.clear();

	SS_FaceList::iterator face_itr = faces.begin();
	while( face_itr != faces.end() )
	{
		delete *face_itr;
		face_itr++;
	}
	faces.clear();

	vList.clear();
}

void SS_Surface::AddFace( Vertex *cv1, Vertex *cv2, Vertex *cv3 )
{
	if ( subdivisionLevel != 0 )
		assert( false && "Can't add faces after surface has been subdivided!" );

	SS_Vertex *v1, *v2, *v3;
	SS_VertexList::iterator v_itr;
	SS_Edge *e1, *e2, *e3;
	SS_EdgeList::iterator e_itr;

	// Find the vertices in the list, or insert them
	if ( vertices.end() == ( v_itr = find( vertices.begin(), vertices.end(), *cv1 ) ) )
		v1 = _AddVertex( cv1 );
	else
		v1 = *v_itr;//find it in the list
	if ( vertices.end() == ( v_itr = find( vertices.begin(), vertices.end(), *cv2 ) ) )
		v2 = _AddVertex( cv2 );
	else
		v2 = *v_itr;
	if ( vertices.end() == ( v_itr = find( vertices.begin(), vertices.end(), *cv3 ) ) )
		v3 = _AddVertex( cv3 );
	else
		v3 = *v_itr;

	// Find the edges in the list, or insert them
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), SS_VertexPair( v1, v2 ) ) ) )
		e1 = _AddEdge( v1, v2 );
	else
		e1 = *e_itr;
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), SS_VertexPair( v2, v3 ) ) ) )
		e2 = _AddEdge( v2, v3 );
	else
		e2 = *e_itr;
	if ( edges.end() == 
				( e_itr = find( edges.begin(), edges.end(), SS_VertexPair( v3, v1 ) ) ) )
		e3 = _AddEdge( v3, v1 );
	else
		e3 = *e_itr;

	SS_Face *newFace = new SS_Face;
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
	Vector normal;
	normal = ( v2->pos - v1->pos ).Cross( v3->pos - v1->pos );
	normal.Normalize();
	// Set the face normals
	v1->normal = normal;
	v2->normal = normal;
	v3->normal = normal;

	v1->faces.push_back(newFace);
	v2->faces.push_back(newFace);
	v3->faces.push_back(newFace);
	/*v1->faces[ v1->numFaces++ ] = newFace;
	assert( v1->numFaces <= 6 );
	v2->faces[ v2->numFaces++ ] = newFace;
	assert( v2->numFaces <= 6 );
	v3->faces[ v3->numFaces++ ] = newFace;
	assert( v3->numFaces <= 6 );*/
}

SS_Vertex* SS_Surface::_AddVertex( Vertex *cv )
{
	// Make sure that this vertex is not already in the list
	assert( vertices.end() == find( vertices.begin(), vertices.end(), *cv ) );

	SS_Vertex *newVertex = new SS_Vertex;
	assert( newVertex );

	newVertex->creationLevel = subdivisionLevel;
	newVertex->cv = cv;
	newVertex->pos = cv->pos;
	newVertex->texel = cv->texel.ToPoint();

	vertices.push_back( newVertex );
	
	newVertex->index = cv->index;

	return newVertex;
}

SS_Edge* SS_Surface::_AddEdge( SS_Vertex *v1, SS_Vertex *v2 )
{
	// Make sure these are distinct vertices
	assert( v1 != v2 );

	// Make sure the end points are in the list
	assert( vertices.end() != find( vertices.begin(), vertices.end(), v1 ) );
	assert( vertices.end() != find( vertices.begin(), vertices.end(), v2 ) );

	assert ( edges.end() == find( edges.begin(), edges.end(), SS_VertexPair( v1, v2 ) ) );

	SS_Edge *newEdge = new SS_Edge;
	assert( newEdge );
	newEdge->vertices[0] = v1;
	newEdge->vertices[1] = v2;

	// Increment the vertex edge counter
	v1->edges.push_back(newEdge);
	v2->edges.push_back(newEdge);
	/*v1->edges[ v1->numEdges++ ] = newEdge;
	assert( v1->numEdges <= 6 );
	v2->edges[ v2->numEdges++ ] = newEdge;
	assert( v2->numEdges <= 6 );*/

	edges.push_back( newEdge );

	return newEdge;
}

void SS_Surface::SubdivideEdge( SS_Edge *e )
{
	SS_Vertex *newVertex = new SS_Vertex;
	assert( newVertex );
	SS_Edge *newEdge = new SS_Edge;
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
	//newVertex->edges[ newVertex->numEdges++ ] = e;
	//newVertex->edges[ newVertex->numEdges++ ] = newEdge;
	//assert( newVertex->numEdges <= 6 );
	
	// Set the other vertex's edge pointer
	SS_Edge **e_ptr = NULL;
	e_ptr = newEdge->vertices[1]->GetEdgePointer( e );
	*e_ptr = newEdge;
}

void SS_Surface::SubdivideFace( SS_Face *f )
{
	// Store the original face pointer
	SS_Face *origFace = f;

	// Find the new vertices that were created inside this face
	SS_Vertex *v1, *v2, *v3;
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
	SS_Vertex *c1, *c2, *c3;
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
	//v1->faces[ v1->numFaces++ ] = f;
	//v3->faces[ v3->numFaces++ ] = f;
	
	// Create the new face (v1, c2, v2)
	f = new SS_Face;
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
	//v1->faces[ v1->numFaces++ ] = f;
	//v2->faces[ v2->numFaces++ ] = f;
	*c2->GetFacePointer( origFace ) = f;

	// Create the new face (v2, c3, v3)
	f = new SS_Face;
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
	//v2->faces[ v2->numFaces++ ] = f;
	//v3->faces[ v3->numFaces++ ] = f;
	*c3->GetFacePointer( origFace ) = f;

	// Create the new face (v1, v2, v3)
	f = new SS_Face;
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
	//v1->faces[ v1->numFaces++ ] = f;
	//v2->faces[ v2->numFaces++ ] = f;
	//v3->faces[ v3->numFaces++ ] = f;
}

void SS_Surface::RepositionVertex( SS_Vertex *v )
{
	assert( v->numFaces > 0 );
	Vector cf;

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

void SS_Surface::Subdivide()
{
	subdivisionLevel++;

	// Linear subdivision of edges
	SS_EdgeList::iterator edge_itr = edges.begin();
	int numEdges = edges.size();
	int numVertices = vertices.size();
	for( int i = 0; i < numEdges; i++, edge_itr++ )
	{
		// Only subdivide old edges (ones not attached to a new vertex)
		if ( (*edge_itr)->vertices[0]->creationLevel != subdivisionLevel
					&& (*edge_itr)->vertices[1]->creationLevel != subdivisionLevel )
			SubdivideEdge( *edge_itr );
		else
		{
			printf("fuck you\n");
			getchar();
		}
	}

	// Create the new faces / Connect the new vertices
	SS_FaceList::iterator face_itr = faces.begin();
	int orinumfaces = faces.size();
	for( int i = 0; i < orinumfaces; i++, face_itr++ )
	{
		//printf("%d %d\n",faces.size(), i);
		// Connect the new vertices into faces
		SubdivideFace( *face_itr );
	}
	
	// Reposition new vertices
	SS_VertexList::iterator vertex_itr = vertices.begin();
	for( int i = 0; i < vertices.size(); i++, vertex_itr++ )
	{
		RepositionVertex( *vertex_itr );
	}
	vertex_itr = vertices.begin();
	for( int i = 0; i < vertices.size(); i++, vertex_itr++ )
		(*vertex_itr)->pos = (*vertex_itr)->newPos;

	UpdateNormals();

	/*
	printf("Subdivided (%i levels)\n" 
		   " %i vertices\n"
		   " %i edges\n"
		   " %i faces\n"
		   , subdivisionLevel
		   , vertices.size()
		   , edges.size()
		   , faces.size() );
	*/
}

void SS_Surface::UpdateNormals()
{
	SS_FaceList::iterator face_itr = faces.begin();
	while( face_itr != faces.end() )
	{
		(*face_itr)->UpdateNormal();
		face_itr++;
	}

	SS_VertexList::iterator vertex_itr = vertices.begin();
	while( vertex_itr != vertices.end() )
	{
		(*vertex_itr)->UpdateNormal();
		vertex_itr++;
	}
}

void SS_Surface::Draw( DRAWFLAGS flags )
{
	if ( flags & WIREFRAME )
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			
	if ( flags & SOLID && flags & FLAT )
	{
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		SS_FaceList::iterator f_itr = faces.begin();
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
		SS_FaceList::iterator f_itr = faces.begin();
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
		SS_VertexList::iterator v_itr = vertices.begin();
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
		SS_VertexList::iterator v_itr = vertices.begin();
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

void SS_Surface::ReadObj(const char* filename, float scale)
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
			Point p;
#ifdef _CRT_SECURE_NO_WARNINGS
			sscanf(str+i+1, "%lf%lf%lf", &p.x, &p.y, &p.z);
#else
			sscanf_s(str+i+1, "%lf%lf%lf", &p.x, &p.y, &p.z);
#endif
			p.x *= scale;
			p.y *= scale;
			p.z *= scale;
			Vertex *v = new Vertex();
			v->pos = p;

			v->index = vList.size()+1;
			vList.push_back(v);
		}
		else if(str[i] = 'f')
		{
			int v1, v2, v3;
#ifdef _CRT_SECURE_NO_WARNINGS
			sscanf(str+i+1, "%d%d%d", &v1, &v2, &v3);
#else
			sscanf_s(str+i+1, "%d%d%d", &v1, &v2, &v3);
#endif
			AddFace(vList[v1-1], vList[v2-1], vList[v3-1]);
		}
	}
	fclose(fp);
}

void SS_Surface::WriteObj(const char* filename)
{
#ifdef _CRT_SECURE_NO_WARNINGS
	FILE* fp = fopen(filename, "r");
#else
	FILE* fp;
	fopen_s(&fp, filename, "w");
#endif
	fprintf(fp, "# loop\n");
	for(SS_VertexList::iterator vitr = vertices.begin(); vitr != vertices.end(); vitr++)
	{
		printf("%d\n", (*vitr)->index);
		fprintf(fp, "v %f %f %f\n", (*vitr)->pos.x, (*vitr)->pos.y, (*vitr)->pos.z);
	}
	for(SS_FaceList::iterator fitr = faces.begin(); fitr != faces.end(); fitr++)
	{
		fprintf(fp, "f %d %d %d\n", (*fitr)->vertices[0]->index, (*fitr)->vertices[1]->index, (*fitr)->vertices[2]->index);
	}
	fclose(fp);
}