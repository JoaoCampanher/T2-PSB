#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

void grayTonesImage();
float getMediumIntensity(float x, float y, float width, float height);
int getIndex(int i, int j, int width);
float getZoneError(float x, float y, float width, float height);
void separeTree(QuadNode *node, int minError);

unsigned int first = 1;
char desenhaBorda = 1;
int totalWidth;
int totalHeight;
RGBPixel *pixelsRGB;

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

    totalWidth = pic->width;
    totalHeight = pic->height;
    pixelsRGB = &pixels[0][0];
    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    grayTonesImage();
    QuadNode *raiz = newNode(0, 0, totalWidth, totalHeight);
    raiz->status = PARCIAL;
    separeTree(raiz, minError);

    return raiz;
}

void grayTonesImage()
{
    for (int row = 0; row < totalHeight; row++)
    {
        for (int column = 0; column < totalWidth; column++)
        {
            int index = getIndex(row, column, totalWidth);
            int intensity = round(0.3 * pixelsRGB[index].r + 0.59 * pixelsRGB[index].g + 0.11 * pixelsRGB[index].b);
            pixelsRGB[index].r = intensity;
            pixelsRGB[index].g = intensity;
            pixelsRGB[index].b = intensity;
        }
    }
}

float getMediumIntensity(float x, float y, float width, float height)
{
    long intensitySum = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            intensitySum += pixelsRGB[getIndex(row, column, totalWidth)].r;
        }
    }
    return intensitySum / (width * height);
}

float getZoneError(float x, float y, float width, float height)
{
    int mediumIntensity = getMediumIntensity(x, y, width, height);
    float summation = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            summation += pow((pixelsRGB[getIndex(row, column, totalWidth)].r - mediumIntensity), 2) / (width * height);
        }
    }
    float result = sqrt(summation);
    return result;
}

int getIndex(int row, int column, int width)
{
    return row * width + column;
}

void separeTree(QuadNode *node, int minError)
{
    if (getZoneError(node->x, node->y, node->width, node->height) > minError)
    {
        float halfWidth = (node->width) / 2;
        float halfHeight = (node->height) / 2;
        float x = node->x;
        float y = node->y;

        QuadNode *nw = newNode(x, y, halfWidth, halfHeight);
        int nwIntensity = getMediumIntensity(x, y, halfWidth, halfHeight);
        nw->status = PARCIAL;
        nw->color[0] = nwIntensity;
        nw->color[1] = nwIntensity;
        nw->color[2] = nwIntensity;

        QuadNode *ne = newNode(x + halfWidth, y, halfWidth, halfHeight);
        int neIntensity = getMediumIntensity(x + halfWidth, y, halfWidth, halfHeight);
        ne->status = PARCIAL;
        ne->color[0] = neIntensity;
        ne->color[1] = neIntensity;
        ne->color[2] = neIntensity;

        QuadNode *sw = newNode(x, y + halfHeight, halfWidth, halfHeight);
        int swIntensity = getMediumIntensity(x, y + halfHeight, halfWidth, halfHeight);
        sw->status = PARCIAL;
        sw->color[0] = swIntensity;
        sw->color[1] = swIntensity;
        sw->color[2] = swIntensity;

        QuadNode *se = newNode(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        int seIntensity = getMediumIntensity(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        se->status = PARCIAL;
        se->color[0] = seIntensity;
        se->color[1] = seIntensity;
        se->color[2] = seIntensity;

        node->NW = nw;
        node->NE = ne;
        node->SW = sw;
        node->SE = se;

        separeTree(node->NW, minError);
        separeTree(node->NE, minError);
        separeTree(node->SW, minError);
        separeTree(node->SE, minError);
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
