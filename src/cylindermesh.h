#pragma once
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include "cylinderwidget.h"

constexpr int CYL_SLICES = 40;


inline void makeCylinderMesh(QVector<Vtx>& vbo,
                             QVector<Tri>& ibo,
                             int n = CYL_SLICES)
{
    vbo.clear(); ibo.clear();

    /* STEP 1: Generate side vertices */
    for (int i = 0; i <= n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);
        float u = float(i) / float(n);

        vbo << Vtx{ {x, -1.0f, z}, {u, 0.5f} };
        vbo << Vtx{ {x, 1.0f, z}, {u, 0.0f} };
    }

    /* STEP 2: Generate side triangles */
    for (int i = 0; i < n; ++i) {
        int bottomLeft  = 2 * i;
        int topLeft     = 2 * i + 1;
        int bottomRight = 2 * (i + 1);
        int topRight    = 2 * (i + 1) + 1;
        ibo << Tri{bottomLeft, bottomRight, topLeft};
        ibo << Tri{topLeft, bottomRight, topRight};
    }

    /* STEP 3: Create cap centers */
    int topCenterIdx = vbo.size();
    vbo << Vtx{ {0.0f, 1.0f, 0.0f}, {0.25f, 0.75f} };

    int bottomCenterIdx = vbo.size();
    vbo << Vtx{ {0.0f, -1.0f, 0.0f}, {0.75f, 0.75f} };

    /* STEP 4: Create cap rim vertices */
    int topRimStart = vbo.size();
    for (int i = 0; i < n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);

        float u_top = 0.25f + 0.2f * x;
        float v_top = 0.75f + 0.2f * z;
        vbo << Vtx{ {x, 1.0f, z}, {u_top, v_top} };
    }

    int bottomRimStart = vbo.size();
    for (int i = 0; i < n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);

        float u_bottom = 0.75f + 0.2f * x;
        float v_bottom = 0.75f + 0.2f * z;
        vbo << Vtx{ {x, -1.0f, z}, {u_bottom, v_bottom} };
    }

    /* STEP 5: Create cap triangles */
    for (int i = 0; i < n; ++i) {
        int current = topRimStart + i;
        int next = topRimStart + ((i + 1) % n);
        ibo << Tri{topCenterIdx, current, next};
    }

    for (int i = 0; i < n; ++i) {
        int current = bottomRimStart + i;
        int next = bottomRimStart + ((i + 1) % n);
        ibo << Tri{bottomCenterIdx, next, current};
    }
}
