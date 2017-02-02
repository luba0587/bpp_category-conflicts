/*************************************CLASSE INSTANCIAS E SEUS M…TODOS ****************************************/

#include "HeaderBPP.h"



//construtor que cria inst‚ncia usando o nome do arquivo de entrada
Instances::Instances(string str, int m)
	: inputFile(str), mCategories(m), nElements(0), bestLB(0)
{
	load();
	solveRelaxedProblem();
}



/************************************ FUN«’ES 'GLOBAIS' ****************************************/

//funÁ„o que imprime uma matriz na tela
void printMatrix(vector< vector<int> > matrix) {

	for (unsigned i = 0; i< matrix.size(); i++) {

		cout << endl << i + 1 << " ";

		for (unsigned j = 0; j<matrix.at(i).size(); j++) {

			cout << matrix.at(i).at(j) << " ";

		}

	}

}


//mÈtodo que lÍ os dados referentes aos objetos - primeiro lÍ matriz completa, depois armazena nos vetores allElements e firstObjectPosition
void Instances::load() {


	//abre o arquivo de entrada da inst‚ncia
	ifstream dataFile;
	dataFile.open(inputFile);

	//se der erro na abertura do arquivo, imprime aviso
	if (dataFile.bad()) {
		cerr << "Nao foi possivel abrir arquivo de entrada!\n";
	}

	//se abrir com sucesso, lÍ e armazena os dados do arquivo
	else {

		cout << "\nArquivo com dados da instancia aberto com sucesso!" << endl;

		//*************** LEITURA DOS DADOS E ARMAZENAGEM EM MATRIZ ***************//

		//matriz com dados de entrada
		vector< vector<int> > matrixData;

		//vari·vel inteira para armazenar os dados lidos
		int input;

		//string line para armazenar linhas lidas
		string line;

#ifndef INST_AUGMENTED
		//lÍ e ignora todas as linhas atÈ encontrar a linha EDGE_WEIGHT_TYPE
		do {
			getline(dataFile, line);
		} while (line.find("EDGE_WEIGHT_TYPE") > 2);

		//lÍ a linha da capacidade atÈ o caractere ":"
		getline(dataFile, line, ':');

		//lÍ a capacidade do bin e armazena na vari·vel adequada
		dataFile >> binCapacity;

		//imprime capacidade na tela, apenas para conferÍncia
		cout << "bin capacity is " << binCapacity << endl;

		//ignora linhas atÈ encontrar a seÁ„o com demanda "DEMAND_SECTION"
		do {
			getline(dataFile, line);
		} while (line.find("DEMAND") > 1);

		//ignora cabeÁalho da matriz de demanda
		do {
			getline(dataFile, line);
		} while (line.length() == 0);

		//enquanto existir linha de entrada
		while (dataFile >> input) {

			vector<int> lineData;

			//...loop que lÍ os dados coluna a coluna
			for (int columns = 1; columns <= mCategories; columns++) {

				//lÍ valor
				dataFile >> input;

				//armazena na linha
				lineData.push_back(input);


			}

			//...armazena os dados da linha na matrix de dados de entrada
			matrixData.push_back(lineData);


		}
#endif // !INST_AUGMENTED

#ifdef INST_AUGMENTED

		//lÍ a capacidade do bin e armazena na vari·vel adequada
		dataFile >> binCapacity;
		dataFile >> input;
		binCapacity = binCapacity*INST_NEW;

		//imprime capacidade na tela, apenas para conferÍncia
		cout << "bin capacity is " << binCapacity << endl;

		//ignora linhas atÈ encontrar a seÁ„o com demanda "DEMAND_SECTION"
		do {
			getline(dataFile, line);
		} while (line.find("DEMAND") > 1);

		//ignora cabeÁalho da matriz de demanda
		do {
			getline(dataFile, line);
		} while (line.length() == 0);

		//enquanto existir linha de entrada
		while (dataFile >> input) {

			vector<int> lineData;

			//...loop que lÍ os dados coluna a coluna
			for (int columns = 1; columns <= mCategories; columns++) {

				//lÍ valor
				dataFile >> input;

				//armazena na linha
				lineData.push_back(input);


			}

			//...armazena os dados da linha na matrix de dados de entrada
			matrixData.push_back(lineData);


		}

#endif // INST_AUGMENTED



		//printMatrix(matrixData);



		//*************** TRANSFER NCIA PARA ESTRUTURA DE DADOS ESCOLHIDA ***************//

		//varre coluna-a-coluna a matrixData
		for (int column = 1; column <= mCategories; column++) {

			//guarda a posiÁ„o do primeiro elemento da categoria
			firstObjectPosition.push_back(allElements.size());

			for (unsigned line = 0; line < matrixData.size(); line++) {

				//se houver elemento (peso >0), grava peso no vetor allElements e loja (linha) no vetor storeOfElement
				if (matrixData.at(line).at(column - 1)>0) {
					allElements.push_back(matrixData.at(line).at(column - 1));
					storeOfElement.push_back(line);
				}

			}//fim do loop que lÍ uma coluna inteira de matrixData

		}//fim do loop que lÍ todas as colunas da matrixData

		 //guarda n˙mero de elementos da inst‚ncia
		nElements = (int)allElements.size();

	}//fim do procedimento realizado caso o arquivo de entrada tenha sido aberto com sucesso

	dataFile.close();



	//impress„o na tela para conferÍncia
	cout << "Elementos = {";
	for (unsigned j = 0; j<allElements.size(); j++) {
		cout << allElements.at(j) << " - ";
	} cout << "}\n";

	cout << "PosiÁ„o de cada categoria = {";
	for (unsigned j = 0; j< firstObjectPosition.size(); j++) {
		cout << firstObjectPosition.at(j) << " - ";
	} cout << "}\n";


}


