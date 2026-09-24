#pragma once
#ifndef PATHFINDER_FOR_DUMBS_INCLUDE
#define PATHFINDER_FOR_DUMBS_INCLUDE
#ifdef _CSTDLIB_

#ifdef _MSC_VER
#ifndef _DEBUG
#pragma warning( disable : 4786 )	// Debugger truncating names.
#pragma warning( disable : 4530 )	// Exception handler isn't used
#endif // !_DEBUG
#endif

#define MAXIMAL 0xFFFFFFFFu
#define CACHBLOCK 128

#ifdef DUMBPATHER_TYPES
	#ifdef __khrplatform_h_
		typedef khronos_int8_t i_1b;
		typedef khronos_uint8_t u_1b;
		typedef khronos_int16_t i_2b;
		typedef khronos_uint16_t u_2b;
		typedef khronos_int32_t i_4b;
		typedef khronos_uint32_t u_4b;
		typedef khronos_int64_t i_8b;
		typedef khronos_uint64_t u_8b;
	#else
		#ifdef _STDINT
			typedef int8_t i_1b;
			typedef uint8_t u_1b;
			typedef int16_t i_2b;
			typedef uint16_t u_2b;
			typedef int32_t i_4b;
			typedef uint32_t u_4b;
			typedef int64_t i_8b;
			typedef uint64_t u_8b;
		#else
			typedef signed char i_1b;
			typedef unsigned char u_1b;
			typedef signed short i_2b;
			typedef unsigned short u_2b;
			typedef signed int i_4b;
			typedef unsigned int u_4b;
			typedef signed long long i_8b;
			typedef unsigned long long u_8b;
		#endif
	#endif
	
	typedef void* PMem;
	typedef const char* c_wrd;
	typedef char* t_wrd;
	typedef const char c_sym;
	typedef char t_sym;
	typedef bool Bool;
	typedef float f_4b;
	typedef double f_8b;

	struct SU2b_xy {
		u_2b x;
		u_2b y;
	};
	#define SPoint2u SU2b_xy

#endif

namespace pthfd {
	//======================================================================
	enum TerrainType : i_1b {
		WALKABLE = '.',
		BLOKABLE = 'X',
		SLOWABLE = 'Y'
	};
	//======================================================================
	class CDumbPather;
	class CPathArray final {
	private:
		SPoint2u* array_;
		u_4b length_;
		friend CDumbPather;
	public:
		CPathArray() : array_(nullptr), length_(0) {}
		~CPathArray() { if (length_) delete[] array_; }
		template <std::integral IntType>
		__forceinline const SPoint2u& operator[](IntType i) const { return array_[i]; }
		__forceinline const u_4b length() const { return length_; }
		__forceinline void clear() { 
			if (length_) {
				delete[] array_;
				array_ = nullptr;
				length_ = 0;
			} 
		}
	};
	class CDumbPather final {
	private:
		//======================================================================
		enum : i_4b {
			NO_SOLUTION = -1,
			AT_THE_END  = 1,
			IS_SOLVED   = 2
		};
		enum : u_4b {
			WALL_COST = 16,
			FAST_LINE = 128,
			SLOW_LINE = 256,
			FAST_DIAG = 181,
			SLOW_DIAG = 362
		};
		//======================================================================
		class CPathNode;
		struct NodeCost final {
			CPathNode* node;
			u_4b cost;
		};
		struct StateCost final {
			u_4b cost;	//< The cost to the state. Use MAXIMAL for infinite cost.
			u_4b state;	//< The state as a u_4b
		};
		//======================================================================
		template <typename T> 
		class CVector final {
		private:
			u_4b m_allocated; u_4b m_size; T* m_buf;
			__forceinline void capacity(u_4b cap) {
				if(m_allocated < cap) {
					u_4b newAllocated = (cap<<1) + cap + 16;
					m_buf = reinterpret_cast<T*>(realloc(reinterpret_cast<void*>(m_buf), newAllocated * sizeof(T)));
					m_allocated = newAllocated;
				}
			}
		public:
			CVector() : m_allocated(8), m_size(0) { m_buf = reinterpret_cast<T*>(malloc(8*sizeof(T))); }
			~CVector() { free(m_buf); }
			__forceinline void clear() { m_size = 0; }	// see warning above
			__forceinline void resize(u_4b s) { capacity(s); m_size = s; }
			__forceinline void push_back(const T& t) { capacity(m_size + 1); m_buf[m_size++] = t; }
			__forceinline u_4b size() const { return m_size; }
			__forceinline T& operator[](u_4b i) { return m_buf[i]; }
		};
		class CPathNode final {
		public:
			CPathNode* child[2];		// Binary search in the hash table. [left, right]
			CPathNode* next, * prev;	// used by open queue
			CPathNode* parent;		// the parent is used to reconstruct the path
			u_4b heapIndex;			// unique id for this path, so the solver can distinguish
			u_4b frame_;			// unique id for this path, so the solver can distinguish
			u_4b state;			// the client state
			u_4b costStart;	// exact
			u_4b estToGoal;		// estimated
			u_4b totalCost;		// could be a function, but save some math.
			i_4b numAdjacent;		// -1  is unknown & needs to be queried
			i_4b cacheIndex;			// position in cache
			Bool inClosed;
			Bool inOpen;

