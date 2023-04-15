//?#version 460

dnum d_cube(vec3 size, dnum3 point) { // half size
	dnum3 dist = sub3(dabs3(point), constant3(size));
	if(realValue(dist.x) < 0 && realValue(dist.y) < 0 && realValue(dist.z) < 0) {
		return dmax(dist.x, dmax(dist.y, dist.z));
	} else {
		return dlength(dmax3(dist, constant(0.0)));
	}
}

dnum d_sphere(float radius, dnum3 point) {
	return sub(dlength(point), constant(radius));
}

dnum d_cylinder(float radius, float height, dnum3 point) {
	dnum hd = sub(dlength(dnum2(point.x, point.z)), constant(radius));
	dnum vd = sub(dabs(point.y), constant(height * 0.5));
	if(realValue(vd) > 0.0 && realValue(hd) > 0.0)
		return dlength(dnum2(hd, vd));
	return dmax(vd, hd);
// IQ:
//	vec2 d = abs(vec2(length(point.xz),point.y)) - vec2(radius,height);
//	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
dnum d_torus(float major_radius, float minor_radius, dnum3 point) { 
	dnum2 q = dnum2(sub(dlength(dnum2(point.x, point.z)), constant(major_radius)), point.y);
	return sub(dlength(q), constant(minor_radius));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
dnum d_ellipsoid(vec3 radii, dnum3 point) { 
	dnum k0 = dlength(dnum3(div(point.x, radii.x), div(point.y,radii.y), div(point.z,radii.z)));
	dnum3 rsq = constant3(radii*radii);
	dnum k1 = dlength(dnum3(div(point.x, rsq.x), div(point.y,rsq.y), div(point.z,rsq.z)));
	return div(mul(k0, sub(k0, constant(1))), k1);
}

// SOURCE: https://iquilezles.org/articles/smin/
dnum d_smooth_union(dnum d1, dnum d2, float k) {
	dnum h = div(dmax(sub(constant(k), dabs(sub(d1, d2))), constant(0)), constant(k));
	return sub(dmin(d1, d2), mul(h, mul(h, mul(h, constant(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
dnum d_smooth_intersection(dnum d1, dnum d2, float k) {
	dnum h = div(dmax(sub(constant(k), dabs(sub(d1, d2))), constant(0)), constant(k));
	return add(dmax(d1, d2), mul(h, mul(h, mul(h, constant(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
dnum d_smooth_substraction(dnum d1, dnum d2, float k) {
	d2 = neg(d2);
	dnum h = div(dmax(sub(constant(k), dabs(sub(d1, d2))), constant(0)), constant(k));
	return add(dmax(d1, d2), mul(h, mul(h, mul(h, constant(k/6)))) );
}

dnum d_plane(vec3 n, float h, dnum3 point) {
	return add(ddot(point, constant3(n)), constant(h));
}