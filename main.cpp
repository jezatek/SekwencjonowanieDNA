#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <numeric>
#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"

#define CYCLES_NO_CHANGE 70
#define N_Start_Population 2000
#define N_offspring 2000
#define N_Mutation 3000

using namespace std;


int length = 0;
string pattern1, pattern2;

void ReadFromXML(std::vector<std::string>& cells1, std::vector<std::string>& cells2, string& key, int& length, string& start, string& pattern1, string& pattern2)
{
    // Load the XML file into a string
    rapidxml::file<> xmlFile("bio_szukanie5.xml");
    rapidxml::xml_document<> doc;
    doc.parse<0>(xmlFile.data());

    // Find the <dna> node and retrieve its attributes
    rapidxml::xml_node<>* dna_node = doc.first_node("dna");
    if (dna_node) {
        // Retrieve dna attributes
        key = dna_node->first_attribute("key")->value();
        length = std::stoi(dna_node->first_attribute("length")->value());
        start = dna_node->first_attribute("start")->value();

        // Find the first <probe> element and its <cell> elements
        rapidxml::xml_node<>* probe_node = dna_node->first_node("probe");
        if (probe_node) {
            // Retrieve the pattern of the first probe
            pattern1 = probe_node->first_attribute("pattern")->value();

            // Extract cell values from the first probe
            for (rapidxml::xml_node<>* cell_node = probe_node->first_node("cell"); cell_node; cell_node = cell_node->next_sibling("cell")) {
                cells1.push_back(cell_node->value());
            }

            // Move to the next <probe> node
            probe_node = probe_node->next_sibling("probe");
            if (probe_node) {
                // Retrieve the pattern of the second probe
                pattern2 = probe_node->first_attribute("pattern")->value();

                // Extract cell values from the second probe
                for (rapidxml::xml_node<>* cell_node = probe_node->first_node("cell"); cell_node; cell_node = cell_node->next_sibling("cell")) {
                    cells2.push_back(cell_node->value());
                }
            }
        }
    }
    else {
        std::cerr << "No <dna> root node found!" << std::endl;
    }
}

int overlap(const string& a, const string& b) {
    int max_overlap = min(a.size(), b.size());
    for (int i = max_overlap; i > 0; --i) {
        if (a.substr(a.size() - i) == b.substr(0, i)) {
            return i;
        }
    }
    return -1;
}

int F(const vector<int>& order, const vector<string>& oligos) {
    int os = order.size();
    int total_length1 = oligos[order[0]].size();
    int total_length2 = oligos[order[os/2]].size();
    for (size_t i = 1; i < os/2; ++i) {
        total_length1 += oligos[order[i]].size() - overlap(oligos[order[i - 1]], oligos[order[i]]);
    }
    for (size_t i = os/2 + 1; i < os; ++i) {
        total_length2 += oligos[order[i]].size() - overlap(oligos[order[i - 1]], oligos[order[i]]);
    }
    return max(total_length1, total_length2) + 1;
}

int WhereInVector(int element, vector<int>& vec)
{
    auto it = find(vec.begin(), vec.end(), element);
    int index = distance(vec.begin(), it);
    return index;
}

vector<int> RandomOligos(const vector<string>& oligos)
{
    int number_of_random_oligos = length - pattern1.size() + 1;

    vector<int> indexes(oligos.size());
    iota(indexes.begin(), indexes.end(), 0);

    random_shuffle(indexes.begin(), indexes.end());

    vector<int> order(number_of_random_oligos);
    for (int i = 0; i < number_of_random_oligos; i++)
        order[i] = indexes[i];

    return order;
}

pair<int, vector<int>> Generate2(const vector<string>& oligos) {
    vector<int> order = RandomOligos(oligos);
    return { F(order, oligos), order };


    //vector<int> order (oligos.size());
    //iota(order.begin(), order.end(), 0);
    //random_shuffle(order.begin(), order.end());
    //return { F(order, oligos), order };
}

vector<pair<int, vector<int>>> Generation(const vector<string>& oligos, int n) {
    vector<pair<int, vector<int>>> population;
    while (n--) population.push_back(Generate2(oligos));
    return population;
}

void mutate(vector<pair<int, vector<int>>>& population, int n) {
    while (n--) {
        int idx = rand() % population.size();
        int pos1 = rand() % population[idx].second.size();
        int pos2 = rand() % population[idx].second.size();
        swap(population[idx].second[pos1], population[idx].second[pos2]);
    }
}

