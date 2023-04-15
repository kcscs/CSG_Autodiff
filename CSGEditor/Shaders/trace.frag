#version 460

#define DISPLAY_MODE_SHADED 0
#define DISPLAY_MODE_STEPS 1
#define DISPLAY_MODE_GRADIENT 2
#define DISPLAY_MODE_GAUSSIAN_CURVATURE 3
#define DISPLAY_MODE_MEAN_CURVATURE 4
#define DISPLAY_MODE_NORMAL_DIFF 5

uniform vec3 eye_pos; // position of camera
uniform mat4x4 view_proj; // proj mtx * view mtx
uniform mat4x4 inv_view_proj;

uniform vec3 to_light; //direction towards the light source

uniform int display_mode;
uniform float vis_multiplier = 0.1f; // multiplier for adjusting strength of curvatures or normal differences
uniform int use_auto_diff = 0;
uniform float eps = 0.01; // epsilon value used for numeric approximations

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

float sdf(vec3 pos);

// sphere tracing settings
const int max_steps = 500;
const float stop_dist = 0.0001;
const float max_dist = 1000.0f;

// used for sphere tracing step count display
vec3 color_by_steps(int steps) {
	float ratio = steps / float(max_steps);
	vec3 rgb = vec3(0);
	rgb.r = min(1.0, ratio * 3);
	rgb.g = clamp((ratio - 1.0/3) * 3, 0.0, 1.0);
	rgb.b = clamp((ratio - 2.0/3) * 3, 0.0, 1.0);
	return rgb;
}

mat3 adjoint(mat3 m) {
	return mat3( //OpenGL defaults to column major matrices
		m[1][1]*m[2][2] - m[2][1]*m[1][2], -(m[1][0]*m[2][2]-m[2][0]*m[1][2]), m[1][0]*m[2][1]-m[2][0]*m[1][1], // first column
		-(m[0][1]*m[2][2]-m[2][1]*m[0][2]), m[0][0]*m[2][2]-m[2][0]*m[0][2], -(m[0][0]*m[2][1]-m[2][0]*m[0][1]), // second column
		m[0][1]*m[1][2]-m[1][1]*m[0][2], -(m[0][0]*m[1][2]-m[1][0]*m[0][2]), m[0][0]*m[1][1]-m[1][0]*m[0][1] // third column
	);
}

#ifdef DERIVATIVES_ENABLED
#if DERIVATIVE_ORDER > 1
float compute_gaussian_curvature(vec3 pos) {
	dnum d = dsdf(variable3(pos, 1, 1, 1));
	vec3 gradient = vec3(d.d[1], d.d[2], d.d[3]);
	mat3 adjoint_hessian = adjoint(mat3(
		d.d[IDX(2,0,0)], d.d[IDX(1,1,0)], d.d[IDX(1,0,1)], // first column 
		d.d[IDX(1,1,0)], d.d[IDX(0,2,0)], d.d[IDX(0,1,1)], // second column
		d.d[IDX(1,0,1)], d.d[IDX(0,1,1)], d.d[IDX(0,0,2)]  // third column
	));

	return dot(transpose(adjoint_hessian) * gradient, gradient) / pow(length(gradient), 4);
}

float compute_mean_curvature(vec3 pos) {
	dnum d = dsdf(variable3(pos, 1, 1, 1));
	vec3 gradient = vec3(d.d[1], d.d[2], d.d[3]);
	mat3 hessian = mat3(
		d.d[IDX(2,0,0)], d.d[IDX(1,1,0)], d.d[IDX(1,0,1)], // first column 
		d.d[IDX(1,1,0)], d.d[IDX(0,2,0)], d.d[IDX(0,1,1)], // second column
		d.d[IDX(1,0,1)], d.d[IDX(0,1,1)], d.d[IDX(0,0,2)]  // third column
	);

	return (dot(transpose(hessian) * gradient, gradient) - dot(gradient, gradient)*(hessian[0][0] + hessian[1][1] + hessian[2][2])) / (2*pow(length(gradient), 3));
}
#endif
#endif

