#pragma once
#include "Assets.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

#include <fstream>
#include <string>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
//#include "MeshGeometry.h"
//#include"../Plugins/PlayStation4/PS4MemoryAware.h"

#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define IS_DIGIT(x) \
  (static_cast<unsigned int>((x) - '0') < static_cast<unsigned int>(10))
#define IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

typedef float real_t;



typedef struct {
	std::string name;

	std::vector<int> intValues;
	std::vector<real_t> floatValues;
	std::vector<std::string> stringValues;
} tag_t;


typedef struct {
	int vertex_index;
	int normal_index;
	int texcoord_index;
} index_t;


typedef struct {
	std::vector<index_t> indices;
	std::vector<unsigned char> num_face_vertices;  // The number of vertices per
												   // face. 3 = polygon, 4 = quad,
												   // ... Up to 255.
	std::vector<int> material_ids;                 // per-face material ID
	std::vector<unsigned int> smoothing_group_ids;  // per-face smoothing group
													// ID(0 = off. positive value
													// = group id)
	std::vector<tag_t> tags;                        // SubD tag
} mesh_t;

struct vertex_index_t {
	int v_idx, vt_idx, vn_idx;
	vertex_index_t() : v_idx(-1), vt_idx(-1), vn_idx(-1) {}
	explicit vertex_index_t(int idx) : v_idx(idx), vt_idx(idx), vn_idx(idx) {}
	vertex_index_t(int vidx, int vtidx, int vnidx)
		: v_idx(vidx), vt_idx(vtidx), vn_idx(vnidx) {}
};


static bool tryParseDouble(const char *s, const char *s_end, double *result) {
	if (s >= s_end) {
		return false;
	}

	double mantissa = 0.0;

	int exponent = 0;

	char sign = '+';
	char exp_sign = '+';
	char const *curr = s;

	// How many characters were read in a loop.
	int read = 0;
	// Tells whether a loop terminated due to reaching s_end.
	bool end_not_reached = false;

	/*
			BEGIN PARSING.
	*/

	if (*curr == '+' || *curr == '-') {
		sign = *curr;
		curr++;
	}
	else if (IS_DIGIT(*curr)) { /* Pass through. */
	}
	else {
		goto fail;
	}

	// Read the integer part.
	end_not_reached = (curr != s_end);
	while (end_not_reached && IS_DIGIT(*curr)) {
		mantissa *= 10;
		mantissa += static_cast<int>(*curr - 0x30);
		curr++;
		read++;
		end_not_reached = (curr != s_end);
	}

	// We must make sure we actually got something.
	if (read == 0) goto fail;
	// We allow numbers of form "#", "###" etc.
	if (!end_not_reached) goto assemble;

	// Read the decimal part.
	if (*curr == '.') {
		curr++;
		read = 1;
		end_not_reached = (curr != s_end);
		while (end_not_reached && IS_DIGIT(*curr)) {
			static const double pow_lut[] = {
				1.0, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001, 0.0000001,
			};
			const int lut_entries = sizeof pow_lut / sizeof pow_lut[0];

			// NOTE: Don't use powf here, it will absolutely murder precision.
			mantissa += static_cast<int>(*curr - 0x30) *
				(read < lut_entries ? pow_lut[read] : std::pow(10.0, -read));
			read++;
			curr++;
			end_not_reached = (curr != s_end);
		}
	}
	else if (*curr == 'e' || *curr == 'E') {
	}
	else {
		goto assemble;
	}

	if (!end_not_reached) goto assemble;

	// Read the exponent part.
	if (*curr == 'e' || *curr == 'E') {
		curr++;
		// Figure out if a sign is present and if it is.
		end_not_reached = (curr != s_end);
		if (end_not_reached && (*curr == '+' || *curr == '-')) {
			exp_sign = *curr;
			curr++;
		}
		else if (IS_DIGIT(*curr)) { /* Pass through. */
		}
		else {
			// Empty E is not allowed.
			goto fail;
		}

		read = 0;
		end_not_reached = (curr != s_end);
		while (end_not_reached && IS_DIGIT(*curr)) {
			exponent *= 10;
			exponent += static_cast<int>(*curr - 0x30);
			curr++;
			read++;
			end_not_reached = (curr != s_end);
		}
		exponent *= (exp_sign == '+' ? 1 : -1);
		if (read == 0) goto fail;
	}

assemble:
	*result = (sign == '+' ? 1 : -1) *
		(exponent ? std::ldexp(mantissa * std::pow(5.0, exponent), exponent)
			: mantissa);
	return true;
fail:
	return false;
}

