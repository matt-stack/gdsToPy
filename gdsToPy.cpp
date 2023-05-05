#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <cmath>

#define DIV 1
//#define DIV 100 // this sets the precision! 1 is full, 100 is a good compremise: 7500, 11400 => 75, 114A
#define DEBUG 1

//std::string myFile{ "nand2.txt" };
//std::string myFile{ "UniExample/UniExample.txt" };
//std::string myFile{ "myexample.txt" };
std::string myFile{ "myexamplebox.txt" };

typedef std::string str;

struct Points {
	int x, y;
	int layer;
	bool onEdge = false;

};

struct Path {

	//int _layer; layer is found in _xy[0].layer (assuming its not zero size)
	int _pathtype;
	int _width;
	std::vector <Points> _fill;
	std::vector < Points> _xy; // relative xy
};

struct References {
	str name; // have a str "name" / Structure map key/value setup to index into
	Points xy; // placement
	std::vector<int> new_placement;
};

struct BoundingBox {
	Points top; // for these we only care about y for top and bottom
	Points bottom;
	Points left; // and Points.x for left and right
	Points right;

	void print() {
		printf("Bounding box: top(%d, %d), bottom(%d, %d), left(%d, %d), right(%d, %d)\n", top.x, top.y,
			bottom.x, bottom.y, left.x, left.y, right.x, right.y);
	}
};

struct Polygon {

	std::vector< Points > xy_pos; // first pair and last pair need to match
	BoundingBox _bounding_box;
	std::vector< Points > _fill;
};

struct Structure {

	std::string _name;
	std::vector < References > _references; // srefs
	std::vector < Polygon > _polygons; // boundaries, std::vec of std::vec
	std::vector < Path > _paths;
	std::vector < Points > complete_xy;
	bool hasSref{ false };
	bool referenceMask {false}; // if a structure is ever referenced, that means it has no
	// absolute position, mask would be set to 1. The xy will be added to the sum of the xy

};


	// globals
	std::vector < Structure > g_Structures;
	std::map < std::string, int > index_map;
	BoundingBox WorldSpaceBB;
	int g_lowest_layer = INT_MAX;
	int g_highest_layer = INT_MIN;

	bool expandReferences(std::string name, int x_offset, int y_offset, std::vector<Points>& complete_xy) { // complete_xy stays on the original structure
		Structure current_structure = g_Structures[index_map[name]];

		//apply the offset to all structures boundaries, then push into complete_xy_pos of structure
		
		for (auto i : current_structure._polygons) {
			//for (auto j : i.xy_pos) {
			for (auto j : i._fill) {
				j.x += x_offset;
				j.y += y_offset;
				complete_xy.push_back(j); // this fills the complete_xy
			}
		}

		for (auto i : current_structure._paths) {
			printf("\tHello?\n");
			for (auto j : i._fill) {
			printf("\tyes\n");
				j.x += x_offset;
				j.y += y_offset;
				complete_xy.push_back(j); // this fills the complete_xyA
				printf("\tAdding point: (%d, %d)\n", j.x, j.y);
				
			}
		}

			//	if (name == "\"abc2\"") {
			//		printf("HELLLLOOOO\n");
			//	}

		if (current_structure.hasSref) {
			for (int i = 0; i < current_structure._references.size(); i++) {
				if (DEBUG) {
					printf("printing structure: %s\n", current_structure._name.data());
				}
				expandReferences(current_structure._references[i].name,
					(current_structure._references[i].xy.x + x_offset),
					(current_structure._references[i].xy.y + y_offset),
					complete_xy); // complete_xy stays the same
			}
		}
		return true;

	};

	bool findMinMaxWorldSpace(Points& top, Points& bottom, Points& left, Points& right) { // at some point need to loop through all structures and get the min and max corners of the absolutes
		Points current_top = { 0 , INT_MIN };
		Points current_bottom = { 0, INT_MAX };
		Points current_left = { INT_MAX, 0};
		Points current_right = { INT_MIN, 0 }; // though we are only ever checking the x of right and left, and the y of top and bottom
		for (auto i : g_Structures) {
				//printf("J::: %d\n", i.complete_xy[0].x);
			for (auto j : i.complete_xy) {
				if (j.y > current_top.y) {
					current_top = j;
				}
				if (j.y < current_bottom.y) {
					current_bottom = j;
				}
				if (j.x < current_left.x) {
					current_left = j;
				}
				if (j.x > current_right.x) {
					current_right = j;
				}
			}
		}
		top = current_top;
		bottom = current_bottom;
		left = current_left;
		right = current_right;

		return true;
	}

	bool setBBoxPolys() {
		for (auto& i : g_Structures) {
			for (auto& j : i._polygons) {
				Points current_top = { 0 , INT_MIN };
				Points current_bottom = { 0, INT_MAX };
				Points current_left = { INT_MAX, 0 };
				Points current_right = { INT_MIN, 0 }; // though we are only ever checking the x of right and left, and the y of top and bottom
				//printf("J::: %d\n", i.complete_xy[0].x);
				for (auto k : j.xy_pos) {
					if (k.y > current_top.y) {
						current_top = k;
					}
					if (k.y < current_bottom.y) {
						current_bottom = k;
					}
					if (k.x < current_left.x) {
						current_left = k;
					}
					if (k.x > current_right.x) {
						current_right = k;
					}
				}

				j._bounding_box.top = current_top;
				j._bounding_box.bottom = current_bottom;
				j._bounding_box.left= current_left;
				j._bounding_box.right = current_right;
				j._bounding_box.print();
				
			}
		}
		return true;
	}

