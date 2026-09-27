# PathFinder_for_Dumbs_and_Me

This header-only library was created as a more basic implementation of MicroPather (https://github.com/leethomason/MicroPather).
The entire interface of the "dumbpather.h" file is available via the "pthfd" namespace.
________________________________________________________________________________________________________________________________
pthfd::TerrainType
    WALKABLE - int8_t num for map data matrix
    BLOKABLE - int8_t num for map data matrix
    SLOWABLE - int8_t num for map data matrix
________________________________________________________________________________________________________________________________
pthfd::CDumbPather interface
    FindPath(SPoint2u(&path_points)[2], CPathArray* path_segments) - Bild path on path_segments
    SetByteMap(i_1b** map_data, u_2b map_size)                     - Set world map
________________________________________________________________________________________________________________________________
pthfd::CPathArray interface
    SPoint2u& operator[](IntType i)  - !!! UNSAFE !!! get array element
    u_4b Length()                    - get array length
    Resize(u_4b array_length)        - resize array
________________________________________________________________________________________________________________________________

Pathfinding is performed by the "CDumbPather" structure, for which three interaction methods are defined:
    1) FindPath(SPoint2u(&path_points)[2], CPathArray* path_segments)
       > `path_points`   - For "SPoint2u" any structure containing uint16_t x, y that is friend with the "CDumbPather" class may be used.
       > `path_segments` - It holds a raw pointer to "SPoint2u" (for array initialization) and the array length.
    2) SetByteMap(i_1b** map_data, u_2b map_size)
       > `map_data` - represents the graph transition matrix as a raw double pointer to `uint8_t`.
       > `map_size` - The matrix dimensions must satisfy SizeX = SizeY; either SizeX or SizeY is passed as `map_size`.
          
________________________________________________________________________________________________________________________________
P.S. I am self-taught. I would welcome criticism and advice.))
