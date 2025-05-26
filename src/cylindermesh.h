#pragma once
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include "cylinderwidget.h"

/* Builds a closed cylinder centred at (0,0,0) */
/* radius=1 , height=2 , subdiv = n slices     */
inline void makeCylinderMesh(QVector<Vtx>& vbo,
                             QVector<Tri>& ibo,
                             int n = 40)
{
    vbo.clear(); ibo.clear();
    /* --- side vertices (2*n) -------------------------------------- */
    for (int i = 0; i < n; ++i) {
        float th = 2.f * M_PI * i / n;
        float x  = std::cos(th), z = std::sin(th);
        float u  = i / float(n);          // last vertex gets u ≈ 1-1/n
        vbo << Vtx{ {x,-1,z}, {u,1.f} }
            << Vtx{ {x, 1,z}, {u,0.f} };
    }
    /* --- top & bottom centre verts -------------------------------- */
    int idxTop = vbo.size();   vbo << Vtx{ {0, 1,0}, {0.25f,0.25f} };
    int idxBot = vbo.size();   vbo << Vtx{ {0,-1,0}, {0.75f,0.25f} };

    /* --- top & bottom ring verts (n each) ------------------------- */
    for(int i=0;i<n;++i){
        float th = 2.f*M_PI*i/n;
        float x = std::cos(th), z = std::sin(th);
        float u = 0.25f*(1.f+ x);
        float v = 0.25f*(1.f+ z);
        vbo << Vtx{ {x, 1,z}, {u, v} }; // top
    }

    for(int i=0;i<n;++i){
        float th = 2.f*M_PI*i/n;
        float x = std::cos(th), z = std::sin(th);
        float u = 0.25f*(3.f+ x);
        float v = 0.25f*(1.f+ z);
        vbo << Vtx{ {x,-1,z}, {u, v} }; // bottom
    }

    /* --- side indices (quads -> 2 tris) --------------------------- */
    for (int i = 0; i < n; ++i) {
        int a = 2 *  i;
        int b = 2 * ((i + 1) % n);        // wrap last→first
        int c = a + 1;
        int d = b + 1;
        ibo << Tri{a,b,c} << Tri{c,b,d};
    }
    /* --- top ------------------------------------------------------ */
    int baseTop = idxTop+1;
    for(int i=0;i<n;++i){
        int a = baseTop + i;
        int b = baseTop + (i+1)%n;
        ibo << Tri{ idxTop, a, b };
    }
    /* --- bottom --------------------------------------------------- */
    int baseBot = idxTop+1+n;
    for(int i=0;i<n;++i){
        int a = baseBot + i;
        int b = baseBot + (i+1)%n;
        ibo << Tri{ idxBot, b, a };      /* note winding for culling */
    }
}
