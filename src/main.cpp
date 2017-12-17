#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <random>

const double INF = 999999;
const double MUTATION_PROBABILITY = 0.1;

enum class TYPE { NONE = -1, TSP, ATSP };
enum class EDGE_WEIGHT_TYPE { NONE = -1, EXPLICIT, EUC_2D, ATT };
enum class EDGE_WEIGHT_FORMAT { NONE = -1, FULL_MATRIX };
enum class SECTION { NONE = -1, EDGE_WEIGHT_SECTION, NODE_COORD_SECTION };
enum class ALGO { LS, GLS };

TYPE str2type(const std::string& str)
{
    if (str == "TSP")
        return TYPE::TSP;
    else if (str == "ATSP")
        return TYPE::ATSP;
    else
        return TYPE::NONE;
}

EDGE_WEIGHT_TYPE str2edgeWeightType(const std::string& str)
{
    if (str == "EXPLICIT")
        return EDGE_WEIGHT_TYPE::EXPLICIT;
    else if (str == "EUC_2D")
        return EDGE_WEIGHT_TYPE::EUC_2D;
    else if (str == "ATT")
        return EDGE_WEIGHT_TYPE::ATT;
    else
        return EDGE_WEIGHT_TYPE::NONE;
}

EDGE_WEIGHT_FORMAT str2edgeWeightFormat(const std::string& str)
{
    if (str == "FULL_MATRIX")
        return EDGE_WEIGHT_FORMAT::FULL_MATRIX;
    else
        return EDGE_WEIGHT_FORMAT::NONE;
}

SECTION str2section(const std::string& str)
{
    if (str == "EDGE_WEIGHT_SECTION")
        return SECTION::EDGE_WEIGHT_SECTION;
    else if (str == "NODE_COORD_SECTION")
        return SECTION::NODE_COORD_SECTION;
    else
        return SECTION::NONE;
}


namespace {

int str2int(const std::string& str)
{
    int minus = 1;
    int x = 0;
    size_t pos = 0;

    if (str[pos] == '-')
    {
        minus = -1;
        ++pos;
    }
    else
        if (str[pos] == '+')
            ++pos;

    while (pos < str.length() && str[pos] >= '0' && str[pos] <= '9')
        x = x * 10 + str[pos++] - '0';

    x *= minus;

    return x;
}

std::string ltrim(const std::string& str)
{
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(str.find_first_not_of(pattern));
}

std::string rtrim(const std::string& str)
{
  const std::string pattern = " \f\n\r\t\v";
  return str.substr(0,str.find_last_not_of(pattern) + 1);
}

std::string trim(const std::string& str)
{
  return ltrim(rtrim(str));
}

int rand(int a, int b)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(a, b);

    return dist(mt);
}

}


class Tsp
{
public:
    Tsp() {}

    std::string getName() const { return m_name; }

    std::string getDescription() const { return m_comment; }

    int getSize() const { return m_size; }

    void showMatrix() { for (auto i : m_matrix) { std::cout << std::endl; for (auto j : i) std::cout << j << " "; } std::cout << std::endl; }

    void showInitial() { for (auto i : m_population) { std::cout << std::endl; for (auto j : i) std::cout << j << " "; } std::cout << std::endl; }

    bool readFromFile(const std::string& filename)
    {
        std::ifstream ifs(filename.c_str());

        if (!ifs.is_open())
            return false;

        std::string tmp;

        while (getline(ifs, tmp))
        {
            std::string val;

            if (parseParam(tmp, "NAME", val))
                m_name = val;
            else if (parseParam(tmp, "COMMENT", val))
                m_comment = val;
            else if (parseParam(tmp, "DIMENSION", val))
                m_size = str2int(val);
            else if (parseParam(tmp, "EDGE_WEIGHT_TYPE", val))
                m_edgeWeightType = str2edgeWeightType(val);
            else if (parseParam(tmp, "TYPE", val))
                m_type = str2type(val);
            else if (parseParam(tmp, "EDGE_WEIGHT_FORMAT", val))
                m_edgeWeightFormat = str2edgeWeightFormat(val);


            if (isEOF(tmp))
                break;

            if (isSection(tmp))
            {
                if (str2section(tmp) == SECTION::EDGE_WEIGHT_SECTION)
                    readMatrix(ifs);
                else if (str2section(tmp) == SECTION::NODE_COORD_SECTION)
                    readNodeCoord(ifs);
            }
        }

        return true;
    }

