# PathFinder_for_Dumbs_and_Me

This header-only library was created as a more basic implementation of MicroPather (https://github.com/leethomason/MicroPather).

The entire working interface is accessible via the "pthfd" namespace.
Pathfinding is performed by the "CDumbPather" structure, for which three interaction methods are defined:

1) Find(SPoint2u(&se)[2], CPathArray* path_class)
* For "SPoint2u" (defined on line 61), any structure containing `uint16_t x, y` that is compatible with the "CDumbPather" class may be used.
* "CPathArray" is a class for storing the found path; it holds a raw pointer to "SPoint2u" (for array initialization) and the array length.

2) SetByteMap(i_1b** map_data, u_2b map_size)
* `map_data` represents the graph transition matrix as a raw double pointer to `uint8_t`.
* The matrix dimensions must satisfy SizeX = SizeY; either SizeX or SizeY is passed as `map_size`.


P.S. I am self-taught. I would welcome criticism and advice.))
