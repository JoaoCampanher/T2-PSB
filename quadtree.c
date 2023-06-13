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
int getAverageRedColor(int x, int y, int width, int height);
int getAverageBlueColor(int x, int y, int width, int height);
int getAverageGreenColor(int x, int y, int width, int height);
int getZoneError(int histogram[], int x, int y, int width, int height);
int averageIntensity(int histogram[]);

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

int getAverageRedColor(int x, int y, int width, int height)
{
    long intensitySum = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            intensitySum += pixels[getIndex(row, column, totalWidth)].r;
        }
    }
    return intensitySum / (width * height);
}

int getAverageBlueColor(int x, int y, int width, int height)
{
    long intensitySum = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            intensitySum += pixels[getIndex(row, column, totalWidth)].b;
        }
    }
    return intensitySum / (width * height);
}

int getAverageGreenColor(int x, int y, int width, int height)
{
    long intensitySum = 0;

    for (int row = y; row < y + height; row++)
    {
        for (int column = x; column < x + width; column++)
        {
            intensitySum += pixels[getIndex(row, column, totalWidth)].g;
        }
    }
    return intensitySum / (width * height);
}

int getZoneError(int histogram[], int x, int y, int width, int height)
{
    int averageI = averageIntensity(histogram);
    int grayTone;
    long long sum = 0;
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
    long long sum = 0;
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
    int averageRed = getAverageRedColor(node->x, node->y, node->width, node->height);
    int averageBlue = getAverageBlueColor(node->x, node->y, node->width, node->height);
    int averageGreen = getAverageGreenColor(node->x, node->y, node->width, node->height);

    int *histogram = generateHistogram(node->x, node->y, node->width, node->height);

    int error = getZoneError(histogram, node->x, node->y, node->width, node->height);

    if (error > minError)
    {

        int halfWidth = (node->width) / 2;
        int halfHeight = (node->height) / 2;
        int x = node->x;
        int y = node->y;
        // printf("\n%d ", error);
        // printf("%d ", halfWidth);
        // printf("%d ", halfHeight);
        // printf("%d ", x);
        // printf("%d ", y);

        QuadNode *nw = newNode(x, y, halfWidth, halfHeight);
        averageRed = getAverageRedColor(x, y, halfWidth, halfHeight);
        averageGreen = getAverageGreenColor(x, y, halfWidth, halfHeight);
        averageBlue = getAverageBlueColor(x, y, halfWidth, halfHeight);
        nw->status = PARCIAL;
        nw->color[0] = averageRed;
        nw->color[1] = averageGreen;
        nw->color[2] = averageBlue;

        QuadNode *ne = newNode(x + halfWidth, y, halfWidth, halfHeight);
        averageRed = getAverageRedColor(x + halfWidth, y, halfWidth, halfHeight);
        averageGreen = getAverageGreenColor(x + halfWidth, y, halfWidth, halfHeight);
        averageBlue = getAverageBlueColor(x + halfWidth, y, halfWidth, halfHeight);
        ne->status = PARCIAL;
        ne->color[0] = averageRed;
        ne->color[1] = averageGreen;
        ne->color[2] = averageBlue;

        QuadNode *sw = newNode(x, y + halfHeight, halfWidth, halfHeight);
        averageRed = getAverageRedColor(x, y + halfHeight, halfWidth, halfHeight);
        averageGreen = getAverageGreenColor(x, y + halfHeight, halfWidth, halfHeight);
        averageBlue = getAverageBlueColor(x, y + halfHeight, halfWidth, halfHeight);
        sw->status = PARCIAL;
        sw->color[0] = averageRed;
        sw->color[1] = averageGreen;
        sw->color[2] = averageBlue;

        QuadNode *se = newNode(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        averageRed = getAverageRedColor(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        averageGreen = getAverageGreenColor(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        averageBlue = getAverageBlueColor(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        se->status = PARCIAL;
        se->color[0] = averageRed;
        se->color[1] = averageGreen;
        se->color[2] = averageBlue;

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
    free(histogram);
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
