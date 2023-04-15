//?#version 460

// real valued functions for easier code generation
float r_zero() { return 0.0; }
float r_conj(float a) { return a; }
float r_realValue(float a) { return a; }
bool r_isReal(float a) { return true; }
float r_neg(float a) { return -a; }
float r_add(float a, float b) { return a + b; }
vec3 r_add3(vec3 a, vec3 b) { return a + b; }
float r_sub(float a, float b) { return a - b; }
vec3 r_sub3(vec3 a, vec3 b) { return a - b; }
float r_mul(float a, float b) { return a * b; }
vec3 r_mul3(vec3 a, float b) { return a * b; }
float r_div(float a, float b) { return a / b; }
vec3 r_div3(vec3 a, float b) { return a / b; }
float r_constant(float val) { return val; }
vec3 r_constant3(vec3 val) { return val; }
float r_variable(float val) { return val; }
vec3 r_variable3(vec3 val) { return val; }

float r_dabs(float a) { return abs(a); }
vec3 r_dabs3(vec3 a) { return abs(a); }
float r_dmin(float a, float b) { return min(a, b); }
vec3 r_dmin3(vec3 v, float a) { return min(v, a); }
float r_dmax(float a, float b) { return max(a, b); }
vec3 r_dmax3(vec3 v, float a) { return max(v, a); }
float r_dclamp(float v, float l, float h) { return clamp(v, l, h); }
float r_dmix(float a, float b, float t) { return mix(a,b,t); }
float r_dsqrt(float a) { return sqrt(a); }
float r_dlength(vec2 a) { return length(a); }
float r_dlength(vec3 a) { return length(a); }

float r_dsin(float a) { return sin(a); }
float r_dcos(float a) { return cos(a); }
float r_ddot(vec3 a, vec3 b) { return dot(a,b); }

#ifdef DERIVATIVES_ENABLED
struct dnum {
    float d[SIZE];
};

struct dnum2 { // only length is implemented
    dnum x;
    dnum y;
};

struct dnum3 {
    dnum x;
    dnum y;
    dnum z;
};

struct dnum4 { // only mat4 multiply is implemented
    dnum x;
    dnum y;
    dnum z;
    dnum w;
};

dnum zero() {
    dnum c;
    for(int i = 0; i < SIZE; ++i) {
        c.d[i] = 0;
    }
    return c;
}

dnum3 asDnum3(dnum4 d) {
    return dnum3(d.x, d.y, d.z);
}

dnum constant(float val) {
    dnum res;
    res.d[0] = val;
    for(int i = 1; i < SIZE; ++i) {
        res.d[i] = 0;
    }
    return res;
}

dnum3 constant3(vec3 val) {
    dnum3 res;
    res.x = constant(val.x);
    res.y = constant(val.y);
    res.z = constant(val.z);
    return res;
}

dnum variable(float val, float dx, float dy, float dz) {
    dnum res;
    res.d[0] = val;
    res.d[1] = dx;
    res.d[2] = dy;
    res.d[3] = dz;
    for(int i = 4; i < SIZE; ++i)
        res.d[i] = 0;
    return res;
}

dnum3 variable3(vec3 point, float dx, float dy, float dz) {
    dnum3 res;
    res.x = variable(point.x, dx, 0.0, 0.0);
    res.y = variable(point.y, 0.0, dy, 0.0);
    res.z = variable(point.z, 0.0, 0.0, dz);
    return res;
}

dnum conj(dnum a) {
    for(int i = 1; i < SIZE; ++i) {
        a.d[i] *= -1;
    }
    return a;
}

dnum neg(dnum a) {
    for(int i = 0; i < SIZE; ++i) {
        a.d[i] *= -1;
    }
    return a;
}

float realValue(dnum a) {
    return a.d[0];
}

bool isReal(dnum a) {
    bool res = true;
    for(int i = 1; i < SIZE; ++i) {
        res = res && (a.d[i] == 0);
    }
    return res;
}

dnum add(dnum a, dnum b) {
    dnum c;
    for(int i = 0; i < SIZE; ++i) {
        c.d[i] = a.d[i] + b.d[i];
    }
    return c;
}

dnum3 add3(dnum3 a, dnum3 b) {
    a.x = add(a.x, b.x);
    a.y = add(a.y, b.y);
    a.z = add(a.z, b.z);
    return a;
}

dnum sub(dnum a, dnum b) {
    dnum c;
    for(int i = 0; i < SIZE; ++i) {
        c.d[i] = a.d[i] - b.d[i];
    }
    return c;
}

dnum3 sub3(dnum3 a, dnum3 b) {
    a.x = sub(a.x, b.x);
    a.y = sub(a.y, b.y);
    a.z = sub(a.z, b.z);
    return a;
}

// tetrahedral indexing
#define IDX(x,y,z) (tetra[(x)+(y)+(z)] + tri[(x)+(y)+(z)+1] - tri[(x)+(y)+1] + (y))

dnum mul(dnum a, dnum b) { // details in thesis
    dnum c = zero();
    for(int x = 0; x <= DERIVATIVE_ORDER; ++x){
        int yend = DERIVATIVE_ORDER - x;
        for(int y = 0; y <= yend; ++y){
            int zend = yend - y;
            for(int z = 0; z <= zend; ++z){
                int iend = zend - z;
                for(int i = 0; i <= iend; ++i) {
                    int jend = iend - i;
                    for(int j = 0; j <= jend; ++j) {
                        int kend = jend - j;
                        for(int k = 0; k <= kend; ++k) {
                            // x+y+z+a+b+c <= DERIVATIVE_ORDER
                            c.d[IDX(x+i,y+j,z+k)] += a.d[IDX(x,y,z)] * b.d[IDX(i,j,k)] * choose[x+i][x] * choose[y+j][y] * choose[z+k][z];
                        }
                    }
                }
            }
        }
    }
    return c;
}