// having a bool return and a bool hidden return is bad design but..
	bool evenOdd(Points	pcheck, std::vector< Points > poly_points, bool& onEdge) { // true if in corner, on boundary, or inside, else false
		bool res = false;
		for (int i = 0; i < poly_points.size() - 1; i++) { // number of edges of a poly is -1 then points
			Points edge_start = poly_points[i];
			Points edge_end = poly_points[i+1];
			if ((pcheck.x == edge_end.x) && (pcheck.y == edge_end.y)) { // corner
				//printf("corner\n");
				//printf("ON CORNER!: %d, %d\n", pcheck.x, pcheck.y);
				onEdge = true;
				return true;
			}
			if ((edge_start.y > pcheck.y) != (edge_end.y > pcheck.y)) {
				//printf("edge\n");
				//int slope = ((pcheck.x - edge_end.x) * (edge_start.y - edge_end.y) - (edge_start.x - edge_end.x) * (pcheck.y - edge_end.y));
				int slope = ((pcheck.x - edge_end.x) * (edge_start.y - edge_end.y) - (edge_start.x - edge_end.x) * (pcheck.y - edge_end.y));
				//printf("pcheck: (%d, %d), edge_start: (%d, %d), edge_end(%d, %d) \n", pcheck.x, pcheck.y, edge_start.x, edge_start.y, edge_end.x, edge_end.y);
				//printf("slope: %d\n", slope);
				if (slope == 0) { // edge, horizontal
					if (DEBUG) {
						//printf("slope check: (%d, %d)\n", pcheck.x, pcheck.y);
					}
					onEdge = true;
					return true;
				}
				if ((slope < 0) != (edge_start.y < edge_end.y)) {

					res = !res;
				}
			}
			if ((((edge_end.x <= pcheck.x)  && (pcheck.x <= edge_start.x)) || ((edge_start.x <= pcheck.x) && (pcheck.x <= edge_end.x))) && (edge_end.y == pcheck.y) && (pcheck.y == edge_start.y)) {
				//printf("ON H EDGE!: pcheck (%d, %d), edge_start (%d, %d), edge_end (%d, %d)\n", pcheck.x, pcheck.y, edge_start.x, edge_start.y, edge_end.x, edge_end.y);
				onEdge = true;
				return true;

			}
		}

		return res;
	}

	bool fillPolygons() { // even-odd polygons

		for (auto& i : g_Structures) {
			for (auto& j : i._polygons) {
				Points pos{};
				pos.x = j._bounding_box.left.x;
				pos.y = j._bounding_box.bottom.y; // + and - 1 for room to breath
					for (; pos.y <= j._bounding_box.top.y; pos.y++) {
						for (; pos.x <= j._bounding_box.right.x; pos.x++) { // honestly could have a jump of 10 for less loopingA
							bool onEdge = false;
							if (evenOdd(pos, j.xy_pos, onEdge)) {
								// add to _fill of the polygon
								pos.layer = j.xy_pos[0].layer; // copy layer of first point in polygon
								pos.onEdge = onEdge;
								j._fill.push_back(pos);
								//printf("fill: (%d, %d)\n", pos.x, pos.y);
							}
						}
						pos.x = j._bounding_box.left.x; // RESET X BACK TO LEFT
					}
			}

		}
		return true;
	}


	bool fillPaths(Path& currentPath, std::vector<Points> temp_poly, BoundingBox bbox) { // even-odd paths

		Points pos{};
		pos.x = bbox.left.x;
		pos.y = bbox.bottom.y; // + and - 1 for room to breath
			for (; pos.y <= bbox.top.y; pos.y++) {
				for (; pos.x <= bbox.right.x; pos.x++) { // honestly could have a jump of 10 for less loopingA
					bool onEdge = false;
					if (evenOdd(pos, temp_poly, onEdge)) {
						// add to _fill of the polygon
						pos.layer = temp_poly[0].layer; // copy layer of first point in polygon
						pos.onEdge = true; // paths are always an edge 
						currentPath._fill.push_back(pos);
						printf("\t\tPATH FILL: (%d, %d)\n", pos.x, pos.y);
					}
				}
				pos.x = bbox.left.x; // RESET X BACK TO LEFT
			}
		return true;
	}

	float slope(Points p1, Points p2) {
		float x1 = p1.x;
		float x2 = p2.x;
		float y1 = p1.y;
		float y2 = p2.y;
		if (x2 - x1 != 0) {
			return ((y2 - y1) / (x2 - x1)); // y2-y1 / x2-x1
		}
		return FLT_MAX; // undefined
	}

	std::tuple<Points, Points, int, int> pointsGivenSlopeAndDistance(Points origin, float slope, int distance) {
		// returns new points with midpoint width across
		// recall: this slope is the negated 1/x of the origin slope
		// alg borrowed  from: https://www.geeksforgeeks.org/find-points-at-a-given-distance-on-a-line-of-given-slope/
		Points end1, end2;
		end1.layer = origin.layer;
		end2.layer = origin.layer;
		int dx_r;
		int dy_r;

		// slope is 0, horizonatal
		if (slope == 0) {
			dx_r = distance;
			dy_r = 0;
			end1.x = origin.x + dx_r;
			end1.y = origin.y; // y is static
			end2.x = origin.x - dx_r;
			end2.y = origin.y;
		}

		// verticle line
		else if (slope == FLT_MAX) {
			dx_r = 0;
			dy_r = distance;
			end1.x = origin.x; // x is static 
			end1.y = origin.y + distance;
			end2.x = origin.x;
			end2.y = origin.y - distance;
		}
		else { 
			float dx = (distance / sqrt(1 + (slope * slope)));
			float dy = slope * dx;
			dx_r = std::round(dx);
			dy_r = std::round(dy);
			end1.x = origin.x + dx_r;
			end1.y = origin.y + dy_r;
			end2.x = origin.x - dx_r;
			end2.y = origin.y - dy_r;
		
		}

		if (DEBUG) {
			std::printf("Origin: (%d, %d), Width: (%d), New Points: (%d, %d), (%d, %d)\n", origin.x, origin.y, distance, end1.x, end1.y, end2.x, end2.y);
		}
		std::tuple<Points, Points, int, int> temp = std::make_tuple(end1, end2, dx_r, dy_r);
		return temp;

	}

	float negateSlope(float slope) {
		float new_slope;
		if (slope == FLT_MAX) {
			new_slope = 0;
		}
		else if (slope == 0) {
			new_slope = FLT_MAX;
		}
		else {
			new_slope = -1 * (1 / slope);
		}
		return new_slope;
	}

	bool handlePaths() {
		for (auto& i : g_Structures) {
			for (auto& j : i._paths) {
				printf("size: %d\n", j._xy.size());
				for (int i = 0; i < j._xy.size()-1; i++) {

					float s = slope(j._xy[i], j._xy[i + 1]);
					// Negative reciprocal slope

					float nr_slope = negateSlope(s);
					if (DEBUG) {
						printf("NR slope: %f\n", nr_slope);
					}
					std::tuple<Points, Points, int, int> topPairWithWidthAndDXDY = pointsGivenSlopeAndDistance(j._xy[i], nr_slope, j._width / 2);
					int dx = std::get<2>(topPairWithWidthAndDXDY);
					int dy = std::get<3>(topPairWithWidthAndDXDY);

					Points corner1 = std::get<0>(topPairWithWidthAndDXDY);
					Points corner2 = std::get<1>(topPairWithWidthAndDXDY);
					Points corner3 = j._xy[i + 1];
					corner3.x -= dx;
					corner3.y -= dy;
					Points corner4 = j._xy[i + 1];
					corner4.x += dx;
					corner4.y += dy;

					if (DEBUG) {
						printf("corner1: (%d, %d), corner2: (%d, %d), corner3: (%d, %d), corner4: (%d, %d)\n", 
							corner1.x, corner1.y, corner2.x, corner2.y, corner3.x, corner3.y, corner4.x, corner4.y);
					}

					std::vector<Points> temp_poly{ corner1, corner2, corner3, corner4, corner1 }; //end at same point
					BoundingBox bbox;

					Points current_top = { 0 , INT_MIN };
					Points current_bottom = { 0, INT_MAX };
					Points current_left = { INT_MAX, 0 };
					Points current_right = { INT_MIN, 0 }; // though we are only ever checking the x of right and left, and the y of top and bottom
					//printf("J::: %d\n", i.complete_xy[0].x);
					for (auto k : temp_poly) {
						if (k.y > current_top.y) {
							current_top = k;
						}
						if (k.y < current_bottom.y) {
							current_bottom = k;
						}
						if (k.x < current_left.x) {
							current_left = k;
						}
						if (k.x > current_right.x) {
							current_right = k;
						}
					}

					bbox.top = current_top;
					bbox.bottom = current_bottom;
					bbox.left= current_left;
					bbox.right = current_right;
					
					for (auto& i : g_Structures) {
						for (auto& j : i._paths) {
							bool res = fillPaths(j, temp_poly, bbox);
							printf("filling: %d, %d\n", j._fill[0].x, j._fill[0].y);
						}
					}
						// fill _fill
				}
			}
		}
		// inverse slope
		return true;
	}


	// This is currently unused, edge darkening happens in EvenOdd alg, works with non-axis aligned shapes, but this one is axis-aligned
