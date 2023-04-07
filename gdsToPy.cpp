#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <bits/stdc++.h>

//#define DIV 1
#define DIV 1 // this sets the precision! 1 is full, 100 is a good compremise: 7500, 11400 => 75, 114A

//std::string myFile{ "nand2.txt" };
std::string myFile{ "myexample.txt" };

typedef std::string str;

struct Points {
	int x, y;
	int layer;

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
	std::vector < Points > complete_xy;
	bool hasSref = false;
	bool referenceMask {0}; // if a structure is ever referenced, that means it has no
	// absolute position, mask would be set to 1. The xy will be added to the sum of the xy

};


	std::vector < Structure > g_Structures;
	std::map < std::string, int > index_map;
	BoundingBox WorldSpaceBB;

	bool expandReferences(std::string name, int x_offset, int y_offset, std::vector<Points>& complete_xy) { // complete_xy stays on the original structure
		Structure current_structure = g_Structures[index_map[name]];

		//apply the offset to all structures boundaries, then push into complete_xy_pos of structure
		
		for (auto i : current_structure._polygons) {
			for (auto j : i.xy_pos) {
				j.x += x_offset;
				j.y += y_offset;
				complete_xy.push_back(j);
			}
		}


		if (current_structure.hasSref) {
		
			for (int i = 0; i < current_structure._references.size(); i++) {
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


	bool evenOdd(Points	pcheck, std::vector< Points > poly_points) { // true if in corner, on boundary, or inside, else false
		bool res = false;
		for (int i = 0; i < poly_points.size() - 1; i++) { // number of edges of a poly is -1 then points
			Points edge_start = poly_points[i];
			Points edge_end = poly_points[i+1];
			if ((pcheck.x == edge_end.x) && (pcheck.y == edge_end.y)) { // corner
				//printf("corner\n");
				return true;
			}
			if ((edge_start.y > pcheck.y) != (edge_end.y > pcheck.y)) {
				//printf("edge\n");
				//int slope = ((pcheck.x - edge_end.x) * (edge_start.y - edge_end.y) - (edge_start.x - edge_end.x) * (pcheck.y - edge_end.y));
				int slope = ((pcheck.x - edge_end.x) * (edge_start.y - edge_end.y) - (edge_start.x - edge_end.x) * (pcheck.y - edge_end.y));
				//printf("pcheck: (%d, %d), edge_start: (%d, %d), edge_end(%d, %d) \n", pcheck.x, pcheck.y, edge_start.x, edge_start.y, edge_end.x, edge_end.y);
				//printf("slope: %d\n", slope);
				if (slope == 0) { // horizontal
					//printf("slope check: (%d, %d)\n", pcheck.x, pcheck.y);
					return true;
				}
				if ((slope < 0) != (edge_start.y < edge_end.y)) {

					res = !res;
				}
			}
		}

		return res;
	}

	bool fillPolygons() { // even-odd

		for (auto& i : g_Structures) {
			for (auto& j : i._polygons) {
				//creating new Points, dont forget the layers
				Points pos{};
				pos.x = j._bounding_box.left.x;
				pos.y = j._bounding_box.bottom.y;
				//if (i._name == "\"via\"") {
					for (; pos.y <= j._bounding_box.top.y; pos.y++) {
						for (; pos.x <= j._bounding_box.right.x; pos.x++) { // honestly could have a jump of 10 for less looping
							//printf("pos checking beans: (%d, %d)\n", pos.x, pos.y);
							if (evenOdd(pos, j.xy_pos)) {
								// add to _fill of the polygon
								j._fill.push_back(pos);
								printf("fill: (%d, %d)\n", pos.x, pos.y);
							}
						}
						pos.x = j._bounding_box.left.x; // RESET X BACK TO LEFT
					//	printf("pos.y: %d", pos.y);
					}
			//	}

			}

		}
		return true;
	}

	bool adjustToCorner(int low, int left) { // index requires 0,0,0 at corner, add the world space bottom.y and left.x to all xy in complete_xy
		for (auto& i : g_Structures) {
			for (auto& j : i.complete_xy) {
				j.x += left; // offsets to corner
				j.y += low;
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
				printf("Beginning new Structure\n");
				char name[512];
				in >> name;
				printf("%s\n", name);

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
					printf("\tFound Reference: \n");
					Points xy{};
					References ref{};

					new_structure.hasSref = true;

					temp_stream >> ref.name;
					printf("\tref name: %s", ref.name.data());

					file.getline(lineBuffer, 512); // gets new line into lineBuffer
					std::istringstream temp_stream_sname(lineBuffer); // make new istringstream on lineBuffer
					temp_stream_sname.getline(key, 512, ':');

					char ctemp1, ctemp2; // this absorbs the ',' after each value
					temp_stream_sname >> xy.x >> ctemp1 >> xy.y >> ctemp2; // reference offsets
					xy.x /= DIV;
					xy.y /= DIV;
					printf("\tpoints: %d, %d\n", xy.x, xy.y);
					ref.xy = xy;

					
					}
					if (strcmp(key, "BOUNDARY") == 0) { // BOUNDARY (polygon)
					printf("\tFound Polygon: \n");
					Polygon new_poly{};
					Points xy{};
					g_Structures[struct_index]._polygons.push_back(new_poly);
					int poly_index = g_Structures[struct_index]._polygons.size() - 1;
					file.getline(lineBuffer, 512); // gets new line into lineBuffer
					std::istringstream temp_stream_boundary(lineBuffer); // make new istringstream on lineBuffer
					temp_stream_boundary.getline(key, 512, ':');

					temp_stream_boundary >> xy.layer;
					printf("\tlayer: %d\n", xy.layer);

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
							g_Structures[struct_index]._polygons[poly_index].xy_pos.push_back(xy);
						}
					}
					printf("\txy: ");
					for (auto i : g_Structures[struct_index]._polygons[poly_index].xy_pos) {
						printf("(%d, %d), ", i.x, i.y);
					}
					printf("\n");
					
					}
				}

			}

		}
		
		return true;

	};

int main(){

	bool res = handleFile();
	if (!res) { printf("error happened in handleFile"); };

	for (int i = 0; i < g_Structures.size(); i++) {
		res = expandReferences(g_Structures[i]._name, 0, 0, g_Structures[i].complete_xy);
	}

	//res = findMinMaxWorldSpace(bbox_top, bbox_bottom, bbox_left, bbox_right);
	res = findMinMaxWorldSpace(WorldSpaceBB.top, WorldSpaceBB.bottom, WorldSpaceBB.left, WorldSpaceBB.right);

	printf("world space: top: (%d, %d) bottom: (%d, %d) left: (%d, %d) right: (%d, %d)\n", WorldSpaceBB.top.x, WorldSpaceBB.top.y,
		WorldSpaceBB.bottom.x, WorldSpaceBB.bottom.y, WorldSpaceBB.left.x, WorldSpaceBB.left.y, WorldSpaceBB.right.x, WorldSpaceBB.right.y);
	
	printf("hello world\n");

	printf("size of g_Structures: %ld\n", g_Structures.size());

	res = setBBoxPolys();

	res = fillPolygons();

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
