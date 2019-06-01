#include <iostream>
#include <vector>

const double INF = 999999;
const double MUTATION_PROBABILITY = 0.3;
const size_t ITERATIONS = 100;
const size_t SELECTION_PART = 2; // how many times selection group less than original population
const size_t ELITE = 1; // number of individuals selecting for crossover without dice rolling
const size_t MUTATION_SIZE = 2; // number of genes pairs susceptible for mutation

enum class TYPE { NONE = -1, TSP, ATSP };
enum class EDGE_WEIGHT_TYPE { NONE = -1, EXPLICIT, EUC_2D, ATT };
enum class EDGE_WEIGHT_FORMAT { NONE = -1, FULL_MATRIX };
enum class SECTION { NONE = -1, EDGE_WEIGHT_SECTION, NODE_COORD_SECTION };

enum class CROSSOVER_SELECTION { PROPORTIONAL, TOURNAMENT };

class TSP
{
public:
    std::string getName() const { return m_name; }
    std::string getDescription() const { return m_comment; }
    int getSize() const { return m_size; }
    void showMatrix() { for (auto i : m_matrix) { std::cout << std::endl; for (auto j : i) std::cout << j << " "; } std::cout << std::endl; }
    void showInitial() { for (auto i : m_population) { std::cout << std::endl; for (auto j : i) std::cout << j << " "; } std::cout << std::endl; }
    bool readFromFile(const std::string& filename);
    bool readInitial(const std::string& filename);
    void solve(CROSSOVER_SELECTION cross = CROSSOVER_SELECTION::PROPORTIONAL,bool verbose = true);

private:
    std::string m_name;
    std::string m_comment;

    TYPE m_type;
    EDGE_WEIGHT_TYPE m_edgeWeightType;
    EDGE_WEIGHT_FORMAT m_edgeWeightFormat;

    size_t m_size;

    double m_record;

    std::vector<std::vector<double> > m_matrix;
    std::vector<std::vector<double> > m_population;
    std::vector<double> m_path;

    size_t m_iterations;

    //parsing
    bool isSection(const std::string& str);
    bool isEOF(const std::string& str);
    bool parseSection(const std::string& str);
    bool parseParam(const std::string& str, const std::string& name, std::string& result);
    bool readMatrix(std::ifstream& ifs);
    bool readNodeCoord(std::ifstream& ifs);
    double dist(const std::pair<double, double>& a, const std::pair<double, double>& b);

    // solving
    double getLenght(const std::vector<double>& path);
    double getFitness(const std::vector<double>& path);
    void sortAndBest();
    void GA(CROSSOVER_SELECTION cross = CROSSOVER_SELECTION::PROPORTIONAL);
    // 1 - [0;k], 2 - [k+1 ; size-1]
    // Partially Mapped Crossover
    std::vector<double> PMX(const std::vector<double>& p1, const std::vector<double>& p2, size_t k);
    std::vector<std::vector<double>> selectionForCrossover(CROSSOVER_SELECTION cross);
    std::vector<std::vector<double>> tournament();
    std::vector<std::vector<double>> proportional();
    std::vector<double> mutation(const std::vector<double>& individual);
    void crossover(const std::vector<std::vector<double>>& population);
};
