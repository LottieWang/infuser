#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <immintrin.h>
#include <chrono>
#include <memory>
#include <algorithm>
#include <random>
#include <fstream>
#include <omp.h>
#include <iomanip>
#include <climits>
#include <cstring>
#if defined (_MSC_VER)
#include <nmmintrin.h>
#endif
#include <algorithm>
#if _MSC_VER >= 1910
#include <execution>
#else
#include <parallel/algorithm>
#include <parallel/numeric>
#include <stdlib.h> 
#include <assert.h>
void* _aligned_malloc(size_t size, size_t alignment) {
	void* p;
	if (posix_memalign(&p, alignment, size))
		return NULL;
	return p;
}
void _aligned_free(void* p) { free(p); }

#endif

using std::vector;
using std::ifstream;
using std::ofstream;
using std::cin;
using std::cout;
using std::unique_ptr;
using std::string;
using std::pair;
using std::stringstream;
using std::make_pair;
using std::endl;
using std::cerr;
using std::tuple;
using std::get;


const uint32_t HASHMASK = INT32_MAX;

struct Timer{
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const { return std::chrono::duration_cast<second_> (clock_::now() - beg_).count(); }
} t;

struct edge_t { unsigned v; signed w; };
struct graph_t {
	size_t* xadj, n = 0, m = 0;
	edge_t* adj;
};

uint64_t popcnt(const char* ptr, const size_t size) {
	uint64_t i = 0, cnt = 0;
	for (; i < size - size % 8; i += 8)
		cnt += _mm_popcnt_u64(*(const uint64_t*)(ptr + i));
	for (; i < size; i++)
		cnt += _mm_popcnt_u64(ptr[i]);
	return cnt;
}
uint64_t parpopcnt(const char* ptr, const size_t size) {
	uint64_t cnt = 0;
#pragma omp parallel for reduction(+:cnt)
	for (long long i = 0; i < size - size % 8; i += 8)
		cnt += _mm_popcnt_u64(*(const uint64_t*)(ptr + i));
	for (long long i = size - size % 8; i < size; i++)
		cnt += _mm_popcnt_u64(ptr[i]);
	return cnt;
}


inline uint32_t __hash(uint64_t h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdL;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53L;
	h ^= h >> 33;
	return h & HASHMASK;//FFFF;
}
inline uint64_t __hash64(uint64_t h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccdL;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53L;
	h ^= h >> 33;
	return h;//FFFF;
}
inline uint32_t edge_hash(const uint32_t x, const uint32_t y) {
	uint64_t h = (((uint64_t)x) << 32) | y;
	return __hash(h);
}
inline uint32_t __hash(const uint32_t x, const uint32_t y) {
	uint64_t h = (x > y) ? (((uint64_t)y) << 32) | x : (((uint64_t)x) << 32) | y;
	return __hash(h);
}
template <typename T>
void cpy(T* buf, T* end, T* out) {
	const int len = end - buf;
#pragma omp parallel for
	for (int i = 0; i < len; i++)
		out[i] = buf[i];
}

template <typename T>
void parfill(T* buf, T* end, T pattern) {
	const int len = end - buf;
#pragma omp parallel for
	for (int i = 0; i < len; i++)
		buf[i] = pattern;
}

template <typename T>
void parfill(vector<T> & buf, T pattern) {
	const int len = buf.size();
#pragma omp parallel for
	for (int i = 0; i < len; i++)
		buf[i] = pattern;
}



template<class T>
unique_ptr<T[], decltype(&_aligned_free)> get_aligned(size_t elems, size_t align = 64){
	auto* ptr = static_cast<T*>(_aligned_malloc(sizeof(T) * elems, align));
	std::fill(ptr, ptr + elems, 0);
	return unique_ptr<T[], decltype(&_aligned_free)>(ptr, &_aligned_free);
}

