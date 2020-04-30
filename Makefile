all: 
	gcc ./WTF.c -o WTF 
	
clean:
	rm WTF

everything: clean 
	echo "all done"

