CPP=g++

default: bin/lxbh-resubmit

bin/%: src/%.C
	$(CPP) -o $@ $<

clean:
	rm -f bin/*
