#include <mesh.h>


int tri2edg[3][2] = {{1,2},{2,0},{0,1}};


Mesh * msh_init()
{
  Mesh *msh = malloc(sizeof(Mesh));
  if ( ! msh ) return NULL;
  
  msh->Dim    = 0;
  msh->NbrVer = 0;
  msh->NbrTri = 0;
  msh->NbrEfr = 0;
  msh->NbrEdg = 0;
  
  msh->Ver = NULL;
  msh->Tri = NULL;  
  msh->Efr = NULL;  
  msh->Edg = NULL;  
  
  msh->bb[0] = 0.0; /* xmin  */
  msh->bb[1] = 0.0; /* xmax  */
  msh->bb[2] = 0.0; /* ymin  */
  msh->bb[3] = 0.0; /* ymax  */
  
  return msh;
}  
  


Mesh * msh_read(char *file, int readEfr)
{
  char   InpFil[1024];
  float  bufFlt[2];
  double bufDbl[2];
  int    i, bufTri[4], bufEfr[3];
  int    FilVer, ref; 
  
  int fmsh = 0;
  
  if ( ! file ) return NULL;
  
  Mesh * msh = msh_init();
    
  //--- set file name 
  strcpy(InpFil,file);
  if ( strstr(InpFil,".mesh") ) {
    if ( !(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)) ) {
      return NULL;
    }    
  }
  else {
    strcat(InpFil,".meshb");
    if ( !(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)) ) {
      strcpy(InpFil,file);
      strcat(InpFil,".mesh");
      if ( !(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)) ) {
        return NULL;
      }    
    } 
  }
  
  printf(" File %s opened Dimension %d Version %d \n", InpFil, msh->Dim, FilVer);
  
  msh->NbrVer = GmfStatKwd(fmsh, GmfVertices);
  msh->NbrTri = GmfStatKwd(fmsh, GmfTriangles);
  
  //--- allocate arrays 
  msh->Ver = calloc( (msh->NbrVer+1), sizeof(Vertex)   );
  msh->Tri = calloc( (msh->NbrTri+1), sizeof(Triangle) );  
  
  
  //--- read vertices   
  GmfGotoKwd(fmsh, GmfVertices);
  if ( msh->Dim == 2 ) {
    if ( FilVer == GmfFloat ) {		// read 32 bits float
      for (i=1; i<=msh->NbrVer; ++i) {
        GmfGetLin(fmsh, GmfVertices, &bufFlt[0], &bufFlt[1], &ref);
        msh->Ver[i].Crd[0] = (double)bufFlt[0];
        msh->Ver[i].Crd[1] = (double)bufFlt[1];
      }
    }
    else  {	// read 64 bits float
      for (i=1; i<=msh->NbrVer; ++i) {
        GmfGetLin(fmsh, GmfVertices, &bufDbl[0], &bufDbl[1], &ref);
        msh->Ver[i].Crd[0] = bufDbl[0];
        msh->Ver[i].Crd[1] = bufDbl[1];
      }  
    }
  }
  else {
    fprintf(stderr,"  ## ERROR: 3D is not implemented\n");
    exit(1);
  }
  
  
  //--- read triangles   
  GmfGotoKwd(fmsh, GmfTriangles);
  for (i=1; i<=msh->NbrTri; ++i) {
    GmfGetLin(fmsh, GmfTriangles, &bufTri[0], &bufTri[1], &bufTri[2], &bufTri[3]);
    msh->Tri[i].Ver[0] = bufTri[0];
    msh->Tri[i].Ver[1] = bufTri[1];
    msh->Tri[i].Ver[2] = bufTri[2];
    msh->Tri[i].Ref    = bufTri[3];
  }
  
  
  //--- read boundary edges
  if ( readEfr == 1 ) {
    msh->NbrEfr = GmfStatKwd(fmsh, GmfEdges);
    msh->Efr    = calloc( (msh->NbrEfr+1), sizeof(Edge) );  

    GmfGotoKwd(fmsh, GmfEdges);
    for (i=1; i<=msh->NbrEfr; ++i) {
      GmfGetLin(fmsh, GmfEdges, &bufEfr[0], &bufEfr[1], &bufEfr[2]);
      msh->Efr[i].Ver[0] = bufEfr[0];
      msh->Efr[i].Ver[1] = bufEfr[1];
      msh->Efr[i].Ref    = bufEfr[2];
    }
  }
  
  
  
  GmfCloseMesh(fmsh);
  
  return msh;
  
}



