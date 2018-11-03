
roxa_cc: roxa_cc.c

test: roxa_cc
	./test.sh

clean:
	rm -f roxa_cc *.o *~ tmp*

