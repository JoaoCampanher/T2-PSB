#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

int getIndex(int i, int j, int width);
void separeTree(QuadNode *node, int minError);
int *generateHistogram(int x, int y, int width, int height);
int getGrayTone(int r, int g, int b);
int getZoneError(int histogram[], int x, int y, int width, int height);
int averageIntensity(int histogram[]);
int getRedColor(int row, int column);
int getGreenColor(int row, int column);
int getBlueColor(int row, int column);
int getAverageColor(int x, int y, int width, int height, int (*getColor)(int, int));
QuadNode *createQuadNode(int x, int y, int width, int height);

unsigned int first = 1;
char desenhaBorda = 1;
int totalWidth;
int totalHeight;
RGBPixel *pixels;

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
    RGBPixel(*pixelsRGB)[pic->width] = (RGBPixel(*)[pic->height])pic->img;

    totalWidth = pic->width;
    totalHeight = pic->height;
    pixels = &pixelsRGB[0][0];
    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    // grayTonesImage();
    QuadNode *raiz = newNode(0, 0, totalWidth, totalHeight);
    raiz->status = PARCIAL;
    separeTree(raiz, minError);

    return raiz;
}

int *generateHistogram(int x, int y, int width, int height)
{
    int *histogram = malloc(256 * sizeof(int));

    for (int i = 0; i < 256; i++)
    {
        histogram[i] = 0;
    }

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            int index = getIndex(row, column, totalWidth);
            int intensity = getGrayTone(pixels[index].r, pixels[index].g, pixels[index].b);
            histogram[intensity]++;
        }
    }

    return histogram;
}

int getGrayTone(int r, int g, int b)
{
    return round(0.3 * r + 0.59 * g + 0.11 * b);
}

int getAverageColor(int x, int y, int width, int height, int (*getColor)(int, int))
{
    unsigned long intensitySum = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            intensitySum += getColor(row, column);
        }
    }
    return intensitySum / (width * height);
}

int getRedColor(int row, int column)
{
    return pixels[getIndex(row, column, totalWidth)].r;
}

int getGreenColor(int row, int column)
{
    return pixels[getIndex(row, column, totalWidth)].g;
}

int getBlueColor(int row, int column)
{
    return pixels[getIndex(row, column, totalWidth)].b;
}

int getZoneError(int histogram[], int x, int y, int width, int height)
{
    int averageI = averageIntensity(histogram);
    int grayTone;
    unsigned long sum = 0;
    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            int index = getIndex(row, column, totalWidth);
            grayTone = getGrayTone(pixels[index].r, pixels[index].g, pixels[index].b);
            sum += pow((grayTone - averageI), 2);
        }
    }

    return sum / (width * height);
}

int averageIntensity(int histogram[])
{
    unsigned long sum = 0;
    int quantity = 0;
    for (int i = 0; i < 256; i++)
    {
        sum += histogram[i] * i;
        quantity += histogram[i];
    }

    return sum / quantity;
}

int getIndex(int row, int column, int width)
{
    return row * width + column;
}

void separeTree(QuadNode *node, int minError)
{
    int x = node->x;
    int y = node->y;

    int width = node->width;
    int height = node->height;

    int *histogram = generateHistogram(x, y, width, height);

    int error = getZoneError(histogram, x, y, width, height);

    int halfW = width / 2;  // half Width
    int halfH = height / 2; // half Height

    if (error > minError && halfW > 0 && halfW > 0)
    {
        int offsetW = width % 2;  // width Offset
        int offsetH = height % 2; // width Height

        node->NW = createQuadNode(x, y, halfW + 1, halfH + 1);
        node->NE = createQuadNode(x + halfW, y, halfW + offsetW, halfH + 1);
        node->SW = createQuadNode(x, y + halfH, halfW + 1, halfH + offsetH);
        node->SE = createQuadNode(x + halfW, y + halfH, halfW + offsetW, halfH + offsetH);

        if (halfH == 1 || halfW == 1)
        {
            node->NW->status = CHEIO;
            node->NE->status = CHEIO;
            node->SW->status = CHEIO;
            node->SE->status = CHEIO;
            return;
        }

        separeTree(node->NW, minError);
        separeTree(node->NE, minError);
        separeTree(node->SW, minError);
        separeTree(node->SE, minError);
    }
    else
    {
        node->status = CHEIO;
    }

    free(histogram);
}

QuadNode *createQuadNode(int x, int y, int width, int height)
{
    QuadNode *node = newNode(x, y, width, height);
    int averageRed = getAverageColor(x, y, width, height, getRedColor);
    int averageGreen = getAverageColor(x, y, width, height, getGreenColor);
    int averageBlue = getAverageColor(x, y, width, height, getBlueColor);
    node->status = PARCIAL;
    node->color[0] = averageRed;
    node->color[1] = averageGreen;
    node->color[2] = averageBlue;

    return node;
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
            GLubyte blue[] = {0, 0, 255};
            glBegin(GL_LINE_LOOP);
            glColor3ubv(blue);
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