double * sol_read(char *file, int mshDim, int mshNbrSol)
{
  char   InpFil[1024];
  int    FilVer, SolTyp, NbrTyp, SolSiz, TypTab[ GmfMaxTyp ]; 
  float  bufFlt;
  double bufDbl;
  int    i, dim, nbrSol;
  
  int fsol = 0;
  
  if ( ! file ) return NULL;
  
  double * sol = NULL;
    
    
  //--- set file name 
  strcpy(InpFil, file);
  if ( strstr(InpFil,".sol") ) {
    if ( !(fsol = GmfOpenMesh(InpFil,GmfRead,&FilVer,&dim)) ) {
      return NULL;
    }    
  }
  else {
    strcat(InpFil,".solb");
    if ( !(fsol = GmfOpenMesh(InpFil,GmfRead,&FilVer,&dim)) ) {
      strcpy(InpFil,file);
      strcat(InpFil,".sol");
      if ( !(fsol = GmfOpenMesh(InpFil,GmfRead,&FilVer,&dim)) ) {
        return NULL;
      }    
    } 
  }
  
  printf(" File %s opened Dimension %d Version %d \n", InpFil, dim, FilVer);
  
  SolTyp = GmfSolAtVertices;		// read only sol at vertices
  nbrSol = GmfStatKwd(fsol, SolTyp, &NbrTyp, &SolSiz, TypTab);
	
	
  if ( nbrSol == 0 ) {
    printf("  ## WARNING: No SolAtVertices in the solution file !\n");
    return NULL;
  }
  if ( dim != mshDim ) {
    printf("  ## WARNING: WRONG DIMENSION NUMBER. IGNORED\n");
    return NULL;
  }
  if ( nbrSol != mshNbrSol ) {
    printf("  ## WARNING: WRONG SOLUTION NUMBER. IGNORED\n");
    return NULL;
  }
  if (  NbrTyp != 1 ) {
    printf("  ## WARNING: WRONG FIELD NUMBER. IGNORED\n");
    return NULL;
  }
  if ( TypTab[0] != GmfSca ) {
    printf("  ## WARNING: WRONG FIELD TYPE. IGNORED\n");
    return NULL;
  }
	
	sol = (double *)calloc(nbrSol+1, sizeof(double));
	
	
  GmfGotoKwd(fsol, SolTyp);

  for (i=1; i<=nbrSol; ++i) {
		if ( FilVer == GmfFloat ) {
	    GmfGetLin(fsol, SolTyp, &bufFlt);
	    sol[i] = (double)bufFlt;
		}
		else {
	    GmfGetLin(fsol, SolTyp, &bufDbl);
	    sol[i] = bufDbl;
		}
  }
	
  if ( !GmfCloseMesh(fsol) ) {
    fprintf(stderr, "  ## ERROR: Cannot close solution file %s ! \n", InpFil);
    //myexit(1);
  }
	
  return sol;	
}




int compar_vertex(const void *a, const void *b)
{
  Vertex *va = (Vertex *) a;
  Vertex *vb = (Vertex *) b;
  return ( vb->icrit - va->icrit );
}

int compar_triangle(const void *a, const void *b)
{
  Triangle *va = (Triangle *) a;
  Triangle *vb = (Triangle *) b;
  return ( vb->icrit - va->icrit );
}

