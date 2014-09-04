#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <set>
#include <map>
#include "GeometricTypes3d.hpp"

#include "tr1_macros.hpp"
#include TR1INC(memory)

/**
 * \brief Contains an edge of a mesh
 * \details an Edge is holding the indices of the vertices in the mesh rather than duplicating the coordinates.
 * \ingroup mesh
 */
struct Edge
{
    Edge(size_t v1,size_t v2);

    /* \brief answer the (index of) first vertex encountered when running the edge in a given direction (0 or 1)
     */
    size_t first_vertex(bool reverse_direction) const;

    /* \brief answer the (index of) second vertex encountered when running the edge in a given direction (0 or 1)
     */
    size_t second_vertex(bool reverse_direction) const;

    size_t vertex_index[2];  //!< The index of the two vertices in the mesh
};

class EdgeImmersionStatus
{
    EdgeImmersionStatus():status(0) {}
public:
    EdgeImmersionStatus(unsigned char status_):status(status_) {}

    EdgeImmersionStatus(
             const double z0, //!< the relative immersion of first vertex
             const double z1  //!< the relative immersion of second vertex
             );

    /* \brief answer whether this edge crosses the free surface
     */
    bool crosses_free_surface() const;

    /* \brief answer whether this edge is totally emerged
     */
    bool is_emerged() const;

    /* \brief answer whether this edge is totally immersed
     */
    bool is_immersed() const;

    /* \brief answer whether this edge touches the free surface
     */
    bool touches_free_surface() const;

    unsigned char status; //!< 2 bits of immersion status: 00=totally emerged, 01=second emerged,first immersed, 10=first emerged,second immersed, 11=totally immersed
                          // + third bit = 1 for potentially crossing (if any of the vertices is exactly on free surface)
};


/**
 * \brief Reference to an edge, with a running direction.
 * \details Usefull to describe the edges of a facet.
 * \ingroup mesh
 */
struct OrientedEdge
{
    OrientedEdge(size_t edge_index_,bool reverse_direction_):edge_index(edge_index_),reverse_direction(reverse_direction_) {}
    size_t edge_index;
    bool   reverse_direction;
};

/**
 * \brief Contains a facet of a mesh
 * \details a Facet is holding the indices of the vertices in the mesh rather than duplicating the coordinates
 * \ingroup mesh
 */
struct Facet
{
    Facet():vertex_index(std::vector<size_t>()),unit_normal(Eigen::MatrixXd::Zero(3,1)),barycenter(Eigen::MatrixXd::Zero(3,1)),area(0) {}
    std::vector<size_t> vertex_index;
    Eigen::Vector3d unit_normal;
    Eigen::Vector3d barycenter;
    double area;
};

typedef std::vector<Facet> VectorOfFacet;

class FacetIterator
{
    public:
        FacetIterator(const VectorOfFacet::const_iterator& begin_, const std::vector<size_t>& list_of_facets_, const size_t p) : begin(begin_), list_of_facets(list_of_facets_), pos(p)
        {
        }

        size_t index() const
        {
            return list_of_facets[pos];
        }

        const Facet& operator*() const
        {
            return *(begin+list_of_facets[pos]);
        }

        const FacetIterator& operator++()
        {
            pos++;
            return *this;
        }

        const Facet* operator->() const
        {
            return (begin+list_of_facets[pos]).operator ->();
        }

        bool operator!=(const FacetIterator& rhs) const
        {
            return (begin!=rhs.begin) or (pos != rhs.pos);
        }

    private:
        VectorOfFacet::const_iterator begin;
        std::vector<size_t> list_of_facets;
        size_t pos;
};

/**
 * \author gj
 * \brief Contains a triangular mesh
 * \ingroup mesh
 */
class Mesh
{
    Mesh();
public:
    Mesh(const Matrix3x& nodes_,
            const std::vector<Edge>& edges_,
            const std::vector<Facet>& facets_,
            const std::vector<std::vector<size_t> >& facetsPerEdge_ , //!< for each Edge (index), the list of Facet (indices) to which the edge belongs
            const std::vector<std::vector<OrientedEdge> >& edgesPerFacet_,  //!< for each Facet (index), the list of Edges composing the facet and their running direction of each edge
            const bool clockwise);


    /* \brief Reset the dynamic data related to the mesh intersection with free surface */
    void reset_dynamic_data();

    /* \brief add an edge
     * \return the edge index */
    size_t add_edge(const size_t first_vertex_index,const size_t last_vertex_index);

