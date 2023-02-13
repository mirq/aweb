/* Sample ARexx script to get the directory of a selected drawer, and
   load the result back into the AWeb window */

/* Get the arguments in a string */
parse arg arguments

/* Initialize the individual parameters */
drawer = ''
dirsopt = ''
allopt = ''

/* Now, get each individual DOS-style argument */
do forever
   parse var arguments nextarg arguments

   /* Leave the iteration after the last argument */
   if nextarg == '' then leave

   /* Now the variable nextarg contains a DOS-style argument of the form
      name="value". Use this to set the appropriate variable */
   interpret nextarg
   end

/* Get a unique identifier */
id = pragma('ID')

/* Create a temporary file. Add a HTML header first: */
address command
'echo >t:dir_' || id '"<html><head><title>Directory of' drawer '</title></head>"'
'echo >>t:dir_' || id '"<body><h1>Directory of' drawer '</h1><pre>"'

/* Get the directory
   dirsopt is either 'DIRS' or empty
   allopt is either 'ALL' or empty */
'dir >>t:dir_' || id drawer dirsopt allopt

/* Load the temp file into AWeb.
   Use the RELOAD switch to force the file to be reloaded */
address
'open file://localhost/t:dir_' || id 'reload'

