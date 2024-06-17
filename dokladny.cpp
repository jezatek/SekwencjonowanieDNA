#include "rapidxml-1.13/rapidxml.hpp"
#include "rapidxml-1.13/rapidxml_utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iterator>

using namespace std;

void ReadFromXML(std::vector<std::string>& cells1, std::vector<std::string>& cells2, string &key, int &length, string& start, string& pattern1, string& pattern2)
{
    // Load the XML file into a string
    rapidxml::file<> xmlFile("bio.xml");
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

void DisplayXMLData(std::vector<std::string>& cells1, std::vector<std::string>& cells2, string& key, int& length, string& start, string& pattern1, string& pattern2)
{
    cout << "Key: " << key << endl;
    cout << "Length: " << length << endl;
    cout << "Start: " << start << endl;
    cout << "Pattern1: " << pattern1 << endl;
    cout << "Pattern2: " << pattern2 << endl;

    cout << endl;
    cout << endl;

    std::cout << "Cells from the first probe:" << std::endl;
    for (int i = 0; i < cells1.size(); i++)
        cout << cells1[i] << endl;

    cout << endl;
    cout << endl;

    std::cout << "Cells from the second probe:" << std::endl;
    for (int i = 0; i < cells2.size(); i++)
        cout << cells2[i] << endl;
}

string EvenStringStart(string &start)
{
    string even_string = "";
    for (int i = 0; i < start.size(); i++)
    {
        if (i % 2 == 0)
            even_string += start[i];
        else if(i % 2 == 1 && i != start.size() - 1)
            even_string += 'X';
    }
    return even_string;
}

string OddStringStart(string& start)
{
    string even_string = "";
    for (int i = 0; i < start.size(); i++)
    {
        if (i % 2 == 1)
            even_string += start[i];
        else if (i % 2 == 0 && i != start.size() - 1)
            even_string += 'X';
    }
    return even_string;
}


int WhereInVector(int element, vector<int> &vec)
{
    // Iterate through the vector and find the position of the element
    for (int i = 0; i < vec.size(); ++i) {
        if (vec[i] == element) {
            return i;  // Return the index of the element if found
        }
    }
    return vec.size();  // Return -1 if element is not found
}

bool EvenValidation(string &even_string, string& odd_string, vector<string> &cells2, vector<int> &used_cells2)
{
    for (int i = 0; i < cells2.size(); i++)
    {
        if (WhereInVector(i, used_cells2) != used_cells2.size())
            continue;

        string validator = cells2[i];
        if (even_string[even_string.size() - 1] != validator[validator.size() - 1])
            continue;
        for (int j = 0; j < validator.size(); j += 2)
        {
            if (odd_string[odd_string.size()-validator.size()+1+j] != validator[j])
                continue;
        }

        used_cells2.push_back(i);
        return true;
    }
    return false;
}

bool AddToOdd(string& odd_string, string adder)
{
    int os = odd_string.size(), as = adder.size();
    for (int i = 0; i < as-2; i = i + 2)
    {
        char a =odd_string[os-as+i+2],b=adder[i];
        if (odd_string[os-as+i+2] != adder[i])
            return false;
    }
    odd_string += 'X';
    odd_string += adder[adder.size() - 1];
    return true;
}

void Subtract(string& str, vector<int>& used_cells1)
{
    str.pop_back();
    str.pop_back();
    used_cells1.pop_back();
}

bool GetSequenceDFS(string& even_string, string& odd_string, vector<string>& cells1, vector<string>& cells2, vector<int>& used_cells1, vector<int>& used_cells2, int& length)
{
    if (even_string.size() == length || odd_string.size() == length)
        return true;

    bool even_turn = true;
    if (odd_string.size() < even_string.size())
        even_turn = false;

    if (even_turn == true)
    {
        for (int i = 0; i < cells1.size(); i++)
        {
            if (WhereInVector(i, used_cells1) != used_cells1.size())
                continue;

            string adder = cells1[i];
            bool even_adding_success = AddToOdd(even_string, adder);
            if (even_adding_success == true)
            {
                used_cells1.push_back(i);
                bool even_validation = EvenValidation(even_string, odd_string, cells2, used_cells2);
                if (even_validation == true)
                {
                    bool success = GetSequenceDFS(even_string, odd_string, cells1, cells2, used_cells1, used_cells2, length);
                    if (success == true)
                        return true;
                    else
                    {
                        Subtract(even_string, used_cells1);
                        used_cells2.pop_back();
                    }
                }
                else
                    Subtract(even_string, used_cells1);
            }
        }
        return false;
    }
    else
    {
        for (int i = 0; i < cells1.size(); i++)
        {
            if (WhereInVector(i, used_cells1) != used_cells1.size())
                continue;

            string adder = cells1[i];
            bool odd_adding_success = AddToOdd(odd_string, adder);
            if (odd_adding_success == true)
            {
                used_cells1.push_back(i);
                bool odd_validation = EvenValidation(odd_string, even_string, cells2, used_cells2);
                if (odd_validation == true)
                {
                    bool success = GetSequenceDFS(even_string, odd_string, cells1, cells2, used_cells1, used_cells2, length);
                    if (success == true)
                        return true;
                    else
                    {
                        Subtract(odd_string, used_cells1);
                        used_cells2.pop_back();
                    }
                }
                else
                    Subtract(odd_string, used_cells1);
            }
        }
        return false;
    }
    return false;
}

int main() /////////////////////////////////////////usunac samego siebie - ominonukleotyd startowy
{
    string key;
    int length = 0;
    string start;
    string pattern1, pattern2;
    vector<string> cells1;
    vector<string> cells2;

    ReadFromXML(cells1, cells2, key, length, start, pattern1, pattern2);

    vector<int> used_cells1;
    vector<int> used_cells2;

    string even_string = EvenStringStart(start);
    string odd_string = OddStringStart(start);

    //DisplayXMLData(cells1, cells2, key, length, start, pattern1, pattern2);

    cout << "start:       " << start << endl;
    cout << "even_string: " << even_string << endl;
    cout << "odd_string:  " << odd_string << endl;

    cout << "cells1[0].size(): " << cells1[0].size() << endl;
    cout << "cells2[0].size(): " << cells2[0].size() << endl;

    cout << "cells1[0]: " << cells1[0] << endl;
    cout << "cells2[0]: " << cells2[0] << endl;
    bool success = GetSequenceDFS(even_string, odd_string, cells1, cells2, used_cells1, used_cells2, length);
    cout << "success: " << success << endl;
    cout << "even_string: " << even_string << endl;
    cout << "odd_string:  " << odd_string << endl;






    return 0;
}