#include <mesh.h>
#include <stdlib.h>
#include <time.h>

int tri2edg[3][2] = {{1, 2}, {2, 0}, {0, 1}};

Mesh *msh_init()
{
  Mesh *msh = malloc(sizeof(Mesh));
  if (!msh)
    return NULL;

  msh->Dim = 0;
  msh->NbrVer = 0;
  msh->NbrTri = 0;
  msh->NbrEfr = 0;
  msh->NbrEdg = 0;

  msh->Ver = NULL;
  msh->Tri = NULL;
  msh->Efr = NULL;
  msh->Edg = NULL;
  msh->Hsh = NULL;

  msh->bb[0] = 0.0; /* xmin  */
  msh->bb[1] = 0.0; /* xmax  */
  msh->bb[2] = 0.0; /* ymin  */
  msh->bb[3] = 0.0; /* ymax  */

  return msh;
}

Mesh *msh_read(char *file, int readEfr)
{
  char InpFil[1024];
  float bufFlt[2];
  double bufDbl[2];
  int i, bufTri[4], bufEfr[3];
  int FilVer, ref;

  int fmsh = 0;

  if (!file)
    return NULL;

  Mesh *msh = msh_init();

  //--- set file name
  strcpy(InpFil, file);
  if (strstr(InpFil, ".mesh"))
  {
    if (!(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)))
    {
      return NULL;
    }
  }
  else
  {
    strcat(InpFil, ".meshb");
    if (!(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)))
    {
      strcpy(InpFil, file);
      strcat(InpFil, ".mesh");
      if (!(fmsh = GmfOpenMesh(InpFil, GmfRead, &FilVer, &msh->Dim)))
      {
        return NULL;
      }
    }
  }

  printf(" File %s opened Dimension %d Version %d \n", InpFil, msh->Dim, FilVer);

  msh->NbrVer = GmfStatKwd(fmsh, GmfVertices);
  msh->NbrTri = GmfStatKwd(fmsh, GmfTriangles);

  msh->NbrVerMax = msh->NbrVer;
  msh->NbrTriMax = msh->NbrTri;

  //--- allocate arrays
  msh->Ver = calloc((msh->NbrVer + 1), sizeof(Vertex));
  msh->Tri = calloc((msh->NbrTri + 1), sizeof(Triangle));

  //--- read vertices
  GmfGotoKwd(fmsh, GmfVertices);
  if (msh->Dim == 2)
  {
    if (FilVer == GmfFloat)
    { // read 32 bits float
      for (i = 1; i <= msh->NbrVer; ++i)
      {
        GmfGetLin(fmsh, GmfVertices, &bufFlt[0], &bufFlt[1], &ref);
        msh->Ver[i].Crd[0] = (double)bufFlt[0];
        msh->Ver[i].Crd[1] = (double)bufFlt[1];
      }
    }
    else
    { // read 64 bits float
      for (i = 1; i <= msh->NbrVer; ++i)
      {
        GmfGetLin(fmsh, GmfVertices, &bufDbl[0], &bufDbl[1], &ref);
        msh->Ver[i].Crd[0] = bufDbl[0];
        msh->Ver[i].Crd[1] = bufDbl[1];
      }
    }
  }
  else
  {
    fprintf(stderr, "  ## ERROR: 3D is not implemented\n");
    exit(1);
  }

  //--- read triangles
  GmfGotoKwd(fmsh, GmfTriangles);
  for (i = 1; i <= msh->NbrTri; ++i)
  {
    GmfGetLin(fmsh, GmfTriangles, &bufTri[0], &bufTri[1], &bufTri[2], &bufTri[3]);
    msh->Tri[i].Ver[0] = bufTri[0];
    msh->Tri[i].Ver[1] = bufTri[1];
    msh->Tri[i].Ver[2] = bufTri[2];
    msh->Tri[i].Ref = bufTri[3];
  }

  //--- read boundary edges
  if (readEfr == 1)
  {
    msh->NbrEfr = GmfStatKwd(fmsh, GmfEdges);
    msh->Efr = calloc((msh->NbrEfr + 1), sizeof(Edge));

    GmfGotoKwd(fmsh, GmfEdges);
    for (i = 1; i <= msh->NbrEfr; ++i)
    {
      GmfGetLin(fmsh, GmfEdges, &bufEfr[0], &bufEfr[1], &bufEfr[2]);
      msh->Efr[i].Ver[0] = bufEfr[0];
      msh->Efr[i].Ver[1] = bufEfr[1];
      msh->Efr[i].Ref = bufEfr[2];
    }
  }

  GmfCloseMesh(fmsh);

  return msh;
}

