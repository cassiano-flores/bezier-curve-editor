// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <string>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <GL/glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "Bezier.h"
#include "Temporizador.h"
#include "ListaDeCoresRGB.h"

Temporizador T;
double AccumDeltaT=0;
Temporizador T2;

Bezier Curvas[20];
unsigned int nCurvas;
Ponto PontosClicados[3];
int nPontoAtual=0;

Poligono PoligonoDeControle;

// Limites logicos da area de desenho
Ponto Min, Max;

bool desenha = false;

Poligono Mapa, MeiaSeta, Mastro;

float angulo=0.0;

Ponto PosAtualDoMouse, posInicialMouse, posFinalMouse;
bool BotaoDown = false;
bool ExibicaoDesenho = true;

bool ModoConexaoCurva = false;
bool ConexaoCurva = false;
Ponto PontoConexaoCurva;

double nFrames=0;
double TempoTotal=0;

enum Modo {ATUALIZACAO_CONTINUIDADE, MOVIMENTACAO_VERTICES,
            REMOVER_CURVA, CONT_DERIVADA, CONT_POSICAO, SEM_CONTINUIDADE};
Modo modoAtual = SEM_CONTINUIDADE;

// Variaveis para armazenar as dimensoes dos botoes
int buttonWidth = 250;
int buttonHeight = 50;

//Botoes
string buttonTexts[] = {"Exibicao desenhos", "Atualizacao continuidade", "Conexao curva existente",
                        "Movimentacao de vertices", "Remover curva", "Continuidade derivada",
                        "Continuidade posicao", "Sem continuidade"};
int intModo = 7;
int n_buttons = sizeof(buttonTexts) / sizeof(buttonTexts[0]);

// **********************************************************************
// Imprime o texto S na posicao (x,y), com a cor 'cor'
// **********************************************************************
void printString(string S, int x, int y, int cor)
{
    defineCor(cor);
    glRasterPos3f(x, y, 0); //define posicao na tela
    for (int i = 0; i < S.length(); i++)
    {
        // GLUT_BITMAP_HELVETICA_10
        // GLUT_BITMAP_TIMES_ROMAN_24
        // GLUT_BITMAP_HELVETICA_18
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, S[i]);
    }
}
#include <sstream>
using std::stringstream;

// **********************************************************************
// Converte um nro em string
// **********************************************************************
string toString(float f)
{
    stringstream S;
    S << f;
    return S.str();
}

// **********************************************************************
// Imprime as coordenadas do ponto P na posicao (x,y), com a cor 'cor'
// **********************************************************************
void ImprimePonto(Ponto P, int x, int y, int cor)
{
    string S;
    S = "( " + toString(P.x) + ", " + toString(P.y) + ")" ;
    printString(S, x, y, cor);
}

// **********************************************************************
//  Vetor de mensagens
// **********************************************************************
string Mensagens[] = {
    "Clique o primeiro ponto.",
    "Clique o segundo ponto.",
    "Clique o terceiro ponto."
};

// **********************************************************************
//  Imprime as mensagens do programa.
//  Funcao chamada na 'display'
// **********************************************************************
void ImprimeMensagens()
{
    printString(Mensagens[nPontoAtual], -14, 13, Yellow);
    if (nPontoAtual>0)
    {
        printString("Ultimo ponto clicado: ",-14, 11, Red);
        ImprimePonto(PontosClicados[nPontoAtual-1], -3,11, Red);
    }
}

// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0/30) // fixa a atualizacao da tela em 30
    {
        AccumDeltaT = 0;
        angulo+=2;
        glutPostRedisplay();
    }
    /*
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }*/
}

// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
// **********************************************************************
void reshape( int w, int h )
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x,Max.x, Min.y,Max.y, -10,+10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// **********************************************************************
//
// **********************************************************************
void DesenhaEixos()
{
    Ponto Meio;
    Meio.x = (Max.x+Min.x)/2;
    Meio.y = (Max.y+Min.y)/2;
    Meio.z = (Max.z+Min.z)/2;

    glBegin(GL_LINES);
    //  eixo horizontal
        glVertex2f(Min.x,Meio.y);
        glVertex2f(Max.x,Meio.y);
    //  eixo vertical
        glVertex2f(Meio.x,Min.y);
        glVertex2f(Meio.x,Max.y);
    glEnd();
}

// **********************************************************************
//
// **********************************************************************
void CarregaModelos()
{
    //Mapa.LePoligono("EstadoRS.txt");
    //MeiaSeta.LePoligono("MeiaSeta.txt");
    //Mastro.LePoligono("Mastro.txt");
}

// **********************************************************************
//
// **********************************************************************
void CriaCurvas()
{
    switch (modoAtual) {

        case SEM_CONTINUIDADE:
            Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2], NORMAL);
            nCurvas++;
            break;

        case CONT_POSICAO:
            if (nCurvas == 1) {
                // Cria a primeira curva a partir dos dois primeiros pontos clicados
                Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2], POSICAO);
                nCurvas++;
            } else {
                if (ConexaoCurva)
                {
                    Curvas[nCurvas] = Bezier(PontoConexaoCurva, PontosClicados[0], PontosClicados[1], POSICAO);
                    nCurvas++;
                    ConexaoCurva = false;

                } else {
                    // Cria uma nova curva a partir do ultimo ponto da ultima curva e do novo ponto clicado
                    Ponto p0 = Curvas[nCurvas-1].getPC(2);
                    Curvas[nCurvas] = Bezier(p0, PontosClicados[0], PontosClicados[1], POSICAO);
                    nCurvas++;
                }
            }
            break;

        case CONT_DERIVADA:
            if (nCurvas == 1) {
                // Cria a primeira curva a partir dos dois primeiros pontos clicados
                Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2], DERIVADA);
                nCurvas++;
            } else {
                if (ConexaoCurva)
                {
                    Ponto p0 = PontoConexaoCurva;
                    Ponto p1 = PontosClicados[0];
                    Ponto p2 = PontosClicados[1];

                    Ponto dp0 = Curvas[nCurvas-1].getDerivada(1.0, p0, Curvas[nCurvas-1].getPC(1), p0);
                    Ponto dp1 = Curvas[nCurvas].getDerivada(0.0, p0, p1, p2);
                    Curvas[nCurvas] = Bezier(p0, p1 + (dp0 * (1.0/3.0)), p2 - (dp1 * (1.0/3.0)), DERIVADA);
                    nCurvas++;
                    ConexaoCurva = false;

                } else {
                    // Cria uma nova curva a partir do ultimo ponto da ultima curva e do novo ponto clicado
                    Ponto p0 = Curvas[nCurvas-1].getPC(2);
                    Ponto p1 = PontosClicados[0];
                    Ponto p2 = PontosClicados[1];

                    // Cria as derivadas e a nova curva
                    Ponto dp0 = Curvas[nCurvas-1].getDerivada(1.0, p0, Curvas[nCurvas-1].getPC(1), p0);
                    Ponto dp1 = Curvas[nCurvas].getDerivada(0.0, p0, p1, p2);
                    Curvas[nCurvas] = Bezier(p0, p1 + (dp0 * (1.0/3.0)), p2 - (dp1 * (1.0/3.0)), DERIVADA);
                    nCurvas++;
                }
            }
            break;

        case REMOVER_CURVA:
            // Percorre todas as curvas existentes para verificar se a aresta clicada pertence a alguma delas
            for (int i = 0; i < nCurvas; i++) {
                Bezier curvaAtual = Curvas[i];

                // Percorre todas as arestas da curva para verificar se a aresta clicada pertence a ela
                for (int j = 0; j < 3; j++) {
                    if (j < 2)
                    {
                        if (estaSobreAresta(curvaAtual.getPC(j), curvaAtual.getPC(j+1), PontosClicados[0]))
                        {
                            // Remove a curva e seus pontos de controle do vetor de Curvas
                            for (int k = i; k < nCurvas-1; k++) {
                                Curvas[k] = Curvas[k+1];
                            }
                            nCurvas--;
                            break;
                        }
                    } else {
                        if (estaSobreAresta(curvaAtual.getPC(j), curvaAtual.getPC(0), PontosClicados[0]))
                        {
                            // Remove a curva e seus pontos de controle do vetor de Curvas
                            for (int k = i; k < nCurvas-1; k++) {
                                Curvas[k] = Curvas[k+1];
                            }
                            nCurvas--;
                            break;
                        }
                    }
                }
            }
            break;

        default:
            break;
    }
}

