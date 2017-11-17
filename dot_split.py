import os

# I used domain tail(?) such as .com .kr .jp .fr etc
f = open('top-1m.csv')
directory = './dot_list/'
char_set = []
dot = ''
count_dot = 0
put_url = {}
count = 0
os.system('mkdir -p dot_list')

while True:
	line = f.readline()
	if not line:
		break
	count = count + 1
	line = line.split(',')[1] # removing index
	char_set = line.split('.')
	dot = char_set[len(char_set)-1]
	dot = dot[:-1] # erasing \n

	#print char_set
	#print dot # .com / .kr / etc
	
	# dictionary ex) .com : www.naver.com
	if (dot in put_url.keys()): # .keys() helps to print out all the keys in put_url dict.
		for key in put_url:
			if (key == dot):
				f2 = open(directory + dot + '.txt','a+') # append to the files	
				f2.write(line+'\n')
				#	put_url[dot].append(line[:-1])
				f2.close()
	else:
		put_url[dot] = []
		f2 = open(directory + dot + '.txt','w')
		f2.write(line+'\n')
		f2.close()
		#put_url[dot].append(line[:-1])
	
	if (count % 10000 == 0):
		print count
		
"""
print 'finished ' + put_url
for key,value in put_url:	# value is list!
	print key,value
	f2 = open(directory + key + '.txt', 'w')
	for i in range(len(value)):
		f2.write(value[i]+'\n')
	f2.close()


	 """
f.close()