double *sol_read(char *file, int mshDim, int mshNbrSol)
{
  char InpFil[1024];
  int FilVer, SolTyp, NbrTyp, SolSiz, TypTab[GmfMaxTyp];
  float bufFlt;
  double bufDbl;
  int i, dim, nbrSol;

  int fsol = 0;

  if (!file)
    return NULL;

  double *sol = NULL;

  //--- set file name
  strcpy(InpFil, file);
  if (strstr(InpFil, ".sol"))
  {
    if (!(fsol = GmfOpenMesh(InpFil, GmfRead, &FilVer, &dim)))
    {
      return NULL;
    }
  }
  else
  {
    strcat(InpFil, ".solb");
    if (!(fsol = GmfOpenMesh(InpFil, GmfRead, &FilVer, &dim)))
    {
      strcpy(InpFil, file);
      strcat(InpFil, ".sol");
      if (!(fsol = GmfOpenMesh(InpFil, GmfRead, &FilVer, &dim)))
      {
        return NULL;
      }
    }
  }

  printf(" File %s opened Dimension %d Version %d \n", InpFil, dim, FilVer);

  SolTyp = GmfSolAtVertices; // read only sol at vertices
  nbrSol = GmfStatKwd(fsol, SolTyp, &NbrTyp, &SolSiz, TypTab);

  if (nbrSol == 0)
  {
    printf("  ## WARNING: No SolAtVertices in the solution file !\n");
    return NULL;
  }
  if (dim != mshDim)
  {
    printf("  ## WARNING: WRONG DIMENSION NUMBER. IGNORED\n");
    return NULL;
  }
  if (nbrSol != mshNbrSol)
  {
    printf("  ## WARNING: WRONG SOLUTION NUMBER : %d vs %d, IGNORED\n",nbrSol,mshNbrSol);
    return NULL;
  }
  if (NbrTyp != 1)
  {
    printf("  ## WARNING: WRONG FIELD NUMBER. IGNORED\n");
    return NULL;
  }
  if (TypTab[0] != GmfSca)
  {
    printf("  ## WARNING: WRONG FIELD TYPE. IGNORED\n");
    return NULL;
  }

  sol = (double *)calloc(nbrSol + 1, sizeof(double));

  GmfGotoKwd(fsol, SolTyp);

  for (i = 1; i <= nbrSol; ++i)
  {
    if (FilVer == GmfFloat)
    {
      GmfGetLin(fsol, SolTyp, &bufFlt);
      sol[i] = (double)bufFlt;
    }
    else
    {
      GmfGetLin(fsol, SolTyp, &bufDbl);
      sol[i] = bufDbl;
    }
  }

  if (!GmfCloseMesh(fsol))
  {
    fprintf(stderr, "  ## ERROR: Cannot close solution file %s ! \n", InpFil);
    // myexit(1);
  }

  return sol;
}

int compar_vertex(const void *a, const void *b)
{
  Vertex *va = (Vertex *)a;
  Vertex *vb = (Vertex *)b;
  return (vb->icrit - va->icrit);
}

int compar_triangle(const void *a, const void *b)
{
  Triangle *va = (Triangle *)a;
  Triangle *vb = (Triangle *)b;
  return (vb->icrit - va->icrit);
}

double area(Vertex P1, Vertex P2, Vertex P3)
{

  double x_1 = P1.Crd[0], y_1 = P1.Crd[1];
  double x_2 = P2.Crd[0], y_2 = P2.Crd[1];
  double x_3 = P3.Crd[0], y_3 = P3.Crd[1];

  double aire = (x_1 * (y_2 - y_3) + x_2 * (y_3 - y_1) + x_3 * (y_1 - y_2)) / 2;

  return aire;
}

double quality_area(Mesh *msh, int iTri)
{

  Vertex verts[3];

  verts[0] = msh->Ver[msh->Tri[iTri].Ver[0]];
  verts[1] = msh->Ver[msh->Tri[iTri].Ver[1]];
  verts[2] = msh->Ver[msh->Tri[iTri].Ver[2]];

  double alpha_1 = 1. / (4 * sqrt(3));

  double res = 0;

  double x_1 = verts[0].Crd[0], y_1 = verts[0].Crd[1];
  double x_2 = verts[1].Crd[0], y_2 = verts[1].Crd[1];
  double x_3 = verts[2].Crd[0], y_3 = verts[2].Crd[1];

  res += (x_2 - x_1) * (x_2 - x_1) + (y_2 - y_1) * (y_2 - y_1);
  res += (x_3 - x_1) * (x_3 - x_1) + (y_3 - y_1) * (y_3 - y_1);
  res += (x_3 - x_2) * (x_3 - x_2) + (y_3 - y_2) * (y_3 - y_2);

  double area = (x_1 * (y_2 - y_3) + x_2 * (y_3 - y_1) + x_3 * (y_1 - y_2)) / 2;

  return res * alpha_1 / area;
}

