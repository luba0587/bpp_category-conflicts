#pragma once


#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "randomc.h"
#include <chrono>
#include <Windows.h>


//#define M_GRANDE 1000000
#define INST_AUGMENTED


#ifdef M_GRANDE
#include "gurobi_c++.h"
#endif


using namespace std;


/*************************************CLASSE para medir tempo de processamento **************************************/
class Timer
{
public:
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const {
		return std::chrono::duration_cast<second_>
			(clock_::now() - beg_).count();
	}

private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
};

/*************************************CLASSE CATEGORIES E SEUS M…TODOS **************************************/
class Categories {

public:

	int index; 		//Ìndice da categoria, de 0 a nCategorias-1
	int compDegree;	//grau de compatibilidade da categoria

	//mÈtodo que inicializa Categoria usando seu Ìndice e seu grau de compatibilidade
	Categories(int k, int degree)
		: index(k), compDegree(degree)
	{}

};

/*************************************ESTRUTURAS DE DADOS e FUN«’ES GLOBAIS****************************************/

//matriz de compatibilidades, m por m, em que 0 indica incompatibilidade entre a categoria da linha e da coluna em quest„o (estrutura declarada no mesmo arquivo do main)
extern vector< vector<int> > compatibilityMatrix;

//vetor de categorias do problema (estrutura declarada no mesmo arquivo do main)
extern vector<Categories> allCategories;

//fator de capacidade do problema
extern double INST_NEW;

//funÁ„o que imprime uma matriz (vetor de vetores) na tela (definido em cInstances)
extern void printMatrix(vector< vector<int> > matrix);

/*************************************CLASSE INSTANCIAS E SEUS M…TODOS ****************************************/
class Instances {

private:

	string inputFile;		//nome do arquivo com dados de entrada
	int binCapacity;		//capacidade do bin
	int mCategories;		//n˙mero de categorias da inst‚ncia
	int nElements;			//n˙mero de elementos da inst‚ncia
	int lowerBound;			//lower bound, dado pela soluÁ„o do problema relaxado (sem compatiblidades e com linearizaÁ„o de xij)
	int bestLB;				//melhor lower bound conhecido - soluÁ„o Ûtima ou resultado de relaxaÁ„o				

	/*************************************ESTRUTURAS DE DADOS ****************************************/

	//vetor de objetos que devem ser inseridos nos bins, ordenados por categoria
	//obs: a posiÁ„o È o Ìndice do objeto e o valor armazenado indica o peso
	vector<int> allElements;

	//vetor que associa loja a cada objeto de allElements
	//obs: a posiÁ„o È o Ìndice do objeto e o valor armazenado È o Ìndice da loja
	vector<int> storeOfElement;

	//vetor com a posiÁ„o do primeiro elemento de cada categoria no vetor que guarda todos os objetos 
	//obs: a posiÁ„o indica categoria e o valor indica posiÁ„o no vetor allElements
	vector<int> firstObjectPosition;

	//mÈtodo que lÍ e grava a capacidade dos bins e os dados referentes aos objetos, armazenados nas estruturas de dados 
	void load();

	//mÈtodo que resolve problema relaxado
	void solveRelaxedProblem();

public:

	//construtor que cria inst‚ncia usando o nome do arquivo de entrada
	Instances(string str, int m);

	//mÈtodo que retorna o vetor com todos os objetos
	vector<int> getElements() { return allElements; }

	//mÈtodo que retorna o peso do objeto j
	int getElementWeight(int j) { return allElements.at(j); }

	//mÈtodo que retorna o vetor com primeira posiÁ„o de cada categoria
	vector<int> getCategoryPosition() { return firstObjectPosition; }
	
	//mÈtodo que retorna informaÁ„o do nome da inst‚ncia
	string getName() { return inputFile; }

	//mÈtodo que retorna informaÁ„o de capacidade dos bins da inst‚ncia
	int getBinCapacity() { return binCapacity; }

	//mÈtodo que retorna informaÁ„o de quantidade de categorias
	int getM() { return mCategories; }
	
	//mÈtodo que retorna informaÁ„o de quantidade de elementos da inst‚ncia
	int getN() { return nElements; }

	//mÈtodo que retorna o lower bound do problema relaxado
	int getRelaxedLB() { return lowerBound; }

	//mÈtodo que retorna o best lower bound do problema
	int getBestLB() { return bestLB; }

	//mÈtodo que infere melhor LB do problema
	void setBestLB(int z) { bestLB = z; }

#ifdef M_GRANDE
	//mÈtodo que acha soluÁ„o Ûtima do problema usando o Gurobi e retorna o n˙mero de bins
	vector<int> findOptimum();
#endif

};


/*************************************CLASSE ELEMENTS E SEUS M…TODOS ****************************************/
class Elements {

public:

	int index;		//Ìndice do elemento, de 0 a n-1
	int weight;		//peso do elemento
	int category;	//Ìndice da categoria do elemento, de 0 a m-1

	//construtor: inicializa objeto usando seus Ìndice, peso e categoria (todos inteiros)
	Elements(int j, int p, int k);

};


