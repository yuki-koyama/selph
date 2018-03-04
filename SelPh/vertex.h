#ifndef VERTEX_H
#define VERTEX_H

#include <vector>

class Vertex
{
public:
    Vertex(std::vector<double> x, int index);

    std::vector<double> x;
    double value;
    int index;
    std::vector<int> neighbors;
};

#endif // VERTEX_H
