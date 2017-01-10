#include "HeaderBPP.h"
#include "randomc.h"


//método de ordenação de categorias não-decrescentemente pelo grau de compatibilidade
bool increasingCompDegree(Categories lhs, Categories rhs) {
	return lhs.compDegree < rhs.compDegree;
}

//construtor da classe EA: cria instância usando solução inicial, executa heurística
EA::EA(Solution &s0)
	: bestFitness(s0.getFitnessValue())
{
	solutionPath.push_back(s0);
	applyEA();
}

//construtor que usa a instância - primeiro acha solução inicial pela construtiva depois executa a heurística
EA::EA(Instances &inst)
	:nItems(inst.getN()), binCapacity(inst.getBinCapacity()), name(inst.getName()), bestSolutionPosition(0)
{
	name.resize(name.size() -4);
	generate_s0(inst);
	bestFitness = solutionPath.front().getFitnessValue();
	applyEA();
	printEvolution();
}

//método que encontra solução inicial aplicando o First Fit modificado (categorias ordenadas) e a insere em SolutionPath
void EA::generate_s0(Instances &inst) {

	//Cria estruturas de dados
	vector<Elements> notPickedElements;
	vector<Bins> resultingBins;

	//popula vetor de elementos
	for (int k = 0; k < inst.getM(); k++) {

		int last_j;
		if (k + 1 < inst.getM()) last_j = inst.getCategoryPosition().at(k + 1) - 1;
		else last_j = inst.getN() - 1;

		for (int j = inst.getCategoryPosition().at(k); j <= last_j; j++) {

			Elements object(j, inst.getElementWeight(j), k);
			notPickedElements.push_back(object);

		}

	}

	//ordena as categorias crescentemente pelo grau de compatibilidade (pra começar por menos compatíveis)
	vector<Categories> sortedCategories = allCategories;
	sort(sortedCategories.begin(), sortedCategories.end(), increasingCompDegree);

	//percorre o vetor de categorias ordenadas, selecionando uma a uma
	for (unsigned k = 0; k < sortedCategories.size(); k++) {

		//calcula o índice do último elemento dessa categoria
		int last_j;
		if (sortedCategories.at(k).index + 1 < (int)sortedCategories.size()) last_j = inst.getCategoryPosition().at(sortedCategories.at(k).index + 1) - 1;
		else last_j = inst.getN() - 1;

		//percorre uma a uma as tarefas da categoria em questão
		for (int j = inst.getCategoryPosition().at(sortedCategories.at(k).index); j <= last_j; j++) {

			//abre primeiro bin, se não houver nenhum aberto
			if (resultingBins.size()<1) {
				Bins b(inst, notPickedElements.at(j));
				resultingBins.push_back(b);
			}

			//caso contrário, tenta alocar no primeiro bin disponível
			else {

				//inicializa variável que indica se foi possível alocar o elemento em um bin já aberto
				int alocated = 0;

				//percorre o vetor de bins procurando um onde o objeto caiba
				for (unsigned b = 0; b<resultingBins.size(); b++) {

					//se o objeto cabe no bin b, aloca e sai do loop
					if (resultingBins.at(b).binFit(notPickedElements.at(j)) == 1) {
						resultingBins.at(b).addElement(notPickedElements.at(j));
						alocated = 1;
						break;
					}

				}

				//se percorreu o vetor de bins e não conseguiu encaixar o elemento, cria um novo bin
				if (alocated <1) {
					Bins b(inst, notPickedElements.at(j));
					resultingBins.push_back(b);
				}

			}//fim do loop que tenta alocar o elemento nos bins já abertos e, se não consegue, cria um novo bin

		}//fim do loop que aloca todos os elementos da k-ésima categoria ordenada

	}//fim do loop que percorre todas as categorias do problema

	//cria objeto da classe solução usando o vetor de bins construído pela heurística
	Solution s(inst.getName(), resultingBins);

	//adiciona solução ao vetor solutionPath da classe EA
	solutionPath.push_back(s);

}

struct Geracoes {

	unsigned geracao;
	vector<Solution> filhos;

};