dnum3 mul3(dnum3 a, dnum b) {
    a.x = mul(a.x, b);
    a.y = mul(a.y, b);
    a.z = mul(a.z, b);
    return a;
}

dnum mul(dnum a, float c) {
    for(int i = 0; i < SIZE; ++i)
        a.d[i] *= c;
    return a;
}

dnum div(dnum a, dnum b) {
    while(!isReal(b)){
        dnum c = conj(b);
        a = mul(a, c);
        b = mul(b, c);
    }

    for(int i = 0; i < SIZE; ++i) {
        a.d[i] /= b.d[0];
    }
    return a;
}

dnum div(dnum a, float c) {
    for(int i = 0; i < SIZE; ++i)
        a.d[i] /= c;
    return a;
}

dnum3 div3(dnum3 a, dnum b) {
    a.x = div(a.x, b);
    a.y = div(a.y, b);
    a.z = div(a.z, b);
    return a;
}

dnum differentiate(dnum a) {
    dnum res;
    for(int i = 0; i < SIZE - 1; ++i) {
        res.d[i] = a.d[i+1];
    }
    res.d[SIZE-1] = 0;
    return res;
}

dnum differentiate_by_x(dnum a) {
    dnum res;
    for(int y = 0; y <= DERIVATIVE_ORDER; ++y) {
        int zend = DERIVATIVE_ORDER - y;
        for(int z = 0; z <= zend; ++z) {
            int xend = zend - z - 1;
            for(int x = 0; x <= xend; ++x) {
                res.d[IDX(x,y,z)] = a.d[IDX(x+1,y,z)];
            }
            res.d[IDX(xend, y, z)] = 0;
        }
    }
    return res;
}

dnum differentiate_by_y(dnum a) {
    dnum res;
    for(int z = 0; z <= DERIVATIVE_ORDER; ++z) {
        int xend = DERIVATIVE_ORDER - z;
        for(int x = 0; x <= xend; ++x) {
            int yend = xend - x - 1;
            for(int y = 0; y <= yend; ++y) {
                res.d[IDX(x,y,z)] = a.d[IDX(x,y+1,z)];
            }
            res.d[IDX(x,yend,z)] = 0;
        }
    }
    return res;
}

dnum differentiate_by_z(dnum a) {
    dnum res;
    for(int x = 0; x <= DERIVATIVE_ORDER; ++x) {
        int yend = DERIVATIVE_ORDER - x;
        for(int y = 0; y <= yend; ++y) {
            int zend = yend - y - 1;
            for(int z = 0; z <= zend; ++z) {
                res.d[IDX(x,y,z)] = a.d[IDX(x,y,z+1)];
            }
            res.d[IDX(x,y,zend)] = 0;
        }
    }
    return res;
}

dnum dabs(dnum a) {
    if(a.d[0] < 0)
        return mul(a, -1.0);
    return a;
}

dnum3 dabs3(dnum3 a) {
    a.x = dabs(a.x);
    a.y = dabs(a.y);
    a.z = dabs(a.z);
    return a;
}

dnum dmax(dnum a, dnum b) {
    if(a.d[0] >= b.d[0])
        return a;
    return b;
}

dnum3 dmax3(dnum3 a, dnum val) {
    a.x = dmax(a.x, val);
    a.y = dmax(a.y, val);
    a.z = dmax(a.z, val);
    return a;
}

dnum dmin(dnum a, dnum b) {
    if(a.d[0] <= b.d[0])
        return a;
    return b;
}

dnum3 dmin3(dnum3 a, dnum val) {
    a.x = dmin(a.x, val);
    a.y = dmin(a.y, val);
    a.z = dmin(a.z, val);
    return a;
}

dnum dclamp(dnum val, dnum low, dnum high) {
    if(realValue(val) < realValue(low)) {
        return low;
    } else if(realValue(val) > realValue(high)) {
        return high;
    }
    return val;
}

dnum dmix(dnum a, dnum b, dnum t) {
    return add(mul(a, sub(constant(1), t)), mul(b, t));
}

dnum dsqrt(dnum a);

dnum dlength(dnum3 a) {
    return dsqrt(add(add(mul(a.x,a.x), mul(a.y,a.y)), mul(a.z,a.z)));
}

dnum dlength(dnum2 a) {
    return dsqrt(add(mul(a.x,a.x), mul(a.y, a.y)));
}

dnum4 mat_mul(mat4 m, dnum4 v) {
    // Remember: OpenGL defaults to column major matrices, which means access is of the format mat[col][row].
    dnum4 result;
    result.x = add(add(add(
                mul(v.x, m[0][0]),
                mul(v.y, m[1][0])),
                mul(v.z, m[2][0])),
                mul(v.w, m[3][0]));
    result.y = add(add(add(
                mul(v.x, m[0][1]),
                mul(v.y, m[1][1])),
                mul(v.z, m[2][1])),
                mul(v.w, m[3][1]));
    result.z = add(add(add(
                mul(v.x, m[0][2]),
                mul(v.y, m[1][2])),
                mul(v.z, m[2][2])),
                mul(v.w, m[3][2]));
    result.w = add(add(add(
                mul(v.x, m[0][3]),
                mul(v.y, m[1][3])),
                mul(v.z, m[2][3])),
                mul(v.w, m[3][3]));
    return result;
}

dnum ddot(dnum3 a, dnum3 b) {
    return add(add(mul(a.x,b.x), mul(a.y,b.y)), mul(a.z,b.z));
}

#endif