// **********************************************************************
//
// **********************************************************************
void init()
{
    // Define a cor do fundo da tela (AZUL)
    glClearColor(0.0f, 0.0f, 0.f, 1.0f);

    CarregaModelos();

    CriaCurvas();

    float d = 15;
    Min = Ponto(-d,-d);
    Max = Ponto(d,d);
}

// **********************************************************************
//
// **********************************************************************
void DesenhaLinha(Ponto P1, Ponto P2)
{
    defineCor(Brown);
    glBegin(GL_LINES);
        glVertex2f(P1.x,P1.y);
        glVertex2f(P2.x,P2.y);
    glEnd();
}

// **********************************************************************
//
// **********************************************************************
void DesenhaCurvas()
{
    for(int i=0; i<nCurvas;i++)
    {
        defineCor(Yellow);
        Curvas[i].Traca();
        defineCor(Brown);
        Curvas[i].TracaPoligonoDeControle();
    }
}

// **********************************************************************
//
// **********************************************************************
void DesenhaPontos()
{
    defineCor(Orange);
    glPointSize(6);
    glBegin(GL_POINTS);
    for(int i=0; i<nPontoAtual;i++)
    {
        glVertex2f(PontosClicados[i].x, PontosClicados[i].y);
    }
    glEnd();
    glPointSize(1);
}

// **********************************************************************
//
// **********************************************************************
void DesenhaMenu()
{
    glPushMatrix();
        //glTranslated(11,13,0); // veja o arquivo MeiaSeta.txt
        //MeiaSeta.desenhaPoligono();
    glPopMatrix();
}

// **********************************************************************
//  void display( void )
// **********************************************************************
void display( void )
{
    // Limpa a tela coma cor de fundo
    glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites logicos da area OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Rubber-band
    Bezier CurvaProj;
    if (ExibicaoDesenho) {
        if (((modoAtual == SEM_CONTINUIDADE) || (nCurvas == 1)))
        {
            switch (nPontoAtual) {

                case 0:
                    break;

                case 1:
                    DesenhaLinha(PontosClicados[nPontoAtual-1], PosAtualDoMouse);
                    break;

                case 2:
                    CurvaProj = Bezier(PontosClicados[nPontoAtual-2], PontosClicados[nPontoAtual-1], PosAtualDoMouse, NORMAL);
                    defineCor(Red);
                    CurvaProj.Traca();
                    defineCor(Red);
                    CurvaProj.TracaPoligonoDeControle();
                    break;

                default:
                    break;
            }
        }

        else if ((modoAtual == CONT_POSICAO) || (modoAtual == CONT_DERIVADA)) {
            if (ModoConexaoCurva)
            {
                if (ConexaoCurva)
                {
                    switch (nPontoAtual) {

                        case 0:
                            DesenhaLinha(PontoConexaoCurva, PosAtualDoMouse);
                            break;

                        case 1:
                            CurvaProj = Bezier(PontoConexaoCurva, PontosClicados[nPontoAtual-1], PosAtualDoMouse, NORMAL);
                            defineCor(Red);
                            CurvaProj.Traca();
                            defineCor(Red);
                            CurvaProj.TracaPoligonoDeControle();
                            break;

                        default:
                            break;
                    }
                }
            } else {
                switch (nPontoAtual) {

                    case 0:
                        DesenhaLinha(Curvas[nCurvas-1].getPC(2), PosAtualDoMouse);
                        break;

                    case 1:
                        CurvaProj = Bezier(Curvas[nCurvas-1].getPC(2), PontosClicados[nPontoAtual-1], PosAtualDoMouse, NORMAL);
                        CurvaProj.Traca();
                        CurvaProj.TracaPoligonoDeControle();
                        break;

                    default:
                        break;
                }
            }
        }
    }

    glLineWidth(1);
    glColor3f(1,1,1); // R, G, B  [0..1]

    DesenhaEixos();
    DesenhaMenu();

    glLineWidth(3);
    glColor3f(1,0,0);

    if (ExibicaoDesenho)
    {
        DesenhaPontos();
        DesenhaCurvas();
    }

    ImprimeMensagens();

    glutSwapBuffers();
}

