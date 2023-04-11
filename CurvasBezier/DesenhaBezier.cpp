// **********************************************************************
// PUCRS/Escola Polit�cnica
// COMPUTA��O GR�FICA
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

Ponto PosAtualDoMouse;
bool BotaoDown = false;

double nFrames=0;
double TempoTotal=0;

enum Modo {SEM_CONTINUIDADE, CONT_POSICAO, CONT_DERIVADA};
Modo modoAtual = SEM_CONTINUIDADE;

// Vari�veis para armazenar as dimens�es dos bot�es
int buttonWidth = 200;
int buttonHeight = 50;

// Vari�veis para armazenar as posi��es dos bot�es
int button1X = 100;
int button1Y = 100;

int button2X = 100;
int button2Y = 200;

int button3X = 100;
int button3Y = 300;

//Botoes
string buttonTexts[] = {"Continuidade derivada", "Continuidade posicao", "Sem continuidade"};
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
            Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2]);
            nCurvas++;
            break;

        case CONT_POSICAO:
            if (nCurvas == 0) {
                // Cria a primeira curva a partir dos dois primeiros pontos clicados
                Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2]);
                nCurvas++;
            } else {
                // Cria uma nova curva a partir do �ltimo ponto da �ltima curva e do novo ponto clicado
                Ponto p0 = Curvas[nCurvas-1].getPC(2);
                Curvas[nCurvas] = Bezier(p0, PontosClicados[1], PontosClicados[2]);
                nCurvas++;
            }
            break;

        case CONT_DERIVADA:
            if (nCurvas == 0) {
                // Cria a primeira curva a partir dos dois primeiros pontos clicados
                Curvas[nCurvas] = Bezier(PontosClicados[0], PontosClicados[1], PontosClicados[2]);
                nCurvas++;
            } else {
                // Cria uma nova curva a partir do �ltimo ponto da �ltima curva e do novo ponto clicado
                Ponto p0 = Curvas[nCurvas-1].getPC(2);
                Ponto p1 = PontosClicados[1];
                Ponto p2 = PontosClicados[2];

                // Define a posi��o do ponto 2 da curva anterior como sendo o mesmo que o ponto 1 da nova curva
                Curvas[nCurvas-1].setPonto(2, p1.x, p1.y);

                // Cria a nova curva
                Ponto dp0 = Curvas[nCurvas-1].getDerivada(1.0, p0, Curvas[nCurvas-1].getPC(1), p1);
                Ponto dp1 = Curvas[nCurvas].getDerivada(0.0, p0, p1, p2);
                Curvas[nCurvas] = Bezier(p1, p1 + (dp0 * (1.0/3.0)), p2 - (dp1 * (1.0/3.0)));
                nCurvas++;
            }
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
    defineCor(Red);
    glPointSize(4);
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

    // Define os limites l�gicos da �rea OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenham os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	glLineWidth(1);
	glColor3f(1,1,1); // R, G, B  [0..1]

    DesenhaEixos();
    DesenhaMenu();

    glLineWidth(3);
    glColor3f(1,0,0);
    DesenhaPontos();
    DesenhaCurvas();
    ImprimeMensagens();

	glutSwapBuffers();
}

// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo n�mero de segundos e informa quanto frames
// se passaram neste per�odo.
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
// Este codigo � baseado em http://hamala.se/forums/viewtopic.php?t=20
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

    PontosClicados[nPontoAtual++] = ConvertePonto(Ponto(x,y,0));
    if (nPontoAtual==3)
    {
        nPontoAtual = 0;
        CriaCurvas();
    }
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
// fun��o para desenhar o bot�o na janela
// **********************************************************************
void DesenhaBotao(int x, int y, int width, int height, string text) {

    // Define a cor de fundo do bot�o (preto)
    glColor3f(0.0f, 0.0f, 0.0f);

    // desenha o quadrado em volta do bot�o
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + buttonWidth, y);
    glVertex2f(x + buttonWidth, y + buttonHeight);
    glVertex2f(x, y + buttonHeight);
    glEnd();

    // desenha o texto do bot�o
    glColor3f(1.0, 1.0, 1.0); // define a cor branca para o texto do bot�o
    glRasterPos2f(x + 10, y + 20);
    for (int i = 0; i < text.length(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// **********************************************************************
// Callbacks da janela de �cones
// **********************************************************************
void display_icons() {

    // Limpa o buffer de cores
    glClear(GL_COLOR_BUFFER_BIT);

    // Define a cor de fundo para a janela de �cones
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

    // Define a matriz de proje��o
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 250, 0, 500);

    // Define a matriz de modelo/vis�o
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // calcula as coordenadas dos bot�es
    int window_width = glutGet(GLUT_WINDOW_WIDTH); // largura da janela de �cones
    int window_height = glutGet(GLUT_WINDOW_HEIGHT); // altura da janela de �cones
    int button_spacing = (window_height - n_buttons * buttonHeight) / (n_buttons + 1); // espa�amento entre os bot�es
    int button_x = (window_width - buttonWidth) / 2; // posi��o x do bot�o central
    int button_y = button_spacing; // posi��o y do primeiro bot�o

    // desenha os bot�es
    for (int i = 0; i < n_buttons; i++) {
        DesenhaBotao(button_x, button_y, buttonWidth, buttonHeight, buttonTexts[i]);
        button_y += buttonHeight + button_spacing; // atualiza a posi��o y para o pr�ximo bot�o
    }

    // Troca os buffers de v�deo
    glutSwapBuffers();
}

void keyboard_icons(unsigned char key, int x, int y)
{
    // C�digo para lidar com eventos de teclado na janela de �cones
    // ...
}

void arrow_keys_icons(int key, int x, int y)
{
    // C�digo para lidar com eventos de teclas especiais na janela de �cones
    // ...
}

// **********************************************************************
// Fun��o callback do mouse
// **********************************************************************
void mouse_icons(int button, int state, int x, int y)
{
    int window_width = glutGet(GLUT_WINDOW_WIDTH); // largura da janela de �cones
    int window_height = glutGet(GLUT_WINDOW_HEIGHT); // altura da janela de �cones
    int button_spacing = (window_height - n_buttons * buttonHeight) / (n_buttons + 1); // espa�amento entre os bot�es
    int button_x = (window_width - buttonWidth) / 2;
    int button1_y = button_spacing;
    int button2_y = button1_y + (buttonHeight + button_spacing);
    int button3_y = button2_y + (buttonHeight + button_spacing);

    // Verifica se o bot�o esquerdo do mouse foi pressionado
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Verifica se o clique foi dentro do bot�o 1
        if (x >= button_x && x <= button_x + buttonWidth &&
            y >= button1_y && y <= button1_y + buttonHeight)
        {
            modoAtual = SEM_CONTINUIDADE;
            printf("Modo sem continuidade selecionado\n");
        }

        // Verifica se o clique foi dentro do bot�o 2
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button2_y && y <= button2_y + buttonHeight)
        {
            modoAtual = CONT_POSICAO;
            printf("Modo com continuidade de posicao selecionado\n");
        }

        // Verifica se o clique foi dentro do bot�o 3
        else if (x >= button_x && x <= button_x + buttonWidth &&
                 y >= button3_y && y <= button3_y + buttonHeight)
        {
            modoAtual = CONT_DERIVADA;
            printf("Modo com continuidade de derivada selecionado\n");
        }
    }
}

void motion_icons(int x, int y)
{
    // C�digo para lidar com eventos de movimento do mouse na janela de �cones
    // ...
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
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
    // que aparecera na barra de t�tulo da janela.
    glutCreateWindow    ( "Animacao com Bezier" );

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // ser� chamada automaticamente quando
    // for necess�rio redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalida��o da tela. A funcao "display"
    // ser� chamada automaticamente sempre que a
    // m�quina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // ser� chamada automaticamente quando
    // o usu�rio alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // ser� chamada automaticamente sempre
    // o usu�rio pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" ser� chamada
    // automaticamente sempre o usu�rio
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);

    // Cria a janela de icones
    glutInitWindowPosition(glutGet(GLUT_WINDOW_X) + glutGet(GLUT_WINDOW_WIDTH), 0);
    glutInitWindowSize(250, 500);
    glutCreateWindow("Icones");

    // Registra as fun��es de callback para a janela de �cones
    glutDisplayFunc(display_icons);
    glutKeyboardFunc(keyboard_icons);
    glutSpecialFunc(arrow_keys_icons);
    glutMouseFunc(mouse_icons);
    glutMotionFunc(motion_icons);

    // executa algumas inicializa��es
    init();

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