double quality_rho(Mesh *msh, int iTri)
{

  Vertex verts[3];

  verts[0] = msh->Ver[msh->Tri[iTri].Ver[0]];
  verts[1] = msh->Ver[msh->Tri[iTri].Ver[1]];
  verts[2] = msh->Ver[msh->Tri[iTri].Ver[2]];

  double alpha_2 = 1. / 3.4661;

  double x_1 = verts[0].Crd[0], y_1 = verts[0].Crd[1];
  double x_2 = verts[1].Crd[0], y_2 = verts[1].Crd[1];
  double x_3 = verts[2].Crd[0], y_3 = verts[2].Crd[1];

  double l_1 = sqrt((x_2 - x_1) * (x_2 - x_1) + (y_2 - y_1) * (y_2 - y_1));
  double l_2 = sqrt((x_3 - x_1) * (x_3 - x_1) + (y_3 - y_1) * (y_3 - y_1));
  double l_3 = sqrt((x_3 - x_2) * (x_3 - x_2) + (y_3 - y_2) * (y_3 - y_2));

  double res = l_2 >= l_1 ? l_2 : l_1;

  res = l_3 >= res ? l_3 : res;

  double area = (x_1 * (y_2 - y_3) + x_2 * (y_3 - y_1) + x_3 * (y_1 - y_2)) / 2;

  area = (area >= 0) ? area : -area;

  double radius = 2 * area / (l_1 + l_2 + l_3);

  return res * alpha_2 / radius;
}

int msh_reorder(Mesh *msh)
{
  int iVer; // iTri,

  if (!msh)
    return 0;
  if (msh->NbrVer <= 0)
    return 0;

  /* compute bonding box */
  for (iVer = 1; iVer <= msh->NbrVer; iVer++)
  {
    /* todo msh->bb : used to compute the Z-curve index */
  }

  for (iVer = 1; iVer <= msh->NbrVer; iVer++)
  {
    msh->Ver[iVer].icrit = rand(); /* change the randon  by Z  order */
    msh->Ver[iVer].idxNew = iVer;
    msh->Ver[iVer].idxOld = iVer;
  }
  // qsort(&msh->Ver[1], msh->NbrVer, sizeof(Vertex), compar_vertex);

  /* update idxNew for vertices */

  /* re-assign triangles ids */

  /* sort triangles */
  // for (iTri=1; iTri<=msh->NbrTri; iTri++) {
  //   msh->Tri[iTri].icrit = rand();   /* change the randon  by an improved order */
  // }
  // qsort(&msh->Tri[1], msh->NbrTri, sizeof(Triangle), compar_triangle);

  return 1;
}

int push(Node **stack, int val)
{
  Node *p = malloc(sizeof(Node));
  int success = p != NULL;

  if (success)
  {
    p->data = val;
    p->next = *stack;
    *stack = p;
  }

  return success;
}

int pop(Node **stack, int *val)
{
  int success = *stack != NULL;

  if (success)
  {
    Node *p = *stack;
    *stack = (*stack)->next;
    *val = p->data;
    free(p);
  }

  return success;
}

int msh_write(Mesh *msh, char *file)
{
  int iVer, iTri;
  int FilVer = 2;

  if (!msh)
    return 0;
  if (!file)
    return 0;

  int fmsh = GmfOpenMesh(file, GmfWrite, FilVer, msh->Dim);
  if (fmsh <= 0)
  {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }

  GmfSetKwd(fmsh, GmfVertices, msh->NbrVer);
  for (iVer = 1; iVer <= msh->NbrVer; iVer++)
    GmfSetLin(fmsh, GmfVertices, msh->Ver[iVer].Crd[0], msh->Ver[iVer].Crd[1], 0);

  GmfSetKwd(fmsh, GmfTriangles, msh->NbrTri);
  for (iTri = 1; iTri <= msh->NbrTri; iTri++)
    GmfSetLin(fmsh, GmfTriangles, msh->Tri[iTri].Ver[0], msh->Tri[iTri].Ver[1], msh->Tri[iTri].Ver[2], msh->Tri[iTri].Ref);

  GmfCloseMesh(fmsh);

  return 1;
}

