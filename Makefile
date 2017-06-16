all:
	g++ pairs.cpp -o pairs `pkg-config --libs opencv`

clean:
	rm -f pairs
