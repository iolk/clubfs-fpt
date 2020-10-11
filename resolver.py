import os
import os.path
import time

os.system("cd ..")

dir_testcases = 'tests/testcases/'
dir_results = 'tests/results/'

executor = 'main.exe'
if not os.path.isfile(executor):
	executor = 'main'

for filename in os.listdir(dir_testcases):
	if os.path.isfile(dir_testcases+filename):
		print(executor+' '+dir_testcases+filename)
		
		start_time = time.time()
		os.system(executor+' '+dir_testcases+filename)
		print("--- %s seconds ---" % (time.time() - start_time))
		os.system('plot_result.py '+dir_testcases+filename)

		name = filename.split('.')[0]
		os.replace('results.out', dir_results+name+'.out')
		os.replace('plot.gexf', dir_results+'plots/'+name+'.gexf')