int localiser(Mesh *msh, Vertex P)
{
  int iTri = msh->NbrTri;
  //printf("%d", iTri);
  double b1, b2, b3;
  Vertex P1, P2, P3;

  for (int i = 0; i < msh->NbrTri; i++)
  {

    if (iTri == 0)
      return 0;

    P1 = msh->Ver[msh->Tri[iTri].Ver[0]];
    P2 = msh->Ver[msh->Tri[iTri].Ver[1]];
    P3 = msh->Ver[msh->Tri[iTri].Ver[2]];

    b1 = area(P, P2, P3);
    b2 = area(P1, P, P3);
    b3 = area(P1, P2, P);
    // printf("%d : %f, %f, %f \n",iTri,b1,b2,b3);

    if ((b1 > 0) && (b2 > 0) && (b3 > 0))
    {
      return iTri;
    }

    srand(time(NULL));

    int rand_b1_b2 = (b2 < 0) ? rand() % 2 : 1;
    int rand_b2_b3 = (b3 < 0) ? rand() % 2 : 1;
    int rand_b1_b3 = (b3 < 0) ? rand() % 2 : 1;

    if ((b1 < 0) && rand_b1_b2 && rand_b1_b3)
    {

      iTri = msh->Tri[iTri].Voi[0];
    }
    else if ((b2 < 0) && rand_b2_b3)
    {

      iTri = msh->Tri[iTri].Voi[1];
    }
    else if (b3 < 0)
    {

      iTri = msh->Tri[iTri].Voi[2];
    }
  }
  return 0;
}

int msh_neighborsQ2(Mesh *msh)
{
  int iTri, iEdg, jTri, jEdg, ip1, ip2, jp1, jp2;

  if (!msh)
    return 0;

  for (iTri = 1; iTri <= msh->NbrTri; iTri++)
  {
    for (iEdg = 0; iEdg < 3; iEdg++)
    {
      ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];

      /* find the Tri different from iTri that has ip1, ip2 as vertices */
      for (jTri = 1; jTri <= msh->NbrTri; jTri++)
      {
        if (iTri == jTri)
          continue;

        for (jEdg = 0; jEdg < 3; jEdg++)
        {
          jp1 = msh->Tri[jTri].Ver[tri2edg[jEdg][0]];
          jp2 = msh->Tri[jTri].Ver[tri2edg[jEdg][1]];

          /* compare the 4 points */
          if (((jp1 == ip1) && (jp2 == ip2)) || ((jp1 == ip2) && (jp2 == ip1)))
          {
            (msh->Tri[iTri]).Voi[iEdg] = jTri;
            (msh->Tri[jTri]).Voi[jEdg] = iTri;
            break;
          }
        }
      }
    }
  }

  return 1;
}

int insert_simple(Mesh *msh, Vertex P)
{
  int TriP = localiser(msh, P);
  msh->NbrVer++;
  if (msh->NbrVer > msh->NbrVerMax)
  {
    msh->Ver = realloc(msh->Ver, (2 * msh->NbrVerMax + 1) * sizeof(Vertex));
    msh->NbrVerMax *= 2;
  }
  msh->Ver[msh->NbrVer] = P;
  msh->NbrTri += 3;
  if (msh->NbrTri > msh->NbrTriMax)
  {
    msh->Tri = realloc(msh->Tri, (2 * msh->NbrTriMax + 1) * sizeof(Triangle));
    msh->NbrTriMax *= 2;
  }
  // Copies du triangle contenant P
  Triangle Tri0 = msh->Tri[TriP];
  Triangle Tri1 = msh->Tri[TriP];
  Triangle Tri2 = msh->Tri[TriP];

  // On remplace par le pt P dans les nouveaux triangles
  Tri0.Ver[0] = msh->NbrVer;
  Tri1.Ver[1] = msh->NbrVer;
  Tri2.Ver[2] = msh->NbrVer;

  // Indices des nouveaux triangles dans msh->Tri
  int iTri0 = msh->NbrTri - 2;
  int iTri1 = msh->NbrTri - 1;
  int iTri2 = msh->NbrTri;

  msh->Tri[iTri0] = Tri0;
  msh->Tri[iTri1] = Tri1;
  msh->Tri[iTri2] = Tri2;

  msh->Tri[TriP].Ver[0] *= -1;
  msh->Tri[TriP].Ver[1] *= -1;
  msh->Tri[TriP].Ver[2] *= -1;

  msh->Tri[iTri0].Voi[1] = iTri1;
  msh->Tri[iTri0].Voi[2] = iTri2;

  msh->Tri[iTri1].Voi[0] = iTri0;
  msh->Tri[iTri1].Voi[2] = iTri2;

  msh->Tri[iTri2].Voi[0] = iTri0;
  msh->Tri[iTri2].Voi[1] = iTri1;

  for (int k = 0; k < 3; k++)
  {

    int iTri = msh->Tri[TriP].Voi[k];
    for (int iEdg = 0; iEdg < 3; iEdg++)
    {

      int ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      int ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];
      if (((ip1 == msh->Tri[iTri0].Ver[1]) && (ip2 == msh->Tri[iTri0].Ver[2])) || ((ip2 == msh->Tri[iTri0].Ver[1]) && (ip1 == msh->Tri[iTri0].Ver[2])))
      {
        msh->Tri[iTri].Voi[iEdg] = iTri0;
      }
      else if (((ip1 == msh->Tri[iTri1].Ver[0]) && (ip2 == msh->Tri[iTri1].Ver[2])) || ((ip2 == msh->Tri[iTri1].Ver[0]) && (ip1 == msh->Tri[iTri1].Ver[2])))
      {
        msh->Tri[iTri].Voi[iEdg] = iTri1;
      }
      else if (((ip1 == msh->Tri[iTri2].Ver[0]) && (ip2 == msh->Tri[iTri2].Ver[1])) || ((ip2 == msh->Tri[iTri2].Ver[0]) && (ip1 == msh->Tri[iTri2].Ver[1])))
      {
        msh->Tri[iTri].Voi[iEdg] = iTri2;
      }
    }
  }

  return 0;
}

