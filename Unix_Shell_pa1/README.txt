
			   ————— USER MANUAL —————

			     (press ‘q’ to Exit)


	Unix Terminal interface project (myshell.c)

	Author: Richard Smith


Description ——
	myshell.c is a simple Unix based shell/command line terminal
	that allows basic commands as listed below. 


Commands List ——
     	cd <directory> - change the current default directory to <directory>
      	clr - clear the screen
     	dir <directory> - list the contents of directory <directory>
     	environ - list all the environment strings
     	echo <comment> - display <comment> on the display followed by a new line
    	help - display the user manual (README.txt)
    	pause - pause operation of the shell until the ENTER key is pressed
	quit - quit the shell


Handling Pipe (|) and More filter —— 
	ls -l |more

	yet to be implemented



Handling Multiple Commands —— 
	date;cal;who

	yet to be implemented


Using Batchfiles —— 
	./myshell batchfile

	Not only can myshell.c take commands from keyboard (stdin), 
	the shell also can take lines of text from a batchfile and
	runs it as the input command.


I/O Redirection —— 
	programname arg1 arg2 < inputfile > outputfile
      
	myshell.c supports i/o-redirection on stdin and stdout.

      Input -
            programname < inputfile

     	    Takes in the a list of arguments to run with the program.

      Output -
            programname > outputfile

            Writes the program output to a file, if there was anything
            previously written to the file, it will be replaced.

      Append -
            programname >> outputfile

     	    Similar to “Output” but appends the current content of the
            outputfile and creates a new file if the outputfile does not exist.


Background Execution ——
	<command> &

	Background execution sounds exactly like what it is. It is a feature
	that allows for a program to run in the background. It keeps a
	process running until a command without background execution is called

	Examples:
		ls &
		echo hey there &



