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


/*************************************CLASSE CATEGORIES E SEUS M�TODOS **************************************/
class Categories {

public:

	int index; 		//�ndice da categoria, de 0 a nCategorias-1
	int compDegree;	//grau de compatibilidade da categoria

	//m�todo que inicializa Categoria usando seu �ndice e seu grau de compatibilidade
	Categories(int k, int degree)
		: index(k), compDegree(degree)
	{}

};

/*************************************ESTRUTURAS DE DADOS e FUN��ES GLOBAIS****************************************/

//matriz de compatibilidades, m por m, em que 0 indica incompatibilidade entre a categoria da linha e da coluna em quest�o (estrutura declarada no mesmo arquivo do main)
extern vector< vector<int> > compatibilityMatrix;

//vetor de categorias do problema (estrutura declarada no mesmo arquivo do main)
extern vector<Categories> allCategories;

//fun��o que imprime uma matriz (vetor de vetores) na tela (definido em cInstances)
extern void printMatrix(vector< vector<int> > matrix);

/*************************************CLASSE INSTANCIAS E SEUS M�TODOS ****************************************/
class Instances {

private:

	string inputFile;		//nome do arquivo com dados de entrada
	int binCapacity;		//capacidade do bin
	int mCategories;		//n�mero de categorias da inst�ncia
	int nElements;			//n�mero de elementos da inst�ncia


	/*************************************ESTRUTURAS DE DADOS ****************************************/

	//vetor de objetos que devem ser inseridos nos bins, ordenados por categoria
	//obs: a posi��o � o �ndice do objeto e o valor armazenado indica o peso
	vector<int> allElements;

	//vetor que associa loja a cada objeto de allElements
	//obs: a posi��o � o �ndice do objeto e o valor armazenado � o �ndice da loja
	vector<int> storeOfElement;

	//vetor com a posi��o do primeiro elemento de cada categoria no vetor que guarda todos os objetos 
	//obs: a posi��o indica categoria e o valor indica posi��o no vetor allElements
	vector<int> firstObjectPosition;


public:

	//construtor que cria inst�ncia usando o nome do arquivo de entrada
	Instances(string str, int m);

	//m�todo que l� e grava a capacidade dos bins e os dados referentes aos objetos, armazenados nas estruturas de dados 
	void load();

	//m�todo que retorna o vetor com todos os objetos
	vector<int> getElements() { return allElements; }

	//m�todo que retorna o peso do objeto j
	int getElementWeight(int j) { return allElements.at(j); }

	//m�todo que retorna o vetor com primeira posi��o de cada categoria
	vector<int> getCategoryPosition() { return firstObjectPosition; }
	
	//m�todo que retorna informa��o do nome da inst�ncia
	string getName() { return inputFile; }

	//m�todo que retorna informa��o de capacidade dos bins da inst�ncia
	int getBinCapacity() { return binCapacity; }

	//m�todo que retorna informa��o de quantidade de categorias
	int getM() { return mCategories; }
	
	//m�todo que retorna informa��o de quantidade de elementos da inst�ncia
	int getN() { return nElements; }

#ifdef M_GRANDE
	//m�todo que acha solu��o �tima do problema usando o Gurobi
	void findLB();
#endif

};


/*************************************CLASSE ELEMENTS E SEUS M�TODOS ****************************************/
class Elements {

public:

	int index;		//�ndice do elemento, de 0 a n-1
	int weight;		//peso do elemento
	int category;	//�ndice da categoria do elemento, de 0 a m-1

	//construtor: inicializa objeto usando seus �ndice, peso e categoria (todos inteiros)
	Elements(int j, int p, int k);

};


/*************************************CLASSE BINS E SEUS M�TODOS ****************************************/
class Bins {

public:

	int capacity;				//capacidade do bin
	int idleCapacity;			//capacidade ociosa do bin

	vector<Elements> elements;	//vetor com elementos do bin
	vector<int> catOnBin;		//vetor de M elementos, que indica o n�mero de objetos da categoria k no bin

