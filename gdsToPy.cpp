#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <map>

typedef std::string str;

struct Points {
	int x, y;
	int layer;
};

struct References {
	str name; // have a str "name" / Structure map key/value setup to index into
	Points xy; // placement
	std::vector<int> new_placement;


	int getAbsolutePos(Points pos, std::string name) {
		//	new_pos = pos + struct
		//		store in new_placement
		return 3;
	}
};

struct Polygon {

	std::vector< Points > xy_pos; // first pair and last pair need to match
	std::vector< Points > _bounding_box;



	void fillPoly() {
		//even - odd alg
		// 	   uses _bounding_box
		//reference the edges vec
		//create bounding box over polygon
		//check all points in the bounding box against edges
	}

};

struct Structure {

	std::string _name;
	std::vector < References > _references; // srefs
	std::vector < Polygon > _polygons; // boundaries, std::vec of std::vec
	std::vector < Points > complete_xy;
	bool referenceMask {0}; // if a structure is ever referenced, that means it has no
	// absolute position, mask would be set to 1. The xy will be added to the sum of the xy

	bool hasSref() {
		//if _references is size 0 then no
		//else yes
		return false;
	}

};

void expandReferences(Structure s) { // recurrsive as an sref can reference a structure that has references
/*
	for (ref : _references) {
		std_vec <int> updated_pos = getAbsolutePos()
			add to _polygons


			if (reference has a sref) {
				recurse expandReferences() with the new struct that was found
			}


	}
	*/
};
	std::vector < Structure > g_Structures;
	std::map < std::string, int > index_map;

	bool handleStructure(std::istream& in, const char* name) {
		Structure new_struct{};
		new_struct._name = name;
		char key[108];
		std::vector<Points> temp_xy;
			in.getline(key, 108, ':');
			printf("key: %s\n", key);
		//while (strcmp(key, "ENDSTR") != 0) { // until end of structure

			in.getline(key, 108, ':');
			printf("key: %s\n", key);

			if (strcmp(key, "SNAME") == 0) {
				char name[108];

				in >> name;
				printf("Found reference: %s\n", name);
				in.getline(key, 108, ':');

				if (strcmp(key, "XY") == 0) {
					while (in) {
						//printf("hello");
						int x, y;
						char ctemp1, ctemp2; // this absorbs the ',' after each value
						in >> x >> ctemp1 >> y >> ctemp2;
						Points xy{ x, y };
						temp_xy.push_back(xy);
						//in.getline(currentLine, 512);

						//in >> value;
					}
				}
				for (auto i : temp_xy) {
					printf("point: %d, %d\n", i.x, i.y);
				}


			}
		//}
		return true;

	};

	//bool handleFile(const char* buffer) {
	bool handleFile() {
		std::ifstream file("nand2.txt");
		if (!file) { printf("No File found!\n"); };
		if (file) { printf("File found!\n"); };
		char buffer[512];
		while (file.getline(buffer, 512)) {
			//file.getline(buffer, 512);
			std::istringstream in(buffer);
			//	std::string key;
			char key[512];

			in.getline(key, 512, ':');
/*
			//in.getline(value, 108, ',');
			printf("KEY: %s\n", key);
			file.getline(lineBuffer, 512);
			std::istringstream temp_stream (lineBuffer);
			//in2.getline(key2, 108, ':');
			temp_stream >> key2;
			printf("KEY2: %s\n", key2);
*/
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
					file.getline(lineBuffer, 512); // gets new line into lineBuffer
					std::istringstream temp_stream_sname(lineBuffer); // make new istringstream on lineBuffer
					temp_stream_sname.getline(key, 512, ':');

					int x, y;
					char ctemp1, ctemp2; // this absorbs the ',' after each value
					temp_stream_sname >> x >> ctemp1 >> y >> ctemp2;
					Points xy{ x, y };
					printf("\tpoints: %d, %d\n", xy.x, xy.y);

					
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

			/*
			if (strcmp(key, "LAYER") == 0) {}
			*/
			/*
			if (strcmp(key, "SNAME") == 0) {
				char name[108];

				in >> name;
				printf("Found reference: %s\n", name);
			}

			if (strcmp(key, "XY") == 0) {
				while (in) {
					//printf("hello");
					int temp;
					char ctemp;
					in >> temp >> ctemp;
					values.push_back(temp);
					//in.getline(currentLine, 512);

					//in >> value;
				}

				printf("Key: %s, values: ", key);
				for (auto i : values) {
					printf("%d ", i);
				}
				printf("\n");
				*/
				//		}
		}
		
		return true;

	};

int main(){

	bool res = handleFile();
	if (!res) { printf("error happened in handleFile"); };
	/*
	std::ifstream file("nand2.txt");
	//file.open("nand2.txt");
	if (!file) { printf("No File found!\n"); };
	if (file) { printf("File found!\n"); };
	std::string cur_line;

	for (int i = 0; i < 50; i++) {
		char buffer[512];
		//file.getline(buffer, 512);
		//printf("%s\n", buffer);
		//bool res = handleFile(buffer);
		bool res = handleFile();
		if (!res) { printf("error happened in handleFile"); };
	}
*/
	
	printf("hello world\n");

	printf("size of g_Structures: %ld\n", g_Structures.size());


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
