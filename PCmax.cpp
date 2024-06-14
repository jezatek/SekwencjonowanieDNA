

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <ctime>
#include <chrono>
 
#define CYCLES_NO_CHANGE 100
#define N_Start_Population 3000
#define N_offspring 3000
 
using namespace std;
 
int Max_t(int n, int *t) {
    int maxi = 0;
    for (int i = 1; i < n; i++)
        if (t[i] > t[maxi]) maxi = i;
    return maxi;
}
 
int F(int gens[], int tasks[], int N_Tasks, int N_Proc) {
    int il_gen = N_Tasks;
    int *res = new int[N_Proc]();
    for (int i = 0; i < il_gen; i++)
        res[gens[i]] += tasks[i];
    il_gen = Max_t(N_Proc, res);
    il_gen = res[il_gen];
    delete[] res;
    return il_gen;
}
 
pair<int, int*> Generate2(int *tasks, int N_Tasks, int N_Proc) {
    int *t = new int[N_Tasks]();
    for (int i = 0; i < N_Tasks; i++) t[i] = rand() % N_Proc;
    return {F(t, tasks, N_Tasks, N_Proc), t};
}
 
vector<pair<int, int*>> Generation(int *tasks, int n, int N_Tasks, int N_Proc) {
    vector<pair<int, int*>> t;
    while (n--) t.push_back(Generate2(tasks, N_Tasks, N_Proc));
    return t;
}
 
void mutate(vector<pair<int, int*>> &v, int n, int N_Tasks, int N_Proc) {
    while (n--) v[rand() % v.size()].second[rand() % N_Tasks] = rand() % N_Proc;
}
 
void Crossover(vector<pair<int, int*>> &v, int first, int sec, int tasks[], int N_Tasks, int N_Proc, vector<pair<int, int*>>& n_v,int i) {
 
    int *sibling1 = new int[N_Tasks]();
    int *sibling2 = new int[N_Tasks]();
    int division = 1 + rand() % (N_Tasks - 2);
    for (int i = 0; i < division && i < N_Tasks; i++) {
        sibling1[i] = v[first].second[i];
        sibling2[i] = v[sec].second[i];
    }
    for (int i = division; i < N_Tasks; i++) {
        sibling2[i] = v[first].second[i];
        sibling1[i] = v[sec].second[i];
    }
    n_v[2*i]={F(sibling1, tasks, N_Tasks, N_Proc), sibling1};
    n_v[2*i+1]={F(sibling2, tasks, N_Tasks, N_Proc), sibling2};
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
    int w = (int)((double)rand() / RAND_MAX*rand() / RAND_MAX * k);
    return w;
}
 
void selection_for_sorted2(vector<pair<int, int*>> &gens,int N_Proc,int N_Tasks) {
    vector<pair<int, int*>> new_gens(N_Start_Population);
 
    for (int i=0;i<N_Start_Population;i++) {
        int pos = choice(gens.size());
        int* new_array = new int[N_Tasks];
        copy(gens[pos].second, gens[pos].second + N_Tasks, new_array);
        new_gens[i]={gens[pos].first, new_array};
    }
    for (auto& gen : gens) {
        delete[] gen.second;
    }
    gens.clear();
    gens = move(new_gens);
}
 
void Reproduction(vector<pair<int, int*>> &gens, int nroffspring, int tasks[], int N_Tasks, int N_Proc) {
    vector<pair<int, int*>> n_v(nroffspring);
    for(int i=0;i<nroffspring/2;i++) {
        int choice1 = choice(gens.size());
        int choice2 = choice(gens.size());
        while (choice1 == choice2) choice2 = choice(gens.size());
        Crossover(gens, choice1, choice2, tasks, N_Tasks, N_Proc,n_v,i);
    }
    gens.insert(gens.end(), n_v.begin(), n_v.end());
}
 
int Genetic(int *tasks, int N_Tasks, int N_Proc) {
    vector<pair<int, int*>> population = Generation(tasks, N_Start_Population, N_Tasks, N_Proc);
    sort(population.begin(), population.end());
 
    int iteration = 0;
    auto start = chrono::system_clock::now();
    while (!Stop(population[0].first)) {
        if(iteration == 100){
            auto end0 = chrono::system_clock::now();
            Reproduction(population, N_offspring, tasks, N_Tasks, N_Proc);
            auto end1 = chrono::system_clock::now();
            mutate(population, 1000, N_Tasks, N_Proc);
            auto end2 = chrono::system_clock::now();
            sort(population.begin(), population.end());
            auto end3 = chrono::system_clock::now();
            selection_for_sorted2(population,N_Proc,N_Tasks);
            auto end4 = chrono::system_clock::now();
            chrono::duration<double> elapsed_seconds = end4-start;
            chrono::duration<double> e0 = end1-end0;
            chrono::duration<double> e1 = end2-end1;
            chrono::duration<double> e2 = end3-end2;
            chrono::duration<double> e3 = end4-end3;
            cout<<"100 iterations: "<<elapsed_seconds.count()<<endl;
            cout<<e0.count()<<" "<<e1.count()<<" "<<e2.count()<<" "<<e3.count()<<endl;
        }
        else{
            Reproduction(population, N_offspring, tasks, N_Tasks, N_Proc);
            mutate(population, 1000, N_Tasks, N_Proc);
            sort(population.begin(), population.end());
            selection_for_sorted2(population,N_Proc,N_Tasks);
        }
        // printf("it %d\t%d %d %d\n", iteration, population[0].first, population[1].first, population[2].first);
        iteration++;
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end-start;
    cout<<"end time: "<<elapsed_seconds.count()<<endl<<"result: "<<population[0].first<<"\n";
    return population[0].first;
}
 
int main(int argc, char *argv[]) { 
    // srand(time(0));
 
    int N_Proc = atoi(argv[1]);
    int N_Tasks = atoi(argv[2]);
    char* filename = argv[3];
 
    int *tasks;
    tasks = new int[N_Tasks]();
    ifstream odczyt(filename);
    for (int i = 0; i < N_Tasks; i++)
        odczyt>>tasks[i];
    odczyt.close();
 
    cout<<"Rozpoczynam Genetyczny"<<"\n";
    Genetic(tasks, N_Tasks, N_Proc);
 
    delete[] tasks;
    return 0;
}