			__forceinline void Init(u_4b _frame, u_4b _state, u_4b _costFromStart, u_4b _estToGoal, CPathNode* _parent) {
				state = _state;
				costStart = _costFromStart;
				estToGoal = _estToGoal;
				CalcTotalCost();
				parent = _parent;
				frame_ = _frame;
				inOpen = 0;
				inClosed = 0;
			}
			__forceinline void Clear() {
				memset(this, 0, sizeof(CPathNode));
				numAdjacent = -1;
				cacheIndex = -1;
				heapIndex = MAXIMAL;
			}

			__forceinline void InitSentinel() {
				Clear();
				Init(0, 0, MAXIMAL, MAXIMAL, 0);
				prev = next = this;
			}
			__forceinline void Unlink() {
				next->prev = prev;
				prev->next = next;
				next = prev = 0;
			}
			__forceinline void AddBefore(CPathNode* addThis) {
				addThis->next = this;
				addThis->prev = prev;
				prev->next = addThis;
				prev = addThis;
			}
			__forceinline void CalcTotalCost() {
				if (costStart < MAXIMAL && estToGoal < MAXIMAL)
					totalCost = costStart + estToGoal;
				else
					totalCost = MAXIMAL;
			}
		};
		class CPathPool final {
		private:
			struct Block {
				Block* nextBlock;
				CPathNode pathNode[1];
			};
			CPathNode** hashTable;
			Block* firstBlock;
			Block* blocks;

			NodeCost* cache;
			i_4b		cacheCap;
			i_4b		cacheSize;

			CPathNode	freeMemSentinel;
			const u_4b	allocate = CACHBLOCK;				// how big a block of pathnodes to allocate at once
			u_4b		nAllocated;				// number of pathnodes allocated (from Alloc())
			u_4b		nAvailable;				// number available for allocation
			u_4b		hashShift;
			u_4b		totalCollide;

