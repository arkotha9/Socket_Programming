all: serverM.cpp serverC.cpp serverEE.cpp serverCS.cpp client.cpp
	g++ serverM.cpp -o serverM
	g++ serverC.cpp -o serverC
	g++ serverEE.cpp -o serverEE
	g++ serverCS.cpp -o serverCS
	g++ client.cpp -o client