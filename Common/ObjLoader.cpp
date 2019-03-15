#include "ObjLoader.h"

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


ObjLoader::ObjLoader(const std::string&filename, attrib_t *attrib, std::vector<shape_t> *shapes)
{

	std::ifstream inStream(filename, std::ios::binary);
	std::cout << "using obj loader!!!\n\n\n";
	std::vector<real_t> v;
	std::vector<real_t> vn;
	std::vector<real_t> vt;
	std::vector<real_t> vc;
	//std::vector<tag_t> tags;
	std::vector<face_t> faceGroup;
	std::vector<int> lineGroup;
	std::string name;


	shape_t shape;

	bool found_all_colors = true;


	std::map<std::string, int> material_map;
	int material = -1;
	int greatest_v_idx = -1;
	int greatest_vn_idx = -1;
	int greatest_vt_idx = -1;

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

			found_all_colors &= parseVertexWithColor(&x, &y, &z, &r, &g, &b, &token);

			v.push_back(x);
			v.push_back(y);
			v.push_back(z);

			if (found_all_colors /*|| default_vcols_fallback*/) {
				vc.push_back(r);
				vc.push_back(g);
				vc.push_back(b);
			}

			continue;
		}

		// normal
		if (token[0] == 'v' && token[1] == 'n' && IS_SPACE((token[2]))) {
			token += 3;
			real_t x, y, z;
			parseReal3(&x, &y, &z, &token);
			vn.push_back(x);
			vn.push_back(y);
			vn.push_back(z);
			continue;
		}

		// texcoord
		if (token[0] == 'v' && token[1] == 't' && IS_SPACE((token[2]))) {
			token += 3;
			real_t x, y;
			parseReal2(&x, &y, &token);
			vt.push_back(x);
			vt.push_back(y);
			continue;
		}

		// line
		if (token[0] == 'l' && IS_SPACE((token[1]))) {
			token += 2;

			line_t line_cache;
			bool end_line_bit = 0;
			while (!IS_NEW_LINE(token[0])) {
				// get index from string
				int idx;
				fixIndex(parseInt(&token), 0, &idx);

				size_t n = strspn(token, " \t\r");
				token += n;

				if (!end_line_bit) {
					line_cache.idx0 = idx;
				}
				else {
					line_cache.idx1 = idx;
					lineGroup.push_back(line_cache.idx0);
					lineGroup.push_back(line_cache.idx1);
					line_cache = line_t();
				}
				end_line_bit = !end_line_bit;
			}

			continue;
		}
		// face
		if (token[0] == 'f' && IS_SPACE((token[1]))) {
			token += 2;
			token += strspn(token, " \t");

			face_t face;

			//  face.smoothing_group_id = current_smoothing_id;
			face.vertex_indices.reserve(3);

			while (!IS_NEW_LINE(token[0])) {
				vertex_index_t vi;
				if (!parseTriple(&token, static_cast<int>(v.size() / 3),
					static_cast<int>(vn.size() / 3),
					static_cast<int>(vt.size() / 2), &vi)) {
					//if (err) {
					   // std::stringstream ss;
					   // ss << "Failed parse `f' line(e.g. zero value for face index. line "
						  //  << line_num << ".)\n";
					   // (*err) += ss.str();
					//}
					//return false;
				}

				greatest_v_idx = greatest_v_idx > vi.v_idx ? greatest_v_idx : vi.v_idx;
				greatest_vn_idx =
					greatest_vn_idx > vi.vn_idx ? greatest_vn_idx : vi.vn_idx;
				greatest_vt_idx =
					greatest_vt_idx > vi.vt_idx ? greatest_vt_idx : vi.vt_idx;

				face.vertex_indices.push_back(vi);
				size_t n = strspn(token, " \t\r");
				token += n;
			}

			// replace with emplace_back + std::move on C++11
			faceGroup.push_back(face);

			continue;
		}

		//// use mtl
		//if ((0 == strncmp(token, "usemtl", 6)) && IS_SPACE((token[6]))) {
		   // token += 7;
		   // std::stringstream ss;
		   // ss << token;
		   // std::string namebuf = ss.str();

		   // int newMaterialId = -1;
		   // if (material_map.find(namebuf) != material_map.end()) {
			  //  newMaterialId = material_map[namebuf];
		   // }
		   // else {
			  //  // { error!! material not found }
		   // }

		   // if (newMaterialId != material) {
			  //  // Create per-face material. Thus we don't add `shape` to `shapes` at
			  //  // this time.
			  //  // just clear `faceGroup` after `exportGroupsToShape()` call.
			  //  exportGroupsToShape(&shape, faceGroup, lineGroup, tags, material, name,
			  //	  /*triangulate*/ true, v);
			  //  faceGroup.clear();
			  //  material = newMaterialId;
		   // }

		   // continue;
		//}




		//// load mtl
		//if ((0 == strncmp(token, "mtllib", 6)) && IS_SPACE((token[6]))) {
		   // if (readMatFn) {
			  //  token += 7;

			  //  std::vector<std::string> filenames;
			  //  SplitString(std::string(token), ' ', filenames);

			  //  if (filenames.empty()) {
			  //	  if (warn) {
			  //		  std::stringstream ss;
			  //		  ss << "Looks like empty filename for mtllib. Use default "
			  //			  "material (line "
			  //			  << line_num << ".)\n";

			  //		  (*warn) += ss.str();
			  //	  }
			  //  }
			  //  else {
			  //	  bool found = false;
			  //	  for (size_t s = 0; s < filenames.size(); s++) {
			  //		  std::string warn_mtl;
			  //		  std::string err_mtl;
			  //		  bool ok = (*readMatFn)(filenames[s].c_str(), materials,
			  //			  &material_map, &warn_mtl, &err_mtl);
			  //		  if (warn && (!warn_mtl.empty())) {
			  //			  (*warn) += warn_mtl;
			  //		  }

			  //		  if (err && (!err_mtl.empty())) {
			  //			  (*err) += err_mtl;
			  //		  }

			  //		  if (ok) {
			  //			  found = true;
			  //			  break;
			  //		  }
			  //	  }

			  //	  if (!found) {
			  //		  if (warn) {
			  //			  (*warn) +=
			  //				  "Failed to load material file(s). Use default "
			  //				  "material.\n";
			  //		  }
			  //	  }
			  //  }
		   // }

		   // continue;
		//}

		//// group name
		//if (token[0] == 'g' && IS_SPACE((token[1]))) {
		   // // flush previous face group.
		   // bool ret = exportGroupsToShape(&shape, faceGroup, lineGroup, tags,
			  //  material, name, triangulate, v);
		   // (void)ret;  // return value not used.

		   // if (shape.mesh.indices.size() > 0) {
			  //  shapes->push_back(shape);
		   // }

		   // shape = shape_t();

		   // // material = -1;
		   // faceGroup.clear();

		   // std::vector<std::string> names;

		   // while (!IS_NEW_LINE(token[0])) {
			  //  std::string str = parseString(&token);
			  //  names.push_back(str);
			  //  token += strspn(token, " \t\r");  // skip tag
		   // }

		   // // names[0] must be 'g'

		   // if (names.size() < 2) {
			  //  // 'g' with empty names
			  //  if (warn) {
			  //	  std::stringstream ss;
			  //	  ss << "Empty group name. line: " << line_num << "\n";
			  //	  (*warn) += ss.str();
			  //	  name = "";
			  //  }
		   // }
		   // else {
			  //  std::stringstream ss;
			  //  ss << names[1];

			  //  // tinyobjloader does not support multiple groups for a primitive.
			  //  // Currently we concatinate multiple group names with a space to get
			  //  // single group name.

			  //  for (size_t i = 2; i < names.size(); i++) {
			  //	  ss << " " << names[i];
			  //  }

			  //  name = ss.str();
		   // }

		   // continue;
		//}

		// object name
	   // if (token[0] == 'o' && IS_SPACE((token[1]))) {
		  //  // flush previous face group.
		  //  bool ret = exportGroupsToShape(&shape, faceGroup, lineGroup, tags,
		  //	  material, name, triangulate, v);
		  //  if (ret) {
		  //	  shapes->push_back(shape);
		  //  }

		  //  // material = -1;
		  //  faceGroup.clear();
		  //  shape = shape_t();

		  //  // @todo { multiple object name? }
		  //  token += 2;
		  //  std::stringstream ss;
		  //  ss << token;
		  //  name = ss.str();

		  //  continue;
	   // }

	   // if (token[0] == 't' && IS_SPACE(token[1])) {
		  //  const int max_tag_nums = 8192;  // FIXME(syoyo): Parameterize.
		  //  tag_t tag;

		  //  token += 2;

		  //  tag.name = parseString(&token);

		  //  tag_sizes ts = parseTagTriple(&token);

		  //  if (ts.num_ints < 0) {
		  //	  ts.num_ints = 0;
		  //  }
		  //  if (ts.num_ints > max_tag_nums) {
		  //	  ts.num_ints = max_tag_nums;
		  //  }

		  //  if (ts.num_reals < 0) {
		  //	  ts.num_reals = 0;
		  //  }
		  //  if (ts.num_reals > max_tag_nums) {
		  //	  ts.num_reals = max_tag_nums;
		  //  }

		  //  if (ts.num_strings < 0) {
		  //	  ts.num_strings = 0;
		  //  }
		  //  if (ts.num_strings > max_tag_nums) {
		  //	  ts.num_strings = max_tag_nums;
		  //  }

		  //  tag.intValues.resize(static_cast<size_t>(ts.num_ints));

		  //  for (size_t i = 0; i < static_cast<size_t>(ts.num_ints); ++i) {
		  //	  tag.intValues[i] = parseInt(&token);
		  //  }

		  //  tag.floatValues.resize(static_cast<size_t>(ts.num_reals));
		  //  for (size_t i = 0; i < static_cast<size_t>(ts.num_reals); ++i) {
		  //	  tag.floatValues[i] = parseReal(&token);
		  //  }

		  //  tag.stringValues.resize(static_cast<size_t>(ts.num_strings));
		  //  for (size_t i = 0; i < static_cast<size_t>(ts.num_strings); ++i) {
		  //	  tag.stringValues[i] = parseString(&token);
		  //  }

		  //  tags.push_back(tag);

		  //  continue;
	   // }

	   // if (token[0] == 's' && IS_SPACE(token[1])) {
		  //  // smoothing group id
		  //  token += 2;

		  //  // skip space.
		  //  token += strspn(token, " \t");  // skip space

		  //  if (token[0] == '\0') {
		  //	  continue;
		  //  }

		  //  if (token[0] == '\r' || token[1] == '\n') {
		  //	  continue;
		  //  }

		  //  if (strlen(token) >= 3) {
		  //	  if (token[0] == 'o' && token[1] == 'f' && token[2] == 'f') {
		  //		  current_smoothing_id = 0;
		  //	  }
		  //  }
		  //  else {
		  //	  // assume number
		  //	  int smGroupId = parseInt(&token);
		  //	  if (smGroupId < 0) {
		  //		  // parse error. force set to 0.
		  //		  // FIXME(syoyo): Report warning.
		  //		  current_smoothing_id = 0;
		  //	  }
		  //	  else {
		  //		  current_smoothing_id = static_cast<unsigned int>(smGroupId);
		  //	  }
		  //  }

		  //  continue;
	   // }  // smoothing group id

	   // // Ignore unknown command.
	//}

	// not all vertices have colors, no default colors desired? -> clear colors
		if (!found_all_colors/* && !default_vcols_fallback*/) {
			vc.clear();
		}

		//if (greatest_v_idx >= static_cast<int>(v.size() / 3)) {
		   // if (warn) {
			  //  std::stringstream ss;
			  //  ss << "Vertex indices out of bounds (line " << line_num << ".)\n"
			  //	  << std::endl;
			  //  (*warn) += ss.str();
		   // }
		//}
		//if (greatest_vn_idx >= static_cast<int>(vn.size() / 3)) {
		   // if (warn) {
			  //  std::stringstream ss;
			  //  ss << "Vertex normal indices out of bounds (line " << line_num << ".)\n"
			  //	  << std::endl;
			  //  (*warn) += ss.str();
		   // }
		//}
		//if (greatest_vt_idx >= static_cast<int>(vt.size() / 2)) {
		   // if (warn) {
			  //  std::stringstream ss;
			  //  ss << "Vertex texcoord indices out of bounds (line " << line_num << ".)\n"
			  //	  << std::endl;
			  //  (*warn) += ss.str();
		   // }
		//}

		//bool ret = exportGroupsToShape(&shape, faceGroup, lineGroup, tags, material,
		   // name, true, v);

		// exportGroupsToShape return false when `usemtl` is called in the last
		// line.
		// we also add `shape` to `shapes` when `shape.mesh` has already some
		// faces(indices)
		if (shape.mesh.indices.size()) {
			shapes->push_back(shape);
		}
		faceGroup.clear();  // for safety


		attrib->vertices.swap(v);
		attrib->normals.swap(vn);
		attrib->texcoords.swap(vt);
		attrib->colors.swap(vc);








		std::cout << "Mesh loaded!!!!" << std::endl;

	}
}

ObjLoader::~ObjLoader()
{

}
