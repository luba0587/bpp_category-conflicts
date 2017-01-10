/*************************************CLASSE INSTANCIAS E SEUS MÉTODOS ****************************************/

#include "HeaderBPP.h"



//construtor que cria instância usando o nome do arquivo de entrada
Instances::Instances(string str, int m)
	: inputFile(str), mCategories(m), nElements(0)
{
	load();
}



/************************************ FUNÇÕES 'GLOBAIS' ****************************************/

//função que imprime uma matriz na tela
void printMatrix(vector< vector<int> > matrix) {

	for (unsigned i = 0; i< matrix.size(); i++) {

		cout << endl << i + 1 << " ";

		for (unsigned j = 0; j<matrix.at(i).size(); j++) {

			cout << matrix.at(i).at(j) << " ";

		}

	}

}


//método que lê os dados referentes aos objetos - primeiro lê matriz completa, depois armazena nos vetores allElements e firstObjectPosition
void Instances::load() {


	//abre o arquivo de entrada da instância
	ifstream dataFile;
	dataFile.open(inputFile);

	//se der erro na abertura do arquivo, imprime aviso
	if (dataFile.bad()) {
		cerr << "Nao foi possivel abrir arquivo de entrada!\n";
	}

	//se abrir com sucesso, lê e armazena os dados do arquivo
	else {

		cout << "\nArquivo com dados da instancia aberto com sucesso!" << endl;

		//*************** LEITURA DOS DADOS E ARMAZENAGEM EM MATRIZ ***************//

		//matriz com dados de entrada
		vector< vector<int> > matrixData;

		//variável inteira para armazenar os dados lidos
		int input;

		//string line para armazenar linhas lidas
		string line;

		//lê e ignora todas as linhas até encontrar a linha EDGE_WEIGHT_TYPE
		do {
			getline(dataFile, line);
		} while (line.find("EDGE_WEIGHT_TYPE") > 2);

		//lê a linha da capacidade até o caractere ":"
		getline(dataFile, line, ':');

		//lê a capacidade do bin e armazena na variável adequada
		dataFile >> binCapacity;

		//imprime capacidade na tela, apenas para conferência
		cout << "bin capacity is " << binCapacity << endl;

		//ignora linhas até encontrar a seção com demanda "DEMAND_SECTION"
		do {
			getline(dataFile, line);
		} while (line.find("DEMAND") > 1);

		//ignora cabeçalho da matriz de demanda
		do {
			getline(dataFile, line);
		} while (line.length() == 0);

		//enquanto existir linha de entrada
		while (dataFile >> input) {

			vector<int> lineData;

			//...loop que lê os dados coluna a coluna
			for (int columns = 1; columns <= mCategories; columns++) {

				//lê valor
				dataFile >> input;

				//armazena na linha
				lineData.push_back(input);


			}

			//...armazena os dados da linha na matrix de dados de entrada
			matrixData.push_back(lineData);


		}

		//printMatrix(matrixData);



		//*************** TRANSFERÊNCIA PARA ESTRUTURA DE DADOS ESCOLHIDA ***************//

		//varre coluna-a-coluna a matrixData
		for (int column = 1; column <= mCategories; column++) {

			//guarda a posição do primeiro elemento da categoria
			firstObjectPosition.push_back(allElements.size());

			for (unsigned line = 0; line < matrixData.size(); line++) {

				//se houver elemento (peso >0), grava peso no vetor allElements e loja (linha) no vetor storeOfElement
				if (matrixData.at(line).at(column - 1)>0) {
					allElements.push_back(matrixData.at(line).at(column - 1));
					storeOfElement.push_back(line);
				}

			}//fim do loop que lê uma coluna inteira de matrixData

		}//fim do loop que lê todas as colunas da matrixData

		 //guarda número de elementos da instância
		nElements = (int)allElements.size();

	}//fim do procedimento realizado caso o arquivo de entrada tenha sido aberto com sucesso

	dataFile.close();



	//impressão na tela para conferência
	cout << "Elementos = {";
	for (unsigned j = 0; j<allElements.size(); j++) {
		cout << allElements.at(j) << " - ";
	} cout << "}\n";

	cout << "Posição de cada categoria = {";
	for (unsigned j = 0; j< firstObjectPosition.size(); j++) {
		cout << firstObjectPosition.at(j) << " - ";
	} cout << "}\n";


}