//aplica meta-heurística (1-lambda) Estratégia Evolucionária
void EA::applyEA() {

	//critério de parada: número máximo de soluções avaliadas
	unsigned stopCriterium = 300;

	//número de filhos de cada geração
	unsigned offspring = 15;

	//critério de estagnação (número máximo de gerações sem melhoria na solução incumbente)
	unsigned maxStagnation = 10;
	
	//contador de gerações sem melhoria
	unsigned stagCounter = 0;

	//cria objeto da classe de gerarador de números aleatórios
	CRandomMersenne ran(1);

	//cria estrutura que guarda todos os resultados para gerar tabela 2
	vector<Geracoes> evolucaoHeuristica;

	//em cada geração de filhos criada:
	for (unsigned gen = 0; gen < stopCriterium / offspring; gen++) {

		//estrutura que guarda todas as soluções filho de uma geração
		vector<Solution> offspringVector;

		//variáveis que guardam a posição do melhor filho no vetor offspringVector e seu valor na função de fitness
		unsigned bestSunPosition=0;
		double bestSunValue=0;

		//gera filhos a partir da solução pai, que é a última solução do vetor solutionPath
		for (unsigned sun = 0; sun < offspring; sun++) {

			//cria filho a partir de mutação do pai e o adiciona no vetor offspring (se não houver sucesso em maxTrials tentativas, simplesmente não cria filho!)
			generateSun(offspringVector, ran);

		}//fim do loop que gera 'offspring' filhos a partir de mutações da solução pai

		//escolhe o melhor filho da prole para ser próximo pai
		for (unsigned sun = 0; sun < offspringVector.size(); sun++) {

			if (offspringVector.at(sun).getFitnessValue() > bestSunValue) {
				bestSunPosition = sun;
				bestSunValue = offspringVector.at(sun).getFitnessValue();
			}

		}//fim do loop que percorre todos os filhos no vetor offspringVector para selecionar o melhor deles

		//insere o melhor filho da prole na última posição do vetor solutionPath
		if (offspringVector.size()>0) solutionPath.push_back(offspringVector.at(bestSunPosition));

		//verifica se houve mudança na solução incumbente
		if (bestSunValue > bestFitness) {

			//caso positivo, zera contador de estagnação...
			stagCounter = 0;

			//... atualiza posição da melhor solução encontrada
			bestSolutionPosition = solutionPath.size() - 1;

			//... e atualiza valor de bestFitness
			bestFitness = bestSunValue;

		}

		//caso negativo, adiciona unidade no contador de estagnção...
		else {

			stagCounter++;

			//... e caso critério de estagnação máxima seja atingido
			if (stagCounter > maxStagnation) {

				//se há maxStagnation gerações não houve melhoria, então avalio o pior filho da prole e o escolho como pai
				cout << "Parada por estagnação!" << endl;

				//variáveis para armazenar posição e valor do pior filho
				unsigned worstSunPosition;
				double worstSunValue = bestSunValue;

				//escolhe o melhor filho da prole para ser próximo pai
				for (unsigned sun = 0; sun < offspringVector.size(); sun++) {

					if (offspringVector.at(sun).getFitnessValue() < worstSunValue) {
						worstSunPosition = sun;
						worstSunValue = offspringVector.at(sun).getFitnessValue();
					}

				}//fim do loop que percorre todos os filhos no vetor offspringVector para selecionar o pior deles

				//removo pai recém-escolhido e armazenado na última posição do vetor solutionPath
				solutionPath.pop_back();
				
				//adiciono pior filho na última posição do vetor solutionPath, para ser par da próxima geração
				if (offspringVector.size()>0) solutionPath.push_back(offspringVector.at(worstSunPosition));

				//zera contador de gerações sem melhoria
				stagCounter = 0;

			}//fim da condicional de atingimento do critério de estagnação


		}//fim da condicional para casos em que não há melhoria na solução incumbente


		//rotina para armazenar resultados - uso na tabela 2
		Geracoes g;
		g.geracao = gen + 1;
		g.filhos = offspringVector;

		evolucaoHeuristica.push_back(g);

	}//fim do loop que cria stopCriterium/offspring gerações

	//geração dos arquivos de saída

	//nome do arquivo com tabela 2 - resultado detalhado da ES
	ostringstream outputFile;
	outputFile << name << " - Tabela 2.csv";
	

	//cria arquivo de resultados
	ofstream tabela2;
	tabela2.open(outputFile.str());

	//cria cabeçalho da tabela 2
	tabela2 << "Geracao;filho;bin;objeto;peso;categoria\n";

	//salva saídas da tabela 2
	//para cada geração
	for (unsigned g = 0; g < evolucaoHeuristica.size(); g++) {
		
		//para cada filho da geração
		for (unsigned s = 0; s < evolucaoHeuristica.at(g).filhos.size(); s++) {

			//para cada bin do filho
			for (unsigned b = 0; b < evolucaoHeuristica.at(g).filhos.at(s).getNBins(); b++) {

				//para cada objeto no bin
				for (unsigned j = 0; j < evolucaoHeuristica.at(g).filhos.at(s).binsVector.at(b).elements.size(); j++) {

					tabela2 << g + 1 << ";" << s + 1 << ";" << b + 1 << ";"
						<< evolucaoHeuristica.at(g).filhos.at(s).binsVector.at(b).elements.at(j).index << ";"
						<< evolucaoHeuristica.at(g).filhos.at(s).binsVector.at(b).elements.at(j).weight << ";"
						<< evolucaoHeuristica.at(g).filhos.at(s).binsVector.at(b).elements.at(j).category << "\n";

				}//percorre tds objetos j

			}//de todos os bins b

		}//de cada filho s

	}//de toda geracao g

	tabela2.close();

}