static inline real_t parseReal(const char **token, double default_value = 0.0) {
	(*token) += strspn((*token), " \t");
	const char *end = (*token) + strcspn((*token), " \t\r");
	double val = default_value;
	tryParseDouble((*token), end, &val);
	real_t f = static_cast<real_t>(val);
	(*token) = end;
	return f;
}


//// Make index zero-base, and also support relative index.
static inline bool fixIndex(int idx, int n, int *ret) {
	if (!ret) {
		return false;
	}

	if (idx > 0) {
		(*ret) = idx - 1;
		return true;
	}

	if (idx == 0) {
		// zero is not allowed according to the spec.
		return false;
	}

	if (idx < 0) {
		(*ret) = n + idx;  // negative value = relative
		return true;
	}

	return false;  // never reach here.
}

static inline int parseInt(const char **token) {
	(*token) += strspn((*token), " \t");
	int i = atoi((*token));
	(*token) += strcspn((*token), " \t\r");
	return i;
}

static bool parseTriple(const char **token, int vsize, int vnsize, int vtsize,
	vertex_index_t *ret) {
	if (!ret) {
		return false;
	}

	vertex_index_t vi(-1);

	if (!fixIndex(atoi((*token)), vsize, &(vi.v_idx))) {
		return false;
	}

	(*token) += strcspn((*token), "/ \t\r");
	if ((*token)[0] != '/') {
		(*ret) = vi;
		return true;
	}
	(*token)++;

	// i//k
	if ((*token)[0] == '/') {
		(*token)++;
		if (!fixIndex(atoi((*token)), vnsize, &(vi.vn_idx))) {
			return false;
		}
		(*token) += strcspn((*token), "/ \t\r");
		(*ret) = vi;
		return true;
	}

	// i/j/k or i/j
	if (!fixIndex(atoi((*token)), vtsize, &(vi.vt_idx))) {
		return false;
	}

	(*token) += strcspn((*token), "/ \t\r");
	if ((*token)[0] != '/') {
		(*ret) = vi;
		return true;
	}

	// i/j/k
	(*token)++;  // skip '/'
	if (!fixIndex(atoi((*token)), vnsize, &(vi.vn_idx))) {
		return false;
	}
	(*token) += strcspn((*token), "/ \t\r");

	(*ret) = vi;

	return true;
}




static inline bool parseReal(const char **token, real_t *out) {
	(*token) += strspn((*token), " \t");
	const char *end = (*token) + strcspn((*token), " \t\r");
	double val;
	bool ret = tryParseDouble((*token), end, &val);
	if (ret) {
		real_t f = static_cast<real_t>(val);
		(*out) = f;
	}
	(*token) = end;
	return ret;
}

static inline void parseReal2(real_t *x, real_t *y, const char **token,
	const double default_x = 0.0,
	const double default_y = 0.0) {
	(*x) = parseReal(token, default_x);
	(*y) = parseReal(token, default_y);
}

static inline void parseReal3(real_t *x, real_t *y, real_t *z,
	const char **token, const double default_x = 0.0,
	const double default_y = 0.0,
	const double default_z = 0.0) {
	(*x) = parseReal(token, default_x);
	(*y) = parseReal(token, default_y);
	(*z) = parseReal(token, default_z);
}

static inline void parseV(real_t *x, real_t *y, real_t *z, real_t *w,
	const char **token, const double default_x = 0.0,
	const double default_y = 0.0,
	const double default_z = 0.0,
	const double default_w = 1.0) {
	(*x) = parseReal(token, default_x);
	(*y) = parseReal(token, default_y);
	(*z) = parseReal(token, default_z);
	(*w) = parseReal(token, default_w);
}



// Extension: parse vertex with colors(6 items)
static inline bool parseVertexWithColor(real_t *x, real_t *y, real_t *z,
	real_t *r, real_t *g, real_t *b,
	const char **token,
	const double default_x = 0.0,
	const double default_y = 0.0,
	const double default_z = 0.0) {
	(*x) = parseReal(token, default_x);
	(*y) = parseReal(token, default_y);
	(*z) = parseReal(token, default_z);

	const bool found_color =
		parseReal(token, *r) && parseReal(token, *g) && parseReal(token, *b);

	if (!found_color) {
		(*r) = (*g) = (*b) = 1.0;
	}

	return found_color;
}