graph_t read_bin(string filename) {
	graph_t g;
	ifstream rf(filename, std::ios::out | std::ios::binary);
	if (!rf) {
		cerr << "Cannot open file!" << endl;
		return g;
	}
	int mode;
	rf.read((char*)&g.n, sizeof(g.n));
	rf.read((char*)&g.m, sizeof(g.m));
	g.xadj = new size_t[g.n + 1];
	g.adj = new edge_t[g.m];
	rf.read((char*)g.xadj, size_t(g.n + 1) * sizeof(g.xadj[0]));
	rf.read((char*)g.adj, size_t(g.m) * sizeof(edge_t));
	return g;
}
graph_t read_txt(string filename) {
	graph_t g;
	ifstream rf(filename);
	if (!rf) {
		cerr << "Cannot open file!" << endl;
		return g;
	}
	rf >> g.n >> g.m;
	g.xadj = new size_t[g.n + 1];
	g.adj = new edge_t[g.m];
	unsigned s, t; float w;
	unsigned i = 0; size_t j = 0;
	while (rf >> s >> t >> w) {
		while (i <= s)
			g.xadj[i++] = j;
		g.adj[j++] = { t, int(w * INT_MAX) };
 	}
	g.xadj[i] = j;
	return g;
}
graph_t read_binary(string filename){
	graph_t g;
	ifstream rf(filename);
    if (!rf.is_open()) {
      cerr << "Error: Cannot open file " << filename << '\n';
      abort();
    }
	size_t n, m, sizes;
    rf.read(reinterpret_cast<char*>(&n), sizeof(size_t));
    rf.read(reinterpret_cast<char*>(&m), sizeof(size_t));
    rf.read(reinterpret_cast<char*>(&sizes), sizeof(size_t));
    assert(sizes == (n + 1) * 8 + m * 4 + 3 * 8);
	g.n = n;
	g.m = m;
	uint64_t* offset = new uint64_t[n+1];
	uint32_t* edges = new uint32_t[m];
	rf.read(reinterpret_cast<char*>(offset), (n+1)*8);
	rf.read(reinterpret_cast<char*>(edges), m*4);
	g.xadj = new size_t[g.n+1];
	g.adj = new edge_t[g.m];
	// int w_int = int(w * INT_MAX);
	for (size_t i = 0; i<n+1; i++){
		g.xadj[i]=offset[i];
	}
	for (size_t i =0; i<m; i++){
		g.adj[i] = {edges[i],0};
	}
	delete[] offset;
	delete[] edges;
	if (rf.peek() != EOF) {
      cerr << "Error: Bad data\n";
      abort();
    }
    rf.close();
	// printf("size of size_t %ld, unsigned %ld, signed %ld, edge_t %ld\n", sizeof(size_t), sizeof(unsigned), sizeof(signed), sizeof(edge_t));
	return g;
}

void AssignUniWeight(graph_t& graph, float w){
  int w_int = int(w * INT_MAX);
  for(size_t i = 0; i<graph.m; i++){
	edge_t edge = graph.adj[i];
    graph.adj[i]={edge.v, w_int};
  }
}

inline uint64_t hash64(uint64_t u) {
  uint64_t v = u * 3935559000370003845ul + 2691343689449507681ul;
  v ^= v >> 21;
  v ^= v << 37;
  v ^= v >> 4;
  v *= 4768777513237032717ul;
  v ^= v << 20;
  v ^= v >> 41;
  v ^= v << 5;
  return v;
}

double Uniform(size_t n, size_t u, size_t v, double l, double r) {
  if (u > v) std::swap(u, v);
  double p = 1.0 * hash64(hash64(hash64(n) + u + 1) + v + 1) / std::numeric_limits<uint64_t>::max();
  return l + (r - l) * p;
}

void AssignUniformRandomWeight(graph_t& graph, double l, double r) {
  for(size_t i =0; i< graph.n; i++) {
    for(size_t j = graph.xadj[i]; j<graph.xadj[i+1]; j++) {
      edge_t edge = graph.adj[j];
	  double w = Uniform(graph.n, i, edge.v, l, r);
      graph.adj[j]={edge.v, int(w*INT_MAX)};
    }
  }
}

void AssignWICWeight(graph_t& graph) {
  for(size_t i = 0; i< graph.n; i++) {
	auto deg_i = graph.xadj[i + 1] - graph.xadj[i];
    for(size_t j = graph.xadj[i]; j<graph.xadj[i+1]; j++) {
      edge_t edge = graph.adj[j];
      auto deg_v = graph.xadj[edge.v + 1] - graph.xadj[edge.v];
      double w = 2.0 / (deg_i + deg_v);
	  graph.adj[j]={edge.v, int(w*INT_MAX)};
    }
  }
}