	//construtor do bin: dada instancia e elementos, calcula idleCapacity e altera vetor de elementos e de categorias no bin
	Bins(Instances &inst, Elements j);

	//m�todo que verifica se elemento pode ser adicionado ao bin
	bool binFit(Elements j);

	//m�todo que adiciona elemento a um bin existente
	void addElement(Elements j);

	//m�todo que remove elemento de um bin usando o pr�prio elemento a ser removido como argumento
	void removeElement(Elements j);

	//m�todo que remove elemento de um bin usando sua posi��o no vetor de elementos
	void removeItemByPosition(int position);

};


/*************************************CLASSE SOLUTION E SEUS M�TODOS ****************************************/
class Solution {

private:

	string instanceName;			//nome do arquivo de entrada da inst�ncia
	vector<int> codifiedSolution;	//solu��o codificada
	int numberOfBins;				//n�mero de bins utilizados	
	double fitnessValue;			//valor na fun��o fitness
	

public:

	vector<Bins> binsVector;		//vetor de bins - acessa primeiro bin, depois seu vetor de elementos
	vector< vector<int> > solution;	//solu��o em formato matricial: cada linha � um bin
							
	//construtor 1: dado nome da inst�ncia e solu��o em vetor �nico, cria matrix de solu��o  e calcula o n�mero de bins usado
	Solution(string inst, vector<int> solutionCode);

	//construtor 2: dado nome da inst�ncia e vetor de bins, calcula fitness e n�mero de bins, cria solu��o codificada e matriz solution
	Solution(string inst, vector<Bins> bins);

	//m�todo que recalcula fitnessValue de uma solu��o
	void calculateFitness();
	
	//m�todo que atualiza matrz solution de uma solu��o
	void updateSolution();

	//m�todo que retorna o nome da inst�ncia de uma solu��o
	string getInstName() { return instanceName; }

	//m�todo que retorna n�mero de bins da solu��o
	int getNBins() { return numberOfBins; }

	//m�todo que retorna o fitness de uma solu��o
	double getFitnessValue() { return fitnessValue; }

	//m�todo que redefine n�mero de bins de uma solu��o
	void resetNumberBins(int n) { numberOfBins = n; }


};


/*******************************CLASSE EVOLUTIONARY ALGORITHM E SEUS M�TODOS *********************************/
class EA {
private:

	int nItems;						//atributo que guarda o n�mero de elementos da inst�ncia
	int binCapacity;				//atributo que guarda a capacidade dos bins da inst�ncia
	string name;					//nome da instancia, sem extensao do arquivo de entrada
	vector<Solution> solutionPath;	//vetor de melhores solu��es encontradas (pensar se faz sentido)
	double bestFitness;				//atributo que guarda o melhor valor da Fitness Function encontrado
	unsigned bestSolutionPosition;		//atributo que guarda a posi��o da solu��o final (melhor encontrada)

public:

	//construtor da classe EA que usa uma solu��o inicial, a partir da qual executa a heur�stica (usado nos testes)
	EA(Solution &s0);

	//construtor que usa a inst�ncia - primeiro acha solu��o inicial pela construtiva depois executa a heur�stica
	EA(Instances &inst);

	//m�todo que encontra solu��o inicial aplicando o First Fit modificado (categorias ordenadas)
	void generate_s0(Instances &inst);

	//m�todo que aplica o algoritmo evolucion�rio
	void applyEA();

	//m�todo que gera filho
	void generateSun(vector<Solution> &offspring, CRandomMersenne &ran);

	//m�todo que imprime evolu��o da heur�stica
	void printEvolution();

	//m�todos que retornam os valores dos atributos
	vector<Solution> getSolutionPath(){ return solutionPath; }
	double getFinalFitness() { return bestFitness; }
	int getProblemSize() { return nItems; }
	int getBinCap() { return binCapacity; }
	string getInstanceName() { return name; }
	unsigned getFinalSPosition() { return bestSolutionPosition; }

};