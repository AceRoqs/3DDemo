//=========================================================================
// Copyright (c) 2003 Toby Jones. All rights reserved.
// Purpose: View information for world
//=========================================================================
#include "PreCompile.h"
#include "world.h"
#include "polygon.h"
#include "coord.h"
#include "gltex.h"

//---------------------------------------------------------------------------
// TODO: 2014: This should not be extern.
const float g_WorldVector[] =
{
    -2, -2, -10,   // left lower front      0
    -2,  2, -10,   // left upper front      1
     2, -2, -10,   // right lower front     2
     2,  2, -10,   // right upper front     3
     2, -2,   0,   // right lower back      4
    -2, -2,   0,   // left lower back       5
    -2,  2,   0,   // left upper back       6
     2,  2,   0,   // right upper back      7
    // sky
    -500,  50, -500,  // left upper front   8
    -500,  50, 500,   // left upper back    9
     500,  50, 500,   // right upper back   10
     500,  50, -500,  // right upper front  11

    // outside wall
    -10, -2, -20,   // left lower front     12
    -10,  2, -20,   // left upper front     13
     10, -2, -20,   // right lower front    14
     10,  2, -20,   // right upper front    15
    // far floor
     10, -2, -20,   //                      16
     10, -2, -10,   //                      17
    -10, -2, -10,   //                      18
    // left far wall
    -10, 2, -10,        //                      19

     10, 2, -10,        //                      20
     10, 4, -10,        //                      21
    -10, 4, -10,        //                      22
      2, 4, -10,        //                      23
     -2, 4, -10,        //                      24
     10,-2, -15,        //                      25
     10, 2, -15,        //                      26
     10,-2, -16,        //                      27
     10, 2, -16,        //                      28
    // bezier curve definitions
    -2, 0, -10,     // 0 29
    -2, 0, -11,     // 1 30
    -3, 0, -11,     // 2 31
    -2,-1, -10,     // 3 32
    -2,-1, -11,     // 4 33
    -3,-1, -11,     // 5 34
    //-2,-2, -10,       // 6 dupe 0
    -2,-2, -11,     // 7 35
    -3,-2, -11,     // 8 36
    -4, 0, -11,     // 9 37
    -4, 0, -10,     // 10 38
    -4,-1, -11,     // 11 39
    -4,-1, -10,     // 12 40
    -4,-2, -11,     // 13 41
    -4,-2, -10      // 14 42
};

static const float WorldTexture[] =
{
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,
    1.0, 1.0,
    8.0, 0.0,
    8.0, 1.0,
    5.0, 0.0,
    5.0, 1.0,
    2.5, 0.0,
    2.5, 1.0,
};

//---------------------------------------------------------------------------
static void load_world_data(
    std::istream& is,
    std::vector<CPolygon>* polys,
    std::vector<Position_vertex>* vertex_formats,
    std::vector<TexCoord>* texture_coords)
{
    unsigned int cTextures;

    is >> cTextures;

/*
    block_t block;
    block.bitmap = nullptr;
    char sz[100];
    is >> sz;
    TGADecodeRGB(sz, &block);
    HANDLE hFile;
    hFile = CreateFile("default.bin",
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       nullptr,
                       CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL,
                       nullptr);
    DWORD dw;
    WriteFile(hFile, block.bitmap, block.xsize * block.ysize * 3, &dw, nullptr);
    CloseHandle(hFile);
    delete[] block.bitmap;
*/
    unsigned int ii;
    for(ii = 0; ii < cTextures; ii++)
    {
        char filename[MAX_PATH];
        is >> filename;
        bind_file_to_gl_texture(filename, ii);
    }

    unsigned int cPolys;
    is >> cPolys;

    //g_paPolys = new(std::nothrow) CPolygon[g_cPolys];

    CPolygon poly;
    for(ii = 0; ii < cPolys; ++ii)
    {
        is >> poly;
        polys->push_back(poly);
    }

    //g_paVerts = new(std::nothrow) Position_vertex[g_cPolys * 4];
    //g_paTexCoord = new(std::nothrow) TexCoord[g_cPolys * 4];
    //*out_vertex_formats = new Position_vertex[cPolys * 4];
    //*out_coords = new TexCoord[cPolys * 4];
    for(ii = 0; ii < cPolys; ii++)
    {
        int ix = (*polys)[ii].getPoint(0);
        Position_vertex out_vertex_format;
        out_vertex_format.aVertex[0] = g_WorldVector[ix * 3];
        out_vertex_format.aVertex[1] = g_WorldVector[ix * 3 + 1];
        out_vertex_format.aVertex[2] = g_WorldVector[ix * 3 + 2];
        vertex_formats->push_back(out_vertex_format);

        ix = (*polys)[ii].getTexCoord(0);
        TexCoord out_coord;
        out_coord.aTexCoord[0] = WorldTexture[ix * 2];
        out_coord.aTexCoord[1] = WorldTexture[ix * 2 + 1];
        texture_coords->push_back(out_coord);

        ix = (*polys)[ii].getPoint(1);
        out_vertex_format.aVertex[0] = g_WorldVector[ix * 3];
        out_vertex_format.aVertex[1] = g_WorldVector[ix * 3 + 1];
        out_vertex_format.aVertex[2] = g_WorldVector[ix * 3 + 2];
        vertex_formats->push_back(out_vertex_format);
        ix = (*polys)[ii].getTexCoord(1);
        out_coord.aTexCoord[0] = WorldTexture[ix * 2];
        out_coord.aTexCoord[1] = WorldTexture[ix * 2 + 1];
        texture_coords->push_back(out_coord);

        ix = (*polys)[ii].getPoint(2);
        out_vertex_format.aVertex[0] = g_WorldVector[ix * 3];
        out_vertex_format.aVertex[1] = g_WorldVector[ix * 3 + 1];
        out_vertex_format.aVertex[2] = g_WorldVector[ix * 3 + 2];
        vertex_formats->push_back(out_vertex_format);
        ix = (*polys)[ii].getTexCoord(2);
        out_coord.aTexCoord[0] = WorldTexture[ix * 2];
        out_coord.aTexCoord[1] = WorldTexture[ix * 2 + 1];
        texture_coords->push_back(out_coord);

        ix = (*polys)[ii].getPoint(3);
        out_vertex_format.aVertex[0] = g_WorldVector[ix * 3];
        out_vertex_format.aVertex[1] = g_WorldVector[ix * 3 + 1];
        out_vertex_format.aVertex[2] = g_WorldVector[ix * 3 + 2];
        vertex_formats->push_back(out_vertex_format);
        ix = (*polys)[ii].getTexCoord(3);
        out_coord.aTexCoord[0] = WorldTexture[ix * 2];
        out_coord.aTexCoord[1] = WorldTexture[ix * 2 + 1];
        texture_coords->push_back(out_coord);
    }
}

//---------------------------------------------------------------------------
void start_load(
    _In_ char* szFileName,
    std::vector<CPolygon>* polys,
    std::vector<Position_vertex>* vertex_formats,
    std::vector<TexCoord>* texture_coords)
{
    std::ifstream fis;
    fis.open(szFileName);
    if(fis.is_open())
    {
        load_world_data(fis, polys, vertex_formats, texture_coords);
        fis.close();
    }
}

