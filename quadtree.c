#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

void grayTonesImage(int width, int height, RGBPixel *pixels);
float getMediumIntensity(float x, float y, float width, float height, RGBPixel *pixels);
int getIndex(int i, int j, int width);
float getZoneError(float x, float y, float width, float height, RGBPixel *pixels);
void separeTree(QuadNode *node, RGBPixel *pixels, int minError);

unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *newNode(int x, int y, int width, int height)
{
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

QuadNode *geraQuadtree(Img *pic, float minError)
{
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel(*pixels)[pic->width] = (RGBPixel(*)[pic->height])pic->img;

    int width = pic->width;
    int height = pic->height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    grayTonesImage(width, height, &pixels[0][0]);
    QuadNode *raiz = newNode(0, 0, width, height);
    raiz->status = PARCIAL;

    printf("%f", getMediumIntensity(width/2, 0, width/2, height/2, &pixels[0][0]));
    separeTree(raiz, &pixels[0][0], minError);
    return raiz;
}

void grayTonesImage(int width, int height, RGBPixel *pixels)
{
    for (int j = 0; j < width; j++)
    {
        for (int i = 0; i < height; i++)
        {
            int intensity = round(0.3 * pixels[getIndex(i, j, width)].r + 0.59 * pixels[getIndex(i, j, width)].g + 0.11 * pixels[getIndex(i, j, width)].b);
            pixels[getIndex(i, j, width)].r = intensity;
            pixels[getIndex(i, j, width)].g = intensity;
            pixels[getIndex(i, j, width)].b = intensity;
        }
    }
}

float getMediumIntensity(float x, float y, float width, float height, RGBPixel *pixels)
{
    long intensitySum = 0;

    for (int j = x; j < x + width; j++)
    {
        for (int i = y; i < y + height; i++)
        {
            intensitySum += pixels[getIndex(i, j, width)].r;
        }
    }
    return intensitySum / (width * height);
}

float getZoneError(float x, float y, float width, float height, RGBPixel *pixels)
{
    int mediumIntensity = getMediumIntensity(x, y, width, height, pixels);
    float summation = 0;

    for (int j = x; j < x + width; j++)
    {
        for (int i = y; i < y + height; i++)
        {
            summation += pow((pixels[getIndex(i, j, width)].r - mediumIntensity), 2) / (width * height);
        }
    }
    float result = sqrt(summation);
    return result;
}

int getIndex(int i, int j, int width)
{
    return i * width + j;
}


void separeTree(QuadNode *node, RGBPixel *pixels, int minError)
{
    if (getZoneError(node->x, node->y, node->width, node->height, pixels) > minError )
    {
        float halfWidth = (node->width) / 2;
        float halfHeight = (node->height) / 2;
        float x = node->x;
        float y = node->y;

        QuadNode *nw = newNode(x, y, halfWidth, halfHeight);
        int nwIntensity = getMediumIntensity(x, y, halfWidth, halfHeight, pixels);
        nw->status = PARCIAL;
        nw->color[0] = nwIntensity;
        nw->color[1] = nwIntensity;
        nw->color[2] = nwIntensity;

        QuadNode *ne = newNode(x + halfWidth, y, halfWidth, halfHeight);
        int neIntensity = getMediumIntensity(x + halfWidth, y, halfWidth, halfHeight, pixels);
        ne->status = PARCIAL;
        ne->color[0] = neIntensity;
        ne->color[1] = neIntensity;
        ne->color[2] = neIntensity;

        QuadNode *sw = newNode(x, y + halfHeight, halfWidth, halfHeight);
        int swIntensity = getMediumIntensity(x, y + halfHeight, halfWidth, halfHeight, pixels);
        sw->status = PARCIAL;
        sw->color[0] = swIntensity;
        sw->color[1] = swIntensity;
        sw->color[2] = swIntensity;

        QuadNode *se = newNode(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        int seIntensity = getMediumIntensity(x + halfWidth, y + halfHeight, halfWidth, halfHeight, pixels);
        se->status = PARCIAL;
        se->color[0] = seIntensity;
        se->color[1] = seIntensity;
        se->color[2] = seIntensity;

        node->NW = nw;
        node->NE = ne;
        node->SW = sw;
        node->SE = se;

        separeTree(node->NW, pixels, minError);
        separeTree(node->NE, pixels, minError);
        separeTree(node->SW, pixels, minError);
        separeTree(node->SE, pixels, minError);
    }
    else
    {
        node->status = CHEIO;
    }
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if (n == NULL)
        return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    // printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz)
{
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n)
{
    if (n == NULL)
        return;

    if (n->NE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if (n == NULL)
        return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