typedef struct {
	std::vector<int> indices;  // pairs of indices for lines
} path_t;

typedef struct {
	std::string name;
	mesh_t mesh;
	path_t path;
} shape_t;


struct face_t {
	unsigned int
		smoothing_group_id;  // smoothing group id. 0 = smoothing groupd is off.
	int pad_;
	std::vector<vertex_index_t> vertex_indices;  // face vertex indices.

	face_t() : smoothing_group_id(0), pad_(0) {}
};

struct line_t {
	int idx0;
	int idx1;
};
// code from https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
template <typename T>
static int pnpoly(int nvert, T *vertx, T *verty, T testx, T testy) {
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx <
			(vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) +
				vertx[i]))
			c = !c;
	}
	return c;
}

struct tag_sizes {
	tag_sizes() : num_ints(0), num_reals(0), num_strings(0) {}
	int num_ints;
	int num_reals;
	int num_strings;
};

struct obj_shape {
	std::vector<real_t> v;
	std::vector<real_t> vn;
	std::vector<real_t> vt;
};

// TODO(syoyo): refactor function.
static bool exportGroupsToShape(shape_t *shape,
	const std::vector<face_t> &faceGroup,
	std::vector<int> &lineGroup,
	const std::vector<tag_t> &tags,
	const int material_id, const std::string &name,
	bool triangulate,
	const std::vector<real_t> &v) {
	if (faceGroup.empty() && lineGroup.empty()) {
		return false;
	}

	if (!faceGroup.empty()) {
		// Flatten vertices and indices
		for (size_t i = 0; i < faceGroup.size(); i++) {
			const face_t &face = faceGroup[i];

			size_t npolys = face.vertex_indices.size();

			if (npolys < 3) {
				// Face must have 3+ vertices.
				continue;
			}

			vertex_index_t i0 = face.vertex_indices[0];
			vertex_index_t i1(-1);
			vertex_index_t i2 = face.vertex_indices[1];

			if (triangulate) {
				// find the two axes to work in
				size_t axes[2] = { 1, 2 };
				for (size_t k = 0; k < npolys; ++k) {
					i0 = face.vertex_indices[(k + 0) % npolys];
					i1 = face.vertex_indices[(k + 1) % npolys];
					i2 = face.vertex_indices[(k + 2) % npolys];
					size_t vi0 = size_t(i0.v_idx);
					size_t vi1 = size_t(i1.v_idx);
					size_t vi2 = size_t(i2.v_idx);

					if (((3 * vi0 + 2) >= v.size()) || ((3 * vi1 + 2) >= v.size()) ||
						((3 * vi2 + 2) >= v.size())) {
						// Invalid triangle.
						// FIXME(syoyo): Is it ok to simply skip this invalid triangle?
						continue;
					}
					real_t v0x = v[vi0 * 3 + 0];
					real_t v0y = v[vi0 * 3 + 1];
					real_t v0z = v[vi0 * 3 + 2];
					real_t v1x = v[vi1 * 3 + 0];
					real_t v1y = v[vi1 * 3 + 1];
					real_t v1z = v[vi1 * 3 + 2];
					real_t v2x = v[vi2 * 3 + 0];
					real_t v2y = v[vi2 * 3 + 1];
					real_t v2z = v[vi2 * 3 + 2];
					real_t e0x = v1x - v0x;
					real_t e0y = v1y - v0y;
					real_t e0z = v1z - v0z;
					real_t e1x = v2x - v1x;
					real_t e1y = v2y - v1y;
					real_t e1z = v2z - v1z;
					real_t cx = std::fabs(e0y * e1z - e0z * e1y);
					real_t cy = std::fabs(e0z * e1x - e0x * e1z);
					real_t cz = std::fabs(e0x * e1y - e0y * e1x);
					const real_t epsilon = std::numeric_limits<real_t>::epsilon();
					if (cx > epsilon || cy > epsilon || cz > epsilon) {
						// found a corner
						if (cx > cy && cx > cz) {
						}
						else {
							axes[0] = 0;
							if (cz > cx && cz > cy) axes[1] = 1;
						}
						break;
					}
				}

				real_t area = 0;
				for (size_t k = 0; k < npolys; ++k) {
					i0 = face.vertex_indices[(k + 0) % npolys];
					i1 = face.vertex_indices[(k + 1) % npolys];
					size_t vi0 = size_t(i0.v_idx);
					size_t vi1 = size_t(i1.v_idx);
					if (((vi0 * 3 + axes[0]) >= v.size()) ||
						((vi0 * 3 + axes[1]) >= v.size()) ||
						((vi1 * 3 + axes[0]) >= v.size()) ||
						((vi1 * 3 + axes[1]) >= v.size())) {
						// Invalid index.
						continue;
					}
					real_t v0x = v[vi0 * 3 + axes[0]];
					real_t v0y = v[vi0 * 3 + axes[1]];
					real_t v1x = v[vi1 * 3 + axes[0]];
					real_t v1y = v[vi1 * 3 + axes[1]];
					area += (v0x * v1y - v0y * v1x) * static_cast<real_t>(0.5);
				}


				face_t remainingFace = face;  // copy
				size_t guess_vert = 0;
				vertex_index_t ind[3];
				real_t vx[3];
				real_t vy[3];

				// How many iterations can we do without decreasing the remaining
				// vertices.
				size_t remainingIterations = face.vertex_indices.size();
				size_t previousRemainingVertices = remainingFace.vertex_indices.size();

				while (remainingFace.vertex_indices.size() > 3 && remainingIterations > 0) {
					npolys = remainingFace.vertex_indices.size();
					if (guess_vert >= npolys) {
						guess_vert -= npolys;
					}

					if (previousRemainingVertices != npolys) {
						// The number of remaining vertices decreased. Reset counters.
						previousRemainingVertices = npolys;
						remainingIterations = npolys;
					}
					else {
						// We didn't consume a vertex on previous iteration, reduce the
						// available iterations.
						remainingIterations--;
					}

					for (size_t k = 0; k < 3; k++) {
						ind[k] = remainingFace.vertex_indices[(guess_vert + k) % npolys];
						size_t vi = size_t(ind[k].v_idx);
						if (((vi * 3 + axes[0]) >= v.size()) ||
							((vi * 3 + axes[1]) >= v.size())) {
							// ???
							vx[k] = static_cast<real_t>(0.0);
							vy[k] = static_cast<real_t>(0.0);
						}
						else {
							vx[k] = v[vi * 3 + axes[0]];
							vy[k] = v[vi * 3 + axes[1]];
						}
					}
					real_t e0x = vx[1] - vx[0];
					real_t e0y = vy[1] - vy[0];
					real_t e1x = vx[2] - vx[1];
					real_t e1y = vy[2] - vy[1];
					real_t cross = e0x * e1y - e0y * e1x;
					// if an internal angle
					if (cross * area < static_cast<real_t>(0.0)) {
						guess_vert += 1;
						continue;
					}

					// check all other verts in case they are inside this triangle
					bool overlap = false;
					for (size_t otherVert = 3; otherVert < npolys; ++otherVert) {
						size_t idx = (guess_vert + otherVert) % npolys;

						if (idx >= remainingFace.vertex_indices.size()) {
							// ???
							continue;
						}

						size_t ovi = size_t(remainingFace.vertex_indices[idx].v_idx);

						if (((ovi * 3 + axes[0]) >= v.size()) ||
							((ovi * 3 + axes[1]) >= v.size())) {
							// ???
							continue;
						}
						real_t tx = v[ovi * 3 + axes[0]];
						real_t ty = v[ovi * 3 + axes[1]];
						if (pnpoly(3, vx, vy, tx, ty)) {
							overlap = true;
							break;
						}
					}

					if (overlap) {
						guess_vert += 1;
						continue;
					}

					// this triangle is an ear
					{
						index_t idx0, idx1, idx2;
						idx0.vertex_index = ind[0].v_idx;
						idx0.normal_index = ind[0].vn_idx;
						idx0.texcoord_index = ind[0].vt_idx;
						idx1.vertex_index = ind[1].v_idx;
						idx1.normal_index = ind[1].vn_idx;
						idx1.texcoord_index = ind[1].vt_idx;
						idx2.vertex_index = ind[2].v_idx;
						idx2.normal_index = ind[2].vn_idx;
						idx2.texcoord_index = ind[2].vt_idx;

						shape->mesh.indices.push_back(idx0);
						shape->mesh.indices.push_back(idx1);
						shape->mesh.indices.push_back(idx2);

						shape->mesh.num_face_vertices.push_back(3);
						shape->mesh.material_ids.push_back(material_id);
						shape->mesh.smoothing_group_ids.push_back(face.smoothing_group_id);
					}

					// remove v1 from the list
					size_t removed_vert_index = (guess_vert + 1) % npolys;
					while (removed_vert_index + 1 < npolys) {
						remainingFace.vertex_indices[removed_vert_index] =
							remainingFace.vertex_indices[removed_vert_index + 1];
						removed_vert_index += 1;
					}
					remainingFace.vertex_indices.pop_back();
				}

				if (remainingFace.vertex_indices.size() == 3) {
					i0 = remainingFace.vertex_indices[0];
					i1 = remainingFace.vertex_indices[1];
					i2 = remainingFace.vertex_indices[2];
					{
						index_t idx0, idx1, idx2;
						idx0.vertex_index = i0.v_idx;
						idx0.normal_index = i0.vn_idx;
						idx0.texcoord_index = i0.vt_idx;
						idx1.vertex_index = i1.v_idx;
						idx1.normal_index = i1.vn_idx;
						idx1.texcoord_index = i1.vt_idx;
						idx2.vertex_index = i2.v_idx;
						idx2.normal_index = i2.vn_idx;
						idx2.texcoord_index = i2.vt_idx;

						shape->mesh.indices.push_back(idx0);
						shape->mesh.indices.push_back(idx1);
						shape->mesh.indices.push_back(idx2);

						shape->mesh.num_face_vertices.push_back(3);
						shape->mesh.material_ids.push_back(material_id);
						shape->mesh.smoothing_group_ids.push_back(face.smoothing_group_id);
					}
				}
			}
			else {
				for (size_t k = 0; k < npolys; k++) {
					index_t idx;
					idx.vertex_index = face.vertex_indices[k].v_idx;
					idx.normal_index = face.vertex_indices[k].vn_idx;
					idx.texcoord_index = face.vertex_indices[k].vt_idx;
					shape->mesh.indices.push_back(idx);
				}

				shape->mesh.num_face_vertices.push_back(
					static_cast<unsigned char>(npolys));
				shape->mesh.material_ids.push_back(material_id);  // per face
				shape->mesh.smoothing_group_ids.push_back(
					face.smoothing_group_id);  // per face
			}
		}

		shape->name = name;
		shape->mesh.tags = tags;
	}

	if (!lineGroup.empty()) {
		shape->path.indices.swap(lineGroup);
	}

	return true;
}


