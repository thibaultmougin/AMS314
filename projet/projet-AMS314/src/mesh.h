/* 


  Mesh Structures that will be used in the project 
  
  
  Provided functions : 
       
  

*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <libmesh6.h>
#include <lplib3.h>
#include <math.h>



typedef double double3[3];
typedef double double2[2];
typedef int    int5[5];
typedef int    int4[4]; 
typedef int    int3[3];
typedef int    int1[1];
typedef unsigned long long int u64;


/* 
  
  A simple mesh structure to hold the geomety/connectivity 
  Note that 2D mesh are stored as 3D Mesh, z-coordinate will be set to zero

  Provided function msh_init, msh_read, msh_write, msh_check
  The mesh files are based on the meshb format (https://github.com/LoicMarechal/libMeshb), can be viewed with ViZiR4, see README to install it. 

  The following functions has to be implemented  : 
    msh_bondingbox  : compute the bounding box of mesh
    msh_reorder     : reorder an input mesh according to Z-ordering 
    msh_reorder     : simple smoothing algorithm of volume points   
    msh_neighbors   : build the set of Tris surrounding elements, efficient implementation with Hash Tab
    msh_neighborsQ2 : a quadratic setting of the neigbhoring structures 

 

*/

typedef struct mesh_vertex
{  
  double Crd[2];
  
  long long int icrit; /* sorting creteria, to be used with qsort  */
  int idxNew;          /* new if after sort */
  int idxOld;          /* initial id  */ 
  
} Vertex; 


typedef struct mesh_edge
{
  int Ver[2]; /* index of the two vertices  */
  int Voi[2]; /* neigbhoring structures */
  int Ref;
  
  long long int icrit; /* sorting creteria, to be used with qsort  */
  
} Edge;


typedef struct mesh_triangle
{
  int Ver[3]; /* index of the three vertices  */
  int Voi[3]; /* neigbhoring structures */
  int Ref;
  
  long long int icrit; /* sorting creteria, to be used with qsort  */
  
} Triangle;


typedef struct mesh_hash_table
{
  int  SizHead;   /* Maxmimum entries, the key is in [0,SizHead-1]*/
  int  NbrObj;    /* Number of object in the hash tables */
  int  NbrMaxObj; /* Maximum of object that can be store in the hash tab */ 
  int  *Head ;    /* Head[key%(SizHead)] = link to the first object having this key  in the LstObj list */
  int5 *LstObj;   /* List of objects in the Hash Tab */
  
  /* LstObj[id][0:1] = ip1-ip2,     the 2 points defining the edge  */
  /* LstObj[id][2:3] = iTri1,iTri2, the two neighboring triangles having ip1-ip2 as points */
  /* LstObj[id][4]   = idnxt,       the link to the next element in collision, if = 0 last element of the list */
  
} HashTable;


typedef struct t_mesh
{
  int Dim;
  int NbrVer, NbrTri, NbrEfr, NbrEdg;
  
  Vertex      *Ver;   /* list of vertices */
  Triangle    *Tri;   /* list of triangles */
  Edge        *Efr;   /* list of boundary edges */
  Edge        *Edg;   /* list of all the mesh egdes */
  HashTable   *Hsh;
  /* bounding box */
  double bb[4];
  
} Mesh; 



/* Provided functions */
Mesh * msh_init();
Mesh * msh_read(char *file, int readEfr);
int    msh_write(Mesh *msh, char *file); 

double * sol_read(char *file, int mshDim, int mshNbrSol);



/* functions to be implemented  */
int    msh_boundingbox(Mesh *msh);         /* compute the bouding box of the mesh                         */
int    msh_reorder(Mesh *msh);             /* perform a mesh using morton curve/octree-based              */
int    msh_smooth(Mesh *msh, int nbrStep); /* a simple mesh smoohting algorithm                           */
int    msh_neighbors(Mesh *msh);           /* build TriVois with a hash table                             */
int    msh_neighborsQ2(Mesh *msh);         /* build TriVois with the naive quadratic approach             */
int nb_edges_boundary(Mesh *msh);


double quality_area(Mesh *msh, int iTri);
double quality_rho(Mesh *msh, int iTri);

/* a provided simple hash table data structure */

/* Implementing the following function should be necessary */
HashTable * hash_init(int SizHead, int NbrMaxObj);          /* alloc and set htable ==> allocate Head, LstObj */

int hash_find(HashTable *hsh, int ip1, int ip2);            /* return the id found (in LstObj ), if 0 the object is not in the list */
int hash_add (HashTable *hsh, int ip1, int ip2, int iTri);  /* ==> add this entry in the hash tab */



/* Fonction used for adaptation */


/* Writes a 2d metric tensor field in a file (extension should be .sol)*/
int msh_write2dmetric(char *file, int nmetric, double3 *metric);  
int msh_write2dfield_Triangles(char *file, int nfield, double *field);
int msh_write2dfield_Vertices(char *file, int nfield, double *field);
