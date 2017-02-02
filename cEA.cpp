#include "HeaderBPP.h"
#include "randomc.h"


//mÈtodo de ordenaÁ„o de categorias n„o-decrescentemente pelo grau de compatibilidade
bool increasingCompDegree(Categories lhs, Categories rhs) {
	return lhs.compDegree < rhs.compDegree;
}

//construtor da classe EA: cria inst‚ncia usando soluÁ„o inicial, executa heurÌstica
EA::EA(Solution &s0)
	: bestFitness(s0.getFitnessValue())
{
	solutionPath.push_back(s0);
	applyEA();
}

//construtor que usa a inst‚ncia - primeiro acha soluÁ„o inicial pela construtiva depois executa a heurÌstica
EA::EA(Instances &inst)
	:nItems(inst.getN()), binCapacity(inst.getBinCapacity()), name(inst.getName()), bestSolutionPosition(0), relaxedLB(inst.getRelaxedLB())
{
	if (inst.getBestLB() > 0) relaxedLB = inst.getBestLB();
	name.resize(name.size() -4);
	generate_s0(inst);
	bestFitness = solutionPath.front().getFitnessValue();
	if (solutionPath.front().getNBins() > relaxedLB) {
		applyEA();
		printEvolution();
	}
	else cout << "\nSolucao inicial eh otima!\n";
}

//mÈtodo que encontra soluÁ„o inicial aplicando o First Fit modificado (categorias ordenadas) e a insere em SolutionPath
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

	//ordena as categorias crescentemente pelo grau de compatibilidade (pra comeÁar por menos compatÌveis)
	vector<Categories> sortedCategories = allCategories;
	sort(sortedCategories.begin(), sortedCategories.end(), increasingCompDegree);

	//percorre o vetor de categorias ordenadas, selecionando uma a uma
	for (unsigned k = 0; k < sortedCategories.size(); k++) {

		//calcula o Ìndice do ˙ltimo elemento dessa categoria
		int last_j;
		if (sortedCategories.at(k).index + 1 < (int)sortedCategories.size()) last_j = inst.getCategoryPosition().at(sortedCategories.at(k).index + 1) - 1;
		else last_j = inst.getN() - 1;

		//percorre uma a uma as tarefas da categoria em quest„o
		for (int j = inst.getCategoryPosition().at(sortedCategories.at(k).index); j <= last_j; j++) {

			//abre primeiro bin, se n„o houver nenhum aberto
			if (resultingBins.size()<1) {
				Bins b(inst, notPickedElements.at(j));
				resultingBins.push_back(b);
			}

			//caso contr·rio, tenta alocar no primeiro bin disponÌvel
			else {

				//inicializa vari·vel que indica se foi possÌvel alocar o elemento em um bin j· aberto
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

				//se percorreu o vetor de bins e n„o conseguiu encaixar o elemento, cria um novo bin
				if (alocated <1) {
					Bins b(inst, notPickedElements.at(j));
					resultingBins.push_back(b);
				}

			}//fim do loop que tenta alocar o elemento nos bins j· abertos e, se n„o consegue, cria um novo bin

		}//fim do loop que aloca todos os elementos da k-Èsima categoria ordenada

	}//fim do loop que percorre todas as categorias do problema

	//cria objeto da classe soluÁ„o usando o vetor de bins construÌdo pela heurÌstica
	Solution s(inst.getName(), resultingBins);

	//adiciona soluÁ„o ao vetor solutionPath da classe EA
	solutionPath.push_back(s);

}


//estrutura que guarda o n˙mero da geraÁ„o e o conjunto de soluÁıes filho
struct Geracoes {

	unsigned geracao;
	vector<Solution> filhos;

};


