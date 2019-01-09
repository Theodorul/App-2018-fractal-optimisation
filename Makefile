all: mandel-serial-wbuff mandel-parallel-openmp mandel-parallel-pthreads mandel-parallel-openmp-dynamic

mandel-serial-wbuff: mandel-serial-wbuff.c
	gcc -o mandel-serial mandel-serial-wbuff.c -lm	

mandel-parallel-pthreads: mandel-parallel-pthreads.c
	gcc -o mandel-parallel-pthreads mandel-parallel-pthreads.c -lm -pthread

mandel-parallel-openmp: mandel-parallel-openmp.c
	gcc -o mandel-parallel-openmp mandel-parallel-openmp.c -lm -fopenmp

mandel-parallel-openmp-dynamic: mandel-parallel-openmp-dynamic.c
	gcc -o $@ $^ -lm -fopenmp

run-serial-mandel: mandel-serial-wbuff
	time ./mandel-serial $$(cat input1-mandel)
run-serial-julia: mandel-serial-wbuff
	time ./mandel-serial $$(cat input2-julia)
run-serial-multiiulia: mandel-serial-wbuff
	time ./mandel-serial $$(cat input3-multiiulia)

run-parallel-openmp-julia: mandel-parallel-openmp
	time ./mandel-parallel-openmp $$(cat input2-julia)
run-parallel-openmp-mandel: mandel-parallel-openmp
	time ./mandel-parallel-openmp $$(cat input1-mandel)
run-parallel-openmp-multiiulia: mandel-parallel-openmp
	time ./mandel-parallel-openmp $$(cat input3-multiiulia)

run-parallel-pthreads-julia: mandel-parallel-pthreads
	time ./mandel-parallel-pthreads $$(cat input2-julia)
run-parallel-pthreads-mandel: mandel-parallel-pthreads
	time ./mandel-parallel-pthreads $$(cat input1-mandel)
run-parallel-pthreads-multiiulia: mandel-parallel-pthreads
	time ./mandel-parallel-pthreads $$(cat input3-multiiulia)

run-parallel-openmp-julia-dynamic: mandel-parallel-openmp-dynamic
	time ./mandel-parallel-openmp-dynamic $$(cat input2-julia)
run-parallel-openmp-mandel-dynamic: mandel-parallel-openmp-dynamic
	time ./mandel-parallel-openmp-dynamic $$(cat input1-mandel)
run-parallel-openmp-multiiulia-dynamic: mandel-parallel-openmp-dynamic
	time ./mandel-parallel-openmp-dynamic $$(cat input3-multiiulia)
