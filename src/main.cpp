#include "common.h"
#include "newgreedy.h"
#include "mixgreedy.h"
#include "sketch.h"
using namespace std;

void oracle(const graph_t& g, size_t K, const int R) {
	auto cache_ptr = make_unique<char[]>(R * g.n);
	auto rand_seeds = get_rands(R);
	float score = 0;
	for (int i; cin >> i;)
		score = run_ic_vertpar(g, i, R, rand_seeds.get(), cache_ptr.get());
	cout << score << endl;
}

int main(int argc, char* argv[]) {
	int K = 50, R = 256, c, blocksize = 32;
	bool directed = false, sorted = true;
	float p = 0.01, eps = 0.3, tr = 0.01, trc = 0.02;
	float w = 0, ua = 0, ub = 1;
	bool WIC = false;
	string method = "HyperFuser", filename;
	ofstream out;
	if (argc < 2){
		cerr << "Usage: " << argv[0] << " -M [Method=[~MixGreedy/HyperFuser]] -R [#MC="<<R<<"] -e [threshold="<<tr<<"] -o [output file(optional)]\n";
		exit(-1);
	}

	for (int i = 1; i < argc; i++) {
		string s(argv[i]);
		if (s == "-K") K = atoi(argv[++i]);
		else if (s == "-R") R = atoi(argv[++i]);
		else if (s == "-M") method = string(argv[++i]);
		else if (s == "-e") eps = atof(argv[++i]);
		else if (s == "-t") tr = atof(argv[++i]);
		else if (s == "-s") sorted = atoi(argv[++i]);
		else if (s == "-c") trc = atof(argv[++i]);
		else if (s == "-o") { out.open(argv[++i]); std::cout.rdbuf(out.rdbuf()); }
		else if (s == "-UIC") {w = atof(argv[++i]);}
		else if (s == "-ua") {ua = atof(argv[++i]);}
		else if (s == "-ub") {ub = atof(argv[++i]);}
		else if (s == "-WIC") {WIC = true;}
		else filename = s;
	}
	// graph_t g = read_txt(filename);
	graph_t g = read_binary(filename);
	if (w!=0)  AssignUniWeight(g, w);
	else if (ub!= 1) AssignUniformRandomWeight(g, ua ,ub);
	else if (WIC) AssignWICWeight(g);
	else {
		std::cout << "no weight assginment specified. -UIC [float] for uni weight, -ua [float] -ub [float] for uniform (ua, ub) -WIC for WIC_SYM" << std::endl;
		return 1;
	}
	printf("n: %d m: %d K: %d R: %d w: %f ua: %f ub: %f WIC: %d\n", g.n, g.m, K, R, w, ua, ub, WIC);
	std::for_each(method.begin(), method.end(), [](char& c) {c = ::tolower(c);});
	std::cout << std::fixed << std::setprecision(2);
	vector<unsigned> seeds(K);
	if (method == "infuser"){
		int repeat = 3;
		std::pair<double, double> run_time = newgreedy(g, K, R, sorted, seeds);
		double sketch_time=0; double select_time=0; double total_time=0;
		printf("round 0:\n	sketch time %f\n 	select time %f\n", get<0>(run_time), get<1>(run_time));
		Timer timer;
		for (int i = 0; i<repeat; i++){
			run_time = newgreedy(g, K, R, sorted, seeds);
			printf("round %d:\n	sketch time %f\n 	select time %f\n", i+1, get<0>(run_time), get<1>(run_time));
			sketch_time += get<0>(run_time);
			select_time += get<1>(run_time);
		}
		total_time = timer.elapsed();
		printf("average sketch construction time: %f\n", sketch_time/repeat);
		printf("average seed selection time: %f\n", select_time/repeat);
		printf("average total time: %f\n", total_time/repeat);
		printf("seeds: ");
		for (auto s : seeds)
			printf("%d ", s);
		printf("\n");
	}else if (method == "hyperfuser")
		hyperfuser(g, K, R, eps, tr, trc, sorted);
	else if (method == "oracle")
		oracle(g, K, R);
	return 0;
}
