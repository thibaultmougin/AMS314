#include <mesh.h>


int main(int argc, char *argv[])
{ 
  double to,ti;
  
  if ( argc < 2 ) {
    printf(" usage : mesh file \n");
    return 0;
  }
  
  /* read a mesh */
  to =  GetWallClock();
  Mesh * msh = msh_read(argv[1], 0);
  ti =  GetWallClock();
  
  if ( ! msh ) return 0;
  
  printf("  Vertices   %10d \n", msh->NbrVer);
  printf("  Triangles  %10d \n", msh->NbrTri);
  printf("  time to read the mesh %lg (s) \n",ti-to);
  
  /* re-order a mesh */ 
  to =  GetWallClock();
  msh_reorder(msh);
  ti =  GetWallClock();  
  printf("  time to re-order the mesh  %lg (s) \n",ti-to);
   
  /* create neigbhors Q2 version */
  to =  GetWallClock();
  msh_neighborsQ2(msh);
  ti =  GetWallClock();
  printf("  time q2 neigh.        %lg (s) \n",ti-to);
  
  /* create neigbhors with hash table */
  to =  GetWallClock();
  msh_neighbors(msh);
  ti =  GetWallClock();
  printf("  time hash tab neigh.  %lg (s) \n",ti-to);
  
  /* write reordered mesh */
  to =  GetWallClock();
  msh_write(msh,"output.meshb");
  ti =  GetWallClock();
  
  int      iTri, iVer;
  double  *Qua = (double  *)malloc(sizeof(double ) * (msh->NbrTri+1));
  double3 *Met = (double3 *)malloc(sizeof(double3) * (msh->NbrVer+1));
  
  
  for (iTri=1; iTri<=msh->NbrTri; iTri++) {
  	 Qua[iTri] = quality_area(msh,iTri);
     //printf("%f",Qua[iTri]);
  } 
  
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
  	 Met[iVer][0] = 1.;
  	 Met[iVer][1] = 0.;
  	 Met[iVer][2] = 1.;
  } 
  
  msh_write2dfield_Triangles("quality.solb", msh->NbrTri, Qua);
  printf("OK quality");
  msh_write2dmetric("metric.solb", msh->NbrVer, Met);
  	
  	
  return 0;
}





