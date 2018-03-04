#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <Eigen/Core>
#include "eigenutility.h"

#include "vertex.h"
#include "constraint.h"

///////////////////////////////////////////////////////
// input:  constraints + vertices (x, index)
// output: vertices (value) + residual
///////////////////////////////////////////////////////

class Graph
{
public:
    Graph();

    inline int addVertex(Eigen::VectorXd x)
    {
        int index = vertices.size();
        Vertex v(EigenUtility::eigen2std(x), index);
        vertices.push_back(v);
        return index;
    }

    std::vector<Constraint> constraints;
    std::vector<Vertex>     vertices;

    // input parameters
    double weight;
    int    nearests;

    // output
    double residual;
    double residual_relative;
    double residual_continuous;

    int dimension() const { assert(!vertices.empty()); return vertices[0].x.size(); }
    void analyze(bool useRegularization = true);
    Vertex& getBestVertex();

private:
    void constructConnectivities();
    void makeLaplacianMatrix();
    void makeConstraintsMatrix();
    void makeLinearSystems();
    void regularize();

    Eigen::VectorXd solveLinearSystems() const;

    // v = #vertices, c = #constraints
    Eigen::MatrixXd L;         // v * v
    Eigen::VectorXd z;         // v * 1
    Eigen::MatrixXd A;         // c * v
    Eigen::VectorXd b;         // c * 1
    Eigen::MatrixXd ATilde;    // (v + c) * v
    Eigen::VectorXd bTilde;    // (v + c)
};

#endif // GRAPH_H