int incirc(Mesh *msh, int iTri, Vertex P)
{

  Vertex vert[3];
  vert[0] = msh->Ver[msh->Tri[iTri].Ver[0]];
  vert[1] = msh->Ver[msh->Tri[iTri].Ver[1]];
  vert[2] = msh->Ver[msh->Tri[iTri].Ver[2]];

  double x_1 = vert[0].Crd[0], y_1 = vert[0].Crd[1];
  double x_2 = vert[1].Crd[0], y_2 = vert[1].Crd[1];
  double x_3 = vert[2].Crd[0], y_3 = vert[2].Crd[1];

  double bc = sqrt((x_3 - x_2) * (x_3 - x_2) + (y_3 - y_2) * (y_3 - y_2));
  double ac = sqrt((x_3 - x_1) * (x_3 - x_1) + (y_3 - y_1) * (y_3 - y_1));
  double ab = sqrt((x_2 - x_1) * (x_2 - x_1) + (y_2 - y_1) * (y_2 - y_1));

  double aire = (x_1 * (y_2 - y_3) + x_2 * (y_3 - y_1) + x_3 * (y_1 - y_2)) / 2;

  double c_x = ((x_1 * x_1 + y_1 * y_1) * y_2 + (x_2 * x_2 + y_2 * y_2) * y_3 + (x_3 * x_3 + y_3 * y_3) * y_1 - (x_3 * x_3 + y_3 * y_3) * y_2 - (x_2 * x_2 + y_2 * y_2) * y_1 - (x_1 * x_1 + y_1 * y_1) * y_3) / (4 * aire);
  double c_y = -((x_1 * x_1 + y_1 * y_1) * x_2 + (x_2 * x_2 + y_2 * y_2) * x_3 + (x_3 * x_3 + y_3 * y_3) * x_1 - (x_3 * x_3 + y_3 * y_3) * x_2 - (x_2 * x_2 + y_2 * y_2) * x_1 - (x_1 * x_1 + y_1 * y_1) * x_3) / (4 * aire);

  double r_c = ac * bc * ab / (4 * aire);

  if ((P.Crd[0] - c_x) * (P.Crd[0] - c_x) + (P.Crd[1] - c_y) * (P.Crd[1] - c_y) <= r_c * r_c)
  {
    return 1;
  }
  return 0;
}

int val_in_pile(Node **pp_stack, int val)
{

  Node *pp = *pp_stack;

  while (pp != NULL)
  {

    if (pp->data == val)
    {
      return 1;
    }

    pp = pp->next;
  }
  return 0;
}

void print_pile(Node **pp_stack)
{
  Node *pp = *pp_stack;
  while (pp != NULL)
  {

    printf("%d, ", pp->data);
    pp = pp->next;
  }
  printf("\n");
}

Node *cavity(Mesh *msh, Vertex P)
{

  Node *cavi = (NULL);
  Node *tests = (NULL);

  int n = 0;
  int iTri = localiser(msh, P);

  push(&cavi, iTri);

  for (int k = 0; k < 3; k++)
  {

    int TriVoi = msh->Tri[iTri].Voi[k];
    if (TriVoi != 0)
    {
      push(&tests, TriVoi);
      n++;
    }
  }
  int val;
  // printf("  P : %f, %f", P.Crd[0], P.Crd[1]);
  for (int i = 0; i < msh->NbrTri; i++)
  {
    // printf(" iteration %d  \n", i);
    // print_pile(&tests);
    // print_pile(&cavi);

    if (i == msh->NbrTri - 1)
    {
      printf("  erreur pile infinie  \n");
    }

    if (n == 0)
    {
      return cavi;
    }

    pop(&tests, &val);
    // printf("  popped val = %d,  incirc = %d \n",val, incirc(msh,val,P));

    n--;
    if (incirc(msh, val, P))
    {
      push(&cavi, val);
      for (int k = 0; k < 3; k++)
      {
        int VoiK = msh->Tri[val].Voi[k];
        if ((VoiK != 0) && !(val_in_pile(&tests, VoiK)) && !(val_in_pile(&cavi, VoiK)))
        {
          push(&tests, VoiK);
          n++;
        }
      }
    }
  }

  // print_pile(&cavi);

  return cavi;
}

