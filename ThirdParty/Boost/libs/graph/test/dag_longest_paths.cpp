
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/vector_property_map.hpp>
#include <boost/test/minimal.hpp>

using namespace boost;

#include <iostream>
using namespace std;

int test_main(int, char*[])
{
    typedef adjacency_list<vecS, vecS, directedS, no_property,
        property<edge_weight_t, int> > Graph;

    Graph graph;
    Graph::vertex_descriptor v1, v2, v3, v4;

    v1 = add_vertex(graph);
    v2 = add_vertex(graph);
    v3 = add_vertex(graph);
    v4 = add_vertex(graph);

    Graph::edge_descriptor e;
    
    e = add_edge(0, 1, graph).first;
    put(edge_weight, graph, e, 1);

    e = add_edge(1, 2, graph).first;
    put(edge_weight, graph, e, 1);

    e = add_edge(3, 1, graph).first;
    put(edge_weight, graph, e, 5);

    vector_property_map<int> distance;

    dag_shortest_paths(graph, 0,
                       distance_map(distance)
                       .distance_compare(std::greater<int>())
                       .distance_inf((std::numeric_limits<int>::min)())
                       .distance_zero(0));

    cout << distance[2] << "\n";

    BOOST_CHECK(distance[2] == 2);

    return 0;
}



