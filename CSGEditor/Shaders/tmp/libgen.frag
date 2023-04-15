dnum dsqrt(dnum d) {
    float tmp;
    dnum result = zero();
    result.d[0] = sqrt(d.d[0]);
    tmp = 1/(2*sqrt(d.d[0]));
    tmp *= d.d[IDX(0, 0, 1)];
    result.d[IDX(0, 0, 1)] += tmp;
    tmp = 1/(2*sqrt(d.d[0]));
    tmp *= d.d[IDX(0, 1, 0)];
    result.d[IDX(0, 1, 0)] += tmp;
    tmp = 1/(2*sqrt(d.d[0]));
    tmp *= d.d[IDX(1, 0, 0)];
    result.d[IDX(1, 0, 0)] += tmp;
    return result;
}
dnum dsin(dnum d) {
    float tmp;
    dnum result = zero();
    result.d[0] = sin(d.d[0]);
    tmp = cos(d.d[0]);
    tmp *= d.d[IDX(0, 0, 1)];
    result.d[IDX(0, 0, 1)] += tmp;
    tmp = cos(d.d[0]);
    tmp *= d.d[IDX(0, 1, 0)];
    result.d[IDX(0, 1, 0)] += tmp;
    tmp = cos(d.d[0]);
    tmp *= d.d[IDX(1, 0, 0)];
    result.d[IDX(1, 0, 0)] += tmp;
    return result;
}
dnum dcos(dnum d) {
    float tmp;
    dnum result = zero();
    result.d[0] = cos(d.d[0]);
    tmp = -sin(d.d[0]);
    tmp *= d.d[IDX(0, 0, 1)];
    result.d[IDX(0, 0, 1)] += tmp;
    tmp = -sin(d.d[0]);
    tmp *= d.d[IDX(0, 1, 0)];
    result.d[IDX(0, 1, 0)] += tmp;
    tmp = -sin(d.d[0]);
    tmp *= d.d[IDX(1, 0, 0)];
    result.d[IDX(1, 0, 0)] += tmp;
    return result;
}