/*************************************CLASSE BINS E SEUS M…TODOS ****************************************/
class Bins {

public:

	int capacity;				//capacidade do bin
	int idleCapacity;			//capacidade ociosa do bin

	vector<Elements> elements;	//vetor com elementos do bin
	vector<int> catOnBin;		//vetor de M elementos, que indica o n˙mero de objetos da categoria k no bin

	//construtor do bin: dada instancia e elementos, calcula idleCapacity e altera vetor de elementos e de categorias no bin
	Bins(Instances &inst, Elements j);

	//mÈtodo que verifica se elemento pode ser adicionado ao bin
	bool binFit(Elements j);

	//mÈtodo que adiciona elemento a um bin existente
	void addElement(Elements j);

	//mÈtodo que remove elemento de um bin usando o prÛprio elemento a ser removido como argumento
	void removeElement(Elements j);

	//mÈtodo que remove elemento de um bin usando sua posiÁ„o no vetor de elementos
	void removeItemByPosition(int position);

};


/*************************************CLASSE SOLUTION E SEUS M…TODOS ****************************************/
class Solution {

private:

	string instanceName;			//nome do arquivo de entrada da inst‚ncia
	vector<int> codifiedSolution;	//soluÁ„o codificada
	int numberOfBins;				//n˙mero de bins utilizados	
	float fitnessValue;				//valor na funÁ„o fitness
	float processingTime;			//tempo transcorrido para alcanÁar essa soluÁ„o

public:

	vector<Bins> binsVector;		//vetor de bins - acessa primeiro bin, depois seu vetor de elementos
	vector< vector<int> > solution;	//soluÁ„o em formato matricial: cada linha È um bin
							
	//construtor 1: dado nome da inst‚ncia e soluÁ„o em vetor ˙nico, cria matrix de soluÁ„o  e calcula o n˙mero de bins usado
	Solution(string inst, vector<int> solutionCode);

	//construtor 2: dado nome da inst‚ncia e vetor de bins, calcula fitness e n˙mero de bins, cria soluÁ„o codificada e matriz solution
	Solution(string inst, vector<Bins> bins);

	//mÈtodo que recalcula fitnessValue de uma soluÁ„o
	void calculateFitness();
	
	//mÈtodo que atualiza matrz solution de uma soluÁ„o
	void updateSolution();

	//mÈtodo que retorna o nome da inst‚ncia de uma soluÁ„o
	string getInstName() { return instanceName; }

	//mÈtodo que retorna n˙mero de bins da soluÁ„o
	int getNBins() { return numberOfBins; }

	//mÈtodo que retorna o fitness de uma soluÁ„o
	double getFitnessValue() { return fitnessValue; }

	//mÈtodo que grava o tempo de processamento
	void setTime(float t) { processingTime = t; }

	//mÈtodo que retorna o tempo de rpocessamento de uma soluÁ„o
	float getTime() { return processingTime; }

	//mÈtodo que imprime o vetor soluÁ„o codificada
	vector<int> getCode() { return codifiedSolution; }

	//mÈtodo que redefine n˙mero de bins de uma soluÁ„o
	void resetNumberBins(int n) { numberOfBins = n; }

};


/*******************************CLASSE EVOLUTIONARY ALGORITHM E SEUS M…TODOS *********************************/
class EA {
private:

	int nItems;						//atributo que guarda o n˙mero de elementos da inst‚ncia
	int binCapacity;				//atributo que guarda a capacidade dos bins da inst‚ncia
	string name;					//nome da instancia, sem extensao do arquivo de entrada
	vector<Solution> solutionPath;	//vetor de soluÁıes pais
	double bestFitness;				//atributo que guarda o melhor valor da Fitness Function encontrado
	unsigned bestSolutionPosition;	//atributo que guarda a posiÁ„o da soluÁ„o final (melhor encontrada)
	int relaxedLB;					//vari·vel que guarda o LB do problema, dado pelo BPP com xij real

public:

	//construtor da classe EA que usa uma solução inicial, a partir da qual executa a heurÌstica (usado nos testes)
	EA(Solution &s0);

	//construtor que usa a instância - primeiro acha solução inicial pela construtiva depois executa a heurÌstica
	EA(Instances &inst);

	//mÈtodo que encontra solução inicial aplicando o First Fit modificado (categorias ordenadas)
	void generate_s0(Instances &inst);

	//mÈtodo que aplica o algoritmo evolucionário
	void applyEA();

	//mÈtodo que gera filho
	void generateSun(vector<Solution> &offspring, CRandomMersenne &ran, Timer t);

	//mÈtodo que imprime evolução da heurística
	void printEvolution();

	//mÈtodos que retornam os valores dos atributos
	vector<Solution> getSolutionPath(){ return solutionPath; }
	double getFinalFitness() { return bestFitness; }
	int getProblemSize() { return nItems; }
	int getBinCap() { return binCapacity; }
	string getInstanceName() { return name; }
	unsigned getFinalSPosition() { return bestSolutionPosition; }
	int getLB1() { return relaxedLB; }

};