			__forceinline u_4b HashSize() { return 1 << hashShift; }
			__forceinline u_4b HashMask() { return ((1 << hashShift) - 1); }
			__forceinline void AddCPathNode(u_4b key, CPathNode* root) {
				if (hashTable[key]) {
					CPathNode* p = hashTable[key];
					while (true) {
						i_4b dir = (root->state < p->state) ? 0 : 1;
						if (p->child[dir]) { p = p->child[dir]; }
						else { p->child[dir] = root; break; }
					}
				}
				else { hashTable[key] = root; }
			}
			__forceinline Block* NewBlock() {
				Block* block = reinterpret_cast<Block*>(calloc(1, sizeof(Block) + sizeof(CPathNode) * (allocate - 1)));
				block->nextBlock = 0;
				nAvailable += allocate;
				for (u_4b i = 0; i < allocate; ++i)
					freeMemSentinel.AddBefore(&block->pathNode[i]);
				return block;
			}
			__forceinline CPathNode* Alloc() {
				if (freeMemSentinel.next == &freeMemSentinel) {
					Block* b = NewBlock();
					b->nextBlock = blocks;
					blocks = b;
				}
				CPathNode* pathNode = freeMemSentinel.next;
				pathNode->Unlink();
				++nAllocated;
				--nAvailable;
				return pathNode;
			}
		public:
			CPathPool(u_4b _typicalAdjacent)
				: firstBlock(0), blocks(0), nAllocated(0), nAvailable(0) {
				freeMemSentinel.InitSentinel();
				cacheCap = allocate * _typicalAdjacent;
				cacheSize = 0;
				cache = reinterpret_cast<NodeCost*>(malloc(cacheCap * sizeof(NodeCost)));
				hashShift = 3;	// 8 (only useful for stress testing) 
				while (HashSize() < allocate) ++hashShift;
				hashTable = reinterpret_cast<CPathNode**>(calloc(HashSize(), sizeof(CPathNode*)));
				blocks = firstBlock = NewBlock();
				//	printf( "HashSize=%d allocate=%d\n", HashSize(), allocate );
				totalCollide = 0;
			}
			~CPathPool() {
				Clear();
				free(firstBlock);
				free(cache);
				free(hashTable);
			}
			__forceinline void Clear() {
				Block* b = blocks;
				while (b) {
					Block* temp = b->nextBlock;
					if (b != firstBlock) {
						free(b);
					}
					b = temp;
				}
				blocks = firstBlock;
				if (nAllocated > 0) {
					freeMemSentinel.next = &freeMemSentinel;
					freeMemSentinel.prev = &freeMemSentinel;

					memset(hashTable, 0, sizeof(CPathNode*) * HashSize());
					for (u_4b i = 0; i < allocate; ++i) {
						freeMemSentinel.AddBefore(&firstBlock->pathNode[i]);
					}
				}
				nAvailable = allocate;
				nAllocated = 0;
				cacheSize = 0;
			}
			__forceinline CPathNode* GetCPathNode(u_4b frame_, u_4b _state, u_4b _costFromStart, u_4b _estToGoal, CPathNode* _parent) {
				u_4b key = _state & HashMask();
				CPathNode* root = hashTable[key];
				while (root) {
					if (root->state == _state) {
						if (root->frame_ == frame_)		// This is the correct state and correct frame_.
							break;
						// Correct state, wrong frame_.
						root->Init(frame_, _state, _costFromStart, _estToGoal, _parent);
						break;
					}
					root = (_state < root->state) ? root->child[0] : root->child[1];
				}
				if (!root) {
					// allocate new one
					root = Alloc();
					root->Clear();
					root->Init(frame_, _state, _costFromStart, _estToGoal, _parent);
					AddCPathNode(key, root);
				}
				return root;
			}
			__forceinline Bool PushCache(const NodeCost* nodes, i_4b nNodes, i_4b* start) {
				*start = -1;
				if (nNodes + cacheSize <= cacheCap) {
					for (i_4b i = 0; i < nNodes; ++i) {
						cache[i + cacheSize] = nodes[i];
					}
					*start = cacheSize;
					cacheSize += nNodes;
					return true;
				}
				return false;
			}
			__forceinline void GetCache(i_4b start, i_4b nNodes, NodeCost* nodes) {
				memcpy(nodes, &cache[start], sizeof(NodeCost) * nNodes);
			}
			__forceinline void AllStates(u_4b frame_, CVector< u_4b >* stateVec)
			{
				for (Block* b = blocks; b; b = b->nextBlock)
				{
					for (u_4b i = 0; i < allocate; ++i)
					{
						if (b->pathNode[i].frame_ == frame_)
							stateVec->push_back(b->pathNode[i].state);
					}
				}
			}
		};
		class COpenQueue final {
		private:
			CVector<CPathNode*> heap_;
			static constexpr u_4b D = 4;