#ifdef M_GRANDE
//método que monta o problema no Gurobi e o resolve
void Instances::findLB() {

	vector<int> codifiedSolution;

	//otimização
	try {

		//INICIALIZAÇÃO DO GUROBI E CRIAÇÃO DO MODELO DE OTIMIZAÇÃO
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);

		//DEFINIÇÃO DE VARIÁVEIS

		//Binaria y(i) indicando se bin i é usado
		GRBVar* y = 0;
		y = model.addVars(nElements, GRB_BINARY);
		model.update();
		for (int i = 0; i < nElements; i++) {
			ostringstream name;
			name << "y(" << i << ")";
			y[i].set(GRB_StringAttr_VarName, name.str());
		}

		//Binaria x(i,j) que indica se objeto j está alocado no bin i
		GRBVar** x = 0;
		x = new GRBVar*[nElements];
		for (int i = 0; i < nElements; i++) {
			x[i] = model.addVars(nElements, GRB_BINARY);
			model.update();
			for (int j = 0; j < nElements; j++) {
				ostringstream name;
				name << "x(" << i << "." << j << ")";
				x[i][j].set(GRB_StringAttr_VarName, name.str());
			}
		}

		//Binaria f(i,k) que indica se existe objeto da categoria k alocado ao bin i
		GRBVar** f = 0;
		f = new GRBVar*[nElements];
		for (int i = 0; i < nElements; i++) {
			f[i] = model.addVars(mCategories, GRB_BINARY);
			model.update();
			for (int k = 0; k < mCategories; k++) {
				ostringstream name;
				name << "f(" << i << "." << k << ")";
				f[i][k].set(GRB_StringAttr_VarName, name.str());
			}
		}



		//DEFINIÇÃO FUNÇÃO OBJETIVO

		//expressão que soma y_i
		GRBLinExpr totalBins = 0;
		for (int i = 0; i < nElements; i++) {
			totalBins += y[i];
		}

		model.setObjective(totalBins, GRB_MINIMIZE);


		//RESTRIÇÕES DO MODELO

		//respeita capacidade dos bins e vincula variáveis x(i,j) e y(i)
		for (int i = 0; i < nElements; i++) {
			GRBLinExpr weightOnBin = 0;
			for (int j = 0; j < nElements; j++) {
				weightOnBin += x[i][j] * allElements[j];
			}
			ostringstream cname;
			cname << "CapacityOfBin(" << i << ")";
			model.addConstr(y[i] * binCapacity >= weightOnBin, cname.str());
		}

		//aloca todo objeto j a um bin
		for (int j = 0; j < nElements; j++) {
			GRBLinExpr sum_xij_in_i = 0;
			for (int i = 0; i < nElements; i++) {
				sum_xij_in_i += x[i][j];
			}
			ostringstream cname;
			cname << "AssignObject(" << j << ")toaBin";
			model.addConstr(sum_xij_in_i == 1, cname.str());
		}

		//define f(i)(k): se o bin i possui algum objeto da categoria k
		for (int i = 0; i < nElements; i++) {

			for (int k = 0; k < mCategories; k++) {

				//calcula o índice do último job da categoria k
				int last_j;
				if (k + 1 < mCategories) last_j = firstObjectPosition[k + 1] - 1;
				else last_j = nElements - 1;

				//soma x[i][j] de todo j pertencente à k
				GRBLinExpr sum_xij_of_k = 0;
				for (int j = firstObjectPosition[k]; j <= last_j; j++) {
					sum_xij_of_k += x[i][j];
				}

				//cria restrição
				ostringstream cname;
				cname << "Determine f(" << i << ")(" << k << ")";
				model.addConstr(f[i][k] * M_GRANDE >= sum_xij_of_k, cname.str());

			}

		}

		//restrição de compatibilidade
		for (int k = 0; k < mCategories; k++) {

			for (int l = k + 1; l < mCategories; l++) {

				//se o valor da matriz de compatibilidade das categorias k e l for 0, há incompatibilidade
				if (compatibilityMatrix[k][l] == 0) {

					//gera uma restrição para cada bin i
					for (int i = 0; i < nElements; i++) {
						ostringstream cname;
						cname << "AssureCategory(" << k << ")isntWithCategory(" << l << "onBin(" << i << ")";
						model.addConstr(f[i][k] + f[i][l] <= 1, cname.str());
					}

				}//gera restrições apenas quando c[k][l]=0

			}

		}

		//restrição para reduzir espaço de soluções
		for (int k = 0; k < nElements; k++) {
			for (int l = k + 1; l < nElements; l++) {
				ostringstream cname;
				cname << "AssureBin(" << k << ")isFilledBeforeBin(" << l << ")";
				model.addConstr(y[k] >= y[l], cname.str());
			}
		}


		//GERA ARQUIVO .lp COM MODELO CONSTRUÍDO
		model.write("modelo.lp");


		//RODA OTIMIZAÇÃO
		model.optimize();


		//IMPRIME RESULTADOS
		int aux = 0;

		//percorre vetor de y(i), enquanto i for bin utilizado
		while (y[aux].get(GRB_DoubleAttr_X)>0) {

			//cout << y[aux].get(GRB_StringAttr_VarName) << " = " << y[aux].get(GRB_DoubleAttr_X);

			//insere separador na solução, igual ao número do bin negativo
			codifiedSolution.push_back(-1 * (aux + 1));

			//percorre todos os x[i][j] do bin i
			for (int j = 0; j < nElements; j++) {

				//se o objeto j estiver no bin i
				if (x[aux][j].get(GRB_DoubleAttr_X)>0) {
					//cout << x[aux][j].get(GRB_StringAttr_VarName) << " = " << allElements[j] << ", ";
					//insere o índice do elemento no vetor solução
					codifiedSolution.push_back(j);
				}

			}
			//cout << endl;

			aux += 1;

		}


	}

	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...) {
		cout << "Exception during optimization" << endl;
	}

	Solution s(inputFile, codifiedSolution);
	printMatrix(s.solution);

}

#endif
