//?#version 460

float r_cube(vec3 size, vec3 point) { // half size
	vec3 dist = r_sub3(r_dabs3(point), r_constant3(size));
	if(r_realValue(dist.x) < 0 && r_realValue(dist.y) < 0 && r_realValue(dist.z) < 0) {
		return r_dmax(dist.x, r_dmax(dist.y, dist.z));
	} else {
		return r_dlength(r_dmax3(dist, r_constant(0.0)));
	}
}

float r_sphere(float radius, vec3 point) {
	return r_sub(r_dlength(point), r_constant(radius));
}

float r_cylinder(float radius, float height, vec3 point) {
	float hd = r_sub(r_dlength(vec2(point.x, point.z)), r_constant(radius));
	float vd = r_sub(r_dabs(point.y), r_constant(height * 0.5));
	if(r_realValue(vd) > 0.0 && r_realValue(hd) > 0.0)
		return r_dlength(vec2(hd, vd));
	return r_dmax(vd, hd);
// IQ:
//	vec2 d = abs(vec2(length(point.xz),point.y)) - vec2(radius,height);
//	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float r_torus(float major_radius, float minor_radius, vec3 point) { 
	vec2 q = vec2(r_sub(r_dlength(vec2(point.x, point.z)), r_constant(major_radius)), point.y);
	return r_sub(r_dlength(q), r_constant(minor_radius));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float r_ellipsoid(vec3 radii, vec3 point) { 
	float k0 = r_dlength(vec3(r_div(point.x, radii.x), r_div(point.y,radii.y), r_div(point.z,radii.z)));
	vec3 rsq = r_constant3(radii*radii);
	float k1 = r_dlength(vec3(r_div(point.x, rsq.x), r_div(point.y,rsq.y), r_div(point.z,rsq.z)));
	return r_div(r_mul(k0, r_sub(k0, r_constant(1))), k1);
}

// SOURCE: https://iquilezles.org/articles/smin/
float r_smooth_union(float d1, float d2, float k) {
	float h = r_div(r_dmax(r_sub(r_constant(k), r_dabs(r_sub(d1, d2))), r_constant(0)), r_constant(k));
	return r_sub(r_dmin(d1, d2), r_mul(h, r_mul(h, r_mul(h, r_constant(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
float r_smooth_intersection(float d1, float d2, float k) {
	float h = r_div(r_dmax(r_sub(r_constant(k), r_dabs(r_sub(d1, d2))), r_constant(0)), r_constant(k));
	return r_add(r_dmax(d1, d2), r_mul(h, r_mul(h, r_mul(h, r_constant(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
float r_smooth_substraction(float d1, float d2, float k) {
	d2 = r_neg(d2);
	float h = r_div(r_dmax(r_sub(r_constant(k), r_dabs(r_sub(d1, d2))), r_constant(0)), r_constant(k));
	return r_add(r_dmax(d1, d2), r_mul(h, r_mul(h, r_mul(h, r_constant(k/6)))) );
}

float r_plane(vec3 n, float h, vec3 point) {
	return r_add(r_ddot(point, r_constant3(n)), r_constant(h));
}