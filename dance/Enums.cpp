#ifndef Enums_c
#define Enums_c

//TODO: Change to enum class

enum result {r_ok, r_err, r_eof};
enum headings { north = 0, west, south, east, num_headings};
enum sensor {f_left = 0, c_left = 1, center = 2, c_right = 3, f_right = 4};
enum states{ st_parsing = 0, st_driving};
enum speeds{ stopped = 0, slow = 30, medium = 50, faster = 80, fast = 100};

#endif // Enums_c