double quality_area(Mesh *msh, int iTri){
  
  Vertex verts[3];

  verts[0]= msh->Ver[msh->Tri[iTri].Ver[0]];
  verts[1]= msh->Ver[msh->Tri[iTri].Ver[1]];
  verts[2]= msh->Ver[msh->Tri[iTri].Ver[2]];

  double alpha_1= 1./(4*sqrt(3));

  double res = 0;

  double x_1 = verts[0].Crd[0], y_1= verts[0].Crd[1];
  double x_2 = verts[1].Crd[0], y_2= verts[1].Crd[1];
  double x_3 = verts[2].Crd[0], y_3= verts[2].Crd[1];

  res += (x_2-x_1)*(x_2-x_1) + (y_2-y_1)*(y_2-y_1);
  res += (x_3-x_1)*(x_3-x_1) + (y_3-y_1)*(y_3-y_1);
  res += (x_3-x_2)*(x_3-x_2) + (y_3-y_2)*(y_3-y_2);

  double area =(x_1*(y_2-y_3)+x_2*(y_3-y_1)+x_3*(y_1-y_2))/2;

  area = (area >= 0) ? area : -area;

  return res *alpha_1/area;
}


double quality_rho(Mesh *msh, int iTri){
  
  Vertex verts[3];

  verts[0]= msh->Ver[msh->Tri[iTri].Ver[0]];
  verts[1]= msh->Ver[msh->Tri[iTri].Ver[1]];
  verts[2]= msh->Ver[msh->Tri[iTri].Ver[2]];

  double alpha_2= 1./3.4661;

  double x_1 = verts[0].Crd[0], y_1= verts[0].Crd[1];
  double x_2 = verts[1].Crd[0], y_2= verts[1].Crd[1];
  double x_3 = verts[2].Crd[0], y_3= verts[2].Crd[1];

  double l_1= sqrt((x_2-x_1)*(x_2-x_1) + (y_2-y_1)*(y_2-y_1));
  double l_2= sqrt((x_3-x_1)*(x_3-x_1) + (y_3-y_1)*(y_3-y_1));
  double l_3= sqrt((x_3-x_2)*(x_3-x_2) + (y_3-y_2)*(y_3-y_2));

  double res = l_2>=l_1 ? l_2 : l_1;

  res = l_3>=res ? l_3 : res;

  double area =(x_1*(y_2-y_3)+x_2*(y_3-y_1)+x_3*(y_1-y_2))/2;

  area = (area >= 0) ? area : -area;

  double radius = 2*area/(l_1+l_2+l_3);

  return res *alpha_2/radius;
}

int msh_reorder(Mesh *msh)
{
  int iVer;  // iTri, 
  
  if ( ! msh            ) return 0;
  if ( msh->NbrVer <= 0 ) return 0;
  
  /* compute bonding box */
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
    /* todo msh->bb : used to compute the Z-curve index */
    
  }
  
  for (iVer=1; iVer<=msh->NbrVer; iVer++) {
    msh->Ver[iVer].icrit  = rand();   /* change the randon  by Z  order */
    msh->Ver[iVer].idxNew = iVer;
    msh->Ver[iVer].idxOld = iVer;
  }
  //qsort(&msh->Ver[1], msh->NbrVer, sizeof(Vertex), compar_vertex); 

  
  /* update idxNew for vertices */
   
  
  /* re-assign triangles ids */
  
  
  /* sort triangles */
  //for (iTri=1; iTri<=msh->NbrTri; iTri++) {
  //  msh->Tri[iTri].icrit = rand();   /* change the randon  by an improved order */  
  //}
  //qsort(&msh->Tri[1], msh->NbrTri, sizeof(Triangle), compar_triangle);
  
  
  return 1;
}




