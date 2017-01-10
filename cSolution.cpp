#include "HeaderBPP.h"

//construtor que cria matrix de solu��o decodificando a representa��o da solu��o em vetor �nico
Solution::Solution(string inst, vector<int> solutionCode)
	: instanceName(inst), codifiedSolution(solutionCode)
{
	vector<int> elementsOnBin;
	for (unsigned j = 0; j < solutionCode.size(); j++) {
		if (solutionCode.at(j) >= 0) elementsOnBin.push_back(solutionCode.at(j));
		else if (elementsOnBin.size()>0) {
			solution.push_back(elementsOnBin);
			elementsOnBin.clear();
		}
	}
	solution.push_back(elementsOnBin);
	numberOfBins = solution.size();
}


//construtor que cria matriz de solu��o decodificando a representa��o da solu��o em vetor de bins
Solution::Solution(string inst, vector<Bins> bins)
	: instanceName(inst), binsVector(bins), fitnessValue(0)
{
	for (unsigned b = 0; b<bins.size(); b++) {
		codifiedSolution.push_back(-1 * (b + 1));
		vector<int> elementsOnBin;
		for (unsigned j = 0; j<bins.at(b).elements.size(); j++) {
			elementsOnBin.push_back(bins.at(b).elements.at(j).index);
			codifiedSolution.push_back(bins.at(b).elements.at(j).index);
		}
		solution.push_back(elementsOnBin);
		double fitnessContribution = 1.0 - (double)(bins.at(b).idleCapacity) / (double) (bins.at(b).capacity);
		fitnessContribution = fitnessContribution*fitnessContribution;
		fitnessValue += fitnessContribution;
	}
	numberOfBins = solution.size();
}

//m�todo que calcula o fitnessValue de uma solu��o
void Solution::calculateFitness() {
	
	//zera o valor da fitness function
	fitnessValue = 0.0;

	//para cada bin do vetor de bins da solu��o
	for (unsigned b = 0; b < numberOfBins; b++) {

		//calcula a parcela de contribui��o � Fitness Function do bin b (capacity-idleCapacity)/capacity
		double fitnessContribution = 1.0 - (double)(binsVector.at(b).idleCapacity) / (double)(binsVector.at(b).capacity);

		//eleva ao quadrado tal parcela
		fitnessContribution = fitnessContribution*fitnessContribution;

		//soma tal parcela ao fitnessValue da solu��o
		fitnessValue += fitnessContribution;

	}

}

//m�todo que cria matriz solution e vetor com solu��o codificada a partir de vetor de bins, atualizando tamb�m o n�mero de bins
void Solution::updateSolution() {

	//limpa as duas estruturas de dados a atualizar
	codifiedSolution.clear();
	solution.clear();

	//percorre o vetor de bins da solu��o, selecionando um a um
	for (unsigned b = 0; b<binsVector.size(); b++) {
		
		//insere o separador na solu��o codificada, com valor = �ndice do bin, negativo
		codifiedSolution.push_back(-1 * (b + 1));
		
		//cria vetor de inteiros para guardar os �ndices dos objetos de cada bin
		vector<int> elementsOnBin;
		
		//percorre o vetor de elementos do bin, populando o vetor de elementos e da solu��o codificada
		for (unsigned j = 0; j<binsVector.at(b).elements.size(); j++) {
			elementsOnBin.push_back(binsVector.at(b).elements.at(j).index);
			codifiedSolution.push_back(binsVector.at(b).elements.at(j).index);
		}

		//guarda o vetor de inteiros na matriz solution
		solution.push_back(elementsOnBin);

	}//fim do loop que percorre todos os bins da solu��o

	//atualiza o n�mero de bins na solu��o
	numberOfBins = solution.size();

}