typedef struct {
	std::vector<NCL::Vector3>		positions;
	std::vector<NCL::Vector2>		texCoords;
	std::vector<NCL::Vector4>	colours;
	std::vector<NCL::Vector3>	normals;
	std::vector<NCL::Vector3>	tangents;
}attrib_n;

static std::istream &safeGetline(std::istream &is, std::string &t) {
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf *sb = is.rdbuf();

	if (se) {
		for (;;) {
			int c = sb->sbumpc();
			switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n') sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty()) is.setstate(std::ios::eofbit);
				return is;
			default:
				t += static_cast<char>(c);
			}
		}
	}

	return is;
}













namespace NCL {
	namespace PS4 {
		//	using namespace sce;
		class ObjLoader
			//	:public NCL::MeshGeometry, public PS4MemoryAware


		{
		public:
			ObjLoader() {
				n = nullptr;
				t = nullptr;
				tan = nullptr;
				//	n = new Vector3();
			};
			~ObjLoader() {
				delete[] n;
				delete[] t;
				delete[] tan;
			};
			//
			//attrib_t* getAttrib() {
			//	return attrib;
			//}
			//

			void loadOBJ(const std::string&filename /*attrib_t *attrib, *//*std::vector<shape_t> *shapes*/) {

				//v.clear();
			//	positions.clear();
				std::vector<NCL::Vector3> p;
				positions = p;


				if (filename == "") {
					return;
				}
				//	std::vector<NCL::Vector3>	v;


				std::ifstream inStream(filename, std::ios::binary);
				std::cout << "using obj loader!!!\n\n\n";


				size_t line_num = 0;
				std::string linebuf;

				while (inStream.peek() != -1) {
					safeGetline(inStream, linebuf);

					line_num++;

					// Trim newline '\r\n' or '\n'
					if (linebuf.size() > 0) {
						if (linebuf[linebuf.size() - 1] == '\n')
							linebuf.erase(linebuf.size() - 1);
					}
					if (linebuf.size() > 0) {
						if (linebuf[linebuf.size() - 1] == '\r')
							linebuf.erase(linebuf.size() - 1);
					}

					// Skip if empty line.
					if (linebuf.empty()) {
						continue;
					}

					// Skip leading space.

					const char *token = linebuf.c_str();
					token += strspn(token, " \t");




					assert(token);
					if (token[0] == '\0') continue;  // empty line

					if (token[0] == '#') continue;  // comment line

					// vertex
					if (token[0] == 'v' && IS_SPACE((token[1]))) {
						token += 2;
						real_t x, y, z;
						real_t r, g, b;

						parseVertexWithColor(&x, &y, &z, &r, &g, &b, &token);
						NCL::Maths::Vector3 temp;
						temp.x = x;
						temp.y = y;
						temp.z = z;

						numvertices++;
						positions.push_back(temp);


						continue;

					}



					if (token[0] == 'f' && IS_SPACE((token[1]))) {
						token += 2;
						real_t x, y, z;
						real_t r, g, b;

						parseVertexWithColor(&x, &y, &z, &r, &g, &b, &token);

						//	 std::cout << "\nx,y,z=" << x << y << z << "\n";
						indices.push_back(x - 1);
						indices.push_back(y - 1);
						indices.push_back(z - 1);

						numIndices += 3;
						continue;

					}



					// texcoord
					if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
						token += 3;
						real_t x, y;
						parseReal2(&x, &y, &token);
						texCoords.push_back(Vector2(x, y));
						continue;
					}


				}


				normalGenerator();


			}

