#include "polygon.h"
#include <windows.h>
#include "../types/vec3f.h"


vec3f polygon::collideSAT(polygon* _poly)
{
	if(!_poly)return false;
	std::vector<vec3f>* verts[] = {&m_vertices, &_poly->m_vertices};
	vec3f MTV;
	float  MinOverlap = -99999;
	//this tests all of this poly's axes against the incoming poly and vice versa.
	for(unsigned int i = 0; i < 2; i++)
	{
		for(unsigned int ii = 0; ii < (*verts[i]).size(); ii++)
		{
			///Create an edge direction vector. Then Find it's normal.
			vec3f edgeDir =  ((*verts[i])[(ii+1)%((*verts[i]).size())]) - ((*verts[i])[ii]);
			edgeDir.NormaliseSelf(); //This might not be nessisary
			vec3f normal = vec3f(-edgeDir.j, edgeDir.i); //This technically isn't a normal. It's just a perp line.

			//Find the projected shape's ranges on the normal.
			float max[2], min[2], diff[2];
			for(unsigned int iii = 0; iii < 2; iii++)
			{
				min[iii] = max[iii] = ((*verts[(i+iii)%2])[0].dot2(normal)); //2d dot product
				for(unsigned int iv = 0; iv < (*verts[(i+iii)%2]).size(); iv++)
				{
					diff[iii] = ((*verts[(i+iii)%2])[iv].dot2(normal));//2d dot product
					if (diff[iii] < min[iii]) min[iii] = diff[iii];
					else if(diff[iii] > max[iii]) max[iii] = diff[iii];
				}
			}
			float d0 = min[0] - max[1]; //overlap 1
			float d1 = min[1] - max[0]; //overlap 2
			if(d0 > 0.0f || d1 > 0.0f) return false;
			else if(d0 > MinOverlap || d1 > MinOverlap)
			{
				MinOverlap = (d0>d1?d0:-d1);
				MTV = normal*MinOverlap;
			}
		}
	}
	char hi[64];
	sprintf(hi, "MTV = (%f,%f,%f)\n", MTV.i, MTV.j, MTV.k);
	OutputDebugString(hi);
	return MTV;
}

/*
vec3f polygon::collideSAT(polygon* _poly)
{
	if(!_poly)return false;
	std::vector<vec3f> verts[] = {m_vertices, _poly->m_vertices};
	vec3f MTV;
	float  MinOverlap = -99999;
	//this tests all of this poly's axes against the incoming poly and vice versa.
	for(unsigned int i = 0; i < 2; i++)
	{
		for(unsigned int ii = 0; ii < verts[i].size(); ii++)
		{
			///Create an edge direction vector. Then Find it's normal.
			vec3f edgeDir =  (verts[i][(ii+1)%(m_vertices.size())]) - (verts[i][ii]);
			edgeDir.NormaliseSelf(); //This might not be nessisary
			vec3f normal = vec3f(-edgeDir.j, edgeDir.i); //This technically isn't a normal. It's just a perp line.

			//Find the projected shape's ranges on the normal.
			float max[2], min[2], diff[2];
			for(unsigned int iii = 0; iii < 2; iii++)
			{
				min[iii] = max[iii] = (verts[(i+iii)%2][0].dot2(normal)); //2d dot product
				for(unsigned int iv = 0; iv < verts[(i+iii)%2].size(); iv++)
				{
					diff[iii] = (verts[(i+iii)%2][iv].dot2(normal));//2d dot product
					if (diff[iii] < min[iii]) min[iii] = diff[iii];
					else if(diff[iii] > max[iii]) max[iii] = diff[iii];
				}
			}
			float d0 = min[0] - max[1]; //overlap 1
			float d1 = min[1] - max[0]; //overlap 2
			if(d0 > 0.0f || d1 > 0.0f) return false;
			else if(d0 > MinOverlap || d1 > MinOverlap)
			{
				MinOverlap = (d0>d1?d0:-d1);
				MTV = normal*MinOverlap;
			}
		}
	}
	char hi[64];
	sprintf(hi, "MTV = (%f,%f,%f)\n", MTV.i, MTV.j, MTV.k);
	OutputDebugString(hi);
	return MTV;
}*/












//*/


