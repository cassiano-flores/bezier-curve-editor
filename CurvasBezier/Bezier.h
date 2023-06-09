//
//  CurvaBezier.hpp
//  OpenGL
//
//  Created by Márcio Sarroglia Pinho on 10/10/21.
//  Copyright © 2021 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef Bezier_h
#define Bezier_h

#include <iostream>
using namespace std;


#ifdef WIN32
#include <windows.h>
#include <GL/glut.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"

enum ModoCurva {NORMAL, POSICAO, DERIVADA};

class Bezier
{
    Ponto Coords[3];

public:
    float ComprimentoTotalDaCurva;
    int cor;

    Bezier();
    Bezier(Ponto P0, Ponto P1, Ponto P2, ModoCurva modo);
    Bezier(Ponto V[]);
    Ponto Calcula(double t);
    Ponto getPC(int i);
    void Traca();
    void TracaPoligonoDeControle();
    double CalculaT(double distanciaPercorrida);
    void calculaComprimentoDaCurva();
    Ponto getDerivada(double t, Ponto p0, Ponto p1, Ponto p2);
    void setPC(int i, double x, double y);
    ModoCurva modo;
    ModoCurva getModo();
    void setModo();
};

bool clicouEmPC(Ponto pontoClicado, Ponto pontoCurva);

#endif