//aplica meta-heurÌstica (1-lambda) EstratÈgia Evolucion·ria
void EA::applyEA() {

	//critÈrio de parada: n˙mero m·ximo de soluÁıes avaliadas
	unsigned genMax = 60;

	//n˙mero de filhos de cada geraÁ„o
	unsigned offspring = 20;

	//critÈrio de estagnaÁ„o (n˙mero m·ximo de geraÁıes sem melhoria na soluÁ„o incumbente)
	unsigned maxStagnation = 10;
	
	//contador de geraÁıes sem melhoria
	unsigned stagCounter = 0;

	//cria objeto para medir tempo de processamento
	Timer t;

	//cria objeto da classe de gerarador de n˙meros aleatÛrios
	int seed = GetTickCount();
	CRandomMersenne ran(seed);

	//cria estrutura que guarda todos os resultados para gerar tabela 2
	vector<Geracoes> evolucaoHeuristica;

	//em cada geraÁ„o de filhos criada:
	for (unsigned gen = 0; gen < genMax; gen++) {

		//estrutura que guarda todas as soluÁıes filho de uma geraÁ„o
		vector<Solution> offspringVector;

		//vari·veis que guardam a posiÁ„o do melhor filho no vetor offspringVector e seu valor na funÁ„o de fitness
		unsigned bestSunPosition=0;
		double bestSunValue=0;

		//gera filhos a partir da soluÁ„o pai, que È a ˙ltima soluÁ„o do vetor solutionPath
		for (unsigned sun = 0; sun < offspring; sun++) {

			//cria filho a partir de mutaÁ„o do pai e o adiciona no vetor offspring (se n„o houver sucesso em maxTrials tentativas, simplesmente n„o cria filho!)
			generateSun(offspringVector, ran, t);

		}//fim do loop que gera 'offspring' filhos a partir de mutaÁıes da soluÁ„o pai

		//escolhe o melhor filho da prole para ser o prÛximo pai
		for (unsigned sun = 0; sun < offspringVector.size(); sun++) {

			if (offspringVector.at(sun).getFitnessValue() > bestSunValue) {
				bestSunPosition = sun;
				bestSunValue = offspringVector.at(sun).getFitnessValue();
			}

		}//fim do loop que percorre todos os filhos no vetor offspringVector para selecionar o melhor deles

		//insere o melhor filho da prole na ˙ltima posiÁ„o do vetor solutionPath
		if (offspringVector.size()>0) solutionPath.push_back(offspringVector.at(bestSunPosition));

		//guarda o tempo total para encontrar essa soluÁ„o (tempo para gerar tods os filhos da prole e os avaliar)
		float elapsedTime = t.elapsed();
		solutionPath.back().setTime(elapsedTime);

		//verifica se houve mudanÁa na soluÁ„o incumbente. Caso positivo:
		if (bestSunValue > bestFitness) {

			//caso positivo, zera contador de estagnaÁ„o...
			stagCounter = 0;

			//... atualiza posiÁ„o da melhor soluÁ„o encontrada
			bestSolutionPosition = solutionPath.size() - 1;

			//... e atualiza valor de bestFitness
			bestFitness = bestSunValue;

			//... e verifica se critÈrio de parada por LB, parando a heurÌstica caso positivo
			if (solutionPath.back().getNBins() == relaxedLB) break;

		}

		//caso negativo, adiciona unidade no contador de estagnaÁ„o...
		else {

			stagCounter++;

			//... e caso critÈrio de estagnaÁ„o m·xima seja atingido e esta n„o seja a ˙ltima geraÁ„o da heurÌstica
			if (stagCounter > maxStagnation && gen + 1 < genMax) {

				//cout << "Parada por estagnaÁ„o!" << endl;

				//se h· maxStagnation geraÁıes n„o houve melhoria, ent„o avalio o pior filho da prole e o escolho como pai
				
				//vari·veis para armazenar posiÁ„o e valor do pior filho
				unsigned worstSunPosition=0;
				double worstSunValue = bestSunValue;

				//escolhe o pior filho da prole para ser prÛximo pai
				for (unsigned sun = 0; sun < offspringVector.size(); sun++) {

					if (offspringVector.at(sun).getFitnessValue() < worstSunValue) {
						worstSunPosition = sun;
						worstSunValue = offspringVector.at(sun).getFitnessValue();
					}

				}//fim do loop que percorre todos os filhos no vetor offspringVector para selecionar o pior deles

				//removo pai recÈm-escolhido e armazenado na ˙ltima posiÁ„o do vetor solutionPath (gravo antes seu tempo de processamento para usar na pior soluÁ„o tb)
				float time = solutionPath.back().getTime(); 
				solutionPath.pop_back();
				
				//adiciono pior filho na ˙ltima posiÁ„o do vetor solutionPath, para ser pai da prÛxima geraÁ„o
				if (offspringVector.size() > 0) {
					solutionPath.push_back(offspringVector.at(worstSunPosition));
					solutionPath.back().setTime(time);
				}

				//zera contador de geraÁıes sem melhoria
				stagCounter = 0;

			}//fim da condicional de atingimento do critÈrio de estagnaÁ„o

		}//fim da condicional para casos em que n„o h· melhoria na soluÁ„o incumbente

		//rotina para armazenar resultados - uso na tabela 2
		Geracoes g;
		g.geracao = gen + 1;
		g.filhos = offspringVector;

		evolucaoHeuristica.push_back(g);

	}//fim do loop que cria stopCriterium/offspring geraÁıes

	
	 
	 //***********geraÁ„o dos arquivos de saÌda

	/*
	//nome do arquivo com tabela 2 - resultado detalhado da ES
	ostringstream outputFile;
	outputFile << name << "-TabelaSolucoes.csv";
	
	//cria arquivo de resultados
	ofstream tabela2;
	tabela2.open(outputFile.str());

	//cria cabeÁalho da tabela 2
	tabela2 << "Geracao;filho;bin;objeto;peso;categoria\n";

	//salva saÌdas da tabela 2
	//para cada geraÁ„o
	for (unsigned g = 0; g < evolucaoHeuristica.size(); g++) {
		
		//para cada filho da geraÁ„o
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

	//nome do arquivo com tabela 3 - evoluÁ„o da ES
	ostringstream outputFile2;
	outputFile2 << name << "-" << INST_NEW << "cap-evolucao.csv";

	//cria arquivo de resultados
	ofstream tabela3;
	tabela3.open(outputFile2.str());

	//cria cabeÁalho da tabela 3
	tabela3 << "IteraÁ„o;GeraÁ„o;Tempo decorrido(s);Valor do filho\n";
	int it = 1;

	//percorre todas as geraÁıes
	for (unsigned g = 0; g < evolucaoHeuristica.size(); g++) {

		//percorre o vetor de filhos da geraÁ„o
		for (unsigned j = 0; j < evolucaoHeuristica.at(g).filhos.size(); j++) {

			tabela3 << it << ";"
				<< g << ";"
				<< evolucaoHeuristica.at(g).filhos.at(j).getTime() << ";"
				<< evolucaoHeuristica.at(g).filhos.at(j).getFitnessValue() << "\n";
	
			it++;

		}

	}
	tabela3.close();

	//nome do arquivo com tabela 4 - solution Path
	ostringstream outputFile3;
	outputFile3 << name <<"-" << INST_NEW << "cap-solutionPath.csv";

	//cria arquivo de resultados
	ofstream tabela4;
	tabela4.open(outputFile3.str());

	//cria cabeÁalho da tabela 2
	tabela4 << "Tempo;Valor da SoluÁ„o\n";

	for (unsigned sol = 0; sol < solutionPath.size(); sol++) {

		tabela4 << solutionPath.at(sol).getTime() << ";"
			<< solutionPath.at(sol).getFitnessValue() << "\n";

	}

	tabela4.close();  //fIM DA GERA«√O DE ARQUIVOS DE SAÕDA

	*/	
	//fIM DA GERA«√O DE ARQUIVOS DE SAÕDA

}