int have_common_edg(Mesh *msh, int iTri, int jTriVoi){
  
  if (iTri==jTriVoi){
    return -1;
  }
  int ip1 = -1, ip2 = -1;
  

  for (int i =0; i<3;i++){
    for (int j =0; j<3;j++){
      if ((msh->Tri[iTri].Ver[i]== msh->Tri[jTriVoi].Ver[j])&&(ip1==-1)){
        ip1 = i;
      }
      else if ((msh->Tri[iTri].Ver[i]== msh->Tri[jTriVoi].Ver[j])&&(ip1!=-1)){
        ip2 = i;
      }
    }
  }

  if (ip2==-1){
    return -1;
  }
  if (ip1==0){
    if (ip2==1){
      return 2;
    }
    else if(ip2==2){
      return 1;
    }
  }
  else if (ip1==1){
    if (ip2==2){
      return 0;
    }
  }
  return -1;
}

void delaunay(Mesh *msh, Vertex P)
{
  Node *cavi = cavity(msh, P);
  Node *ref_cavi = cavity(msh, P);
  Node *new_cavi = (NULL);

  int new_triangles_nb=0;

  msh->NbrVer++;
  if (msh->NbrVer > msh->NbrVerMax)
  {
    msh->Ver = realloc(msh->Ver, (2 * msh->NbrVerMax + 1) * sizeof(Vertex));
    msh->NbrVerMax *= 2;
  }
  msh->Ver[msh->NbrVer] = P;

  while (cavi != NULL)
  {
    int iTri = cavi->data;

    msh->Tri[iTri].Ver[0] *= -1;
    msh->Tri[iTri].Ver[1] *= -1;
    msh->Tri[iTri].Ver[2] *= -1;
  
    if (!val_in_pile(&ref_cavi, msh->Tri[iTri].Voi[0]))
    {
      
      msh->NbrTri += 1;
      new_triangles_nb++;
      if (msh->NbrTri > msh->NbrTriMax)
      {
        msh->Tri = realloc(msh->Tri, (2 * msh->NbrTriMax + 1) * sizeof(Triangle));
        msh->NbrTriMax *= 2;
      }

      Triangle nouveau;
      nouveau.Ver[0] = msh->NbrVer;
      nouveau.Ver[1] = -msh->Tri[iTri].Ver[1];
      nouveau.Ver[2] = -msh->Tri[iTri].Ver[2];

      nouveau.Voi[0]= msh->Tri[iTri].Voi[0];
      
      for (int k = 0;k<3;k++){
        if (msh->Tri[msh->Tri[iTri].Voi[0]].Voi[k]==iTri){
          msh->Tri[msh->Tri[iTri].Voi[0]].Voi[k]=msh->NbrTri;
        }
      }

      msh->Tri[msh->NbrTri] = nouveau;
      push(&new_cavi,msh->NbrTri);
    }
    if (!val_in_pile(&ref_cavi, msh->Tri[iTri].Voi[1]))
    {

      msh->NbrTri += 1;
      new_triangles_nb++;

      if (msh->NbrTri > msh->NbrTriMax)
      {
        msh->Tri = realloc(msh->Tri, (2 * msh->NbrTriMax + 1) * sizeof(Triangle));
        msh->NbrTriMax *= 2;
      }

      Triangle nouveau;
      nouveau.Ver[0] = -msh->Tri[iTri].Ver[0];
      nouveau.Ver[1] = msh->NbrVer;
      nouveau.Ver[2] = -msh->Tri[iTri].Ver[2];

      nouveau.Voi[1]= msh->Tri[iTri].Voi[1];

      for (int k = 0;k<3;k++){
        if (msh->Tri[msh->Tri[iTri].Voi[1]].Voi[k]==iTri){
          msh->Tri[msh->Tri[iTri].Voi[1]].Voi[k]=msh->NbrTri;
        }
      }

      msh->Tri[msh->NbrTri] = nouveau;
      push(&new_cavi,msh->NbrTri);

    }
    if (!val_in_pile(&ref_cavi, msh->Tri[iTri].Voi[2]))
    {

      msh->NbrTri += 1;

      new_triangles_nb++;

      if (msh->NbrTri > msh->NbrTriMax)
      {
        msh->Tri = realloc(msh->Tri, (2 * msh->NbrTriMax + 1) * sizeof(Triangle));
        msh->NbrTriMax *= 2;
      }

      Triangle nouveau;
      nouveau.Ver[0] = -msh->Tri[iTri].Ver[0];
      nouveau.Ver[1] = -msh->Tri[iTri].Ver[1];
      nouveau.Ver[2] = msh->NbrVer;

      nouveau.Voi[2]= msh->Tri[iTri].Voi[2];
      for (int k = 0;k<3;k++){
        if (msh->Tri[msh->Tri[iTri].Voi[2]].Voi[k]==iTri){
          msh->Tri[msh->Tri[iTri].Voi[2]].Voi[k]=msh->NbrTri;
        }
      }
      msh->Tri[msh->NbrTri] = nouveau;

      push(&new_cavi,msh->NbrTri);

    }

    
    cavi = cavi->next;
  }
  
  //printf("   new trinalges : %d  \n", new_triangles_nb);
  while (new_cavi != NULL){
      int iTri = new_cavi->data;

      for (int k = msh->NbrTri-new_triangles_nb+1;k<=msh->NbrTri;k++){
        int com_edg = have_common_edg(msh,iTri, k);
        if (com_edg!=-1){
          msh->Tri[iTri].Voi[com_edg] = k;
        }
      }

    new_cavi=new_cavi->next;
  }

  /*
  msh->NbrVer++;
  if(msh->NbrVer>msh->NbrVerMax){
    msh->Ver = realloc( msh->Ver, (2*msh->NbrVerMax+1)* sizeof(Vertex));
    msh->NbrVerMax *=2;
  }
  msh->Ver[msh->NbrVer] = P;
  msh->NbrTri += 3;
  if(msh->NbrTri>msh->NbrTriMax){
    msh->Tri = realloc( msh->Tri, (2*msh->NbrTriMax+1)* sizeof(Triangle));
    msh->NbrTriMax *=2;
  }*/
}

