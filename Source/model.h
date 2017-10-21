#ifndef __GWU_MODEL__
#define __GWU_MODEL__

//================================
// ModelFace
//================================
class ModelFace {
public :
	std::vector< int > indices;

public :
	ModelFace() {
	}

	~ModelFace() {
	}
};

//================================
// Model
//================================
class Model {
public:
	std::vector< vec3 > verts;
	std::vector< ModelFace > faces;
	std::vector<vec3>  colors;

public:
	Model() {
	}

	~Model() {
	}

	//=============================================
	// Load Model
	//=============================================
	void Free(void) {
		verts.clear();
		faces.clear();
		colors.clear();
	}

	bool LoadModel(const char* path) {
		if (!path) {
			//printf("not find");
			return false;
		}

		Free();

		// open file
		FILE *fp = fopen(path, "r");
		if (!fp) {
			//printf("not find");
			return false;
		}

		unsigned int numVerts = 0;
		unsigned int numFaces = 0;
		// num of vertices and indices
		fscanf(fp, "data%d%d", &numVerts, &numFaces);

		// alloc vertex and index buffer
		verts.resize(numVerts);
		faces.resize(numFaces);
		colors.resize(numFaces);
		// read vertices
		for (unsigned int i = 0; i < numVerts; i++) {
			fscanf(fp, "%f%f%f", &verts[i].x, &verts[i].y, &verts[i].z);
		}

		// read indices
		for (unsigned int i = 0; i < numFaces; i++) {
			int numSides = 0;
			fscanf(fp, "%i", &numSides);
			faces[i].indices.resize(numSides);
			colors[i].x = rand() % 255;
			colors[i].y = rand() % 255;
			colors[i].z = rand() % 255;
			for (unsigned int k = 0; k < faces[i].indices.size(); k++) {
				fscanf(fp, "%i", &faces[i].indices[k]);
			}
		}

		// close file
		fclose(fp);

		ResizeModel();

		return true;
	}

	//=============================================
	// Render Model
	//=============================================
	void DrawEdges2D(void) {
		glBegin(GL_LINES);
		for (unsigned int i = 0; i < faces.size(); i++) {
			for (unsigned int k = 0; k < faces[i].indices.size(); k++) {
				int p0 = faces[i].indices[k];
				int p1 = faces[i].indices[(k + 1) % faces[i].indices.size()];
				glVertex2fv(verts[p0].ptr());
				glVertex2fv(verts[p1].ptr());
			}
		}
		glEnd();
	}