vec3 approx_gradient(vec3 pos) {
	vec3 d;
	d.x = sdf(pos+vec3(eps,0,0))-sdf(pos-vec3(eps,0,0));
	d.y = sdf(pos+vec3(0,eps,0))-sdf(pos-vec3(0,eps,0));
	d.z = sdf(pos+vec3(0,0,eps))-sdf(pos-vec3(0,0,eps));
	return d/(2*eps);
}

vec3 approx_normal(vec3 pos) {
	vec3 d;
	d.x = sdf(pos+vec3(eps,0,0))-sdf(pos-vec3(eps,0,0));
	d.y = sdf(pos+vec3(0,eps,0))-sdf(pos-vec3(0,eps,0));
	d.z = sdf(pos+vec3(0,0,eps))-sdf(pos-vec3(0,0,eps));
	return normalize(d);
} // symmetric difference

#ifdef DERIVATIVES_ENABLED
vec3 dual_normal(vec3 pos) {
	dnum derivatives = dsdf(variable3(pos, 1, 1, 1));
	vec3 normal = vec3(derivatives.d[1], derivatives.d[2], derivatives.d[3]);
	return normalize(normal); // needed for not exact sdf-s
}
#endif

mat3 approx_hessian(vec3 pos) {
	vec3 gxp = approx_gradient(pos+vec3(eps,0,0));
	vec3 gxm = approx_gradient(pos+vec3(-eps,0,0));
	vec3 gyp = approx_gradient(pos+vec3(0,eps,0));
	vec3 gym = approx_gradient(pos+vec3(0,-eps,0));
	vec3 gzp = approx_gradient(pos+vec3(0,0,eps));
	vec3 gzm = approx_gradient(pos+vec3(0,0,-eps));
	mat3 hessian;
	hessian = mat3(
		gxp.x-gxm.x, gxp.y-gxm.y, gxp.z-gxm.z, // first column 
		gyp.x-gym.x, gyp.y-gym.y, gyp.z-gym.z, // second column
		gzp.x-gzm.x, gzp.y-gzm.y, gzp.z-gzm.z  // third column
	);
	return hessian / (2*eps);
}

float approx_gaussian(vec3 pos) {
	vec3 gradient = approx_gradient(pos);
	mat3 adjoint_hessian = adjoint(approx_hessian(pos));

	return dot(transpose(adjoint_hessian) * gradient, gradient) / pow(length(gradient), 4);
}

float approx_mean(vec3 pos) {
	vec3 gradient = approx_gradient(pos);
	mat3 hessian = approx_hessian(pos);

	return (dot(transpose(hessian) * gradient, gradient) - dot(gradient, gradient)*(hessian[0][0] + hessian[1][1] + hessian[2][2])) / (2*pow(length(gradient), 3));
}

vec3 get_ray(vec2 uv) {
	vec4 a = inv_view_proj * vec4(uv.x, uv.y, -1, 1);
	vec4 b = inv_view_proj * vec4(uv.x, uv.y, 1, 1);

	vec3 a3 = a.xyz/a.w;
	vec3 b3 = b.xyz/b.w;

	return normalize(b3 - a3);
}

