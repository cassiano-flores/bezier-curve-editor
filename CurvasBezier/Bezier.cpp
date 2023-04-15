//
//  Bezier.cpp
//  OpenGL
//

#include "Bezier.h"
#include "ListaDeCoresRGB.h"

// **********************************************************************
Bezier::Bezier()
{
    for (int i=0;i<3;i++)
        Coords[i] = Ponto(0,0,0);
    ComprimentoTotalDaCurva = 0;
    cor = rand() % 100;
    //cout << "Cor: " << cor << endl;
}
// **********************************************************************
//
// **********************************************************************
void Bezier::calculaComprimentoDaCurva()
{
    double DeltaT = 1.0/50;
    double t=DeltaT;
    Ponto P1, P2;

    ComprimentoTotalDaCurva = 0;

    P1 = Calcula(0.0);
    while(t<1.0)
    {
        P2 = Calcula(t);
        ComprimentoTotalDaCurva += calculaDistancia(P1,P2);
        P1 = P2;
        t += DeltaT;
    }
    P2 = Calcula(1.0); // faz o fechamento da curva
    ComprimentoTotalDaCurva += calculaDistancia(P1,P2);
    //cout << "ComprimentoTotalDaCurva: " << ComprimentoTotalDaCurva << endl;

}
// **********************************************************************
Bezier::Bezier(Ponto P0, Ponto P1, Ponto P2, ModoCurva modo)
{
    Coords[0] = P0;
    Coords[1] = P1;
    Coords[2] = P2;
    this->modo = modo;
    calculaComprimentoDaCurva();
    cor = rand() % 100;
}
// **********************************************************************
Bezier::Bezier(Ponto V[])
{
    for (int i=0;i<3;i++)
        Coords[i] = V[i];
    calculaComprimentoDaCurva();
    cor = rand() % 100;
}
// **********************************************************************
//
// **********************************************************************
Ponto Bezier::Calcula(double t)
{
    Ponto P;
    double UmMenosT = 1-t;

    P =  Coords[0] * UmMenosT * UmMenosT + Coords[1] * 2 * UmMenosT * t + Coords[2] * t*t;
    return P;
}
// **********************************************************************
//
// **********************************************************************
double Bezier::CalculaT(double distanciaPercorrida)
{
    return (distanciaPercorrida/ComprimentoTotalDaCurva);
}
// **********************************************************************
//
// **********************************************************************
Ponto Bezier::getPC(int i)
{
    return Coords[i];
}
// **********************************************************************
void Bezier::Traca()
{
    double t=0.0;
    double DeltaT = 1.0/50;
    Ponto P;
    //cout << "DeltaT: " << DeltaT << endl;
    defineCor(Yellow);
    glBegin(GL_LINE_STRIP);

    while(t<1.0)
    {
        P = Calcula(t);
        //P.imprime("P: ");
        glVertex2f(P.x, P.y);
        t += DeltaT;
    }
    P = Calcula(1.0); // faz o fechamento da curva
    glVertex2f(P.x, P.y);
    glEnd();
}
// **********************************************************************
//
// **********************************************************************
void Bezier::TracaPoligonoDeControle()
{
    defineCor(Brown);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<3;i++)
        glVertex3f(Coords[i].x,Coords[i].y,Coords[i].z);
    glEnd();

}
// **********************************************************************
//
// **********************************************************************
Ponto Bezier::getDerivada(double t, Ponto p0, Ponto p1, Ponto p2)
{
    double t1 = 1.0 - t;
    double x = (p1.x - p0.x) * 2.0 * t1 + (p2.x - p1.x) * 2.0 * t;
    double y = (p1.y - p0.y) * 2.0 * t1 + (p2.y - p1.y) * 2.0 * t;
    return Ponto(x, y);
}
// **********************************************************************
//
// **********************************************************************
bool clicouEmPC(Ponto pontoClicado, Ponto pontoCurva) {

    // Calcula a distância entre os pontos pontoClicado e pontoCurva
    double distP1P2 = calculaDistancia(pontoClicado, pontoCurva);

    // Verifica se a distância entre os pontos é relativamente pequena
    return abs(distP1P2) <= 1.0;
}
// **********************************************************************
//
// **********************************************************************
void Bezier::setPC(int i, double x, double y)
{
    Coords[i].x = x;
    Coords[i].y = y;
}
// **********************************************************************
//
// **********************************************************************
ModoCurva Bezier::getModo()
{
    return modo;
}
// **********************************************************************
//
// **********************************************************************
void Bezier::setModo()
{
    if (modo == POSICAO)
    {
        modo = DERIVADA;
    }
    else if (modo == DERIVADA)
    {
        modo = POSICAO;
    }
}