			__forceinline void SiftUp(u_4b i) {
				CPathNode* x = heap_[i];
				while (i > 0) {
					u_4b p = (i - 1) >> 2;
					CPathNode* y = heap_[p];
					if (y->totalCost <= x->totalCost) break;
					heap_[i] = y; y->heapIndex = i;
					i = p;
				}
				heap_[i] = x; x->heapIndex = i;
			}
			__forceinline void SiftDown(u_4b i) {
				CPathNode* x = heap_[i];
				const u_4b n = heap_.size();
				while (true) {
					u_4b c = i * D + 1;
					if (c >= n) break;
					u_4b best = c;
					u_4b end = (c + D < n) ? c + D : n;
					for (u_4b k = c + 1; k < end; ++k)
						if (heap_[k]->totalCost < heap_[best]->totalCost) best = k;
					if (heap_[best]->totalCost >= x->totalCost) break;
					heap_[i] = heap_[best]; heap_[i]->heapIndex = i;
					i = best;
				}
				heap_[i] = x; x->heapIndex = i;
			}
		public:
			__forceinline void Update(CPathNode* n) {
				SiftUp(n->heapIndex);
				SiftDown(n->heapIndex);
			}
			__forceinline void Clear() { heap_.clear(); }
			__forceinline void Push(CPathNode* n) {
				n->heapIndex = heap_.size();
				heap_.push_back(n);
				SiftUp(n->heapIndex);
				n->inOpen = 1;
			}
			__forceinline Bool Empty() const { return heap_.size() == 0; }
			__forceinline CPathNode* Pop() {
				CPathNode* top = heap_[0];
				const u_4b sz = heap_.size();
				if (sz > 1) {
					CPathNode* last = heap_[sz - 1];
					heap_.resize(sz - 1);
					heap_[0] = last;
					last->heapIndex = 0;
					SiftDown(0);
				}
				else {
					heap_.resize(0);
				}
				top->inOpen = 0;
				top->heapIndex = MAXIMAL;
				return top;
			}

		};
		//======================================================================

