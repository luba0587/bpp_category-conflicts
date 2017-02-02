*******************************************
* PTR 5001 - Trabalho Final - LuÌsa Cavalcanti
*
* Problema de bin packing com bins homogÍneos e incompatibilidades por categorias
* 	Inst‚ncias do problema de roteirizaÁ„o
*
*
*******************************************/

#include "HeaderBPP.h"



/*************************************ESTRUTURAS DE DADOS GLOBAIS*************************************************/

//matriz de compatibilidades, m por m, em que 0 indica incompatibilidade entre a categoria da linha e da coluna em quest„o
vector< vector<int> > compatibilityMatrix;

//vetor de categorias do problema
vector<Categories> allCategories;

//vari·vel que guarda o fator de multiplicaÁ„o da capacidade
double INST_NEW;


/*************************************FUN«’ES sem CLASSE ***********************************************************/
//funÁ„o que lÍ os dados de compatibilidade problema, armazena na compatibilityMatrix e no vetor allCategories
int loadCMatrix(string inputFileName) {

	//vari·vel para armazenar o n˙mero de categorias do problema (retorna valor zero caso a leitura n„o seja realizada com sucesso)
	int nCategories = 0;

	//abre o arquivo de entrada da inst‚ncia
	ifstream dataFile;
	dataFile.open(inputFileName);

	//se der erro na abertura do arquivo, imprime aviso
	if (dataFile.bad()) {
		cerr << "Nao foi possivel abrir arquivo de entrada!\n";
	}

	//se abrir com sucesso, lÍ e armazena os dados do arquivo
	else {

		cout << "\nArquivo com compatibilidades aberto com sucesso!" << endl;

		//vari·vel inteira para armazenar os dados lidos
		int input;

		//string line para armazenar linhas in˙teis
		string line;

		//ignora primeira linhas
		getline(dataFile, line);

		//usa segunda linha, que tem o cabeÁalho das colunas, para verificar quantas categorias existem
		getline(dataFile, line);
		istringstream iss(line);
		while (iss >> nCategories) {}
		//cout << "Ha " << nCategories << "categorias no problema" << endl;

		//lÍ a matriz de compatibilidade linha a linha
		for (int lines = 1; lines <= nCategories; lines++) {

			//cria vetor tempor·rio para guardar valores da linha
			vector<int> lineInput;

			//cria vari·vel para guardar o grau de compatibilidade da categoria
			int degree = 0;

			//ignora primeira coluna, que tem apenas Ìndice da categoria
			dataFile >> input;

			//...loop que lÍ os dados coluna a coluna
			for (int columns = 1; columns <= nCategories; columns++) {

				//lÍ valor
				dataFile >> input;

				//soma no grau de compatibilidade da categoria
				degree += input;

				//armazena no vetor lineInput
				lineInput.push_back(input);

			}//fim do loop que lÍ e grava dados da linha

			 //guarda linha lida na matriz de compatibilidades
			compatibilityMatrix.push_back(lineInput);

			//cria categoria
			Categories k(lines - 1, degree);

			//armazena na estrutura de dados
			allCategories.push_back(k);

		}//fim do loop que lÍ e armazena a matriz de compatibilidade entre as nCategories categorias

	}//fim do procedimento realizado caso o arquivo de entrada tenha sido aberto com sucesso

	dataFile.close();

	return nCategories;

}

//funcaoo que le os dados de LBbest problema e armazena em vetor vetor allBestLBs
void loadBenchmarks(string inputFileName, vector<int> &allBestLBs, unsigned iMax) {

	ifstream dataFile;
	dataFile.open(inputFileName);

	////ignora primeira linhas
	string line;
	getline(dataFile, line);

	//vari·vel para armazenar valores lidos
	int value;

	for (unsigned i = 0; i < iMax; i++) {

		for (unsigned f = 1; f <= 4; f++) {

			dataFile >> value;

			if (INST_NEW>1){
			
				if (INST_NEW < 2) {
					
					if(INST_NEW == 1.5 && f==3) allBestLBs.push_back(value);

					else if(f==2) allBestLBs.push_back(value);
				
				}

				else if(f==4) allBestLBs.push_back(value);
			
			}

			else if(f==1) allBestLBs.push_back(value);

		}

	}

	dataFile.close();


}