    /* \brief add an vertex
     * \return the vertex index */
    size_t add_vertex(const EPoint &vertex_coords);

    /* \brief create a new facet dynamically
     * \return the facet index in facets vector */
    size_t create_facet_from_edges(
            const std::vector<OrientedEdge>& edge_list, //!< The list of edges and running direction composing the facet
            const EPoint &unit_normal                   //!< The unit_normal is shared in case of facet split, let's not recompute it
            );

    Matrix3x nodes;            //!< Coordinates of static vertices in mesh
    std::vector<Edge> edges;   //!< All edges in mesh
    std::vector<Facet> facets; //!< For each facet, the indexes of its nodes, unit normal, barycenter & area
    std::vector<std::vector<size_t> > facetsPerEdge; //!< for each Edge (index), the list of Facet (indices) to which the edge belongs
    std::vector<std::vector<OrientedEdge> > edgesPerFacet; //!< for each Facet (index), the list of Edges composing the facet and running direction of each edge

    size_t static_nodes;       //!< Number of static nodes
    size_t static_edges;       //!< Number of static edges
    size_t static_facets;      //!< Number of static facets
    Matrix3x all_nodes;        //!< Coordinates of all vertices in mesh, including dynamic ones added for free surface intersection
    size_t node_count;         //!< Total number of nodes used, including dynamic ones
    double orientation_factor; //!< -1 if the facet is orientation clockwise, +1 otherwise
};

typedef TR1(shared_ptr)<Mesh> MeshPtr;
typedef TR1(shared_ptr)<const Mesh> const_MeshPtr;

struct MeshIntersector
{
    MeshIntersector(
            const MeshPtr mesh_,                  //!< the mesh to intersect
            const std::vector<double> &immersions //!< the relative immersion of each static vertex of the mesh
            );

    /* \brief Update the intersection of the mesh with free surface
     * \details the intersection requires new Vertices/Edges/Facets stored as dynamic data in the end of container members */
    void update_intersection_with_free_surface();

    FacetIterator begin_immersed() const
    {
        return FacetIterator(mesh->facets.begin(), index_of_immersed_facets, 0);
    }

    FacetIterator end_immersed() const
    {
        return FacetIterator(mesh->facets.begin(), index_of_immersed_facets, index_of_immersed_facets.size());
    }

    FacetIterator begin_emerged() const
    {
        return FacetIterator(mesh->facets.begin(), index_of_emerged_facets, 0);
    }

    FacetIterator end_emerged() const
    {
        return FacetIterator(mesh->facets.begin(), index_of_emerged_facets, index_of_emerged_facets.size());
    }

    /* \brief Compute the point of intersection with free surface between two vertices
     * \details One of the vertices must be emerged and the other immersed */
    static EPoint edge_intersection(const EPoint& A, const double dzA, const EPoint& B, const double dzB);

    /* \brief split an edge into an emerged and an immersed part */
    size_t split_partially_immersed_edge(
            const size_t edge_index ,                                //!< the index of edge to be split
            std::vector<EdgeImmersionStatus> &edges_immersion_status //!< the immersion status of each edge
            );

    /* \brief split a Facet into an emerged and an immersed part */
    void split_partially_immersed_facet(
            size_t facet_index,                                             //!< the index of facet to be split
            const std::vector<EdgeImmersionStatus> &edges_immersion_status, //!< the immersion status of each edge
            const std::map<size_t,size_t >& added_edges                     //!< the map of split edges
            );

    /* \brief Extract the coordinates of a specific facet */
    Matrix3x coordinates_of_facet(size_t facet_index) const;

    /* \brief Extract the relative immersions of a specific facet */
    std::vector<double> immersions_of_facet(size_t facet_index) const;

    MeshPtr mesh;

    std::vector<double> all_immersions; //<! the immersions of all nodes (including the dynamically added ones)
    std::vector<size_t> index_of_emerged_facets;  //!< list of all emerged facets (included the dynamically ones created by split)
    std::vector<size_t> index_of_immersed_facets; //!< list of all immersed facets (included the dynamically ones created by split)

    friend class ImmersedFacetIterator;
    friend class EmergedFacetIterator;
};

typedef TR1(shared_ptr)<MeshIntersector> MeshIntersectorPtr;
typedef TR1(shared_ptr)<const MeshIntersector> const_MeshIntersectorPtr;

#endif //MESH_HPP
