.PHONY: all clean test

ALL := calc calc-test

all: $(ALL)

calc: calc.o engine.o
	$(CXX) -o $@ $^

calc-test: test.o engine.o
	$(CXX) -o $@ $^

test: calc-test
	@./calc-test

clean:
	rm -f $(ALL) *.o
