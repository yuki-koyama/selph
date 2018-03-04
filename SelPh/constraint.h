#ifndef CONSTRAINT_H
#define CONSTRAINT_H

class Constraint
{
public:
    Constraint(int vertexIndexA, int vertexIndexB, double d);

    int vertexIndexA;
    int vertexIndexB;

    double d;
};

#endif // CONSTRAINT_H