	void DrawEdges(void) {
		vec3 color(0,0,0);
		glBegin(GL_LINES);
		for (unsigned int i = 0; i < faces.size(); i++) {
			//color = Color_smooth(i, faces.size());
			//glColor3f(color.x, color.y, color.z);
			for (unsigned int k = 0; k < faces[i].indices.size(); k++) {			
				color = Color_smooth(k, 256+rand()%1024);
				glColor3f(color.x, color.y, color.z);
				int p0 = faces[i].indices[k];
				int p1 = faces[i].indices[(k + 1) % faces[i].indices.size()];
				glVertex3fv(verts[p0].ptr());
				glVertex3fv(verts[p1].ptr());
			}
		}
		glEnd();
	}
	//draw flat model
	void DrawFlat(void)
	{
		// surface material attributes
		GLfloat material_Ka[] = { 0.11f, 0.06f, 0.11f, 1.0f };
		GLfloat material_Kd[] = { 0.43f, 0.47f, 0.54f, 1.0f };
		GLfloat material_Ks[] = { 0.55f, 0.33f, 0.52f, 1.0f };
		GLfloat material_Ke[] = { 0.10f, 0.00f, 0.10f, 1.0f };
		GLfloat material_Se = 10;
		//backface removal
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);            
		vec3 color(0,0,0);
		  for (unsigned int i = 0; i < faces.size(); i++) {
			  color = colors[i];
			 // glColor4ub(color.x, color.y, color.z,255);    //fill color
			  glColor3f(1.0, 0, 0);
			 
			  glBegin(GL_POLYGON);                          //fill with polygon
			   // surface material attributes
			  for (unsigned int k = 0; k < faces[i].indices.size(); k++) {
				  int p0 = faces[i].indices[k];
				  int p1 = faces[i].indices[(k + 1) % faces[i].indices.size()];
				  //glColor3f(color.x, color.y, color.z);
				  glNormal3fv(verts[p0].ptr());             // add appropriate normals
				  glVertex3fv(verts[p0].ptr());              
				//  color.set(color.x,color.y,color.z);
				
			  }
			  glEnd();
		  }
		  glDisable(GL_CULL_FACE);
	 }

	vec3 Color_smooth(int i,int n)
	{
		vec3 color;
		int r, g, b;
		if (i >= 0 && i <= n / 5)
		{
			r = 255;
			g = i * 255 / (n / 5);
			b = 0;
		}
		else if (i>n / 5 && i <= 2 * n / 5)
		{
			r = 255 - (i - n / 5) * 255 / (n / 5);
			g = 255;
			b = 0;
		}
		else if (i>2 * n / 5 && i <= 3 * n / 5)
		{
			r = 0;
			g = 255;
			b = (i - 2 * n / 5) * 255 / (n / 5);
		}
		else if (i>3 * n / 5 && i <= 4 * n / 5)
		{
			r = 0;
			g = 255 - (i - 3 * n / 5) * 255 / (n / 5);
			b = 255;
		}
		else
		{
			r = (i - 4 * n / 5) * 255 / (n / 5);
			g = 0;
			b = 255;
		}
		color.x = r, color.y = g, color.z=b;
		return color;
	 }
	//=============================================
	// Resize Model
	//=============================================
	// scale the model into the range of [ -0.9, 0.9 ]
	void ResizeModel( void ) {
		// bound
		vec3 min, max;
		if ( !CalcBound( min, max ) ) {
			return;
		}

		// max side
		vec3 size = max - min;

		float r = size.x;
		if ( size.y > r ) {
			r = size.y;
		}
		if ( size.z > r ) {
			r = size.z;
		}

		if ( r < 1e-6f ) {
			r = 0;
		} else {
			r = 1.0f / r;
		}

		// scale
		for ( unsigned int i = 0; i < verts.size(); i++ ) {
			// [0, 1]
			verts[ i ] = ( verts[ i ] - min ) * r;
			
			// [-1, 1]
			verts[ i ] = verts[ i ] * 2.0f - vec3( 1.0f, 1.0f, 1.0f );

			// [-0.9, 0.9]
			verts[ i ] *= 0.9;
		}
	}
	
	bool CalcBound( vec3 &min, vec3 &max ) {
		if ( verts.size() <= 0 ) {
			return false;
		}

		min = verts[ 0 ];
		max = verts[ 0 ];

		for ( unsigned int i = 1; i < verts.size(); i++ ) {
			vec3 v = verts[ i ];

			if ( v.x < min.x ) {
				min.x = v.x;
			} else if ( v.x > max.x ) {
				max.x = v.x;
			}

			if ( v.y < min.y ) {
				min.y = v.y;
			} else if ( v.y > max.y ) {
				max.y = v.y;
			}

			if ( v.z < min.z ) {
				min.z = v.z;
			} else if ( v.z > max.z ) {
				max.z = v.z;
			}
		}

		return true;
	}

	//=============================================
	// Transform Model
	//=============================================
	// scale model
	void Scale( float r ) {
		for ( unsigned int i = 0; i < verts.size(); i++ ) {
			verts[ i ] *= r;
		}
	}

	void Translate( const vec3 &offset ) {
		// translate ...
	}

	void Rotate(float angle, int axis) {
		// rotate ...
		vec3 c = center();
		glTranslatef(c.x, c.y, c.z);
		switch (axis)
		{
		case 0: glRotatef(angle, 0.0, 0.0, 1.0);   //z-axis
			break;
		case 1: glRotatef(angle, 1.0, 0.0, 0.0);   //x-axis
			break;
		case 2: glRotatef(angle, 0.0, 1.0, 0.0);   //y-axis
			break;

		}

		glTranslatef(-c.x, -c.y, -c.z);

	}
	vec3 center()
	{
		vec3 min = verts[0];
		vec3 max = verts[0];
		vec3 center;
		for (unsigned int i = 1; i < verts.size(); i++) {
			vec3 v = verts[i];

			if (v.x < min.x) {
				min.x = v.x;
			}
			else if (v.x > max.x) {
				max.x = v.x;
			}

			if (v.y < min.y) {
				min.y = v.y;
			}
			else if (v.y > max.y) {
				max.y = v.y;
			}

			if (v.z < min.z) {
				min.z = v.z;
			}
			else if (v.z > max.z) {
				max.z = v.z;
			}
		}
		center.x = (min.x + max.x) / 2;
		center.y = (min.y + max.y) / 2;
		center.z = (min.z + max.z) / 2;
		return center;
	}
};

#endif // __GWU_MODEL__