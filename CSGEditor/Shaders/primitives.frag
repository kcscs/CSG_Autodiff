//?#version 460

_dnum_ _TEMPLATE_cube(vec3 size, _dnum3_ point) { // half size
	_dnum3_ dist = _sub3_(_dabs3_(point), _constant3_(size));
	if(_realValue_(dist.x) < 0 && _realValue_(dist.y) < 0 && _realValue_(dist.z) < 0) {
		return _dmax_(dist.x, _dmax_(dist.y, dist.z));
	} else {
		return _dlength_(_dmax3_(dist, _constant_(0.0)));
	}
}

_dnum_ _TEMPLATE_sphere(float radius, _dnum3_ point) {
	return _sub_(_dlength_(point), _constant_(radius));
}

_dnum_ _TEMPLATE_cylinder(float radius, float height, _dnum3_ point) {
	_dnum_ hd = _sub_(_dlength_(_dnum2_(point.x, point.z)), _constant_(radius));
	_dnum_ vd = _sub_(_dabs_(point.y), _constant_(height * 0.5));
	if(_realValue_(vd) > 0.0 && _realValue_(hd) > 0.0)
		return _dlength_(_dnum2_(hd, vd));
	return _dmax_(vd, hd);
// IQ:
//	vec2 d = abs(vec2(length(point.xz),point.y)) - vec2(radius,height);
//	return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
_dnum_ _TEMPLATE_torus(float major_radius, float minor_radius, _dnum3_ point) { 
	_dnum2_ q = _dnum2_(_sub_(_dlength_(_dnum2_(point.x, point.z)), _constant_(major_radius)), point.y);
	return _sub_(_dlength_(q), _constant_(minor_radius));
}

// source: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
_dnum_ _TEMPLATE_ellipsoid(vec3 radii, _dnum3_ point) { 
	_dnum_ k0 = _dlength_(_dnum3_(_div_(point.x, radii.x), _div_(point.y,radii.y), _div_(point.z,radii.z)));
	_dnum3_ rsq = _constant3_(radii*radii);
	_dnum_ k1 = _dlength_(_dnum3_(_div_(point.x, rsq.x), _div_(point.y,rsq.y), _div_(point.z,rsq.z)));
	return _div_(_mul_(k0, _sub_(k0, _constant_(1))), k1);
}

// SOURCE: https://iquilezles.org/articles/smin/
_dnum_ _TEMPLATE_smooth_union(_dnum_ d1, _dnum_ d2, float k) {
	_dnum_ h = _div_(_dmax_(_sub_(_constant_(k), _dabs_(_sub_(d1, d2))), _constant_(0)), _constant_(k));
	return _sub_(_dmin_(d1, d2), _mul_(h, _mul_(h, _mul_(h, _constant_(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
_dnum_ _TEMPLATE_smooth_intersection(_dnum_ d1, _dnum_ d2, float k) {
	_dnum_ h = _div_(_dmax_(_sub_(_constant_(k), _dabs_(_sub_(d1, d2))), _constant_(0)), _constant_(k));
	return _add_(_dmax_(d1, d2), _mul_(h, _mul_(h, _mul_(h, _constant_(k/6)))) );
}

// SOURCE: https://iquilezles.org/articles/smin/ (derived from smin)
_dnum_ _TEMPLATE_smooth_substraction(_dnum_ d1, _dnum_ d2, float k) {
	d2 = _neg_(d2);
	_dnum_ h = _div_(_dmax_(_sub_(_constant_(k), _dabs_(_sub_(d1, d2))), _constant_(0)), _constant_(k));
	return _add_(_dmax_(d1, d2), _mul_(h, _mul_(h, _mul_(h, _constant_(k/6)))) );
}

_dnum_ _TEMPLATE_plane(vec3 n, float h, _dnum3_ point) {
	return _add_(_ddot_(point, _constant3_(n)), _constant_(h));
}