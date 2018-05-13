BEGIN{IFS="\t"}
{
	if{NF>2}
	{
	`	print $1 >> currentState;
		print $2 >> finalState;
		
	}
}