int msh_write(Mesh *msh, char *file)
{
  int iVer, iTri;
  int FilVer = 2;
  
  if ( ! msh  ) return 0;
  if ( ! file ) return 0;
  
  int fmsh = GmfOpenMesh(file, GmfWrite, FilVer, msh->Dim);
  if ( fmsh <=  0 ) {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }
  
  GmfSetKwd(fmsh, GmfVertices, msh->NbrVer);
  for (iVer=1; iVer<=msh->NbrVer; iVer++) 
    GmfSetLin(fmsh, GmfVertices, msh->Ver[iVer].Crd[0], msh->Ver[iVer].Crd[1], 0); 
  
  GmfSetKwd(fmsh, GmfTriangles, msh->NbrTri);
  for (iTri=1; iTri<=msh->NbrTri; iTri++)  
    GmfSetLin(fmsh, GmfTriangles, msh->Tri[iTri].Ver[0], msh->Tri[iTri].Ver[1], msh->Tri[iTri].Ver[2], msh->Tri[iTri].Ref);  
     
  GmfCloseMesh(fmsh);
  
  return 1;
}





int msh_neighborsQ2(Mesh *msh)
{
  int iTri, iEdg, jTri, jEdg, ip1, ip2, jp1, jp2;
  
  if ( ! msh ) return 0;
  
  for (iTri=1; iTri<=msh->NbrTri; iTri++) {
    for (iEdg=0; iEdg<3; iEdg++) {
      ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];
			
      /* find the Tri different from iTri that has ip1, ip2 as vertices */
      for (jTri=1; jTri<=msh->NbrTri; jTri++) {
        if ( iTri == jTri ) 
          continue;
        
        for (jEdg=0; jEdg<3; jEdg++) {
          jp1 = msh->Tri[jTri].Ver[tri2edg[jEdg][0]];
          jp2 = msh->Tri[jTri].Ver[tri2edg[jEdg][1]];
          
          /* compare the 4 points */
          if (((jp1==ip1) && (jp2==ip2))||((jp1==ip2) && (jp2==ip1))){
            (msh->Tri[iTri]).Voi[iEdg]=jTri;
            (msh->Tri[jTri]).Voi[jEdg]=iTri;
            break;
          }
        }
      }
      
    }
  }
  
  return 1;
}   



int msh_neighbors(Mesh *msh)
{
  int iTri, iEdg, ip1, ip2, found;
  
  if ( ! msh ) return 0;

  HashTable* hsh_tab = hash_init(2*msh->NbrVer,3*msh->NbrTri);

  for (iTri=1; iTri<=msh->NbrTri; iTri++) {
    for (iEdg=0; iEdg<3; iEdg++) {
      ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];

      found = hash_find(hsh_tab, ip1,ip2);

      if (found==0){

        hash_add(hsh_tab,ip1,ip2,iTri);
      }
      else{

        int Tri1 = hsh_tab->LstObj[found][2];
        hsh_tab->LstObj[found][3]=iTri;
        int Tri2 = hsh_tab->LstObj[found][3];

        msh->Tri[Tri1].Voi[iEdg]=Tri2;
        msh->Tri[Tri2].Voi[iEdg]=Tri1;
      }
      /*printf("%d, %d, %d, %d\n",iTri,ip1,ip2,msh->Tri[iTri].Voi[iEdg]);
      fflush(stdout);*/
    }
  }
/*
  for (int i =0;i<hsh_tab->NbrObj;i++){
    printf("%d, %d, %d, %d, %d \n", hsh_tab->LstObj[i][0],hsh_tab->LstObj[i][1],hsh_tab->LstObj[i][2],hsh_tab->LstObj[i][3],hsh_tab->LstObj[i][4]);
    fflush(stdout);
  }*/

  msh->Hsh=hsh_tab;
  
  return hsh_tab->NbrObj;

}   


int nb_edges_boundary(Mesh *msh)
{
  int n =0;
  for (int i=0;i<msh->Hsh->NbrObj;i++){
    if (msh->Hsh->LstObj[i][3]==0){
      n++;
    }
  }
  
  return n;

}   




