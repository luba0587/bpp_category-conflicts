#include "HeaderBPP.h"


//construtor: inicializa objeto usando seus índice, peso e categoria (todos inteiros)
Elements::Elements(int j, int p, int k)
	: index(j), weight(p), category(k)
{}



//construtor do bin - inicializa já colocando um elemento no bin criado
Bins::Bins(Instances &inst, Elements j)
: capacity(inst.getBinCapacity()), idleCapacity(inst.getBinCapacity() - j.weight)
{
	elements.push_back(j);
	catOnBin.resize(inst.getM(), 0);
	catOnBin.at(j.category) += 1;
}


//método que verifica se elemento pode ser adicionado ao bin
bool Bins::binFit(Elements j) {

	//se houver capacidade suficiente no bin
	if (j.weight <= idleCapacity) {

		//... e já existir objeto da mesma categoria de j no bin, retorna VERDADEIRO
		if (catOnBin.at(j.category) > 0) return true;

		//... senão precisa checar se a categoria é compatível com os objetos que já estão no bin
		else {

			//percorre todos as colunas da matriz de compatibilidade, linha j.category 
			for (unsigned k = 0; k<compatibilityMatrix.at(j.category).size(); k++) {

				//se categoria k é incompatível com categoria de j e existe elemento de k no bin
				if (compatibilityMatrix.at(j.category).at(k)<1 && catOnBin.at(k) > 0) {
					return false;
					break;
				}

			}//fim do loop que percorre todas as categorias, buscando incompatíveis

			 //se tiver chegado ao fim do loop, é porque não foi encontrado objeto incompatível no bin
			return true;

		}

	}

	//se não há capacidade suficiente no bin, retorna falso
	else return false;

}//fim do método


//método que adiciona elemento a um bin existente
void Bins::addElement(Elements j) {
	elements.push_back(j);
	idleCapacity -= j.weight;
	catOnBin.at(j.category) += 1;
}


void Bins::removeElement(Elements item) {

	//percorre o vetor de elementos posição a posição
	for (unsigned j = 0; j < elements.size(); j++) {

		//quando encontra o item a ser removido do bin...
		if (item.index == elements.at(j).index) {
		
			//remove item substituindo pelo último elemento do vetor de elementos
			elements.at(j) = elements.back();
			elements.pop_back();

			//atualiza capacidade ociosa do bin
			idleCapacity += item.weight;

			//atualiza vetor com contagem das categorias do bin
			catOnBin.at(item.category) -= 1;

			//sai do loop
			break;

		}//fim da condicional usada ao encontrar o elemento buscado

	}//fim do loop que percorre vetor de elementos no bin


}

void Bins::removeItemByPosition(int j) {

	//atualiza capacidade ociosa do bin
	idleCapacity += elements.at(j).weight;

	//atualiza vetor com contagem das categorias do bin
	catOnBin.at(elements.at(j).category) -= 1;
	
	//remove item substituindo pelo último elemento do vetor de elementos
	elements.at(j) = elements.back();
	elements.pop_back();
	
}