#include <assert.h>
#define isbitset(x,i) ((x[i>>3] & (1<<(i&7)))!=0)
#define setbit(x,i) x[i>>3]|=(1<<(i&7));
#define CLEARBIT(x,i) x[i>>3]&=(1<<(i&7))^0xFF;
template <class T>
vector<graph_t> split(const graph_t& g, size_t batch_size, T& rand_seeds) {
	vector<graph_t> samples;
#pragma omp parallel for
	for (int sid = 0; sid < rand_seeds.size(); sid++) {
		graph_t __g = g;
		__g.m = 0;
		// first pass to detect # of edges sampled
		for (int s = 0; s < g.n; s++) {
			const auto hash_s = _mm_crc32_u32(0, s);
			for (int it = g.xadj[s]; it < g.xadj[s + 1]; it++) {
				const edge_t e = g.adj[it];
				const auto hash_sv = _mm_crc32_u32(s, e.v)>>1;
				for (int r = 0; r < batch_size; r++) {
					unsigned rnd = rand_seeds[sid][r];
					if (((rnd ^ hash_sv) <= e.w))
					{
						__g.m++;
						break;
					}
				}
			}
		}
		__g.xadj = new size_t[g.n + 1];
		__g.adj = new edge_t[__g.m];
		int j = 0; __g.xadj[0] = 0;
		for (int s = 0; s < g.n; s++) {
			const auto hash_s = _mm_crc32_u32(0, s);
			for (int it = g.xadj[s]; it < g.xadj[s + 1]; it++) {
				const edge_t e = g.adj[it];
				const auto hash_sv = _mm_crc32_u32(s, e.v)>>1;
				for (int r = 0; r < batch_size; r++) {
					unsigned rnd = rand_seeds[sid][r];
					if (((rnd ^ hash_sv) <= e.w))
					{
						__g.adj[j++] = g.adj[it];
						break;
					}
				}
			}
			__g.xadj[s + 1] = j;

		}
		__g.m = j;
#pragma omp critical
		samples.push_back(__g);
		/* */
		cout << float(__g.m) / (g.m) << endl;
		//cout << memcmp(g.adj, __g.adj, sizeof(edge_t) * g.m) << endl;
		//cout << memcmp(g.xadj, __g.xadj, sizeof(int) * g.n) << endl;
		//for (int i = 0; i < 10; i++)
		//	cout << g.xadj[i] << " " << __g.xadj[i] << endl;
	}
	return samples;
}
//
vector<graph_t> split(const graph_t& g, size_t R, int* rand_seeds, int n_splits) {
	int step = (R / n_splits);
	vector<graph_t> samples;
//#pragma omp parallel for
	for (int r = 0; r < R; r += step) {
		graph_t __g;
		__g.n = g.n;
		__g.m = 0;
		// first pass to detect # of edges sampled
		for (int s = 0; s < g.n; s++) {
			const auto hash_s = _mm_crc32_u32(0, s);
			for (int it = g.xadj[s]; it < g.xadj[s + 1]; it++) {
				const edge_t e = g.adj[it];
				const auto hash_sv = _mm_crc32_u32(s, e.v)>>1;
				for (int b = r; b < r + step; b++) {
					int rnd = rand_seeds[b];
					if (((rnd ^ hash_sv) < e.w)) {
						__g.m++;
						break;
					}
				}
			}
		}
		__g.xadj = new size_t[g.n + 1];
		__g.adj = new edge_t[__g.m];
		int j = 0; __g.xadj[0] = 0;
		for (int s = 0; s < g.n; s++) {
			const auto hash_s = _mm_crc32_u32(0, s);
			for (int it = g.xadj[s]; it < g.xadj[s + 1]; it++) {
				const edge_t e = g.adj[it];
				const auto hash_sv = _mm_crc32_u32(s, e.v)>>1;
				for (int b = r; b < r + step; b++) {
					int rnd = rand_seeds[b];
					if (((rnd ^ hash_sv) < e.w)) {
						__g.adj[j++] = g.adj[it];
						break;
					}
				}
			}
			__g.xadj[s + 1] = j;
		}
		__g.m = j;
#pragma omp critical
		samples.push_back(__g);
	}
	return samples;
}
//unique_ptr<int[], decltype(&_aligned_free)> get_rands(size_t size) {
//	std::default_random_engine e1(42);
//	std::uniform_int_distribution<int> uniform_dist(0, INT_MAX);
//
//	auto rand_seeds = get_aligned<int>(size);
//	for (int i = 0; i < size; i++)
//		rand_seeds[i] = uniform_dist(e1);
//	return move(rand_seeds);
//}

unique_ptr<int[], decltype(&_aligned_free)> get_rands(size_t size, size_t offset=0) {
	std::default_random_engine e1(42);
	std::uniform_int_distribution<int> uniform_dist(0, INT_MAX);
	for (size_t i = 0; i < offset; i++)
		uniform_dist(e1); // burn few to get deterministic values

	auto rand_seeds = get_aligned<int>(size);
	for (size_t i = 0; i < size; i++)
		rand_seeds[i] = uniform_dist(e1);
	return move(rand_seeds);
}