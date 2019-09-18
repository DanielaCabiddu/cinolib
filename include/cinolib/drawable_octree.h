/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2016: Marco Livesu                                              *
*                                                                               *
*  The MIT License                                                              *
*                                                                               *
*  Permission is hereby granted, free of charge, to any person obtaining a      *
*  copy of this software and associated documentation files (the "Software"),   *
*  to deal in the Software without restriction, including without limitation    *
*  the rights to use, copy, modify, merge, publish, distribute, sublicense,     *
*  and/or sell copies of the Software, and to permit persons to whom the        *
*  Software is furnished to do so, subject to the following conditions:         *
*                                                                               *
*  The above copyright notice and this permission notice shall be included in   *
*  all copies or substantial portions of the Software.                          *
*                                                                               *
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
*  FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE *
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
*  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS *
*  IN THE SOFTWARE.                                                             *
*                                                                               *
*  Author(s):                                                                   *
*                                                                               *
*     Marco Livesu (marco.livesu@gmail.com)                                     *
*     http://pers.ge.imati.cnr.it/livesu/                                       *
*                                                                               *
*     Italian National Research Council (CNR)                                   *
*     Institute for Applied Mathematics and Information Technologies (IMATI)    *
*     Via de Marini, 6                                                          *
*     16149 Genoa,                                                              *
*     Italy                                                                     *
*********************************************************************************/
#ifndef CINO_DRAWABLE_OCTREE_H
#define CINO_DRAWABLE_OCTREE_H

#include <cinolib/drawable_object.h>
#include <cinolib/drawable_aabb.h>
#include <cinolib/octree.h>

namespace cinolib
{

template<typename T, uint MaxDepth = 7, uint PrescribedItemsPerLeaf = 1>
class DrawableOctree : public Octree<T,MaxDepth,PrescribedItemsPerLeaf>, public DrawableObject
{
    AbstractPolygonMesh<> *ptr;

    public:

        explicit DrawableOctree( AbstractPolygonMesh<> & m);
        explicit DrawableOctree(const std::vector<T> & items, const std::vector<Bbox> & bboxes);

        ~DrawableOctree() {}

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void  draw(const float scene_size=1) const;
        vec3d scene_center() const;
        float scene_radius() const;

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        ObjectType object_type() const { return DRAWABLE_CURVE; }

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void update_aabbs();
        void update_aabbs(const OctreeNode * node);

        //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        void set_color(const Color & c);
        void set_thickness(float t);

    private:

        std::vector<DrawableAABB> aabbs;
        Color color = Color::BLACK();
        float thickness = 1.0;
};

}

#ifndef  CINO_STATIC_LIB
#include "drawable_octree.cpp"
#endif

#endif // CINO_DRAWABLE_OCTREE_H