int msh_neighbors(Mesh *msh)
{
  int iTri, iEdg, ip1, ip2, found;

  if (!msh)
    return 0;

  HashTable *hsh_tab = hash_init(2 * msh->NbrVer, 3 * msh->NbrTri);

  for (iTri = 1; iTri <= msh->NbrTri; iTri++)
  {
    for (iEdg = 0; iEdg < 3; iEdg++)
    {
      ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];

      found = hash_find(hsh_tab, ip1, ip2);

      if (found == 0)
      {

        hash_add(hsh_tab, ip1, ip2, iTri);
      }
      else
      {

        // int Tri1 = hsh_tab->LstObj[found][2];
        hsh_tab->LstObj[found][3] = iTri;
        // int Tri2 = hsh_tab->LstObj[found][3];

        // msh->Tri[Tri1].Voi[iEdg]=Tri2;
        // msh->Tri[Tri2].Voi[iEdg]=Tri1;
      }
    }
  }

  for (iTri = 1; iTri <= msh->NbrTri; iTri++)
  {
    for (iEdg = 0; iEdg < 3; iEdg++)
    {
      ip1 = msh->Tri[iTri].Ver[tri2edg[iEdg][0]];
      ip2 = msh->Tri[iTri].Ver[tri2edg[iEdg][1]];
      found = hash_find(hsh_tab, ip1, ip2);

      if (found == 0)
      {

        printf("     erreur found %d %d \n", ip1, ip2);
      }

      else
      {

        int Tri1 = hsh_tab->LstObj[found][2];
        int Tri2 = hsh_tab->LstObj[found][3];

        if (Tri1 == iTri)
        {
          msh->Tri[Tri1].Voi[iEdg] = Tri2;
        }

        else if (Tri2 == iTri)
        {
          msh->Tri[Tri2].Voi[iEdg] = Tri1;
        }
        else
        {
          printf("erreur tri1 tri2");
        }
      }
    }
  } /*
   for (int i =0;i<hsh_tab->NbrObj;i++){
     printf("%d, %d, %d, %d, %d \n", hsh_tab->LstObj[i][0],hsh_tab->LstObj[i][1],hsh_tab->LstObj[i][2],hsh_tab->LstObj[i][3],hsh_tab->LstObj[i][4]);
     fflush(stdout);
   }*/

  msh->Hsh = hsh_tab;

  return hsh_tab->NbrObj;
}

int nb_edges_boundary(Mesh *msh)
{
  int n = 0;
  for (int i = 0; i < msh->Hsh->NbrObj; i++)
  {
    if (msh->Hsh->LstObj[i][3] == 0)
    {
      n++;
    }
  }

  return n;
}