// **********************************************************************
// ContaTempo(double tempo)
// conta um certo numero de segundos e informa quanto frames
// se passaram neste periodo.
// **********************************************************************
void ContaTempo(double tempo)
{
    Temporizador T;

    unsigned long cont = 0;
    cout << "Inicio contagem de " << tempo << "segundos ..." << flush;
    while(true)
    {
        tempo -= T.getDeltaT();
        cont++;
        if (tempo <= 0.0)
        {
            cout << "fim! - Passaram-se " << cont << " frames." << endl;
            break;
        }
    }
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case 27:        // Termina o programa qdo
            exit ( 0 );   // a tecla ESC for pressionada
            break;
        case 't':
            ContaTempo(3);
            break;
        case ' ':
            desenha = !desenha;
        break;
        default:
            break;
    }
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
    switch ( a_keys )
    {
        case GLUT_KEY_LEFT:

            break;
        case GLUT_KEY_RIGHT:

            break;
        case GLUT_KEY_UP:       // Se pressionar UP
            glutFullScreen ( ); // Vai para Full Screen
            break;
        case GLUT_KEY_DOWN:     // Se pressionar UP
                                // Reposiciona a janela
            glutPositionWindow (50,50);
            glutReshapeWindow ( 700, 500 );
            break;
        default:
            break;
    }
}

// **********************************************************************
// Converte as coordenadas do ponto P de coordenadas de tela para
// coordenadas de universo (sistema de referencias definidas na glOrtho
// (ver funcao reshape)
// Este codigo eh baseado em http://hamala.se/forums/viewtopic.php?t=20
// **********************************************************************
Ponto ConvertePonto(Ponto P)
{
    GLint viewport[4];
    GLdouble modelview[16],projection[16];
    GLfloat wx=P.x,wy,wz;
    GLdouble ox=0.0,oy=0.0,oz=0.0;
    glGetIntegerv(GL_VIEWPORT,viewport);
    P.y=viewport[3]-P.y;
    wy=P.y;
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    glReadPixels(P.x,P.y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&wz);
    gluUnProject(wx,wy,wz,modelview,projection,viewport,&ox,&oy,&oz);
    return Ponto(ox,oy,oz);
}

