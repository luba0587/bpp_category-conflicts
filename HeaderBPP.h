#pragma once


#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
//#include "mersenne.cpp"
#include "randomc.h"


#define M_GRANDE 1000000

#ifdef M_GRANDE
#include "gurobi_c++.h"
#endif


using namespace std;


/*************************************CLASSE CATEGORIES E SEUS MÉTODOS **************************************/
class Categories {

public:

	int index; 		//índice da categoria, de 0 a nCategorias-1
	int compDegree;	//grau de compatibilidade da categoria

	//método que inicializa Categoria usando seu índice e seu grau de compatibilidade
	Categories(int k, int degree)
		: index(k), compDegree(degree)
	{}

};

/*************************************ESTRUTURAS DE DADOS e FUNÇÕES GLOBAIS****************************************/

//matriz de compatibilidades, m por m, em que 0 indica incompatibilidade entre a categoria da linha e da coluna em questão (estrutura declarada no mesmo arquivo do main)
extern vector< vector<int> > compatibilityMatrix;

//vetor de categorias do problema (estrutura declarada no mesmo arquivo do main)
extern vector<Categories> allCategories;

//função que imprime uma matriz (vetor de vetores) na tela (definido em cInstances)
extern void printMatrix(vector< vector<int> > matrix);

/*************************************CLASSE INSTANCIAS E SEUS MÉTODOS ****************************************/
class Instances {

private:

	string inputFile;		//nome do arquivo com dados de entrada
	int binCapacity;		//capacidade do bin
	int mCategories;		//número de categorias da instância
	int nElements;			//número de elementos da instância


	/*************************************ESTRUTURAS DE DADOS ****************************************/

	//vetor de objetos que devem ser inseridos nos bins, ordenados por categoria
	//obs: a posição é o índice do objeto e o valor armazenado indica o peso
	vector<int> allElements;

	//vetor que associa loja a cada objeto de allElements
	//obs: a posição é o índice do objeto e o valor armazenado é o índice da loja
	vector<int> storeOfElement;

	//vetor com a posição do primeiro elemento de cada categoria no vetor que guarda todos os objetos 
	//obs: a posição indica categoria e o valor indica posição no vetor allElements
	vector<int> firstObjectPosition;


public:

	//construtor que cria instância usando o nome do arquivo de entrada
	Instances(string str, int m);

	//método que lê e grava a capacidade dos bins e os dados referentes aos objetos, armazenados nas estruturas de dados 
	void load();

	//método que retorna o vetor com todos os objetos
	vector<int> getElements() { return allElements; }

	//método que retorna o peso do objeto j
	int getElementWeight(int j) { return allElements.at(j); }

	//método que retorna o vetor com primeira posição de cada categoria
	vector<int> getCategoryPosition() { return firstObjectPosition; }
	
	//método que retorna informação do nome da instância
	string getName() { return inputFile; }

	//método que retorna informação de capacidade dos bins da instância
	int getBinCapacity() { return binCapacity; }

	//método que retorna informação de quantidade de categorias
	int getM() { return mCategories; }
	
	//método que retorna informação de quantidade de elementos da instância
	int getN() { return nElements; }

#ifdef M_GRANDE
	//método que acha solução ótima do problema usando o Gurobi
	void findLB();
#endif

};


/*************************************CLASSE ELEMENTS E SEUS MÉTODOS ****************************************/
class Elements {

public:

	int index;		//índice do elemento, de 0 a n-1
	int weight;		//peso do elemento
	int category;	//índice da categoria do elemento, de 0 a m-1

	//construtor: inicializa objeto usando seus índice, peso e categoria (todos inteiros)
	Elements(int j, int p, int k);

};


/*************************************CLASSE BINS E SEUS MÉTODOS ****************************************/
class Bins {

public:

	int capacity;				//capacidade do bin
	int idleCapacity;			//capacidade ociosa do bin

	vector<Elements> elements;	//vetor com elementos do bin
	vector<int> catOnBin;		//vetor de M elementos, que indica o número de objetos da categoria k no bin

	//construtor do bin: dada instancia e elementos, calcula idleCapacity e altera vetor de elementos e de categorias no bin
	Bins(Instances &inst, Elements j);

	//método que verifica se elemento pode ser adicionado ao bin
	bool binFit(Elements j);

	//método que adiciona elemento a um bin existente
	void addElement(Elements j);

	//método que remove elemento de um bin usando o próprio elemento a ser removido como argumento
	void removeElement(Elements j);

	//método que remove elemento de um bin usando sua posição no vetor de elementos
	void removeItemByPosition(int position);

};


/*************************************CLASSE SOLUTION E SEUS MÉTODOS ****************************************/
class Solution {

private:

	string instanceName;			//nome do arquivo de entrada da instância
	vector<int> codifiedSolution;	//solução codificada
	int numberOfBins;				//número de bins utilizados	
	double fitnessValue;			//valor na função fitness
	

public:

	vector<Bins> binsVector;		//vetor de bins - acessa primeiro bin, depois seu vetor de elementos
	vector< vector<int> > solution;	//solução em formato matricial: cada linha é um bin
							
	//construtor 1: dado nome da instância e solução em vetor único, cria matrix de solução  e calcula o número de bins usado
	Solution(string inst, vector<int> solutionCode);

	//construtor 2: dado nome da instância e vetor de bins, calcula fitness e número de bins, cria solução codificada e matriz solution
	Solution(string inst, vector<Bins> bins);

	//método que recalcula fitnessValue de uma solução
	void calculateFitness();
	
	//método que atualiza matrz solution de uma solução
	void updateSolution();

	//método que retorna o nome da instância de uma solução
	string getInstName() { return instanceName; }

	//método que retorna número de bins da solução
	int getNBins() { return numberOfBins; }

	//método que retorna o fitness de uma solução
	double getFitnessValue() { return fitnessValue; }

	//método que redefine número de bins de uma solução
	void resetNumberBins(int n) { numberOfBins = n; }


};


/*******************************CLASSE EVOLUTIONARY ALGORITHM E SEUS MÉTODOS *********************************/
class EA {
private:

	int nItems;						//atributo que guarda o número de elementos da instância
	int binCapacity;				//atributo que guarda a capacidade dos bins da instância
	string name;					//nome da instancia, sem extensao do arquivo de entrada
	vector<Solution> solutionPath;	//vetor de melhores soluções encontradas (pensar se faz sentido)
	double bestFitness;				//atributo que guarda o melhor valor da Fitness Function encontrado
	unsigned bestSolutionPosition;		//atributo que guarda a posição da solução final (melhor encontrada)

public:

	//construtor da classe EA que usa uma solução inicial, a partir da qual executa a heurística (usado nos testes)
	EA(Solution &s0);

	//construtor que usa a instância - primeiro acha solução inicial pela construtiva depois executa a heurística
	EA(Instances &inst);

	//método que encontra solução inicial aplicando o First Fit modificado (categorias ordenadas)
	void generate_s0(Instances &inst);

	//método que aplica o algoritmo evolucionário
	void applyEA();

	//método que gera filho
	void generateSun(vector<Solution> &offspring, CRandomMersenne &ran);

	//método que imprime evolução da heurística
	void printEvolution();

	//métodos que retornam os valores dos atributos
	vector<Solution> getSolutionPath(){ return solutionPath; }
	double getFinalFitness() { return bestFitness; }
	int getProblemSize() { return nItems; }
	int getBinCap() { return binCapacity; }
	string getInstanceName() { return name; }
	unsigned getFinalSPosition() { return bestSolutionPosition; }

};