/*************************************MAIN **********************************************************************/
int main() {

	//vetor com todas as inst‚ncias que devem ser executadas
	vector<string> allInstances;

	//vetor que guarda o melhor LB da inst‚ncia, lido em arquivo de entrada
	vector<int> allBestLBs;

#ifdef M_GRANDE
	//vetor que guarda os resutados Ûtimos das inst‚ncias, calculados pelo Gurobi, em n˙mero de bins
	vector<Solution> allLowerBounds;
#endif // M_GRANDE

	
	//vetor que guarda os resultados da aplicaÁ„o da heurÌstica nas inst‚ncias
	vector<EA> allResults;

	//abre o arquivo de entrada da inst‚ncia
	ifstream dataFile;
	
#ifdef INST_AUGMENTED
	dataFile.open("ArquivosIntancias-saidas.txt");
#else
	dataFile.open("nomesInst.txt");
#endif // INST_AUGMENTED
	
	string line;

	while (getline(dataFile, line)) {
		allInstances.push_back(line);
	}

	//carrega matriz de compatibilidade e salva qtde. de categorias na vari·vel nCategories
	int m = loadCMatrix("Reduced_set.vrp");
	
	//executa tudo repetidas vezes para avaliar robustez
	unsigned executions=1;
	for(unsigned test=0; test<executions;test++){

		//se houver sucesso na leitura da matriz de compatibilidade, carrega dados dos objetos, j· guardando nas estruturas de dados apropriadas
		if (m > 0) {

			cout << "instancias: " << allInstances.size() << endl;

			//vetor com fatores de capacidade a executar
			vector<double> capacityFactor;
			capacityFactor.push_back(2.0);
			capacityFactor.push_back(1.5);
			capacityFactor.push_back(1.2);
			capacityFactor.push_back(1.0);

			//para cada fator de capacidade definido, executa todas as inst‚ncias e gera arquivo de saÌda
			for (unsigned exec = 0; exec < capacityFactor.size(); exec++) {

				//define fator de capacidade desta execuÁ„o
				INST_NEW = capacityFactor.at(exec);

				//carrega vetor de LBbest das inst‚ncias, a partir de arquivo de entrada, considerando o fator de capacidade adotado
				loadBenchmarks("LBbest.txt", allBestLBs, allInstances.size());

				for (unsigned i = 0; i < allInstances.size(); i++) {

					//cria inst‚ncia usando o nome do arquivo de entrada e a qtde de categorias do problema
					Instances inst(allInstances.at(i), m);

					//*****encontra LB do problema, pela otimizaÁ„o, usando o Gurobi
	#ifdef M_GRANDE
					Timer t;
					Solution s(inst.getName(), inst.findOptimum());
					s.setTime(t.elapsed());
					allLowerBounds.push_back(s);
	#endif

					//*****infere valor de LBbest, desde que o arquivo de entrada esteja com tds as inst‚ncias
					if (allInstances.size() == allBestLBs.size()) {
						cout << "\nUsando LBbest\n\n";
						inst.setBestLB(allBestLBs.at(i));
					}

					//****roda heurÌstica

					//gera objeto da classe EA que contÈm a trajetÛria da aplicaÁ„o da meta-heurÌstica
					EA evolutionaryAlgorithm(inst);

					//guarda o objeto no vetor de resultados
					allResults.push_back(evolutionaryAlgorithm);

				}//fim do loop que percorre todas as int‚ncias do vetor allInstances

				//nome do arquivo com tabela 1 - resumo dos resultados
				ostringstream outputFile;
				outputFile << "output"<< test <<"-" << INST_NEW << "xCap-Resumo.csv";

				//cria arquivo de resultados
				ofstream tabela1;
				tabela1.open(outputFile.str());

				//cria cabeÁalho da tabela 1
				tabela1 << "Fator Capacidade;Instancia;Capacidade Bin;Numero Itens;LBbest;s0- #Bins;s0- fitness;s*- #Bins;s*- fitness;Tempo total(s);";
	#ifdef M_GRANDE
				tabela1 << "Gurobi - #Bins; Gurobi - Solution; TempoGurobi(s)\n";
	#else
				tabela1 << "\n";
	#endif // M_GRANDE
			
				//salva saÌdas da tabela 1
				for (unsigned i = 0; i < allResults.size(); i++) {

					tabela1 << INST_NEW << ";"
						<< allResults.at(i).getInstanceName() << ";"
						<< allResults.at(i).getBinCap() << ";"
						<< allResults.at(i).getProblemSize() << ";"
						<< allResults.at(i).getLB1() << ";"
						<< allResults.at(i).getSolutionPath().at(0).getNBins() << ";"
						<< allResults.at(i).getSolutionPath().at(0).getFitnessValue() << ";"
						<< allResults.at(i).getSolutionPath().at(allResults.at(i).getFinalSPosition()).getNBins() << ";"
						<< allResults.at(i).getFinalFitness() << ";"
						<< allResults.at(i).getSolutionPath().back().getTime() << ";";
	#ifdef M_GRANDE
					tabela1 << allLowerBounds.at(i).getNBins() << ";";
					for (int j = 0; j < allLowerBounds.at(i).getCode().size(); j++) {
						tabela1 << allLowerBounds.at(i).getCode().at(j) << ", ";
					}
					tabela1 << ";" << allLowerBounds.at(i).getTime();
	#endif // M_GRANDE
					tabela1 << "\n";

				}

				tabela1.close();

				//limpa o vetor de resultados antes de partir para o prÛximo fator de capacidades
				allResults.clear();

			}//FIM DA CONDICIONAL QUE PERCORRE O VETOR DE FATORES DE CAPACIDADE

		}//fim da condicional que soh roda as instancias se a matriz de compatibilidade tiver sido carregada com sucesso

	}//fim das execucoes

	system("pause");

	return 0;

}
