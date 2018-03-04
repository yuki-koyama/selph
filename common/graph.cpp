#include "graph.h"

#include <cmath>
#include <algorithm>
#include <iostream>

#include <Eigen/Cholesky>

using namespace std;
using namespace Eigen;

namespace {
extern double getLength(const vector<double> &x1, const vector<double> &x2);
extern double getWeight(const vector<double> &x1, const vector<double> &x2);
}

Graph::Graph() : weight(5.0), nearests(100000)
{
}

void Graph::analyze(bool useRegularization) {
    constructConnectivities();
    makeLaplacianMatrix();
    makeConstraintsMatrix();
    makeLinearSystems();
    VectorXd optimalSolution = solveLinearSystems();

    // compute residual
    residual_relative   = (A * optimalSolution - b).norm();
    residual_continuous = (L * optimalSolution).norm();
    residual = residual_relative + weight * residual_continuous;

    int v = vertices.size();
    for (int i = 0; i < v; ++ i) {
        vertices[i].value = optimalSolution(i);
    }

    if (useRegularization) regularize();
}

// to make sure that the values are in [0, 1]
void Graph::regularize() {
    double max = vertices[0].value;
    double min = vertices[0].value;
    int size = vertices.size();
    for (int i = 0; i < size; ++ i) {
        double value = vertices[i].value;
        max = (max < value) ? value : max;
        min = (min > value) ? value : min;
    }
    for (int i = 0; i < size; ++ i) {
        vertices[i].value = (vertices[i].value - min) / (max - min);
    }
}

namespace {
struct Item {
    int     index;
    double   value;

    Item(int index, double value) : index(index), value(value) {}

    bool operator<(const Item& right) const {
        return value < right.value;
    }
};
}

// the results do NOT include its own
void Graph::constructConnectivities() {
    // based on "k-nearest neighborhoods"
    for (Vertex& v : vertices) {
        v.neighbors.clear();
        vector<Item> items;
        for (Vertex& v2 : vertices) {
            // itself
            if (v.index == v2.index) {
                continue;
            }
            // others
            double length = getLength(v.x, v2.x);
            items.push_back(Item(v2.index, length));
        }
        int k = min<int>(nearests, items.size());
        partial_sort(items.begin(), items.begin() + k, items.end());
        for (int i = 0; i < k; ++ i) {
            v.neighbors.push_back(items[i].index);
        }
    }
}

void Graph::makeLaplacianMatrix() {
    int v = vertices.size();

    // make L (v * v)
    L = MatrixXd::Zero(v, v);
    for (int row = 0; row < v; ++ row) {
        vector<double> rowVector;
        rowVector.resize(v, 0.0);
        Vertex &currentVertex = vertices[row];
        rowVector[row] = 1.0;
        double sum = 0.0;
        for (int index : currentVertex.neighbors) {
            Vertex &vertex = vertices[index];
            double w = getWeight(vertex.x, currentVertex.x);
            rowVector[vertex.index] = - w;
            sum += w;
        }
        for (int index : currentVertex.neighbors) {
            Vertex &vertex = vertices[index];
            rowVector[vertex.index] /= sum;
        }
        for (int i = 0; i < v; ++ i) {
            L(row, i) = rowVector[i];
        }
    }
}

void Graph::makeConstraintsMatrix() {
    int v = vertices.size();
    int c = constraints.size();

    // make A (c * v) and b (c * 1)
    A = MatrixXd::Zero(c, v);
    b = VectorXd(c);
    for (int i = 0; i < c; ++ i) {
        Constraint &constraint = constraints[i];
        A(i, constraint.vertexIndexA) = + 1.0;
        A(i, constraint.vertexIndexB) = - 1.0;
        b(i) = constraint.d;
    }
}

void Graph::makeLinearSystems() {
    // make ATilde and bTilde
    ATilde = A.transpose() * A + weight * L.transpose() * L;
    bTilde = A.transpose() * b;
}

Eigen::VectorXd Graph::solveLinearSystems() const
{
    int N = vertices.size();
    //    cout << "Solving linear system (matrix size: " << N << " x " << N << ")" << endl;

    LDLT<MatrixXd> cholesky(ATilde);
    VectorXd x = cholesky.solve(bTilde);

    //    PartialPivLU<MatrixXd> lu(ATilde);
    //    VectorXd x = lu.solve(bTilde);

    bool solutionExists = (ATilde * x).isApprox(bTilde);
    if (!solutionExists) cerr << "Error: Failed to find a solution" << endl;

    VectorXd solution(N);
    for (int i = 0; i < N; ++ i) {
        solution[i] = x(i);
    }
    return solution;
}

Vertex& Graph::getBestVertex() {
    Vertex* v = &vertices[0];
    double maximum = vertices[0].value;
    for (Vertex& current : vertices) {
        if (maximum <= current.value) {
            maximum = current.value;
            v = &current;
        }
    }
    return *v;
}

//////////////////////////////////////////////////////////////////////////
// miscs (not class method)
//////////////////////////////////////////////////////////////////////////

namespace {
double getWeight(const vector<double> &x1, const vector<double> &x2) {
    double length = getLength(x1, x2);
    return pow(length + 1e-05, - 1.0);
}

double getLength(const vector<double> &x1, const vector<double> &x2) {
    assert (x1.size() == x2.size());

    int size = x1.size();
    double sum = 0.0;
    for (int i = 0; i < size; ++ i) {
        sum += (x1[i] - x2[i]) * (x1[i] - x2[i]);
    }

    return sqrt(sum);
}
}