		//friend class CPathNode;
		CPathPool	pathNodePool_;
		CVector< StateCost >	statesCostVec_;	// local to Search, but put here to reduce memory allocation
		CVector< NodeCost  >	nodesCostVec_;	// local to Search, but put here to reduce memory allocation
		CVector< u_4b >			costsVec_;
		u_4b finder_frame_;			// incremented with every solve, used to determine if cached data needs to be refreshed
		//======================================================================
		i_1b** map_cells_;
		u_2b map_size_;
		__forceinline u_4b GetEstimateCost(u_4b stateStart, u_4b stateEnd) {
			SPoint2u
				s = SPoint2u{ static_cast<u_2b>(stateStart % map_size_), static_cast<u_2b>(stateStart / map_size_) },
				e = SPoint2u{ static_cast<u_2b>(stateEnd % map_size_),   static_cast<u_2b>(stateEnd / map_size_) };
			return (
				(s.x > e.x ? static_cast<u_4b>(s.x - e.x) : static_cast<u_4b>(e.x - s.x)) +
				(s.y > e.y ? static_cast<u_4b>(s.y - e.y) : static_cast<u_4b>(e.y - s.y)) ) << 7;
		}
		__forceinline void GetAdjacentCost(u_4b state, CVector<StateCost>* neighbors) {
			SPoint2u e, s = SPoint2u{ static_cast<u_2b>(state % map_size_), static_cast<u_2b>(state / map_size_) };
			//----------------------------------------------------------------------
			e.y = --s.y;
			if (e.y < map_size_) {
				e.x = --s.x;
				i_1b*& line = map_cells_[e.y];
				u_4b indx = static_cast<u_4b>(map_size_) * static_cast<u_4b>(e.y) + static_cast<u_4b>(e.x);
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_DIAG, indx });
						else
							neighbors->push_back({ FAST_DIAG, indx });
					}
				}
				++e.x;
				++indx;
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_LINE, indx });
						else
							neighbors->push_back({ FAST_LINE, indx });
					}
				}
				++e.x;
				++indx;
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_DIAG, indx });
						else
							neighbors->push_back({ FAST_DIAG, indx });
					}
				}
			}
			//----------------------------------------------------------------------
			++e.y;
			if (e.y < map_size_) {
				e.x = s.x;
				i_1b*& line = map_cells_[e.y];
				u_4b indx = static_cast<u_4b>(map_size_) * static_cast<u_4b>(e.y) + static_cast<u_4b>(e.x);
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_LINE, indx });
						else
							neighbors->push_back({ FAST_LINE, indx });
					}
				}
				e.x += 2;
				indx += 2;
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_LINE, indx });
						else
							neighbors->push_back({ FAST_LINE, indx });
					}
				}
			}
			//----------------------------------------------------------------------
			++e.y;
			if (e.y < map_size_) {
				e.x = s.x;
				i_1b*& line = map_cells_[e.y];
				u_4b indx = static_cast<u_4b>(map_size_) * static_cast<u_4b>(e.y) + static_cast<u_4b>(e.x);
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_DIAG, indx });
						else
							neighbors->push_back({ FAST_DIAG, indx });
					}
				}
				++e.x;
				++indx;
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_LINE, indx });
						else
							neighbors->push_back({ FAST_LINE, indx });
					}
				}
				++e.x;
				++indx;
				if (e.x < map_size_) {
					if (line[e.x] != TerrainType::BLOKABLE) {
						if (line[e.x] == TerrainType::SLOWABLE)
							neighbors->push_back({ SLOW_DIAG, indx });
						else
							neighbors->push_back({ FAST_DIAG, indx });
					}
				}
			}
		}
		//======================================================================
		__forceinline void Achieved(CPathNode* node, u_4b start, u_4b end, CVector<u_4b>* _path) {
			CVector<u_4b>& path = *_path;
			CPathNode* it = node;
			i_4b count = 1;
			path.clear();
			while (it->parent) {
				++count;
				it = it->parent;
			}
			if (count < 3) {
				path.resize(2);
				path[0] = start;
				path[1] = end;
			}
			else {
				path.resize(count);
				path[0] = start;
				path[count - 1] = end;
				count -= 2;
				it = node->parent;
				while (it->parent) {
					path[count] = it->state;
					it = it->parent;
					--count;
				}
			}
		}
		__forceinline void Environs(CPathNode* mpather_node, CVector<NodeCost>* pNodeCost) {
			if (mpather_node->numAdjacent == 0) {
				pNodeCost->resize(0);
			}
			else if (mpather_node->cacheIndex < 0) {
				statesCostVec_.resize(0);
				GetAdjacentCost(mpather_node->state, &statesCostVec_);
				pNodeCost->resize(statesCostVec_.size());
				mpather_node->numAdjacent = statesCostVec_.size();
				if (mpather_node->numAdjacent > 0) {
					const u_4b stateCostVecSize = statesCostVec_.size();
					const StateCost* stateCostVecPtr = &statesCostVec_[0];
					NodeCost* pNodeCostPtr = &(*pNodeCost)[0];
					for (u_4b i = 0; i < stateCostVecSize; ++i) {
						u_4b state = stateCostVecPtr[i].state;
						pNodeCostPtr[i].cost = stateCostVecPtr[i].cost;
						pNodeCostPtr[i].node = pathNodePool_.GetCPathNode(finder_frame_, state, MAXIMAL, MAXIMAL, 0);
					}
					i_4b start = 0;
					if (pNodeCost->size() > 0 && pathNodePool_.PushCache(pNodeCostPtr, pNodeCost->size(), &start)) {
						mpather_node->cacheIndex = start;
					}
				}
			}
			else {
				pNodeCost->resize(mpather_node->numAdjacent);
				NodeCost* pNodeCostPtr = &(*pNodeCost)[0];
				pathNodePool_.GetCache(mpather_node->cacheIndex, mpather_node->numAdjacent, pNodeCostPtr);
				for (i_4b i = 0; i < mpather_node->numAdjacent; ++i) {
					CPathNode* pNode = pNodeCostPtr[i].node;
					if (pNode->frame_ != finder_frame_) {
						pNode->Init(finder_frame_, pNode->state, MAXIMAL, MAXIMAL, 0);
					}
				}
			}
		}
		__forceinline i_4b Search(u_4b startNode, u_4b endNode, CVector< u_4b >* path, u_4b* cost) {
			path->clear();
			*cost = 0;
			if (startNode == endNode) return AT_THE_END;
			++finder_frame_;
			COpenQueue open;
			CPathNode* newCPathNode = pathNodePool_.GetCPathNode(
				finder_frame_, startNode, 0,
				GetEstimateCost(startNode, endNode), 0
			);
			open.Push(newCPathNode);
			//statesCostVec_.resize(0);
			//nodesCostVec_.resize(0);
			while (!open.Empty()) {
				CPathNode* node = open.Pop();
				if (node->state == endNode) {
					*cost = node->costStart;
					Achieved(node, startNode, endNode, path);
					return IS_SOLVED;
				}
				else {
					node->inClosed = 1;
					Environs(node, &nodesCostVec_);
					for (i_4b i = 0; i < node->numAdjacent; ++i) {
						if (nodesCostVec_[i].cost == MAXIMAL) continue;
						CPathNode* child = nodesCostVec_[i].node;
						CPathNode* inOpen = child->inOpen ? child : 0;
						CPathNode* inClosed = child->inClosed ? child : 0;
						CPathNode* inEither = (CPathNode*)(((u_8b)inOpen) | ((u_8b)inClosed));
						u_4b newCost = node->costStart + nodesCostVec_[i].cost;
						if (inEither) {
							if (newCost < child->costStart) {
								child->parent = node;
								child->costStart = newCost;
								child->estToGoal = GetEstimateCost(child->state, endNode);
								child->CalcTotalCost();
								if (inOpen) open.Update(child);
							}
						}
						else {
							child->parent = node;
							child->costStart = newCost;
							child->estToGoal = GetEstimateCost(child->state, endNode), child->CalcTotalCost();
							open.Push(child);
						}
					}
				}
			}
			return NO_SOLUTION;
		}
	public:
		~CDumbPather() {}
		CDumbPather() : pathNodePool_(8), finder_frame_(0), map_cells_(nullptr), map_size_(0) {}
		
		__forceinline void Find(SPoint2u(&se)[2], CPathArray* path_class) {//i_4b* path_length, SPoint2u** outPath) {
			SPoint2u& start = se[0];
			SPoint2u& end = se[1];
			SPoint2u*& path = path_class->array_;
			u_4b& length = path_class->length_;
			path_class->clear();
			{
				Bool end_unvalid   = !(end.y   < map_size_ && end.x   < map_size_ ? map_cells_[ end.y ][ end.x ] != TerrainType::BLOKABLE : false);
				Bool start_unvalid = !(start.y < map_size_ && start.x < map_size_ ? map_cells_[start.y][start.x] != TerrainType::BLOKABLE : false);
				if (start_unvalid || end_unvalid) return;
			}

			CVector<u_4b> pathNodes;
			u_4b totalCost;
			u_4b frstNode = static_cast<u_4b>(map_size_) * static_cast<u_4b>(start.y) + static_cast<u_4b>(start.x);
			u_4b lastNode = static_cast<u_4b>(map_size_) * static_cast<u_4b>(end.y) + static_cast<u_4b>(end.x);
			i_4b result = Search(frstNode, lastNode, &pathNodes, &totalCost);
			if (result == IS_SOLVED) {
				length = pathNodes.size();
				path = new SPoint2u[length];
				if (!path) { length = 0; return; }
				for (u_4b i = 0; i < length; ++i) {
					path[i] = SPoint2u{
						static_cast<u_2b>(pathNodes[i] % map_size_), 
						static_cast<u_2b>(pathNodes[i] / map_size_) 
					};
				}
			}
		}
		__forceinline void SetByteMap(i_1b** map_data, u_2b map_size) { 
			map_cells_ = map_data;
			map_size_ = map_size;
			pathNodePool_.Clear();
			finder_frame_ = 0;
		}
		__forceinline void ResetNodes() {
			pathNodePool_.Clear();
			finder_frame_ = 0;
		}
#ifdef _IOSTREAM_
		__forceinline void PrintStateInfo(u_4b state) {
			SPoint2u p = SPoint2u{ static_cast<u_2b>(state % map_size_), static_cast<u_2b>(state / map_size_) };
			printf("(%d, %d)", p.x, p.y);
		}
		__forceinline void PrintCacheState() {
			std::cout << "PathFinder Cache :\n\tHead: " << sizeof(*this) << std::endl;
		}
#endif
	};
};

#undef CACHBLOCK
#undef MAXIMAL
#endif // _CSTDLIB_
#endif // PATHFINDER_FOR_DUMBS_INCLUDE