// **********************************************************************
// Captura o clique do botao esquerdo do mouse sobre a area de
// desenho
// **********************************************************************
void Mouse(int button,int state,int x,int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        posInicialMouse = posFinalMouse = Ponto(x, glutGet(GLUT_WINDOW_HEIGHT) - y, 0);
    }

    if (modoAtual == MOVIMENTACAO_VERTICES) {
        // Botao esquerdo do mouse pressionado
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            Ponto P(x,y);
            posInicialMouse = ConvertePonto(P);

            // Percorre todas as curvas existentes para verificar se o ponto clicado pertence a alguma delas
            for (int i = 0; i < nCurvas; i++) {
                Bezier curvaAtual = Curvas[i];

                // Percorre todos os pontos de controle da curva para verificar qual foi clicado
                for (int j = 0; j < 3; j++) {

                    if (clicouEmPC(posInicialMouse, curvaAtual.getPC(j)))
                    {
                        // Armazena o ponto de controle clicado
                        posInicialMouse = curvaAtual.getPC(j);
                    }
                }
            }
        }
        // Botao esquerdo do mouse liberado
        else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            Ponto P(x,y);
            posFinalMouse = ConvertePonto(P);

            // Atualiza a posicao do ponto de controle clicado
            for (int i = 0; i < nCurvas; i++) {
                Bezier& curvaAtual = Curvas[i];

                // Percorre todos os pontos de controle da curva para verificar qual foi clicado
                for (int j = 0; j < 3; j++) {
                    if (clicouEmPC(posInicialMouse, curvaAtual.getPC(j)))
                    {
                        // Atualiza a posicao do ponto de controle clicado
                        curvaAtual.setPC(j, posFinalMouse.x, posFinalMouse.y);

                        // Verifica se a curva anterior tambem precisa ser atualizada
                        if (i > 0 && j == 0)
                        {
                            Bezier& curvaAnterior = Curvas[i-1];
                            if ((curvaAnterior.getPC(0) == posInicialMouse) || (curvaAnterior.getPC(1) == posInicialMouse) ||
                                 (curvaAnterior.getPC(2) == posInicialMouse))
                            {
                                curvaAnterior.setPC(2, posFinalMouse.x, posFinalMouse.y);
                            }
                        }

                        // Verifica se a proxima curva tambem precisa ser atualizada
                        if (i < nCurvas-1 && j == 2)
                        {
                            Bezier& proximaCurva = Curvas[i+1];
                            if ((proximaCurva.getPC(0) == posInicialMouse) || (proximaCurva.getPC(1) == posInicialMouse) ||
                                 (proximaCurva.getPC(2) == posInicialMouse))
                            {
                                proximaCurva.setPC(0, posFinalMouse.x, posFinalMouse.y);
                            }
                        }
                    }
                }
            }
        }
    }

    if (ModoConexaoCurva) {
        // Botao esquerdo do mouse pressionado
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            Ponto P(x,y);
            posInicialMouse = ConvertePonto(P);

            // Percorre todas as curvas existentes para verificar se o ponto clicado pertence a alguma delas
            for (int i = 0; i < nCurvas; i++) {
                Bezier curvaAtual = Curvas[i];

                // Percorre todos os pontos de controle da curva para verificar qual foi clicado
                for (int j = 0; j < 3; j++) {

                    if (clicouEmPC(posInicialMouse, curvaAtual.getPC(j)))
                    {
                        // Armazena o ponto de controle clicado
                        PontoConexaoCurva = curvaAtual.getPC(j);
                        ConexaoCurva = true;
                    }
                }
            }
        }
    }

    if (modoAtual == ATUALIZACAO_CONTINUIDADE) {
        vector<Bezier*> CurvasAtualizacao;
        int count = 0;

        // Botão esquerdo do mouse pressionado
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            Ponto P(x, y);
            posInicialMouse = ConvertePonto(P);

            // Percorre todas as curvas existentes para verificar se o ponto clicado pertence a alguma delas
            bool sair = false;
            for (int i = 0; i < nCurvas && !sair; i++) {
                Bezier* curvaAtual = &Curvas[i];

                // Percorre todos os pontos de controle da curva para verificar qual foi clicado
                for (int j = 0; j < 3 && !sair; j++) {
                    if (clicouEmPC(posInicialMouse, curvaAtual->getPC(j))) {
                        // Armazena a curva que contém o ponto clicado
                        CurvasAtualizacao.push_back(curvaAtual);
                        count++;

                        if (count == 2) {
                            // Altera o modo de continuidade de todas as curvas em "CurvasAtualizacao"
                            Bezier* primeiraCurva = CurvasAtualizacao[0];
                            Bezier* segundaCurva = CurvasAtualizacao[1];
                            Ponto p0 = primeiraCurva->getPC(2);
                            Ponto p1 = primeiraCurva->getPC(1);
                            Ponto p2 = primeiraCurva->getPC(0);

                            int indexPC = -1;
                            if (p1 == posInicialMouse) {
                                indexPC = 1;
                            } else if (p2 == posInicialMouse) {
                                indexPC = 0;
                            }

                            if ((primeiraCurva->getModo() == POSICAO) || (segundaCurva->getModo() == POSICAO))  {
                                // Ajusta os pontos de controle existentes para serem condizentes com o modo "DERIVADA"
                                Ponto dp1 = primeiraCurva->getDerivada(1.0, p0, p1, p2);
                                Ponto dp2 = segundaCurva->getDerivada(0.0, p0, p1, p2);
                                p1 = p1 - (dp1 * (1.0 / 3.0));
                                primeiraCurva->setPC(indexPC, p1.x, p1.y);
                                p2 = p2 + (dp2 * (1.0 / 3.0));
                                primeiraCurva->setPC(indexPC == 0 ? 1 : 0, p2.x, p2.y);

                            } else if ((primeiraCurva->getModo() == DERIVADA) || (segundaCurva->getModo() == DERIVADA)) {
                                // Ajusta os pontos de controle existentes para serem condizentes com o modo "POSICAO"
                                p1 = ((p1 - p0) * (1.0/3.0)) + p0;
                                primeiraCurva->setPC(indexPC, p1.x, p1.y);
                                p2 = ((p2 - p0) * (1.0/3.0)) + p0;
                                primeiraCurva->setPC(indexPC == 0 ? 1 : 0, p2.x, p2.y);
                            }

                            primeiraCurva->setModo();
                            segundaCurva->setModo();
                            sair = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    //cout << "Mouse..." << endl;

    if(state==GLUT_DOWN && button==GLUT_LEFT_BUTTON)
    {
        cout << "Botao ESQ Desceu" << endl;
    }
    if(state==GLUT_UP && button==GLUT_LEFT_BUTTON)
    {
        cout << "Botao ESQ Subiu" << endl;
    }
    if(state==GLUT_UP)
      return;
    if(button==GLUT_RIGHT_BUTTON)
     return;

    if ((!ModoConexaoCurva) || (ConexaoCurva && (!clicouEmPC(PontoConexaoCurva, ConvertePonto(Ponto(x,y,0))))))
    {
        PontosClicados[nPontoAtual++] = ConvertePonto(Ponto(x,y,0));
    }

    if ((modoAtual == SEM_CONTINUIDADE) || (nCurvas == 1))
    {
        if (nPontoAtual==3)
        {
            nPontoAtual = 0;
            CriaCurvas();
        }
    }

    else if ((modoAtual == CONT_POSICAO) || (modoAtual == CONT_DERIVADA) || (ModoConexaoCurva))
    {
        if (nPontoAtual==2)
        {
            nPontoAtual = 0;
            CriaCurvas();
        }
    }

    else if ((modoAtual == REMOVER_CURVA) || (modoAtual == ATUALIZACAO_CONTINUIDADE))
    {
        if (nPontoAtual==1)
        {
            nPontoAtual = 0;
            CriaCurvas();
        }
    }

    else if ((modoAtual == MOVIMENTACAO_VERTICES) || (modoAtual == ATUALIZACAO_CONTINUIDADE))
    {
        nPontoAtual = 0;
        CriaCurvas();
    }
}

// **********************************************************************
// Sempre captura as coordenadas do mouse sobre a area de desenho
// **********************************************************************
void PassiveMotion(int x, int y)
{
    Ponto P(x,y);
    PosAtualDoMouse = ConvertePonto(P);

    glutPostRedisplay();
}

// **********************************************************************
// Captura as coordenadas do mouse do mouse sobre a area de
// desenho, enquanto um dos botoes esta sendo pressionado
// **********************************************************************
void Motion(int x, int y)
{
    Ponto P(x,y);
    PosAtualDoMouse = ConvertePonto(P);
    PosAtualDoMouse.imprime("Mouse:");
    cout << endl;
}

// **********************************************************************
// funcao para desenhar o botao na janela
// **********************************************************************
void DesenhaBotao(int x, int y, int width, int height, string text, bool status) {

    // Define a cor de fundo do botao (preto)
    glColor3f(0.0f, 0.0f, 0.0f);

    // desenha o quadrado em volta do botao
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + buttonWidth, y);
    glVertex2f(x + buttonWidth, y + buttonHeight);
    glVertex2f(x, y + buttonHeight);
    glEnd();

    // desenha o texto do botao
    if (status) {
        glColor3f(1.0, 1.0, 0);
    } else {
        glColor3f(1.0, 1.0, 1.0);
    }
    glRasterPos2f(x + 10, y + 20);
    for (int i = 0; i < text.length(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// **********************************************************************
// Callbacks da janela de icones
// **********************************************************************
void display_icons() {

    // Limpa o buffer de cores
    glClear(GL_COLOR_BUFFER_BIT);

    // Define a cor de fundo para a janela de icones
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // Define a matriz de projecao
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 300, 0, 500);

    // Define a matriz de modelo/visao
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // calcula as coordenadas dos botoes
    int window_width = glutGet(GLUT_WINDOW_WIDTH); // largura da janela de icones
    int window_height = glutGet(GLUT_WINDOW_HEIGHT); // altura da janela de icones
    int button_spacing = (window_height - n_buttons * buttonHeight) / (n_buttons + 1); // espacamento entre os botoes
    int button_x = (window_width - buttonWidth) / 2; // posicao x do botao central
    int button_y = button_spacing; // posicao y do primeiro botao

    // desenha os botoes
    for (int i = 0; i < n_buttons; i++) {
        if (i == intModo) {
            DesenhaBotao(button_x, button_y, buttonWidth, buttonHeight, buttonTexts[i], true);
        } else {
            DesenhaBotao(button_x, button_y, buttonWidth, buttonHeight, buttonTexts[i], false);
        }

        if ((i == 0) && (ExibicaoDesenho)) {
            DesenhaBotao(button_x, button_y, buttonWidth, buttonHeight, buttonTexts[i], true);
        }

        if ((i == 2) && (ModoConexaoCurva)) {
            DesenhaBotao(button_x, button_y, buttonWidth, buttonHeight, buttonTexts[i], true);
        }
        button_y += buttonHeight + button_spacing; // atualiza a posicao y para o proximo botao
    }

    // Troca os buffers de video
    glutSwapBuffers();
}

void keyboard_icons(unsigned char key, int x, int y)
{
}

void arrow_keys_icons(int key, int x, int y)
{
}

void motion_icons(int x, int y)
{
}

// **********************************************************************
// Funcao callback do mouse
// **********************************************************************
void mouse_icons(int button, int state, int x, int y)
{
    int window_width = glutGet(GLUT_WINDOW_WIDTH); // largura da janela de icones
    int window_height = glutGet(GLUT_WINDOW_HEIGHT); // altura da janela de icones
    int button_spacing = (window_height - n_buttons * buttonHeight) / (n_buttons + 1); // espacamento entre os botoes
    int button_x = (window_width - buttonWidth) / 2;
    int button1_y = button_spacing;
    int button2_y = button1_y + (buttonHeight + button_spacing);
    int button3_y = button2_y + (buttonHeight + button_spacing);
    int button4_y = button3_y + (buttonHeight + button_spacing);
    int button5_y = button4_y + (buttonHeight + button_spacing);
    int button6_y = button5_y + (buttonHeight + button_spacing);
    int button7_y = button6_y + (buttonHeight + button_spacing);
    int button8_y = button7_y + (buttonHeight + button_spacing);

    // Verifica se o botao esquerdo do mouse foi pressionado
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Verifica se o clique foi dentro do botao 1
        if (x >= button_x && x <= button_x + buttonWidth &&
            y >= button1_y && y <= button1_y + buttonHeight)
        {
            ModoConexaoCurva = false;
            modoAtual = SEM_CONTINUIDADE;
            intModo = 7;
            printf("Modo sem continuidade selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 2
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button2_y && y <= button2_y + buttonHeight)
        {
            modoAtual = CONT_POSICAO;
            intModo = 6;
            printf("Modo com continuidade de posicao selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 3
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button3_y && y <= button3_y + buttonHeight)
        {
            modoAtual = CONT_DERIVADA;
            intModo = 5;
            printf("Modo com continuidade de derivada selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 4
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button4_y && y <= button4_y + buttonHeight)
        {
            ModoConexaoCurva = false;
            modoAtual = REMOVER_CURVA;
            intModo = 4;
            printf("Edicao remocao de curvas selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 5
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button5_y && y <= button5_y + buttonHeight)
        {
            ModoConexaoCurva = false;
            modoAtual = MOVIMENTACAO_VERTICES;
            intModo = 3;
            printf("Edicao movimentacao de vertices selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 6
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button6_y && y <= button6_y + buttonHeight)
        {
            if ((modoAtual == CONT_POSICAO) || (modoAtual == CONT_DERIVADA))
            {
                if (ModoConexaoCurva)
                {
                    ModoConexaoCurva = false;
                    printf("Edicao conexao com uma curva ja existente desativado\n");
                } else {
                    ModoConexaoCurva = true;
                    printf("Edicao conexao com uma curva ja existente ativado\n");
                }
            } else {
                printf("Edicao conexao com uma curva ja existente requer o modo CONTINUIDADE POSICAO ou CONTINUIDADE DERIVADA\n");
            }
        }

        // Verifica se o clique foi dentro do botao 7
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button7_y && y <= button7_y + buttonHeight)
        {
            ModoConexaoCurva = false;
            modoAtual = ATUALIZACAO_CONTINUIDADE;
            intModo = 1;
            printf("Edicao atualizacao do modo de continuidade entre duas curvas selecionado\n");
        }

        // Verifica se o clique foi dentro do botao 8
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button8_y && y <= button8_y + buttonHeight)
        {
            if (ExibicaoDesenho)
            {
                ExibicaoDesenho = false;
                printf("Exibicao das curvas e poligonos de controle desativado\n");
            } else {
                ExibicaoDesenho = true;
                printf("Exibicao das curvas e poligonos de controle ativado\n");
            }
        }
    }
}

// **********************************************************************
//  void main ( int argc, char** argv )
// **********************************************************************
int main ( int argc, char** argv )
{
    cout << "Programa OpenGL" << endl;

    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de titulo da janela.
    glutCreateWindow    ( "Animacao com Bezier" );

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // sera chamada automaticamente quando
    // for necessario redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalidacao da tela. A funcao "display"
    // sera chamada automaticamente sempre que a
    // maquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // sera chamada automaticamente quando
    // o usuario alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // sera chamada automaticamente sempre
    // o usuario pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" sera chamada
    // automaticamente sempre o usuario
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );
    glutMouseFunc(Mouse);
    glutPassiveMotionFunc(PassiveMotion);
    glutMotionFunc(Motion);

    // Cria a janela de icones
    glutInitWindowPosition(glutGet(GLUT_WINDOW_X) + glutGet(GLUT_WINDOW_WIDTH), 0);
    glutInitWindowSize(300, 500);
    glutCreateWindow("Icones");

    // Registra as funcoes de callback para a janela de icones
    glutDisplayFunc(display_icons);
    glutKeyboardFunc(keyboard_icons);
    glutSpecialFunc(arrow_keys_icons);
    glutMouseFunc(mouse_icons);
    glutMotionFunc(motion_icons);

    // executa algumas inicializacoes
    init();

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