void EA::generateSun(vector<Solution> &offspring, CRandomMersenne &ran, Timer t) {

	//cria vetor de bins da soluÁ„o filho
	Solution sun(solutionPath.back().getInstName(), solutionPath.back().binsVector);

	//n˙mero m·ximo de tentativas de gerar filho vi·vel 
	unsigned maxTrials = 2 * nItems / 3;
	
	//inicializa vari·vel auxiliar que vale 1 caso filho tenha sido criado com sucesso e 0 caso contr·rio
	int success = 0;

	//inicializa contador de tentativas de construir filho vi·vel sorteando um item para troca de bin
	int trials = 0;

	//sorteia aleatoriamente um bin 
	int random_b = ran.IRandom(0, solutionPath.back().getNBins() - 1);

	do {

		//atualiza contador de tentativas
		trials += 1;

		//sorteia aleatoriamente um objeto no bin escolhido (OBS: elementos em bins com menos elementos ter„o maior probabilidade de serem trocados de posiÁ„o)
		int random_j = ran.IRandom(0, solutionPath.back().binsVector.at(random_b).elements.size()-1);

		//percorre o vetor de um bins procurando o primeiro em que o elemento random_j do bin random_b caiba
		for (unsigned b = 0; b < solutionPath.back().getNBins(); b++) {
			
			//se b for diferente do bin de origem do objeto a inserir
			if ((int) b != random_b) {

				//se o elemento na posiÁ„o random_j do random_bin cabe no bin b:
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

					//atualiza os atributos da soluÁ„o filho
					sun.updateSolution();

					//recalcula o fitnessValue da soluÁ„o filho
					sun.calculateFitness();

					//calcula o tempo de processamento atÈ a criaÁ„o desse filho
					float elapsedTime = t.elapsed();
					sun.setTime(elapsedTime);

					//imprime na tela (conferÍncia)
					/*cout << "InserÁ„o do objeto " << solutionPath.back().binsVector.at(random_b).elements.at(random_j).index
						<< " do bin " << random_b+1 << " no bin " << b+1 << endl<<endl;*/

					//altera valor da vari·vel success e interrompe busca por bin em que o elemento sorteado caiba
					success = 1;
					break;

				}//fim da condicional utilizada quando o elemento cabe no bin b

			}//fim da condicional que sÛ avalia inserÁ„o em bin diferente do original (random_b)

		}//fim do loop que procura bin onde o elemento sorteado caiba, percorrendo todo o vetor de bins

		//se j· tiver feito (sem sucesso) muitas tentativas de inserÁ„o (nItem/3), muda o bin sorteado
		if(success == 0 && trials == nItems/3) random_b = ran.IRandom(0, solutionPath.back().getNBins()-1);

	} while (success == 0 && trials<maxTrials);


	//se houve sucesso na criaÁ„o de soluÁ„o filho
	if (success == 1) {

		//atualiza vetor de soluÁıes
		offspring.push_back(sun);
	
	}

	//se n„o tiver havido sucesso na criaÁ„o do filho, copia o pai
	else {
		offspring.push_back(solutionPath.back());
		offspring.back().setTime(t.elapsed());
	}

	//altera n˙mero de bins da soluÁ„o caso tenha havido reduÁ„o (n„o deveria mais precisar dessa linha, pq j· est· sendo feito isso em updateSolution()
	if (sun.binsVector.size() < sun.getNBins()) sun.resetNumberBins(sun.binsVector.size());


}


void EA::printEvolution() {

	cout << "\nEvolucao da fitness funcion: ";

	for (unsigned gen = 0; gen < solutionPath.size(); gen++) {

		cout << solutionPath.at(gen).getFitnessValue() << " - ";

	}

	cout << endl;

}
