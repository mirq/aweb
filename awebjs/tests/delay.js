/* Testing delay */


for(i = 0; i < 100; i++)
{
	delay(i);
	date = new Date();
	print(i, "\t",date.getTime());
}