			vector<Vector3> getPosition() {
				return positions;
			}

			vector<unsigned int> getIndices() {
				return indices;
			}
			vector<Vector3> getNormals() {
				return normals;
			}
			vector<Vector3> getTangents() {
				return tangents;
			}
			vector<Vector2> getTexcood() {
				
/*				if (texCoords.size() != numvertices) {
					return realTexCoords;
				}
				else */return texCoords;
			}
			void normalGenerator() {
				//	vector<Vector3> normals;
				//	if (indices[numIndices]) { // Generate per - vertex normals
				n = new Vector3[numvertices];
				tan = new Vector3[numvertices];
			//	t = new Vector2[numvertices];
				for (int i = 0; i < numvertices; ++i) {
					n[i] = Vector3();
				}
				for (int i = 0; i < numIndices; i += 3) {
					unsigned int a = indices[i];
					unsigned int b = indices[i + 1];
					unsigned int c = indices[i + 2];
					Vector3 normal = Vector3::Cross(
						(positions[b] - positions[a]), (positions[c] - positions[a]));
					normal.Normalise();

					n[a] = normal;
					n[b] = normal;
					n[c] = normal;


				//	Vector3 tangent = GenerateTangent(positions[a], positions[b],
				//		positions[c], texCoords[a],
				//		texCoords[a], texCoords[a]);
				//	tan[a] = tangent;
				//	tan[b] = tangent;
				//	tan[c] = tangent;
			

				}


				for (int i = 0; i < numvertices; ++i) {
					normals.push_back(n[i]);
					tan[i].Normalise();
					tangents.push_back(tan[i]);

					if (texCoords.size() != numvertices) {
					//	realTexCoords.push_back(t[i]);
					}
			
				}


			}
			void tangentGenerator() {

			}
		private:
		
			GeometryPrimitive	primType;
			vector<Vector3>		positions;
			int numIndices = 0;
			int numvertices = 0;

			vector<Vector2>		texCoords;
			vector<Vector2>		realTexCoords;
			vector<Vector4>		colours;
			vector<Vector3>		normals;
			Vector3*		n;
			Vector3*		tan;
			Vector2*		t;
			vector<Vector3>		tangents;
			vector<unsigned int>	indices;


			Vector3 GenerateTangent(const Vector3 &a, const Vector3 &b,
				const Vector3 &c, const Vector2 & ta,
				const Vector2 & tb, const Vector2 & tc) {
				Vector2 coord1 = tb - ta;
				Vector2 coord2 = tc - ta;

				Vector3 vertex1 = b - a;
				Vector3 vertex2 = c - a;

				Vector3 axis = Vector3(vertex1 * coord2.y - vertex2 * coord1.y);
				float factor = 1.0f / (coord1.x * coord2.y - coord2.x * coord1.y);
				return axis * factor;
			}



		};

	}
}