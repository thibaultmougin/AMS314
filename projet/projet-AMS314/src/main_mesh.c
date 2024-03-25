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
  //msh_neighborsQ2(msh);
  ti =  GetWallClock();
  printf("  time q2 neigh.        %lg (s) \n",ti-to);
  /* create neigbhors with hash table */
  to =  GetWallClock();

  int nb_aretes = msh_neighbors(msh);
  ti =  GetWallClock();
  printf("  time hash tab neigh.  %lg (s) \n",ti-to);

  Vertex P;
  P.Crd[0]=0.4;
  P.Crd[1]=0.4;

  printf("  nb d'arêtes : %d \n",nb_aretes);
  printf("  nb d'arêtes frontières : %d \n",nb_edges_boundary(msh));
  fflush(stdout);

  int nbr_col= nb_collisions(msh->Hsh);
  printf("  nb collisions : %d \n", nbr_col);
  printf("  nb collisions/aretes : %f \n", (double)nbr_col/(double)nb_aretes);

  insert_simple(msh,P);

  Vertex P2;

  P2.Crd[0]=0.6;
  P2.Crd[1]=0.4;


  insert_simple(msh,P2);


  P2.Crd[0]=0.7;
  P2.Crd[1]=0.7;

  insert_simple(msh,P2);

  
  for (int k=0;k<=msh->NbrTri;k++){
    printf("%d : %d, %d, %d, Voisins : %d, %d, %d \n",k,msh->Tri[k].Ver[0],msh->Tri[k].Ver[1],msh->Tri[k].Ver[2],msh->Tri[k].Voi[0],msh->Tri[k].Voi[1],msh->Tri[k].Voi[2]);
  }



  /* write reordered mesh */
  to =  GetWallClock();
  msh_write(msh,"output.meshb");
  ti =  GetWallClock();
  
  int      iTri, iVer;
  double  *Qua = (double  *)malloc(sizeof(double ) * (msh->NbrTri+1));
  double3 *Met = (double3 *)malloc(sizeof(double3) * (msh->NbrVer+1));
  

  for (iTri=1; iTri<=msh->NbrTri; iTri++) {
  	 Qua[iTri] = quality_rho(msh,iTri);
     //printf("%f",Qua[iTri]);
  } 
  
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
  	 Met[iVer][0] = 1.;
  	 Met[iVer][1] = 0.;
  	 Met[iVer][2] = 1.;
  } 

  msh_write2dfield_Triangles("quality.solb", msh->NbrTri, Qua);
  msh_write2dmetric("metric.solb", msh->NbrVer, Met);
  	
  	
  return 0;
}



/*

make: 'mesh' is up to date.
 File data/carre_4h.mesh opened Dimension 2 Version 1 
  Vertices         2625 
  Triangles        5084 
  time to read the mesh 0.00566983 (s) 
  time to re-order the mesh  3.69549e-05 (s) 
  time q2 neigh.        0.280322 (s) 
  time hash tab neigh.  0.000411987 (s) 
 File data/carre_2h.mesh opened Dimension 2 Version 2 
  Vertices        10663 
  Triangles       20938 
  time to read the mesh 0.021153 (s) 
  time to re-order the mesh  0.000194073 (s) 
  time q2 neigh.        4.78475 (s) 
  time hash tab neigh.  0.00282598 (s) 
 File data/carre_h.mesh opened Dimension 2 Version 2 
  Vertices        43758 
  Triangles       86742 
  time to read the mesh 0.0832739 (s) 
  time to re-order the mesh  0.000831842 (s) 
  time q2 neigh.        80.4208 (s) 
  time hash tab neigh.  0.00966191 (s) 
 File data/carre_05h.mesh opened Dimension 2 Version 2 
  Vertices       178746 
  Triangles      355946 
  time to read the mesh 0.29301 (s) 
  time to re-order the mesh  0.00235009 (s) 
  time q2 neigh.        1746.55 (s) 
  time hash tab neigh.  0.0568562 (s) 
*/




