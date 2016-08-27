/****************************************************************************
* Italian National Research Council                                         *
* Institute for Applied Mathematics and Information Technologies, Genoa     *
* IMATI-GE / CNR                                                            *
*                                                                           *
* Author: Marco Livesu (marco.livesu@gmail.com)                             *
*                                                                           *
* Copyright(C) 2016                                                         *
* All rights reserved.                                                      *
*                                                                           *
* This file is part of CinoLib                                              *
*                                                                           *
* CinoLib is free software; you can redistribute it and/or modify           *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 3 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
****************************************************************************/
#include <cinolib/unstable/curve.h>

namespace cinolib
{

CINO_INLINE
std::ostream & operator<<(std::ostream & in, const Curve & c)
{
    for(Curve::Sample s : c.samples())
    {
        in << s.pos <<  " \t " << s.tid << " \t ";
        for(double w : s.bary) in << w << "  ";
        in << std::endl;
    }
    in << std::endl;
    return in;
}

CINO_INLINE
Curve::Curve()
{}


CINO_INLINE
Curve::Curve(const std::vector<vec3d> & samples)
{
    for(vec3d p : samples) append_sample(p);
}

CINO_INLINE
Skel Curve::export_as_skeleton() const
{
    std::vector<double> coords;
    std::vector<int>    segs;

    for(size_t i=0; i<sample_list.size(); ++i)
    {
        coords.push_back(sample_list.at(i).pos.x());
        coords.push_back(sample_list.at(i).pos.y());
        coords.push_back(sample_list.at(i).pos.z());

        if (i>0)
        {
            segs.push_back(i-1);
            segs.push_back( i );
        }
    }

    return Skel(coords, segs);
}

CINO_INLINE
const Bbox & Curve::bbox() const
{
    return bb;
}


CINO_INLINE
const std::vector<Curve::Sample> & Curve::samples() const
{
    return sample_list;
}


CINO_INLINE
int Curve::size() const
{
    return sample_list.size();
}


CINO_INLINE
double Curve::length() const
{
    double l = 0.0;
    for(size_t i=1; i<sample_list.size(); ++i)
    {
        l += sample_list.at(i).pos.dist(sample_list.at(i-1).pos);
    }
    return l;
}

CINO_INLINE
void Curve::append_sample(const vec3d & s)
{
    Sample sample;
    sample.pos = s;
    append_sample(sample);
}

CINO_INLINE
void Curve::append_sample(const Sample & s)
{
    sample_list.push_back(s);
    bb.min = bb.min.min(s.pos);
    bb.max = bb.max.max(s.pos);
}

CINO_INLINE
void Curve::pop_back()
{
    sample_list.pop_back();
}

CINO_INLINE
vec3d Curve::sample_curve_at(const float t) const
{
    return sample_curve_at(t, length());
}


CINO_INLINE
vec3d Curve::sample_curve_at(const float t, const double tot_length) const
{
    assert(size() > 1);
    assert(tot_length > 0);
    assert(t>=0.0);
    assert(t<=1.0);

    double curr_l = 0.0;
    double curr_t = 0.0;
    for(size_t i=1; i<sample_list.size(); ++i)
    {
        double seg_l   = sample_list.at(i-1).pos.dist(sample_list[i].pos);
        double delta_t = seg_l / tot_length;

        if (delta_t == 0) continue;

        if (curr_t + delta_t >= t - 1e-7)
        {
            double alpha = (t-curr_t)/(delta_t);
            return (1.0-alpha)*sample_list.at(i-1).pos + alpha*sample_list.at(i).pos;
        }
        else
        {
            curr_t += delta_t;
            curr_l += seg_l;
        }
    }
    assert(false);
    return vec3d();
}


CINO_INLINE
void Curve::resample_curve(const int n_samples)
{
    assert(n_samples >= 1);

    double tot_length = length();
    double delta_t    = 1.0/double(n_samples);
    double t          = 0.0;

    if (size()  < 2) return;
    if (length()<=0) return;

    std::vector<Sample> new_samples;
    for(int i=0; i<=n_samples; ++i)
    {
        Sample s;
        s.pos = sample_curve_at(t,tot_length);
        new_samples.push_back(s);
        t += delta_t;
    }
    sample_list = new_samples;
}


CINO_INLINE
std::vector<int> Curve::tessellate(Trimesh & m) const
{
    for(Sample s : sample_list)
    {
        if (s.tid == -1 || s.bary.size() < m.verts_per_element)
        {
            std::cerr << "WARNING: no link between mesh and curve existing." << std::endl;
            assert(false && "Impossible to tessellate the curve");
        }
    }

    // 1) Split edges
    //
    // ordered list of mesh vertices belonging to the curve
    //
    std::vector<int>  curve_vids;
    std::map<int,int> edges_split;
    for(size_t i=0; i<sample_list.size(); ++i)
    {
        const Sample & s = sample_list.at(i);
        int off;
        if (triangle_bary_is_edge(s.bary, off))
        {
            int   eid         = m.triangle_edge_local_to_global(s.tid, off);
            int   vid0        = m.triangle_vertex_id(s.tid, TRI_EDGES[off][0]);
            int   vid1        = m.triangle_vertex_id(s.tid, TRI_EDGES[off][1]);
            float f0          = m.vertex_u_text(vid0);
            float f1          = m.vertex_u_text(vid1);
            float f_new       = f0 * s.bary.at(TRI_EDGES[off][0]) + f1 * s.bary.at(TRI_EDGES[off][1]);
            int fresh_id      = m.add_vertex(sample_list.at(i).pos, f_new);
            edges_split[eid]  = fresh_id;
            curve_vids.push_back(fresh_id);
        }
        else if (triangle_bary_is_vertex(s.bary, off))
        {
            curve_vids.push_back(m.triangle_vertex_id(s.tid, off));
        }
        else assert(false && "Inner samples are not supported for tessellation yet");
    }

    // 2) Form triangles
    //
    // Serialized triangle split instructions. Structure:
    // tid | # of sub-triangles | first tri (three entries each) | next tris (three entries each)
    // the first tri will be generated using the set_triangle() routine
    // the next tris will be generated using the add_triangle() routine
    //
    std::vector<int> split_list;
    for(size_t i=1; i<sample_list.size(); ++i)
    {
        const Sample & s0 = sample_list.at(i-1);
        const Sample & s1 = sample_list.at( i );

        int  id0, id1;
        bool id0_is_edge = m.elem_bary_is_edge(s0.tid, s0.bary, id0);
        bool id1_is_edge = m.elem_bary_is_edge(s1.tid, s1.bary, id1);

        if (id0_is_edge && id1_is_edge)
        {
            int tid   = m.shared_triangle(id0, id1);
            int pivot = m.shared_vertex(id0, id1);
            int new0  = edges_split.at(id0);
            int new1  = edges_split.at(id1);
            int old0  = m.edge_vertex_id(id0,0); if (old0==pivot) old0 = m.edge_vertex_id(id0,1);
            int old1  = m.edge_vertex_id(id1,0); if (old1==pivot) old1 = m.edge_vertex_id(id1,1);

            split_list.push_back(tid);  // triangle to split
            split_list.push_back(3);    // # of sub-triangles to create
            split_list.push_back(new0);  split_list.push_back(new1);  split_list.push_back(old0); // t0
            split_list.push_back(old0);  split_list.push_back(new1);  split_list.push_back(old1); // t1
            split_list.push_back(pivot); split_list.push_back(new1);  split_list.push_back(new0); // t2
        }
        else if (id0_is_edge && !id1_is_edge)
        {
            // it might also be internal, but this is should not happen so far....
            assert(m.elem_bary_is_vertex(s1.tid, s1.bary, id1));

            // fint the triangle containing both vertex (id1) and edge (id0)
            int tid = -1;
            for(int id : m.adj_edg2tri(id0)) if (m.triangle_contains_vertex(id, id1)) tid = id;
            assert(tid!=-1);

            int old0  = m.edge_vertex_id(id0,0);
            int old1  = m.edge_vertex_id(id0,1);
            int pivot = m.vertex_opposite_to(tid, old0, old1);
            int new_v = edges_split.at(id0);

            split_list.push_back(tid);  // triangle to split
            split_list.push_back(2);    // # of sub-triangles to create
            split_list.push_back(old0);  split_list.push_back(pivot); split_list.push_back(new_v); // t0
            split_list.push_back(old1);  split_list.push_back(new_v); split_list.push_back(pivot); // t1
        }
        else if (!id0_is_edge && id1_is_edge)
        {
            // it might also be internal, but this is should not happen so far....
            assert(m.elem_bary_is_vertex(s0.tid, s0.bary, id0));

            // fint the triangle containing both vertex (id0) and edge (id1)
            int tid = -1;
            for(int id : m.adj_edg2tri(id1)) if (m.triangle_contains_vertex(id, id0)) tid = id;
            assert(tid!=-1);

            int old0  = m.edge_vertex_id(id1,0);
            int old1  = m.edge_vertex_id(id1,1);
            int pivot = m.vertex_opposite_to(tid, old0, old1);
            int new_v = edges_split.at(id1);

            split_list.push_back(tid);  // triangle to split
            split_list.push_back(2);    // # of sub-triangles to create
            split_list.push_back(old0);  split_list.push_back(pivot); split_list.push_back(new_v); // t0
            split_list.push_back(old1);  split_list.push_back(new_v); split_list.push_back(pivot); // t1
        }
        else assert(false && "Unsupported tessellation configuration! (to be added, probably)");
    }

    tessellate(m, split_list);
    return curve_vids;
}


CINO_INLINE
void Curve::tessellate(Trimesh & m, const std::vector<int> & split_list) const
{
    // Serialized triangle split. Structure:
    //
    //      tid | # of sub-triangles | first tri (three entries each) | next tris (three entries each)
    //
    //      the first tri will be generated using the set_triangle() routine
    //      the next tris will be generated using the add_triangle() routine
    //
    for(size_t i=0; i<split_list.size();)
    {
        int tid     = split_list.at(i);
        int subtris = split_list.at(i+1);
        int v0      = split_list.at(i+2);
        int v1      = split_list.at(i+3);
        int v2      = split_list.at(i+4);
        vec3d n_og  = m.triangle_normal(tid);
        vec3d n     = triangle_normal(m.vertex(v0), m.vertex(v1), m.vertex(v2));
        bool flip   = (n_og.dot(n) < 0); // if the normal is opposite flip them all...

        if (flip) std::swap(v1,v2);
        m.set_triangle(tid, v0, v1, v2);

        int base = i+5;
        for(int j=0; j<subtris-1; ++j)
        {
            v0 = split_list.at(base+3*j);
            v1 = split_list.at(base+3*j+1);
            v2 = split_list.at(base+3*j+2);
            if (flip) std::swap(v1,v2);
            m.add_triangle(v0, v1, v2);
        }

        i+=2+3*subtris;
    }

    // sanitize edge connectivity...
    logger.disable();
    m.update_adjacency();
    logger.enable();
}

}