void main()
{
	vec2 uv = fs_in_tex*2 - vec2(1,1);
	vec3 ray = get_ray(uv);

	int steps = max_steps;	
	
	// sphere tracing
	vec3 pos = eye_pos;
	vec3 prev_pos = pos;
	float dist = sdf(pos);
	float t = 0;
	while(steps > 0 && dist > stop_dist && t < max_dist) {
		--steps;
		prev_pos = pos;
		pos += ray * dist;
		t += dist;
		dist = sdf(pos);
	}

	// out of steps
	if(steps == max_steps) {
		fs_out_col = vec4(1,1,0,1);
		vec4 dev_coord = view_proj * vec4(pos,1);
		dev_coord /= dev_coord.w;
		float depth = dev_coord.z*0.5f+0.5f;
		gl_FragDepth = depth;
		return;
	}

	// compute correct depth value for depth buffer
	// this allows us to use incremental rendering later for the gizmos with proper depth testing
	vec4 dev_coord = view_proj * vec4(pos,1);
	dev_coord /= dev_coord.w;
	float depth = dev_coord.z*0.5f+0.5f;
	gl_FragDepth = depth;


	if(display_mode == DISPLAY_MODE_STEPS) {
		fs_out_col = vec4(color_by_steps(max_steps-steps), 1);
		return;
	} 

	if(t >= max_dist){
		discard;
	}

	if(isinf(pos.x) || isinf(pos.y) || isinf(pos.z)){
		discard; // replace this line with a vivid color for debugging
	}

	// compute normal vector with approximation or autodiff
	vec3 normal;
	#ifdef DERIVATIVES_ENABLED
		if(use_auto_diff > 0){
			normal = dual_normal(pos);
		} else {
	#endif
	normal = approx_normal(pos);
	#ifdef DERIVATIVES_ENABLED
	}
	#endif

	if(display_mode == DISPLAY_MODE_GRADIENT) {
		fs_out_col = vec4(normal, 1);
		return;
	}

	#ifdef DERIVATIVES_ENABLED

	if(display_mode == DISPLAY_MODE_NORMAL_DIFF) {
		vec3 correct = use_auto_diff == 1 ? normal : dual_normal(pos);
		vec3 approx = use_auto_diff == 0 ? normal : approx_normal(pos);
		vec3 diff = correct - approx;
		fs_out_col = vec4(abs(diff)*5, 1);
		return;
	}

	// Curvatures with automatic differentiation
	#if DERIVATIVE_ORDER > 1
	if(use_auto_diff == 1) {
	if(display_mode == DISPLAY_MODE_GAUSSIAN_CURVATURE) {
		float curvature = compute_gaussian_curvature(pos);
//		if(isnan(curvature)) { Uncomment for debugging
//			fs_out_col = vec4(1,0,1,1);
//			return;
//		}
		if(curvature >= 0) {
			fs_out_col = mix(vec4(1,1,1,1), vec4(0, 1, 0, 1), curvature * vis_multiplier);
		} else {
			fs_out_col = mix(vec4(1,1,1,1), vec4(1, 0, 0, 1), -curvature * vis_multiplier);
		}
		return;
	}
	else if(display_mode == DISPLAY_MODE_MEAN_CURVATURE) {
		float curvature = compute_mean_curvature(pos);
		if(curvature >= 0) {
			fs_out_col = mix(vec4(1,1,1,1), vec4(0, 1, 0, 1), curvature * vis_multiplier);
		} else {
			fs_out_col = mix(vec4(1,1,1,1), vec4(1, 0, 0, 1), -curvature * vis_multiplier);
		}
		return;
	}
	} else 
	#endif
	#endif
	// curvatures with numeric approximation
	if(display_mode == DISPLAY_MODE_GAUSSIAN_CURVATURE) {
		float curvature = approx_gaussian(pos);
		if(isnan(curvature)) {
			fs_out_col = vec4(1,0,1,1);
			return;
		}
		if(curvature >= 0) {
			fs_out_col = mix(vec4(1,1,1,1), vec4(0, 1, 0, 1), curvature * vis_multiplier);
		} else {
			fs_out_col = mix(vec4(1,1,1,1), vec4(1, 0, 0, 1), -curvature * vis_multiplier);
		}
		return;
	} else if(display_mode == DISPLAY_MODE_MEAN_CURVATURE) {
		float curvature = approx_mean(pos);
		if(curvature >= 0) {
			fs_out_col = mix(vec4(1,1,1,1), vec4(0, 1, 0, 1), curvature * vis_multiplier);
		} else {
			fs_out_col = mix(vec4(1,1,1,1), vec4(1, 0, 0, 1), -curvature * vis_multiplier);
		}
		return;
	}

	if(isnan(normal.x)){
		fs_out_col = vec4(1,0,1,1);
		return;
	}

	// phong shading
	vec3 view = eye_pos - pos;
	float kd = 0.5f;

	float diffuse = kd * max(0, dot(normal, to_light));
	float specular = 0.5f * pow(max(dot(normalize(view), reflect(-to_light, normal)),0), 30);

	fs_out_col = vec4(diffuse,diffuse,diffuse,1) + vec4(specular,specular,specular,1);
}