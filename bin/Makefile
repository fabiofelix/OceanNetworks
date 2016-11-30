# 
# <regra> -> id_regra: <dependencias> 
#                      <lista_comandos>
#<dependencias> -> <lista_regras> <lista_arquivos> | lambda
#<lista_regras> -> id_regra <lista_regras> | lambda
#<lista_arquivos> -> nome.extensão <lista_arquivos> | lambda
#<lista_comandos> -> comando \
#                    <lista_comandos> | lambda

CXX = g++
CXXFLAGS = -g -o 
SOURCE = extract
OPENCVFLAGS = `pkg-config --libs opencv` 

trabalho:
	$(CXX) $(SOURCE).cpp $(CXXFLAGS) $(SOURCE) $(OPENCVFLAGS)