/*///code optimisation one.
bool polygon::collideSAT(polygon* _poly)
{
	if(!_poly)return false;

	std::vector<vec3f> verts[] = {m_vertices, _poly->m_vertices};
	//this tests all of this poly's axes against the incoming poly.
	for(unsigned int i = 0; i < 2; i++) //testing the two poly's axes
	{
		for(unsigned int ii = 0; ii < verts[i].size(); ii++)
		{
			////////////////////////////////////////////////////////////////////////////////////
			///first section = fine.
			vec3f edgeDir =  (verts[i][(ii+1)%(m_vertices.size())]) - (verts[i][ii]); //this works out the direction.
			//edgeDir.NormaliseSelf();
			vec3f normal = vec3f(-edgeDir.j, edgeDir.i); //test this axis.
			////////////////////////////////////////////////////////////////////////////////////
			//trying to find the range on the seperated axis.
			float max0, min0, diff0;
			min0 = max0 = (verts[i][0].dot2(normal)); //2d dot product
			for(unsigned int iii = 0; iii < verts[i].size(); iii++)
			{
				diff0 = (verts[i][iii].dot2(normal));//2d dot product
				if (diff0 < min0) min0 = diff0;
				else if(diff0 > max0) max0 = diff0;
			}
			//trying to find the range on the seperated axis.
			float max1, min1, diff1;
			max1 = min1 = (verts[(i+1)%2][0].dot2(normal));
			for(unsigned int iii = 0; iii < verts[(i+1)%2].size(); iii++)
			{
				diff1 = (verts[(i+1)%2][iii].dot2(normal));//2d dot product
				if (diff1 < min1) min1 = diff1;
				else if(diff1 > max1) max1 = diff1;
			}
			float d0 = min0 - max1;
			float d1 = min1 - max0;
			if(d0 > 0.0f || d1 > 0.0f) return false;
		}
	}
	return true;
}//*/

//no code optimisations
/*
bool polygon::collideSAT(polygon* _poly)
{
	if(!_poly)return false;
	//this tests all of this poly's axes against the incoming poly.
	for(unsigned int i = 0; i < m_vertices.size(); i++)
	{
		////////////////////////////////////////////////////////////////////////////////////
		///first section = fine.
		vec3f edgeDir =  (m_vertices[(i+1)%(m_vertices.size())]) - (m_vertices[i]); //this works out the direction.
		//edgeDir.NormaliseSelf();
		vec3f normal = vec3f(-edgeDir.j, edgeDir.i); //test this axis.
		////////////////////////////////////////////////////////////////////////////////////
		//trying to find the range on the seperated axis.
		float max0, min0, diff0;
		min0 = max0 = (m_vertices[0].dot2(normal)); //2d dot product
		for(unsigned int ii = 0; ii < numEdges(); ii++)
		{
			diff0 = (m_vertices[ii].dot2(normal));//2d dot product
			if (diff0 < min0) min0 = diff0;
			else if(diff0 > max0) max0 = diff0;
		}
		//trying to find the range on the seperated axis.
		float max1, min1, diff1;
		max1 = min1 = (_poly->m_vertices[0].dot2(normal));
		for(unsigned int ii = 0; ii < _poly->numEdges(); ii++)
		{
			diff1 = (_poly->m_vertices[ii].dot2(normal));//2d dot product
			if (diff1 < min1) min1 = diff1;
			else if(diff1 > max1) max1 = diff1;
		}
		float d0 = min0 - max1;
		float d1 = min1 - max0;
		if(d0 > 0.0f || d1 > 0.0f) return false;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(int i = 0; i < _poly->numEdges(); i++)
	{
		vec3f edgeDir = (_poly->m_vertices[(i+1)%_poly->numEdges()] - (_poly->m_vertices[i])); //this works out the direction.
		//edgeDir.NormaliseSelf();
		vec3f normal = vec3f(-edgeDir.j, edgeDir.i); //NOT SURE, THIS DOESN'T LOOK LIKE A NORMAL. test this axis.
		
		//trying to find the range on the seperated axis.
		float max0, min0, diff1;
		max0 = min0 = (_poly->m_vertices[0].dot2(normal)); //2d dot product
		for(int ii = 0; ii < _poly->numEdges(); ii++)
		{
			diff1 = (_poly->m_vertices[ii].dot2(normal));//2d dot product
			if (diff1 < min0) min0 = diff1;
			else if(diff1 > max0) max0 = diff1;
		}
		//trying to find the range on the seperated axis.
		float max1, min1, hisDiff;
		max1 = min1 = (m_vertices[0].dot2(normal));
		for(int ii = 0; ii < numEdges(); ii++)
		{
			hisDiff = (m_vertices[ii].dot2(normal));//2d dot product
			if (hisDiff < min1) min1 = hisDiff;
			else if(hisDiff > max1) max1 = hisDiff;
		}
		float d0 = min0 - max1;
		float d1 = min1 - max0;
		if(d0 > 0.0f || d1 > 0.0f) return false;
	}

	return true;
}//*/
