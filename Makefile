CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall
OBJS     = main.o GerenciadorJogos.o ArvoreB.o IndiceSecundario.o
ALVO     = jogos

$(ALVO): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(ALVO) $(OBJS)

main.o: main.cpp GerenciadorJogos.h Registro.h
	$(CXX) $(CXXFLAGS) -c main.cpp

GerenciadorJogos.o: GerenciadorJogos.cpp GerenciadorJogos.h Registro.h ArvoreB.h IndiceSecundario.h
	$(CXX) $(CXXFLAGS) -c GerenciadorJogos.cpp

ArvoreB.o: ArvoreB.cpp ArvoreB.h
	$(CXX) $(CXXFLAGS) -c ArvoreB.cpp

IndiceSecundario.o: IndiceSecundario.cpp IndiceSecundario.h
	$(CXX) $(CXXFLAGS) -c IndiceSecundario.cpp

clean:
	rm -f $(ALVO) $(OBJS) *.dat *.idx *.inv