int hash_fct(int ip1, int ip2)
{
  // return ip1*ip2;
  return ip1 + ip2;
  // return ip1 >ip2? ip2 : ip1;
  // return ip1-ip2 > 0? ip1-ip2 : ip2-ip1;
}

HashTable *hash_init(int SizHead, int NbrMaxObj)
{
  HashTable *hsh = malloc(sizeof(HashTable));

  hsh->Head = calloc(SizHead, sizeof(int));
  hsh->LstObj = calloc(NbrMaxObj, sizeof(int5));
  hsh->NbrObj = 0;
  hsh->NbrMaxObj = NbrMaxObj;
  hsh->SizHead = SizHead;

  return hsh;
}

int hash_find(HashTable *hsh, int ip1, int ip2)
{

  int current = hsh->Head[hash_fct(ip1, ip2) % (hsh->SizHead)];

  while (1)
  {

    if (((hsh->LstObj[current][0] == ip1) && (hsh->LstObj[current][1] == ip2)) || ((hsh->LstObj[current][0] == ip2) && (hsh->LstObj[current][1] == ip1)))
      return current;

    if (hsh->LstObj[current][4] == 0)
      return 0;

    current = hsh->LstObj[current][4];
  }

  return 0;
}

int hash_add(HashTable *hsh, int ip1, int ip2, int iTri)
{

  int iObj = hsh->NbrObj;

  hsh->NbrObj++;

  int current = hsh->Head[hash_fct(ip1, ip2) % (hsh->SizHead)];

  if (current == 0)
  {
    hsh->Head[hash_fct(ip1, ip2) % (hsh->SizHead)] = iObj;
    hsh->LstObj[iObj][0] = ip1;
    hsh->LstObj[iObj][1] = ip2;
    hsh->LstObj[iObj][2] = iTri;
    hsh->LstObj[iObj][3] = 0;
    hsh->LstObj[iObj][4] = 0;
    return 0;
  }

  while (1)
  {

    if (hsh->LstObj[current][4] == 0)
    {
      hsh->LstObj[current][4] = iObj;
      hsh->LstObj[iObj][0] = ip1;
      hsh->LstObj[iObj][1] = ip2;
      hsh->LstObj[iObj][2] = iTri;
      hsh->LstObj[iObj][3] = 0;
      hsh->LstObj[iObj][4] = 0;
      return 0;
    }
    if ((hsh->LstObj[current][0] == ip2) && (hsh->LstObj[current][1] == ip1))
    {
      hsh->LstObj[current][3] = iTri;
      return 0;
    }
    current = hsh->LstObj[current][4];
  }

  return 0;
}

int nb_collisions(HashTable *hsh)
{
  int n = 0;
  for (int i = 0; i < hsh->NbrObj; i++)
  {
    if (hsh->LstObj[i][4] != 0)
    {
      n++;
    }
  }
  // tm
  return n;
}

int msh_write2dfield_Vertices(char *file, int nfield, double *field)
{
  int iVer;

  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if (fmsh <= 0)
  {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }

  int sizfld[1];
  sizfld[0] = GmfSca;

  GmfSetKwd(fmsh, GmfSolAtVertices, nfield, 1, sizfld);

  for (iVer = 1; iVer <= nfield; iVer++)
    GmfSetLin(fmsh, GmfSolAtVertices, &field[iVer]);

  GmfCloseMesh(fmsh);

  return 1;
}

int msh_write2dfield_Triangles(char *file, int nfield, double *field)
{
  int iTri;

  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if (fmsh <= 0)
  {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }

  int sizfld[1];
  sizfld[0] = GmfSca;

  GmfSetKwd(fmsh, GmfSolAtTriangles, nfield, 1, sizfld);

  for (iTri = 1; iTri <= nfield; iTri++)
    GmfSetLin(fmsh, GmfSolAtTriangles, &field[iTri]);

  GmfCloseMesh(fmsh);

  return 1;
}

int msh_write2dmetric(char *file, int nmetric, double3 *metric)
{
  int iVer;

  int fmsh = GmfOpenMesh(file, GmfWrite, GmfDouble, 2);
  if (fmsh <= 0)
  {
    printf("  ## ERROR: CANNOT CREATE FILE \n");
    return 0;
  }

  int sizfld[1];
  sizfld[0] = GmfSymMat;

  GmfSetKwd(fmsh, GmfSolAtVertices, nmetric, 1, sizfld);

  for (iVer = 1; iVer <= nmetric; iVer++)
    GmfSetLin(fmsh, GmfSolAtVertices, &metric[iVer][0], &metric[iVer][1], &metric[iVer][2]);

  GmfCloseMesh(fmsh);

  return 1;
}
