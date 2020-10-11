import os
import os.path
import time
import multiprocessing
import concurrent.futures

dir_testcases = 'tests/testcases/'
dir_results = 'tests/results/'

def resolve_testcase(filename):	
	dir_testcases = 'tests/testcases/'
	dir_results = 'tests/results/'

	executor = 'main.exe'
	if not os.path.isfile(executor):
		executor = './main'	

	start_time = time.time()
	os.system(executor+' '+dir_testcases+filename)
	end_time = time.time()
	os.system('python3 plot_result.py '+dir_testcases+filename)

	name = filename.split('.')[0]
	os.replace('results.out', dir_results+name+'.out')
	os.replace('plot.gexf', dir_results+'plots/'+name+'.gexf')
	return executor+' '+dir_testcases+filename+"--- "+str(end_time - start_time)+" seconds ---"

files = []
for filename in os.listdir(dir_testcases):
	if os.path.isfile(dir_testcases+filename):
		files.append(filename)
		
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as exd:
	futures = []
	for name in files:
		future = exd.submit(resolve_testcase, name)
		futures.append(future)

	for future in futures:	
		print(future.result())