HashTable * hash_init(int SizHead, int NbrMaxObj)
{
	HashTable *hsh = malloc(sizeof(HashTable));
	
  hsh->Head = calloc(SizHead , sizeof(int));
  hsh->LstObj = calloc(NbrMaxObj,sizeof(int5));
  hsh->NbrObj=0;
  hsh->NbrMaxObj=NbrMaxObj;
  hsh->SizHead=SizHead;

  return hsh;
}


int hash_find(HashTable *hsh, int ip1, int ip2)
{
  
  int current = hsh->Head[(ip1+ip2)%(hsh->SizHead)];

  while(1){
    
    if(((hsh->LstObj[current][0]==ip1)&&(hsh->LstObj[current][1]==ip2))||((hsh->LstObj[current][0]==ip2)&&(hsh->LstObj[current][1]==ip1)))
      return current;

    if (hsh->LstObj[current][4]==0)
      return 0;

    current = hsh->LstObj[current][4];

  }
	
	return 0;
}


int hash_add(HashTable *hsh, int ip1, int ip2, int iTri)
{

  int iObj = hsh->NbrObj;

  hsh->NbrObj++;

  int current = hsh->Head[(ip1+ip2)%(hsh->SizHead)];
  
  if(current==0){
    hsh->Head[(ip1+ip2)%(hsh->SizHead)]=iObj;
    hsh->LstObj[iObj][0]=ip1;
    hsh->LstObj[iObj][1]=ip2;
    hsh->LstObj[iObj][2]=iTri;
    hsh->LstObj[iObj][3]=0; 
    hsh->LstObj[iObj][4]=0;
    return 0;
  }


  while(1){

    if (hsh->LstObj[current][4]==0){
      hsh->LstObj[current][4]=iObj;
      hsh->LstObj[iObj][0]=ip1;
      hsh->LstObj[iObj][1]=ip2;
      hsh->LstObj[iObj][2]=iTri;
      hsh->LstObj[iObj][3]=0; 
      hsh->LstObj[iObj][4]=0;
      return 0;
    }
    if((hsh->LstObj[current][0]==ip2)&&(hsh->LstObj[current][1]==ip1)){
      hsh->LstObj[current][3]=iTri; 
      return 0;
    }
    current = hsh->LstObj[current][4];
  }

	
	return 0;
}




int msh_write2dfield_Vertices(char *file, int nfield, double *field) 
{
  int iVer;
  
  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if ( fmsh <=  0 ) {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }
  
  int sizfld[1];
  sizfld[0] = GmfSca;
  
  GmfSetKwd(fmsh, GmfSolAtVertices, nfield, 1, sizfld);
  
  for (iVer=1; iVer<=nfield; iVer++) 
    GmfSetLin(fmsh, GmfSolAtVertices, &field[iVer]); 
  
  GmfCloseMesh(fmsh);
  
  return 1;
}



int msh_write2dfield_Triangles(char *file, int nfield, double *field) 
{
  int iTri;
  
  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if ( fmsh <=  0 ) {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }
  
  int sizfld[1];
  sizfld[0] = GmfSca;
  
  GmfSetKwd(fmsh, GmfSolAtTriangles, nfield, 1, sizfld);
  
  for (iTri=1; iTri<=nfield; iTri++) 
    GmfSetLin(fmsh, GmfSolAtTriangles, &field[iTri]); 
  
  GmfCloseMesh(fmsh);
  
  return 1;
}



int msh_write2dmetric(char *file, int nmetric, double3 *metric) 
{  
  int iVer;
  
  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if ( fmsh <=  0 ) {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }
  
  int sizfld[1];
  sizfld[0] = GmfSymMat;
  
  GmfSetKwd(fmsh, GmfSolAtVertices, nmetric, 1, sizfld);
  
  for (iVer=1; iVer<=nmetric; iVer++) 
    GmfSetLin(fmsh, GmfSolAtVertices, &metric[iVer][0], &metric[iVer][1], &metric[iVer][2]); 
  
  GmfCloseMesh(fmsh);
  
  return 1;
}