    bool readInitial(const std::string& filename)
    {
        std::ifstream ifs(filename);

        if (!ifs.is_open())
            return false;

        while(ifs.good())
        {
            m_population.push_back(std::vector<double>());
            for (size_t i = 0; i < m_size; i++)
            {
                double tmp;
                ifs >> tmp;

                if (!ifs.good())
                {
                    m_population.pop_back();
                    if (m_population.empty())
                        return false;
                    else
                        return true;
                }

                m_population.back().push_back(tmp);
            }
        }

        return true;
    }



    void solve(bool verbose = true)
    {
        std::chrono::time_point<std::chrono::system_clock> start, end;
        int time = 0;

        if (verbose)
        {
            std::cout << "Initial lenght: " << getLenght(m_path) << std::endl;
            std::cout << "Initial: ";
            for (auto i : m_path) std::cout << i << " ";
            std::cout << std::endl;
        }

        start = std::chrono::system_clock::now();

        //...

        end = std::chrono::system_clock::now();
        time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

        if (verbose)
        {
            std::cout << "Iterations: " << m_iterations << std::endl;
            std::cout << "Elapsed time: " << time << " ms" << std::endl;
            std::cout << "Record lenght: " << m_record << std::endl;
            std::cout << "Path: ";
            for (auto i : m_path) std::cout << i << " ";
            std::cout << std::endl << std::endl;
        }
    }

    // 1 - [0;k], 2 - [k+1 ; size-1]
    // Partially Mapped Crossover
    std::vector<double> PMX(const std::vector<double>& p1, const std::vector<double>& p2, size_t k)
    {
        std::vector<double> res;
        std::vector<double> swath;

        for (size_t i = 0; i < p1.size(); i++)
            if (i <= k)
                res.push_back(p1[i]);
            else
                res.push_back(-1);

        for (size_t i = 0; i < p1.size(); i++)
            if (i <= k)
                swath.push_back(p1[i]);
            else
                swath.push_back(-1);

        for (size_t i = 0; i <= k; i++)
        {
            if (std::find(res.begin(), res.end(), p2[i]) == res.end())
            {
                size_t pos = i;
                double val = p2[i];

                while (pos <= k)
                    pos = std::distance(p2.begin(), std::find(p2.begin(), p2.end(), p1[pos]));

                res[pos] = val;
            }
        }

        for (size_t i = 0; i < res.size(); i++)
            if (res[i] == -1)
                res[i] = p2[i];

        return res;
    }

    std::vector<std::vector<double>> selectionForCrossover()
    {
        std::vector<std::vector<double>> result;

        for (size_t i = 0; i < m_population.size() / 2; i++)
        {
            int a = rand(0, m_population.size()-1);
            int b = rand(0, m_population.size()-1);

            while (a == b)
                b = rand(0, m_population.size()-1);

            if (getFitness(m_population[a]) > getFitness(m_population[b]))
                result.push_back(m_population[a]);
            else
                result.push_back(m_population[b]);
        }

        return result;
    }

    std::vector<double> mutation(const std::vector<double>& individual)
    {
        std::vector<double> result(individual);

        double die = 1.0 / rand(0, 100);
        if (die < MUTATION_PROBABILITY)
        {
            int a = rand(0, individual.size() - 1);
            int b = rand(0, individual.size() - 1);

            while (a == b)
                b = rand(0, individual.size() - 1);

            std::swap(result[a], result[b]);
        }

        return result;
    }

    void crossover(const std::vector<std::vector<double>>& population)
    {
        std::vector<std::vector<double>> result;

        int k = rand(1, population[0].size() - 2);

        for (size_t i = 0; i < population.size() - 1; i++)
        {
            m_population.push_back(mutation(PMX(population[i], population[i+1], k)));
            m_population.push_back(mutation(PMX(population[i+1], population[i], k)));
        }
    }

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
    bool isSection(const std::string& str)
    {
        return str2section(str) != SECTION::NONE;
    }