void EA::generateSun(vector<Solution> &offspring, CRandomMersenne &ran) {

	//cria vetor de bins da solução filho
	Solution sun(solutionPath.back().getInstName(), solutionPath.back().binsVector);

	//número máximo de tentativas de gerar filho viável 
	unsigned maxTrials = 2 * nItems / 3;
	
	//inicializa variável auxiliar que vale 1 caso filho tenha sido criado com sucesso e 0 caso contrário
	int success = 0;

	//inicializa contador de tentativas de construir filho viável sorteando um item para troca de bin
	int trials = 0;

	//sorteia aleatoriamente um bin 
	int random_b = ran.IRandom(0, solutionPath.back().getNBins() - 1);

	do {

		//atualiza contador de tentativas
		trials += 1;

		//sorteia aleatoriamente um objeto no bin escolhido (OBS: elementos em bins com menos elementos terão maior probabilidade de serem trocados de posição)
		int random_j = ran.IRandom(0, solutionPath.back().binsVector.at(random_b).elements.size()-1);

		//percorre o vetor de um bins procurando o primeiro em que o elemento random_j do bin random_b caiba
		for (unsigned b = 0; b < solutionPath.back().getNBins(); b++) {
			
			//se b for diferente do bin de origem do objeto a inserir
			if ((int) b != random_b) {

				//se o elemento na posição random_j do random_bin cabe no bin b:
				if (solutionPath.back().binsVector.at(b).binFit(solutionPath.back().binsVector.at(random_b).elements.at(random_j)) == 1) {

					//adiciona o elemento ao bin encontrado
					sun.binsVector.at(b).addElement(solutionPath.back().binsVector.at(random_b).elements.at(random_j));

					//remove o elemento do bin original
					sun.binsVector.at(random_b).removeItemByPosition(random_j);

					//se o bin original ficar vazio, o remove do vetor de bins
					if (sun.binsVector.at(random_b).elements.size() < 1) {

						sun.binsVector.at(random_b) = sun.binsVector.back();
						sun.binsVector.pop_back();

					}

					//atualiza os atributos da solução filho
					sun.updateSolution();

					//recalcula o fitnessValue da solução filho
					sun.calculateFitness();

					//imprime na tela (conferência)
					cout << "Inserção do objeto " << solutionPath.back().binsVector.at(random_b).elements.at(random_j).index
						<< " do bin " << random_b+1 << " no bin " << b+1 << endl<<endl;

					//altera valor da variável success e interrompe busca por bin em que o elemento sorteado caiba
					success = 1;
					break;

				}//fim da condicional utilizada quando o elemento cabe no bin b

			}//fim da condicional que só avalia inserção em bin diferente do original (random_b)

		}//fim do loop que procura bin onde o elemento sorteado caiba, percorrendo todo o vetor de bins

		//se já tiver feito (sem sucesso) muitas tentativas de inserção (nItem/3), muda o bin sorteado
		if(success == 0 && trials == nItems/3) random_b = ran.IRandom(0, solutionPath.back().getNBins()-1);

	} while (success == 0 && trials<maxTrials);


	//se houve sucesso na criação de solução filho
	if (success == 1) {

		//atualiza vetor de soluções
		offspring.push_back(sun);
		
		//imprime na tela que filho foi criado com sucesso
		cout << "\nSolucao filho numero " << offspring.size() << " encontrada com sucesso! Fitness alterado de "
			<< solutionPath.back().getFitnessValue() << " para "
			<< offspring.back().getFitnessValue() << endl;
		
		//imprime solução filho
		/*cout << "Solução filho:"<< endl;
		printMatrix(offspring.back().solution);*/

	}

	//altera número de bins da solução caso tenha havido redução (não deveria mais precisar dessa linha, pq já está sendo feito isso em updateSolution()
	if (sun.binsVector.size() < sun.getNBins()) sun.resetNumberBins(sun.binsVector.size());


}


void EA::printEvolution() {

	cout << "\nEvolucao da fitness funcion: ";

	for (unsigned gen = 0; gen < solutionPath.size(); gen++) {

		cout << solutionPath.at(gen).getFitnessValue() << " - ";

	}

	cout << endl;

}