void Crossover(const vector<pair<int, vector<int>>>& population, int parent1, int parent2, const vector<string>& oligos, vector<pair<int, vector<int>>>& offspring, int idx) {
    const vector<int>& p1 = population[parent1].second;
    const vector<int>& p2 = population[parent2].second;
    int size = length - pattern1.size() + 1;

    vector<int> child1(size, -1), child2(size, -1);
    vector<bool> in_child1(oligos.size(), false), in_child2(oligos.size(), false);

    int start = rand() % size;
    int end = start + rand() % (size - start);

    for (int i = start; i <= end; ++i) {
        child1[i] = p1[i];
        in_child1[p1[i]] = true;
        child2[i] = p2[i];
        in_child2[p2[i]] = true;
    }

    int fill_pos1 = 0, fill_pos2 = 0;
    for (int i = 0; i < size; ++i) {
        if (child1[i] == -1) {
            while (in_child1[p2[fill_pos1]]) fill_pos1++;
            child1[i] = p2[fill_pos1++];
        }
        if (child2[i] == -1) {
            while (in_child2[p1[fill_pos2]]) fill_pos2++;
            child2[i] = p1[fill_pos2++];
        }
    }

    offspring[2 * idx] = { F(child1, oligos), child1 };
    offspring[2 * idx + 1] = { F(child2, oligos), child2 };
}

bool Stop(int best) {
    static unsigned short countdown = CYCLES_NO_CHANGE;
    static int result = best;
    if (best == result) countdown--;
    else {
        countdown = CYCLES_NO_CHANGE;
        result = best;
    }
    return !countdown;
}

int choice(int k) {
    return (int)((double)rand() / RAND_MAX * rand() / RAND_MAX * k);
}

void selection_for_sorted2(vector<pair<int, vector<int>>>& population) {
    vector<pair<int, vector<int>>> new_population(N_Start_Population);
    for (int i = 0; i < N_Start_Population; ++i) {
        int pos = choice(population.size());
        new_population[i] = population[pos];
    }
    population = move(new_population);
}

void Reproduction(vector<pair<int, vector<int>>>& population, int nroffspring, const vector<string>& oligos) {
    vector<pair<int, vector<int>>> offspring(nroffspring);
    for (int i = 0; i < nroffspring / 2; ++i) {
        int choice1 = choice(population.size());
        int choice2 = choice(population.size());
        while (choice1 == choice2) choice2 = choice(population.size());
        Crossover(population, choice1, choice2, oligos, offspring, i);
    }
    population.insert(population.end(), offspring.begin(), offspring.end());
}

int Genetic(const vector<string>& oligos) {
    vector<pair<int, vector<int>>> population = Generation(oligos, N_Start_Population);
    sort(population.begin(), population.end());

    int iteration = 0;
    auto start = chrono::system_clock::now();
    while (!Stop(population[0].first)) {   
        if (iteration % 10 == 0)
            cout << iteration << " " << population[0].first << endl;
        Reproduction(population, N_offspring, oligos);
        mutate(population, N_Mutation);
        sort(population.begin(), population.end());
        selection_for_sorted2(population);
        sort(population.begin(), population.end());
        iteration++;
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "end time: " << elapsed_seconds.count() << endl << "result: " << population[0].first << "\n";
    return population[0].first;
}

int main(int argc, char* argv[]) {
    srand(time(0));

    string key;
    //int length = 0;
    string start;
    //string pattern1, pattern2;
    vector<string> cells1;
    vector<string> cells2;

    ReadFromXML(cells1, cells2, key, length, start, pattern1, pattern2);

    //int N_Oligos = atoi(argv[1]);
    //char* filename = argv[2];

    //vector<string> oligos(N_Oligos);
    //ifstream odczyt(filename);


    vector<string> oligos = cells1;



    //vector<string> test = {"AAA", "BBB", "CCC", "DDD", "EEE", "FFF", "GGG"};
    //vector<int> random_oligos = RandomOligos(test);
    //for (int i = 0; i < random_oligos.size(); i++)
    //    cout << random_oligos[i] << endl;

    //vector<string> test = {"AAA", "AAB", "ABB", "AXT", "GXC", "GXA", "GGG"};
    //cout << overlap(test[3], test[5]) << endl;



    //for (int i = 0; i < N_Oligos; i++)
    //    odczyt >> oligos[i];
    //odczyt.close();

    cout << "Rozpoczynam Genetyczny" << "\n";
    Genetic(oligos);

    return 0;
}