    bool isEOF(const std::string& str)
    {
        if (str == "EOF")
            return true;

        return false;
    }

    bool parseSection(const std::string& str)
    {
        return str2section(str) != SECTION::NONE;
    }

    bool parseParam(const std::string& str, const std::string& name, std::string& result)
    {
        size_t pos;

        if (str.find(name) == std::string::npos)
            return false;

        if ((pos = str.find(':')) == std::string::npos)
            return false;

        std::string val(str, pos + 1, str.size());
        result = trim(val);

        return true;
    }

    bool readMatrix(std::ifstream& ifs)
    {
        for (size_t i = 0; i < m_size; i++)
        {
            m_matrix.push_back(std::vector<double>());
            for (size_t j = 0; j < m_size; j++)
            {
                double tmp;
                ifs >> tmp;

                if (!ifs)
                    return false;

                m_matrix.back().push_back(tmp);
            }
        }

        return true;
    }

    bool readNodeCoord(std::ifstream& ifs)
    {
        std::vector<std::pair<double, double> > coord;
        int n;

        for (size_t i = 0; i < m_size; i++)
        {
            std::pair<double, double> tmp;
            ifs >> n;
            ifs >> tmp.first;
            ifs >> tmp.second;

            coord.push_back(tmp);
        }

        for (size_t i = 0; i < m_size; i++)
        {
            m_matrix.push_back(std::vector<double>());
            for (size_t j = 0; j < m_size; j++)
                if (i == j)
                    m_matrix.back().push_back(INF);
                else
                    m_matrix.back().push_back(dist(coord[i], coord[j]));
        }

        return true;
    }

    double dist(const std::pair<double, double>& a, const std::pair<double, double>& b)
    {
        if (m_edgeWeightType == EDGE_WEIGHT_TYPE::ATT)
        {
            double r = sqrt(((b.first - a.first) * (b.first - a.first) + (b.second - a.second) * (b.second - a.second)) / 10.0);
            return round(r) < r ? round(r) + 1.0 : round(r);
        }

        return sqrt((b.first - a.first) * (b.first - a.first) + (b.second - a.second) * (b.second - a.second));
    }


    //solving
    double getLenght(const std::vector<double>& path)
    {
        double result = 0;

        for (size_t i = 0; i < m_size - 1; i++)
            result += m_matrix[path[i]][path[i + 1]];

        result += m_matrix[path[m_size - 1]][path[0]];

        return result;
    }

    double getFitness(const std::vector<double>& path)
    {
        double sum = 0.0;
        for (auto i : m_population) sum += getLenght(i);
        return 1.0 - getLenght(path) / sum;
    }


};


int main(int argc, char** argv)
{
    Tsp a;

//    std::cout << argv[1] << " " << argv[2] << std::endl;

    if (argc != 3)
    {
       std::cout << "Wrong arguments number:" << argc << std::endl;
       return -1;
    }

    if (!a.readFromFile(argv[1]))
    {
        std::cout << "Task read failed!" << std::endl;
        return -1;
    }

    if (!a.readInitial(argv[2]))
    {
        std::cout << "Initial failed!" << std::endl;
        return -1;
    }

    std::cout << "Name: " << a.getName() << std::endl;
    std::cout << "Description: " << a.getDescription() << std::endl;
    std::cout << "Size: " << a.getSize() << std::endl;
//    std::cout << "Matrix: ";
//    a.showMatrix();
//    std::cout << "Initial: ";
//    a.showInitial();

//    auto res = a.PMX({10, 9, 6, 5, 3, 7, 8, 1, 4, 2}, {10, 5, 3, 7, 4, 1 ,8, 2, 6 ,9}, 2);
//    for (auto i : res) std::cout << i << " "; std::cout << std::endl;

    auto p = a.selectionForCrossover();
    for (auto i : p) { std::cout << std::endl; for (auto j : i) std::cout << j << " "; } std::cout << std::endl;


    return 0;
}