//mÈtodo que resolve o problema relaxado
void Instances::solveRelaxedProblem()
{
	int totalWeight=0;
	for (unsigned j = 0; j < allElements.size(); j++) {
		totalWeight += allElements.at(j);
	}
	if(totalWeight%binCapacity >0) lowerBound = totalWeight / binCapacity + 1;
	else lowerBound = totalWeight / binCapacity;
}


#ifdef M_GRANDE
//mÈtodo que monta o problema no Gurobi e o resolve, retornando a soluÁ„o encontrada
vector<int> Instances::findOptimum() {

	vector<int> codifiedSolution;

	//otimizaÁ„o
	try {

		//INICIALIZA«√O DO GUROBI E CRIA«√O DO MODELO DE OTIMIZA«√O
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);

		//DEFINI«√O DE VARI¡VEIS

		//Binaria y(i) indicando se bin i È usado
		GRBVar* y = 0;
		y = model.addVars(nElements, GRB_BINARY);
		model.update();
		for (int i = 0; i < nElements; i++) {
			ostringstream name;
			name << "y(" << i << ")";
			y[i].set(GRB_StringAttr_VarName, name.str());
		}

		//Binaria x(i,j) que indica se objeto j est· alocado no bin i
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



		//DEFINI«√O FUN«√O OBJETIVO

		//express„o que soma y_i
		GRBLinExpr totalBins = 0;
		for (int i = 0; i < nElements; i++) {
			totalBins += y[i];
		}

		model.setObjective(totalBins, GRB_MINIMIZE);


		//RESTRI«’ES DO MODELO

		//respeita capacidade dos bins e vincula vari·veis x(i,j) e y(i)
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

				//calcula o Ìndice do ˙ltimo job da categoria k
				int last_j;
				if (k + 1 < mCategories) last_j = firstObjectPosition[k + 1] - 1;
				else last_j = nElements - 1;

				//soma x[i][j] de todo j pertencente ‡ k
				GRBLinExpr sum_xij_of_k = 0;
				for (int j = firstObjectPosition[k]; j <= last_j; j++) {
					sum_xij_of_k += x[i][j];
				}

				//cria restriÁ„o
				ostringstream cname;
				cname << "Determine f(" << i << ")(" << k << ")";
				model.addConstr(f[i][k] * M_GRANDE >= sum_xij_of_k, cname.str());

			}

		}

		//restriÁ„o de compatibilidade
		for (int k = 0; k < mCategories; k++) {

			for (int l = k + 1; l < mCategories; l++) {

				//se o valor da matriz de compatibilidade das categorias k e l for 0, h· incompatibilidade
				if (compatibilityMatrix[k][l] == 0) {

					//gera uma restriÁ„o para cada bin i
					for (int i = 0; i < nElements; i++) {
						ostringstream cname;
						cname << "AssureCategory(" << k << ")isntWithCategory(" << l << "onBin(" << i << ")";
						model.addConstr(f[i][k] + f[i][l] <= 1, cname.str());
					}

				}//gera restriÁıes apenas quando c[k][l]=0

			}

		}

		//restriÁ„o para reduzir espaÁo de soluÁıes
		for (int k = 0; k < nElements; k++) {
			for (int l = k + 1; l < nElements; l++) {
				ostringstream cname;
				cname << "AssureBin(" << k << ")isFilledBeforeBin(" << l << ")";
				model.addConstr(y[k] >= y[l], cname.str());
			}
		}


		//GERA ARQUIVO .lp COM MODELO CONSTRUÕDO
		string mName;
		mName = inputFile + ".lp";
		model.update();
		model.write(mName);

		//Insere limite de tempo para rodar otimizaÁ„o
		//model.getEnv().set(GRB_DoubleParam_TimeLimit, 600);

		//RODA OTIMIZA«√O
		model.optimize();

		//IMPRIME RESULTADOS
		int aux = 0;

		//percorre vetor de y(i), enquanto i for bin utilizado
		while (y[aux].get(GRB_DoubleAttr_X)>0) {

			//cout << y[aux].get(GRB_StringAttr_VarName) << " = " << y[aux].get(GRB_DoubleAttr_X);

			//insere separador na soluÁ„o, igual ao n˙mero do bin negativo
			codifiedSolution.push_back(-1 * (aux + 1));

			//percorre todos os x[i][j] do bin i
			for (int j = 0; j < nElements; j++) {

				//se o objeto j estiver no bin i
				if (x[aux][j].get(GRB_DoubleAttr_X)>0) {
					//cout << x[aux][j].get(GRB_StringAttr_VarName) << " = " << allElements[j] << ", ";
					//insere o Ìndice do elemento no vetor soluÁ„o
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

	return codifiedSolution;

}

#endif