/*	bool darkenEdge() {
		for (auto& i : g_Structures) {
			for (auto& j : i._polygons) {
				for (auto& k : j._fill) { // currently this only works because nand2 is only squares, bounding box are axis-aligned
					if ((k.x == j._bounding_box.left.x) ||
						(k.x == j._bounding_box.right.x) ||
						(k.y == j._bounding_box.top.y) ||
						(k.y == j._bounding_box.bottom.y)) {
						// on bounding box, yes edge
						k.onEdge = true;
					}
					//else if ((k+1).x == 0) {
						
					//}
				}
			}
		}
		return true;
	} */

	bool adjustToCorner(int low, int left) { // index requires 0,0,0 at corner, add the world space bottom.y and left.x to all xy in complete_xy
		if (low > 0 && left > 0) { // all xy are positive, need to subtract to get to origin
			low *= -1;
			left *= -1;
		}
		if (low > 0 && left < 0) { // left is negative
			low *= -1;
			left *= 1;
		}
		if (low < 0 && left > 0) { // low is negative
			low *= 1;
			left *= -1;
		}
		if (low < 0 && left < 0) { // both are negative
			low *= 1;
			left *= 1;
		}
		for (auto& i : g_Structures) {
			for (auto& j : i.complete_xy) {
				j.x -= left; // offsets to corner
				j.y -= low;
			}
		}
		return true;
	}


	//bool handleFile(const char* buffer) {
	bool handleFile() {
		std::ifstream file(myFile);
		if (!file) { printf("No File found!\n"); };
		if (file) { printf("File found!\n"); };
		char buffer[512];
		while (file.getline(buffer, 512)) {
			//file.getline(buffer, 512);
			std::istringstream in(buffer);
			//	std::string key;
			char key[512];

			in.getline(key, 512, ':');
			if (strcmp(key, "STRNAME") == 0) { // then we are beginning a new structure
				if (DEBUG) {
					printf("Beginning new Structure\n");
				}
				char name[512];
				in >> name;
				if (DEBUG) {
					printf("%s\n", name);
				}
				Structure new_structure{};
				new_structure._name = name;
				g_Structures.push_back(new_structure);
				int struct_index = g_Structures.size() - 1; // sequential, so strucutres are added in order
				index_map.insert(std::pair<std::string, int>(new_structure._name, struct_index));

				// two paths, SREF aka SNAME and BOUNDARY
				while (strcmp(key, "ENDSTR") != 0) {
					char lineBuffer[512];
					file.getline(lineBuffer, 512);
					std::istringstream temp_stream(lineBuffer);
					temp_stream.getline(key, 512, ':');

					if (strcmp(key, "SNAME") == 0) { // SNAME (reference)
						if (DEBUG) {
							printf("\tFound Reference: \n");
						}
						Points xy{};
						References ref{};
							
						g_Structures[struct_index].hasSref = true;

						temp_stream >> ref.name;
						if (DEBUG) {
							printf("\tref name: %s", ref.name.data());
						}

						g_Structures[index_map[ref.name]].referenceMask = true; // if a structure is reference, mask is true

						file.getline(lineBuffer, 512); // gets new line into lineBuffer
						std::istringstream temp_stream_sname(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_sname.getline(key, 512, ':');

						char ctemp1, ctemp2; // this absorbs the ',' after each value
						temp_stream_sname >> xy.x >> ctemp1 >> xy.y >> ctemp2; // reference offsets
						xy.x /= DIV;
						xy.y /= DIV;
						if (DEBUG) {
							printf("\tpoints: %d, %d\n", xy.x, xy.y);
						}
						ref.xy = xy;

						g_Structures[struct_index]._references.push_back(ref);

				
					} // SNAME (REFERENCE)
					if (strcmp(key, "BOUNDARY") == 0) { // BOUNDARY (polygon)
						if (DEBUG) {
							printf("\tFound Polygon: \n");
						}
						Polygon new_poly{};
						Points xy{};
						g_Structures[struct_index]._polygons.push_back(new_poly);
						int poly_index = g_Structures[struct_index]._polygons.size() - 1; 
						file.getline(lineBuffer, 512); // gets new line into lineBuffer
						std::istringstream temp_stream_boundary(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_boundary.getline(key, 512, ':');
	
						temp_stream_boundary >> xy.layer;
						if (DEBUG) {
							printf("\tlayer: %d\n", xy.layer);
						}
	
						// set lowest and highest layers
						if (xy.layer < g_lowest_layer) {
							g_lowest_layer = xy.layer;
						}
						if (xy.layer > g_highest_layer) {
							g_highest_layer = xy.layer;
						}
	
						file.getline(lineBuffer, 512); // skipping the datatype line
	
						file.getline(lineBuffer, 512); // XY line
						std::istringstream temp_stream_xy(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_xy.getline(key, 512, ':');
						
						if (strcmp(key, "XY") == 0) {
							while (temp_stream_xy) {
								char ctemp1, ctemp2;
								temp_stream_xy >> xy.x >> ctemp1 >> xy.y >> ctemp2;
								xy.x /= DIV;
								xy.y /= DIV;
								xy.onEdge = true; // this gets overwritten by fill :(
								g_Structures[struct_index]._polygons[poly_index].xy_pos.push_back(xy);
							}
						}
						if (DEBUG) {
							printf("\txy: ");
							for (auto i : g_Structures[struct_index]._polygons[poly_index].xy_pos) {
								printf("(%d, %d), ", i.x, i.y);
						}
						printf("\n");
						}
					
					} // BOUNDARY
					if (strcmp(key, "PATH") == 0) { // PATH
						if (DEBUG) {
							printf("\tFound Path: \n");
						}
						Path new_path{};
						Points xy{};
						bool skip = false; // PATH_TYPE is optional and sometimes skipped

						file.getline(lineBuffer, 512); // gets new line into lineBuffer
						std::istringstream temp_stream_pathlayer(lineBuffer); // make new istringstream on lineBuffer

						temp_stream_pathlayer.getline(key, 512, ':');
	
						temp_stream_pathlayer >> xy.layer;
						if (DEBUG) {
							printf("\tlayer: %d\n", xy.layer);
						}
	
						file.getline(lineBuffer, 512); // skipping the datatype line
						std::istringstream temp_stream_datatype(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_datatype.getline(key, 512, ':');

						if (strcmp(key, "DATATYPE") == 0) {
							// do nothing
						}
						
						file.getline(lineBuffer, 512); // pathtype line
						std::istringstream temp_stream_pathtype(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_pathtype.getline(key, 512, ':');

						if (strcmp(key, "PATHTYPE") == 0) {
							temp_stream_pathtype >> new_path._pathtype;
							if (DEBUG) {
								printf("\tpathtype: %d\n", new_path._pathtype);
							}
						}

						file.getline(lineBuffer, 512); // width line
						std::istringstream temp_stream_width(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_width.getline(key, 512, ':');

						if (strcmp(key, "WIDTH") == 0) {
							int temp_width;
							temp_stream_width >> temp_width;
							temp_width /= DIV; // apply div
							new_path._width = temp_width;
							if (DEBUG) {
								printf("\twidth: %d\n", new_path._width);
							}
						}
	
						g_Structures[struct_index]._paths.push_back(new_path);
						int path_index = g_Structures[struct_index]._paths.size() - 1; 


						file.getline(lineBuffer, 512); // XY line
						std::istringstream temp_stream_xy(lineBuffer); // make new istringstream on lineBuffer
						temp_stream_xy.getline(key, 512, ':');
						
						if (strcmp(key, "XY") == 0) {
							while (temp_stream_xy) {
								char ctemp1, ctemp2;
								temp_stream_xy >> xy.x >> ctemp1 >> xy.y >> ctemp2;
								xy.x /= DIV;
								xy.y /= DIV;
								xy.onEdge = true; // this gets overwritten by fill :(
								g_Structures[struct_index]._paths[path_index]._xy.push_back(xy);
							}
						}
						if (DEBUG) {
							printf("\txy: ");
							for (auto i : g_Structures[struct_index]._paths[path_index]._xy) {
								printf("(%d, %d), ", i.x, i.y);
						}
						printf("\n");
						}

					
					} // PATH
				}

			}

		}
		
		return true;

	};

	bool setTheWorldVector(std::vector<std::vector<std::vector<int>>>& world, int u, int v) {
		// set the base
		for (int i = 0; i < u; i++) {
			for (int j = 0; j < v; j++) {
				//world[i][j][0] = 300;
			}
		}

		for (auto i : g_Structures) {
			if (i.referenceMask != 1) {
				printf("setting world vector\n");
				int random = rand() % 1000 + 100;
				for (auto j : i.complete_xy) {
					if (j.onEdge) {
						//world[j.layer][j.y][j.x] = j.layer * 2 +500; // make edges extra visible
						world[j.layer][j.y][j.x] = 1; // make edges extra visible
					}
					else {
						//world[j.layer][j.y][j.x] = j.layer;
						world[j.layer][j.y][j.x] = j.layer * 2;
					}
				}
			}
		}
		return true;
	}

	bool printToFile(const std::vector<std::vector<std::vector<int>>>& world) {
		// need to jump around vectors, say goodbye to caching
		for (int o = 0; o < world.size(); o++) {
			//printf("%ld\n", world[0][0].size());
			std::string filename {"fileout_"};
			filename += std::to_string(o);
			filename += ".txt";
			std::ofstream levelfile(filename);
			//for (int i = 0; i < world[0].size(); i++) {
			for (int j = 0; j < world[0].size(); j++) { // constant size
				for (int i = 0; i < world[0][0].size(); i++) {
					//printf("world sizes: %ld, %ld, %ld\n", world.size(), world[0].size(), world[0][0].size());
					levelfile << world[o][j][i] << " ";
				}
				levelfile << "\n";
			}

			levelfile.close();
		}
		return true;
	}

int main(){

	bool res = handleFile();
	if (!res) { printf("error happened in handleFile"); };

	res = setBBoxPolys();

	res = fillPolygons();

//	res = darkenEdges(); still working on this

	//PATHS
	printf("pre\n");
	res = handlePaths();
	printf("post\n");
	// end PATHS


	for (int i = 0; i < g_Structures.size(); i++) {
		res = expandReferences(g_Structures[i]._name, 0, 0, g_Structures[i].complete_xy);
	}

	//res = findMinMaxWorldSpace(bbox_top, bbox_bottom, bbox_left, bbox_right);
	res = findMinMaxWorldSpace(WorldSpaceBB.top, WorldSpaceBB.bottom, WorldSpaceBB.left, WorldSpaceBB.right);

	printf("world space: top: (%d, %d) bottom: (%d, %d) left: (%d, %d) right: (%d, %d)\n", WorldSpaceBB.top.x, WorldSpaceBB.top.y,
		WorldSpaceBB.bottom.x, WorldSpaceBB.bottom.y, WorldSpaceBB.left.x, WorldSpaceBB.left.y, WorldSpaceBB.right.x, WorldSpaceBB.right.y);
	
	printf("hello world\n");

	printf("size of g_Structures: %ld\n", g_Structures.size());

// check reference masks are correct
	for (auto i : g_Structures) {
		printf("%s, reference mask %d\n", i._name.data(), i.referenceMask);
	}

	printf("lowerest layer: %d\n", g_lowest_layer);
	printf("higherest layer: %d\n", g_highest_layer);

	res = adjustToCorner(WorldSpaceBB.bottom.y, WorldSpaceBB.left.x);

	BoundingBox adjustedBB;

	res = findMinMaxWorldSpace(adjustedBB.top, adjustedBB.bottom, adjustedBB.left, adjustedBB.right);

	printf("adjusted space post adjust: top: (%d, %d) bottom: (%d, %d) left: (%d, %d) right: (%d, %d)\n", adjustedBB.top.x, adjustedBB.top.y,
		adjustedBB.bottom.x, adjustedBB.bottom.y, adjustedBB.left.x, adjustedBB.left.y, adjustedBB.right.x, adjustedBB.right.y);

	//x size = worldspaceBB.right.x
	//y size = worldspaceBB.top.y
	//z size = highest layer 
	int x = adjustedBB.right.x+1;
	int y = adjustedBB.top.y+1;
	int z = g_highest_layer+1;
	printf("size of world: %d, %d, %d\n", x, y, z);
	//std::vector < std::vector< std::vector < int > > > world (x, std::vector<std::vector<int>>(y, std::vector<int>(z)));
	std::vector < std::vector< std::vector < int > > > world (z, std::vector<std::vector<int>>(y, std::vector<int>(x)));

	/*
	//PATHS
	printf("pre\n");
	res = handlePaths();
	printf("post\n");
	// end PATHS
	*/

	res = setTheWorldVector(world, x, y);

	res = printToFile(world);

	printf("Done!\n");


	// then go through each structures fill, and each the world with
	// world[x] = layer // no x offset, starts a 0
	// world[y+y_offset] = layer;
	// world[layer+layer_offset] = layer;

	/*
	remaining items:
	set the lowest layer
	set a fill to the gorund layer for reference, world space bounding box size, lowest layer
	offset to edge

	final: go through every strucutres complete_xy and map into the world 2d vector, 1 if present and !

	*/

	// THEN add the fill to world space and the strucutres complete_xy

	// dont forget to adjust all complete_xy by the new origin, recenter

	// stream all info into strucutre vectors, store each structure in a map, store all
	// referenced strucutres in a serperate buffer
	// use seperate buffer of references to adjust mask bits for structures
	// get absolute position of each polygon in each structure
	// size of the world is the smallest x, y, largest x, y
	// fill the polygons with even-odd alg and bounding boxs
	// put a 1 in a world size 2d vector if there is a pos in abs space
	// 

return 0;
}
