#include "alfe/main.h"
#include "alfe/vectors.h"
#include "alfe/user.h"
#include "alfe/bitmap.h"
#include "alfe/cga.h"
#include "alfe/fix.h"

typedef Fixed<7, Word> UFix8p8;
typedef Fixed<7, Int16> SFix8p8;

typedef Vector3<SFix8p8> Point3;

Byte characters[] = {
    0x00,  // ........
    0x10,  // *.......
    0x5c,  // **......
    0x62,  // ***.....
    0x4c,  // ****....
    0x44,  // *****...
    0x35,  // ******..
    0x45,  // *******.

    0x00,  // ........
    0x00,  // ........
    0x00,  // .x......
    0x27,  // .**.....
    0x6c,  // .***....
    0x32,  // .****...
    0x30,  // .*****..
    0x01,  // .******.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ..x.....
    0x21,  // ..**....
    0x05,  // ..***...
    0x0c,  // ..****..
    0x15,  // ..*****.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x04,  // ...*....
    0x0f,  // ...**...
    0x34,  // ...***..
    0x4a,  // ...****.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ....x...
    0x6a,  // ....**..
    0xf4,  // ....***.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // .....x..
    0x2f,  // .....**.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x11,  // ......*.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

Byte characters2[] = {
    0x00,  // ........
    0x10,  // *.......
    0x5c,  // **......
    0x62,  // ***.....
    0x4c,  // ****....
    0x44,  // *****...
    0x35,  // ******..
    0x45,  // *******.

    0x00,  // ........
    0x00,  // ........
    0x10,  // *.......
    0x27,  // .**.....
    0x6c,  // .***....
    0x32,  // .****...
    0x30,  // .*****..
    0x01,  // .******.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x04,  // ...*....
    0x21,  // ..**....
    0x05,  // ..***...
    0x0c,  // ..****..
    0x15,  // ..*****.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x04,  // ...*....
    0x0f,  // ...**...
    0x34,  // ...***..
    0x4a,  // ...****.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x04,  // ...*....
    0x6a,  // ....**..
    0xf4,  // ....***.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x11,  // ......*.
    0x2f,  // .....**.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x11,  // ......*.

    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};


struct Face
{
    Face() { }
    Face(int colour, std::initializer_list<int> vertices)
    {
        _colour = colour;
        _nVertices = vertices.size();
        _vertices.allocate(_nVertices);
        _vertex0 = &_vertices[0];
        for (int i = 0; i < _nVertices; ++i)
            _vertex0[i] = vertices.begin()[i];
    }
    int _colour;
    int* _vertex0;
    int _nVertices;
private:
    Array<int> _vertices;
};

struct Shape
{
    Shape(std::initializer_list<Point3> vertices, float scale,
        std::initializer_list<Face> faces)
    {
        _nVertices = vertices.size();
        _vertices.allocate(_nVertices);
        _vertex0 = &_vertices[0];
        float distance = (392.0 / 165.0)*(5.0 / 12.0);
        scale = distance/(scale * sqrt(1 + distance*distance));
        for (int i = 0; i < _nVertices; ++i) {
            _vertex0[i] = vertices.begin()[i] * scale;
            _vertex0[i].x = SFix8p8::fromRepresentation(adjust(_vertex0[i].x.representation()));
            _vertex0[i].y = SFix8p8::fromRepresentation(adjust(_vertex0[i].y.representation()));
            _vertex0[i].z = SFix8p8::fromRepresentation(adjust(_vertex0[i].z.representation()));
        }

        _nFaces = faces.size();
        _faces.allocate(_nFaces);
        _face0 = &_faces[0];
        for (int i = 0; i < _nFaces; ++i)
            _face0[i] = faces.begin()[i];
    }
    Point3* _vertex0;
    int _nVertices;
    Face* _face0;
    int _nFaces;
private:
    Array<Point3> _vertices;
    Array<Face> _faces;

    int adjust(int r)
    {
        if (r < 0)
            return -adjust(-r);
        switch (r) {
            case 0: return 0;
            case 66: return 66;
            case 67: return 66;
            case 97: return 98;
            case 107: return 107;
            case 156: return 158;
            case 157: return 158;
            case 173: return 174;
            case 174: return 174;
            case 186: return 186;
            default:
                printf("%i\n",r);
                return r;
        }
    }
};

Byte colours[] = {
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0a,
    0x0b,
    0x0c,
    0x0d,
    0x0e,
    0x0f};

static const float phi = (sqrt(5.0f) + 1)/2;

Shape shapes[] {
    {{{   -1,    -1,    -1},  // Cube
      {   -1,    -1,     1},
      {   -1,     1,    -1},
      {   -1,     1,     1},
      {    1,    -1,    -1},
      {    1,    -1,     1},
      {    1,     1,    -1},
      {    1,     1,     1}},
      sqrt(3.0f),
     {{ 9, { 0,  4,  6,  2}},
      {10, { 4,  5,  7,  6}},
      {11, { 5,  1,  3,  7}},
      {12, { 1,  0,  2,  3}},
      {13, { 2,  6,  7,  3}},
      {14, { 0,  1,  5,  4}}}},

    {{{    1,     0,     0},  // Octahedron
      {   -1,     0,     0},
      {    0,     1,     0},
      {    0,    -1,     0},
      {    0,     0,     1},
      {    0,     0,    -1}},
      1,
     {{1, { 4,  2,  0}},
      {2, { 5,  0,  2}},
      {3, { 4,  0,  3}},
      {4, { 5,  3,  0}},
      {5, { 4,  1,  2}},
      {6, { 5,  2,  1}},
      {7, { 4,  3,  1}},
      {8, { 5,  1,  3}}}},

    {{{    1,     1,     1},  // Tetrahedron
      {    1,    -1,    -1},
      {   -1,     1,    -1},
      {   -1,    -1,     1}},
      sqrt(3.0f),
     {{1, { 1,  2,  3}},
      {2, { 0,  3,  2}},
      {4, { 3,  0,  1}},
      {8, { 2,  1,  0}}}},

    {{{  phi,     1,     0},  // Icosahedron
      { -phi,     1,     0},
      {  phi,    -1,     0},
      { -phi,    -1,     0},
      {    1,     0,   phi},
      {    1,     0,  -phi},
      {   -1,     0,   phi},
      {   -1,     0,  -phi},
      {    0,   phi,     1},
      {    0,  -phi,     1},
      {    0,   phi,    -1},
      {    0,  -phi,    -1}},
      sqrt(phi*phi + 1)*1.01f,
     {{1, { 4,  8,  0}},
      {2, {10,  5,  0}},
      {3, { 9,  4,  2}},
      {4, { 5, 11,  2}},
      {5, { 8,  6,  1}},
      {6, { 7, 10,  1}},
      {7, { 6,  9,  3}},
      {6, {11,  7,  3}},
      {9, { 8, 10,  0}},
      {10, {10,  8,  1}},
      {11, {11,  9,  2}},
      {12, { 9, 11,  3}},
      {13, { 0,  2,  4}},
      {14, { 2,  0,  5}},
      {15, { 3,  1,  6}},
      {1, { 1,  3,  7}},
      {2, { 4,  6,  8}},
      {3, { 6,  4,  9}},
      {4, { 7,  5, 10}},
      {5, { 5,  7, 11}}}},

    {{{    1,     1,     1},  // Dodecahedron
      {    1,     1,    -1},
      {    1,    -1,     1},
      {    1,    -1,    -1},
      {   -1,     1,     1},
      {   -1,     1,    -1},
      {   -1,    -1,     1},
      {   -1,    -1,    -1},
      {phi-1,   phi,     0},
      {1-phi,   phi,     0},
      {phi-1,  -phi,     0},
      {1-phi,  -phi,     0},
      {  phi,     0, phi-1},
      {  phi,     0, 1-phi},
      { -phi,     0, phi-1},
      { -phi,     0, 1-phi},
      {    0, phi-1,   phi},
      {    0, 1-phi,   phi},
      {    0, phi-1,  -phi},
      {    0, 1-phi,  -phi}},
      sqrt(3.0f),
     {{ 1, {13, 12,  0,  8,  1}},
      { 2, {14, 15,  5,  9,  4}},
      { 3, {12, 13,  3, 10,  2}},
      { 4, {15, 14,  6, 11,  7}},
      { 5, {17, 16,  0, 12,  2}},
      { 6, {18, 19,  3, 13,  1}},
      { 9, {16, 17,  6, 14,  4}},
      {10, {19, 18,  5, 15,  7}},
      {11, { 9,  8,  0, 16,  4}},
      {12, {10, 11,  6, 17,  2}},
      {13, { 8,  9,  5, 18,  1}},
      {14, {11, 10,  3, 19,  7}}}}};


typedef Fixed<16, Int32> Fix16p16;
typedef Fixed<7, Int32> Fix24p8;
typedef Vector2<UFix8p8> Point2;

class SineTable
{
public:
    SineTable()
    {
        for (int i = 0; i < 2560; ++i) {
            double s = ::sin(i*tau/2048.0);
            _table[i] = s;
            _halfTable[i] = s/2.0;
        }
    }
    SFix8p8 sin(int a) { return _table[a]; }
    SFix8p8 cos(int a) { return _table[a + 512]; }
    SFix8p8 coscos(int a, int b)
    {
        return _halfTable[(a+b & 0x7ff) + 512] +
            _halfTable[(a-b & 0x7ff) + 512];
    }
    SFix8p8 sinsin(int a, int b)
    {
        return _halfTable[(a-b & 0x7ff) + 512] -
            _halfTable[(a+b & 0x7ff) + 512];
    }
    SFix8p8 sincos(int a, int b)
    {
        return _halfTable[a+b & 0x7ff] + _halfTable[a-b & 0x7ff];
    }
    SFix8p8 cossin(int a, int b)
    {
        return _halfTable[a+b & 0x7ff] - _halfTable[a-b & 0x7ff];
    }
private:
    SFix8p8 _table[2560];
    SFix8p8 _halfTable[2560];
};

struct TransformedPoint
{
    Vector3<Fix16p16> _xyz;
    Vector2<Fix24p8> _xy;
    SFix8p8 _z;
};

SineTable sines;

class Projection
{
public:
    void init(int theta, int phi, SFix8p8 distance, Vector3<SFix8p8> scale,
        Vector2<SFix8p8> offset)
    {
        Vector3<SFix8p8> s(scale.x*distance, scale.y*distance, scale.z);
        //s.x = SFix8p8::fromRepresentation(0x7f5c);
        //s.y = SFix8p8::fromRepresentation(0x3511);
        _xx = s.x*sines.sin(theta);
        _xy = -s.y*(sines.cos(theta)*sines.cos(phi)); //sines.coscos(theta, phi);
        _xz = -s.z*(sines.cos(theta)*sines.sin(phi)); //sines.cossin(theta, phi);
        _yx = s.x*sines.cos(theta);
        _yy = s.y*(sines.sin(theta)*sines.cos(phi)); //sines.sincos(theta, phi);
        _yz = s.z*(sines.sin(theta)*sines.sin(phi)); //sines.sinsin(theta, phi);
        _zy = s.y*sines.sin(phi);
        _zz = -s.z*sines.cos(phi);
        _distance = distance;
        _offset = offset;
    }
    TransformedPoint modelToScreen(Point3 model)
    {
        TransformedPoint r;
        r._xyz.x = lmul(_xx, model.x) + lmul(_yx, model.y);
            /*+ lmul(_zx, model.z)*/
        r._xyz.y = lmul(_xy, model.x) + lmul(_yy, model.y) +
            lmul(_zy, model.z);
        r._xyz.z = lmul(_xz, model.x) + lmul(_yz, model.y) +
            lmul(_zz, model.z);
        Int16 d = ((r._xyz.z.representation() >> 8) + _distance.representation() + 0xa00)/5;
        r._z = SFix8p8::fromRepresentation(d);
        r._xy = Vector2<Fix24p8>(
            Fix24p8::fromRepresentation((r._xyz.x.representation() / d +
                _offset.x.representation()) & 0xffffffff),
            Fix24p8::fromRepresentation((r._xyz.y.representation() / d +
                _offset.y.representation()) & 0xffffffff));
        return r;
    }
private:
    SFix8p8 _distance;
    Vector2<SFix8p8> _offset;
    SFix8p8 _xx;
    SFix8p8 _xy;
    SFix8p8 _xz;
    SFix8p8 _yx;
    SFix8p8 _yy;
    SFix8p8 _yz;
    //SFix8p8 _zx;
    SFix8p8 _zy;
    SFix8p8 _zz;
};

//int globalCount = 0;
//
class SpanBuffer
{
public:
    SpanBuffer()
    {
        _lines.allocate(200);
        _lines0 = &_lines[0];
    }
    void clear()
    {
        for (int y = 0; y < 200; ++y)
            _lines0[y].clear();
    }
    void addSpan(int c, int xL, int xR, int y)
    {
        _lines0[y].addSpan(colours[c], xL, xR);
    }
    void renderDeltas(Byte* vram, SpanBuffer* last)
    {
        for (int y = 0; y < 165; ++y) {
            _lines[y].renderDeltas(vram, &last->_lines0[y]);
            vram += 49*2;
        }
    }
private:
    class Line
    {
    public:
        Line()
        {
            _spans.allocate(64);
            _s = &_spans[0];
            _s[0]._x = 0;
            clear();
        }
        void clear()
        {
            // Can remove this first line if we know we're going to be painting
            // the entire screen.
            _s[0]._c = 0;
            _s[1]._x = 392;
            _n = 1;
        }
        void addSpan(int c, int xL, int xR)
        {
            //if (xL < 0 || xR > 392)
            //    printf("Error 1!\n");
            if (xL >= xR)
                return;
            int i;
            for (i = 1; i < _n; ++i)
                if (xL < _s[i]._x)
                    break;
            int j;
            for (j = i; j < _n; ++j)
                if (xR < _s[j]._x)
                    break;
            --i;
            --j;
            if (c == _s[i]._c)
                xL = _s[i]._x;
            else
                if (i > 0 && xL == _s[i]._x && c == _s[i - 1]._c) {
                    --i;
                    xL = _s[i]._x;
                }
            if (c == _s[j]._c)
                xR = _s[j + 1]._x;
            else
                if (j < _n - 1 && xR == _s[j + 1]._x && c == _s[j + 1]._c) {
                    ++j;
                    xR = _s[j + 1]._x;
                }
            int o = j - i;
            if (xL == _s[i]._x) {
                // Left of new span at left of left old
                if (xR == _s[j + 1]._x) {
                    // Right of new span at right of right old
                    _s[i]._c = c;
                    _n -= o;
                    for (int k = i + 1; k <= _n; ++k)
                        _s[k] = _s[k + o];
                }
                else {
                    --o;
                    _n -= o;
                    switch (o) {
                        case -1:
                            // Have to do the update after the move or we'd
                            // be stomping on data we need to keep.
                            for (int k = _n; k >= i - o; --k)
                                _s[k] = _s[k + o];
                            _s[i]._c = c;
                            _s[i + 1]._x = xR;
                            break;
                        case 0:
                            _s[i]._c = c;
                            _s[i + 1]._x = xR;
                            break;
                        default:
                            _s[i]._c = c;
                            _s[i + 1]._x = xR;
                            _s[i + 1]._c = _s[i + 1 + o]._c;
                            for (int k = i + 2; k <= _n; ++k)
                                _s[k] = _s[k + o];
                            break;
                    }
                }
                if (_s[_n]._x != 392 || _s[_n - 1]._c != 0)
                    printf("Error 2a\n");
                return;
            }
            if (xR == _s[j + 1]._x) {
                // Right of new span at right of right old
                --o;
                _n -= o;
                switch (o) {
                    case -1:
                        // Untested
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        for (int k = _n; k > i - o; --k)
                            _s[k] = _s[k + o];
                        break;
                    case 0:
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        break;
                    default:
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        for (int k = i + 2; k <= _n; ++k)
                            _s[k] = _s[k + o];
                        break;
                }
            }
            else {
                o -= 2;
                _n -= o;
                switch (o) {
                    case -2:
                        // Have to do the update after the move
                        for (int k = _n; k > i - o; --k)
                            _s[k] = _s[k + o];
                        _s[i + 2]._c = _s[i]._c;
                        _s[i + 2]._x = xR;
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        break;
                    case -1:
                        // Have to do the update after the move
                        for (int k = _n; k > i - o; --k)
                            _s[k] = _s[k + o];
                        _s[i + 2]._x = xR;
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        break;
                    case 0:
                        _s[i + 2]._x = xR;
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        break;
                    default:
                        // Untested
                        _s[i + 2]._x = xR;
                        _s[i + 2]._c = _s[i + 2 + o]._c;
                        _s[i + 1]._x = xL;
                        _s[i + 1]._c = c;
                        for (int k = i + 3; k <= _n; ++k)
                            _s[k] = _s[k + o];
                }
            }
            if (_s[_n]._x != 392 || _s[_n - 1]._c != 0)
                printf("Error 2\n");
        }
        void renderDeltas(Byte* vram, const Line* o) const
        {
            Span* s = _s;
            int xL, xR;
            int edge1 = -1, edge2 = -1;
            int xLCell = s->_x >> 3;
            int c0 = 0;

            do {
                xL = s->_x;
                int c = s->_c;
                ++s;
                xR = s->_x;
                int xRCell = xR >> 3;
                if (xRCell > xLCell) {
                    if (edge1 != -1) {
                        if (edge2 == -1)
                            vram[xLCell*2] = characters[edge1];
                        else
                            vram[xLCell*2] = characters[edge1*8 + edge2];
                        vram[xLCell*2 + 1] = c0 + c*16;
                        ++xLCell;
                    }
                    while (xLCell < xRCell) {
                        vram[xLCell*2] = 0xdb;
                        vram[xLCell*2 + 1] = c;
                        ++xLCell;
                    }
                    edge1 = -1;
                    edge2 = -1;
                }
                if (edge1 == -1)
                    edge1 = xR & 7;
                else
                    edge2 = xR & 7;
                c0 = c;
            } while (xR < 392);
        }
        void renderDeltas1(Byte* vram, const Line* o) const
        {
            Byte* vram0 = vram;

            const Span* sn = _s;
            const Span* so = o->_s;
            int cn = sn->_c;
            ++sn;
            int xLn = 0;
            int xRn = sn->_x;
            int co = so->_c;
            ++so;
            int xLo = 0;
            int xRo = so->_x;

            int edge1 = -1, edge2 = -1, c0 = 0;
            do {
                if ((xRn & 0x1f8) == (xLn & 0x1f8)) {
                    if (cn != co || edge1 != -1) {
                        //Byte newBits = cn & mask[xLn & 3] & ~mask[xRn & 3];
                        //if (!havePartial) {
                        //    if ((xLn & 3) == 0)
                        //        partial = 0;
                        //    else
                        //        partial = vram[xLn >> 2] & ~mask[xLn & 3];
                        //    havePartial = true;
                        //}
                        //partial |= newBits;
                    }
                    else {
                        if ((xRo & 0x1f8) == (xLn & 0x1f8)) {
                            //partial = ((vram[xLn >> 2] & ~mask[xRo & 3]) | (cn & mask[xRo & 3])) & ~mask[xRn & 3];
                            //havePartial = true;
                        }
                    }
                }
                else {
                    if (cn != co) {
                        //if (!havePartial)
                        //    partial = vram[xLn >> 2] & ~mask[xLn & 3];
                        //vram[xLn >> 3] = partial | (cn & mask[xLn & 3]);
                    }
                    else {
                        //if (havePartial)
                        //    vram[xLn >> 2] = partial | (cn & mask[xLn & 3]);
                        //else {
                        //    if ((xRo & 0xfc) == (xLn & 0xfc)) {
                        //        Byte* p = vram + (xLn >> 2);
                        //        *p = (*p & ~mask[xLn & 3]) | (cn & mask[xLn & 3]);
                        //    }
                        //}
                    }
                    xLn = (xLn + 3) & 0xfc;
                    int storeStart = xLn;
                    int storeEnd = xLn;
                    bool store = false;
                    do {
                        if (store) {
                            if (cn == co) {
                                int aligned = (xLo + 3) & 0xfc;
                                if (xRn >= aligned)
                                    storeEnd = aligned;
                                else
                                    storeEnd = xLo;
                                store = false;
                            }
                        }
                        else {
                            if (cn != co) {
                                if ((xLo & 0xfc) > (storeEnd & 0xfc) + 4) {
                                    if (storeStart < storeEnd) {
                                        int startByte = storeStart >> 2;
                                        memset(vram + startByte, cn, (storeEnd >> 2) - startByte);
                                    }
                                    storeStart = xLo;
                                }
                                store = true;
                            }
                        }
                        if (xRo >= xRn)
                            break;
                        xLo = xRo;
                        co = so->_c;
                        ++so;
                        xRo = so->_x;
                    } while (true);
                    if (store)
                        storeEnd = xRn;
                    if (storeStart < storeEnd) {
                        int startByte = storeStart >> 2;
                        memset(vram + startByte, cn, (storeEnd >> 2) - startByte);
                        //if ((xRn & 3) != 0) {
                        //    partial = cn & ~mask[xRn & 3];
                        //    havePartial = true;
                        //}
                        //else
                        //    havePartial = false;
                    }
                    else {
                        //if (co != cn && (xRn & 3) != 0) {
                        //    partial = cn & ~mask[xRn & 3];
                        //    havePartial = true;
                        //}
                        //else
                        //    havePartial = false;
                    }
                }
                xLn = xRn;
                cn = sn->_c;
                ++sn;
                xRn = sn->_x;
                if (xRo == xLn) {
                    xLo = xRo;
                    co = so->_c;
                    ++so;
                    xRo = so->_x;
                }
            } while (xLn < 0xff);

            Byte vram2[64];
            //renderDeltas0(vram2, o);
            for (int i = 0; i < 64; ++i)
                if (vram0[i] != vram2[i])
                    printf("Error\n");

//            renderDeltas0(vram0, o);
        }
    private:
        struct Span
        {
            Word _x;
            Byte _c;
        };
        Span* _s;
        Array<Span> _spans;
        int _n;
    };
    Line* _lines0;
    Array<Line> _lines;
};

class SpanWindow : public RootWindow
{
public:
    SpanWindow()
      : _wisdom(File("wisdom")), _output(&_data, &_sequencer, &_bitmap),
        _theta(0), _phi(0), _dTheta(3), _dPhi(5), _autoRotate(true), _shape(0)
    {
        _sequencer.setROM(File("5788005.u33", true));

        _output.setConnector(0);          // RGBI
        _output.setScanlineProfile(0);    // rectangle
        _output.setHorizontalProfile(0);  // rectangle
        _output.setScanlineWidth(1);
        _output.setScanlineBleeding(2);   // symmetrical
        _output.setHorizontalBleeding(2); // symmetrical
        _output.setZoom(2);
        _output.setHorizontalRollOff(0);
        _output.setHorizontalLobes(4);
        _output.setVerticalRollOff(0);
        _output.setVerticalLobes(4);
        _output.setSubPixelSeparation(1);
        _output.setPhosphor(0);           // colour
        _output.setMask(0);
        _output.setMaskSize(0);
        _output.setAspectRatio(5.0/6.0);
        _output.setOverscan(0);
        _output.setCombFilter(0);         // no filter
        _output.setHue(0);
        _output.setSaturation(100);
        _output.setContrast(100);
        _output.setBrightness(0);
        _output.setShowClipping(false);
        _output.setChromaBandwidth(1);
        _output.setLumaBandwidth(1);
        _output.setRollOff(0);
        _output.setLobes(1.5);
        _output.setPhase(1);

        static const int regs = -CGAData::registerLogCharactersPerBank;
        Byte cgaRegistersData[regs] = { 0 };
        Byte* cgaRegisters = &cgaRegistersData[regs];
        cgaRegisters[CGAData::registerLogCharactersPerBank] = 12;
        cgaRegisters[CGAData::registerScanlinesRepeat] = 1;
        cgaRegisters[CGAData::registerHorizontalTotalHigh] = 0;
        cgaRegisters[CGAData::registerHorizontalDisplayedHigh] = 0;
        cgaRegisters[CGAData::registerHorizontalSyncPositionHigh] = 0;
        cgaRegisters[CGAData::registerVerticalTotalHigh] = 0;
        cgaRegisters[CGAData::registerVerticalDisplayedHigh] = 0;
        cgaRegisters[CGAData::registerVerticalSyncPositionHigh] = 0;
        cgaRegisters[CGAData::registerMode] = 0x09;
        cgaRegisters[CGAData::registerPalette] = 0x0f;
        cgaRegisters[CGAData::registerHorizontalTotal] = 114 - 1;
        cgaRegisters[CGAData::registerHorizontalDisplayed] = 49;
        cgaRegisters[CGAData::registerHorizontalSyncPosition] = 90;
        cgaRegisters[CGAData::registerHorizontalSyncWidth] = 10;
        cgaRegisters[CGAData::registerVerticalTotal] = 256 - 1;
        cgaRegisters[CGAData::registerVerticalTotalAdjust] = 6;
        cgaRegisters[CGAData::registerVerticalDisplayed] = 165;
        cgaRegisters[CGAData::registerVerticalSyncPosition] = 224;
        cgaRegisters[CGAData::registerInterlaceMode] = 2;
        cgaRegisters[CGAData::registerMaximumScanline] = 0;
        cgaRegisters[CGAData::registerCursorStart] = 6;
        cgaRegisters[CGAData::registerCursorEnd] = 7;
        cgaRegisters[CGAData::registerStartAddressHigh] = 0;
        cgaRegisters[CGAData::registerStartAddressLow] = 0;
        cgaRegisters[CGAData::registerCursorAddressHigh] = 0;
        cgaRegisters[CGAData::registerCursorAddressLow] = 0;
        _data.change(0, -regs, regs, &cgaRegistersData[0]);
        _data.setTotals(238944, 910, 238875);
        _data.change(0, 0, 0x4000, &_vram[0]);

        _outputSize = _output.requiredSize();

        add(&_bitmap);
        add(&_animated);

        _animated.setDrawWindow(this);
        _animated.setRate(60);

        _buffer = 0;
        _buffers[0].clear();
        _buffers[1].clear();

        for (int i = 0; i < sizeof(shapes)/sizeof(shapes[0]); ++i) {
            for (int j = 0; j < shapes[i]._nFaces; ++j) {
                int n = shapes[i]._face0[j]._nVertices;
                _a.ensure(n + 1);
                _b.ensure(n + 1);
                _c.ensure(n + 1);
                _x.ensure(n);
                _y.ensure(n);
                _newX.ensure(n);
                _newY.ensure(n);
                _moved.ensure(n);
            }
        }
        _maxBytesChanged = 0;
        _lastShape = -1;
        _borders = true;
    }
    ~SpanWindow() { _output.join(); }
    void create()
    {
        setText("CGA +HRES Span buffer");
        setInnerSize(_outputSize);
        _bitmap.setTopLeft(Vector(0, 0));
        _bitmap.setInnerSize(_outputSize);
        RootWindow::create();
        _animated.start();
    }
    virtual void draw()
    {
        Projection p;
        _theta = (_theta + _dTheta) & 0x7ff;
        _phi = (_phi + _dPhi) & 0x7ff;
        float zs = 1;
        float ys = 106.0f; //82.5f;   
        float xs = 12*ys/5;
        float distance = (392.0 / 165.0)*(5.0 / 12.0);
        p.init(_theta, _phi, distance, Vector3<float>(xs, ys, zs),
            Vector2<float>(196.0, 82.5));

        Shape* shape = &shapes[_shape];
        _corners.ensure(shape->_nVertices);
        TransformedPoint* corners = &_corners[0];

        for (int i = 0; i < shape->_nVertices; ++i)
            corners[i] = p.modelToScreen(shape->_vertex0[i]);

        //memset(&_vram[0], 0, 0x4000);
        //printf("%i %i ",_theta,_phi);
        Face* face = shape->_face0;
        int nFaces = shape->_nFaces;
        for (int i = 0; i < nFaces; ++i, ++face) {
            int* vertex = face->_vertex0;
            TransformedPoint p0 = corners[vertex[0]];
            TransformedPoint p1 = corners[vertex[1]];
            TransformedPoint p2 = corners[vertex[2]];

            Vector2<SInt16> s0(p0._xy.x.representation() >> 9, p0._xy.y.representation() >> 9);
            Vector2<SInt16> s1(p1._xy.x.representation() >> 9, p1._xy.y.representation() >> 9);
            Vector2<SInt16> s2(p2._xy.x.representation() >> 9, p2._xy.y.representation() >> 9);
            s1 -= s0;
            s2 -= s0;
            if (s1.x*s2.y >= s1.y*s2.x)
                continue;

            float xMin = 392;
            float xMax = 0;
            float yMin = 165;
            float yMax = 0;
            int nVertices = face->_nVertices;
            for (int j = 0; j < nVertices; ++j) {
                TransformedPoint p = corners[*vertex];
                float x = p._xy.x.representation() / 128.0f;
                float y = p._xy.y.representation() / 128.0f;
                _x[j] = x;
                _y[j] = y;
                if (x < xMin)
                    xMin = x;
                if (y < yMin)
                    yMin = y;
                if (x > xMax)
                    xMax = x;
                if (y > yMax)
                    yMax = y;
                ++vertex;
            }
            // _a[1] corresponds to _x[0] and _x[1]
            float lastX = _x[nVertices - 1];
            float lastY = _y[nVertices - 1];
            for (int j = 0; j < nVertices; ++j) {
                float x = _x[j];
                float y = _y[j];
                float a = y - lastY;
                float b = x - lastX;
                float c = y*lastX - x*lastY;
                if (_borders)
                    c += 3.5*sqrt(a*a + b*b/5.76);
                _a[j] = a;
                _b[j] = b;
                _c[j] = c;
                lastX = x;
                lastY = y;
            }
            _a[nVertices] = _a[0];
            _b[nVertices] = _b[0];
            _c[nVertices] = _c[0];

            // _x[0] corresponds to _a[0] and _a[1]
            float lastA = _a[0];
            float lastB = _b[0];
            float lastC = _c[0];
            bool viable = true;
            for (int j = 0; j < nVertices; ++j) {
                float a = _a[j + 1];
                float b = _b[j + 1];
                float c = _c[j + 1];

                float e = a*lastB - lastA*b;
                if (_borders && e > -300 && e < 300) {
                    viable = false;
                    break;
                }
                float d = 1.0f/e;
                float x = (lastB*c - b*lastC)*d;
                float y = (lastA*c - a*lastC)*d;
                //if (x < 0 || y < 0 || x >= 392 || y >= 165) {
                if (_borders && (x < xMin || y < yMin || x > xMax || y > yMax)) {
                    viable = false;
                    break;
                }
                _newX[j] = x;
                _newY[j] = y;
                _moved[j]._xy.x = Fix24p8::fromRepresentation(x*128.0);
                _moved[j]._xy.y = Fix24p8::fromRepresentation(y*128.0);

                //float d = 256.0f/(a*lastB - lastA*b);
                //_moved[j]._xy.x = Fix24p8::fromRepresentation((lastB*c - b*lastC)*d);
                //_moved[j]._xy.y = Fix24p8::fromRepresentation((lastA*c - a*lastC)*d);
                lastA = a;
                lastB = b;
                lastC = c;
            }
            if (!viable)
                continue;

            p0 = _moved[0];
            p1 = _moved[1];
            p2 = _moved[2];
            int c = face->_colour;
            fillTriangle(p0._xy, p1._xy, p2._xy, c);
            for (int j = 3; j < nVertices; ++j) {
                TransformedPoint p3 = _moved[j];
                fillTriangle(p0._xy, p2._xy, p3._xy, c);
                p2 = p3;
            }
        }
        //printf("\n");
        int count = 0;
        for (int i = 0; i < 0x4000; ++i)
            _vramTemp[i] = _vram[i];

        _buffers[_buffer].renderDeltas(&_vram[0], &_buffers[1 - _buffer]);

        for (int i = 0; i < 0x4000; ++i)
            if (_vramTemp[i] != _vram[i])
                ++count;
        if (count > _maxBytesChanged)
            _maxBytesChanged = count;
        if (_lastShape != _shape)
            _maxBytesChanged = 0;
        _lastShape = _shape;
        printf("%i bytes changed, max = %i\n", count, _maxBytesChanged);

        _buffer = 1 - _buffer;
        _buffers[_buffer].clear();
        _data.change(0, 0, 0x4000, &_vram[0]);
        _output.restart();
        _animated.restart();
    }
    bool keyboardEvent(int key, bool up)
    {
        if (up)
            return false;
        switch (key) {
            case VK_RIGHT:
                if (_autoRotate)
                    _dTheta += 1;
                else
                    _theta += 1;
                return true;
            case VK_LEFT:
                if (_autoRotate)
                    _dTheta -= 1;
                else
                    _theta -= 1;
                return true;
            case VK_UP:
                if (_autoRotate)
                    _dPhi -= 1;
                else
                    _phi -= 1;
                return true;
            case VK_DOWN:
                if (_autoRotate)
                    _dPhi += 1;
                else
                    _phi += 1;
                return true;
            case 'N':
                _shape = (_shape + 1) % (sizeof(shapes)/sizeof(shapes[0]));
                return true;
            case 'B':
                _borders = !_borders;
                return true;
            case VK_SPACE:
                _autoRotate = !_autoRotate;
                if (!_autoRotate) {
                    _dTheta = 0;
                    _dPhi = 0;
                }
                return true;
        }
        return false;
    }
private:
    void horizontalLine(int xL, int xR, int y, int c)
    {
        if (y < 0 || y >= 165 || xL < 0 || xR > 392)
             printf("Error 3\n");

        _buffers[_buffer].addSpan(c, xL, xR, y);


        //int l = ((y & 1) << 13) + (y >> 1)*80 + 8;
        //c = colours[c][y & 1];

        //for (int x = xL; x < xR; ++x) {
        //    int a = l + (x >> 2);
        //    int s = (x & 3) << 1;
        //    Byte m = 0xc0 >> s;
        //    _vram[a] = (_vram[a] & ~m) | (c & m);
        //    ++_count;
        //}
    }
    void fillTrapezoid(int yStart, int yEnd, UFix8p8 dL, UFix8p8 dR, int c)
    {
        for (int y = yStart; y < yEnd; ++y) {
            horizontalLine(_xL.intFloor(), _xR.intFloor(), y, c);
            _xL += dL;
            _xR += dR;
        }
    }
    UFix8p8 slopeLeft(UFix8p8 dx, UFix8p8 dy, UFix8p8 x0, UFix8p8 y0,
        UFix8p8* x)
    {
        if (dy < 1) {
            *x = x0 - muld(y0, dx, dy);
            return UFix8p8::fromRepresentation(0xffff);
        }
        else {
            UFix8p8 dxdy = dx/dy;
            *x = x0 - y0*dxdy;
            return dxdy;
        }
    }
    UFix8p8 slopeRight(UFix8p8 dx, UFix8p8 dy, UFix8p8 x0, UFix8p8 y0,
        UFix8p8* x)
    {
        if (dy < 1) {
            *x = x0 + muld(y0, dx, dy);
            return UFix8p8::fromRepresentation(0xffff);
        }
        else {
            UFix8p8 dxdy = dx/dy;
            *x = x0 + y0*dxdy;
            return dxdy;
        }
    }
    UFix8p8 slope(UFix8p8 ux, UFix8p8 vx, UFix8p8 dy, UFix8p8 y0, UFix8p8* x)
    {
        if (ux > vx)
            return slopeRight(ux - vx, dy, vx, y0, x);
        return -slopeLeft(vx - ux, dy, vx, y0, x);
    }
    void fillTriangle(Point2 a, Point2 b, Point2 c, int colour)
    {
        if (a.y > b.y) swap(a, b);
        if (b.y > c.y) swap(b, c);
        if (a.y > b.y) swap(a, b);

        if (a.y == b.y) {
            if (b.y == c.y)
                return;
            if (a.x > b.x)
                swap(a, b);
            int yab = a.y.intCeiling();
            int yc = (c.y + 1).intFloor();
            UFix8p8 yac = c.y - a.y;
            UFix8p8 yaa = yab - a.y;
            fillTrapezoid(yab, yc, slope(c.x, a.x, yac, yaa, &_xL), slope(c.x, b.x, yac, yaa, &_xR), colour);
            return;
        }
        int ya = (a.y + 1).intFloor();
        UFix8p8 yab = b.y - a.y;
        if (b.y == c.y) {
            if (b.x > c.x)
                swap(b, c);
            int ybc = (b.y + 1).intFloor();
            UFix8p8 yaa = ya - a.y;
            fillTrapezoid(ya, ybc, slope(b.x, a.x, yab, yaa, &_xL), slope(c.x, a.x, yab, yaa, &_xR), colour);
            return;
        }

        int yb = (b.y + 1).intFloor();
        int yc = (c.y + 1).intFloor();
        UFix8p8 xb;
        UFix8p8 yaa = ya - a.y;
        UFix8p8 ybb = yb - b.y;
        UFix8p8 yac = c.y - a.y;
        UFix8p8 ybc = c.y - b.y;
        if (b.x > a.x) {
            UFix8p8 dab = slopeRight(b.x - a.x, yab, a.x, yaa, &xb);
            if (c.x > a.x) {
                UFix8p8 xc;
                UFix8p8 dac = slopeRight(c.x - a.x, yac, a.x, yaa, &xc);
                if (dab < dac) {
                    _xL = xb;
                    _xR = xc;
                    fillTrapezoid(ya, yb, dab, dac, colour);
                    fillTrapezoid(yb, yc, slope(c.x, b.x, ybc, ybb, &_xL), dac, colour);
                }
                else {
                    _xL = xc;
                    _xR = xb;
                    fillTrapezoid(ya, yb, dac, dab, colour);
                    fillTrapezoid(yb, yc, dac, slope(c.x, b.x, ybc, ybb, &_xR), colour);
                }
            }
            else {
                UFix8p8 dca = slopeLeft(a.x - c.x, yac, a.x, yaa, &_xL);
                _xR = xb;
                fillTrapezoid(ya, yb, -dca, dab, colour);
                fillTrapezoid(yb, yc, -dca, -slopeLeft(b.x - c.x, ybc, b.x, ybb, &_xR), colour);
            }
        }
        else {
            UFix8p8 dba = slopeLeft(a.x - b.x, yab, a.x, yaa, &xb);
            if (c.x > a.x) {
                UFix8p8 dac = slopeRight(c.x - a.x, yac, a.x, yaa, &_xR);
                _xL = xb;
                fillTrapezoid(ya, yb, -dba, dac, colour);
                fillTrapezoid(yb, yc, slopeRight(c.x - b.x, ybc, b.x, ybb, &_xL), dac, colour);
            }
            else {
                UFix8p8 xc;
                UFix8p8 dca = slopeLeft(a.x - c.x, yac, a.x, yaa, &xc);
                if (dba > dca) {
                    _xL = xb;
                    _xR = xc;
                    fillTrapezoid(ya, yb, -dba, -dca, colour);
                    fillTrapezoid(yb, yc, slope(c.x, b.x, ybc, ybb, &_xL), -dca, colour);
                }
                else {
                    _xL = xc;
                    _xR = xb;
                    fillTrapezoid(ya, yb, -dca, -dba, colour);
                    fillTrapezoid(yb, yc, -dca, slope(c.x, b.x, ybc, ybb, &_xR), colour);
                }
            }
        }
    }

    FFTWWisdom<float> _wisdom;
    CGAData _data;
    CGASequencer _sequencer;
    CGAOutput _output;
    AnimatedWindow _animated;
    BitmapWindow _bitmap;
    int _theta;
    int _phi;
    int _dTheta;
    int _dPhi;
    bool _autoRotate;
    Vector _outputSize;
    Byte _vram[0x4000];
    Byte _vramTemp[0x4000];
    UFix8p8 _xL;
    UFix8p8 _xR;
    bool _fp;
    int _shape;
    int _lastShape;
    Array<TransformedPoint> _corners;
    int _count;
    SpanBuffer _buffers[2];
    int _buffer;
    Array<float> _a;
    Array<float> _b;
    Array<float> _c;
    Array<float> _x;
    Array<float> _y;
    Array<float> _newX;
    Array<float> _newY;
    Array<TransformedPoint> _moved;
    int _maxBytesChanged;
    bool _borders;
};

class Program : public WindowProgram<SpanWindow>
{
};
